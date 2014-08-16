#pragma once

#include <cstdint>

namespace arc { namespace tuple_util {

	/*************************************************************************************************
	 * make indices
	 *
	 * source: http://cpptruths.blogspot.co.uk/2012/06/perfect-forwarding-of-parameter-groups.html
	 *
	 * The indices need to be matched by a template pattern to be able to use them. Example:
	 * 
	 * template<uint32_t ...I, typename ...Types>
	 * void function(indices<I...> it, std::tuple<Types...>& t) 
	 * { 
	 *    template_util::foreach_unroll{ some_function(std::get<I>(t))... }; 
	 * }
	 *
	 * using Indices = make_indices(Types...)::type;
	 *
	 * function(Indices(),my_tuple);
	 *
	/************************************************************************************************/

	template<uint32_t...> struct indices{};

	template<uint32_t I, typename IndexTuple, typename... Types>
	struct make_indices_impl;

	template<uint32_t I, unsigned... Indices, typename T, typename... Types>
	struct make_indices_impl < I, indices<Indices...>, T, Types... >
	{
		typedef typename
			make_indices_impl < I + 1,
			indices<Indices..., I>,
			Types... > ::type type;
	};

	template<uint32_t I, unsigned... Indices>
	struct make_indices_impl < I, indices<Indices...> >
	{
		typedef indices<Indices...> type;
	};

	template<typename... Types>
	struct make_indices
		: make_indices_impl < 0, indices<>, Types... >
	{};

	/************************************************************************************************************************
	* dereference
	*
	* Given a tuple of pointers (or iterators) this returns a tuple of references to the dereferenced pointers (or iterators).
	*
	/************************************************************************************************************************/

	template<typename ...Types>
	struct _derefence_tuple_impl
	{
		using Indices = typename tuple_util::make_indices<Types...>::type;

		template<unsigned... I> static
			auto eval2(tuple_util::indices<I...>, std::tuple<Types...>& t)
			-> decltype(std::tie(*std::get<I>(t)...))
		{
			return std::tie(*std::get<I>(t)...);
		}

		static auto eval(std::tuple<Types...>& t)
			-> decltype(eval2(Indices(), t))
		{
			return eval2(Indices(), t);
		}
	};

	template<typename ...Types>
	auto dereference(std::tuple<Types...>& t)
		-> decltype(_derefence_tuple_impl<Types...>::eval(t))
	{
		return _derefence_tuple_impl<Types...>::eval(t);
	}

	template<typename ...Types>
	auto dereference(const std::tuple<Types...>& t)
		-> decltype(_derefence_tuple_impl<Types...>::eval((std::tuple<Types...>&)t))
	{
		return _derefence_tuple_impl<Types...>::eval((std::tuple<Types...>&)t);
	}

}} // namespace arc::tuple_util