add_executable(electromagnetism "main.cpp" "MainLoop.cpp" )

target_link_libraries(electromagnetism engine)
target_link_libraries(electromagnetism glfw)
target_link_libraries(electromagnetism math)
target_link_libraries(electromagnetism json)

targetAddGenerated(electromagnetism ${CMAKE_CURRENT_SOURCE_DIR})
