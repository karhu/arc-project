#include "engine.hpp"

#include <iostream>

#define GLEW_STATIC
#include "GL/glew.h"

#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#include "arc/core.hpp"
#include "arc/lua/State.hpp"
#include "arc/memory/Allocator.hpp"
#include "arc/collections/HashMap.inl"

namespace arc { namespace engine
{

	void _init_glew(const Config& config);
	void _init_sdl2(const Config& config);
	void _check_opengl_extensions();
	void _print_opengl_info();

	static SDL_Window* _sdl_main_window = nullptr;
	static SDL_GLContext _sdl_gl_context = nullptr;
	
	struct EngineState
	{
		EngineState()
			: subsystem_registry(longterm_allocator())
			, frame_begin_callbacks(longterm_allocator())
		{}

		HashMap<Subsystem*> subsystem_registry;
		HashMap<std::function<void(double)>> frame_begin_callbacks;
	};

	EngineState* _state = nullptr;

	void initialize(const Config& config)
	{
		// init state
		_state = longterm_allocator().create<EngineState>();

		// init SDL
		_init_sdl2(config);
		// initialize OpenGL extensions
		_init_glew(config);
		// output some useful information
		_print_opengl_info();
	}

	void shutdown()
	{
		SDL_HideWindow(_sdl_main_window);
		SDL_Quit();
	}

	memory::Allocator& longterm_allocator()
	{
		static memory::Mallocator malloc;
		return malloc;
	}

	void _init_sdl2(const Config& config)
	{
		// init SDL
		SDL_SetMainReady();
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		{
			ARC_ASSERT(false, "SDL_Init error");
		}

		// basic settings
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, config.gl_version_major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, config.gl_version_minor);
		
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		
		//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1);

		// create window 
		int32 window_flags = SDL_WINDOW_OPENGL;
		if (config.window_hidden) 
			window_flags |= SDL_WINDOW_HIDDEN;
		else 
			window_flags |= SDL_WINDOW_SHOWN;

		auto sdl_window = SDL_CreateWindow("arc",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			config.window_width, config.window_height,
			window_flags);

		if (sdl_window == nullptr)
		{
			LOG_CRITICAL(engine_log, "Error initializing SDL window");
			ARC_FAIL_GRACEFULLY();
		}

		// OpenGL context
		auto sdl_gl_context = SDL_GL_CreateContext(sdl_window);
		
		if (sdl_gl_context == nullptr)
		{
			LOG_CRITICAL(engine_log, "SDL OpenGL context is null: ", SDL_GetError());
			ARC_FAIL_GRACEFULLY();
		}

		// TODO LOG & FAIL GRACEFULLY
		SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);

