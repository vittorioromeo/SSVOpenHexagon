// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

/*
Copyright (c) 2010, Pierre KRIEGER
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Check out Pierre Krieger's new version of luawrapper here:
// https://github.com/Tomaka17/luawrapper

#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <tuple>
#include <type_traits>

extern "C"
{

#ifdef SSVOH_USE_LUAJIT
#include <luajit.h>
#else
#include <lua.h>
#endif

#include <lualib.h>
#include <lauxlib.h>
}

namespace Lua
{

template <typename>
struct RemoveMemberPtr;

template <typename TReturn, typename TThis, typename... TArgs>
struct RemoveMemberPtr<TReturn (TThis::*)(TArgs...) const>
{
    using Type = TReturn(TArgs...);
};


template <typename FnType>
struct FnTupleWrapper
{
};

template <typename... TArgs>
struct FnTupleWrapper<void(TArgs...)>
{
    static constexpr int count{sizeof...(TArgs)};
    using ParamsType = std::tuple<TArgs...>;

    template <typename T>
    [[nodiscard, gnu::always_inline]] inline constexpr static std::tuple<> call(
        T&& fn, ParamsType&& mTpl)
    {
        std::apply(std::forward<T>(fn), std::move(mTpl));
        return {};
    }
};

template <typename R, typename... TArgs>
struct FnTupleWrapper<R(TArgs...)>
{
    static constexpr int count{sizeof...(TArgs)};
    using ParamsType = std::tuple<TArgs...>;

    template <typename T>
    [[nodiscard, gnu::always_inline]] inline constexpr static std::tuple<R>
    call(T&& fn, ParamsType&& mTpl)
    {
        return std::tuple<R>{std::apply(std::forward<T>(fn), std::move(mTpl))};
    }
};

/**
 * @brief Defines a Lua context
 *
 * A Lua context is used to interpret Lua code. Since everything in Lua is a
 * variable (including functions),
 * we only provide few functions like readVariable and writeVariable. Note
 * that these functions can visit arrays,
 * ie. calling readVariable("a.b.c") will read variable "c" from array "b",
 * which is itself located in array "a".
 * You can also write variables with C++ functions so that they are callable
 * by Lua. Note however that you HAVE TO convert
 * your function to std::function (not directly std::bind or a lambda
 * function) so the class can detect which argument types
 * it wants. These arguments may only be of basic types (int, float, etc.)
 * or std::string.
 */
class LuaContext
{
public:
    explicit LuaContext(bool openDefaultLibs = true)
    {
        // we pass this allocator function to lua_newstate we use our custom
        // allocator instead of luaL_newstate, to trace memory usage
        struct Allocator
        {
            static void* allocator(
                void*, void* ptr, std::size_t, std::size_t nsize)
            {
                if(nsize == 0)
                {
                    free(ptr);
                    return nullptr;
                }

                return realloc(ptr, nsize);
            }
        };

        // lua_newstate can return null if allocation failed
        _state = lua_newstate(&Allocator::allocator, nullptr);

        if(_state == nullptr)
        {
            throw std::bad_alloc{};
        }

        // opening default library if required to do so
        if(openDefaultLibs)
        {
            luaL_openlibs(_state);
        }
    }

    LuaContext(const LuaContext&) = delete;
    LuaContext& operator=(const LuaContext&) = delete;

    LuaContext(LuaContext&& s) : _state(s._state)
    {
        s._state = nullptr;
    }

    LuaContext& operator=(LuaContext&& s)
    {
        std::swap(_state, s._state);
        return *this;
    }

    ~LuaContext()
    {
        if(_state != nullptr)
        {
            lua_close(_state);
        }
    }


    /// \brief The table type can store any key and any value, and can be
    /// read or written by LuaContext
    class Table;

    /// \brief Thrown when an error happens during execution (like not
    /// enough parameters for a function)
    struct ExecutionErrorException : std::runtime_error
    {
        ExecutionErrorException(const std::string& msg)
            : std::runtime_error(msg.c_str())
        {
        }
    };

    /// \brief Generated by readVariable or isVariableArray when the asked
    /// variable doesn't exist/is nil
    struct VariableDoesntExistException : std::runtime_error
    {
        VariableDoesntExistException(const std::string& variable)
            : std::runtime_error(
                  (std::string("Variable \"") + variable +
                      std::string("\" doesn't exist in lua context"))
                      .c_str())
        {
        }
    };

    /// \brief Thrown when a syntax error happens in a lua script
    struct SyntaxErrorException : std::runtime_error
    {
        SyntaxErrorException(const std::string& msg)
            : std::runtime_error(msg.c_str())
        {
        }
    };

    /// \brief Thrown when trying to cast a lua variable to an invalid type
    struct WrongTypeException : std::runtime_error
    {
        WrongTypeException()
            : std::runtime_error(
                  "Trying to cast a lua variable to an invalid type")
        {
        }
    };

    /// \brief Executes lua code from the stream \param code A stream that
    /// lua will read its code from
    [[gnu::always_inline]] inline void executeCode(std::istream& code)
    {
        _load(code);
        _call<std::tuple<>>(std::tuple<>());
    }

