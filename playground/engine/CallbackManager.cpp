#include "CallbackManager.hpp"

#include "arc/collections/HashMap.inl"

namespace arc { namespace engine {

	CallbackManager::CallbackManager(memory::Allocator* alloc)
		: m_callbacks(*alloc), m_alloc(alloc)
	{}

	bool CallbackManager::register_callback(StringHash32 category, StringView name, std::function<void()> cb)
	{
		auto name_hash = string_hash32(name);
		auto& entry = m_callbacks.get(category.value(), Array<Callback>(*m_alloc));

		// check if a callback with the same name is already present in this category
		for (auto& c : entry.value()) { if (c.name_hash == name_hash) return false; }
		
		entry.value().push_back(Callback{ name_hash, name, cb });
		return true;
	}

	bool CallbackManager::unregister_callback(StringHash32 category, StringView name)
	{
		auto name_hash = string_hash32(name);
		auto entry = m_callbacks.lookup(category.value());
		if (!entry) return false;

		// look for a callback with the requested name in this category
		auto& list = entry->value();
		for (uint32_t i = 0; i < list.size(); i++)
		{
			// if we find it, remove it from the list and return
			auto& e = list[i];
			if (e.name_hash == name_hash)
			{
				list[i] = list.back();
				list.pop_back();
				return true;
			}
		}
		
		return false;
	}

	void CallbackManager::call_callbacks(StringHash32 category)
	{
		auto entry = m_callbacks.lookup(category.value());
		if (!entry) return;

		for (auto& c : entry->value()) c.function();
	}

}}