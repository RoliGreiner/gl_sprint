cmake_minimum_required(VERSION 3.20)
project(gl_sprint LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Where your exercise code lives. You normally never change this.
set(SPRINT_EXERCISE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/days" CACHE PATH "Folder that holds day1 ... day5")

find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)

enable_testing()

# ---------------------------------------------------------------------------
# Third-party code
# ---------------------------------------------------------------------------
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# stb_image and stb_image_write are compiled once here.
# Do not define STB_IMAGE_IMPLEMENTATION in your own code.
add_library(stb STATIC framework/stb_impl.c)
target_include_directories(stb PUBLIC external/stb)

# ---------------------------------------------------------------------------
# Framework (provided, don't edit): window helper for the apps, test runner
# ---------------------------------------------------------------------------
add_library(sprint_framework STATIC framework/sprint/window.cpp)
target_include_directories(sprint_framework PUBLIC framework)
target_link_libraries(sprint_framework PUBLIC glad glfw glm::glm OpenGL::GL)

add_library(sprint_test STATIC
    framework/sprint/test_runner.cpp
    framework/sprint/gl_test.cpp)
target_include_directories(sprint_test PUBLIC framework)
target_link_libraries(sprint_test PUBLIC glad glfw glm::glm OpenGL::GL stb)

# ---------------------------------------------------------------------------
# One block per day:
#   dayN_lib    your exercise code (everything in days/dayN except main.cpp)
#   dayN        your interactive program (days/dayN/main.cpp)
#   dayN_tests  the automatic checks (tests/dayN_tests.cpp), also run by ctest
# Each day can use the code of all earlier days.
# ---------------------------------------------------------------------------
set(SPRINT_PREVIOUS_DAYS "")

function(sprint_add_day N)
    set(dir "${SPRINT_EXERCISE_DIR}/day${N}")
    if(NOT EXISTS "${dir}/main.cpp")
        message(STATUS "gl-sprint: ${dir}/main.cpp not found, skipping day ${N}")
        return()
    endif()
    file(GLOB lib_sources CONFIGURE_DEPENDS "${dir}/*.cpp")
    list(REMOVE_ITEM lib_sources "${dir}/main.cpp")
    file(GLOB shader_files CONFIGURE_DEPENDS "${dir}/shaders/*")

    add_library(day${N}_lib STATIC ${lib_sources} ${shader_files})
    target_include_directories(day${N}_lib PUBLIC "${SPRINT_EXERCISE_DIR}")
    target_compile_definitions(day${N}_lib PRIVATE SPRINT_DAY_DIR="${dir}")
    target_compile_options(day${N}_lib PRIVATE -Wall -Wextra)
    target_link_libraries(day${N}_lib PUBLIC sprint_framework stb ${SPRINT_PREVIOUS_DAYS})

    add_executable(day${N} "${dir}/main.cpp")
    target_compile_definitions(day${N} PRIVATE SPRINT_DAY_DIR="${dir}")
    target_compile_options(day${N} PRIVATE -Wall -Wextra)
    target_link_libraries(day${N} PRIVATE day${N}_lib)

    add_executable(day${N}_tests tests/day${N}_tests.cpp)
    target_include_directories(day${N}_tests PRIVATE tests)
    target_link_libraries(day${N}_tests PRIVATE day${N}_lib sprint_test)

    add_test(NAME day${N} COMMAND day${N}_tests)
    set_tests_properties(day${N} PROPERTIES LABELS "day${N}" TIMEOUT 60)

    set(SPRINT_PREVIOUS_DAYS ${SPRINT_PREVIOUS_DAYS} day${N}_lib PARENT_SCOPE)
endfunction()

sprint_add_day(1)
sprint_add_day(2)
sprint_add_day(3)
sprint_add_day(4)
sprint_add_day(5)
