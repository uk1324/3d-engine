add_executable(electromagnetism "main.cpp" "MainLoop.cpp" "../framework/ShaderManager.cpp" "../framework/Camera.cpp" "../framework/Renderer2d.cpp" "../framework/ShapeRenderer2d.cpp" "../framework/Dbg.cpp")

target_link_libraries(electromagnetism engine)
target_link_libraries(electromagnetism glfw)
target_link_libraries(electromagnetism math)
target_link_libraries(electromagnetism json)

targetAddGenerated(electromagnetism ${CMAKE_CURRENT_SOURCE_DIR})
targetAddGenerated(electromagnetism ${CMAKE_CURRENT_SOURCE_DIR}/../framework/)