option(SSVOH_HEADLESS_TESTS "Compile tests in headless mode." FALSE)

# Add a custom target for the unit tests.
add_custom_target(tests COMMENT "Build all the unit tests.")

# `check` depends on `tests`.
add_dependencies(check tests)

# Include directories.
include_directories(${SSVOPENHEXAGON_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR})

function(ssvoh_add_test file)
    set(target_name "${file}-test")

    add_executable(${target_name} "${file}")
    add_dependencies(tests ${target_name})
    add_test(NAME ${target_name} COMMAND ${target_name})

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
ssvoh_add_test("ReplayExecution.t.cpp")
ssvoh_add_test("ReplayExecutionBenchmark.t.cpp")
ssvoh_add_test("Utils_BuildPackId.t.cpp")
ssvoh_add_test("Utils_Split.t.cpp")
