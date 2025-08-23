//
//  Lua.cpp
//
//  Created by Roald Christesen on 03.10.2017
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.08.2025
//

#include "Scripting/Lua.hpp"
#include "Math/NumberSeries.hpp"
// #include "CSV/CSVData.hpp"
#include "File/File.hpp"
#include "Image/Image.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    Lua::Lua(LuaInitAction init_action) noexcept {
        _init(init_action);
    }


    Lua::~Lua() noexcept {

        close();
    }


/**
 *  @brief Initializes the Lua interpreter and registers custom functions.
 *
 *  This method creates and configures a new Lua virtual machine (if it hasn't
 *  already been created), loads the standard Lua libraries, overrides the
 *  default `print()` function with a C++ implementation, and registers several
 *  custom C++ functions under the Lua namespace `"grain"`.
 *
 *  If a user-provided initialization callback (`init_action`) is supplied, it
 *  will be invoked after the Lua environment is fully initialized.
 *
 *  @param init_action A callback function of type `LuaInitAction` to perform
 *                     additional initialization logic. It may be `nullptr`.
 *
 *  @return `ErrorCode::None` if the initialization completed successfully,
 *          or the corresponding `ErrorCode` if an error occurred during
 *          initialization.
 *
 *  @note This function is marked `noexcept` and will catch any `ErrorCode`
 *  exceptions internally.
 */
    ErrorCode Lua::_init(LuaInitAction init_action) noexcept {

        auto result = ErrorCode::None;

        try {
            clearMessages();

            if (m_lua_vm == nullptr) {

                // Creates a new Lua interpreter
                m_lua_vm = luaL_newstate();

                // Load standard Lua libraries
                luaL_openlibs(m_lua_vm);

                {
                    // Override Lua print function
                    lua_pushcfunction(m_lua_vm, _funcLuaPrintRedirect);
                    lua_setglobal(m_lua_vm, "print");
                }

                {
                    // Create and register namespace `grain` and methods
                    addGlobalTable("grain");
                    registerLuaFunction("grain", "setupModulePath", _lua_setupModulePath);
                    registerLuaFunction("grain", "scriptPath", _funcScriptPath);
                    registerLuaFunction("grain", "scriptDir", _funcScriptDirectory);

                    registerLuaFunction("grain", "random", _funcRandom);
                    registerLuaFunction("grain", "randomInt", _funcRandomInt);
                    registerLuaFunction("grain", "randomDist", _funcRandomDist);
                    registerLuaFunction("grain", "randomChoice", _funcRandomChoice);
                    registerLuaFunction("grain", "shuffle", _funcShuffle);
                    registerLuaFunction("grain", "chance", _funcChance);
                    registerLuaFunction("grain", "perNoergaardNumber", _funcPerNoergaardInfinitNumber);
                    registerLuaFunction("grain", "collatzSequenceNumber", _funcCollatzSequenceNumber);
                }

                // Call external initialization hook if it exists
                if (init_action != nullptr) {
                    init_action(this);
                }
            }

            m_is_initialized = true;
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


    ErrorCode Lua::addLib(LuaLib* lib) noexcept {

        return lib->init(this);
    }


/**
 *  @brief Executes a Lua script from a specified directory and file name.
 *
 *  Constructs the full file path by combining the given directory path and file
 *  name, then delegates execution to the overloaded `run(const String&)` method.
 *
 *  @param dir_path The path to the directory containing the Lua script.
 *  @param file_name The name of the Lua script file to run.
 *
 *  @return `ErrorCode::None` if the script was executed successfully,
 *          or the corresponding `ErrorCode` if execution failed.
 *
 *  @note This function is marked `noexcept` and catches `ErrorCode` exceptions
 *        internally.
 */
    ErrorCode Lua::run(const String& dir_path, const String& file_name) noexcept {

        auto result = ErrorCode::None;

        try {
            String file_path = dir_path + "/" + file_name;
            result = run(file_path);
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


/**
 *  @brief Executes a Lua script located at the specified file path.
 *
 *  This function checks whether the file exists and, if so, executes it using
 *  the Lua interpreter (`luaL_dofile`). If the Lua script fails to run,
 *  the error message is captured from the Lua stack and stored in
 *  `m_last_err_message`.
 *
 *  @param file_path The full path to the Lua script file to be executed.
 *
 *  @return `ErrorCode::None` if the script was found and executed successfully,
 *          or an appropriate `ErrorCode` such as `ErrorCode::FileNotFound` or
 *          `ErrorCode::LuaCodeError` if execution failed.
 *
 *  @note If a Lua runtime error occurs, the error message is stored in
 *        `m_last_err_message` for later inspection.
 */
    ErrorCode Lua::run(const String& file_path) noexcept {

        auto result = ErrorCode::None;

        try {

            if (File::fileExists(file_path) == false) {
                throw ErrorCode::FileNotFound;
            }

            // Set the path to `file_path`

            // [Step 1] Extract directory from `file_path`
            auto script_dir_path = file_path.fileDirPath();
            // std::cout << "script_dir_path: " << script_dir_path << std::endl;

            // [Step 2] Add to package.path
            lua_getglobal(m_lua_vm, "package");
            lua_getfield(m_lua_vm, -1, "path");  // stack: package, path
            String current_path = lua_tostring(m_lua_vm, -1);
            // std::cout << "current_path: " << current_path << std::endl;

            String new_path = current_path + ";" + script_dir_path + "/?.lua";
            // std::cout << "new_path: " << new_path << std::endl;

            lua_pop(m_lua_vm, 1); // pop old path
            lua_pushstring(m_lua_vm, new_path.utf8());
            lua_setfield(m_lua_vm, -2, "path");  // package.path = new_path
            lua_pop(m_lua_vm, 1); // pop package

            ///

            if (int err = luaL_dofile(m_lua_vm, file_path.utf8()) != LUA_OK) {
                const char* error_message = lua_tostring(m_lua_vm, -1);
                if (error_message) {
                    m_last_err_message = error_message;
                    result = ErrorCode::LuaCodeError;
                }

                lua_pop(m_lua_vm, 1);  // Pop the error message from the stack
            }
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


/**
 *  @brief Executes a Lua code string directly in the Lua interpreter.
 *
 *  @param code A null-terminated C-string containing valid Lua code to execute.
 *
 *  @return `ErrorCode::None` if the code was executed without errors,
 *          or `ErrorCode::LuaCodeError` if Lua reported a runtime error.
 *          Other `ErrorCode` values may be returned if thrown and caught.
 *
 *  @note If an error message is present, it is stored in `m_last_err_message`
 *        for later retrieval.
 */
    ErrorCode Lua::runCode(const char* code) noexcept {

        auto result = ErrorCode::None;

        try {

            if (int err = luaL_dostring(m_lua_vm, code) != LUA_OK) {
                const char* error_message = lua_tostring(m_lua_vm, -1);
                if (error_message) {
                    m_last_err_message = error_message;
                    result = ErrorCode::LuaCodeError;
                }

                lua_pop(m_lua_vm, 1);  // Pop the error message from the stack
            }
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


/**
 *  @brief Closes the Lua interpreter and releases associated resources.
 *
 *  If the Lua virtual machine (`m_lua_vm`) is currently active, this function
 *  closes it using `lua_close()` and sets the internal pointer to `nullptr`.
 *  It is safe to call this function multiple times; it will have no effect if
 *  the Lua VM is already closed.
 */
    void Lua::close() noexcept {

        if (m_lua_vm != nullptr) {
            lua_close(m_lua_vm);
            m_lua_vm = nullptr;
        }
    }


    int Lua::_funcLuaPrintRedirect(lua_State* l) {

        int arg_n = lua_gettop(l);
        std::string output;

        for (int i = 1; i <= arg_n; ++i) {
            const char* str = lua_tostring(l, i);
            if (str) {
                output += str;
            }
            else {
                output += luaL_tolstring(l, i, nullptr);
                lua_pop(l, 1);
            }
            if (i < arg_n) {
                output += " ";
            }
        }

        std::cout << "[Lua] " << output << std::endl;
        return 0;
    }


    int Lua::_funcScriptPath(lua_State* l) {

        lua_Debug ar;
        if (lua_getstack(l, 1, &ar) == 0 || !lua_getinfo(l, "S", &ar)) {
            return luaL_error(l, "Unable to get script path");
        }

        const char* source = ar.source;
        if (source && source[0] == '@') {
            lua_pushstring(l, source + 1);  // Remove '@'
        } else {
            lua_pushstring(l, source);
        }

        return 1;  // Return the script path string
    }


    int Lua::_funcScriptDirectory(lua_State* l) {

        lua_Debug ar;
        if (lua_getstack(l, 1, &ar) == 0 || !lua_getinfo(l, "S", &ar)) {
            return luaL_error(l, "Unable to get script directory");
        }

        const char* source = ar.source;
        std::string fullPath = (source && source[0] == '@') ? source + 1 : source;

        try {
            std::filesystem::path path(fullPath);
            std::filesystem::path dir = path.parent_path();
            lua_pushstring(l, dir.string().c_str());
            return 1;
        }
        catch (...) {
            return luaL_error(l, "Failed to extract directory path");
        }
    }


    int Lua::_funcRandom(lua_State* l) {

        double result = 0.0;
        double min = 0.0, max = 1.0;

        int arg_n = lua_gettop(l);
        switch (arg_n) {
            case 0:
                result = Random::next();
                break;
            case 1:
                max = _lua_toDouble(l, 1);
                result = Random::next(0.0, max);
                break;
            case 2:
                min = _lua_toDouble(l, 1);
                max = _lua_toDouble(l, 2);
                if (min > max) {
                    return luaL_error(l, "Invalid range: min must be <= max");
                }
                result = Random::next(min, max);
                break;
            default:
                return luaL_error(l, "Expected 0, 1 or 2 arguments");
        }

        return _lua_pushDouble(l, result);
    }


    int Lua::_funcRandomInt(lua_State* l) {

        int32_t result = 0;
        int32_t min = 0, max = INT32_MAX;

        int32_t arg_n = lua_gettop(l);
        switch (arg_n) {
            case 0:
                result = Random::nextInt();  // Default random int
                break;

            case 1:
                max = _lua_toInt32(l, 1);
                result = Random::nextInt(max);  // Random integer in [0, max]
                break;

            case 2:
                min = _lua_toInt32(l, 1);
                max = _lua_toInt32(l, 2);
                if (min > max) {
                    return luaL_error(l, "Invalid range: min must be <= max");
                }
                result = Random::nextInt(min, max);  // Random integer in [min, max]
                break;

            default:
                return luaL_error(l, "Expected 0, 1, or 2 arguments");
        }

        return _lua_pushInt32(l, result);  // Push result to Lua
    }


    int Lua::_funcRandomDist(lua_State* l) {

        double center = 0.0, width = 1.0;

        int32_t arg_n = lua_gettop(l);

        if (arg_n == 1) {
            width = _lua_toDouble(l, 1);
        }
        else if (arg_n == 2) {
            center = _lua_toDouble(l, 1);
            width = _lua_toDouble(l, 2);
        }
        else {
            return luaL_error(l, "Expected 1 or 2 arguments: width or (center, width)");
        }

        if (width < 0.0) {
            return luaL_error(l, "Width must be non-negative");
        }

        double value = center + Random::nextBipolar(width);
        return _lua_pushDouble(l, value);
    }


    int Lua::_funcRandomChoice(lua_State* l) {

        if (lua_istable(l, 1)) {
            int32_t len = static_cast<int32_t>(lua_rawlen(l, 1));
            if (len == 0) {
                return luaL_error(l, "Array is empty");
            }

            int32_t index = Random::nextInt(1, len);  // Assuming inclusive range
            lua_rawgeti(l, 1, index);  // Push the value at random index onto the stack
            return 1;  // Return it directly (do not convert to number)
        }
        else {
            return luaL_error(l, "Expected an array");
        }
    }


    int Lua::_funcShuffle(lua_State* l) {

        if (!lua_istable(l, 1)) {
            return luaL_error(l, "Expected an array");
        }

        int32_t len = static_cast<int32_t>(lua_rawlen(l, 1));
        if (len <= 1) {
            return 0;  // Nothing to shuffle
        }

        for (int32_t i = len; i > 1; --i) {
            int32_t j = Random::nextInt(1, i);

            // Swap table[i] and table[j]
            lua_rawgeti(l, 1, i);  // push table[i]
            lua_rawgeti(l, 1, j);  // push table[j]

            lua_rawseti(l, 1, i);  // table[i] = table[j]
            lua_rawseti(l, 1, j);  // table[j] = table[i]
        }

        return 0;  // In-place modification, no return value
    }


    int Lua::_funcChance(lua_State* l) {

        int32_t arg_n = lua_gettop(l);

        if (arg_n == 0) {
            // 50% chance
            return _lua_pushBool(l, Random::chance());
        }
        else if (arg_n == 1) {
            // chance(probability)
            double prob = _argClampedDouble(l, 1);
            return _lua_pushBool(l, Random::chance(prob));
        }
        else {
            return luaL_error(l, "Expected 0 or 1 argument: chance([probability])");
        }
    }


    int Lua::_funcPerNoergaardInfinitNumber(lua_State* l) {

        int32_t arg_n = lua_gettop(l);
        if (arg_n != 1) {
            return luaL_error(l, "Expected exactly 1 argument: v");
        }
        else {
            int32_t v = _lua_toInt32(l, 1);
            int32_t result = NumberSeries::perNoergaardInfinitNumber(v);
            return _lua_pushInt32(l, result);
        }
    }


    int Lua::_funcCollatzSequenceNumber(lua_State* l) {

        int32_t arg_n = lua_gettop(l);

        int32_t v = 0;
        int32_t max_depth = 1000000;

        if (arg_n < 1) {
            return luaL_error(l, "Expected exactly 1 or 2 arguments: v [, max_depth]");
        }

        if (arg_n >= 1) {
            v = _lua_toInt32(l, 1);
        }
        if (arg_n >= 2) {
            max_depth = _lua_toInt32(l, 1);
        }

        int32_t result = NumberSeries::collatzSequenceNumber(v, max_depth);
        return _lua_pushInt32(l, result);
    }


    int32_t Lua::_lua_setupModulePath(lua_State* l) {

        // 1. Get debug table
        lua_getglobal(l, "debug");  // stack: debug
        if (!lua_istable(l, -1)) {
            lua_pop(l, 1);
            return luaL_error(l, "debug library not found");
        }

        // 2. Get debug.getinfo function
        lua_getfield(l, -1, "getinfo");  // stack: debug, getinfo
        if (!lua_isfunction(l, -1)) {
            lua_pop(l, 2);
            return luaL_error(l, "debug.getinfo not found");
        }

        // 3. Push the stack level (1) and "S" as arguments to getinfo
        lua_pushinteger(l, 1);  // stack: debug, getinfo, 1
        lua_pushstring(l, "S");  // stack: debug, getinfo, 1, "S"

        // 4. Call debug.getinfo(1, "S")
        if (lua_pcall(l, 2, 1, 0) != LUA_OK) {  // stack: debug, info
            const char* err = lua_tostring(l, -1);
            lua_pop(l, 2);
            return luaL_error(l, err);
        }

        // 5. Get source field from returned info table
        lua_getfield(l, -1, "source");  // stack: debug, info, source
        const char* source = lua_tostring(l, -1);
        if (!source) {
            lua_pop(l, 3);
            return luaL_error(l, "source field not found");
        }

        // 6. source looks like "@path/to/script.lua", remove '@'
        std::string src(source);
        if (!src.empty() && src[0] == '@') {
            src.erase(0, 1);
        } else {
            lua_pop(l, 3);
            return luaL_error(l, "source does not start with '@'");
        }

        // 7. Extract directory from path (strip filename)
        size_t last_slash = src.find_last_of("/\\");
        if (last_slash == std::string::npos) {
            lua_pop(l, 3);
            return luaL_error(l, "Could not find directory separator in source path");
        }
        std::string dir = src.substr(0, last_slash + 1); // include trailing slash

        // 8. Get package.path
        lua_getglobal(l, "package");  // stack: debug, info, source, package
        lua_getfield(l, -1, "path");  // stack: debug, info, source, package, path
        const char* current_path = lua_tostring(l, -1);
        if (!current_path) {
            lua_pop(l, 5);
            return luaL_error(l, "package.path is nil");
        }

        // 9. Append new path
        std::string new_path = std::string(current_path) + ";" + dir + "?.lua";

        // 10. Set package.path = new_path
        lua_pop(l, 1);  // pop old path, stack: debug, info, source, package
        lua_pushstring(l, new_path.c_str());  // stack: debug, info, source, package, new_path
        lua_setfield(l, -2, "path");  // package.path = new_path; stack: debug, info, source, package

        // 11. Clean stack
        lua_pop(l, 3);  // pop package, info, source, debug (stack balanced)

        return 0;  // no return values to Lua
    }


}  // End of namespace Grain
