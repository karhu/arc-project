#pragma once

#include "SimpleComponent.hpp"
#include "arc/math/vectors.hpp"

namespace arc
{
	struct SimpleMaterialData
	{
		vec4 color;
	};

	class SimpleMaterialComponent : public SimpleComponent < SimpleMaterialComponent, SimpleMaterialData >
	{
	public:
		vec4 get_color() { return data()->color; }
		void set_color(vec4 color) { data()->color = color; }
	};

} // namespace arc