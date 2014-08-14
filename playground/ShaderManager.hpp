
#include "arc/core.hpp"
#include "arc/collections/HashMap.hpp"
#include "arc/hash/StringHash.hpp"

#include "engine.hpp"


namespace arc { namespace engine {

	enum class VertexInputType : uint8
	{
		Float = 0,
		Unsigned = 1,
		Signed = 2,
	};

	class ShaderManager : public Subsystem
	{
	public:
		struct ID
		{
		public:
			uint32 value() { return m_value; }
		private:
			uint32 m_value;
			friend class ShaderManager;
		};
	
		struct VertexAttribute
		{
			String name;
			VertexInputType type;
			uint8 element_count;
			uint8 location;
		};
	public:
		ShaderManager(memory::Allocator& alloc);
		ARC_NO_COPY(ShaderManager);
	public:
		static uint64 SubsystemType();
		virtual const char* name() override;
	protected:
		virtual bool initialize(lua::State& config) override;
		virtual bool finalize() override;
	public:
		ID create_from_file(StringView file_path);

		bool register_vertex_attribute(StringView name, VertexInputType type, uint8 element_count, uint8 location);
		const VertexAttribute* get_vertex_attribute(StringHash name);
		/*
		protected:
		LuaShaderSystem();
		friend class engine::Backend;
		*/
	private:
		lua::State m_lua;
		lua::Value m_fun_load_file;
		lua::Value m_fun_gen_code;
	private:
		bool equal(VertexAttribute& a, VertexAttribute& b);

		HashMap<VertexAttribute> m_vertex_attributes;
	};

}} //namespace arc::engine

