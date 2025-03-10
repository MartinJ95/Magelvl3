#pragma once
#include <array>
#include <assert.h>

template <typename T, size_t S>
class TFixedArray
{
public:
	TFixedArray(const T& Val) :
		m_Data{Val}, m_size(0)
	{}
	TFixedArray(const TFixedArray<T, S>& Other)
	{
		for (int i = 0; i < m_Data.size(); i++)
		{
			m_Data[i] = Other.m_Data[i];
		}
		m_size = Other.m_size;
	}
	TFixedArray(TFixedArray<T, S>&& Other)
	{
		for (int i = 0; i < m_Data.size(); i++)
		{
			m_Data[i] = std::move(Other.m_Data[i]);
		}
		m_size = Other.m_size;
	}
	void operator=(const TFixedArray<T, S>& Other)
	{
		for (int i = 0; i < m_Data.size(); i++)
		{
			m_Data[i] = Other.m_Data[i];
		}
		m_size = Other.m_size;
	}
	void operator=(TFixedArray<T, S>&& Other)
	{
		for (int i = 0; i < m_Data.size(); i++)
		{
			m_Data[i] = std::move(Other.m_Data[i]);
		}
		m_size = Other.m_size;
	}
	void Add(const T& Val);
	void Remove(const T& Val);
	unsigned int Size() const
	{
		return m_size;
	}
	T& operator[](int Index)
	{
		assert(Index >= 0 && Index < m_size);
		return m_Data[Index];
	}
protected:
	std::array<T, S> m_Data;
	unsigned int m_size;
};

template<typename T, size_t S>
inline void TFixedArray<T, S>::Add(const T& Val)
{

	assert(m_size+1 < S);

	m_Data[m_size] = Val;
	m_size++;
}

template<typename T, size_t S>
inline void TFixedArray<T, S>::Remove(const T& Val)
{
	for (int i = 0; i < m_size; i++)
	{
		if (m_data[i] != Val)
			continue;

		if (i < m_size-1)
			std::swap(m_data[i], m_data[m_size-1]);

		m_size--;
		return;
	}
}