    /// \brief Executes lua code from the stream and returns a value \param
    /// code A stream that lua will read its code from
    template <typename T>
    [[nodiscard, gnu::always_inline]] inline T executeCode(std::istream& code)
    {
        _load(code);
        return _call<T>(std::tuple<>());
    }

    /// \brief Executes lua code given as parameter \param code A string
    /// containing code that will be executed by lua
    [[gnu::always_inline]] inline void executeCode(const std::string& code)
    {
        _load(code);
        _call<std::tuple<>>(std::tuple<>());
    }

    /// \brief Executes lua code given as parameter and returns a value
    /// \param code A string containing code that will be executed by lua
    template <typename T>
    [[nodiscard, gnu::always_inline]] inline T executeCode(
        const std::string& code)
    {
        std::istringstream str(code);
        return executeCode<T>(str);
    }


    /// \brief Tells that lua will be allowed to access an object's function
    template <typename T, typename R, typename... Args>
    [[gnu::always_inline]] inline void registerFunction(
        const std::string& name, R (T::*f)(Args...))
    {
        _registerFunction(
            name, [f](const std::shared_ptr<T>& ptr, Args... args) {
                return ((*ptr).*f)(args...);
            });
    }

    template <typename T, typename R, typename... Args>
    [[gnu::always_inline]] inline void registerFunction(
        const std::string& name, R (T::*f)(Args...) const)
    {
        _registerFunction(
            name, [f](const std::shared_ptr<T>& ptr, Args... args) {
                return ((*ptr).*f)(args...);
            });
    }

    template <typename T, typename R, typename... Args>
    [[gnu::always_inline]] inline void registerFunction(
        const std::string& name, R (T::*f)(Args...) volatile)
    {
        _registerFunction(
            name, [f](const std::shared_ptr<T>& ptr, Args... args) {
                return ((*ptr).*f)(args...);
            });
    }

    template <typename T, typename R, typename... Args>
    [[gnu::always_inline]] inline void registerFunction(
        const std::string& name, R (T::*f)(Args...) const volatile)
    {
        _registerFunction(
            name, [f](const std::shared_ptr<T>& ptr, Args... args) {
                return ((*ptr).*f)(args...);
            });
    }

    /// \brief Adds a custom function to a type determined using the
    /// function's first parameter
    /// \sa allowFunction
    /// \param fn Function which takes as first parameter a std::shared_ptr
    template <typename T>
    [[gnu::always_inline]] inline void registerFunction(
        const std::string& name, T&& fn, decltype(&T::operator())* = nullptr)
    {
        _registerFunction(name, std::forward<T>(fn));
    }

    /// \brief Inverse operation of registerFunction
    template <typename T>
    [[gnu::always_inline]] inline void unregisterFunction(
        const std::string& name)
    {
        _unregisterFunction<T>(name);
    }

    /// \brief Calls a function stored in a lua variable
    /// \details Template parameter of the function should be the expected
    /// return type (tuples and void are supported)
    /// \param mVarName Name of the variable containing the function to call
    /// \param ... Parameters to pass to the function
    template <typename R, typename... Args>
    [[nodiscard, gnu::always_inline]] inline R callLuaFunction(
        const std::string& mVarName, Args&&... args)
    {
        _getGlobal(mVarName);
        return _call<R>(std::make_tuple(std::forward<Args>(args)...));
    }

    /// \brief Returns true if the value of the variable is an array \param
    /// mVarName Name of the variable to check
    [[nodiscard, gnu::always_inline]] inline bool isVariableArray(
        const std::string& mVarName) const
    {
        _getGlobal(mVarName);

        const bool answer = lua_istable(_state, -1);
        lua_pop(_state, 1);
        return answer;
    }

    /// \brief Writes an empty array into the given variable \note To write
    /// something in the array, use writeVariable. Example:
    /// writeArrayIntoVariable("myArr"); writeVariable("myArr.something",
    /// 5);
    [[gnu::always_inline]] inline void writeArrayIntoVariable(
        const std::string& mVarName)
    {
        lua_newtable(_state);
        _setGlobal(mVarName);
    }

    /// \brief Returns true if variable exists (ie. not nil)
    [[nodiscard, gnu::always_inline]] inline bool doesVariableExist(
        const std::string& mVarName) const
    {
        _getGlobal(mVarName);

        const bool answer = lua_isnil(_state, -1);
        lua_pop(_state, 1);
        return !answer;
    }

    /// \brief Destroys a variable \details Puts the nil value into it
    [[gnu::always_inline]] inline void clearVariable(
        const std::string& mVarName)
    {
        lua_pushnil(_state);
        _setGlobal(mVarName);
    }

    /// \brief Returns the content of a variable \throw
    /// VariableDoesntExistException if variable doesn't exist \note If you
    /// wrote a ObjectWrapper<T> into a variable, you can only read its
    /// value using a std::shared_ptr<T>
    template <typename T>
    [[nodiscard, gnu::always_inline]] inline T readVariable(
        const std::string& mVarName) const
    {
        _getGlobal(mVarName);
        return _readTopAndPop(1, (T*)nullptr);
    }

