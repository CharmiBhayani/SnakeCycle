cmake_minimum_required(VERSION 3.14)
project(SnakeCycleTests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable coverage flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0 -g")

# Add GoogleTest from third_party
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/googletest-1.14.0)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

enable_testing()

add_executable(runTests
  tests/test_rule1_food_appearance.cpp
  tests/test_rule2_fruit_score.cpp
  tests/test_rule3_fruit_placement.cpp
  tests/test_rule4_initial_state.cpp
  tests/test_rule5_collision.cpp
)

target_link_libraries(runTests
  GTest::gtest_main
  gcov
)

include(GoogleTest)
gtest_discover_tests(runTests)
