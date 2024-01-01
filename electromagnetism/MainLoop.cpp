#include <electromagnetism/MainLoop.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Color.hpp>
#include <imgui/imgui.h>
#include <glad/glad.h>
using namespace ImGui;
#include <engine/Math/MarchingSquares.hpp>
#include <engine/Math/Utils.hpp>
#include <framework/Dbg.hpp>
#include <Timer.hpp>
#include <engine/Window.hpp>
#include <iostream>

#define PATH "C:/Users/user/Desktop/bin/"

static Vec2T<int> sizee;

float ssss = 2.25f;

MainLoop::MainLoop()
	: texture{ []() {
		const auto image = Image32::fromFile(PATH "out1.png");
		sizee = Vec2T<int>(image->size());
		//const auto image = ImageRgba::fromFile("C:/Users/user/Desktop/bez.png");
		/*Image32 texture(image->size().x / 4, image->size().y / 4);*/
		/*Image32 texture(image->size().x / 6, image->size().y / 6);*/
		/*Image32 texture(image->size().x / 6, image->size().y / 6);*/
		Image32 texture(image->size().x / 6, image->size().y / 6);
		/*Image32 texture(image->size().x, image->size().y);*/
		//DynamicTexture texture{ image->size() / 10 };
		if (image.has_value()) {
			texture.copyAndResize(*image);
		}
		return texture;
	}() }
	, renderer(Renderer2d::make()) {

	floatTexture = Array2d<float>(texture.size().x, texture.size().y);
	vectorField = Array2d<Vec2>(texture.size().x, texture.size().y);
	Window::minimize();
	Window::setSize(Vec2T<int>(Vec2(sizee) * ssss));
	std::cout << sizee.x * ssss << ' ' << sizee.y * ssss << '\n';
	std::cout << Window::size().x << ' ' << Window::size().y << '\n';

	for (i64 y = 0; y < texture.size().y; y++) {
		for (i64 x = 0; x < texture.size().x; x++) {
			particles.push_back(Particle{
				.pos = Vec2(x, y),
				.vel = Vec2(0.0f)
			});
		}
	}
	std::cout << 'a' << texture.size().x / Window::size().x << '\n';
	std::cout << 'a' << texture.size().y / Window::size().y << '\n';

	/*for (i64 y = 0; y < texture.size().y; y++) {
		for (i64 x = 0; x < texture.size().x; x++) {
			springs.push_back(Spring{
				.restPos = Vec2(x, y),
				.displacementFromRestPos = Vec2(0.0f, 0.0f),
				.vel = Vec2(0.0f)
			});
		}
	}*/

	//const auto scale = 5;
	const auto scale = 1;
	for (i64 y = 0 - 15; y < texture.size().y * scale + 15; y++) {
		for (i64 x = 0 - 15; x < texture.size().x * scale + 15; x++) {
			springs.push_back(Spring{
				.restPos = Vec2(x, y) / scale,
				.displacementFromRestPos = Vec2(0.0f, 0.0f),
				.vel = Vec2(0.0f)
			});
		}
	}

}

