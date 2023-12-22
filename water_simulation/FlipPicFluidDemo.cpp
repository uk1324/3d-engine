#include <water_simulation/FlipPicFluidDemo.hpp>
#include <framework/Dbg.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Color.hpp>
#include <glad/glad.h>

const Vec2T<i64> gridSize{ 200, 100 };

FlipPicFluidDemo::FlipPicFluidDemo() 
	: fluid(gridSize)
	, renderer(Renderer2d::make())
	, display(gridSize.x, gridSize.y) {

	for (i64 x = 0; x < fluid.gridSize.x; x++) {
		for (i64 y = 0; y < fluid.gridSize.y; y++) {
			if (x == 0 || y == 0 || x == fluid.gridSize.x - 1 || y == fluid.gridSize.y - 1) {
				fluid.setIsWall(x, y, true);
			}
		}
	}
}

void FlipPicFluidDemo::update() {
	const auto gridSizeCameraSpace = fluid.gridCellSize * Vec2(fluid.gridSize);
	const auto gridCenter = gridSizeCameraSpace / 2.0f;

	renderer.camera.pos = gridCenter;
	renderer.camera.changeSizeToFitBox(gridSizeCameraSpace);

	fluid.update(dt);

	/*const auto cursorPos = Input::cursorPosClipSpace() * renderer.camera.clipSpaceToWorldSpace();
	const auto cursorGridPos = Vec2T<i64>((cursorPos / fluid.gridCellSize).applied(floor));*/

	//for (i64 i = 0; i < fluid.particlePositions.size(); i++) {
	//	Dbg::drawDisk(fluid.particlePositions[i], fluid.particleRadius);
	//}

	for (auto& pixel : display.indexed()) {
		if (fluid.isWall(pixel.pos.x, pixel.pos.y)) {
			pixel = Pixel32(50);
		} else {
			/*pixel = Pixel32(0);*/
			pixel = Pixel32(Vec3(abs(fluid.velY.at(pixel.pos) / 10.0f)));
		}
	}

	glClear(GL_COLOR_BUFFER_BIT);
	renderer.imageRenderer.drawImage(display.span2d().asConst(), renderer.camera.makeTransform(gridCenter, 0.0f, gridSizeCameraSpace / 2.0f));
	renderer.update();
}
