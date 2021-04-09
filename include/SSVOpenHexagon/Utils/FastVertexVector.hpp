// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <SSVUtils/Core/Common/LikelyUnlikely.hpp>

#include <cstddef>
#include <cstring>
#include <memory>

namespace hg::Utils {

template <sf::PrimitiveType TPrimitive>
struct FastVertexVector : public sf::Drawable
{
private:
    union VertexUnion
    {
        // To avoid invoking the default constructor of `sf::Vertex` when
        // resizing the dynamic array.
        sf::Vertex _v;

        VertexUnion()
        {}
    };

    static_assert(sizeof(VertexUnion) == sizeof(sf::Vertex));
    static_assert(alignof(VertexUnion) == alignof(sf::Vertex));

    std::unique_ptr<VertexUnion[]> _data{nullptr};
    std::size_t _size{};
    std::size_t _capacity{};

public:
    [[gnu::always_inline]] void reserve_more(const std::size_t n)
    {
        reserve(_size * 2 + n);
    }

    void reserve(const std::size_t n)
    {
        if(SSVU_LIKELY(_capacity >= n))
        {
            return;
        }

        auto new_data = std::make_unique<VertexUnion[]>(n);

        if(SSVU_UNLIKELY(_data != nullptr))
        {
            std::memcpy(
                new_data.get(), _data.get(), sizeof(sf::Vertex) * _size);
        }
        else
        {
            SSVOH_ASSERT(_size == 0);
            SSVOH_ASSERT(_capacity == 0);
        }

        _data = std::move(new_data);
        _capacity = n;
    }

    [[gnu::always_inline]] void unsafe_emplace_other(
        const FastVertexVector& rhs) noexcept
    {
        SSVOH_ASSERT(_size + rhs._size <= _capacity);

        if(SSVU_UNLIKELY(rhs.size() == 0))
        {
            return;
        }

        SSVOH_ASSERT(_data != nullptr);

        std::memcpy(_data.get() + _size, rhs._data.get(),
            sizeof(sf::Vertex) * rhs._size);

        _size += rhs._size;
    }

    [[gnu::always_inline]] void clear() noexcept
    {
        _size = 0;
    }

    [[gnu::always_inline, nodiscard]] std::size_t size() const noexcept
    {
        return _size;
    }

    template <typename... Ts>
    [[gnu::always_inline]] void unsafe_emplace_back(Ts&&... xs)
    {
        SSVOH_ASSERT(_size <= _capacity);
        SSVOH_ASSERT(_data != nullptr);

        new(&_data[_size++]._v) sf::Vertex{std::forward<Ts>(xs)...};
    }

    template <typename... Ts>
    [[gnu::always_inline]] void batch_unsafe_emplace_back(
        const sf::Color& color, Ts&&... positions)
    {
        SSVOH_ASSERT(_size + sizeof...(positions) <= _capacity);
        SSVOH_ASSERT(_data != nullptr);

        ((new(&_data[_size++]._v) sf::Vertex{positions, color}), ...);
    }

    void draw(sf::RenderTarget& mRenderTarget,
        sf::RenderStates mRenderStates) const override
    {
        if(SSVU_UNLIKELY(_data == nullptr))
        {
            SSVOH_ASSERT(_size == 0);
            SSVOH_ASSERT(_capacity == 0);
            return;
        }

        // UB:
        mRenderTarget.draw(reinterpret_cast<sf::Vertex*>(_data.get()), _size,
            TPrimitive, mRenderStates);
    }

    [[gnu::always_inline, nodiscard]] sf::Vertex& operator[](
        const std::size_t i) noexcept
    {
        SSVOH_ASSERT(i < _size);
        SSVOH_ASSERT(_data != nullptr);

        return _data[i]._v;
    }

    [[gnu::always_inline, nodiscard]] const sf::Vertex& operator[](
        const std::size_t i) const noexcept
    {
        SSVOH_ASSERT(i < _size);
        SSVOH_ASSERT(_data != nullptr);

        return _data[i]._v;
    }
};

class FastVertexVectorTris
    : public FastVertexVector<sf::PrimitiveType::Triangles>
{};

class FastVertexVectorQuads : public FastVertexVector<sf::PrimitiveType::Quads>
{};

} // namespace hg::Utils
