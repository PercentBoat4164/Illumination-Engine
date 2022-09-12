#pragma once

#include <utility>
#include <vector>
#include <utility>
#include <string>
#include <numeric>
#include <stdexcept>
#include <array>

namespace IE::Core {
	template<typename StoredType>
	class MultiDimensionalVector {
	public:
		std::vector<StoredType> m_data;
		std::vector<size_t> m_dimensions;
		
		using iterator = typename std::vector<StoredType>::iterator;
		
		template<typename ...Args>
		explicit MultiDimensionalVector<StoredType>(Args... t_args);
		
		explicit MultiDimensionalVector<StoredType>(std::vector<size_t> t_dimensions);
		
		template<typename IndexType>
		IE::Core::MultiDimensionalVector<StoredType> operator[](std::initializer_list<std::initializer_list<IndexType>> t_points);
		
		template<typename IndexType>
		StoredType &operator[](std::initializer_list<IndexType> point);
		
		bool empty();
		
		IE::Core::MultiDimensionalVector<StoredType> &operator=(const IE::Core::MultiDimensionalVector<StoredType> &other);
		
		void clear();
	};
}