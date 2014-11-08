#pragma once

#include "../engine/CallbackManager.hpp"

#include "Keys.hpp"

namespace arc { namespace input {

	class KeyboardState
	{
		public:
			KeyboardState();
		public:
			const char* name();
			StringHash KeyboardState::name_hash();
		public:
			bool register_callbacks(engine::CallbackManager& cbm);
			bool unregister_callbacks(engine::CallbackManager& cbm);
		public:
			/* returns wheter a key was held down during the last frame */
			bool down(Key k);
			/* returns how often a key was pressed during the last frame */
			uint8_t pressed(Key k);
			/* returns how often a key was released during the last frame */
			uint8_t released(Key k);
		protected:
			void update_frame_begin();
		private:
			uint16_t m_h_state[512];
	};

}} // namespace arc::input