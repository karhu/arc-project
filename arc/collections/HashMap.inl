#pragma once

#include "HashMap.hpp"

#include "arc/core.hpp"
#include "arc/collections/Array.inl"

namespace arc
{
    template<typename T>
	HashMap<T>::HashMap(memory::Allocator &alloc)
        : m_hashes(alloc), m_data(alloc)
    {}


	template<typename T>
	void HashMap<T>::initialize(memory::Allocator* alloc)
	{
		m_hashes.initialize(alloc);
		m_data.initialize(alloc);
	}

	template<typename T>
	void HashMap<T>::finalize()
	{
		m_hashes.finalize();
		m_data.finalize();
	}

	template<typename T>
	bool HashMap<T>::is_initialized()
	{
		return m_hashes.is_initialized();
	}

    template<typename T>
    bool HashMap<T>::remove(uint64 key)
    {
        auto fr = find_prev(key);

        // no such entry present
        if (fr.d_idx == END_INDEX) return false;

        // first entry in hash collision chain
        if (fr.prev_d_idx == END_INDEX)
        {
            m_hashes[fr.h_idx] = m_data[fr.d_idx].m_next;
        }
        // later entry
        else
        {
            m_data[fr.prev_d_idx].m_next = m_data[fr.d_idx].m_next;
        }

        // last entry in the data array
        if (fr.d_idx == m_data.size() -1)
        {
            m_data.pop_back();
            return true;
        }

        // else swap last entry and entry to be removed
        // this needs some updates to the book keeping
        auto fr2 = find_prev(m_data.back().m_key);
        if (fr2.prev_d_idx == END_INDEX)
            m_hashes[fr2.h_idx] = fr.d_idx;
        else
            m_data[fr2.prev_d_idx].m_next = fr.d_idx;

        m_data[fr.d_idx] = std::move(m_data.back());
        m_data.pop_back();

        return true;
    }

    template<typename T>
    bool HashMap<T>::contains(uint64 key) const
    {
        return lookup(key) != nullptr;
    }



    template<typename T>
    typename HashMap<T>::FindResult HashMap<T>::find(uint64 key) const
    {
        FindResult fr;
        fr.h_idx = END_INDEX;
        fr.d_idx = END_INDEX;

        // empty hash map case
        if (m_hashes.size() == 0) return fr;

        // hash uint64 value
        fr.h_idx = key % m_hashes.size();

        // move through the linked list of entries with hash collisions
        fr.d_idx = m_hashes[fr.h_idx];
        while (fr.d_idx != END_INDEX)
        {
            auto& data = m_data[fr.d_idx];
            if (data.m_key == key) return fr;
            fr.d_idx = data.m_next;
        }
        return fr;
    }

    template<typename T>
    typename HashMap<T>::FindResultPrev HashMap<T>::find_prev(uint64 key) const
    {
        FindResultPrev fr;
        fr.h_idx = END_INDEX;
        fr.d_idx = END_INDEX;
        fr.prev_d_idx = END_INDEX;

        // empty hash map case
        if (m_hashes.size() == 0) return fr;

        // hash uint64 value
        fr.h_idx = key % m_hashes.size();

        // move through the linked list of entries with hash collisions
        fr.d_idx = m_hashes[fr.h_idx];
        while (fr.d_idx != END_INDEX)
        {
            auto& data = m_data[fr.d_idx];
            if (data.m_key == key) return fr;
            fr.prev_d_idx = fr.d_idx;
            fr.d_idx = data.m_next;
        }
        return fr;
    }

    template<typename T>
    bool HashMap<T>::present(typename const HashMap<T>::FindResult& fr) const
    {
        return fr.d_idx != END_INDEX;
    }

    template<typename T>
    void HashMap<T>::reserve(uint32 size)
    {
        float s = size;
        rehash((s*10.0f/7.0f)+1);
    }

    template<typename T>
    void HashMap<T>::clear()
    {
        m_hashes.clear();
        m_data.clear();
    }

    template<typename T>
    uint32 HashMap<T>::size() const
    {
        return m_data.size();
    }


    template<typename T>
	typename HashMap<T>::Entry& HashMap<T>::set(uint64 key, const T& value)
    {
        auto fr = find(key);
        if (present(fr))
        {
            m_data[fr.d_idx].m_value = value;
            return m_data[fr.d_idx];
        }
        else
        {
            if (m_hashes.size() == 0)
            {
                rehash(8);
                fr.h_idx = key % m_hashes.size();
            }
			Entry entry;
			entry.m_key = key;
			entry.m_value = value;
			entry.m_next = m_hashes[fr.h_idx];
			m_data.push_back(entry);

            m_hashes[fr.h_idx] = m_data.size() - 1;
            if (load_factor() > 0.7f) rehash(m_hashes.size()*2);
			return m_data.back();
        }
    }

    template<typename T>
    float HashMap<T>::load_factor() const
    {
        return m_hashes.size() == 0 ? 0 : float(m_data.size()) / float(m_hashes.size());
    }

    template<typename T>
    uint32 HashMap<T>::collision_count() const
    {
        uint32 counter = 0;
        for (auto& v : m_data)
        {
            if (v.m_next != END_INDEX) counter++;
        }
        return counter;
    }

