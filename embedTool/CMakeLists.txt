project(embedTool)
add_executable(embedTool "main.cpp" "../engine/Utils/PreprocessIncludes.cpp" "../engine/Utils/FileIo.cpp" "../engine/Utils/CrashReport.cpp" "../engine/Utils/Assertions.cpp")

target_link_libraries(embedTool json)

target_include_directories(embedTool PUBLIC "../" "../engine/Utils")