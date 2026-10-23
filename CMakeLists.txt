cmake_minimum_required(VERSION 3.25)
project(pidginspec VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(nlohmann_json 3.11 REQUIRED)

add_library(pidginspec_score src/score.cpp)
target_include_directories(pidginspec_score PUBLIC include)
target_link_libraries(pidginspec_score PUBLIC nlohmann_json::nlohmann_json)
target_compile_options(pidginspec_score PRIVATE -Wall -Wextra -Wpedantic -Werror)

add_executable(pidginspec src/main.cpp)
target_link_libraries(pidginspec PRIVATE pidginspec_score)
target_compile_options(pidginspec PRIVATE -Wall -Wextra -Wpedantic -Werror)

enable_testing()
add_executable(pidginspec_tests tests/score_tests.cpp)
target_link_libraries(pidginspec_tests PRIVATE pidginspec_score)
target_compile_options(pidginspec_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME pidginspec_tests COMMAND pidginspec_tests)
