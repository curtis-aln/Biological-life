#pragma once

#include <vector>
#include <numeric>


template <class E>
class o_vector
{
	// this vector contains all of the items and is never directly modified
	std::vector<E> vector;

	std::vector<unsigned> avalableIndexes;
	std::vector<unsigned> unavalableIndexes;

	unsigned int m_maxItems;


public:
	explicit o_vector(const unsigned int maxItems = 0) : m_maxItems(maxItems)
	{
		avalableIndexes.resize(maxItems, 0);
		std::iota(avalableIndexes.begin(), avalableIndexes.end(), 0);
		unavalableIndexes.reserve(maxItems);
	}


	[[nodiscard]] unsigned int size() const { return static_cast<unsigned>(avalableIndexes.size()); }


	void reserve(const unsigned int maxItems)
	{
		m_maxItems = maxItems;
		vector.reserve(maxItems);
	}


	void emplace(E item) // only used to initilise a vector
	{
		vector.emplace_back(item);
	}


	E* add()
	{
		if (size() >= vector.size())
			return nullptr;

		const unsigned newIndex = unavalableIndexes.back();
		unavalableIndexes.pop_back();
		avalableIndexes.push_back(newIndex);

		return &vector[newIndex];
	}

	std::vector<unsigned>& getAvalableIndexes()
	{
		return avalableIndexes;
	}


	E& at(unsigned int index)
	{
		return vector[index];
	}


	void remove(const unsigned index)
	{
		avalableIndexes.erase(std::ranges::remove(avalableIndexes, index).begin(), avalableIndexes.end());
		unavalableIndexes.push_back(index);
	}
};