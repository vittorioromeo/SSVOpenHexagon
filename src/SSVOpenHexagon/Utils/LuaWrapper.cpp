// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace Lua {

LuaContext::LuaContext(bool openDefaultLibs)
{
    // we pass this allocator function to lua_newstate we use our custom
    // allocator instead of luaL_newstate, to trace memory usage
    struct Allocator
    {
        static void* allocator(void*, void* ptr, std::size_t, std::size_t nsize)
        {
            if (nsize == 0)
            {
                free(ptr);
                return nullptr;
            }

            return realloc(ptr, nsize);
        }
    };

    // lua_newstate can return null if allocation failed
    _state = lua_newstate(&Allocator::allocator, nullptr);

    if (_state == nullptr)
    {
        throw std::bad_alloc{};
    }

    // opening default library if required to do so
    if (openDefaultLibs)
    {
        luaL_openlibs(_state);
    }
}

LuaContext::LuaContext(LuaContext&& s) noexcept : _state(s._state)
{
    s._state = nullptr;
}

LuaContext& LuaContext::operator=(LuaContext&& s) noexcept
{
    std::swap(_state, s._state);
    return *this;
}

LuaContext::~LuaContext()
{
    if (_state != nullptr)
    {
        lua_close(_state);
    }
}


LuaContext::ExecutionErrorException::ExecutionErrorException(
    const std::string& msg)
    : std::runtime_error(msg.c_str())
{}

LuaContext::VariableDoesntExistException::VariableDoesntExistException(
    const std::string& variable)
    : std::runtime_error((std::string("Variable \"") + variable +
                          std::string("\" doesn't exist in lua context"))
                             .c_str())
{}

LuaContext::SyntaxErrorException::SyntaxErrorException(const std::string& msg)
    : std::runtime_error(msg.c_str())
{}

LuaContext::WrongTypeException::WrongTypeException()
    : std::runtime_error("Trying to cast a lua variable to an invalid type")
{}

void LuaContext::_getGlobal(std::string_view mVarName) const
{
    // first a little optimization: if mVarName contains no dot, we can
    // directly call lua_getglobal
    if (std::find(mVarName.begin(), mVarName.end(), '.') == mVarName.end())
    {
        lua_getglobal(_state, mVarName.data());
        return;
    }

    // mVarName is split by dots '.' in arrays and subarrays
    // the nextVar variable contains a pointer to the next part to
    // proceed
    auto nextVar = mVarName.begin();

    do
    {
        // since we are going to modify nextVar, we store its value here
        auto currentVar = nextVar;

        // first we extract the part between currentVar and the next dot
        // we encounter
        nextVar = std::find(currentVar, mVarName.end(), '.');
        std::string buffer(currentVar, nextVar);
        // since nextVar is pointing to a dot, we have to increase it
        // first in order to find the next variable
        if (nextVar != mVarName.end()) ++nextVar;

        // ask lua to find the part stored in buffer
        // if currentVar == begin, this is a global variable and push it
        // on the stack
        // otherwise we already have an array pushed on the stack by the
        // previous loop
        if (currentVar == mVarName.begin())
        {
            lua_getglobal(_state, buffer.c_str());
        }
        else
        {
            // if mVarName is "a.b" and "a" is not a table (eg. it's a
            // number or a std::string), this happens
            // we don't have a specific exception for this, we consider
            // this as a variable-doesn't-exist
            if (!lua_istable(_state, -1))
            {
                lua_pop(_state, 1);
                throw VariableDoesntExistException(std::string{mVarName});
            }

            // replacing the current table in the stack by its member
            lua_pushstring(_state, buffer.c_str());
            lua_gettable(_state, -2);
            lua_remove(_state, -2);
        }

        // lua will accept anything as variable name, but if the
        // variable doesn't exist
        //   it will simply push "nil" instead of a value
        // so if we have a nil on the stack, the variable didn't exist
        // and we throw
        if (lua_isnil(_state, -1))
        {
            lua_pop(_state, 1);
            throw VariableDoesntExistException(std::string{mVarName});
        }

        currentVar = nextVar; // updating currentVar
    }
    while (nextVar != mVarName.end());
}

