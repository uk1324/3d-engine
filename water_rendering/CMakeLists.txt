add_executable(water_rendering "main.cpp" "MainLoop.cpp" "../game/FpsController" "ShaderManager.cpp")

target_link_libraries(water_rendering engine)
target_link_libraries(water_rendering glfw)
target_link_libraries(water_rendering math)
target_link_libraries(water_rendering json)

targetAddGenerated(water_rendering ${CMAKE_CURRENT_SOURCE_DIR})