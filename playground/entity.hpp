#pragma once

#include "arc/core.hpp"
#include "arc/lua/State.hpp"
#include "arc/util/ManualTypeId.hpp"

// public interface ///////////////////////////////////////////
namespace arc { namespace entity {

		using ComponentIndex = uint32;
		using ComponentType  = uint8;

		const uint32 INVALID_ENTITY_INDEX = std::numeric_limits<uint32>::max();
		const uint32 INVALID_COMPONENT_INDEX = std::numeric_limits<uint32>::max();

		struct Handle
		{
		public: // constructor
			Handle(uint32 index = INVALID_ENTITY_INDEX);
		private:
			uint32 m_index;
		private:
			friend uint32 _index(Handle h);
		};

		bool initialize(lua::State& config);
		bool finalize();
		bool is_initialized();

		template<typename T>
		bool register_component(uint32 max_count);

		Handle create();
		//bool destroy(Handle h);

		bool valid(Handle h);
		
		template<typename T>
		T get(Handle h);

		template<typename T>
		bool has(Handle h);

		template<typename T>
		T add(Handle h);

		template<typename T>
		bool remove(Handle h);

}} // namespace arc::entity


// implementation utils ///////////////////////////////////////////
namespace arc { namespace entity {

	uint32 _index(Handle h);
	uint32 _get_component(uint32 entity_index, ComponentType type);
	bool   _has_component(uint32 entity_index, ComponentType type);
	uint32 _add_component(uint32 entity_index, ComponentType type, uint32 component_index);
	bool   _remove_component_later(Handle h, ComponentType type);

/*	using RemovalCallback = void *(fun)()

	void   _register_component_type(ComponentType type, RemovalCallback rm_cb);*/

	struct ComponentTypeContext
	{
		using Type = uint8;
		static const uint8 First = 0;
		static const uint8 Last = 254;
		static const uint8 Invalid = 255;
	};

}} // namespace arc::entity

// template function implementations //////////////////////////////
namespace arc { namespace entity {

	template<typename T>
	bool register_component(uint32 max_count)
	{
		ARC_ASSERT(is_initialized(), "Entity system is not initialized.");

		auto& alloc = engine::longterm_allocator();
		bool ok = T::Backend::Initialize(&alloc, max_count);

		if (ok)
		{
			ManualTypeId<ComponentTypeContext, T>::Initialize();
			return true;
		}

		return false;
	}

	template<typename T>
	T get(Handle h)
	{
		ARC_ASSERT(is_initialized(), "Entity system is not initialized.");
		ARC_ASSERT(valid(h), "Trying to use invalid EntityInterface");

		auto type_id = ManualTypeId<ComponentTypeContext, T>::Value();
		auto component_index = _get_component(_index(h), type_id);
		return T::Backend::Get(component_index);
	}

	template<typename T>
	bool has(Handle h)
	{
		ARC_ASSERT(is_initialized(), "Entity system is not initialized.");
		ARC_ASSERT(valid(h), "Trying to use invalid EntityHandle.");
		auto type_id = ManualTypeId<ComponentTypeContext, T>::Value();
		return _has_component(_index(h), type_id);
	}

	template<typename T>
	T add(Handle h)
	{
		ARC_ASSERT(is_initialized(), "Entity system is not initialized.");
		ARC_ASSERT(valid(h), "Trying to use invalid EntityHandle.");

		auto type_id = ManualTypeId<ComponentTypeContext, T>::Value();

		// check if the component is already present
		if (_has_component(_index(h), type_id)) return {};

		// create the component
		uint32 component_index = T::Backend::Create(_index(h));
		_add_component(_index(h), type_id, component_index);

		return T::Backend::Get(component_index);
	}

	template<typename T>
	bool remove(Handle h)
	{
		ARC_ASSERT(is_initialized(), "Entity system is not initialized.");
		ARC_ASSERT(valid(h), "Trying to use invalid EntityHandle.");

		auto type_id = ManualTypeId<ComponentTypeContext, T>::Value();

		// get the component
		uint32 component_index = _get_component(_index(h), type_id);

		return _remove_component_later(h, type_id);
	}

}} // namespace arc::entity


#include <boost/iterator/iterator_facade.hpp>

template <typename T1, typename T2>
struct OldZipIteratorHelper 
{
	using ValueT = std::tuple<typename std::iterator_traits<T1>::value_type, typename std::iterator_traits<T2>::value_type>;
	using RefT   = std::tuple<typename std::iterator_traits<T1>::value_type&, typename std::iterator_traits<T2>::value_type&>;
};

