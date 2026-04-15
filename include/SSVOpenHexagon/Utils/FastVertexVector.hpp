// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Macros.hpp"
#include "SSVOpenHexagon/Utils/UniquePtrArray.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/SizeT.hpp"

#include <cstring>

namespace hg::Utils
{

template <sf::PrimitiveType TPrimitive>
struct FastVertexVector
{
private:
    union VertexUnion
    {
        // To avoid invoking the default constructor of `sf::Vertex` when
        // resizing the dynamic array.
        sf::Vertex _v;

        VertexUnion()
        {
        }
    };

    static_assert(sizeof(VertexUnion) == sizeof(sf::Vertex));
    static_assert(alignof(VertexUnion) == alignof(sf::Vertex));

    Utils::UniquePtrArray<VertexUnion> _data{nullptr};
    sf::base::SizeT                    _size{};
    sf::base::SizeT                    _capacity{};

public:
    [[gnu::always_inline, gnu::flatten]] void reserve_more(const sf::base::SizeT n)
    {
        reserve(_size * 2 + n);
    }

    void reserve(const sf::base::SizeT n)
    {
        if (_capacity >= n) [[likely]]
        {
            return;
        }

        auto new_data = Utils::makeUniqueArray<VertexUnion>(n);

        if (_data != nullptr) [[unlikely]]
        {
            std::memcpy(new_data.get(), _data.get(), sizeof(sf::Vertex) * _size);
        }
        else
        {
            SSVOH_ASSERT(_size == 0);
            SSVOH_ASSERT(_capacity == 0);
        }

        _data     = SSVOH_MOVE(new_data);
        _capacity = n;
    }

    [[gnu::always_inline, gnu::flatten]] void unsafe_emplace_other(const FastVertexVector& rhs) noexcept
    {
        SSVOH_ASSERT(_size + rhs._size <= _capacity);

        if (rhs.size() == 0) [[unlikely]]
        {
            return;
        }

        SSVOH_ASSERT(_data != nullptr);

        std::memcpy(_data.get() + _size, rhs._data.get(), sizeof(sf::Vertex) * rhs._size);

        _size += rhs._size;
    }

    [[gnu::always_inline]] void clear() noexcept
    {
        _size = 0;
    }

    [[nodiscard, gnu::always_inline]] sf::base::SizeT size() const noexcept
    {
        return _size;
    }

    template <typename... Ts>
    [[gnu::always_inline, gnu::flatten]] void unsafe_emplace_back(Ts&&... xs)
    {
        SSVOH_ASSERT(_size <= _capacity);
        SSVOH_ASSERT(_data != nullptr);

        SFML_BASE_PLACEMENT_NEW(&_data[_size++]._v)
        sf::Vertex{SSVOH_FWD(xs)...};
    }

    template <typename... Ts>
    [[gnu::always_inline, gnu::flatten]] void batch_unsafe_emplace_back(const sf::Color& color, Ts&&... positions)
    {
        SSVOH_ASSERT(_size + sizeof...(positions) <= _capacity);
        SSVOH_ASSERT(_data != nullptr);

        ((SFML_BASE_PLACEMENT_NEW(&_data[_size++]._v) sf::Vertex{positions, color}), ...);
    }

    void draw(sf::RenderTarget& mRenderTarget, sf::RenderStates mRenderStates) const
    {
        if (_data == nullptr) [[unlikely]]
        {
            SSVOH_ASSERT(_size == 0);
            SSVOH_ASSERT(_capacity == 0);
            return;
        }

        mRenderTarget.drawVertices(
            {
                .vertexSpan    = {reinterpret_cast<const sf::Vertex*>(_data.get()), _size},
                .primitiveType = TPrimitive,
            },
            mRenderStates);
    }

    [[nodiscard, gnu::always_inline]] sf::Vertex& operator[](const sf::base::SizeT i) noexcept
    {
        SSVOH_ASSERT(i < _size);
        SSVOH_ASSERT(_data != nullptr);

        return _data[i]._v;
    }

    [[nodiscard, gnu::always_inline]] const sf::Vertex& operator[](const sf::base::SizeT i) const noexcept
    {
        SSVOH_ASSERT(i < _size);
        SSVOH_ASSERT(_data != nullptr);

        return _data[i]._v;
    }

    [[nodiscard, gnu::always_inline]] sf::Vertex* begin() noexcept
    {
        SSVOH_ASSERT(_data != nullptr);
        return &(_data[0]._v);
    }

    [[nodiscard, gnu::always_inline]] const sf::Vertex* begin() const noexcept
    {
        SSVOH_ASSERT(_data != nullptr);
        return &(_data[0]._v);
    }

    [[nodiscard, gnu::always_inline]] sf::Vertex* end() noexcept
    {
        return begin() + _size;
    }

    [[nodiscard, gnu::always_inline]] const sf::Vertex* end() const noexcept
    {
        return begin() + _size;
    }
};

class FastVertexVectorTris : public FastVertexVector<sf::PrimitiveType::Triangles>
{
public:
    [[gnu::always_inline, gnu::flatten]] void batch_unsafe_emplace_back_quad(
        const sf::Color& color,
        const sf::Vec2f  nw,
        const sf::Vec2f  sw,
        const sf::Vec2f  se,
        const sf::Vec2f  ne)
    {
        batch_unsafe_emplace_back(color, //
                                  nw,
                                  sw,
                                  se, //
                                  nw,
                                  se,
                                  ne);
    }

    [[gnu::always_inline, gnu::flatten]] void unsafe_emplace_back_quad( //
        const sf::Vec2f  nw,
        const sf::Color& colorNW, //
        const sf::Vec2f  sw,
        const sf::Color& colorSW, //
        const sf::Vec2f  se,
        const sf::Color& colorSE, //
        const sf::Vec2f  ne,
        const sf::Color& colorNE)
    {
        unsafe_emplace_back(nw, colorNW);
        unsafe_emplace_back(sw, colorSW);
        unsafe_emplace_back(se, colorSE);
        unsafe_emplace_back(nw, colorNW);
        unsafe_emplace_back(se, colorSE);
        unsafe_emplace_back(ne, colorNE);
    }

    [[gnu::always_inline, gnu::flatten]] void reserve_more_quad(const sf::base::SizeT n)
    {
        reserve_more(n * 6);
    }

    [[gnu::always_inline]] void reserve_quad(const sf::base::SizeT n)
    {
        reserve(n * 6);
    }
};

} // namespace hg::Utils
