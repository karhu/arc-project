#pragma once

// pack_element ////////////////////////////////////////////////////////////////////////////////
// source: http://stackoverflow.com/questions/5484930/split-variadic-template-arguments

template <size_t _Ip, class _Tp>
class pack_element_imp;

template <class ..._Tp>
struct pack_types {};

template <size_t Ip>
class pack_element_imp<Ip, pack_types<> >
{
public:
	static_assert(Ip == 0, "tuple_element index out of range");
	static_assert(Ip != 0, "tuple_element index out of range");
};

template <class Hp, class ...Tp>
class pack_element_imp<0, pack_types<Hp, Tp...> >
{
public:
	typedef Hp type;
};

template <size_t Ip, class Hp, class ...Tp>
class pack_element_imp<Ip, pack_types<Hp, Tp...> >
{
public:
	typedef typename pack_element_imp<Ip - 1, pack_types<Tp...> >::type type;
};

template <size_t Ip, class ...Tp>
class pack_element
{
public:
	typedef typename pack_element_imp<Ip, pack_types<Tp...> >::type type;
};

// pack::foreach ////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Example: 
// struct MyHandlerType { template<typename T> static bool iteration(uint32 index, int& counter) { counter++; return true; } };
// int counter = 0;
// bool finished = pack<int,vec4,mat3>::foreach<MyHandlerType>(&counter);
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

template <typename ...Types>
struct parameter_pack
{
	static const uint32 size = sizeof...(Types);

	template<typename Handler, typename UserDataT>
	static bool foreach(UserDataT&& user_data)
	{
		return _impl<Handler, UserDataT>::unroll<size>(user_data);
	}

	template<typename HandlerT, typename UserDataT> 
	struct _impl
	{
		template<uint32 N>
		static bool unroll(UserDataT&& user_data)
		{
			using ElementType = pack_element<size - N, Types...>::type;
			bool cont = HandlerT::iteration<ElementType>(size - N, user_data);
			if (cont)
				return unroll<N - 1>(user_data);
			else
				return false;
		}

		template<> static bool unroll<0>(UserDataT&& user_data) { return true; }
	};
};

*/