    /// \brief
    template <typename T>
    [[nodiscard, gnu::always_inline]] inline bool readVariableIfExists(
        const std::string& mVarName, T& out)
    {
        if(!doesVariableExist(mVarName))
        {
            return false;
        }

        out = readVariable<T>(mVarName);
        return true;
    }

    /// \brief Changes the content of a global lua variable
    /// \details Accepted values are: all base types (integers, floats),
    /// std::string, std::function or ObjectWrapper<...>. All objects are
    /// passed by copy and destroyed by the garbage collector.
    template <typename T>
    void writeVariable(const std::string& mVarName, T&& data)
    {
        static_assert(!std::is_same_v<std::tuple<T>, T>,
            "Error: you can't use LuaContext::writeVariable with a tuple");

        const int pushedElems = _push(std::forward<T>(data));

        try
        {
            _setGlobal(mVarName);
        }
        catch(...)
        {
            lua_pop(_state, pushedElems - 1);
            throw;
        }

        lua_pop(_state, pushedElems - 1);
    }

private:
    // the state is the most important variable in the class since it is our
    // interface with Lua
    // the mutex is here because the lua design is not thread safe (based on
    // a stack)
    //   eg. if multiple thread call "writeVariable" at the same time, we
    //   don't want them to be executed simultaneously
    // the mutex should be locked by all public functions that use the stack
    lua_State* _state;

    // all the user types in the _state must have the value of &typeid(T) in
    // their
    //   metatable at key "_typeid"

