
#include "State.hpp"

#include "arc/string/String.hpp"
#include "arc/string/StringView.hpp"

// lua_Integer 64bit
// lua_Unsigned 32bit

namespace arc { namespace lua {

    State::State()
    {
        m_state = luaL_newstate();
        ARC_ASSERT(m_state != nullptr, "could not create lua state");
    }

    State::~State()
    {
        lua_close(m_state);
    }

    void State::open_standard_libs()
    {
        luaL_openlibs(m_state);
    }

    bool State::execute_file(const char* file_path)
    {
        auto ret_code = luaL_dofile(m_state, file_path);

        // TODO better error handling

        return ret_code == LUA_OK;
    }

    Value::Value()
        : m_state(nullptr)
        , m_ref(LUA_NOREF)
    {}

    Value::~Value()
    {
        if (m_state != nullptr)
            luaL_unref(m_state, LUA_REGISTRYINDEX, m_ref);
    }

    Value::Value(Value&& other)
        : m_state(other.m_state)
        , m_ref(other.m_ref)
        , m_error(other.m_error)
    {
        other.clear();
    }

    Value &Value::operator =(Value&& other)
    {
        m_state = other.m_state;
        m_ref = other.m_ref;
        m_error= other.m_error;

        other.clear();
        return *this;
    }

    bool Value::valid()
    {
        return m_state != nullptr;
    }

    bool Value::is_nil()
    {
        if (!valid()) return false;
        return m_ref == LUA_REFNIL;
    }

    bool Value::is_error()
    {
        ARC_ASSERT(m_error ? valid() : true, "Error value should always be valid");
        return m_error;
    }

    Type Value::type()
    {
        if (!valid()) return Type::INVALID;
        if (is_error()) return Type::ERROR;
        if (m_ref == LUA_REFNIL) return Type::NIL;

        // get value from registry
        lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_ref);

        // get type
        auto type = lua_type(m_state,STACK_TOP);

        // pop value
        lua_pop(m_state,1);

        return static_cast<Type>(type);
    }

    bool Value::get(int32& out_value)
    {
        // safety check
        if (!valid()) return false;

        // get value from registry
        lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_ref);

        // check type
        auto type = lua_type(m_state,STACK_TOP);
        if (type != LUA_TNUMBER)
        {
            lua_pop(m_state,1);
            return false;
        }

        // read integer
        out_value = lua_tointeger(m_state,STACK_TOP);
        lua_pop(m_state,1);
        return true;
    }

    bool Value::get(String& out_value)
    {
        // safety check
        if (!valid()) return false;

        // get value from registry
        lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_ref);

        // check type
        auto type = lua_type(m_state,STACK_TOP);
        if (type != LUA_TSTRING)
        {
            lua_pop(m_state,1);
            return false;
        }

        // read string
        size_t len;
        const char* ptr = lua_tolstring(m_state,STACK_TOP,&len);
        out_value = String(ptr,len);
        lua_pop(m_state,1);
        return true;
    }

    bool Value::get(bool &out_value)
    {
        // safety check
        if (!valid()) return false;

        // get value from registry
        lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_ref);

        // check type
        auto type = lua_type(m_state,STACK_TOP);
        if (type != LUA_TBOOLEAN)
        {
            lua_pop(m_state,1);
            return false;
        }

        // read string
        out_value = lua_toboolean(m_state,STACK_TOP) != 0;
        lua_pop(m_state,1);
        return true;
    }

    void Value::clear()
    {
        m_error = false;
        m_ref = LUA_NOREF;
        m_state = nullptr;
    }

    /* Creates the Value from the top of the stack (and pops it) */
    Value::Value(lua_State *state)
        : m_state(state)
    {
        m_ref = luaL_ref(m_state,LUA_REGISTRYINDEX);
    }

    void push(lua_State *state, int32 value)
    {
        lua_pushinteger(state,value);
    }

    void push(lua_State *state, const String &value)
    {
        lua_pushlstring(state, value.c_str(), value.length());
    }

    void push(lua_State *state, StringView value)
    {
        lua_pushlstring(state,value.c_str(),value.length());
    }

    void push(lua_State *state, Value &value)
    {
        ARC_ASSERT(state == value.m_state, "value belongs to different lua state");

        if (!value.valid()) lua_pushnil(state);

        // get value and push it onto stack
        lua_rawgeti(state, LUA_REGISTRYINDEX, value.m_ref);
    }

    // each value /////////////////////////////////////////////////////////////////

    each_value::iterator each_value::begin() { return iterator{*this}; }

    each_value::iterator each_value::end() { return iterator{*this}; }

    each_value::iterator each_value::iterator::operator ++()
    {
        int32 ret = lua_next(each.m_table.m_state, STACK_TOP-1);
        if (ret == 0)
            each.m_value = Value();
        else
            each.m_value = Value(each.m_table.m_state);

        return *this;
    }

    Value& each_value::iterator::operator *()
    {
        return each.m_value;
    }

    each_value::each_value(Value& v)
        : m_table(v)
    {
        if (!m_table.valid()) return;

        // remember stack state
        m_backup = lua_gettop(m_table.m_state);

        // get value from registry
        lua_rawgeti(m_table.m_state, LUA_REGISTRYINDEX, m_table.m_ref);

        // check type
        auto type = lua_type(m_table.m_state,STACK_TOP);
        if (type != LUA_TTABLE)
        {
            m_value = Value();
            return;
        }

        // get first table entry
        lua_pushnil(m_table.m_state);
        int32 ret = lua_next(m_table.m_state, STACK_TOP-1);
        if (ret == 0)
        {
            m_value = Value();
            return;
        }

        m_value = Value(m_table.m_state);
    }

    bool each_value::iterator::operator!=(const each_value::iterator& other)
    {
        return each.m_value.valid();
    }

    each_value::~each_value()
    {
        if (!m_table.valid()) return;

        lua_settop(m_table.m_state, m_backup);
    }

    const char *str(Type t)
    {
        switch(t)
        {
        case Type::INVALID: return "Type::INVALID";
        case Type::ERROR: return "Type::ERROR";
        case Type::BOOL: return "Type::BOOL";
        case Type::NIL: return "Type::NIL";
        case Type::NUMBER: return "Type::NUMBER";
        case Type::STRING: return "Type::STRING";
        case Type::TABLE: return "Type::TABLE";
        case Type::FUNCTION: return "Type::FUNCTION";
        default: return "<unknown>";
        }
    }



}} // namespace arc::lua
