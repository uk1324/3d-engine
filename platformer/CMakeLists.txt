add_executable(platformer "main.cpp" "Game.cpp" "GameRenderer.cpp" "Player.cpp" "Editor.cpp" "Blocks.cpp" "Level.cpp" "JsonUtils.cpp" "Constants.cpp" "Entities.cpp" "Collision.cpp" "GameRoom.cpp" "Audio/LoadWav.cpp" "Audio/Audio.cpp" "SoundGeneration.cpp"  "Audio/AudioErrorHandling.cpp" "Audio/AudioBuffer.cpp" "Audio/AudioSource.cpp" "Audio/AudioStream.cpp" "Assets.cpp" "../dependencies/stb_vorbis/stb_vorbis.c" "Audio/AudioFileStream.cpp" "Audio/AudioFormat.cpp" "GameAudio.cpp" "Menu.cpp" "MainLoop.cpp" "SettingsManager.cpp")

target_link_libraries(platformer engine)
target_link_libraries(platformer glfw)
target_link_libraries(platformer math)
target_link_libraries(platformer json)
target_link_libraries(platformer framework)
target_link_libraries(platformer math-compiler)
target_link_libraries(platformer OpenAL)

if (WIN32)
	target_link_libraries(platformer "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/freetype.lib")
else()
	target_link_libraries(platformer freetype)
endif()

target_include_directories(platformer PRIVATE "../dependencies/math-compiler/src")
target_include_directories(platformer PRIVATE "../dependencies/openal-soft/al")

targetAddGenerated(platformer ${CMAKE_CURRENT_SOURCE_DIR})

# If this is on then the console logs won't show
if (FINAL_RELEASE)
	if (WIN32)
		set_target_properties(platformer PROPERTIES WIN32_EXECUTABLE TRUE)
	endif()
endif()

if (FINAL_RELEASE)
	set(EMBED_TOOL "$<TARGET_FILE:embedTool>")
	add_custom_command(
		TARGET platformer PRE_BUILD
		COMMAND ${EMBED_TOOL}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		DEPENDS ${EMBED_TOOL}
	)
	set(BUILD_TOOL "$<TARGET_FILE:buildTool>")
	set(targetName "platformer")
	add_custom_command(
		TARGET ${targetName} POST_BUILD
		COMMAND ${BUILD_TOOL} $<TARGET_FILE:platformer>
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		DEPENDS ${BUILD_TOOL}
	)
endif()