    // the getGlobal function is equivalent to lua_getglobal, except that it
    // can interpret
    //   any variable name in a table form (ie. names like table.value are
    //   supported)
    // see also https://www.lua.org/manual/5.1/manual.html#lua_getglobal
    // same for setGlobal <=> lua_setglobal
    // important: _setGlobal will pop the value even if it throws an
    // exception, while _getGlobal won't push the value if it throws an
    // exception
    void _getGlobal(const std::string& mVarName) const
    {
        // first a little optimization: if mVarName contains no dot, we can
        // directly call lua_getglobal
        if(find(mVarName.begin(), mVarName.end(), '.') == mVarName.end())
        {
            lua_getglobal(_state, mVarName.c_str());
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
            nextVar = find(currentVar, mVarName.end(), '.');
            std::string buffer(currentVar, nextVar);
            // since nextVar is pointing to a dot, we have to increase it
            // first in order to find the next variable
            if(nextVar != mVarName.end()) ++nextVar;

            // ask lua to find the part stored in buffer
            // if currentVar == begin, this is a global variable and push it
            // on the stack
            // otherwise we already have an array pushed on the stack by the
            // previous loop
            if(currentVar == mVarName.begin())
            {
                lua_getglobal(_state, buffer.c_str());
            }
            else
            {
                // if mVarName is "a.b" and "a" is not a table (eg. it's a
                // number or a std::string), this happens
                // we don't have a specific exception for this, we consider
                // this as a variable-doesn't-exist
                if(!lua_istable(_state, -1))
                {
                    lua_pop(_state, 1);
                    throw VariableDoesntExistException(mVarName);
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
            if(lua_isnil(_state, -1))
            {
                lua_pop(_state, 1);
                throw VariableDoesntExistException(mVarName);
            }

            currentVar = nextVar; // updating currentVar
        } while(nextVar != mVarName.end());
    }

    void _setGlobal(const std::string& mVarName)
    {
        try
        {
            assert(lua_gettop(_state) >= 1); // making sure there's
                                             // something on the stack (ie.
                                             // the value to set)

            // two possibilities: either "variable" is a global variable, or
            // a member of an array
            std::size_t lastDot = mVarName.find_last_of('.');

            if(lastDot == std::string::npos)
            {
                // this is the first case, we simply call setglobal (which
                // cleans the stack)
                lua_setglobal(_state, mVarName.c_str());
            }
            else
            {
                const auto tableName = mVarName.substr(0, lastDot);

                // in the second case, we call _getGlobal on the table name
                _getGlobal(tableName);

                try
                {
                    if(!lua_istable(_state, -1))
                    {
                        throw VariableDoesntExistException(mVarName);
                    }

                    // now we have our value at -2 (was pushed before
                    // _setGlobal is called) and our table at -1
                    lua_pushstring(_state,
                        mVarName.substr(lastDot + 1).c_str()); // value at -3,
                                                               // table at -2,
                                                               // key at -1
                    lua_pushvalue(_state, -3); // value at -4, table at -3,
                                               // key at -2, value at -1
                    lua_settable(_state, -3);  // value at -2, table at -1
                    lua_pop(_state, 2);        // stack empty \o/
                }
                catch(...)
                {
                    lua_pop(_state, 2);
                    throw;
                }
            }
        }
        catch(...)
        {
            lua_pop(_state, 1);
            throw;
        }
    }
    // simple function that reads the top # elements of the stack, pops
    // them, and returns them
    // warning: first parameter is the number of parameters, not the
    // parameter index
    // if _read generates an exception, stack is popped anyway
    template <typename R>
    [[gnu::always_inline]] inline std::enable_if_t<!std::is_void_v<R>, R>
    _readTopAndPop(int nb, R* ptr = nullptr) const
    {
        try
        {
            const R value = _read(-nb, ptr);
            lua_pop(_state, nb);
            return value;
        }
        catch(...)
        {
            lua_pop(_state, nb);
            throw;
        }
    }

    [[gnu::always_inline]] inline void _readTopAndPop(int nb, void*) const
    {
        lua_pop(_state, nb);
    }

    /**************************************************/
    /*            FUNCTIONS REGISTRATION              */
    /**************************************************/
    // the "registerFunction" public functions call this one
    // this function writes in registry the list of functions for each
    // possible custom type (ie. T when pushing std::shared_ptr<T>)
    // to be clear, registry[&typeid(type)] contains an array where keys are
    // function names and values are functions
    //              (where type is the first parameter of the functor)
    template <typename T>
    void _registerFunction(const std::string& name, T&& function)
    {
        using FunctionType =
            typename RemoveMemberPtr<decltype(&T::operator())>::Type;

        using ObjectType = std::tuple_element_t<0,
            typename FnTupleWrapper<FunctionType>::ParamsType>::element_type;

        // trying to get the existing functions list
        lua_pushlightuserdata(
            _state, const_cast<std::type_info*>(&typeid(ObjectType)));

        lua_gettable(_state, LUA_REGISTRYINDEX);

        // if it doesn't exist, we create one, then write it in registry but
        // keep it pushed
        if(!lua_istable(_state, -1))
        {
            assert(lua_isnil(_state, -1));

            lua_pop(_state, 1);
            lua_newtable(_state);
            lua_pushlightuserdata(
                _state, const_cast<std::type_info*>(&typeid(ObjectType)));
            lua_pushvalue(_state, -2);
            lua_settable(_state, LUA_REGISTRYINDEX);
        }

        // now we have our functions list on top of the stack, we write the
        // function here
        lua_pushstring(_state, name.c_str());
        _push(std::forward<T>(function));
        lua_settable(_state, -3);
        lua_pop(_state, 1);
    }

    // inverse operation of _registerFunction
    template <typename T>
    void _unregisterFunction(const std::string& name)
    {
        // trying to get the existing functions list
        lua_pushlightuserdata(_state, const_cast<std::type_info*>(&typeid(T)));
        lua_gettable(_state, LUA_REGISTRYINDEX);

        if(!lua_istable(_state, -1))
        {
            lua_pop(_state, -1);
            return;
        }

        lua_pushstring(_state, name.c_str());
        lua_pushnil(_state);
        lua_settable(_state, -3);
        lua_pop(_state, 1);
    }

    /**************************************************/
    /*              LOADING AND CALLING               */
    /**************************************************/
    // this function loads data from the stream and pushes a function at the
    // top of the stack
    // it is defined in the .cpp
    void _load(std::istream& code)
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
            {
            }

            // read function; "data" must be an instance of Reader
            static const char* read(lua_State*, void* data, std::size_t* size)
            {
                assert(size != nullptr);
                assert(data != nullptr);

                Reader& me = *((Reader*)data);
                if(me.stream.eof())
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
            lua_load(_state, &Reader::read, &reader, "chunk"
#ifndef SSVOH_USE_LUAJIT
                ,
                nullptr
#endif
            );

        // now we have to check return value
        if(loadReturnValue != 0)
        {
            // there was an error during loading, an error message was
            // pushed on the stack
            const char* errorMsg = lua_tostring(_state, -1);
            lua_pop(_state, 1);

            if(loadReturnValue == LUA_ERRMEM)
            {
                throw std::bad_alloc();
            }
            else if(loadReturnValue == LUA_ERRSYNTAX)
            {
                throw SyntaxErrorException(std::string(errorMsg));
            }
        }
    }

    void _load(const std::string& code)
    {

        // since the lua_load function requires a static function, we use
        // this structure
        // the Reader structure is at the same time an object storing an
        // std::istream and a buffer,
        // and a static function provider
        struct SReader
        {
            const std::string& str;
            bool consumed;

            SReader(const std::string& s) : str(s), consumed{false}
            {
            }

            // read function; "data" must be an instance of Reader
            static const char* read(lua_State*, void* data, std::size_t* size)
            {
                assert(size != nullptr);
                assert(data != nullptr);

                SReader& me = *((SReader*)data);

                if(me.consumed || me.str.empty())
                {
                    *size = 0;
                    return nullptr;
                }

                me.consumed = true;

                *size = me.str.size();
                return me.str.data();
            }
        };

        // we create an instance of Reader, and we call lua_load
        SReader sReader(code);
        const int loadReturnValue =
            lua_load(_state, &SReader::read, &sReader, "chunk"
#ifndef SSVOH_USE_LUAJIT
                ,
                nullptr
#endif
            );

        // now we have to check return value
        if(loadReturnValue != 0)
        {
            // there was an error during loading, an error message was
            // pushed on the stack
            const char* errorMsg = lua_tostring(_state, -1);
            lua_pop(_state, 1);

            if(loadReturnValue == LUA_ERRMEM)
            {
                throw std::bad_alloc();
            }
            else if(loadReturnValue == LUA_ERRSYNTAX)
            {
                throw SyntaxErrorException(std::string(errorMsg));
            }
        }
    }

    // this function calls what is on the top of the stack and removes it
    // (just like lua_call)
    // if an exception is triggered, the top of the stack will be removed
    // anyway
    // In should be a tuple (at least until variadic templates are supported
    // everywhere), Out can be anything
    template <typename Out, typename In>
    Out _call(const In& in)
    {
        static_assert(std::tuple_size_v<In> >= 0,
            "Error: template parameter 'In' should be a tuple");

        int outArguments{0};
        int inArguments{0};

        try
        {
            // we push the parameters on the stack
            outArguments = std::tuple_size_v<std::tuple<Out>>;
            inArguments = _push(in);
        }
        catch(...)
        {
            lua_pop(_state, 1);
            throw;
        }

        // calling pcall automatically pops the parameters and pushes output
        const int pcallReturnValue =
            lua_pcall(_state, inArguments, outArguments, 0);

        // if pcall failed, analyzing the problem and throwing
        if(pcallReturnValue != 0)
        {
            // an error occurred during execution, an error message was
            // pushed on the stack
            const std::string errorMsg =
                _readTopAndPop(1, (std::string*)nullptr);

            if(pcallReturnValue == LUA_ERRMEM)
            {
                throw std::bad_alloc();
            }
            else if(pcallReturnValue == LUA_ERRRUN)
            {
                throw ExecutionErrorException(errorMsg);
            }
            else
            {
                throw std::runtime_error("UNKNOWN RC: " + errorMsg);
            }
        }

        // pcall succeeded, we pop the returned values and return them
        try
        {
            return _readTopAndPop(outArguments, (Out*)nullptr);
        }
        catch(...)
        {
            lua_pop(_state, outArguments);
            throw;
        }
    }


    /**************************************************/
    /*                 TABLE CLASS                    */
    /**************************************************/
public:
    class Table
    {
    public:
        Table() = default;

        Table(Table&& t)
        {
            swap(t, *this);
        }

        Table& operator=(Table&& t)
        {
            swap(t, *this);
            return *this;
        }

        template <typename... Args>
        explicit Table(Args&&... args)
        {
            insert(std::forward<Args>(args)...);
        }

        friend void swap(Table& a, Table& b)
        {
            std::swap(a._elements, b._elements);
        }

        template <typename Key, typename Value, typename... Args>
        void insert(Key&& k, Value&& v, Args&&... args)
        {
            typedef typename ToPushableType<std::decay_t<Key>>::type RKey;
            typedef typename ToPushableType<std::decay_t<Value>>::type RValue;

            _elements.emplace_back(new Element<RKey, RValue>(
                std::forward<Key>(k), std::forward<Value>(v)));

            insert(std::forward<Args>(args)...);
        }

        void insert()
        {
        }

        template <typename Value, typename Key>
        Value read(const Key& key)
        {
            using Key2 = typename ToPushableType<Key>::type;
            using Value2 = typename ToPushableType<Value>::type;

            for(auto i = _elements.rbegin(); i != _elements.rend(); ++i)
            {
                auto element = dynamic_cast<Element<Key2, Value2>*>(i->get());

                if(element != nullptr && element->key == key)
                {
                    return element->value;
                }
            }

            throw VariableDoesntExistException("<key in table>");
        }

    private:
        Table(const Table&);
        Table& operator=(const Table&);

        // this is the base structure
        // the push function should add the key/value pair to the table
        // currently at the top of the stack
        struct ElementBase
        {
            virtual ~ElementBase()
            {
            }

            virtual void push(LuaContext&) const = 0;
        };

        // derivate of ElementBase, real implementation
        template <typename Key, typename Value>
        struct Element : public ElementBase
        {
            Key key;
            Value value;

            Element(Key&& k, Value&& v)
                : key(std::forward<Key>(k)), value(std::forward<Value>(v))
            {
            }

            void push(LuaContext& ctxt) const
            {
                assert(lua_istable(ctxt._state, -1));

                ctxt._push(key);
                ctxt._push(value);
                lua_settable(ctxt._state, -3);
            }
        };

        // pushing the whole array
        friend class LuaContext;

        int _push(LuaContext& ctxt) const
        {
            lua_newtable(ctxt._state);

            try
            {
                for(auto& i : _elements)
                {
                    i->push(ctxt);
                }
            }
            catch(...)
            {
                lua_pop(ctxt._state, 1);
                throw;
            }

            return 1;
        }

        // elements storage
        std::vector<std::unique_ptr<ElementBase>> _elements;
    };

private:
    /**************************************************/
    /*                PUSH FUNCTIONS                  */
    /**************************************************/
    // this structure converts an input type to a pushable output type
    template <typename Input, typename = void>
    struct ToPushableType;

    // first the basic ones: integer, number, boolean, string
    [[gnu::always_inline]] inline int _push()
    {
        return 0;
    }

    [[gnu::always_inline]] inline int _push(bool v)
    {
        lua_pushboolean(_state, v);
        return 1;
    }

    [[gnu::always_inline]] inline int _push(const std::string& s)
    {
        lua_pushstring(_state, s.c_str());
        return 1;
    }

    [[gnu::always_inline]] inline int _push(const char* s)
    {
        lua_pushstring(_state, s);
        return 1;
    }

    // pushing floating numbers
    template <typename T>
    [[gnu::always_inline]] inline std::enable_if_t<std::is_floating_point_v<T>,
        int>
    _push(T nb)
    {
        lua_pushnumber(_state, nb);
        return 1;
    }

    // pushing integers
    template <typename T>
    [[gnu::always_inline]] inline std::enable_if_t<std::is_integral_v<T>, int>
    _push(T nb)
    {
        lua_pushinteger(_state, nb);
        return 1;
    }

    // using variadic templates, you can push multiple values at once
    template <typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) > 1)>>
    [[gnu::always_inline]] inline int _push(Ts&&... xs)
    {
        int p = 0;

        try
        {
            ((p += _push(std::forward<Ts>(xs))), ...);
        }
        catch(...)
        {
            lua_pop(_state, p);
            throw;
        }

        return p;
    }

