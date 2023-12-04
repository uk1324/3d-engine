#include <water_simulation/MainLoop.hpp>
#include <glad/glad.h>
#include <water_simulation/Shaders/texturedQuadData.hpp>
#include <StructUtils.hpp>
#include <framework/ShaderManager.hpp>
#include <framework/Camera.hpp>
#include <engine/Window.hpp>
#include <engine/Input/Input.hpp>
#include <framework/Instancing.hpp>
#include <engine/Math/Random.hpp>
#include <Dbg.hpp>
//#include <engine/Math/Aabb.hpp>
//#include <engine/Input/Input.hpp>
//#include <engine/Utils/Gui.hpp>
//#include <imgui/imgui.h>
//#include <iostream>
//#include <Bits.hpp>
//#include <water_simulation/AdvectionDemo.hpp>
//#include <water_simulation/Fluid.hpp>

static constexpr TexturedQuadVertex fullscreenQuadVerts[]{
	{ Vec2{ -1.0f, 1.0f }, Vec2{ 0.0f, 1.0f } },
	{ Vec2{ 1.0f, 1.0f }, Vec2{ 1.0f, 1.0f } },
	{ Vec2{ -1.0f, -1.0f }, Vec2{ 0.0f, 0.0f } },
	{ Vec2{ 1.0f, -1.0f }, Vec2{ 1.0f, 0.0f } },
};

static constexpr u32 fullscreenQuadIndices[]{
	0, 1, 2, 2, 1, 3
};

MainLoop MainLoop::make() {
	const Vec2T<i64> gridSize(200, 100);

	// TODO: Could use lambdas in some cases instead of moves. The issue is that function require a certain order or parameters. Also I won't be able to access previous values returned from a function. Not sure if there is another way to do it.

	auto image = *Image32::fromFile("assets/ignore/a.jpg");
	Image32 imageResized(gridSize.x, gridSize.y);
	imageResized.copyAndResize(image);

	auto makeSmoke = [&]() -> Array2d<float> {
		Array2d<float> smoke(gridSize.x, gridSize.y);
		memset(smoke.data(), 0, smoke.dataBytesSize());
		return smoke;
	};

	MainLoop value{
		.fluid = EulerianFluid(gridSize, 0.02f),
		.smokeR = makeSmoke(),
		.smokeG = makeSmoke(),
		.smokeB = makeSmoke(),
		.image = Image32(gridSize.x, gridSize.y),
		.renderer = ImageRenderer::make(),
	};

	for (i64 x = 0; x < gridSize.x; x++) {
		for (i64 y = 0; y < gridSize.y; y++) {
			value.smokeR(x, y) = imageResized(x, y).r / 255.0f;
			value.smokeG(x, y) = imageResized(x, y).g / 255.0f;
			value.smokeB(x, y) = imageResized(x, y).b / 255.0f;
		}
	}

	return value;
	/*return MainLoop{
		.image = Image32(128, 128),
		.renderer = ImageRenderer::make()
	};*/
	/*auto image = Image32::fromFile("assets/image.png");
	Image32 resized(fluid0.numX, fluid0.numY);
	resized.copyAndResize(*image);*/

	/*for (int x = 0; x < fluid0.numX; x++) {
		for (int y = 0; y < fluid0.numY; y++) {

			const auto gridSize = 10;
			const auto ix = x / gridSize;
			const auto iy = y / gridSize;
			if (ix % 2 == iy % 2) {
				setColor(x, y, Vec3(0.0f));
			} else {
				setColor(x, y, Vec3(255.0f));
			}

		}
	}*/

	//for (i64 i = 0; i < fluid.numX; i++) {
	//	for (i64 j = 0; j < fluid.numY; j++) {
	//		auto s = 1.0;	// fluid
	//		if (i == 0 || i == fluid.numX - 1 || j == 0 || j == fluid.numY - 1)
	//			s = 0.0;	// solid
	//		fluid.s[i * fluid.numY + j] = s;
	//	}
	//}

	/*auto texture = Texture::generate();
	texture.bind();

	EulerianFluid fluid(Vec2T<i64>(200, 100), 1.0f / 100.0f);


	value.imageData.resize(value.fluid.gridSize.x * value.fluid.gridSize.y * 3);

	for (int x = 0; x < value.fluid.gridSize.x; x++) {
		for (int y = 0; y < value.fluid.gridSize.y; y++) {
			const auto offset = (y * value.fluid.gridSize.x + x) * 3.0f;
			value.imageData[offset] = random01();
			value.imageData[offset + 1] = random01();
			value.imageData[offset + 2] = random01();
		}
	}

	return value;*/
}

