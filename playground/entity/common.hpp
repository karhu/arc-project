#pragma once

#include "arc/core.hpp"
#include "arc/util/ManualTypeId.hpp"

#include <limits>

// TODO:
// - free list for entities + entity generations
// - better Entity implementation
// - Update interface / Fast Iteration interface
// - resize?
// - deferred entity removal
// - deferred entity delete?

// DONE:
// - second ComponentType
// - register components
// - Entity as proper Engine subsystem
// - deferred component removal
// - refactor into nice functional interface

namespace arc { namespace engine {

	const uint32 INVALID_ENTITY_INDEX = std::numeric_limits<uint32>::max();
	const uint32 INVALID_COMPONENT_INDEX = std::numeric_limits<uint32>::max();

	struct ComponentIdContext
	{
		using Type = uint8;
		static const uint8 First = 0;
		static const uint8 Last = 254;
		static const uint8 Invalid = 255;
	};

}} // namespace arc::engine