void LuaContext::_setGlobal(std::string_view mVarName)
try
{
    SSVOH_ASSERT(lua_gettop(_state) >= 1); // making sure there's
                                           // something on the stack
                                           // (ie. the value to set)

    // two possibilities: either "variable" is a global variable, or
    // a member of an array
    std::size_t lastDot = mVarName.find_last_of('.');

    if (lastDot == std::string::npos)
    {
        // this is the first case, we simply call setglobal (which
        // cleans the stack)
        lua_setglobal(_state, mVarName.data());
        return;
    }

    std::string varNameAsStr{mVarName}; // needed for null-terminated substrs
    const auto tableName = varNameAsStr.substr(0, lastDot);

    // in the second case, we call _getGlobal on the table name
    _getGlobal(tableName);

    try
    {
        if (!lua_istable(_state, -1))
        {
            throw VariableDoesntExistException(varNameAsStr);
        }

        // now we have our value at -2 (was pushed before
        // _setGlobal is called) and our table at -1
        lua_pushstring(_state,
            varNameAsStr.substr(lastDot + 1).c_str()); // value at -3,
                                                       // table at -2,
                                                       // key at -1
        lua_pushvalue(_state, -3); // value at -4, table at -3,
                                   // key at -2, value at -1
        lua_settable(_state, -3);  // value at -2, table at -1
        lua_pop(_state, 2);        // stack empty \o/
    }
    catch (...)
    {
        lua_pop(_state, 2);
        throw;
    }
}
catch (...)
{
    lua_pop(_state, 1);
    throw;
}

void* LuaContext::callbackCallImpl(lua_State* lua)
{
    // this function is called when the lua script tries to call our
    // custom data type
    // what we do is we simply call the function
    SSVOH_ASSERT(lua_gettop(lua) >= 1);
    SSVOH_ASSERT(lua_isuserdata(lua, 1));

    auto function = lua_touserdata(lua, 1);

    SSVOH_ASSERT(function);
    return function;
}


void LuaContext::_load(std::istream& code)
{
    // since the lua_load function requires a static function, we use
    // this structure
    // the Reader structure is at the same time an object storing an
    // std::istream and a buffer,
    // and a static function provider
    struct Reader
    {
        std::istream& stream;
        char buffer[512];

        Reader(std::istream& str) : stream(str)
        {}

        // read function; "data" must be an instance of Reader
        static const char* read(lua_State*, void* data, std::size_t* size)
        {
            SSVOH_ASSERT(size != nullptr);
            SSVOH_ASSERT(data != nullptr);

            Reader& me = *((Reader*)data);
            if (me.stream.eof())
            {
                *size = 0;
                return nullptr;
            }

            me.stream.read(me.buffer, sizeof(me.buffer));
            *size = std::size_t(
                me.stream.gcount()); // gcount could return a value
                                     // larger than a std::size_t, but
                                     // its maximum is sizeof(me.buffer)
                                     // so there's no problem
            return me.buffer;
        }
    };

    // we create an instance of Reader, and we call lua_load
    Reader reader(code);

    const int loadReturnValue =
        lua_load(_state, &Reader::read, &reader, "chunk");

    // now we have to check return value
    if (loadReturnValue != 0)
    {
        // there was an error during loading, an error message was
        // pushed on the stack
        const std::string errorMsg = _readTopAndPop(1, (std::string*)nullptr);

        if (loadReturnValue == LUA_ERRMEM)
        {
            throw std::bad_alloc();
        }
        else if (loadReturnValue == LUA_ERRSYNTAX)
        {
            throw SyntaxErrorException(errorMsg);
        }
    }
}

void LuaContext::_load(std::string_view code)
{
    const int loadReturnValue =
        luaL_loadbuffer(_state, code.data(), code.size(), "chunk");

    // now we have to check return value
    if (loadReturnValue != 0)
    {
        // there was an error during loading, an error message was
        // pushed on the stack
        const std::string errorMsg = _readTopAndPop(1, (std::string*)nullptr);

        if (loadReturnValue == LUA_ERRMEM)
        {
            throw std::bad_alloc();
        }
        else if (loadReturnValue == LUA_ERRSYNTAX)
        {
            throw SyntaxErrorException(errorMsg);
        }
    }
}

