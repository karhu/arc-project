#pragma once

#include <cstdlib>
#include <tuple>

#include "core/compatibility.hpp"

// common macros
/////////////////////////////////////////////////////////////////////////////

/** no copy constructor & assignment operator macro **/
#define ARC_NO_COPY(TYPE)                     \
    TYPE(const TYPE &) = delete;              \
    TYPE& operator=(const TYPE &) = delete    \

/** default move constructor & assignment operator macro **/
#define ARC_DEFAULT_MOVE(TYPE)                \
    TYPE(TYPE&&) = default;                   \
    TYPE& operator=(TYPE&&) = default         \

/** controled program exit in case of fatal error **/
#define ARC_FAIL_GRACEFULLY()          \
	exit(EXIT_FAILURE)

/** controled program exit in case of fatal error **/
#define ARC_FAIL_GRACEFULLY_MESSAGE(message)          \
	exit(EXIT_FAILURE)

// common forward declarations
/////////////////////////////////////////////////////////////////////////////

namespace arc
{
    namespace memory
    {
        class Allocator;
    }
}

namespace arc
{
	using std::tie;
	using std::tuple;
}

// common includes
/////////////////////////////////////////////////////////////////////////////

#include "arc/core/assert.hpp"
//#include "arc/core/exception.hpp"
#include "arc/core/numeric_types.hpp"
//#include "arc/core/smart_pointers.hpp"
//#include "arc/core/slice.hpp"
//#include "arc/logging/interface.hpp"

