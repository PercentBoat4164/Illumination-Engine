#pragma once

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

    explicit MultiDimensionalVector<StoredType>(std::vector<size_t> t_dimensions);

    template<typename IndexType>
    IE::Core::MultiDimensionalVector<StoredType>
    operator[](std::initializer_list<std::initializer_list<IndexType>> t_points);

    template<typename IndexType>
    StoredType &operator[](std::initializer_list<IndexType> point);

    [[nodiscard]] bool empty() const;

    IE::Core::MultiDimensionalVector<StoredType> &
    operator=(const IE::Core::MultiDimensionalVector<StoredType> &t_other);

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

    [[nodiscard]] std::vector<size_t> getDimensions() const;

    [[nodiscard]] size_t getDimensionality() const;

    [[nodiscard]] size_t size() const {
        return m_data.size();
    }

    void assign(StoredType *t_position, size_t t_length) {
        if (m_data.size() != t_length) m_dimensions = {t_length};
        m_data.assign(t_position, (StoredType *) (size_t) t_position + t_length);
    }

    template<typename... Args>
    void resize(Args... t_args);

    void resize(const std::vector<size_t> &t_dimensions);
};
}  // namespace IE::Core