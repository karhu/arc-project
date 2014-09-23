#pragma once

#include "arc/core/assert.hpp"
#include "arc/core/numeric_types.hpp"
#include "arc/core/compatibility.hpp"

namespace arc
{
    template<typename T>
    struct Slice
    {
    public:
        using ElementType = T;
    public:
        ARC_CONSTEXPR Slice(T* data = nullptr, uint64 size = 0);
    public:
        /// element access
        T& operator[] (uint64 idx);

        /// element access
        const T& operator[] (uint64 idx) const;

        /// raw pointer
        T* ptr();

        /// raw pointer
        const T* ptr() const;

        /// element count
        uint64 size() const;
	public:
		void trim_front(uint32 n);
		void trim_back(uint32 n);

    private:
        T* _data = nullptr;
        uint64 _size = 0;
    };

	// implementation ////////////////////////////////////////////////////////////////

    template<typename T> inline
    ARC_CONSTEXPR Slice<T>::Slice(T* data, uint64 size) : _data(data), _size(size) {}

    template<typename T> inline
    ARC_CONSTEXPR Slice<T> make_slice(T* data, uint64 size)
    {
        return Slice<T>(data,size);
    }

    template<typename T> inline
    ARC_CONSTEXPR const Slice<T> make_slice(const T* data, uint64 size)
    {
        return Slice<T>(const_cast<T*>(data),size);
    }

    template<typename T> inline
    T& Slice<T>::operator[] (uint64 idx)
    {
        return _data[idx];
    }

    template<typename T> inline
    const T& Slice<T>::operator[] (uint64 idx) const
    {
        return _data[idx];
    }

    template<typename T> inline
    T* Slice<T>::ptr()
    {
        return _data;
    }

    template<typename T> inline
    const T* Slice<T>::ptr() const
    {
        return _data;
    }

    template<typename T> inline
    uint64 Slice<T>::size() const
    {
        return _size;
    }

	template<typename T> inline
	void Slice<T>::trim_front(uint32 n)
	{
		ARC_ASSERT(n <= _size, "Trim value larger than buffer length");
		_data += n;
		_size -= n;
	}

	template<typename T> inline
	void Slice<T>::trim_back(uint32 n)
	{
		ARC_ASSERT(n <= _size, "Trim value larger than buffer length");
		_size -= n;
	}


	// range based for-loop support /////////////////////////////////////////////////////

    template<typename T> ARC_CONSTEXPR
    T* begin(arc::Slice<T>& a) { return a.ptr(); }

    template<typename T> ARC_CONSTEXPR
    T* end (arc::Slice<T>& a) { return a.ptr() + a.size(); }

    template<typename T> ARC_CONSTEXPR
    const T* begin(const arc::Slice<T>& a) { return a.ptr(); }

    template<typename T> ARC_CONSTEXPR
    const T* end (const arc::Slice<T>& a) { return a.ptr() + a.size(); }

} // namespace std