void LuaContext::_pushSPtrImpl(int (*garbageCallback)(lua_State*),
    const std::type_info& tiSharedPtr, const std::type_info& tiObject)
try
{
    // creating the metatable (over the object on the stack)
    // lua_settable pops the key and value we just pushed, so stack
    // management is easy
    // all that remains on the stack after these function calls is
    // the metatable
    lua_newtable(_state);

    try
    {
        // using the garbage collecting function we created above
        lua_pushstring(_state, "__gc");
        lua_pushcfunction(_state, garbageCallback);
        lua_settable(_state, -3);

        // settings typeid of shared_ptr this time
        lua_pushstring(_state, "_typeid");
        lua_pushlightuserdata(
            _state, const_cast<std::type_info*>(&tiSharedPtr));
        lua_settable(_state, -3);

        // as __index we set the table located in registry at type
        // name
        // see comments at _registerFunction
        lua_pushstring(_state, "__index");
        lua_pushlightuserdata(_state, const_cast<std::type_info*>(&tiObject));
        lua_gettable(_state, LUA_REGISTRYINDEX);

        if (!lua_istable(_state, -1))
        {
            SSVOH_ASSERT(lua_isnil(_state, -1));
            lua_pop(_state, 1);
            lua_newtable(_state);
            lua_pushlightuserdata(
                _state, const_cast<std::type_info*>(&tiObject));
            lua_pushvalue(_state, -2);
            lua_settable(_state, LUA_REGISTRYINDEX);
        }

        lua_settable(_state, -3);

        // at this point, the stack contains the object at offset -2
        // and the metatable at offset -1
        // lua_setmetatable will bind the two together and pop the
        // metatable
        // our custom type remains on the stack (and that's what we
        // want since this is a push function)
        lua_setmetatable(_state, -2);
    }
    catch (...)
    {
        lua_pop(_state, 1);
        throw;
    }
}
catch (...)
{
    lua_pop(_state, 1);
    throw;
}

void LuaContext::_pushFnImpl(int (*callbackCall)(lua_State*),
    int (*callbackGarbage)(lua_State*), const std::type_info& tiObject)
{
    // creating the metatable (over the object on the stack)
    // lua_settable pops the key and value we just pushed, so stack
    // management is easy
    // all that remains on the stack after these function calls is the
    // metatable
    lua_newtable(_state);

    lua_pushstring(_state, "__call");
    lua_pushcfunction(_state, callbackCall);
    lua_settable(_state, -3);

    lua_pushstring(_state, "_typeid");
    lua_pushlightuserdata(_state, const_cast<std::type_info*>(&tiObject));
    lua_settable(_state, -3);

    lua_pushstring(_state, "__gc");
    lua_pushcfunction(_state, callbackGarbage);
    lua_settable(_state, -3);

    // at this point, the stack contains the object at offset -2 and the
    // metatable at offset -1
    // lua_setmetatable will bind the two together and pop the metatable
    // our custom function remains on the stack (and that's what we
    // want)
    lua_setmetatable(_state, -2);
}

void LuaContext::_registerFunctionImpl(
    const char* nameCStr, const std::type_info& tiObjectType)
{
    // trying to get the existing functions list
    lua_pushlightuserdata(_state, const_cast<std::type_info*>(&tiObjectType));

    lua_gettable(_state, LUA_REGISTRYINDEX);

    // if it doesn't exist, we create one, then write it in registry but
    // keep it pushed
    if (!lua_istable(_state, -1))
    {
        SSVOH_ASSERT(lua_isnil(_state, -1));

        lua_pop(_state, 1);
        lua_newtable(_state);
        lua_pushlightuserdata(
            _state, const_cast<std::type_info*>(&tiObjectType));
        lua_pushvalue(_state, -2);
        lua_settable(_state, LUA_REGISTRYINDEX);
    }

    // now we have our functions list on top of the stack, we write the
    // function here
    lua_pushstring(_state, nameCStr);
}

} // namespace Lua
