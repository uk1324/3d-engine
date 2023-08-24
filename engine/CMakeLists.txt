add_subdirectory(Math)

add_library(
	engine 
	"../dependencies/imgui/imgui.cpp"
	"../dependencies/imgui/imgui_draw.cpp"
	"../dependencies/imgui/imgui_impl_glfw.cpp"
	"../dependencies/imgui/imgui_tables.cpp"
	"../dependencies/imgui/imgui_widgets.cpp"
	"../dependencies/imgui/imgui_impl_opengl3.cpp"
	"../dependencies/imgui/imgui_impl_opengl3.h" 
	"../dependencies/imgui/imgui_demo.cpp"

	"../dependencies/stb_image/stb_image.cpp"
	"../dependencies/stb_image/stb_image_write.cpp"
	"../dependencies/stb_image/stb_image_resize.cpp"

	"../dependencies/glad/src/glad.c"

	"Graphics/Shader.cpp" "Graphics/Shader.hpp" "Graphics/ShaderProgram.hpp" "Graphics/ShaderProgram.cpp" "Graphics/Texture.hpp" "Graphics/Texture.cpp" "Graphics/Vbo.hpp" "Graphics/Vbo.cpp" "Graphics/Vao.hpp" "Graphics/Vao.cpp" "Graphics/Ibo.cpp" "Graphics/Fbo.hpp" "Graphics/Fbo.cpp" "Graphics/TextureAtlasGenerator.cpp" "Input/Input.hpp" "Input/Input.cpp" "Window.hpp" "Window.cpp" "Engine.hpp"  "Engine.cpp"   "Input/MouseButton.hpp"  "Utils/Color32.cpp" "Utils/Image32.cpp" "Utils/Timer.hpp" "Utils/ByteBuffer.hpp" "Utils/ByteBuffer.cpp" "Utils/Timer.cpp" "Utils/Gui.cpp"  "Utils/Json.cpp" "Utils/FileIo.hpp" "../engine/Utils/FileIo.cpp" "Utils/StringStream.cpp" "Utils/Utf8.cpp" "Utils/Put.cpp" "Utils/PreprocessIncludes.cpp" "Utils/Assertions.cpp" "Utils/CrashReport.cpp" "Utils/FixedUpdateLoop.cpp"  "Utils/HashCombine.cpp")

target_include_directories(
	engine PUBLIC
	"../dependencies/glad/include"
	"../dependencies/glfw/include"
	"../dependencies/stb_image/include"
	"../dependencies/imgui"
	"../dependencies"
	"../"
	"./Utils"
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

target_include_directories(engine PRIVATE "./")