    // pushing tables
    [[gnu::always_inline]] inline int _push(const Table& table)
    {
        return table._push(*this);
    }

    // pushing maps
    template <typename Key, typename Value>
    int _push(const std::map<Key, Value>& map)
    {
        lua_newtable(_state);

        for(const auto& [k, v] : map)
        {
            _push(k);
            _push(v);
            lua_settable(_state, -3);
        }

        return 1;
    }

    // when you call _push with a functor, this definition should be used
    // (thanks to SFINAE)
    // it will determine the function category using its () operator, then
    //   generate a callable user data and push it
    template <typename T, typename Op = decltype(&std::decay_t<T>::operator())>
    int _push(T&& fn, Op = nullptr)
    {
        // the () operator has type "R(T::*)(Args)", this typedef converts
        // it to "R(Args)"
        using FnType = typename RemoveMemberPtr<Op>::Type;

        // when the lua script calls the thing we will push on the stack, we
        // want "fn" to be executed
        // if we used lua's cfunctions system, we could not detect when the
        // function is no longer in use, which could cause problems
        // so we use userdata instead

        // we will create a userdata which contains a copy of a lambda
        // function [](lua_State*) -> int
        // but first we have to create it
        auto functionToPush([this, fn = std::forward<T>(fn)](lua_State* state) {
            // note that I'm using "" because of g++,
            // I don't know if it is required by standards or if it is a
            // bug
            assert(_state == state);

            // FnTupleWrapper<FnType> is a specialized template
            // structure which defines
            // "ParamsType", "ReturnType" and "call"
            // the first two correspond to the params list and return
            // type as tuples
            //   and "call" is a static function which will call a
            //   function
            //   of this type using parameters passed as a tuple
            using TupledFunction = FnTupleWrapper<FnType>;

            // checking if number of parameters is correct
            constexpr int paramsCount = TupledFunction::count;
            if(lua_gettop(state) < paramsCount)
            {
                // if not, using lua_error to return an error
                luaL_where(state, 1);
                lua_pushstring(state, "this function requires at least ");
                lua_pushnumber(state, paramsCount);
                lua_pushstring(state, " parameter(s)");
                lua_concat(state, 4);
                return lua_error(state); // lua_error throws an
                                         // exception when compiling as
                                         // C++
            }

            // pushing the result on the stack and returning number of
            // pushed elements
            return _push(
                // calling the function, result should be a tuple
                TupledFunction::call(fn,

                    // reading parameters from the stack
                    _read(-paramsCount,
                        static_cast<typename TupledFunction::ParamsType*>(
                            nullptr))));
        });

        // typedefing the type of data we will push
        using FunctionPushType = decltype(functionToPush);

        auto callbackCall = [](lua_State* lua) {
            // this function is called when the lua script tries to call our
            // custom data type
            // what we do is we simply call the function
            assert(lua_gettop(lua) >= 1);
            assert(lua_isuserdata(lua, 1));

            auto function = (FunctionPushType*)lua_touserdata(lua, 1);

            assert(function);
            return (*function)(lua);
        };

        auto callbackGarbage = [](lua_State* lua) {
            // this one is called when lua's garbage collector no longer
            // needs our custom data type
            // we call std::function<int (lua_State*)>'s destructor
            assert(lua_gettop(lua) == 1);

            auto function = (FunctionPushType*)lua_touserdata(lua, 1);

            assert(function);
            function->~FunctionPushType();
            return 0;
        };

        // creating the object
        // lua_newuserdata allocates memory in the internals of the lua
        // library and returns it so we can fill it
        //   and that's what we do with placement-new
        auto const functionLocation = (FunctionPushType*)lua_newuserdata(
            _state, sizeof(FunctionPushType));

        new(functionLocation) FunctionPushType(std::move(functionToPush));

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
        lua_pushlightuserdata(_state, const_cast<std::type_info*>(&typeid(T)));
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

        return 1;
    }

