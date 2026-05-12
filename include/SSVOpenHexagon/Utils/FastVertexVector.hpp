// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawQuadsSettings.hpp"
#include "SFML/Graphics/DrawVerticesSettings.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


namespace hg::Utils
{

// `sf::Vertex` buffer for individual triangles. Drawn via `drawVertices` with
// `PrimitiveType::Triangles`, so callers must emit 3 vertices per triangle.
//
// Inherits from `sf::base::Vector<sf::Vertex>` and only adds the helpers that
// build on top of it (shared-color batch emit, append-from-other guarded
// against empty source, and the `draw()` shortcut).
class FastVertexVectorTris : public sf::base::Vector<sf::Vertex>
{
public:
    using Base = sf::base::Vector<sf::Vertex>;
    using Base::Base;

    template <typename... Ts>
    [[gnu::always_inline, gnu::flatten]] void batchUnsafeEmplaceBack(const sf::Color color, Ts&&... positions)
    {
        unsafePushBackMultiple(sf::Vertex{positions, color}...);
    }

    [[gnu::always_inline, gnu::flatten]] void unsafeAppend(const FastVertexVectorTris& other) noexcept
    {
        if (other.size() == 0u) [[unlikely]]
        {
            return;
        }

        unsafeEmplaceBackRange(other.data(), other.size());
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        if (size() == 0u) [[unlikely]]
        {
            return;
        }

        target.drawVertices(
            {
                .vertexSpan    = {data(), size()},
                .primitiveType = sf::PrimitiveType::Triangles,
            },
            states);
    }
};

// `sf::Vertex` buffer for quads. Stores 4 vertices per quad in the order
// expected by `RenderTarget::drawQuads` (precomputed index pattern
// `0,1,2,1,2,3`): the second and third vertex of each group of four form
// the shared diagonal. Quad-emit helpers take corners labeled
// (nw, sw, se, ne) and reorder them to (sw, nw, se, ne) so the rendered
// diagonal goes between `nw` and `se` -- matching what the old 6-vertex
// emit produced.
class FastVertexVectorQuads : public sf::base::Vector<sf::Vertex>
{
public:
    using Base = sf::base::Vector<sf::Vertex>;
    using Base::Base;

    [[gnu::always_inline]] void reserveQuad(const sf::base::SizeT n)
    {
        reserve(n * 4u);
    }

    [[gnu::always_inline, gnu::flatten]] void reserveMoreQuad(const sf::base::SizeT n)
    {
        reserveMore(n * 4u);
    }

    [[gnu::always_inline, gnu::flatten]] void batchUnsafeEmplaceBackQuad(
        const sf::Color color,
        const sf::Vec2f nw,
        const sf::Vec2f sw,
        const sf::Vec2f se,
        const sf::Vec2f ne)
    {
        // Emit order: sw, nw, se, ne. With the `drawQuads` index pattern
        // `0,1,2,1,2,3`, this produces triangles (sw, nw, se) and
        // (nw, se, ne) -- same NW-SE diagonal as the legacy 6-vertex emit.
        unsafePushBackMultiple(sf::Vertex{sw, color}, sf::Vertex{nw, color}, sf::Vertex{se, color}, sf::Vertex{ne, color});
    }

    [[gnu::always_inline, gnu::flatten]] void unsafeEmplaceBackQuad( //
        const sf::Vec2f nw,
        const sf::Color colorNW, //
        const sf::Vec2f sw,
        const sf::Color colorSW, //
        const sf::Vec2f se,
        const sf::Color colorSE, //
        const sf::Vec2f ne,
        const sf::Color colorNE)
    {
        unsafeEmplaceBack(sw, colorSW);
        unsafeEmplaceBack(nw, colorNW);
        unsafeEmplaceBack(se, colorSE);
        unsafeEmplaceBack(ne, colorNE);
    }

    // See `FastVertexVectorTris::unsafeAppend` for the empty-source rationale.
    [[gnu::always_inline, gnu::flatten]] void unsafeAppend(const FastVertexVectorQuads& other) noexcept
    {
        if (other.size() == 0u) [[unlikely]]
        {
            return;
        }

        unsafeEmplaceBackRange(other.data(), other.size());
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        const sf::base::SizeT totalSize = size();
        if (totalSize == 0u) [[unlikely]]
        {
            return;
        }

        // `RenderTarget::drawQuads` is bounded by the size of its
        // precomputed quad-index array. Chunk the draw so we never
        // exceed that, regardless of buffer size.
        for (sf::base::SizeT offset = 0u; offset < totalSize;)
        {
            const sf::base::SizeT chunkSize = sf::base::min(totalSize - offset,
                                                            +sf::RenderTarget::drawQuadsMaxVerticesPerCall);

            target.drawQuads(
                {
                    .vertexSpan    = {data() + offset, chunkSize},
                    .primitiveType = sf::PrimitiveType::Triangles,
                },
                states);

            offset += chunkSize;
        }
    }
};

} // namespace hg::Utils
