// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/LuaScripting.hpp"

#include "SSVOpenHexagon/Global/Assert.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"
#include "SSVOpenHexagon/Utils/LuaWrapper.hpp"
#include "SSVOpenHexagon/Utils/LuaMetadata.hpp"
#include "SSVOpenHexagon/Utils/LuaMetadataProxy.hpp"
#include "SSVOpenHexagon/Utils/ScopeGuard.hpp"
#include "SSVOpenHexagon/Core/RandomNumberGenerator.hpp"
#include "SSVOpenHexagon/Components/CCustomWallHandle.hpp"
#include "SSVOpenHexagon/Components/CCustomWallManager.hpp"
#include "SSVOpenHexagon/Utils/TypeWrapper.hpp"

#include <SSVUtils/Core/Log/Log.hpp>

#include <sstream>
#include <tuple>
#include <vector>
#include <string>
#include <cstddef>

namespace hg::LuaScripting {

template <typename F>
Utils::LuaMetadataProxy addLuaFn(
    Lua::LuaContext& lua, const std::string& name, F&& f)
{
    // TODO: does this handle duplicates properly? Both menu and game call the
    // same thing.

    lua.writeVariable(name, std::forward<F>(f));
    return Utils::LuaMetadataProxy{
        Utils::TypeWrapper<F>{}, getMetadata(), name};
}

static void initRandom(Lua::LuaContext& lua, random_number_generator& rng)
{
    const auto rndReal = [&rng]() -> float
    { return rng.get_real<float>(0, 1); };

    const auto rndIntUpper = [&rng](int upper) -> float
    { return rng.get_int<int>(1, upper); };

    const auto rndInt = [&rng](int lower, int upper) -> float
    { return rng.get_int<int>(lower, upper); };

    addLuaFn(lua, "u_rndReal", rndReal)
        .doc("Return a random real number in the [0; 1] range.");

    addLuaFn(lua, "u_rndIntUpper", rndIntUpper)
        .arg("upper")
        .doc("Return a random integer number in the [1; `$0`] range.");

    addLuaFn(lua, "u_rndInt", rndInt)
        .arg("lower")
        .arg("upper")
        .doc("Return a random integer number in the [`$0`; `$1`] range.");

    addLuaFn(lua, "u_rndSwitch",
        [rndReal, rndIntUpper, rndInt](int mode, int lower, int upper) -> float
        {
            if(mode == 0)
            {
                return rndReal();
            }

            if(mode == 1)
            {
                return rndIntUpper(upper);
            }

            if(mode == 2)
            {
                return rndInt(lower, upper);
            }

            SSVOH_ASSERT(false);
            return 0;
        })
        .arg("mode")
        .arg("lower")
        .arg("upper")
        .doc(
            "Internal replacement for `math.random`. Calls `u_rndReal()` with "
            "`$0 == 0`, `u_rndUpper($2)` with `$0 == 1`, and `u_rndInt($1, "
            "$2)` with `$0 == 2`.");

    // ------------------------------------------------------------------------
    // Register Lua function to get random seed for the current attempt:
    addLuaFn(lua, "u_getAttemptRandomSeed", //
        [&rng] { return rng.seed(); })
        .doc(
            "Obtain the current random seed, automatically generated at the "
            "beginning of the level. `math.randomseed` is automatically "
            "initialized with the result of this function at the beginning of "
            "a level.");

    // ------------------------------------------------------------------------
    // Initialize Lua random seed from random generator one:
    try
    {
        // TODO: likely not needed anymore
        lua.executeCode("math.randomseed(u_getAttemptRandomSeed())\n");
    }
    catch(...)
    {
        ssvu::lo("hg::LuaScripting::initRandom")
            << "Failure to initialize Lua random generator seed\n";
    }
}

static void redefineIoOpen(Lua::LuaContext& lua)
try
{
    lua.executeCode(
        "local open = io.open; io.open = function(filename) return "
        "open(filename, \"r\"); end");
}
catch(...)
{
    ssvu::lo("hg::LuaScripting::redefineIoOpen")
        << "Failure to redefine Lua's `io.open` function\n";

    throw;
}

static void redefineRandom(Lua::LuaContext& lua)
try
{
    lua.executeCode(R"(math.random = function(a, b)
    if a == nil and b == nil then
        return u_rndSwitch(0, 0, 0)
    elseif b == nil then
        return u_rndSwitch(1, 0, a)
    else
        return u_rndSwitch(2, a, b)
    end
end
)");
}
catch(...)
{
    ssvu::lo("hg::LuaScripting::redefineRandom")
        << "Failure to redefine Lua's `math.random` function\n";

    throw;
}

static void destroyMaliciousFunctions(Lua::LuaContext& lua)
{
    // This destroys the "os" library completely. This library is capable of
    // file manipulation, running shell commands, and messing up the replay
    // system completely. os.execute(), one of the functions in this library,
    // can be used to create malware and is capable of destroying computers.
    lua.clearVariable("os");

    // This destroys some of the "io" functions completely.
    // This library is dedicated to manipulating files and their contents,
    // which can be used maliciously.
    lua.clearVariable("io.popen");
    lua.clearVariable("io.flush");
    lua.clearVariable("io.write");
    lua.clearVariable("io.setvbuf");

    // This destroys the "debug" library completely. The debug library is next
    // to useless in Open Hexagon (considering we have our own methods of
    // debugging), and it allows people to access destroyed modules with the
    // getregistry function.
    lua.clearVariable("debug");

    // This function allows pack developers to set the seed in Lua.
    lua.clearVariable("math.randomseed");

    // These functions are being deleted as they can assist in restoring
    // destroyed modules. However, we cannot destroy the whole library as
    // the other functions are needed for the "require" function to work
    // properly.
    lua.clearVariable("package.loadlib");
    lua.clearVariable("package.searchpath");
}

static void initUtils(Lua::LuaContext& lua, const bool inMenu)
{
    addLuaFn(lua, "u_inMenu", //
        [inMenu] { return inMenu; })
        .doc(
            "Returns `true` if the script is being executed in the menu, "
            "`false` otherwise.");

    addLuaFn(lua, "u_getVersionMajor", //
        [] { return GAME_VERSION.major; })
        .doc("Returns the major of the current version of the game");

    addLuaFn(lua, "u_getVersionMinor", //
        [] { return GAME_VERSION.minor; })
        .doc("Returns the minor of the current version of the game");

    addLuaFn(lua, "u_getVersionMicro", //
        [] { return GAME_VERSION.micro; })
        .doc("Returns the micro of the current version of the game");

    addLuaFn(lua, "u_getVersionString", //
        []() -> std::string { return GAME_VERSION_STR; })
        .doc("Returns the string representing the current version of the game");
}

static void initCustomWalls(Lua::LuaContext& lua, CCustomWallManager& cwManager)
{
    addLuaFn(lua, "cw_create", //
        [&cwManager]() -> CCustomWallHandle
        { return cwManager.create([](CCustomWall&) {}); })
        .doc("Create a new custom wall and return a integer handle to it.");

    addLuaFn(lua, "cw_createDeadly", //
        [&cwManager]() -> CCustomWallHandle {
            return cwManager.create(
                [](CCustomWall& cw) { cw.setDeadly(true); });
        })
        .doc(
            "Create a new deadly custom wall and return a integer handle to "
            "it.");

    addLuaFn(lua, "cw_createNoCollision", //
        [&cwManager]() -> CCustomWallHandle {
            return cwManager.create(
                [](CCustomWall& cw) { cw.setCanCollide(false); });
        })
        .doc(
            "Create a new custom wall without collision and return a integer "
            "handle to it.");

    addLuaFn(lua, "cw_destroy", //
        [&cwManager](CCustomWallHandle cwHandle)
        { cwManager.destroy(cwHandle); })
        .arg("cwHandle")
        .doc("Destroy the custom wall represented by `$0`.");

    addLuaFn(lua, "cw_setVertexPos", //
        [&cwManager](
            CCustomWallHandle cwHandle, int vertexIndex, float x, float y) {
            cwManager.setVertexPos(cwHandle, vertexIndex, sf::Vector2f{x, y});
        })
        .arg("cwHandle")
        .arg("vertexIndex")
        .arg("x")
        .arg("y")
        .doc(
            "Given the custom wall represented by `$0`, set the position of "
            "its vertex with index `$1` to `{$2, $3}`.");

    addLuaFn(lua, "cw_moveVertexPos", //
        [&cwManager](
            CCustomWallHandle cwHandle, int vertexIndex, float x, float y) {
            cwManager.moveVertexPos(cwHandle, vertexIndex, sf::Vector2f{x, y});
        })
        .arg("cwHandle")
        .arg("vertexIndex")
        .arg("offsetX")
        .arg("offsetY")
        .doc(
            "Given the custom wall represented by `$0`, add `{$2, $3}` to the "
            "position of its vertex with index `$1`.");

    addLuaFn(lua, "cw_moveVertexPos4Same", //
        [&cwManager](CCustomWallHandle cwHandle, float x, float y) {
            cwManager.moveVertexPos4Same(cwHandle, sf::Vector2f{x, y});
        })
        .arg("cwHandle")
        .arg("offsetX")
        .arg("offsetY")
        .doc(
            "Given the custom wall represented by `$0`, add `{$1, $2}` to the "
            "position of its vertex with indices `0`, `1`, `2`, and `3`.");

    addLuaFn(lua, "cw_setVertexColor", //
        [&cwManager](CCustomWallHandle cwHandle, int vertexIndex, int r, int g,
            int b, int a) {
            cwManager.setVertexColor(
                cwHandle, vertexIndex, sf::Color(r, g, b, a));
        })
        .arg("cwHandle")
        .arg("vertexIndex")
        .arg("r")
        .arg("g")
        .arg("b")
        .arg("a")
        .doc(
            "Given the custom wall represented by `$0`, set the color of "
            "its vertex with index `$1` to `{$2, $3, $4, $5}`.");

    addLuaFn(lua, "cw_setVertexPos4",            //
        [&cwManager](CCustomWallHandle cwHandle, //
            float x0, float y0,                  //
            float x1, float y1,                  //
            float x2, float y2,                  //
            float x3, float y3)
        {
            cwManager.setVertexPos4(cwHandle, //
                sf::Vector2f{x0, y0},         //
                sf::Vector2f{x1, y1},         //
                sf::Vector2f{x2, y2},         //
                sf::Vector2f{x3, y3});
        })
        .arg("cwHandle")
        .arg("x0")
        .arg("y0")
        .arg("x1")
        .arg("y1")
        .arg("x2")
        .arg("y2")
        .arg("x3")
        .arg("y3")
        .doc(
            "Given the custom wall represented by `$0`, set the position of "
            "its vertex with index `0` to `{$1, $2}`, index `1` to `{$3, $4}`, "
            "index `2` to `{$5, $6}`, index `3` to `{$7, $8}`. More efficient "
            "than invoking `cw_setVertexPos` four times in a row.");

    addLuaFn(lua, "cw_setVertexColor4",          //
        [&cwManager](CCustomWallHandle cwHandle, //
            int r0, int g0, int b0, int a0,      //
            int r1, int g1, int b1, int a1,      //
            int r2, int g2, int b2, int a2,      //
            int r3, int g3, int b3, int a3)
        {
            cwManager.setVertexColor4(cwHandle, //
                sf::Color(r0, g0, b0, a0),      //
                sf::Color(r1, g1, b1, a1),      //
                sf::Color(r2, g2, b2, a2),      //
                sf::Color(r3, g3, b3, a3));
        })
        .arg("cwHandle")
        .arg("r0")
        .arg("g0")
        .arg("b0")
        .arg("a0")
        .arg("r1")
        .arg("g1")
        .arg("b1")
        .arg("a1")
        .arg("r2")
        .arg("g2")
        .arg("b2")
        .arg("a2")
        .arg("r3")
        .arg("g3")
        .arg("b3")
        .arg("a3")
        .doc(
            "Given the custom wall represented by `$0`, set the color of "
            "its vertex with index `0` to `{$1, $2, $3, $4}`, index `1` to "
            "`{$5, $6, $7, $8}`, index `2` to `{$9, $10, $11, $12}`, index `3` "
            "to `{$13, $14, $15, $16}`. More efficient than invoking "
            "`cw_setVertexColor` four times in a row.");

    addLuaFn(lua, "cw_setVertexColor4Same", //
        [&cwManager](CCustomWallHandle cwHandle, int r, int g, int b, int a)
        { cwManager.setVertexColor4Same(cwHandle, sf::Color(r, g, b, a)); })
        .arg("cwHandle")
        .arg("r")
        .arg("g")
        .arg("b")
        .arg("a")
        .doc(
            "Given the custom wall represented by `$0`, set the color of "
            "its vertices with indiced `0`, `1`, `2`, and `3' to `{$1, $2, $3, "
            "$4}`. More efficient than invoking `cw_setVertexColor` four times "
            "in a row.");

    addLuaFn(lua, "cw_setCollision", //
        [&cwManager](CCustomWallHandle cwHandle, bool collision)
        { cwManager.setCanCollide(cwHandle, collision); })
        .arg("cwHandle")
        .arg("collision")
        .doc(
            "Given the custom wall represented by `$0`, set the collision "
            "of the custom wall to `$1`. If false, the player cannot die "
            "from this wall and can move through the wall. By default, all "
            "custom walls can collide with the player.");

    addLuaFn(lua, "cw_setDeadly", //
        [&cwManager](CCustomWallHandle cwHandle, bool deadly)
        { cwManager.setDeadly(cwHandle, deadly); })
        .arg("cwHandle")
        .arg("deadly")
        .doc(
            "Given the custom wall represented by `$0`, set wherever "
            "it instantly kills player on touch. This is highly "
            "recommended for custom walls that are either very small "
            "or very thin and should definitively kill the player.");

    addLuaFn(lua, "cw_setKillingSide", //
        [&cwManager](CCustomWallHandle cwHandle, unsigned int side)
        { cwManager.setKillingSide(cwHandle, side); })
        .arg("cwHandle")
        .arg("side")
        .doc(
            "Given the custom wall represented by `$0`, set which "
            "one of its sides should beyond any doubt cause the "
            "death of the player. Acceptable values are `0` to `3`. "
            "In a standard wall, side `0` is the side closer to the center. "
            "This parameter is useless if the custom wall is deadly.");

    addLuaFn(lua, "cw_getCollision", //
        [&cwManager](CCustomWallHandle cwHandle) -> bool
        { return cwManager.getCanCollide(cwHandle); })
        .arg("cwHandle")
        .doc(
            "Given the custom wall represented by `$0`, get whether it can "
            "collide with player or not.");

    addLuaFn(lua, "cw_getDeadly", //
        [&cwManager](CCustomWallHandle cwHandle) -> bool
        { return cwManager.getDeadly(cwHandle); })
        .arg("cwHandle")
        .doc(
            "Given the custom wall represented by `$0`, get whether it "
            "instantly kills the player on touch or not.");

    addLuaFn(lua, "cw_getKillingSide", //
        [&cwManager](CCustomWallHandle cwHandle) -> unsigned int
        { return cwManager.getKillingSide(cwHandle); })
        .arg("cwHandle")
        .doc(
            "Given the custom wall represented by `$0`, get which one of its "
            "sides always causes the death of the player.");

    addLuaFn(lua, "cw_getVertexPos", //
        [&cwManager](CCustomWallHandle cwHandle,
            int vertexIndex) -> std::tuple<float, float>
        {
            const sf::Vector2f& pos =
                cwManager.getVertexPos(cwHandle, vertexIndex);

            return {pos.x, pos.y};
        })
        .arg("cwHandle")
        .arg("vertexIndex")
        .doc(
            "Given the custom wall represented by `$0`, return the position of "
            "its vertex with index `$1`.");

    addLuaFn(lua, "cw_getVertexPos4", //
        [&cwManager](
            CCustomWallHandle cwHandle) -> std::tuple<float, float, float,
                                            float, float, float, float, float>
        {
            const auto& [p0, p1, p2, p3] = cwManager.getVertexPos4(cwHandle);
            return {p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y};
        })
        .arg("cwHandle")
        .doc(
            "Given the custom wall represented by `$0`, return the position of "
            "its vertics with indices `0`, `1`, `2`, and `3`, as a tuple.");

    // TODO:
    /*
    addLuaFn(lua, "cw_isOverlappingPlayer", //
        [&cwManager](CCustomWallHandle cwHandle) -> bool {
            return cwManager.isOverlappingPlayer(cwHandle);
        })
        .arg("cwHandle")
        .doc(
            "Return `true` if the custom wall represented by `$0` is "
            "overlapping the player, `false` otherwise.");
    */

    addLuaFn(lua, "cw_clear", //
        [&cwManager] { cwManager.clear(); })
        .doc("Remove all existing custom walls.");
}

[[nodiscard]] Utils::LuaMetadata& getMetadata()
{
    static Utils::LuaMetadata lm;
    return lm;
}

void init(Lua::LuaContext& lua, random_number_generator& rng, const bool inMenu,
    CCustomWallManager& cwManager)
{
    initRandom(lua, rng);
    redefineIoOpen(lua);
    redefineRandom(lua);

    // ------------------------------------------------------------------------
    // Remove potentially malicious Lua functions, including `math.randomseed`:
    destroyMaliciousFunctions(lua);

    initUtils(lua, inMenu);
    initCustomWalls(lua, cwManager);
}

void printDocs()
{
    Utils::LuaMetadata& lm = getMetadata();

    for(std::size_t i = 0; i < lm.getNumCategories(); ++i)
    {
        std::cout << '\n' << lm.prefixHeaders.at(i) << "\n\n";

        lm.forFnEntries(
            [](const std::string& ret, const std::string& name,
                const std::string& args, const std::string& docs)
            {
                std::cout << "* **`" << ret << " " << name << "(" << args
                          << ")`**: " << docs << "\n\n";
            },
            i);
    }
}

const std::vector<std::string>& getAllFunctionNames()
{
    Utils::LuaMetadata& lm = getMetadata();

    static std::vector<std::string> result = [&]
    {
        std::vector<std::string> v;

        for(std::size_t i = 0; i < lm.getNumCategories(); ++i)
        {
            lm.forFnEntries([&](const std::string&, const std::string& name,
                                const std::string&, const std::string&)
                { v.emplace_back(name); },
                i);
        }

        return v;
    }();

    return result;
}

std::string getDocsForFunction(const std::string& fnName)
{
    Utils::LuaMetadata& lm = getMetadata();

    bool found = false;
    std::ostringstream oss;

    for(std::size_t i = 0; i < lm.getNumCategories(); ++i)
    {
        lm.forFnEntries(
            [&](const std::string& ret, const std::string& name,
                const std::string& args, const std::string& docs)
            {
                if(found || name != fnName)
                {
                    return; // basically a `continue`
                }

                found = true;

                oss << ret << " " << name << "(" << args << "):\n"
                    << docs << "\n\n";
            },
            i);
    }

    if(!found)
    {
        return "UNKNOWN FUNCTION";
    }

    return oss.str();
}

} // namespace hg::LuaScripting