    // when pushing a unique_ptr, it is simply converted to a shared_ptr
    // this definition is necessary because unique_ptr has an implicit bool
    // conversion operator
    // with C++0x, this bool operator will certainly be declared explicit
    template <typename T>
    int _push(std::unique_ptr<T>&& mObj)
    {
        return _push(std::shared_ptr<T>(std::move(mObj)));
    }

    // when pushing a shared_ptr, we create a custom type
    // we store a copy of the shared_ptr inside lua's internals
    //   and add it a metatable: __gc for destruction and __index pointing
    //   to the corresponding
    //   table in the registry (see _registerFunction)
    template <typename T>
    int _push(std::shared_ptr<T>&& mObj)
    {
        // this is a structure providing static C-like functions that we can
        // feed to lua
        struct Callback
        {
            // this function is called when lua's garbage collector no
            // longer needs our shared_ptr
            // we simply call its destructor
            static int garbage(lua_State* lua)
            {
                assert(lua_gettop(lua) == 1);

                std::shared_ptr<T>* ptr =
                    (std::shared_ptr<T>*)lua_touserdata(lua, 1);

                assert(ptr && *ptr);
                ptr->~shared_ptr();

                return 0;
            }
        };

        // creating the object
        // lua_newuserdata allocates memory in the internals of the lua
        // library and returns it so we can fill it
        //   and that's what we do with placement-new
        std::shared_ptr<T>* const pointerLocation =
            (std::shared_ptr<T>*)lua_newuserdata(
                _state, sizeof(std::shared_ptr<T>));

        try
        {
            new(pointerLocation) std::shared_ptr<T>(std::move(mObj));

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
                lua_pushcfunction(_state, &Callback::garbage);
                lua_settable(_state, -3);

                // settings typeid of shared_ptr this time
                lua_pushstring(_state, "_typeid");
                lua_pushlightuserdata(_state,
                    const_cast<std::type_info*>(&typeid(std::shared_ptr<T>)));
                lua_settable(_state, -3);

                // as __index we set the table located in registry at type
                // name
                // see comments at _registerFunction
                lua_pushstring(_state, "__index");
                lua_pushlightuserdata(
                    _state, const_cast<std::type_info*>(&typeid(T)));
                lua_gettable(_state, LUA_REGISTRYINDEX);

                if(!lua_istable(_state, -1))
                {
                    assert(lua_isnil(_state, -1));
                    lua_pop(_state, 1);
                    lua_newtable(_state);
                    lua_pushlightuserdata(
                        _state, const_cast<std::type_info*>(&typeid(T)));
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
            catch(...)
            {
                lua_pop(_state, 1);
                throw;
            }
        }
        catch(...)
        {
            lua_pop(_state, 1);
            throw;
        }

        return 1;
    }

    // pushing tuples is also possible, though a bit complicated
    template <typename... Args>
    [[gnu::always_inline]] inline int _push(const std::tuple<Args...>& t)
    {
        return [ this, &t ]<int... Is>(std::integer_sequence<int, Is...>)
        {
            return (_push(std::get<Is>(t)) + ... + 0);
        }
        (std::make_integer_sequence<int, sizeof...(Args)>{});
    }

    /**************************************************/
    /*                READ FUNCTIONS                  */
    /**************************************************/
    // to use the _read function, pass as second parameter a null pointer
    // whose base type is the wanted return type
    // eg. if you want an int, pass "static_cast<int*>(nullptr)" as second
    // parameter

    // reading void
    [[gnu::always_inline]] inline void _read(int, void const* = nullptr) const
    {
    }

    // first the integer types
    template <typename T>
    [[gnu::always_inline]] inline std::enable_if_t<
        std::numeric_limits<T>::is_integer, T>
    _read(const int index, T const* = nullptr) const
    {
        if(lua_isuserdata(_state, index))
        {
            throw WrongTypeException{};
        }

        return T(lua_tointeger(_state, index));
    }

    // then the floating types
    template <typename T>
    [[gnu::always_inline]] inline std::enable_if_t<
        std::numeric_limits<T>::is_specialized &&
            !std::numeric_limits<T>::is_integer,
        T>
    _read(const int index, T const* = nullptr) const
    {
        if(lua_isuserdata(_state, index))
        {
            throw WrongTypeException{};
        }

        return T(lua_tonumber(_state, index));
    }

    // boolean
    [[gnu::always_inline]] inline bool _read(
        const int index, bool const* = nullptr) const
    {
        if(lua_isuserdata(_state, index))
        {
            throw WrongTypeException{};
        }

        return lua_toboolean(_state, index) != 0; // "!= 0" removes a
                                                  // warning because
                                                  // lua_toboolean returns
                                                  // an int
    }

    // string
    // lua_tostring returns a temporary pointer, but that's not a problem
    // since we copy
    //   the data in a std::string
    [[gnu::always_inline]] inline std::string _read(
        const int index, std::string const* = nullptr) const
    {
        if(lua_isuserdata(_state, index))
        {
            throw WrongTypeException{};
        }

        return lua_tostring(_state, index);
    }

    // maps
    template <typename Key, typename Value>
    std::map<Key, Value> _read(
        const int index, std::map<Key, Value> const* = nullptr) const
    {
        if(!lua_istable(_state, index))
        {
            throw WrongTypeException{};
        }


        std::map<Key, Value> retValue;

        // we traverse the table at the top of the stack
        lua_pushnil(_state); // first key
        while(lua_next(_state, index - 1) != 0)
        {
            // now a key and its value are pushed on the stack
            retValue.emplace(_read(-2, static_cast<Key*>(nullptr)),
                _read(-2, static_cast<Value*>(nullptr)));
            lua_pop(_state, 1); // we remove the value but keep the key for
                                // the next iteration
        }

        return retValue;
    }

    // reading array
    Table _read(int index, Table const* = nullptr) const
    {
        if(!lua_istable(_state, index))
        {
            throw WrongTypeException{};
        }

        throw std::logic_error{"Not implemented"};

        /*Table table;

// we traverse the table at the top of the stack
lua_pushnil(_state);            // first key
while(lua_next(_state, -2) != 0) {
        // now a key and its value are pushed on the stack
        auto keyType = lua_type(_state, -2);
        auto valueType = lua_type(_state, -1);

        switch (keyType) {
                case LUA_TNUMBER:                       break;
                case LUA_TBOOLEAN:                      break;
                case LUA_TSTRING:                       break;
                case LUA_TTABLE:                        break;
                case LUA_TFUNCTION:                     break;
                case LUA_TUSERDATA:                     break;
                case LUA_TLIGHTUSERDATA:        break;
                default:                throw(WrongTypeException());
        }

        lua_pop(_state, 1);             // we remove the value but keep the
key for the next iteration
}

return table;*/
    }

    // reading a shared_ptr
    // we check that type is correct by reading the metatable
    template <typename T>
    std::shared_ptr<T> _read(
        int mIdx, std::shared_ptr<T> const* = nullptr) const
    {
        if(!lua_isuserdata(_state, mIdx) || !lua_getmetatable(_state, mIdx))
        {
            throw WrongTypeException{};
        }

        // now we have our metatable on the top of the stack
        // retrieving its _typeid member
        lua_pushstring(_state, "_typeid");
        lua_gettable(_state, -2);

        // if wrong typeid, we throw
        if(lua_touserdata(_state, -1) !=
            const_cast<std::type_info*>(&typeid(std::shared_ptr<T>)))
        {
            lua_pop(_state, 2);
            throw WrongTypeException{};
        }

        lua_pop(_state, 2);

        // now we know that the type is correct, we retrieve the pointer
        const auto ptr =
            static_cast<std::shared_ptr<T>*>(lua_touserdata(_state, mIdx));

        assert(ptr && *ptr);
        return *ptr; // returning a copy of the shared_ptr
    }

    template <typename... Ts>
    [[gnu::always_inline]] inline auto _read(
        const int index, std::tuple<Ts...> const* = nullptr)
    {
        return [ this, index ]<int... Is>(std::integer_sequence<int, Is...>)
        {
            return std::make_tuple(
                _read(index + Is, static_cast<std::decay_t<Ts>*>(nullptr))...);
        }
        (std::make_integer_sequence<int, sizeof...(Ts)>{});
    }

    [[gnu::always_inline]] inline constexpr std::tuple<> _read(
        int, std::tuple<> const* = nullptr) const noexcept
    {
        return {};
    }
};

template <typename T>
struct IsFunctor
{
    using one = char;
    using two = long;

