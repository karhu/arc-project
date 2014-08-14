#include "allocator.hpp"

#include "arc/logging/log.hpp"

#include "arc/core/numeric_types.hpp"

#include <cstdlib>
#include <iostream>

struct malloc_log
{
	static const char* name() { return "malloc"; }
	static int priority() { return arc::log::PRIORITY_CRITICAL;  }
};

namespace arc {
namespace memory
{
    void* Mallocator::allocate(uint64 size, uint32 align)
    {
        //TODO handle alignment
        auto ptr = std::malloc((size_t)size);
		LOG_DEBUG(malloc_log, ptr, " = allocate(", size, ",", align, ")");
        return ptr;
    }

    void Mallocator::free(void *data)
    {
        //TODO handle alignment
		LOG_DEBUG(malloc_log, "free(", data, ")");
        std::free(data);
    }

}} //namespaces
