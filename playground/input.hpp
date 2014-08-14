#pragma once

#include "engine.hpp"
#include "Keys.hpp"

namespace arc { namespace input {

	class KeyboardState final : public engine::Subsystem
	{
		public:
			static uint64 SubsystemType();
		public:
			const char* name() override;
			StringHash KeyboardState::name_hash();
		public:
			KeyboardState(memory::Allocator& alloc);
		public:
			/* returns wheter a key was held down during the last frame */
			bool down(Key k);
			/* returns wheter a key was freshly pressed during the last frame */
			bool pressed(Key k);
			/* returns wheter a key was freshly released during the last frame */
			bool released(Key k);
		protected:
			bool initialize(lua::State& config) override;
			bool finalize() override;
		private:
			void on_frame_begin(double dt);
			uint8 m_h_state[512];
	};

}} // namespace arc::input