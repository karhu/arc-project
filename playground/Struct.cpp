#include "Struct.hpp"

namespace arc
{
	const DynamicTypeData* TypeInfo<float>::dynamic = new DynamicTypeData{ "float", alignof(float), sizeof(float) };
	const DynamicTypeData* TypeInfo<vec3>::dynamic = new DynamicTypeData{ "vec3",  alignof(vec3),  sizeof(vec3)  };

	const std::array<Field, 2> Description::fields = std::array < Field, 2 > {
		Field{ TypeInfo<float>::dynamic, SH32("scale"), "scale" },
		Field{ TypeInfo<vec3>::dynamic, SH32("position"), "position" },
	};

}