add_executable(
	game
	"main.cpp"
 "Game.cpp" "Renderer.cpp" "FpsController.cpp")

target_link_libraries(game engine)
target_link_libraries(game glfw)
target_link_libraries(game math)

targetAddGenerated(game ${CMAKE_CURRENT_SOURCE_DIR})