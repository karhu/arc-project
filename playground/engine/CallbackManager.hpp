#pragma once

#include "arc/core.hpp"
#include "arc/string/StringView.hpp"
#include "arc/hash/StringHash.hpp"

#include "arc/collections/HashMap.hpp"

#include <functional>

namespace arc { namespace engine {

	class CallbackManager
	{
	public:
		CallbackManager(memory::Allocator* alloc);
	public:
		bool register_callback(StringHash32 category, StringView name, std::function<void()> cb);
		bool unregister_callback(StringHash32 category, StringView name);
	public:
		void call_callbacks(StringHash32 category);
	private:
		struct Callback
		{
			StringHash32 name_hash;
			String name;
			std::function<void()> function;
		};
		HashMap<Array<Callback>> m_callbacks;
		memory::Allocator* m_alloc;
	};

}}