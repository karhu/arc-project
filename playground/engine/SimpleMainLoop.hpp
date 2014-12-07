#pragma once

#include <stdint.h>
#include <chrono>
#include <iostream>

#include "arc/common.hpp"
#include "arc/renderer/Renderer_GL44.hpp"
#include "arc/gl/functions.hpp"
#include "arc/collections/Array.inl"

#include "../engine.hpp"
#include "../input/KeyboardState.hpp"

namespace arc
{
	struct SimpleMainLoop
	{
	public:
		SimpleMainLoop() : SimpleMainLoop(engine::Config(), renderer::Config()) {}

		SimpleMainLoop(const engine::Config& engine_config, const renderer::Config& renderer_config)
			: m_engine_config(engine_config)
			, m_renderer_config(renderer_config)
		{
			// initialize logging
			arc::log::DefaultLogger default_logger;
			arc::log::set_logger(default_logger);

			// initialize engine
			engine::initialize(m_engine_config);

			// initialize renderer
			renderer::AllocatorConfig alloc_config;
			alloc_config.longterm_allocator = &m_longterm_allocator;
			m_renderer = m_longterm_allocator.create<renderer::Renderer_GL44>(m_renderer_config, alloc_config);

			// initialize keyboard input
			m_keyboard = m_longterm_allocator.create<input::KeyboardState>();
		}

		~SimpleMainLoop()
		{
			m_longterm_allocator.destroy(m_renderer);
			m_longterm_allocator.destroy(m_keyboard);

			engine::shutdown();
		}

	public:
		struct Context
		{
			arc::memory::Mallocator&        longterm_allocator;
			arc::renderer::Renderer_GL44&   renderer;
			arc::input::KeyboardState&      keyboard;

			const arc::engine::Config&      engine_config;
		};

		enum class Status : uint8_t
		{
			CONTINUE = 0,
			STOP = 1,
			ERROR = 2,
		};

	public:
		void set_render_function(std::function<Status(Context& context, double dt)> fn)
		{
			m_render_fn = fn;
		}

		void set_update_function(std::function<Status(Context& context, double dt)> fn)
		{
			m_update_fn = fn;
		}

		void set_initialize_function(std::function<bool(Context& context)> fn)
		{
			m_init_fn = fn;
		}
	public:
		Status run()
		{
			if (!m_render_fn || !m_update_fn || !m_init_fn)
			{
				LOG_ERROR("Not all callback functions have been set.");
				return Status::ERROR;
			}

			Context context{
				m_longterm_allocator,
				*m_renderer,
				*m_keyboard,
				m_engine_config
			};

			bool init_ok = m_init_fn(context);
			if (!init_ok)
			{
				LOG_ERROR("Error calling initialization function.");
				return Status::ERROR;
			}

			// timing variables
			using namespace std;
			auto clock = chrono::steady_clock();
			using TimePoint = decltype(clock.now());
			using Duration = chrono::duration < int64_t, std::micro >; // chrono::steady_clock::duration;
			TimePoint t_logic_state, t_last_render;
			Duration d_logic_step = chrono::microseconds(1000000 / 60);
			double d_logic_step_seconds = (1.0 / 1000000.0) * (double)chrono::duration_cast<std::chrono::microseconds>(d_logic_step).count();

			// start clocks
			t_last_render = clock.now();
			t_logic_state = clock.now();

			// main loop
			Status status = Status::CONTINUE;
			bool running = true;
			while (running)
			{
				// call logic update
				TimePoint now = clock.now();
				while (t_logic_state < now)
				{
					engine::deprecated_update();
					m_keyboard->update_frame_begin();
					auto pre_upd = clock.now();
					status = m_update_fn(context, d_logic_step_seconds);
					auto delta_upd = clock.now() - pre_upd;
					double dt_upd = (1.0 / 1000000.0) * (double)chrono::duration_cast<std::chrono::microseconds>(delta_upd).count();

					t_logic_state += d_logic_step;

					//std::cout << t_logic_state.time_since_epoch().count() << ", " << now.time_since_epoch().count() << std::endl;

					if (status == Status::ERROR)
					{
						LOG_ERROR("Error running update function.");
						running = false;
						break;
					}
					else if (status == Status::STOP)
					{
						running = false;
						break;
					}
				}

				// compute time since last render update
				now = clock.now();
				auto delta = now - t_last_render;
				double dt_render = (1.0 / 1000000.0) * (double)chrono::duration_cast<std::chrono::microseconds>(delta).count();
				t_last_render = now;

				std::cout << "<" << dt_render << ">\n";

				// call render update
				m_renderer->update_frame_begin();
				m_render_fn(context, dt_render);

				if (status == Status::ERROR)
				{
					LOG_ERROR("Error running render function.");
					break;
				}
				else if (status == Status::STOP)
				{
					break;
				}

				// end of frame context updates
				m_renderer->update_frame_end();
				engine::deprecated_swap();
			}

			return status;
		}

	public:
		memory::Allocator& longterm_allocator() { return m_longterm_allocator; }
	protected:
		arc::memory::Mallocator         m_longterm_allocator;
		arc::log::DefaultLogger	        m_default_logger;
	protected:
		engine::Config					m_engine_config;
		renderer::Config				m_renderer_config;
	protected:
		arc::renderer::Renderer_GL44*   m_renderer = nullptr;
		arc::input::KeyboardState*      m_keyboard = nullptr;
	protected:
		std::function<Status(Context& context, double dt)>    m_render_fn = nullptr;
		std::function<Status(Context& context, double dt)>    m_update_fn = nullptr;
		std::function<bool(Context& context)>			      m_init_fn = nullptr;
	};
}