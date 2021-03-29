# Add a custom target for the unit tests.
add_custom_target(tests COMMENT "Build all the unit tests.")

# `check` depends on `tests`.
add_dependencies(check tests)

# Include directories.
include_directories(${SSVOPENHEXAGON_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR})

# Generate all the unit tests.
vrm_cmake_generate_unit_tests_glob("*.cpp")

foreach(_t IN LISTS vrm_cmake_out)
    target_link_libraries(${_t} ${SFML_LIBRARIES})
    target_link_libraries(${_t} ${LUALIB})
    target_link_libraries(${_t} zlib)
    target_link_libraries(${_t} ${PUBLIC_LIBRARIES})
    target_link_libraries(${_t} "${CMAKE_CURRENT_SOURCE_DIR}/../_RELEASE/discord_game_sdk.dll")
    target_link_libraries(${_t} SSVOpenHexagonLib)
endforeach()