    template<typename T>
    typename HashMap<T>::Entry* HashMap<T>::begin()
    {
        return reinterpret_cast<arc::HashMap<T>::Entry*>(arc::begin(m_data));
    }

    template<typename T>
    typename HashMap<T>::Entry* HashMap<T>::end()
    {
        return reinterpret_cast<arc::HashMap<T>::Entry*>(arc::end(m_data));
    }

    template<typename T>
    const typename HashMap<T>::Entry* HashMap<T>::begin() const
    {
        return reinterpret_cast<arc::HashMap<T>::Entry*>(arc::begin(m_data));
    }

    template<typename T>
    const typename HashMap<T>::Entry* HashMap<T>::end() const
    {
        return reinterpret_cast<arc::HashMap<T>::Entry*>(arc::end(m_data));
    }

    template<typename T>
    void HashMap<T>::rehash(uint32 new_size)
    {
        ARC_ASSERT(new_size != 0, "Can't rehash to zero size");

        // resize hash storage
        m_hashes.resize(new_size);
        for (auto& h : m_hashes) h = END_INDEX;
        for (auto& v : m_data) v.m_next = END_INDEX;

        // reinsert values
        for (uint32 i=0; i<m_data.size(); i++)
        {
            auto key = m_data[i].m_key;
            auto h_idx = key % new_size;
            if (m_hashes[(uint32)h_idx] == END_INDEX)
            {
				m_hashes[(uint32)h_idx] = i;
                continue;
            }
			auto d_idx = m_hashes[(uint32)h_idx];
            while (m_data[d_idx].m_next != END_INDEX)
            {
                d_idx = m_data[d_idx].m_next;
            }
            m_data[d_idx].m_next = i;
        }
    }

	template<typename T>
	typename HashMap<T>::Entry* HashMap<T>::lookup(uint64 key)
	{
		auto fr = find(key);
		if (fr.d_idx == END_INDEX)
			return nullptr;
		else
			return &m_data[fr.d_idx];
	}

	template<typename T>
	typename const HashMap<T>::Entry* HashMap<T>::lookup(uint64 key) const
	{
		auto fr = find(key);
		if (fr.d_idx == END_INDEX)
			return nullptr;
		else
			return &m_data[fr.d_idx];
	}

	template<typename T>
	typename HashMap<T>::Entry& HashMap<T>::get(uint64 key, T&& fallback_init)
	{
		auto fr = find(key);
		if (present(fr))
		{
			return m_data[fr.d_idx];
		}
		else
		{
			if (m_hashes.size() == 0)
			{
				rehash(8);
				fr.h_idx = key % m_hashes.size();
			}

			Entry entry(key, std::forward<T>(fallback_init), m_hashes[fr.h_idx]);
			m_data.push_back(std::move(entry));
			m_hashes[fr.h_idx] = m_data.size() - 1;

			if (load_factor() > 0.7f) rehash(m_hashes.size() * 2);
			return m_data.back();
		}
	}

	template<typename T>
	typename HashMap<T>::Entry& HashMap<T>::get(uint64 key, T&& fallback_init, bool& o_fallback_used)
	{
		auto fr = find(key);
		if (present(fr))
		{
			o_fallback_used = false;
			return m_data[fr.d_idx];
		}
		else
		{
			o_fallback_used = true;
			if (m_hashes.size() == 0)
			{
				rehash(8);
				fr.h_idx = key % m_hashes.size();
			}

			Entry entry(key, std::forward<T>(fallback_init), m_hashes[fr.h_idx]);
			m_data.push_back(std::move(entry));
			m_hashes[fr.h_idx] = m_data.size() - 1;

			if (load_factor() > 0.7f) rehash(m_hashes.size() * 2);
			return m_data.back();
		}
	}

	template<typename T>
	typename HashMap<T>::Entry& HashMap<T>::get(uint64 key, const T& fallback_init)
	{
		auto fr = find(key);
		if (present(fr))
		{
			return m_data[fr.d_idx];
		}
		else
		{
			if (m_hashes.size() == 0)
			{
				rehash(8);
				fr.h_idx = key % m_hashes.size();
			}


			Entry entry(key, fallback_init, m_hashes[fr.h_idx]);
			m_data.push_back(std::move(entry));
			m_hashes[fr.h_idx] = m_data.size() - 1;

			if (load_factor() > 0.7f) rehash(m_hashes.size() * 2);
			return m_data.back();
		}
	}

	template<typename T>
	typename HashMap<T>::Entry& HashMap<T>::get(uint64 key, const T& fallback_init, bool& o_fallback_used)
	{
		auto fr = find(key);
		if (present(fr))
		{
			o_fallback_used = false;
			return m_data[fr.d_idx];
		}
		else
		{
			o_fallback_used = true;
			if (m_hashes.size() == 0)
			{
				rehash(8);
				fr.h_idx = key % m_hashes.size();
			}

			Entry entry(key, fallback_init, m_hashes[fr.h_idx]);
			m_data.push_back(std::move(entry));
			m_hashes[fr.h_idx] = m_data.size() - 1;

			if (load_factor() > 0.7f) rehash(m_hashes.size() * 2);
			return m_data.back();
		}
	}

} // namespace arc
