#include "input.hpp"

#include <iostream>

#include "SDL2\SDL.h"

namespace arc { namespace input {

	const char* KeyboardState::name() { return "arc::input::KeyboardState"; }
	StringHash  KeyboardState::name_hash() { return SH("arc::input::KeyboardState"); }

	uint64 KeyboardState::SubsystemType() { return SH("arc::input::KeyboardState").value(); }

	KeyboardState::KeyboardState(memory::Allocator& alloc)
	{}

	static const uint8 STATE_DOWN = 0x1;
	static const uint8 STATE_PRESSED = 0x2;
	static const uint8 STATE_RELEASED = 0x4;

	bool KeyboardState::initialize(lua::State& config)
	{
		// initialize hardware key state
		for (uint32 i = 0; i < SDL_NUM_SCANCODES; i++) m_h_state[i] = 0;

		static_assert(SDL_NUM_SCANCODES <= 512, "we assume 511 to be the highest possible scancode");

		std::function<void(double)> cb = std::bind(&KeyboardState::on_frame_begin, this, std::placeholders::_1);
		engine::register_frame_begin_cb(name_hash(), cb);

		return true;
	}

	bool KeyboardState::finalize()
	{
		engine::unregister_frame_begin_cb(name_hash());
		return true;
	}

	struct _KEY_FILTER_USERDATA
	{
		uint8* h_state_new;
	};

	int _sdl_key_filter(void* obj, SDL_Event* event)
	{
		const int HANDLED = 0;
		const int UNHANDLED = 1;

		_KEY_FILTER_USERDATA& ud = *((_KEY_FILTER_USERDATA*)obj);
		SDL_Event& ev = *event;

		if (event->type == SDL_KEYDOWN)
		{
			SDL_KeyboardEvent& ev = event->key;
			// add state down & released
			ud.h_state_new[ev.keysym.scancode] |= STATE_PRESSED | STATE_DOWN; 
			return HANDLED;
		}
		if (event->type == SDL_KEYUP)
		{
			SDL_KeyboardEvent& ev = event->key;
			// add state released
			ud.h_state_new[ev.keysym.scancode] |= STATE_RELEASED; 
			// clear state down
			ud.h_state_new[ev.keysym.scancode] &= ~STATE_DOWN; 
			return HANDLED;
		}
		return UNHANDLED;
	}

	bool KeyboardState::down(Key k)
	{
		// we also return down if the state is only pressed
		// this is the case if press and release happened in the 
		// same frame
		return 0 != (m_h_state[(uint16)k] & (STATE_DOWN | STATE_PRESSED));
	}

	bool KeyboardState::pressed(Key k)
	{
		return 0 != (m_h_state[(uint16)k] & STATE_PRESSED);
	}

	bool KeyboardState::released(Key k)
	{
		return 0 != (m_h_state[(uint16)k] & STATE_RELEASED);
	}

	void KeyboardState::on_frame_begin(double dt)
	{
		// forget about press and release
		for (uint32 i = 0; i < SDL_NUM_SCANCODES; i++)
		{
			// only keep state down
			m_h_state[i] &= STATE_DOWN;
		}

		_KEY_FILTER_USERDATA ud = { m_h_state };
		SDL_PumpEvents();
		SDL_FilterEvents(_sdl_key_filter, &ud);
	}

}} // namespace arc::input