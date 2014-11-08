#include "KeyboardState.hpp"

#include <iostream>

#include "SDL2\SDL.h"

namespace arc { namespace input {

	const char* KeyboardState::name() { return "arc::input::KeyboardState"; }
	StringHash  KeyboardState::name_hash() { return SH("arc::input::KeyboardState"); }

	static const uint8 STATE_DOWN = 0x1;
	static const uint8 STATE_PRESSED = 0x2;
	static const uint8 STATE_RELEASED = 0x4;

	KeyboardState::KeyboardState()
	{
		for (uint32 i = 0; i < SDL_NUM_SCANCODES; i++) m_h_state[i] = 0;

		static_assert(SDL_NUM_SCANCODES <= 512, "we assume 511 to be the highest possible scancode");
	}

	bool KeyboardState::register_callbacks(engine::CallbackManager& cbm)
	{
		bool ok = cbm.register_callback(
			SH32("engine_frame_begin"),
			"KeyboardState::update_frame_begin()",
			std::bind(&KeyboardState::update_frame_begin, this));

		return ok;
	}

	bool unregister_callbacks(engine::CallbackManager& cbm)
	{
		bool ok = cbm.unregister_callback(
			SH32("engine_frame_begin"),
			"KeyboardState::update_frame_begin()");
		return ok;
	}

	union KeyState
	{
		struct
		{
			uint16_t current : 1;
			uint16_t down_once : 1;
			uint16_t pressed_counter : 7;
			uint16_t released_counter : 7;
		} fields;
		uint16_t raw;
	};

	int _sdl_key_filter(void* obj, SDL_Event* event)
	{
		const int HANDLED = 0;
		const int UNHANDLED = 1;

		KeyState* state = (KeyState*)obj;
		SDL_Event& ev = *event;

		if (event->type == SDL_KEYDOWN)
		{
			SDL_KeyboardEvent& ev = event->key;
			ARC_ASSERT(ev.keysym.scancode < 512, "sdl scan code out of bounds");

			// add state down & released
			state[ev.keysym.scancode].fields.pressed_counter += 1;
			state[ev.keysym.scancode].fields.down_once = true;
			state[ev.keysym.scancode].fields.current = true;
			return HANDLED;
		}
		if (event->type == SDL_KEYUP)
		{
			SDL_KeyboardEvent& ev = event->key;
			ARC_ASSERT(ev.keysym.scancode < 512, "sdl scan code out of bounds");

			// add state down & released
			state[ev.keysym.scancode].fields.released_counter += 1;
			state[ev.keysym.scancode].fields.down_once = true;
			state[ev.keysym.scancode].fields.current = false;

			return HANDLED;
		}

		return UNHANDLED;
	}

	bool KeyboardState::down(Key k)
	{
		// we also return down if the state is only pressed
		// this is the case if press and release happened in the 
		// same frame
		KeyState ks; ks.raw = m_h_state[(uint16)k];
		return ks.fields.down_once;
	}

	uint8_t KeyboardState::pressed(Key k)
	{
		KeyState ks; ks.raw = m_h_state[(uint16)k];
		return ks.fields.pressed_counter;
	}

	uint8_t KeyboardState::released(Key k)
	{
		KeyState ks; ks.raw = m_h_state[(uint16)k];
		return ks.fields.released_counter;
	}

	void KeyboardState::update_frame_begin()
	{
		// forget about press and release
		for (uint32 i = 0; i < SDL_NUM_SCANCODES; i++)
		{
			KeyState state; state.raw = m_h_state[i];
			state.fields.pressed_counter = 0;
			state.fields.released_counter = 0;
			state.fields.down_once = state.fields.current;
			m_h_state[i] = state.raw;
		}

		SDL_PumpEvents();
		SDL_FilterEvents(_sdl_key_filter, &m_h_state);
	}

}} // namespace arc::input