#include <RandomAccess2d.hpp>
#include <Span2d.hpp>

//template<typename Matrix, typename ItemType> requires RandomAccessGet2d<Matrix, ItemType>
//ItemType abc(Matrix test) {
//	return test.get(1, 1);
//}

#include <Timer.hpp>

void MainLoop::update() {


	//abc(a);
	/*auto x = abc<Span2d<float>, float>(a);*/
	//auto x = abc<Span2d<float>>(a);
	/*auto x = abc<Span2d<float>>(a);*/

	/*glClear(GL_COLOR_BUFFER_BIT);
	return;*/
	Camera camera;
	camera.aspectRatio = Window::aspectRatio();


	float gravity = 0.0f;
	i32 solverIterations = 40;
	Timer timer;

	timer.reset();
	fluid.integrate(dt, gravity);
	timer.guiTookMiliseconds("integrate");

	timer.reset();
	std::fill(fluid.pressure.begin(), fluid.pressure.end(), 0.0f);
	fluid.solveIncompressibility(solverIterations, dt);
	timer.guiTookMiliseconds("solve incompresibility");

	timer.reset();
	fluid.advectVelocity(dt);
	timer.guiTookMiliseconds("advect velocity");

	//fluid.update(dt, 0.0f, 40);
	/*fluid.advectQuantity(smokeR.span2d(), dt);
	fluid.advectQuantity(smokeG.span2d(), dt);
	fluid.advectQuantity(smokeB.span2d(), dt);*/

	//Gui::put("update took: %", timer.elapsedMilliseconds());

	const auto gridSize = fluid.cellSpacing * Vec2(fluid.gridSize);
	const auto gridCenter = gridSize / 2.0f;
	camera.pos = gridCenter;
	camera.changeSizeToFitBox(gridSize);

	auto transform = camera.makeTransform(gridCenter, 0.0f, gridSize / 2.0f);
	//auto transform = camera.makeTransform(Vec2(0.0f), 0.0f, Vec2(image.size().xOverY(), 1.0f));
	//transform[1][1] = -transform[1][1];
	//fluid.update(1.0f / 60.0f, 0.0f, 40);

	const auto cursorPos = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
	const auto cursorGridPos = Vec2T<i64>((cursorPos / fluid.cellSpacing).applied(floor));

	static Vec2 obstaclePos(0.0f);

	float obstacleRadius = fluid.cellSpacing * 5.0f;
	static bool obstacleReleased = true;
	if (Input::isMouseButtonHeld(MouseButton::LEFT)) {
		/*if (cursorGridPos.x > 0 && cursorGridPos.y > 0 && cursorGridPos.x < fluid.gridSize.x && cursorGridPos.y < fluid.gridSize.y) {
			fluid.at(fluid.smoke, cursorGridPos.x, cursorGridPos.y) = 0.0f;
			fluid.at(fluid.velX, cursorGridPos.x, cursorGridPos.y) = 5.0f;
		}*/

		//Vec2 newPos = Vec2{ fluid.gridPos } *SPACE_BETWEEN_CELLS;
		const auto newPos = cursorPos;
		Vec2 vel;
		if (obstacleReleased) {
			obstacleReleased = false;
			vel = Vec2(0.0f);
		} else {
			vel = (newPos - obstaclePos) / dt;
		}
		obstaclePos = newPos;

		for (i64 x = 1; x < fluid.gridSize.x - 2; x++) {
			for (i64 y = 1; y < fluid.gridSize.y - 2; y++) {
				const auto cellPos = Vec2(Vec2T(x, y)) * fluid.cellSpacing;
				const auto n = fluid.gridSize.y;
				if (!fluid.isWall(x, y) && (cellPos - obstaclePos).lengthSq() < pow(obstacleRadius, 2.0f)) {
					// Smoke doesn't impact velocities it only advectes (moves) with them. It is used to visualize how the fluid moves. The color doesn't represent the amount of fluid at a point. The fluid is incompressible so it has the same amount everywhere. The smoke is kind of like a fluid moving inside a fluid and it can vary from place to place. 

					// This code just sets the value of the smoke to a changing value to better show changes and it also looks cool. Could just have different smokes values at different points and just move them.
					//fluid.at(fluid.smoke, x, y) = 0.5f + 0.5f * sin(elapsed / Time::deltaTime() * 0.1f);
					//fluid.spanFrom(fluid.smoke)(x, y) = 0.5f;
					//smoke(x, y) = 0.5f;

					fluid.at(fluid.velX, x, y) = vel.x;
					fluid.at(fluid.velX, x + 1, y) = vel.x;
					fluid.at(fluid.velY, x, y) = vel.y;
					fluid.at(fluid.velY, x, y + 1) = vel.y;

					/*fluid.spanFrom(fluid.velX)(x, y) = vel.x;
					fluid.spanFrom(fluid.velX)(x + 1, y) = vel.x;
					fluid.spanFrom(fluid.velY)(x, y) = vel.y;
					fluid.spanFrom(fluid.velY)(x, y + 1) = vel.y;*/
				}

			}
		}
	}
	if (Input::isMouseButtonUp(MouseButton::LEFT)) {
		obstacleReleased = true;
	}

	chk(showDivergence, false) {
		for (auto pixel : image.indexed()) {
			pixel = Pixel32(Vec3(fluid.at(fluid.divergence, pixel.pos.x, pixel.pos.y)));
		}
	} else {
		for (auto pixel : image.indexed()) {
			/*pixel = Pixel32(Vec3(fluid.at(fluid.smoke, pixel.pos.x, pixel.pos.y)));*/
			/*pixel = Pixel32(Vec3(smoke(pixel.pos.x, pixel.pos.y)));*/
			pixel = Pixel32(Vec3(smokeR(pixel.pos.x, pixel.pos.y), smokeG(pixel.pos.x, pixel.pos.y), smokeB(pixel.pos.x, pixel.pos.y)));
			//pixel = Pixel32(Vec3(pixel.pos.x, pixel.pos.y, 0.0f) / Vec3(fluid.gridSize.x, fluid.gridSize.y, 0.0f));
		}
	}

	renderer.drawImage(image.data(), image.size(), transform);

	renderer.update();
}


