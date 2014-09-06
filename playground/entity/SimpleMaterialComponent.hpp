#pragma once

#include "SimpleComponent.hpp"
#include "arc/math/vectors.hpp"

namespace arc
{
	struct SimpleMaterialData
	{
		vec4 color;
	};

	struct SimpleMaterialComponentCallbacks { static void Register() {} };

	class SimpleMaterialComponent : public SimpleComponent < 
		SimpleMaterialComponent, 
		SimpleMaterialData,
		SimpleMaterialComponentCallbacks>
	{
	public:
		vec4 get_color() { return data()->color; }
		void set_color(vec4 color) { data()->color = color; }
	};

	

} // namespace arc


namespace arc
{

	struct SimpleRenderData
	{
		mat4 transform;
		vec4 color;
		bool visible = false;
		uint32 mesh_id = 0;
	};

	struct SimpleRenderComponentCallbacks { static void Register(); };

	struct SimpleRenderComponent : public SimpleComponent < 
			SimpleRenderComponent, 
			SimpleRenderData, 
			SimpleRenderComponentCallbacks >
	{
	public:
		bool get_visibility() const		{ return data()->visible; }
		vec4 get_color() const			{ return data()->color; }
		mat4 get_transform() const		{ return data()->transform; }
	public:
		void set_visibility(bool value) 		{ data()->visible = value; }
		void set_color(const vec4& value) 		{ data()->color = value; }
		void set_transform(const mat4& value)   { data()->transform = value; }
	};

}