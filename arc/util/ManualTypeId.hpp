#pragma once

namespace arc
{

	template<typename Context>
	struct _ManualTypeIdState
	{
		static typename Context::Type s_next;
	};

	template<typename Context>
	/*static*/ typename Context::Type _ManualTypeIdState<Context>::s_next = Context::First;

	template<typename Context, typename Type>
	struct ManualTypeId
	{
		static bool Valid() { return s_id != Context::Invalid; }
		static void Initialize();
		static void AssertInitialized();
		static typename Context::Type Value();
	private:
		static typename Context::Type s_id;
	};

	template<typename Context, typename Type>
	/*static*/ typename Context::Type ManualTypeId<Context, Type>::s_id = Context::Invalid;

	template<typename Context, typename Type>
	/*static*/ void ManualTypeId<Context, Type>::Initialize()
	{
		// check whether this type was already initialized
		if (s_id != Context::Invalid)
		{
			ARC_ASSERT(false, "Already initialized.");
			return;
		}

		// grab the next id
		auto& next = _ManualTypeIdState<Context>::s_next;

		// check if it is a valid id
		if (next == Context::Invalid)
		{
			ARC_ASSERT(false, "ID space is exhausted");
			return;
		}

		// save the next id
		s_id = next;

		// increment id counter
		if (next == Context::Last)
			next = Context::Invalid;
		else
			next += 1;
	}

	template<typename Context, typename Type>
	/*static*/  void ManualTypeId<Context, Type>::AssertInitialized()
	{
		// check whether this type was already initialized
		if (s_id != Context::Invalid)
		{
			return;
		}

		// grab the next id
		auto& next = _ManualTypeIdState<Context>::s_next;

		// check if it is a valid id
		if (next == Context::Invalid)
		{
			ARC_ASSERT(false, "ID space is exhausted");
			return;
		}

		// save the next id
		s_id = next;

		// increment id counter
		if (next == Context::Last)
			next = Context::Invalid;
		else
			next += 1;
	}

	template<typename Context, typename Type>
	/*static*/ typename Context::Type ManualTypeId<Context, Type>::Value()
	{
		return s_id;
	}


} // namespace arc;