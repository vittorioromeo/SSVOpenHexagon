option(SSVOH_HEADLESS_TESTS "Compile tests in headless mode." FALSE)

# Add a custom target for the unit tests.
add_custom_target(tests COMMENT "Build all the unit tests.")

# `check` depends on `tests`.
add_dependencies(check tests)

# Include directories.
include_directories(${SSVOPENHEXAGON_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR})

# Set to TRUE in `ssvoh_add_test` (via the optional second argument) for
# tests that bring up a real GL context. Such tests leak unavoidable
# bytes deep inside the GL driver / windowing libraries -- the leak
# stacks come back as `<unknown module>` because the symbols are stripped
# from the system libraries, so the per-symbol `lsan.supp` patterns
# can't match. Disabling leak detection entirely is the pragmatic choice
# for these tests; the rest of the suite still has it on.
function(ssvoh_add_test file)
    set(opts OPENS_WINDOW)
    cmake_parse_arguments(SSVOH_TEST "${opts}" "" "" ${ARGN})

    set(target_name "${file}-test")

    add_executable(${target_name} "${file}")
    add_dependencies(tests ${target_name})

    # Tests that exercise the asset pipeline (`ReplayExecution`,
    # `ReplayExecutionBenchmark`) call `HGAssets::loadAssets`, which
    # expects a `Packs/` folder relative to the working directory. Run
    # tests from `_RELEASE/` so they find the same pack/level data the
    # release build uses. Tests that don't touch the asset pipeline are
    # CWD-agnostic, so it's safe to set this uniformly.
    add_test(
        NAME ${target_name}
        COMMAND ${target_name}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/_RELEASE
    )

    # Default: keep LSan on, point it at the suppressions file so
    # third-party D-Bus / libGL allocations from process startup don't
    # fail the test. `OPENS_WINDOW` tests turn LSan off entirely
    # because the GL driver leaks come back with stripped frames that
    # the `leak:` patterns can't match.
    if(SSVOH_TEST_OPENS_WINDOW)
        set(_test_env "ASAN_OPTIONS=detect_leaks=0")
    else()
        set(_test_env "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/test/lsan.supp:print_suppressions=0")
    endif()

    set_tests_properties(${target_name} PROPERTIES ENVIRONMENT "${_test_env}")

    if(SSVOH_USE_PCH)
        target_precompile_headers(${target_name} REUSE_FROM SSVOpenHexagonLib)
    endif()

    if(${SSVOH_HEADLESS_TESTS})
        target_compile_definitions(${target_name} PUBLIC "SSVOH_HEADLESS_TESTS")
    endif()

    target_link_libraries(${target_name}
        ${SFML_LIBRARIES}
        libluajit
        zlib
        ${PUBLIC_LIBRARIES}
        SSVOpenHexagonLib
        SSVOpenHexagonLibC
    )
endfunction()

# Generate all the unit tests.
ssvoh_add_test("FixedFunction.t.cpp")
ssvoh_add_test("Replay.t.cpp")
ssvoh_add_test("ReplayExecution.t.cpp" OPENS_WINDOW) # `sf::GraphicsContext::create()` -> libGL leaks
ssvoh_add_test("ReplayExecutionBenchmark.t.cpp")
ssvoh_add_test("PackLoading.t.cpp")
ssvoh_add_test("Utils_BuildPackId.t.cpp")
ssvoh_add_test("Utils_Split.t.cpp")
ssvoh_add_test("NetworkNonBlockingReceive.t.cpp")
ssvoh_add_test("HexagonServer.t.cpp")
ssvoh_add_test("HexagonServerLoad.t.cpp")
ssvoh_add_test("HexagonServerManyClients.t.cpp")
