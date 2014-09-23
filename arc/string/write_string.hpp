#pragma once

#include "arc/common.hpp"
#include "arc/collections/Slice.hpp"

namespace arc
{
	// TODO: writer interface instead of buffer? etc?
	bool write_string(Slice<char>& buffer, uint32 v);
	bool write_string(Slice<char>& buffer, int32 v);

	bool write_string(Slice<char>& buffer, void* v);
}