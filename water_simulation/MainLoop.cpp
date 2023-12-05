#include <water_simulation/MainLoop.hpp>
#include <glad/glad.h>
#include <StructUtils.hpp>
#include <framework/ShaderManager.hpp>
#include <framework/Camera.hpp>
#include <engine/Input/Input.hpp>
#include <framework/Instancing.hpp>
#include <engine/Math/Random.hpp>
#include <Dbg.hpp>
#include <framework/Dbg.hpp>
#include <engine/Math/Color.hpp>
#include <engine/Math/Polygon.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/Utils.hpp>

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
		.renderer = Renderer2d::make(),
	};

	for (i64 x = 0; x < gridSize.x; x++) {
		for (i64 y = 0; y < gridSize.y; y++) {
			value.smokeR(x, y) = imageResized(x, y).r / 255.0f;
			value.smokeG(x, y) = imageResized(x, y).g / 255.0f;
			value.smokeB(x, y) = imageResized(x, y).b / 255.0f;
		}
	}

	/*i32 size = 10;
	for (i32 x = 0; x < size; x++) {
		value.particles.push_back(Vec2((x / static_cast<float>(size)) * 0.2f + 0.5, 0.5f));
	}*/

	return value;
}

void MainLoop::update() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	fluid.update(dt, 0.0f, 40);
	fluid.advectQuantity(smokeR.span2d(), dt);
	fluid.advectQuantity(smokeG.span2d(), dt);
	fluid.advectQuantity(smokeB.span2d(), dt);

	const auto gridSize = fluid.cellSpacing * Vec2(fluid.gridSize);
	const auto gridCenter = gridSize / 2.0f;
	renderer.camera.pos = gridCenter;
	renderer.camera.changeSizeToFitBox(gridSize);

	auto transform = renderer.camera.makeTransform(gridCenter, 0.0f, gridSize / 2.0f);

	const auto cursorPos = Input::cursorPosClipSpace() * renderer.camera.clipSpaceToWorldSpace();
	const auto cursorGridPos = Vec2T<i64>((cursorPos / fluid.cellSpacing).applied(floor));

	static Vec2 obstaclePos(0.0f);

	float obstacleRadius = fluid.cellSpacing * 5.0f;
	static bool obstacleReleased = true;
	if (Input::isMouseButtonHeld(MouseButton::LEFT)) {
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
					fluid.at(fluid.velX, x, y) = vel.x;
					fluid.at(fluid.velX, x + 1, y) = vel.x;
					fluid.at(fluid.velY, x, y) = vel.y;
					fluid.at(fluid.velY, x, y + 1) = vel.y;
				}

			}
		}
	}
	if (Input::isMouseButtonUp(MouseButton::LEFT)) {
		obstacleReleased = true;
	}

	if (Input::isKeyDown(KeyCode::P)) {
		i32 count = 1000;
		for (i32 i = 0; i < count; i++) {
			float angle = (i / static_cast<float>(count)) * TAU<float>;
			particles.push_back(gridCenter + Vec2::oriented(angle) * 0.5f);
		}
	}

	if (Input::isKeyDown(KeyCode::O)) {
		i32 perSide = 100;
		float size = 0.5f;

		for (i32 xi = 0; xi < perSide; xi++) {
			float t = xi / static_cast<float>(perSide);
			const auto x = lerp(-size / 2.0f, size / 2.0f, t);
			particles.push_back(gridCenter + Vec2(x, size / 2.0f));
		}

		for (i32 yi = 1; yi < perSide - 1; yi++) {
			float t = yi / static_cast<float>(perSide);
			const auto y = lerp(size / 2.0f, -size / 2.0f, t);
			particles.push_back(gridCenter + Vec2(size / 2.0f, y));
		}

		for (i32 xi = 0; xi < perSide; xi++) {
			float t = xi / static_cast<float>(perSide);
			const auto x = lerp(size / 2.0f, -size / 2.0f, t);
			particles.push_back(gridCenter + Vec2(x, -size / 2.0f));
		}

		for (i32 yi = 1; yi < perSide - 1; yi++) {
			float t = yi / static_cast<float>(perSide);
			const auto y = lerp(-size / 2.0f, size / 2.0f, t);
			particles.push_back(gridCenter + Vec2(-size / 2.0f, y));
		}
	}

	chk(showDivergence, false) {
		for (auto& pixel : image.indexed()) {
			pixel = Pixel32(Vec3(fluid.at(fluid.divergence, pixel.pos.x, pixel.pos.y)));
		}
	} else {
		for (auto& pixel : image.indexed()) {
			pixel = Pixel32(Vec3(smokeR(pixel.pos.x, pixel.pos.y), smokeG(pixel.pos.x, pixel.pos.y), smokeB(pixel.pos.x, pixel.pos.y)));
		}
	}

	for (auto& particle : particles) {
		particle += fluid.sampleVel(particle) * dt;
		if (particle.x < 0.0f) {
			particle.x = 0.0f;
		}
		if (particle.y < 0.0f) {
			particle.y = 0.0f;
		}

		if (particle.x > gridSize.x) {
			particle.x = gridSize.x;
		}
		if (particle.y > gridSize.y) {
			particle.y = gridSize.y;
		}
	}

	std::vector<i64> particlesToRemoveIndices;

	if (particles.size() > 0) {
		Vec2 lastNotRemovedParticlePos = particles[0];

		for (i64 i = 1; i < static_cast<i64>(particles.size()) - 1; i++) {
			const auto dist = lastNotRemovedParticlePos.distanceTo(particles[i]);
			if (dist < 0.01f) {
				particlesToRemoveIndices.push_back(i);
			} else {
				lastNotRemovedParticlePos = particles[i];
			}
		}
		// It would probably be most efficient to just rebuild the vector without the ereased elements.
		for (auto it = particlesToRemoveIndices.crbegin(); it != particlesToRemoveIndices.crend(); ++it) {
			particles.erase(particles.begin() + *it);
		}
	}

	if (particles.size() > 0) {
		std::vector<Vec2> newParticles;
		for (i64 i = 0; i < static_cast<i64>(particles.size()) - 1; i++) {
			const auto dist = particles[i].distanceTo(particles[i + 1]);
			newParticles.push_back(particles[i]);
			if (dist > 0.05f) {
				newParticles.push_back((particles[i] + particles[i + 1]) / 2.0f);
			}
		}
		newParticles.push_back(particles[particles.size() - 1]);
		particles = newParticles;
	}
	
	if (initialArea == -1.0f && particles.size() > 0) {
		initialArea = simplePolygonArea(particles);
	}
	const auto areaScaling = simplePolygonArea(particles) / initialArea;
	//dbgGui(simplePolygonArea(particles));
	dbgGui(areaScaling);

	dbgGui(particles.size());
	//Gui::put("particle count: %", )


	/*std::vector<i64> particlesToAddIndices;
	for (i64 i = 0; i < particles.size() - 1; i++) {
		const auto dist = particles[i].distanceTo(particles[i + 1]);
		if (dist > 0.03f) {
			particlesToAddIndices.push_back(i);
		}
	}*/

	/*renderer.drawImage(image.span2d().asConst(), transform);
	Dbg::drawPolygon(particles, Color3::BLACK, 0.01f);*/
	/*for (const auto& particle : particles) {
		Dbg::drawDisk(particle, 0.05f, Color3::WHITE);
	}*/
	Dbg::drawLine(gridCenter - Vec2(0.5f, -0.5f), gridCenter + Vec2(0.5f, -0.5f), Color3::BLUE);
	Dbg::drawDisk(gridCenter, 0.2f, Color3::RED);
	Dbg::drawDisk(cursorPos, 0.2f, Color3::GREEN);
	Dbg::drawLine(gridCenter - Vec2(0.5f), gridCenter + Vec2(0.5f), Color3::BLUE);
	renderer.update();
}