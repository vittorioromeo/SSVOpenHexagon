
template <typename F>
void forAliveCustomWallHandles(F&& f)
{
    for(CCustomWallHandle h = 0; h < (int)_customWalls.size(); ++h)
    {
        if(!_handleAvailable[h])
        {
            f(h);
        }
    }
}

template <typename F>
void forDeadCustomWallHandles(F&& f)
{
    for(CCustomWallHandle h = 0; h < (int)_customWalls.size(); ++h)
    {
        if(_handleAvailable[h])
        {
            f(h);
        }
    }
}

template <typename F>
void forCustomWalls(F&& f)
{
    for(CCustomWallHandle h = 0; h < (int)_customWalls.size(); ++h)
    {
        if(!_handleAvailable[h])
        {
            f(_customWalls[h]);
        }
    }
}

template <typename F>
[[nodiscard]] bool anyCustomWall(F&& f)
{
    for(CCustomWallHandle h = 0; h < (int)_customWalls.size(); ++h)
    {
        if(!_handleAvailable[h])
        {
            if(f(_customWalls[h]))
            {
                return true;
            }
        }
    }

    return false;
}

void CCustomWallManager::destroyAllOutOfBounds(const sf::Vector2f& bounds)
{
    for(CCustomWallHandle h = 0; h < (int)_customWalls.size(); ++h)
    {
        if(_handleAvailable[h])
        {
            continue;
        }

        const CCustomWall& cw = _customWalls[h];

        const auto vertexOutOfBounds = [&cw, &bounds](const int vertexIdx) {
            const sf::Vector2f& vertex = cw.getVertexPos(vertexIdx);

            return vertex.x < -bounds.x || vertex.x > bounds.x ||
                   vertex.y < -bounds.y || vertex.y > bounds.y;
        };

        if(vertexOutOfBounds(0) || vertexOutOfBounds(1) ||
            vertexOutOfBounds(2) || vertexOutOfBounds(3))
        {
            destroyUnchecked(h);
        }
    }
}

addLuaFn("cw_destroyAllOutOfBounds", //
    [this](float x, float y) {
        cwManager.destroyAllOutOfBounds({x, y});
    })
    .arg("boundsX")
    .arg("boundsY")
    .doc(
        "Destroy all custom walls whose any vertex resides outside the "
        "boundary region `{-$0, -$1};{$0, $1}`.");

addLuaFn("cw_forAliveHandles", //
    [this](const std::string& code) {
        static std::string buf;

        buf.clear();
        buf.reserve((code.size() + 1) * cwManager.count());

        cwManager.forAliveCustomWallHandles([&code](CCustomWallHandle cwh) {
            for(const char c : code)
            {
                if(c != '$')
                {
                    buf += c;
                    continue;
                }

                CCustomWallHandle h = cwh;
                const unsigned int nDigits = getNumDigits(h);
                const std::size_t lastIdx = buf.size();
                buf.resize(lastIdx + nDigits);

                for(unsigned int i = 0; i < nDigits; ++i)
                {
                    buf[lastIdx + nDigits - 1 - i] = (h % 10) + '0';
                    h /= 10;
                }
            }

            buf += '\n';
        });

        Utils::runLuaCode(lua, buf);
    })
    .arg("code")
    .doc("TODO");

addLuaFn("cw_forDeadHandles", //
    [this](const std::string& code) {
        static std::string buf;

        buf.clear();
        buf.reserve((code.size() + 1) * cwManager.count());

        cwManager.forDeadCustomWallHandles([&code](CCustomWallHandle cwh) {
            for(const char c : code)
            {
                if(c != '$')
                {
                    buf += c;
                    continue;
                }

                CCustomWallHandle h = cwh;
                const unsigned int nDigits = getNumDigits(h);
                const std::size_t lastIdx = buf.size();
                buf.resize(lastIdx + nDigits);

                for(unsigned int i = 0; i < nDigits; ++i)
                {
                    buf[lastIdx + nDigits - 1 - i] = (h % 10) + '0';
                    h /= 10;
                }
            }

            buf += '\n';
        });

        Utils::runLuaCode(lua, buf);
    })
    .arg("code")
    .doc("TODO");

[[nodiscard, gnu::always_inline]] constexpr inline unsigned int countDigits(
    CCustomWallHandle x)
{
    unsigned int i = 1;
    while((x /= 10) && ++i)
        ;

    return i;
}

// clang-format off
[[nodiscard, gnu::always_inline]] constexpr inline unsigned int getNumDigits(
    CCustomWallHandle x)
{
    return
        x < 10          ? 1u  :
        x < 100         ? 2u  :
        x < 1000        ? 3u  :
        x < 10000       ? 4u  :
        x < 100000      ? 5u  :
        x < 1000000     ? 6u  :
        x < 10000000    ? 7u  :
        x < 100000000   ? 8u  :
        x < 1000000000  ? 9u  : countDigits(x);
}
// clang-format on
