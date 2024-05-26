add_executable(platformer "main.cpp" "Game.cpp" "GameRenderer.cpp" "Player.cpp" "Editor.cpp" "Blocks.cpp" "Level.cpp" "JsonUtils.cpp" "Constants.cpp" "Entities.cpp" "Collision.cpp" "GameRoom.cpp" "Audio/LoadWav.cpp" "Audio/Audio.cpp" "SoundGeneration.cpp"  "Audio/AudioErrorHandling.cpp" "Audio/AudioBuffer.cpp" "Audio/AudioSource.cpp" "Audio/AudioStream.cpp" "Assets.cpp" "../dependencies/stb_vorbis/stb_vorbis.c" "Audio/AudioFileStream.cpp" "Audio/AudioFormat.cpp" "GameAudio.cpp")

target_link_libraries(platformer engine)
target_link_libraries(platformer glfw)
target_link_libraries(platformer math)
target_link_libraries(platformer json)
target_link_libraries(platformer framework)
target_link_libraries(platformer math-compiler)
target_link_libraries(platformer OpenAL)

target_include_directories(platformer PRIVATE "../dependencies/math-compiler/src")
target_include_directories(platformer PRIVATE "../dependencies/openal-soft/al")

targetAddGenerated(platformer ${CMAKE_CURRENT_SOURCE_DIR})