//
//  Lua.hpp
//
//  Created by Roald Christesen on 03.10.2017
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 22.08.2025
//

#ifndef GrainLua_hpp
#define GrainLua_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "String/String.hpp"
#include "Color/RGB.hpp"
#include "CSS/CSSColor.hpp"

#include <filesystem>

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}


namespace Grain {

    class Lua;

    typedef int32_t (Lua::*LuaFunc)(lua_State* l);

    typedef void (*LuaInitAction)(Lua* lua);
    typedef int (*LuaCFunc)(lua_State* l);


    class LuaLib : Object {
    public:
        LuaLib() {}
        virtual ~LuaLib() = default;

        virtual ErrorCode init(Lua* lua) noexcept {
            return ErrorCode::None;
        }
    };


    class Lua : Object {
    public:
        enum {
            kErrNoContext = 0,
            kErrRunException,
            kErrMissingArgs,
            kErrUnknownCommand,
            kErrUidOutOfRange,
            kErrResourceNotFound,
            kErrLuaArgumentError
        };

    protected:
        lua_State* m_lua_vm = nullptr;
        String m_last_err_message;  ///< Last error message.
        bool m_is_initialized = false;

    public:
        Lua(LuaInitAction init_action = nullptr) noexcept;
        ~Lua() noexcept;

        const char* className() const noexcept override { return "Lua"; }

        lua_State* luaState() noexcept { return m_lua_vm; }
        const char* lastErrMessage() noexcept { return m_last_err_message.utf8(); }

        ErrorCode _init(LuaInitAction init_action) noexcept;

        ErrorCode addLib(LuaLib* lib) noexcept;

        void setGlobalString(const char* name, const char* value) {
            if (value == nullptr) {
                lua_pushnil(m_lua_vm);
            }
            else {
                lua_pushstring(m_lua_vm, value);
            }
            lua_setglobal(m_lua_vm, name);
        }

        void setGlobalInteger(const char* name, int64_t value) {
            lua_pushinteger(m_lua_vm, value);
            lua_setglobal(m_lua_vm, name);
        }

        void setGlobalNumber(const char* name, double value) {
            lua_pushnumber(m_lua_vm, value);
            lua_setglobal(m_lua_vm, name);
        }

        void setGlobalPointer(const char* name, void* ptr) {
            lua_pushlightuserdata(m_lua_vm, ptr);
            lua_setglobal(m_lua_vm, name);
        }

        /**
         *  @brief Retrieve the C pointer stored in the Lua global variable.
         */
        void* getGlobalPointer(const char* name) {
            lua_getglobal(m_lua_vm, name);
            void* ptr = lua_touserdata(m_lua_vm, -1);
            lua_pop(m_lua_vm, 1);
            return ptr;
        }

        static void* getGlobalPointer(lua_State* l, const char* name) {
            lua_getglobal(l, name);
            void* ptr = lua_touserdata(l, -1);
            lua_pop(l, 1);
            return ptr;
        }

        void addGlobalTable(const char* table_name) {
            lua_newtable(m_lua_vm);
            lua_setglobal(m_lua_vm, table_name);
        }

        void openTable(const char* table_name) {
            // Retrieve the table
            lua_getglobal(m_lua_vm, table_name);
        }

        void closeTable() {
            // Pop the table from the stack
            lua_pop(m_lua_vm, 1);
        }

        void setTableInteger(const char* key, const int64_t value) {
            // Push the string key
            lua_pushstring(m_lua_vm, key);
            // Push the table value
            lua_pushinteger(m_lua_vm, value);
        }

        void setTableDouble(const char* key, const double value) {
            // Push the string key
            lua_pushstring(m_lua_vm, key);
            // Push the table value
            lua_pushnumber(m_lua_vm, value);
        }

        void setTableString(const char* key, const char* value) {
            // Push the string key
            lua_pushstring(m_lua_vm, key);
            // Push the table value
            lua_pushstring(m_lua_vm, value);
        }

        /**
         *  @brief Call lua function without arguments.
         *
         *  Lua function should return an integer.
         */
        int64_t callFunction(const char* function_name) {
            int64_t result = 0;
            // Load the Lua function onto the stack.
            lua_getglobal(m_lua_vm, function_name);
            if (!lua_isfunction(m_lua_vm, -1)) {
                std::cerr << "Error: '" << function_name << "' is not a function!" << std::endl; // TODO: Error message!
                lua_pop(m_lua_vm, 1); // Remove non-function from the stack.
                // TODO: Handle error!
                return 0;
            }

            // Call the Lua function with 0 arguments, expect 1 result.
            if (lua_pcall(m_lua_vm, 0, 1, 0) != LUA_OK) {
                // If there's an error during the function call, print it,
                std::cerr << "Error calling Lua function: " << lua_tostring(m_lua_vm, -1) << std::endl; // TODO: Error message!
                // Remove error message from the stack.
                lua_pop(m_lua_vm, 1);
                // TODO: Handle error!
                return 0;
            }

            // Get the result from the stack.
            if (lua_isboolean(m_lua_vm, -1)) {
                result = lua_toboolean(m_lua_vm, -1);
            }
            else if (lua_isinteger(m_lua_vm, -1)) {
                result = lua_tointeger(m_lua_vm, -1);
            }

            // Pop the result from the stack.
            lua_pop(m_lua_vm, 1);

            return result;
        }