		// remember objects
		_sdl_main_window = sdl_window;
		_sdl_gl_context = sdl_gl_context;
	}

	void _init_glew(const Config& config)
	{
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			LOG_CRITICAL(engine_log, "GLEW error: ", (const char*)glewGetErrorString(err));
			ARC_FAIL_GRACEFULLY();
		}
	}

	void _check_opengl_extensions()
	{
		std::cout << std::endl;

		auto extensions = {
			"GL_ARB_shader_storage_buffer_object",
			"GL_ARB_bindless_texture",
			"GL_ARB_sparse_texture",
			"GL_ARB_buffer_storage",
			"GL_EXT_direct_state_access",
			"GL_ARB_multi_draw_indirect",
			"GL_ARB_shader_draw_parameters",
			"GL_ARB_shader_subroutine",
			"GL_ARB_enhanced_layouts",
		};

		std::cout << "Supported OpenGL extensions: \n";
		for (auto& e : extensions)
		{
			if (glewIsSupported(e))
				std::cout << "   " << e << "\n";
		}
		std::cout << std::endl;
		std::cout << "Unsupported OpenGL extensions: \n";
		for (auto& e : extensions)
		{
			if (!glewIsSupported(e))
				std::cout << "   " << e << "\n";
		}
		std::cout << std::endl;
	}

	void _print_opengl_info()
	{
		// query
		int32 sdl_gl_major, sdl_gl_minor;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &sdl_gl_major);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &sdl_gl_minor);

		int32 bits_depthbuffer;
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &bits_depthbuffer);

		LOG_INFO(engine_log, "SDL OpenGL Version: ", sdl_gl_major, ".", sdl_gl_minor);
		LOG_INFO(engine_log, "Depth Buffer Bits:  ", bits_depthbuffer);
		LOG_INFO(engine_log, "GLEW version:       ", glewGetString(GLEW_VERSION));

		LOG_INFO(engine_log, "OpenGL renderer :   ", glGetString(GL_RENDERER));
		LOG_INFO(engine_log, "OpenGL version:     ", glGetString(GL_VERSION));


		_check_opengl_extensions();
	}

	Subsystem*& _assert_subsystem(uint64 type)
	{
		ARC_ASSERT(_state != nullptr, "arc::engine is not initialized");
		auto& entry = _state->subsystem_registry.get(type, nullptr);
		return entry.value();
	}

	Subsystem* _get_subsystem(uint64 type)
	{
		ARC_ASSERT(_state != nullptr, "arc::engine is not initialized");
		auto entry = _state->subsystem_registry.lookup(type);
		return entry ? entry->value() : nullptr;
	}

	bool initialize_subsystems(lua::State& config)
	{
		ARC_ASSERT(_state != nullptr, "arc::engine is not initialized");
		bool ok = true;
		for (auto& entry : _state->subsystem_registry)
		{
			ok = ok && entry.value()->assert_initialized(config);
		}
		return ok;
	}

	bool finalize_subsystems()
	{
		ARC_ASSERT(_state != nullptr, "arc::engine is not initialized");
		bool ok = true;
		for (auto& entry : _state->subsystem_registry)
		{
			ok = ok && entry.value()->assert_finalized();
			longterm_allocator().destroy<Subsystem>(entry.value());
		}
		return ok;
	}

	// Event handling ////////////////////////////////////////////////////////////

	void _handle_sdl_events()
	{
		return;
		SDL_Event ev;

		while (SDL_PollEvent(&ev))
		{
			switch (ev.type)
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				break;
			}
		}
	}

	// Updates /////////////////////////////////////////////////////////////////

	bool register_frame_begin_cb(StringHash name, const std::function<void(double)>& cb)
	{
		ARC_ASSERT(_state != nullptr, "arc::engine is not initialized");

		auto& reg = _state->frame_begin_callbacks;
		bool added;
		reg.get(name.value(), cb, added);
		ARC_ASSERT(added, "A frame_begin callback with this name allready exists.");

		return added;
	}

	bool unregister_frame_begin_cb(StringHash name)
	{
		ARC_ASSERT(_state != nullptr, "arc::engine is not initialized");

		auto& reg = _state->frame_begin_callbacks;
		bool removed = reg.remove(name.value());
		ARC_ASSERT(removed, "A frame_begin callback with this name is not present.");

		return removed;
	}

	// Subsystem /////////////////////////////////////////////////////////////////
	
	bool Subsystem::assert_initialized(lua::State& config)
	{
		if (!m_initialized)
		{
			m_initialized = initialize(config);
			if (!m_initialized)
			{
				LOG_CRITICAL(engine_log, "Could not initialize subsystem: ", name());
			}
			else
			{
				LOG_INFO(engine_log, "Subsystem initialized: ", name());
			}
		}
		return m_initialized;
	}

	bool Subsystem::assert_finalized()
	{
		if (!m_finalized)
		{
			m_finalized = finalize();
			if (!m_finalized)
			{
				LOG_CRITICAL(engine_log, "Could not finalize subsystem: ", name());
			}
			else
			{
				LOG_INFO(engine_log, "Subsystem finalized: ", name());
			}
		}
		return m_finalized;
	}

	void deprecated_swap()
	{
		ARC_ASSERT(_sdl_main_window != nullptr, "Main window is not initialized");
		SDL_GL_SwapWindow(_sdl_main_window);
	}

	void deprecated_update()
	{
		SDL_PumpEvents();
		_handle_sdl_events();

		double dt = 1.0 / 30.0;

		// call frame begin callbacks
		for (auto& e : _state->frame_begin_callbacks)
		{
			e.value()(dt);
		}
	}

}} // namespace arc::engine