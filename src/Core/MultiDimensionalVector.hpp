#pragma once

#include <algorithm>
#include <array>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace IE::Core {
template<typename StoredType>
class MultiDimensionalVector {
private:
    std::vector<StoredType> m_data;
    std::vector<size_t>     m_dimensions;

public:
    template<typename... Args>
    explicit MultiDimensionalVector(Args... t_args) : m_data{}, m_dimensions() {
        m_dimensions.assign({t_args...});
        size_t size{!m_dimensions.empty()};
        ((size *= t_args), ...);
        m_data.resize(size, {});
    }

    explicit MultiDimensionalVector<StoredType>(std::vector<size_t> t_dimensions) :
            m_data{},
            m_dimensions(std::move(t_dimensions)) {
        size_t i;
        m_data.resize(
          std::accumulate(
            m_dimensions.cbegin(),
            m_dimensions.cend(),
            !m_dimensions.empty(),
            [&](size_t result, const int &) { return result + m_dimensions[i++]; }
          ),
          {}
        );
    }

    template<typename IndexType>
    IE::Core::MultiDimensionalVector<StoredType>
    operator[](std::initializer_list<std::initializer_list<IndexType>> t_points) {
        // Check parameter validity.
        if (t_points.size() != 2) {
            throw std::range_error(
              "Invalid range. Found " + std::to_string((long) t_points.size()) + " indices. Should be 2."
            );
        }
        if (t_points.begin()->size() != (t_points.end() - 1)->size())
            throw std::range_error("Dimensionality of indices do not match!");
        if (t_points.begin()->size() > m_dimensions.size())
            throw std::range_error("Dimensionality of input exceeds dimensionality of vector.");

        // Convert to vectors for ease of iteration and such
        std::vector<IndexType> point1(t_points.begin()->begin(), t_points.begin()->end());
        std::vector<IndexType> point2((t_points.end() - 1)->begin(), (t_points.end() - 1)->end());

        // Add 1 in front of the dimensions so that it can easily be used for length calculations
        std::vector<size_t> dimensions(m_dimensions.size());
        dimensions[0] = 1;
        size_t i;
        std::generate(dimensions.begin() + 1, dimensions.end(), [&] { return m_dimensions[i]; });

        // Generate the index locations
        i = -1;
        size_t index1{(size_t) std::accumulate(point1.cbegin(), point1.cend(), 0, [&](size_t result, const int &) {
            ++i;
            return result + point1[i] * dimensions[i];
        })};
        i = -1;
        size_t index2{(size_t) std::accumulate(point2.cbegin(), point2.cend(), 0, [&](size_t result, const int &) {
            ++i;
            return result + point2[i] * dimensions[i];
        })};

        // Generate dimensions of the new multidimensional array
        i = 0;
        dimensions.resize(point1.size());
        std::generate(dimensions.begin(), dimensions.end(), [&] { return point2[i] - point1[i] + 1; });

        // Generate, fill, and return the new multidimensional array
        auto *result = new IE::Core::MultiDimensionalVector<StoredType>(dimensions);
        result->m_data.assign(m_data.begin() + index1, m_data.begin() + index2);
        return *result;
    };

    template<typename IndexType>
    StoredType &operator[](std::initializer_list<IndexType> point) {
        // Convert the point to a vector for indexing purposes
        std::vector<IndexType> point1(point.begin(), point.end());

        // Add 1 in front of the dimensions so that it can easily be used for length calculations
        std::vector<size_t> dimensions(m_dimensions.size());
        dimensions[0] = 1;
        size_t i;
        std::generate(dimensions.begin() + 1, dimensions.end(), [&] { return m_dimensions[i]; });

        // Find index and return data
        return m_data[(size_t) std::accumulate(point1.cbegin(), point1.cend(), 0, [&](int, const int &) {
            return point1[i] * dimensions[i];
        })];
    }

    [[nodiscard]] bool empty() const {
        return m_data.empty();
    }

    IE::Core::MultiDimensionalVector<StoredType> &
    operator=(const IE::Core::MultiDimensionalVector<StoredType> &t_other) {
        m_dimensions = t_other.m_dimensions;
        m_data       = t_other.m_data;
        return *this;
    }

    IE::Core::MultiDimensionalVector<StoredType> &operator=(StoredType *t_other) {
        size_t size{std::accumulate(m_dimensions.begin(), m_dimensions.end(), 1u, std::multiplies())};
        assign(t_other, size);
        return *this;
    }

    void clear() {
        m_data.clear();
        m_dimensions.clear();
    }

    StoredType *data() const {
        return const_cast<unsigned char *const>(m_data.data());
    }

    [[nodiscard]] std::vector<size_t> getDimensions() const {
        return m_dimensions;
    }

    [[nodiscard]] size_t getDimensionality() const {
        return m_dimensions.size();
    }

    [[nodiscard]] size_t size() const {
        return m_data.size();
    }

    void assign(StoredType *t_position, size_t t_length) {
        if (m_data.size() != t_length) m_dimensions = {t_length};
        m_data.assign(t_position, (StoredType *) (size_t) t_position + t_length);
    }

    template<typename... Args>
    void resize(Args... t_args) {
        m_dimensions.assign({t_args...});
        size_t i{1};
        ((i *= t_args), ...);
        m_data.resize(i);
    }

    void resize(const std::vector<size_t> &t_dimensions) {
        m_dimensions = t_dimensions;
        m_data.resize(std::accumulate(t_dimensions.begin(), t_dimensions.end(), 1, std::multiplies()));
    }
};
}  // namespace IE::Core