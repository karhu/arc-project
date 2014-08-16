#pragma once

#include <boost/iterator/iterator_facade.hpp>

#include "arc/util/template_util.hpp"
#include "arc/util/tuple_util.hpp"

namespace arc {

	template <typename ...Types>
	struct ZipIteratorHelper
	{
		using ValueT = std::tuple < typename std::iterator_traits<Types>::value_type... >;
		using RefT = std::tuple < typename std::iterator_traits<Types>::value_type&... >;
	};

	template <typename ...Types>
	struct ZipIterator
		: public boost::iterator_facade < ZipIterator<Types...>,
		typename ZipIteratorHelper<Types...>::ValueT,
		boost::random_access_traversal_tag,
		typename ZipIteratorHelper<Types...>::RefT >
	{
	public:
		using RefT = typename ZipIteratorHelper<Types...>::RefT;
		using ThisT = typename ZipIterator < Types... >;
		using Indices = typename tuple_util::make_indices<Types...>::type;
	public:
		ZipIterator() {}
		ZipIterator(Types&& ...args) :m_iterators(std::make_tuple(std::forward<Types>(args)...)) {}
	private:
		template<uint32_t ...I>
		void _increment(tuple_util::indices<I...> it) { template_util::foreach_unroll{ ++std::get<I>(m_iterators)... }; }
		template<uint32_t ...I>
		void _decrement(tuple_util::indices<I...> it) { template_util::foreach_unroll{ --std::get<I>(m_iterators)... }; }

		template<typename T>
		int _advance_fun(T& iter, ptrdiff_t n) { iter += n; return 0; }

		template<uint32_t ...I>
		void _advance(tuple_util::indices<I...> it, ptrdiff_t n) { template_util::foreach_unroll(_advance_fun(std::get<I>(m_iterators), n)...); }

	private:
		RefT dereference() { return tuple_util::dereference<Types...>(m_iterators); }
		const RefT dereference() const { return tuple_util::dereference <Types...>(m_iterators); }

		void increment() { _increment(Indices()); }
		void decrement() { _decrement(Indices()); }
		void advance(ptrdiff_t n) { _advance(Indices(), n); }

		bool equal(const ThisT& other) const { return std::get<0>(m_iterators) == std::get<0>(other.m_iterators); }

		ptrdiff_t distance_to(const ThisT& other) const { return std::get<0>(other.m_iterators) - std::get<0>(m_iterators); }
	private:
		std::tuple<Types...> m_iterators;
	private:
		friend class boost::iterator_core_access;
	};

	template<typename ...Types> inline
		ZipIterator<Types...> make_zip_iterator(Types ...args)
	{
		return ZipIterator<Types...>(std::forward<Types>(args)...);
	}

}