auto MainLoop::update() -> void {
	ImGui::Text("%g %g", Window::size().x, Window::size().y);
	ImGui::Text("%g %g", sizee.x * ssss, sizee.y * ssss);
	Timer timr;

	const auto gridSize = Vec2{ texture.size() };
	const auto gridCenter = gridSize / 2.0f;
	renderer.camera.pos = gridCenter;
	renderer.camera.setWidth(gridSize.x);

	//Dbg::drawDisk(gridCenter, 3.0f);

	float scroll = Input::scrollDelta();
	if (scroll > 0.0f) {
		renderer.camera.zoom *= 1.05f;
	}
	if (scroll < 0.0f) {
		renderer.camera.zoom /= 1.05f;
	}
	//camera.scrollOnCursorPos();

	if (frame <= 444) {
		swappedColors = false;
	} else if (frame <= 821) {
		swappedColors = true;
	} else if (frame <= 1263) {
		swappedColors = false;
	} else if (frame <= 1686) {
		swappedColors = true;
	} else if (frame <= 3332) {
		swappedColors = false;
	}

	Begin("marching squares");
	Checkbox("draw image", &drawImage);
	//Checkbox("pixel perfect", &pixelPerfect);
	//Checkbox("connect diagonals", &connectDiagonals);
	End();

	if (Input::isKeyDown(KeyCode::A)) {
		paused = !paused;
	}


	if (Input::isKeyDown(KeyCode::LEFT)) {
		frame--;
	}

	if (Input::isKeyDown(KeyCode::RIGHT)) {
		frame++;
	}

	if (!paused) {
		frame += 1;
		//frame += 2;
	}
		

	auto image = Image32::fromFile((PATH "out" + std::to_string(frame / 2) + std::string(".png")).data());
	if (image.has_value()) {

		texture.copyAndResize(*image);

		visited.clear();
		vertices.clear();
		texture.copyAndResize(*image);

		for (i64 y = 0; y < texture.size().y; y++) {
			for (i64 x = 0; x < texture.size().x; x++) {
				floatTexture(x, y) = texture(x, y).r > 127 ? 1.0f : -1.0f;
			}
		}

		//for (auto& p : texture) {
		//	floatTexture.at()
		//	//const auto grayscaled = Color3::toGrayscale(p.color3());
		//	// 
		//	//p = p.r > 127 ? Pixel32(Color3::WHITE) : Pixel32(Color3::BLACK);
		//}
		//floatTexture.
		vertices = ::marchingSquares(floatTexture.span2d().asConst(), pixelPerfect, connectDiagonals, 0.0f);

		for (i64 y = 0; y < texture.size().y / 2; y++) {
			for (i64 x = 0; x < texture.size().x; x++) {
				std::swap(texture(x, y), texture(x, texture.size().y - 1 - y));
			}
		}
	}

	/*for (const auto& shape : vertices) {
		Dbg::drawPolygon(shape, Color3::WHITE, 0.2f);
	}*/
	//float particleRadius = 0.250f;
	float springRadius = 0.250f / 2.0f;

	// TODO: !!!!!! Make a function vector that takes in a function that takes in as input the axis.
	// vec.forAllAxis
	// TODO: Make a function for bilinear interpolation.
	auto sampleVectorField = [this](Vec2 pos) {
		auto i0 = Vec2T<i64>(pos.applied(floor));
		/*if (i0.x < -5 || i0.y < -5) {
			return Vec2(0.0f);
		}*/
		for (i32 i = 0; i < 2; i++) {
			if (i0[i] < 0) {
				i0[i] = 0;
				/*pos[i] = 0.0f;*/
				pos[i] -= pos[i];
			}
		}
		auto i1 = i0 + Vec2T<i64>(1);
		for (i32 i = 0; i < 2; i++) {
			if (i0[i] > texture.size()[i] - 1) {
				i0[i] = texture.size()[i] - 1;
			}
			if (i1[i] > texture.size()[i] - 1) {
				i1[i] = texture.size()[i] - 1;
			}
		}

		const auto v00 = vectorField(i0.x, i0.y);
		const auto v10 = vectorField(i1.x, i0.y);
		const auto v01 = vectorField(i0.x, i1.y);
		const auto v11 = vectorField(i1.x, i1.y);
		Vec2 t = pos - Vec2(i0);

		const auto y0 = lerp(v00, v10, t.x);
		const auto y1 = lerp(v01, v11, t.x);
		const auto v = lerp(y0, y1, t.y);
		return v;
	};

	if (!paused) {
		for (auto& spring : springs) {
			const auto pos = spring.restPos + spring.displacementFromRestPos;
			auto a = Vec2T<i64>(pos.applied(floor));

			const auto dt = (1.0f / 60.0f);
			/*float strength = 0.1f;*/
			/*float strength = 4.0f;*/
			//float strength = 4.0f;
			/*spring.vel += vectorField(a.x, a.y) * strength * dt;*/

			float strength = 5.0f;
			spring.vel += sampleVectorField(pos) * strength * dt;
			spring.vel *= 0.98f; // damping
			spring.vel -= spring.displacementFromRestPos * 32.0f * dt;
			const auto velLength = spring.vel.length();
			/*const auto s = 7.0f;
			if (velLength > s) {
				spring.vel = spring.vel * (s / velLength);
			}*/
			spring.displacementFromRestPos += spring.vel * dt;


			//float strength = 6.0f;
			//spring.vel += sampleVectorField(pos) * strength * dt;
			//spring.vel *= 0.96f; // damping
			//spring.vel -= spring.displacementFromRestPos * 32.0f * dt;
			//spring.displacementFromRestPos += spring.vel * dt;


			//float strength = 8.0f;
			//spring.vel += sampleVectorField(pos) * strength * dt;
			//spring.vel *= 0.92f; // damping
			//spring.vel -= spring.displacementFromRestPos * 32.0f * dt;
			//spring.displacementFromRestPos += spring.vel * dt;

			//const auto dt = (1.0f / 60.0f);
			///*float strength = 0.1f;*/
			//float strength = 0.4f;
			//spring.vel += vectorField(a.x, a.y) * strength * dt;
			//spring.vel *= 0.99f; // damping
			//spring.vel -= spring.displacementFromRestPos * 0.5f * dt;
			//spring.displacementFromRestPos += spring.vel * dt;
		}
	}

	//if (!paused) {
	//	for (auto& particle : particles) {
	//		/*if (particle.x < -particleRadius) {
	//			particle.x = 0.0f;
	//		}
	//		if (particle.y < 0.0f) {
	//			particle.y = 0.0f;
	//		}
	//		if (particle.x > texture.size().x - 1) {
	//			particle.x = texture.size().x - 1;
	//		}
	//		if (particle.y > texture.size().y - 1) {
	//			particle.y = texture.size().y - 1;
	//		}*/

	//		/*if (particle.pos.x < 0.0f) {
	//			particle.pos.x = 0.0f;
	//		}
	//		if (particle.pos.y < 0.0f) {
	//			particle.pos.y = 0.0f;
	//		}
	//		if (particle.pos.x > texture.size().x - 1) {
	//			particle.pos.x = texture.size().x - 1;
	//		}
	//		if (particle.pos.y > texture.size().y - 1) {
	//			particle.pos.y = texture.size().y - 1;
	//		}*/

	//		if (particle.pos.x < -particleRadius) {
	//			particle.pos.x = texture.size().x + particleRadius;
	//		}
	//		if (particle.pos.y < -particleRadius) {
	//			particle.pos.y = texture.size().y + particleRadius;
	//		}

	//		if (particle.pos.x > texture.size().x + particleRadius) {
	//			particle.pos.x = -particleRadius;
	//		}
	//		if (particle.pos.y > texture.size().y + particleRadius) {
	//			particle.pos.y = -particleRadius;
	//		}

	//		auto a = Vec2T<i64>(particle.pos.applied(floor));
	//		if (a.x < 0) {
	//			a.x = 0;
	//		}
	//		if (a.y < 0) {
	//			a.y = 0;
	//		}
	//		if (a.x > texture.size().x - 1) {
	//			a.x = texture.size().x - 1;
	//		}
	//		if (a.y > texture.size().y - 1) {
	//			a.y = texture.size().y - 1;
	//		}

	//		const auto dt = (1.0f / 60.0f);
	//		float strength = 0.1f;
	//		particle.vel += vectorField(a.x, a.y) * strength * dt;
	//		particle.pos += particle.vel * dt;
	//	}
	//}

	for (i64 y = 0; y < texture.size().y; y++) {
		for (i64 x = 0; x < texture.size().x; x++) {
			vectorField(x, y) = Vec2(0.0f);
		}
	}

	Timer timer;
	for (const auto& polygon : vertices) {
		for (const auto& vertex : polygon) {
			//Dbg::drawDisk(vertex, 0.5f);
			for (i64 y = 0; y < texture.size().y; y++) {
				for (i64 x = 0; x < texture.size().x; x++) {
					Vec2 pos(x, y);
					const auto vector = pos - vertex;
					const auto lengthSquared = vector.lengthSq();
					if (lengthSquared != 0.0f) {
						vectorField(x, y) += vector / lengthSquared;
					}
				}
			}
		}
		//Dbg::drawPolygon(shape, Color3::WHITE, 0.2f);
	}

	/*for (i64 y = 0; y < texture.size().y; y++) {
		for (i64 x = 0; x < texture.size().x; x++) {
			for (const auto& polygon : vertices) {
				for (const auto& vertex : polygon) {
					Vec2 pos(x, y);
					const auto vector = pos - vertex;
					const auto lengthSquared = vector.lengthSq();
					if (lengthSquared != 0.0f) {
						vectorField(x, y) += vector / lengthSquared;
					}
				}
			}
		}*/
	//}

	timer.guiTookMiliseconds("update");

	/*for (i64 y = 0; y < texture.size().y; y++) {
		for (i64 x = 0; x < texture.size().x; x++) {
			vectorField(x, y) /= pointCount;
		}
	}*/
	

	float maxLength = std::numeric_limits<float>::min();

	for (i64 y = 0; y < texture.size().y; y++) {
		for (i64 x = 0; x < texture.size().x; x++) {
			const auto ray = vectorField(x, y);
			const auto length = ray.length();
			if (length > maxLength) {
				maxLength = length;
			}
		}
	}

	//const auto lineWidth = 0.10f;
	//const auto step = 1;
	//for (i64 y = 0; y < texture.size().y; y += step) {
	//	for (i64 x = 0; x < texture.size().x; x += step) {
	//		const auto ray = vectorField(x, y);
	//		const auto pos = Vec2(x, y) + Vec2(0.5f);
	//		const auto length = ray.length();
	//		const auto color = Color3::scientificColoring(length, 0.0f, maxLength * 0.85f);
	//		/*Dbg::drawLine(pos, pos + ray, color, lineWidth);*/
	//		const auto end = pos + ray.normalized() / 2.0f * step;
	//		Dbg::drawLine(pos, end, color, lineWidth);
	//		const auto direction = ray.angle();
	//		Dbg::drawLine(end, end - Vec2::oriented(direction + 0.8f) * 0.2f * step, color, lineWidth);
	//		Dbg::drawLine(end, end - Vec2::oriented(direction - 0.8f) * 0.2f * step, color, lineWidth);
	//	}
	//}

	/*for (const auto& particle : particles) {
		Dbg::drawDisk(particle.pos, particleRadius);
	}*/
	glClear(GL_COLOR_BUFFER_BIT);

	for (const auto& spring : springs) {
		//Dbg::drawDisk(spring.restPos + spring.displacementFromRestPos, springRadius, Color3::RED);
		const auto pos = Vec2T<i64>(spring.restPos + spring.displacementFromRestPos).clamped(Vec2T<i64>(0), texture.size() - Vec2T<i64>(1));
		const auto base = 0.7f;
		const auto value = texture(pos.x, pos.y).r / 255.0f;
		const auto color = lerp(base, 1.0f, value);
		//Dbg::drawDisk(spring.restPos + spring.displacementFromRestPos, springRadius, Vec3(color));
		Dbg::drawDisk(spring.restPos + spring.displacementFromRestPos, springRadius, Color3::WHITE);
	}

	auto transform = renderer.camera.makeTransform(gridCenter, 0.0f, gridSize / 2.0f);
	if (drawImage) {
		renderer.drawImage(texture.span2d().asConst(), transform);
	}
	glViewport(0, 0, Window::size().x, Window::size().y);
	renderer.update();
	Dbg::update();
	Image32 im(Window::size().x, Window::size().y);
	glReadPixels(0, 0, Window::size().x, Window::size().y, GL_RGBA, GL_UNSIGNED_BYTE, im.data());
	for (auto& pixel : im) {
		pixel.a = 255;
	}
	for (i64 y = 0; y < im.size().y / 2; y++) {
		for (i64 x = 0; x < im.size().x; x++) {
			std::swap(im(x, y), im(x, im.size().y - 1 - y));
		}
	}

	//if (frame % 2 == 0) {
	//	//Timer t;
	//	im.saveToBmp(("./output/o/out" + std::to_string(frame / 2) + std::string(".bmp")).data());
	//	//std::cout << "write" << t.elapsedMilliseconds() << "\n";
	//}
	timr.guiTookMiliseconds("frame");
}

//MainLoop::MainLoop() {
//
//}
//
//void MainLoop::update() {
//
//}