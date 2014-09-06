#pragma once

#include "SimpleMaterialComponent.hpp"

#include <iostream>

namespace arc
{
	struct MeshInfo
	{
		uint8  primitive_type;
		uint8  index_type;
		uint16 mesh_group;
		uint32 mesh_offset;
	};

	static void src_cb_render_main()
	{
		std::cout << "----------------------------\n";
		using Backend = SimpleRenderComponent::Backend;
		for (uint32 i = 0; i < Backend::s_active_count; i++)
		{
			auto& data = Backend::s_component_data[i];
			auto entity = Backend::s_entity_index_data[i];

			if (data.visible)
			{
				std::cout << entity << " ";
			}
		}
		std::cout << "\n";


		// count max nr of objects rendered
		// get temporary buffers for per instance data
		// (get temporary buffers for per material data)
		// get temporary buffers for draw commands
		// iterate over objects
	}

	/*static*/ void SimpleRenderComponentCallbacks::Register()
	{
		// Render Callback
		entity::register_callback(entity::CallbackType::RenderMain, 0.0f, &src_cb_render_main);
	}
}