    template <typename C>
    static one test(decltype(&C::operator()));

    template <typename C>
    static two test(...);

    enum
    {
        value = sizeof(test<T>(0)) == sizeof(char)
    };
};

template <typename T>
struct LuaContext::ToPushableType<T&>
{
    using type = typename ToPushableType<T>::type;
};

template <typename T>
struct LuaContext::ToPushableType<const T&>
{
    using type = typename ToPushableType<T>::type;
};

template <typename T>
struct LuaContext::ToPushableType<T, std::enable_if_t<std::is_integral_v<T>>>
{
    using type = lua_Integer;
};

template <typename T>
struct LuaContext::ToPushableType<T,
    std::enable_if_t<std::is_floating_point_v<T>>>
{
    using type = lua_Number;
};

template <>
struct LuaContext::ToPushableType<bool>
{
    using type = bool;
};

template <>
struct LuaContext::ToPushableType<const char*>
{
    using type = std::string;
};

template <int N>
struct LuaContext::ToPushableType<const char[N]>
{
    using type = std::string;
};

template <int N>
struct LuaContext::ToPushableType<char[N]>
{
    using type = std::string;
};

template <>
struct LuaContext::ToPushableType<std::string>
{
    using type = std::string;
};

template <typename T>
struct LuaContext::ToPushableType<std::unique_ptr<T>>
{
    using type = std::shared_ptr<T>;
};

template <typename T>
struct LuaContext::ToPushableType<std::shared_ptr<T>>
{
    using type = std::shared_ptr<T>;
};

template <>
struct LuaContext::ToPushableType<LuaContext::Table>
{
    using type = LuaContext::Table;
};

template <typename T>
struct LuaContext::ToPushableType<T, std::enable_if_t<IsFunctor<T>::value>>
{
    using type = T;
};

} // namespace Lua
