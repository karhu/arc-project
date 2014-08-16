#pragma once

namespace arc { namespace template_util {

	struct foreach_unroll { template<typename ...T> foreach_unroll(T...) {} };

}} // namespace arc::template_util