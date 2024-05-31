cmake_minimum_required(VERSION 3.15)
project(client)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CODEGEN_TOOL_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(GENERATED_PATH "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(EXECUTABLE_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(FINAL_RELEASE TRUE)

if (FINAL_RELEASE)
	add_definitions(-DFINAL_RELEASE)
endif()

include("./codeGenTool/targetAddGenerated.cmake")

add_subdirectory("./dependencies/glfw")
add_subdirectory("./dependencies/math-compiler/src")
add_subdirectory("./dependencies/openal-soft")

add_subdirectory(engine)

add_subdirectory(engine/json)

add_subdirectory(framework)
 
add_subdirectory(game)

add_subdirectory(water_rendering)

add_subdirectory(water_simulation)

add_subdirectory(electromagnetism)

add_subdirectory(platformer)

add_subdirectory(buildTool)

add_subdirectory(embedTool)

set(gtest_force_shared_crt True)
add_subdirectory("./dependencies/googletest")
add_subdirectory(test)

# Working directory can be set by clicking debug > debug and launch settings for <target> and settings "currentDir": "${projectDir}"
# Tasks https://learn.microsoft.com/en-us/visualstudio/ide/customize-build-and-debug-tasks-in-visual-studio?view=vs-2022
# https://learn.microsoft.com/en-us/cpp/build/tasks-vs-json-schema-reference-cpp?view=msvc-170

# The default configuration of RelWithDebInfo is a lot slower than the default configuration when creating a visual studio solution with Release mode. The incompresibility solve step of the fluid simulator is around 2 times slower when using RelWithDebInfo than when using cmake Release. The time using cmake Release mode seems the same as Visual studio Release option.
# According to this
# https://stackoverflow.com/questions/48754619/what-are-cmake-build-type-debug-release-relwithdebinfo-and-minsizerel
# on gcc RelWithDebInfo uses O2 and Release used O3
# Here are also some websites mentioning this issue.
# https://reviews.llvm.org/D55056
# https://groups.google.com/g/llvm-dev/c/8NjvujK0QzE
# https://github.com/conan-io/conan/issues/4221