template<typename T1, typename T2>
class OldZipIterator 
	: public boost::iterator_facade< OldZipIterator<T1,T2>,
									 typename OldZipIteratorHelper<T1, T2>::ValueT,
									 boost::random_access_traversal_tag,
									 typename OldZipIteratorHelper<T1, T2>::RefT >
{
public:
	using RefType = typename OldZipIteratorHelper<T1, T2>::RefT;
	using ThisType = typename OldZipIterator<T1, T2>;
public:
	OldZipIterator() {}
	OldZipIterator(T1 p1, T2 p2) : m_p1(p1), m_p2(p2) {}
public:
	RefType dereference() { return RefType(*m_p1, *m_p2); }
	const RefType dereference() const { return RefType(*m_p1, *m_p2); }
	bool equal(const ThisType& other) const { return m_p1 == other.m_p1; }
	void increment() { ++m_p1; ++m_p2; }
	void decrement() { --m_p1; --m_p2; }
	void advance(ptrdiff_t n) { m_p1 += n; m_p2 += n; }
	ptrdiff_t distance_to(const ThisType& other) const { return other.m_p1 - m_p1; }
private:
	T1 m_p1;
	T2 m_p2;
};

template<typename T1, typename T2> inline
OldZipIterator<T1, T2> make_old_zip_iterator(T1 iter1, T2 iter2)
{
	return { iter1, iter2 };
}

#include "template_util.hpp"

template<typename ...Types>
struct dereference_tuple_impl
{
	using Indices =  typename make_indices<Types...>::type;

	template<unsigned... I> static
	auto eval2(index_tuple<I...>, std::tuple<Types...>& t)
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
auto dereference_tuple(std::tuple<Types...>& t)
	-> decltype(dereference_tuple_impl<Types...>::eval(t))
{
	return dereference_tuple_impl<Types...>::eval(t);
}

template<typename ...Types>
auto dereference_tuple(const std::tuple<Types...>& t)
-> decltype(dereference_tuple_impl<Types...>::eval((std::tuple<Types...>&)t))
{
	return dereference_tuple_impl<Types...>::eval((std::tuple<Types...>&)t);
}

template <typename ...Types>
struct ZipIteratorHelper
{
	using ValueT = std::tuple<typename std::iterator_traits<Types>::value_type...>;
	using RefT = std::tuple<typename std::iterator_traits<Types>::value_type&...>;
};

struct pass { template<typename ...T> pass(T...) {} };

template <typename ...Types>
struct ZipIterator
	: public boost::iterator_facade< ZipIterator<Types...>,
									 typename ZipIteratorHelper<Types...>::ValueT,
									 boost::random_access_traversal_tag,
									 typename ZipIteratorHelper<Types...>::RefT >
{
public:
	using RefT = typename ZipIteratorHelper<Types...>::RefT;
	using ThisT = typename ZipIterator<Types...>;
	using Indices = typename make_indices<Types...>::type;
public:
	ZipIterator() {}
	ZipIterator(Types&& ...args) :m_iterators(std::make_tuple(std::forward<Types>(args)...)) {}
private:
	template<uint32_t ...I>
	void _increment(index_tuple<I...> it) { pass{ ++std::get<I>(m_iterators)... }; }
	template<uint32_t ...I>
	void _decrement(index_tuple<I...> it) { pass{ --std::get<I>(m_iterators)... }; }
	
	template<typename T>
	int _advance_fun(T& iter, ptrdiff_t n) { iter += n; return 0; }

	template<uint32_t ...I>
	void _advance(index_tuple<I...> it, ptrdiff_t n) { pass( _advance_fun(std::get<I>(m_iterators), n)... ); }

private:
	RefT dereference() { return dereference_tuple<Types...>(m_iterators); }
	const RefT dereference() const { return dereference_tuple<Types...>(m_iterators); }

	void increment() { _increment(Indices()); }
	void decrement() { _decrement(Indices()); }
	void advance(ptrdiff_t n) { _advance(Indices(),n); }

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

/*
template<typename ...Types>
class StructOfArrays
{
public:
	static const uint32 FIELD_COUNT = sizeof...(Types);
public:
	void initialize(memory::Allocator* alloc, uint32 size)
	{
		ARC_ASSERT(m_alloc == nullptr, "Already initialized.");

		m_size = size;
		m_alloc = alloc;

		parameter_pack<Types...>::foreach<InitIterator>(*this);
	}

public:
	template<uint32 FieldIndex, typename T>
	T& get(uint32 index)
	{
		static_assert(FieldIndex < FIELD_COUNT, "Field index out of bounds.");
		using FieldType = pack_element<FieldIndex, Types...>::type;
		static_assert(std::is_same<T, FieldType>::value, "Incorrect type requested");

		ARC_ASSERT(index < m_size, "Invalid element index: %u",index);

		FieldType* data = static_cast<FieldType*>(m_data[FieldIndex]);
		return data[index];
	}

private:
	using ThisT = StructOfArrays < Types... > ;

	struct InitIterator { template<typename T> static bool iteration(uint32 index, ThisT& self) 
	{
		self.m_data[index] = self.m_alloc->allocate(sizeof(T)*self.m_size, alignof(T));
		return true;
	}};

private:
	void* m_data[FIELD_COUNT];
	uint32 m_size = 0;
	memory::Allocator* m_alloc = nullptr;
};

*/