add_executable(water_simulation "main.cpp" "AdvecitonDemo.cpp" "EulerianFluidDemo.cpp" "EulerianFluid.cpp" "MainLoop.cpp" "FlipPicFluid.cpp" "FlipPicFluidDemo.cpp" "PolynomialInterpolationDemo.cpp" "SpecialFunctionsDemo.cpp" "IntegrationDemo.cpp" "GraphDemo.cpp" "FirstOrderSystemGraph.cpp" "SecondOrderSystemGraph.cpp" "PlotCompiler.cpp" "SettingsManager.cpp" "FunctionPlotter2d.cpp" "Camera3d.cpp")

target_link_libraries(water_simulation engine)
target_link_libraries(water_simulation glfw)
target_link_libraries(water_simulation math)
target_link_libraries(water_simulation json)
target_link_libraries(water_simulation framework)
target_link_libraries(water_simulation math-compiler)

target_include_directories(water_simulation PRIVATE "../dependencies/math-compiler/src")

targetAddGenerated(water_simulation ${CMAKE_CURRENT_SOURCE_DIR})