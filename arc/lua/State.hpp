#pragma once

#include <luajit-2.0/lua.hpp>
#include <functional>

#include "arc/core.hpp"
#include "arc/string/String.hpp"


namespace arc { namespace lua {

    const int STACK_TOP = -1;
    const int LUA_OK = 0;

    struct State;
    struct Reference;
    struct each_value;

    enum class Type
    {
        NIL = LUA_TNIL,
        BOOL = LUA_TBOOLEAN,
        NUMBER = LUA_TNUMBER,
        STRING = LUA_TSTRING,
        TABLE = LUA_TTABLE,
        FUNCTION = LUA_TFUNCTION,
        INVALID = 10,
        ERROR = 11,
    };

    const char* str(Type t);

    struct Value
    {
        Value();
        ~Value();

        ARC_NO_COPY(Value);
    public:
        Value(Value&& other);
        Value& operator=(Value&& other);
    public:
        /* pops the top value from the stack and creates a value linked to it */
        Value(lua_State* state);
    public:
        bool valid();
        bool is_nil();
        bool is_error();
    public:
        Type type();
    public:
        bool get(int32 &out_value);
        bool get(String& out_value);
        bool get(bool& out_value);
    public:
        template<typename ...Args>
        Value call( Args&&... args);
    public:
        template<typename ...Selectors>
        Value select(Selectors&& ... selectors);
    public:
        void clear();
    public:
        lua_State* m_state;
        int32 m_ref;
        bool  m_error = false;
    private:
        friend struct arc::lua::State;
    };

    struct State
    {
        State();
        ~State();
    public:
        void open_standard_libs();
    public:
        bool execute_file(const char* file_path);
    public:
        template<typename ...Selectors>
        Value select(const char* global_name, Selectors&& ... tail);
    public:
        lua_State* m_state;
    };

    // each_value ////////////////////////////////////////////////////////////////////////////

    struct each_value
    {
        struct iterator
        {
        public:
            bool operator!=(const iterator& other);
            iterator operator++();
            Value& operator*();
        public:
            each_value& each;
        };

        public:
            each_value(Value& v);
            ~each_value();
        public:
            iterator begin();
            iterator end();
        private:
            Value& m_table;
            Value  m_value;
            int32  m_backup;
    };

    // Free functions ////////////////////////////////////////////////////////////////////////

    template<typename ...Selectors>
    Value select_unroll(lua_State* state, int32 type, const char* front, Selectors&& ... tail);

    template<typename ...Selectors>
    Value select_unroll(lua_State* state, int32 type, int32 front, Selectors&& ... tail);

    Value select_unroll(lua_State* state, int32 type);

    void push(lua_State* state, int32 value);
    void push(lua_State* state, StringView value);
    void push(lua_State* state, const String& value);
    void push(lua_State *state, Value& value);

    // Implementation ////////////////////////////////////////////////////////////////////////

    template<typename ...Selectors> inline
    Value select_unroll(lua_State* state, int32 type, const char* front, Selectors&& ... tail)
    {
        // trying to index a non table value
        if (type != LUA_TTABLE) return Value();

        // do the indexing
        lua_pushstring(state,front);
        lua_gettable(state,STACK_TOP-1);
        int32 element_type = lua_type(state,STACK_TOP);

        // key was not present
        if (element_type == LUA_TNIL) return Value();

        return select_unroll(state,element_type,std::forward<Selectors>(tail)...);
    }

    template<typename ...Selectors> inline
    Value select_unroll(lua_State* state, int32 type, int32 front, Selectors&& ... tail)
    {
        // trying to index a non table value
        if (type != LUA_TTABLE) return Value();

        // do the indexing
        lua_pushinteger(state,front);
        lua_gettable(state,STACK_TOP-1);
        int32 element_type = lua_type(state,STACK_TOP);

        // key was not present
        if (element_type == LUA_TNIL) return Value();

        return select_unroll(state,element_type,std::forward<Selectors>(tail)...);
    }

    inline
    Value select_unroll(lua_State* state, int32 type)
    {
        return Value(state);
    }

    template<typename ...Selectors> inline
    Value State::select(const char* global_name, Selectors&& ... selectors)
    {
        // remember stack state
        int32 top = lua_gettop(m_state);

        // index global table
        lua_getglobal(m_state,global_name);
        int32 type = lua_type(m_state,STACK_TOP);

        // didn't find global value
        if (type == LUA_TNIL)
        {
            lua_pop(m_state,1);
            return Value();
        }

        // recurse
        Value result = select_unroll(m_state,type,std::forward<Selectors>(selectors)...);

        // restore stack state and return
        lua_settop(m_state,top);
        return result;
    }

    template<typename ...Selectors> inline
    Value Value::select(Selectors&& ... selectors)
    {
        // safety check
        if (!valid()) return Value();

        // remember stack state
        int32 top = lua_gettop(m_state);

        // get value from registry & type
        lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_ref);
        int32 type = lua_type(m_state,STACK_TOP);

        // start recursion
        Value result = select_unroll(m_state,type,std::forward<Selectors>(selectors)...);

        // restore stack state and return
        lua_settop(m_state,top);
        return result;
    }

    struct pass { template<typename ...T> pass(T...) {} };

    template<typename ...Args> inline
    Value Value::call(Args&&... args)
    {
        // safety check
        if (!valid())
        {
            ARC_ASSERT(false,"Trying to call invalid lua::Value");
            return Value();
        }

        // get value from registry (and push onto stack)
        lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_ref);

        // check type
        auto type = lua_type(m_state,STACK_TOP);
        if (type != LUA_TFUNCTION)
        {
            lua_pop(m_state,1);

            // set error state
            lua_pushliteral(m_state,"Error: Trying to call non function type.");
            auto error = Value(m_state);
            error.m_error = true;
            return error;
        }

        // push args
        int32 n_args = sizeof...(Args);
        pass{(push(m_state,args),1)...};

        // read result
        int32 ret = lua_pcall(m_state,n_args,1,0);
        if (ret != LUA_OK)
        {
            Value error(m_state);
            error.m_error = true;
            return error;
        }

        return Value(m_state);
    }

}} // namespace arc::lua
