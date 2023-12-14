#include <water_simulation/MainLoop.hpp>
#include <glad/glad.h>
#include <StructUtils.hpp>
#include <framework/ShaderManager.hpp>
#include <framework/Camera.hpp>
#include <engine/Input/Input.hpp>
#include <framework/Instancing.hpp>
#include <Dbg.hpp>
#include <framework/Dbg.hpp>
#include <engine/Math/Random.hpp>
#include <engine/Math/Color.hpp>
#include <engine/Math/Polygon.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Math/Utils.hpp>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Math/OdeIntegration/Euler.hpp>

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

	return value;
}

void MainLoop::setSmoke(i64 x, i64 y, Vec3 color) {
	smokeR(x, y) = color.x;
	smokeG(x, y) = color.y;
	smokeB(x, y) = color.z;
}

// TODO: Using rk4 to advect control volumes in potential flows.

void MainLoop::update() {
	/*
	Cool:

	When you draw a circle in the center of the wind tunnel scene, vortex shedding starts to happen, creating a von karmann vortex street behind the circle. Vortices are created by low pressure zones. If you enable pressure visualization you will see those zones. After a vortex is created on the bottom a new one will form on the top. If you add colored smoke to the bottom of the circle you will see that only every other vortex gets colored.
	*/

	const auto gridSizeCameraSpace = fluid.cellSpacing * Vec2(fluid.gridSize);
	const auto gridCenter = gridSizeCameraSpace / 2.0f;

	renderer.camera.pos = gridCenter;
	renderer.camera.changeSizeToFitBox(gridSizeCameraSpace);

	const auto cursorPos = Input::cursorPosClipSpace() * renderer.camera.clipSpaceToWorldSpace();
	const auto cursorGridPos = Vec2T<i64>((cursorPos / fluid.cellSpacing).applied(floor));

	auto spawnParticlesCircle = [&] {
		particles.clear();
		i32 count = 1000;
		for (i32 i = 0; i < count; i++) {
			float angle = (i / static_cast<float>(count)) * TAU<float>;
			particles.push_back(gridCenter + Vec2::oriented(angle) * 0.5f);
		}
		initialArea = simplePolygonArea(particles);
	};

	auto spawnParticlesSquare = [&] {
		particles.clear();
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
		initialArea = simplePolygonArea(particles);
	};

	auto updateSimulation = [&] {
		auto enfornceBoundaryConditions = [&] {
			for (i64 x = 0; x < fluid.gridSize.x; x++) {
				for (i64 y = 0; y < fluid.gridSize.y; y++) {
					if (fluid.isWall(x, y) || x == 0 || y == 0 || x == fluid.gridSize.x - 1 || y == fluid.gridSize.y - 1) {
						setSmoke(x, y, Vec3(0.0f));
					}
				}
			}

			if (scene == Scene::WIND_TUNNEL) {
				const auto peroid = 20;
				for (i64 y = 0; y < fluid.gridSize.y - 1; y += peroid) {
					const auto width = 10;
					for (i64 yi = y; yi < y + width; yi++) {
						setSmoke(1, yi, Vec3(1.0f));
					}
					for (i64 yi = y + width; yi < y + peroid; yi++) {
						setSmoke(1, yi, Vec3(0.0f));
					}
				}
			}

			switch (scene) {
				using enum Scene;

			case BOX:
				for (i64 x = 0; x < fluid.gridSize.x; x++) {
					for (i64 y = 0; y < fluid.gridSize.y; y++) {
						if (x == 0 || y == 0 || x == fluid.gridSize.x - 1 || y == fluid.gridSize.y - 1) {
							fluid.setIsWall(x, y, true);
						}
					}
				}

				for (i64 x = 0; x < fluid.gridSize.x; x++) {
					if (fluid.isWall(x, fluid.gridSize.y - 1)) {
						fluid.at(fluid.velX, x, 0) = 0.0f;
						fluid.at(fluid.velY, x, 0) = 0.0f;
						fluid.at(fluid.velX, x, fluid.gridSize.y - 1) = 0.0f;
						fluid.at(fluid.velY, x, fluid.gridSize.y - 1) = 0.0f;
					}
				}

				for (i64 y = 0; y < fluid.gridSize.y; y++) {
					if (fluid.isWall(fluid.gridSize.x - 1, y)) {
						fluid.at(fluid.velX, 0, y) = 0.0f;
						fluid.at(fluid.velY, 0, y) = 0.0f;
						fluid.at(fluid.velX, fluid.gridSize.x - 1, y) = 0.0f;
						fluid.at(fluid.velY, fluid.gridSize.x - 1, y) = 0.0f;
					}
				}

				for (i64 y = 0; y < fluid.gridSize.y - 1; y++) {
					for (i64 x = 0; x < fluid.gridSize.x - 1; x++) {
						if (fluid.isWall(x, y)) {
							fluid.removeVelocityAround(x, y);
						}
					}
				}

				break;

			case WIND_TUNNEL:
				for (i64 i = 0; i < fluid.gridSize.x; i++) {
					for (i64 j = 0; j < fluid.gridSize.y; j++) {
						if (i == 0 || j == 0 || j == fluid.gridSize.y - 1) {
							fluid.setIsWall(i, j, true);
						} else if (i == fluid.gridSize.x - 1) {
							fluid.setIsWall(i, j, false);
						}

						if (i == 1) {
							//f.u[i * n + j] = inVel;
							fluid.at(fluid.velX, i, j) = windTunnelVelocity;
						}
					}
				}

				for (i64 y = 1; y < fluid.gridSize.y - 1; y++) {
					for (i64 x = 1; x < fluid.gridSize.x - 1; x++) {
						if (fluid.isWall(x, y)) {
							fluid.removeVelocityAround(x, y);
						}
					}
				}
				break;
			}
		};

		auto updateParticles = [&] {
			for (auto& particle : particles) {
				particle += fluid.sampleVel(particle) * dt;
				if (particle.x < 0.0f) {
					particle.x = 0.0f;
				}
				if (particle.y < 0.0f) {
					particle.y = 0.0f;
				}

				if (particle.x > gridSizeCameraSpace.x) {
					particle.x = gridSizeCameraSpace.x;
				}
				if (particle.y > gridSizeCameraSpace.y) {
					particle.y = gridSizeCameraSpace.y;
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
					if (dist > 0.01f) {
						newParticles.push_back((particles[i] + particles[i + 1]) / 2.0f);
					}
				}
				newParticles.push_back(particles[particles.size() - 1]);
				particles = newParticles;
			}
		};

		auto updatePathlineParticles = [&] {
			for (auto& particle : pathlineParticles) {
				auto rhs = [&](Vec2 x, float t) {
					return fluid.sampleVel(x);
				};
				switch (particle.integrationMethod) {
					using enum IntegrationMethod;

				case EULER:
					particle.pos = eulerStep(rhs, particle.pos, 0.0f, dt);
					break;

				case RK4:
					particle.pos = rungeKutta4Step(rhs, particle.pos, 0.0f, dt);
					break;

				}
				if (particle.positionHistory.size() >= 1 && distance(particle.pos, particle.positionHistory.back()) < 0.05f) {
					continue;
				}
				particle.positionHistory.push_back(particle.pos);
			}
		};

		enfornceBoundaryConditions();

		if (!paused) {
			elapsed += dt;
			fluid.update(dt, 0.0f, 40);
			fluid.advectQuantity(smokeR.span2d(), dt);
			fluid.advectQuantity(smokeG.span2d(), dt);
			fluid.advectQuantity(smokeB.span2d(), dt);

			updateParticles();
			updatePathlineParticles();
		}
	};

	auto processInput = [&] {
		if (Input::isKeyDown(KeyCode::X)) {
			paused = !paused;
		}

		auto updateVelocityBrush = [&] {
			if (cycleColors) {
				velocityBrushSmokeColor = Color3::fromHsv(elapsed * colorCyclingSpeed, 1.0f, 1.0f);
			}

			float radius = brushRadiusCellCount * fluid.cellSpacing;
			if (Input::isMouseButtonHeld(MouseButton::LEFT)) {
				const auto newPos = cursorPos;
				Vec2 vel;
				if (velocityBrushReleased) {
					velocityBrushReleased = false;
					vel = Vec2(0.0f);
				} else {
					vel = (newPos - velocityBrushPos) / dt;
				}
				velocityBrushPos = newPos;

				for (i64 x = 1; x < fluid.gridSize.x - 2; x++) {
					for (i64 y = 1; y < fluid.gridSize.y - 2; y++) {
						const auto cellPos = Vec2(Vec2T(x, y)) * fluid.cellSpacing;
						const auto n = fluid.gridSize.y;
						if (!fluid.isWall(x, y) && (cellPos - velocityBrushPos).lengthSq() < pow(radius, 2.0f)) {
							fluid.at(fluid.velX, x, y) = vel.x;
							fluid.at(fluid.velX, x + 1, y) = vel.x;
							fluid.at(fluid.velY, x, y) = vel.y;
							fluid.at(fluid.velY, x, y + 1) = vel.y;
							if (velocityBrushAddSmoke) {
								setSmoke(x, y, velocityBrushSmokeColor);
							}
						}

					}
				}
			}
			if (Input::isMouseButtonUp(MouseButton::LEFT)) {
				velocityBrushReleased = true;
			}
		};
		
		auto updateWallBrush = [&] {
			std::optional<bool> wallValue;

			if (Input::isMouseButtonHeld(MouseButton::RIGHT)) {
				wallValue = true;
			} else if (Input::isMouseButtonHeld(MouseButton::MIDDLE)) {
				wallValue = false;
			}

			if (wallValue.has_value()) {
				const auto r = Vec2T<i64>(brushRadiusCellCount + 1);
				const auto min = (cursorGridPos - r).clamped(Vec2T<i64>{ 1 }, fluid.gridSize - Vec2T<i64>{ 2 });
				const auto max = (cursorGridPos + r).clamped(Vec2T<i64>{ 1 }, fluid.gridSize - Vec2T<i64>{ 2 });
				for (i64 x = min.x; x <= max.x; x++) {
					for (i64 y = min.y; y <= max.y; y++) {
						if (Vec2(cursorGridPos).distanceTo(Vec2(x, y)) < brushRadiusCellCount) {
							fluid.setIsWall(x, y, *wallValue);
						}
					}
				}
			}

			brushRadiusCellCount += Input::scrollDelta();
			brushRadiusCellCount = std::max(1.0f, brushRadiusCellCount);
		};

		auto updateSpawnParticles = [&] {
			if (Input::isKeyDown(KeyCode::P)) {
				spawnParticlesCircle();
			}

			if (Input::isKeyDown(KeyCode::O)) {
				spawnParticlesSquare();
			}
		};

		auto updateSpawnPathlineParticles = [&] {
			if (Input::isKeyDown(KeyCode::H)) {
				const auto position = cursorPos;
				pathlineParticles.push_back(PathlineParticle{
					.pos = position,
					.color = random01Vec3(),
					.integrationMethod = IntegrationMethod::EULER,
					.positionHistory = { position },
				});
			}
		};

		updateVelocityBrush();
		updateWallBrush();
		updateSpawnParticles();
		updateSpawnPathlineParticles();
	};

	auto displayGui = [&] {
		ImGui::SeparatorText("settings");
		ImGui::Checkbox("paused", &paused);

		ImGui::Combo("display", reinterpret_cast<int*>(&displayState), displayStateNames);

		if (displayState == DisplayState::PRESSURE) {
			ImGui::Checkbox("use default displayed pressure bounds", &useDefaultDisplayedPressureBounds);
			if (!useDefaultDisplayedPressureBounds) {
				ImGui::InputFloat("displayed pressure min", &displayedPressureMin);
				ImGui::InputFloat("displayed pressure max", &displayedPressureMax);
			}
		}

		ImGui::Checkbox("draw streamlines", &drawStreamlines);

		ImGui::Combo("scene", reinterpret_cast<int*>(&scene), sceneNames);
		if (scene == Scene::WIND_TUNNEL) {
			ImGui::InputFloat("wind tunnel velocity", &windTunnelVelocity);
		}

		ImGui::Checkbox("velocity brush add smoke", &velocityBrushAddSmoke);
		if (velocityBrushAddSmoke) {
			ImGui::Checkbox("cycle colors", &cycleColors);

			if (cycleColors) {
				ImGui::InputFloat("color cycling speed", &colorCyclingSpeed);
			}

			if (cycleColors) {
				ImGui::BeginDisabled();
			}
			ImGui::ColorEdit3("velocity brush smoke color", velocityBrushSmokeColor.data());
			if (cycleColors) {
				ImGui::EndDisabled();
			}
		}

		if (ImGui::Button("clear smoke")) {
			std::ranges::fill(smokeR.span(), 0.0f);
			std::ranges::fill(smokeG.span(), 0.0f);
			std::ranges::fill(smokeB.span(), 0.0f);
		}

		ImGui::SeparatorText("volume particles");
		if (ImGui::Button("spawn particles circle")) {
			spawnParticlesCircle();
		}

		if (ImGui::Button("spawn particles square")) {
			spawnParticlesSquare();
		}

		if (particles.size() != 0 && ImGui::Button("remove particles")) {
			particles.clear();
		}

		Gui::put("particle count: %", particles.size());
		if (initialArea.has_value()) {
			const auto areaScaling = simplePolygonArea(particles) / *initialArea;
			Gui::put("area scaling %", areaScaling);
		}

		ImGui::SeparatorText("pathline particles");
		ImGui::Text("Press H to spawn particle");
		std::optional<i32> particleToRemoveIndex;
		for (i32 i = 0; i < pathlineParticles.size(); i++) {
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode(std::to_string(i).c_str())) {
				auto& particle = pathlineParticles[i];
				ImGui::Combo("integration method", reinterpret_cast<int*>(&particle.integrationMethod), integrationMethodNames);
				ImGui::ColorEdit3("color", particle.color.data());
				if (ImGui::Button("remove")) {
					particleToRemoveIndex = i;
				}
				ImGui::TreePop();
			}
		}

		if (particleToRemoveIndex.has_value()) {
			pathlineParticles.erase(pathlineParticles.begin() + *particleToRemoveIndex);
		}

		ImGui::SeparatorText("isobars");
		ImGui::Checkbox("draw automatic isobars", &drawAutomaticIsobars);

		if (drawAutomaticIsobars) {
			ImGui::InputFloat("automatic isobars min", &automaticIsobarsMin);
			ImGui::InputFloat("automatic isobars max", &automaticIsobarsMax);
			ImGui::InputFloat("automatic isobars step", &automaticIsobarsStep);
		} else {
			std::optional<i32> isobarToRemoveIndex;

			if (ImGui::Button("add")) {
				isobars.push_back(Isobar{
					.color = Color3::WHITE,
					.value = 0.0f,
				});
			}
			for (i32 i = 0; i < isobars.size(); i++) {
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::TreeNode(std::to_string(i).c_str())) {
					auto& isobar = isobars[i];
					ImGui::ColorEdit3("color", isobar.color.data());
					ImGui::InputFloat("value", &isobar.value);
					if (ImGui::Button("remove")) {
						isobarToRemoveIndex = i;
					}
					ImGui::TreePop();
				}
			}

			if (isobarToRemoveIndex.has_value()) {
				isobars.erase(isobars.begin() + *isobarToRemoveIndex);
			}
		}

		ImGui::SeparatorText("info");

		const auto maxDivergence = *std::ranges::max_element(fluid.divergence);
		const auto minDivergence = *std::ranges::max_element(fluid.divergence);
		Gui::put("min divergence: %", minDivergence);
		Gui::put("max divergence: %", maxDivergence);

		const auto pressureMin = *std::ranges::min_element(fluid.pressure);
		const auto pressureMax = *std::ranges::max_element(fluid.pressure);
		Gui::put("pressure min: %", pressureMin);
		Gui::put("pressure max: %", pressureMax);

		if (useDefaultDisplayedPressureBounds) {
			displayedPressureMin = pressureMin;
			displayedPressureMax = pressureMax;
		}
	};

	auto render = [&] {
		glClear(GL_COLOR_BUFFER_BIT);

		switch (displayState) {
			using enum DisplayState;

		case SMOKE:
			for (auto& pixel : image.indexed()) {
				pixel = Pixel32(Vec3(smokeR(pixel.pos.x, pixel.pos.y), smokeG(pixel.pos.x, pixel.pos.y), smokeB(pixel.pos.x, pixel.pos.y)));
			}
			break;

		case DIVERGENCE:
			for (auto& pixel : image.indexed()) {
				/*pixel = Pixel32(Vec3(fluid.at(fluid.divergence, pixel.pos.x, pixel.pos.y)));*/
				pixel = Pixel32(Vec3(fluid.at(fluid.velY, pixel.pos.x, pixel.pos.y)));
			}
			break;

		case PRESSURE:
			for (auto& pixel : image.indexed()) {
				pixel = Pixel32(Color3::scientificColoring(
					fluid.at(fluid.pressure, pixel.pos.x, pixel.pos.y),
					displayedPressureMin,
					displayedPressureMax));
			}
			break;
		}
		for (auto& pixel : image.indexed()) {
			if (fluid.isWall(pixel.pos.x, pixel.pos.y)) {
				pixel = Pixel32(Color3::WHITE / 3.0f);
			}
		}

		Dbg::drawCircle(cursorPos, brushRadiusCellCount * fluid.cellSpacing);

		auto transform = renderer.camera.makeTransform(gridCenter, 0.0f, gridSizeCameraSpace / 2.0f);

		renderer.drawImage(image.span2d().asConst(), transform);
		Dbg::drawPolygon(particles, Color3::BLACK, 0.01f);

		for (const auto& particle : pathlineParticles) {
			static constexpr float pathlineParticleTrailWidth = 0.01f;
			Dbg::drawDisk(particle.pos, 0.02, particle.color);
			Dbg::drawPolyline(particle.positionHistory, particle.color, pathlineParticleTrailWidth);
			if (particle.positionHistory.size() > 1 && particle.pos != particle.positionHistory.back()) {
				Dbg::drawLine(particle.pos, particle.positionHistory.back(), particle.color, pathlineParticleTrailWidth);
			}
		}

		if (drawStreamlines) {
			for (i64 xi = 2; xi < fluid.gridSize.x; xi += 5) {
				for (i64 yi = 2; yi < fluid.gridSize.y; yi += 5) {
					const auto startPos = (Vec2(xi, yi) + Vec2(0.5f)) * fluid.cellSpacing;
					const auto stepCount = 15;
					const auto streamlineStepSize = 0.01f;

					auto pos = startPos;
					for (i32 i = 0; i < stepCount; i++) {
						const auto rhs = [&](Vec2 x, float t) {
							return fluid.sampleVel(x);
						};
						const auto oldPos = pos;
						pos = eulerStep(rhs, pos, 0.0f, streamlineStepSize);
						Dbg::drawLine(oldPos, pos, Color3::WHITE, 0.005f);
					}
				}
			}
		}

		for (int x = 0; x < fluid.gridSize.x; x++) {
			for (int y = 0; y < fluid.gridSize.y; y++) {
				if (fluid.isWall(x, y)) {
					fluid.at(fluid.pressure, x, y) = std::numeric_limits<float>::quiet_NaN();
				}
			}
		}

		auto scaleMarchingSquaresOutput = [&] {
			for (auto& line : marchingSquaresOutput) {
				auto transform = [&](Vec2 v) {
					v *= fluid.cellSpacing;
					return v;
				};
				line.a = transform(line.a);
				line.b = transform(line.b);
			}
		};

		if (drawAutomaticIsobars) {
			if (automaticIsobarsMin < automaticIsobarsMax && automaticIsobarsStep > 0.0f) {
				for (float value = automaticIsobarsMin; value < automaticIsobarsMax; value += automaticIsobarsStep) {
					marchingSquares2(marchingSquaresOutput, fluid.spanFrom(fluid.pressure).asConst(), value, true);
					scaleMarchingSquaresOutput();
					for (const auto& line : marchingSquaresOutput) {
						Dbg::drawLine(line.a, line.b, Color3::WHITE, 0.005f);
					}
				}
			}
		} else {
			for (const auto& isobar : isobars) {
				marchingSquares2(marchingSquaresOutput, fluid.spanFrom(fluid.pressure).asConst(), isobar.value, true);
				scaleMarchingSquaresOutput();

				for (const auto& line : marchingSquaresOutput) {
					Dbg::drawLine(line.a, line.b, isobar.color, 0.005f);
				}
			}
		}

		ShaderManager::update();
		renderer.update();
	};

	updateSimulation();
	processInput();
	displayGui();
	render();
}