        // * * * * *

        void removeGlobalsByName(const char* name, fourcc_t mode = 'full') {
            auto len = strlen(name);
            // Push the global table (_G) onto the stack.
            lua_pushglobaltable(m_lua_vm);
            // Start iterating over the global table.
            // First key for lua_next (initial key is nil).
            lua_pushnil(m_lua_vm);
            while (lua_next(m_lua_vm, -2) != 0) {
                // lua_next pushes the key and value onto the stack, so key is at -2 and value is at -1.
                // Check if the key is a string.
                if (lua_type(m_lua_vm, -2) == LUA_TSTRING) {
                    const char* key = lua_tostring(m_lua_vm, -2);
                    bool remove_flag = false;
                    switch (mode) {
                        case 'full':
                            remove_flag = strcmp(key, name ) == 0;
                            break;
                        case 'beg_':
                            remove_flag = strncmp(key, name, len) == 0;
                            break;
                        default:
                            break;
                    }
                    if (remove_flag == true) {
                        // Set _G[key] to nil (remove the key).
                        lua_pushnil(m_lua_vm);
                        lua_setglobal(m_lua_vm, key);

                        // lua_setfield(m_lua_vm, -4, key);
                    }
                }
                // Pop the value, keep the key for the next iteration.
                lua_pop(m_lua_vm, 1);
            }
            // Pop the global table off the stack.
            lua_pop(m_lua_vm, 1);
        }

        static bool rgbFromStack(lua_State* l, int32_t arg_n, int32_t arg_offs, RGB& out_rgb) {
            int32_t n = arg_n - arg_offs + 1;
            if (n == 1) {
                if (lua_type(l, arg_offs) == LUA_TSTRING) {
                    auto err = CSSColor::parseColorToRGB(luaL_checkstring(l, arg_offs), out_rgb);
                    return err == ErrorCode::None;
                }
            }
            else if (n == 3) {
                out_rgb.m_data[0] = luaL_checknumber(l, arg_offs);
                out_rgb.m_data[1] = luaL_checknumber(l, arg_offs + 1);
                out_rgb.m_data[2] = luaL_checknumber(l, arg_offs + 2);
                return true;
            }
            else if (n == 4) {
                if (lua_type(l, arg_offs) == LUA_TSTRING) {
                    out_rgb.setSystemAndValues(
                            luaL_checkstring(l, arg_offs),
                            luaL_checknumber(l, arg_offs + 1),
                            luaL_checknumber(l, arg_offs + 2),
                            luaL_checknumber(l, arg_offs + 3));
                }
                return true;
            }
            else if (n == 5) {
                if (lua_type(l, arg_offs) == LUA_TSTRING) {
                    out_rgb.setSystemAndValues(
                            luaL_checkstring(l, arg_offs),
                            luaL_checknumber(l, arg_offs + 1),
                            luaL_checknumber(l, arg_offs + 2),
                            luaL_checknumber(l, arg_offs + 3),
                            luaL_checknumber(l, arg_offs + 4));
                }
                return true;
            }

            return false;
        }

        static bool integerFromStack(lua_State* l, int32_t arg_offs, int64_t& out_value) {
            if (lua_isinteger(l, arg_offs)) {
                out_value = lua_tointeger(l, arg_offs);
                return true;
            }
            return false;
        }

        static bool doubleFromStack(lua_State* l, int32_t arg_offs, double& out_value) {
            if (lua_isnumber(l, arg_offs)) {
                out_value = lua_tonumber(l, arg_offs);
                return true;
            }
            return false;
        }

        static const char* stringFromStack(lua_State* l, int32_t arg_offs) {
            static const char* empty_str = "";
            if (lua_isstring(l, arg_offs)) {
                return lua_tostring(l, arg_offs);
            }
            else {
                return empty_str;
            }
        }

        ErrorCode run(const String& dir_path, const String& file_name) noexcept;
        ErrorCode run(const String& file_path) noexcept;
        ErrorCode runCode(const char* code) noexcept;
        void close() noexcept;

        void clearMessages() noexcept {}    // TODO: Implement!
        void printMessages() noexcept {}    // TODO: Implement!

