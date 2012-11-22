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

#include "LuaContext.h"

Lua::LuaContext::LuaContext(bool openDefaultLibs) {
	// we pass this allocator function to lua_newstate
	// we use our custom allocator instead of luaL_newstate, so we
	//   may trace the memory usage in the future
	struct Allocator {
		static void* allocator(void *ud, void *ptr, size_t osize, size_t nsize) {
			if (nsize == 0) {
				free(ptr);
				return nullptr;
			} else {
				return realloc(ptr, nsize);
			}
		}
	};
	
	// lua_newstate can return null if allocation failed
	_state = lua_newstate(&Allocator::allocator, nullptr);
	if (_state == nullptr)		throw(std::bad_alloc());

	// opening default library if required to do so
	if (openDefaultLibs)
		luaL_openlibs(_state);
}

void Lua::LuaContext::_load(std::istream& code) {
	// since the lua_load function requires a static function, we use this structure
	// the Reader structure is at the same time an object storing an istream and a buffer,
	//   and a static function provider
	struct Reader {
		Reader(std::istream& str) : stream(str) {}
		std::istream&		stream;
		char				buffer[512];

		// read function ; "data" must be an instance of Reader
		static const char* read(lua_State* l, void* data, size_t* size) {
			assert(size != nullptr);
			assert(data != nullptr);
			Reader& me = *((Reader*)data);
			if (me.stream.eof())	{ *size = 0; return nullptr; }

			me.stream.read(me.buffer, sizeof(me.buffer));
			*size = size_t(me.stream.gcount());		// gcount could return a value larger than a size_t, but its maximum is sizeof(me.buffer) so there's no problem
			return me.buffer;
		}
	};

	// we create an instance of Reader, and we call lua_load
	std::unique_ptr<Reader> reader(new Reader(code));
	auto loadReturnValue = lua_load(_state, &Reader::read, reader.get(), "chunk");

	// now we have to check return value
	if (loadReturnValue != 0) {
		// there was an error during loading, an error message was pushed on the stack
		const char* errorMsg = lua_tostring(_state, -1);
		lua_pop(_state, 1);
		if (loadReturnValue == LUA_ERRMEM)			throw(std::bad_alloc());
		else if (loadReturnValue == LUA_ERRSYNTAX)	throw(SyntaxErrorException(std::string(errorMsg)));

	}
}

void Lua::LuaContext::_getGlobal(const std::string& variableName) const {
	// first a little optimization: if variableName contains no dot, we can directly call lua_getglobal
	if (std::find(variableName.begin(), variableName.end(), '.') == variableName.end()) {
		lua_getglobal(_state, variableName.c_str());
		return;
	}

	// variableName is split by dots '.' in arrays and subarrays
	// the nextVar variable contains a pointer to the next part to proceed
	auto nextVar = variableName.begin();

	do {
		// since we are going to modify nextVar, we store its value here
		auto currentVar = nextVar;

		// first we extract the part between currentVar and the next dot we encounter
		nextVar = std::find(currentVar, variableName.end(), '.');
		std::string buffer(currentVar, nextVar);
		// since nextVar is pointing to a dot, we have to increase it first in order to find the next variable
		if (nextVar != variableName.end())
			++nextVar;

		// ask lua to find the part stored in buffer
		// if currentVar == begin, this is a global variable and push it on the stack
		//   otherwise we already have an array pushed on the stack by the previous loop
		if (currentVar == variableName.begin()) {
			lua_getglobal(_state, buffer.c_str());

		} else {
			// if variableName is "a.b" and "a" is not a table (eg. it's a number or a string), this happens
			// we don't have a specific exception for this, we consider this as a variable-doesn't-exist
			if (!lua_istable(_state, -1)) {
				lua_pop(_state, 1);
				throw(VariableDoesntExistException(variableName));
			}

			// replacing the current table in the stack by its member
			lua_pushstring(_state, buffer.c_str());
			lua_gettable(_state, -2);
			lua_remove(_state, -2);
		}

		// lua will accept anything as variable name, but if the variable doesn't exist
		//   it will simply push "nil" instead of a value
		// so if we have a nil on the stack, the variable didn't exist and we throw
		if (lua_isnil(_state, -1)) {
			lua_pop(_state, 1);
			throw(VariableDoesntExistException(variableName));
		}

		// updating currentVar
		currentVar = nextVar;
	} while (nextVar != variableName.end());
}

void Lua::LuaContext::_setGlobal(const std::string& variable) {
	try {
		assert(lua_gettop(_state) >= 1);		// making sure there's something on the stack (ie. the value to set)

		// two possibilities: either "variable" is a global variable, or a member of an array
		size_t lastDot = variable.find_last_of('.');
		if (lastDot == std::string::npos) {
			// this is the first case, we simply call setglobal (which cleans the stack)
			lua_setglobal(_state, variable.c_str());

		} else {
			const auto tableName = variable.substr(0, lastDot);

			// in the second case, we call _getGlobal on the table name
			_getGlobal(tableName);
			try {
				if (!lua_istable(_state, -1))
					throw(VariableDoesntExistException(variable));

				// now we have our value at -2 (was pushed before _setGlobal is called) and our table at -1
				lua_pushstring(_state, variable.substr(lastDot + 1).c_str());		// value at -3, table at -2, key at -1
				lua_pushvalue(_state, -3);											// value at -4, table at -3, key at -2, value at -1
				lua_settable(_state, -3);											// value at -2, table at -1
				lua_pop(_state, 2);													// stack empty \o/
			} catch(...) {
				lua_pop(_state, 2);
				throw;
			}
		}
	} catch(...) {
		lua_pop(_state, 1);
		throw;
	}
}

bool Lua::LuaContext::isVariableArray(const std::string& variableName) const {
	_getGlobal(variableName);
	bool answer = lua_istable(_state, -1);
	lua_pop(_state, 1);
	return answer;
}

void Lua::LuaContext::writeArrayIntoVariable(const std::string& variableName) {
	lua_newtable(_state);
	_setGlobal(variableName);
}

