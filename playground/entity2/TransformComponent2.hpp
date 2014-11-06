#pragma once

#include "entity2.hpp"
#include "SimpleComponent.hpp"

#include "arc/common.hpp"
#include "arc/math/vectors.hpp"

// TODO:
// - fix resizing bug

namespace arc { namespace entity2 {

	// TransformComponent ///////////////////////////////////////////////////////////////////////////

	struct TransformData
	{
		vec3  position;
		float scale;
		vec4  orientation;
	};

	class TransformComponentHandle : public SimpleComponentHandle<TransformData>
	{
	public:
		vec3  get_position()    { return data().position; }
		vec4  get_orientation() { return data().orientation; }
		float get_scale()       { return data().scale; }
	public:
		void set_position(vec3 position)       { data().position = position; }
		void set_orientation(vec4 orientation) { data().orientation = orientation; }
		void set_scale(float position)         { data().scale = position; }
	};

	using TransformComponent = SimpleComponent < TransformData, TransformComponentHandle > ;

	// RenderComponent //////////////////////////////////////////////////////////////////////////////

	struct RenderData
	{
		mat4 transform;
		vec4 color;
		bool visible = false;
		uint32 mesh_id = 0;
	};


	struct RenderComponentHandle : public SimpleComponentHandle<RenderData>
	{
	public:
		bool get_visibility() const		{ return data().visible; }
		vec4 get_color() const			{ return data().color; }
		mat4 get_transform() const		{ return data().transform; }
	public:
		void set_visibility(bool value) 		{ data().visible = value; }
		void set_color(const vec4& value) 		{ data().color = value; }
		void set_transform(const mat4& value)   { data().transform = value; }
	};

	using RenderComponent = SimpleComponent < RenderData, RenderComponentHandle >;

	// TestComponent /////////////////////////////

	struct TestComponentData
	{
		mat4 transform;
		vec4 color;
		uint32 mesh_id;
	};

	struct TestComponentHandle : public SimpleComponentHandle<TestComponentData>
	{
	public:
		vec4 get_color() const			{ return data().color; }
		mat4 get_transform() const		{ return data().transform; }
	public:
		void set_color(const vec4& value) 		{ data().color = value; }
		void set_transform(const mat4& value)   { data().transform = value; }
	public:
		bool valid() { return m_ptr != nullptr; }
	private:
		TestComponentData* m_ptr = nullptr;
	};

	struct TestComponent
	{
		using Data = TestComponentData;
		using Handle = TestComponentHandle;
	public:
		TestComponent(memory::Allocator* alloc, uint32 initial_capacity);
	public:
		Handle add_component(entity2::Handle h);
		Handle get_component(entity2::Handle h);
	private:
		HashMap<int32>			m_mapping;
		Array<Data>				m_data;
		Array<entity2::Handle>	m_entities;
	};

	// TestComponent2 /////////////////////////////
#if 0
	struct TestComponent2;

	struct TestComponent2Handle
	{
	public:
		vec4 get_color() const					{ return m_ptr->m_data_color[m_offset]; }
		mat4 get_transform() const				{ return m_ptr->m_data_transform[m_offset]; }
	public:
		void set_color(const vec4& value) 		{ m_ptr->m_data_color[m_offset] = value; }
		void set_transform(const mat4& value)   { m_ptr->m_data_transform[m_offset] = value; }
	public:
		bool valid() { return m_ptr != nullptr; }
	private:
		TestComponent2* m_ptr = nullptr;
		uint32_t m_offset = 0;
	};

	struct TestComponent2
	{
		using Data = TestComponentData;
		using Handle = TestComponent2Handle;
	public:
		TestComponent2(memory::Allocator* alloc, uint32 initial_capacity);
	public:
		Handle add_component(entity2::Handle h);
		Handle get_component(entity2::Handle h);
	private:
		Array<mat4>				m_data_transform;
		Array<vec4>				m_data_color;
		Array<uint32>			m_data_mesh_id;

		HashMap<int32>			m_mapping;
		Array<entity2::Handle>	m_entities;
	private:
		friend class TestComponent2Handle;
	};
#endif
}}