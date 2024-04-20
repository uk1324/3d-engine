add_executable(platformer "main.cpp" "Game.cpp" "GameRenderer.cpp" "Player.cpp" "Editor.cpp" "Blocks.cpp" "Level.cpp" "JsonUtils.cpp" "Constants.cpp")

target_link_libraries(platformer engine)
target_link_libraries(platformer glfw)
target_link_libraries(platformer math)
target_link_libraries(platformer json)
target_link_libraries(platformer framework)
target_link_libraries(platformer math-compiler)

target_include_directories(platformer PRIVATE "../dependencies/math-compiler/src")

targetAddGenerated(platformer ${CMAKE_CURRENT_SOURCE_DIR})