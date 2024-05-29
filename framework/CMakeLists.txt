add_library(
	framework
	"../framework/ShaderManager.cpp" "../framework/Camera.cpp" "../framework/Renderer2d.cpp" "../framework/ShapeRenderer2d.cpp" "../framework/Dbg.cpp" "FontRendering/Font.cpp"
 "CameraUtils.cpp" "FontRendering/FontRenderer.cpp")

targetAddGenerated(framework "./")

target_include_directories(
	framework PUBLIC
	"../"
	"../engine/Utils"
	"../dependencies/glad/include"
	"../dependencies/"
)