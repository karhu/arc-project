#if 0
#pragma once

#include "SimpleComponent.hpp"
#include "arc/math/vectors.hpp"

namespace arc
{
	struct TransformData
	{
		vec4 position;
		vec4 scale;
		vec4 orientation;
	};

	struct TransformComponentCallbacks { static void Register() {}; };

	class TransformComponent : public SimpleComponent < 
		TransformComponent, 
		TransformData,
		TransformComponentCallbacks>
	{
	public:
		vec4 get_position()    { return data()->position; }
		vec4 get_orientation() { return data()->orientation; }
		vec4 get_scale()       { return data()->scale; }
	public:
		void set_position(vec4 position)    { data()->position = position; }
		void set_orientation(vec4 position) { data()->orientation = position; }
		void set_scale(vec4 position)       { data()->scale = position; }
	};

	struct RenderData
	{
		uint16 material_id;
		uint16 material_instance_id;
		uint32 geometry_id;
	};

} // namespace arc

#endif