//void MainLoop::update() {
//	
//
//	//abc(a);
//	/*auto x = abc<Span2d<float>, float>(a);*/
//	//auto x = abc<Span2d<float>>(a);
//	/*auto x = abc<Span2d<float>>(a);*/
//
//	/*glClear(GL_COLOR_BUFFER_BIT);
//	return;*/
//	Camera camera;
//	camera.aspectRatio = Window::aspectRatio();
//	
//
//	float gravity = 0.0f;
//	i32 solverIterations = 40;
//	Timer timer;
//
//	timer.reset();
//	fluid.integrate(dt, gravity);
//	timer.guiTookMiliseconds("integrate");
//
//	timer.reset();
//	std::fill(fluid.pressure.begin(), fluid.pressure.end(), 0.0f);
//	fluid.solveIncompressibility(solverIterations, dt);
//	timer.guiTookMiliseconds("solve incompresibility");
//
//	timer.reset();
//	fluid.advectVelocity(dt);
//	timer.guiTookMiliseconds("advect velocity");
//
//	//fluid.update(dt, 0.0f, 40);
//	/*fluid.advectQuantity(smokeR.span2d(), dt);
//	fluid.advectQuantity(smokeG.span2d(), dt);
//	fluid.advectQuantity(smokeB.span2d(), dt);*/
//
//	//Gui::put("update took: %", timer.elapsedMilliseconds());
//
//	const auto gridSize = fluid.cellSpacing * Vec2(fluid.gridSize);
//	const auto gridCenter = gridSize / 2.0f;
//	camera.pos = gridCenter;
//	camera.changeSizeToFitBox(gridSize);
//
//	auto transform = camera.makeTransform(gridCenter, 0.0f, gridSize / 2.0f);
//	//auto transform = camera.makeTransform(Vec2(0.0f), 0.0f, Vec2(image.size().xOverY(), 1.0f));
//	//transform[1][1] = -transform[1][1];
//	//fluid.update(1.0f / 60.0f, 0.0f, 40);
//
//	const auto cursorPos = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
//	const auto cursorGridPos = Vec2T<i64>((cursorPos / fluid.cellSpacing).applied(floor));
//
//	static Vec2 obstaclePos(0.0f);
//
//	float obstacleRadius = fluid.cellSpacing * 5.0f;
//	static bool obstacleReleased = true;
//	if (Input::isMouseButtonHeld(MouseButton::LEFT)) {
//		/*if (cursorGridPos.x > 0 && cursorGridPos.y > 0 && cursorGridPos.x < fluid.gridSize.x && cursorGridPos.y < fluid.gridSize.y) {
//			fluid.at(fluid.smoke, cursorGridPos.x, cursorGridPos.y) = 0.0f;
//			fluid.at(fluid.velX, cursorGridPos.x, cursorGridPos.y) = 5.0f;
//		}*/
//
//		//Vec2 newPos = Vec2{ fluid.gridPos } *SPACE_BETWEEN_CELLS;
//		const auto newPos = cursorPos;
//		Vec2 vel;
//		if (obstacleReleased) {
//			obstacleReleased = false;
//			vel = Vec2(0.0f);
//		} else {
//			vel = (newPos - obstaclePos) / dt;
//		}
//		obstaclePos = newPos;
//
//		for (i64 x = 1; x < fluid.gridSize.x - 2; x++) {
//			for (i64 y = 1; y < fluid.gridSize.y - 2; y++) {
//				const auto cellPos = Vec2(Vec2T(x, y)) * fluid.cellSpacing;
//				const auto n = fluid.gridSize.y;
//				if (!fluid.isWall(x, y) && (cellPos - obstaclePos).lengthSq() < pow(obstacleRadius, 2.0f)) {
//					// Smoke doesn't impact velocities it only advectes (moves) with them. It is used to visualize how the fluid moves. The color doesn't represent the amount of fluid at a point. The fluid is incompressible so it has the same amount everywhere. The smoke is kind of like a fluid moving inside a fluid and it can vary from place to place. 
//
//					// This code just sets the value of the smoke to a changing value to better show changes and it also looks cool. Could just have different smokes values at different points and just move them.
//					//fluid.at(fluid.smoke, x, y) = 0.5f + 0.5f * sin(elapsed / Time::deltaTime() * 0.1f);
//					//fluid.spanFrom(fluid.smoke)(x, y) = 0.5f;
//					//smoke(x, y) = 0.5f;
//					fluid.spanFrom(fluid.velX)(x, y) = vel.x;
//					fluid.spanFrom(fluid.velX)(x + 1, y) = vel.x;
//					fluid.spanFrom(fluid.velY)(x, y) = vel.y;
//					fluid.spanFrom(fluid.velY)(x, y + 1) = vel.y;
//				}
//
//			}
//		}
//	}
//	if (Input::isMouseButtonUp(MouseButton::LEFT)) {
//		obstacleReleased = true;
//	}
//	
//	chk(showDivergence, false) {
//		for (auto pixel : image.indexed()) {
//			pixel = Pixel32(Vec3(fluid.at(fluid.divergence, pixel.pos.x, pixel.pos.y)));
//		}
//	} else {
//		for (auto pixel : image.indexed()) {
//			/*pixel = Pixel32(Vec3(fluid.at(fluid.smoke, pixel.pos.x, pixel.pos.y)));*/
//			/*pixel = Pixel32(Vec3(smoke(pixel.pos.x, pixel.pos.y)));*/
//			pixel = Pixel32(Vec3(smokeR(pixel.pos.x, pixel.pos.y), smokeG(pixel.pos.x, pixel.pos.y), smokeB(pixel.pos.x, pixel.pos.y)));
//			//pixel = Pixel32(Vec3(pixel.pos.x, pixel.pos.y, 0.0f) / Vec3(fluid.gridSize.x, fluid.gridSize.y, 0.0f));
//		}
//	}
//	
//	renderer.drawImage(image.data(), image.size(), transform);
//
//	renderer.update();
//}
