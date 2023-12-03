add_executable(water_simulation "main.cpp" "AdvecitonDemo.cpp" "MainLoop.cpp" "../framework/ShaderManager.cpp" "../framework/Camera.cpp" "../framework/ImageRenderer.cpp" "Fluid.cpp")

target_link_libraries(water_simulation engine)
target_link_libraries(water_simulation glfw)
target_link_libraries(water_simulation math)
target_link_libraries(water_simulation json)

targetAddGenerated(water_simulation ${CMAKE_CURRENT_SOURCE_DIR})