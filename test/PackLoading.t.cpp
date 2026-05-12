// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

// Regression test for pack folder-path handling.
//
// `HGAssets::loadPackAssets` builds subfolder paths via `packData.folderPath / "Levels"`,
// `/ "Styles"`, ..., using `sf::Path::operator/`. The asset loader expects each shipped
// pack to expose those subdirs at the resolved path.
//
// This test exercises both code paths that populate `packDatas`:
//
//  1. Boot-time scan via `HGAssets`'s constructor against the real
//     `_RELEASE/Packs/` tree (the test CMake target sets the working
//     directory to `_RELEASE/`).
//
//  2. Runtime workshop install via `HGAssets::installPackAtRuntime`,
//     which Steam UGC drives with absolute paths like
//     `/home/.../steamapps/workshop/content/<appId>/<fileId>` -- never
//     terminated with a `/`. A regression in this path looks like
//     "downloaded workshop pack doesn't show up until restart": the
//     pack metadata gets registered (packListVersion bumps) but every
//     subdir lookup misses, so no level data is ever loaded.

#include "SSVOpenHexagon/Data/PackData.hpp"
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "TestUtils.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/String.hpp"

namespace
{

void assertScriptsSubdirResolves(const sf::Path& path)
{
    TEST_ASSERT(!path.empty());

    // Every shipped pack carries a `Scripts/` directory; this asserts
    // that the exact concatenation used in `loadPackAssets` resolves
    // to a real path.
    TEST_ASSERT((path / "Scripts").isDirectory());
}

} // namespace

int main()
{
    hg::Config::loadConfig({});

    hg::HGAssets assets{
        nullptr /* steamManager */, //
        true /* headless */,        //
        true /* levelsOnly */       //
    };

    // ---------------------------------------------------------------------
    // (1) Boot-time scan: every pack loaded by the constructor must have a
    // valid `folderPath` that resolves the canonical `Scripts/` subdir.
    {
        const auto& packDatas = assets.getPackDatas();

        // The release tree ships with several packs; if loading silently
        // fails, this catches it immediately.
        TEST_ASSERT(!packDatas.empty());

        // At least one pack must expose a `Levels/` subdir for the
        // concatenation sanity check to be meaningful end-to-end (the
        // `base` pack ships with `Scripts/` only).
        bool sawAnyLevelsSubdir = false;

        for (const auto& [id, pd] : packDatas)
        {
            assertScriptsSubdirResolves(pd->folderPath);

            if ((pd->folderPath / "Levels").isDirectory())
            {
                sawAnyLevelsSubdir = true;
            }
        }

        TEST_ASSERT(sawAnyLevelsSubdir);
    }

    // ---------------------------------------------------------------------
    // (2) Runtime workshop install: simulate Steam delivering an absolute
    // folder path. Use an existing pack folder, canonicalized to absolute,
    // after first tearing down the boot-time copy so the idempotency guard
    // doesn't short-circuit the install.
    {
        // Pick a pack we know ships with `Scripts/` and (for the level-data
        // path coverage) `Levels/` -- `cube` qualifies.
        const sf::base::String victimId{"ohvrvanilla_vittorio_romeo_cube_1"};

        const sf::Path                     origFolderPath = assets.getPackDatas().at(victimId)->folderPath;
        const sf::base::Optional<sf::Path> absFolder      = origFolderPath.getAbsolute();
        TEST_ASSERT(absFolder.hasValue());

        TEST_ASSERT(assets.removePackAtRuntime(victimId));
        TEST_ASSERT_EQ(assets.getPackDatas().count(victimId), 0u);

        const sf::base::Optional<sf::base::String> newId = assets.installPackAtRuntime(*absFolder);

        TEST_ASSERT(newId.hasValue());
        TEST_ASSERT_EQ(*newId, victimId);

        // The freshly-installed pack's `folderPath` must let downstream
        // code resolve subdirs via `path / "Subdir"`. Before the fix, the
        // implicit trailing-slash contract was string-based and the
        // workshop path (no trailing slash) silently broke every subdir
        // lookup.
        const auto& reloaded = assets.getPackDatas().at(victimId);
        assertScriptsSubdirResolves(reloaded->folderPath);

        // The `Levels/` subdir must resolve via the stored path -- this
        // is the exact failure mode of the "newly downloaded workshop
        // pack doesn't show up" bug.
        TEST_ASSERT((reloaded->folderPath / "Levels").isDirectory());
    }
}
