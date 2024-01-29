add_executable(water_simulation "main.cpp" "AdvecitonDemo.cpp" "EulerianFluidDemo.cpp" "../framework/ShaderManager.cpp" "../framework/Camera.cpp" "../framework/Renderer2d.cpp" "../framework/ShapeRenderer2d.cpp" "../framework/Dbg.cpp" "EulerianFluid.cpp" "MainLoop.cpp" "FlipPicFluid.cpp" "FlipPicFluidDemo.cpp")

target_link_libraries(water_simulation engine)
target_link_libraries(water_simulation glfw)
target_link_libraries(water_simulation math)
target_link_libraries(water_simulation json)

targetAddGenerated(water_simulation ${CMAKE_CURRENT_SOURCE_DIR})