        /**
         *  @brief Registers a C function in a named Lua table.
         *
         *  This function looks up a global Lua table by name, and if it exists,
         *  adds a new C function to it with the given function name.
         *
         *  @param table_name  The name of the global Lua table to modify.
         *  @param func_name   The name of the function as it will appear in Lua (i.e., the table key).
         *  @param func        The C function to register (of type lua_CFunction).
         *
         *  @details
         *  This is useful for organizing your Lua API into namespaces (tables).
         *  For example, if you have a Lua table called `grain`, this function
         *  can be used to add C functions like `grain.print`, `grain.random`, etc.
         *
         *  If the table does not exist or is not a table, an error message is printed
         *  and no function is registered.
         *
         *  @note
         *  This function assumes the table is a global Lua variable. It does not
         *  create the table automatically.
         *
         *  @see lua_pushcfunction, lua_setfield, lua_getglobal
         */
        void registerLuaFunction(const char* table_name, const char* func_name, lua_CFunction func) {
            lua_getglobal(m_lua_vm, table_name);  // Push table
            if (!lua_istable(m_lua_vm, -1)) {
                lua_pop(m_lua_vm, 1);
                fprintf(stderr, "Lua error: '%s' is not a table\n", table_name);
                return;
            }
            lua_pushcfunction(m_lua_vm, func);
            lua_setfield(m_lua_vm, -2, func_name);
            lua_pop(m_lua_vm, 1);  // Pop table
        }

        /**
         *  @brief This function is a convenience function provided by the Lua API.
         *
         *  It registers a C function directly in the global environment with the
         *  name specified by name. It simplifies the process of making a C function
         *  available to Lua scripts by automatically pushing the function onto the
         *  stack and setting it as a global variable.
         *
         *  @note
         *  When to Use: Use lua_register when you want to add a C function to the
         *  global environment without dealing with the stack explicitly. It’s a
         *  straightforward way to expose functions to Lua scripts that are intended
         *  to be used globally.
         */
        void registerFunc(const char* name, LuaCFunc c_func) {
            lua_register(m_lua_vm, name, c_func);
        }

        void pushCFunction(lua_CFunction c_func, const char* func_name) {
            lua_pushcfunction(m_lua_vm, c_func);
            lua_setfield(m_lua_vm, -2, func_name);
        }


        //

        static int _funcLuaPrintRedirect(lua_State* l);
        static int _funcCall(lua_State* l, LuaFunc func);

        static int _funcScriptPath(lua_State* l);
        static int _funcScriptDirectory(lua_State* l);

        static int _funcRandom(lua_State* l);
        static int _funcRandomInt(lua_State* l);
        static int _funcRandomDist(lua_State* l);
        static int _funcRandomChoice(lua_State* l);
        static int _funcShuffle(lua_State* l);
        static int _funcChance(lua_State* l);
        static int _funcPerNoergaardInfinitNumber(lua_State* l);
        static int _funcCollatzSequenceNumber(lua_State* l);


        // Lua helper functions

        static double _argClampedDouble(lua_State* l, int32_t index) {
            double v = luaL_checknumber(l, index);
            return v < 0.0f ? 0.0f : (v > 1.0f) ? 1.0f : v;
        }

        static bool _lua_toBool(lua_State* l, int32_t index) {
            return static_cast<bool>(lua_toboolean(l, index));
        }

        static int32_t _lua_toInt32(lua_State* l, int32_t index) {
            return static_cast<int32_t>(lua_tointeger(l, index));
        }

        static float _lua_toFloat(lua_State* l, int32_t index) {
            return static_cast<float>(lua_tonumber(l, index));
        }

        static double _lua_toDouble(lua_State* l, int32_t index) {
            return static_cast<double>(lua_tonumber(l, index));
        }

        static const char* _lua_toStr(lua_State* l, int32_t index) {
            static const char* emptyString = "";
            return lua_isstring(l, index) ? lua_tostring(l, index) : emptyString;
        }

        static int _lua_pushBool(lua_State* l, bool value) {
            lua_pushboolean(l, value);    // Push the return value(s)
            return 1;                     // Return the count of return values, 1 in this case
        }

        static int _lua_pushInt32(lua_State* l, int32_t value) {
            lua_pushinteger(l, value);    // Push the return value(s)
            return 1;                     // Return the count of return values, 1 in this case
        }

        static int _lua_pushInt64(lua_State* l, int64_t value) {
            lua_pushinteger(l, value);    // Push the return value(s)
            return 1;                     // Return the count of return values, 1 in this case
        }

        static int _lua_pushDouble(lua_State* l, double value) {
            lua_pushnumber(l, value);     // Push the return value(s)
            return 1;                     // Return the count of return values, 1 in this case
        }

        static void _lua_setGlobal(lua_State* l, const char* name, int32_t value) {
            lua_pushinteger(l, value);
            lua_setglobal(l, name);
        }

        static int32_t _lua_getGlobal(lua_State* l, const char* name) {
            lua_getglobal(l, name);
            int32_t result = lua_tonumber(l, -1);
            lua_pop(l, 1);
            return result;
        }

        static int32_t _lua_setupModulePath(lua_State* l);
    };


} // End of namespace Grain

#endif // GrainLua_hpp
