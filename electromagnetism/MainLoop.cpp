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
#include <engine/Math/OdeIntegration/Euler.hpp>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Window.hpp>
#include <iostream>

// TODO: !!!!!! Make a function vector that takes in a function that takes in as input the axis.
// vec.forAllAxis
// TODO: Make a function for bilinear interpolation.

#define PATH "C:/Users/user/Desktop/bin/"

static Vec2T<int> sizee;

MainLoop::MainLoop()
	: texture{ []() {
		const auto image = Image32::fromFile(PATH "out1.png");
		sizee = Vec2T<int>(image->size());
		/*Image32 texture(image->size().x / 3, image->size().y / 3);*/
		//Image32 texture(image->size().x / 2, image->size().y / 2);
		//Image32 texture(image->size().x / 1, image->size().y / 1);
		Image32 texture(image->size().x * 2, image->size().y * 2);
		if (image.has_value()) {
			texture.copyAndResize(*image);
		}
		return texture;
	}() }
	, renderer(Renderer2d::make()) {

	u = Array2d<float>(texture.size().x, texture.size().y);
	//u_t = Array2d<float>(texture.size().x, texture.size().y);
	u_t = Array2d<double>(texture.size().x, texture.size().y);
	textureFloat = Array2d<float>(texture.size().x, texture.size().y);
	std::ranges::fill(u.span(), 0.0f);
	std::ranges::fill(u_t.span(), 0.0f);
	/*Window::minimize();
	Window::setSize(Vec2T<int>(Vec2(sizee) * ssss));*/

}

auto MainLoop::update() -> void {
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


	Checkbox("draw image", &drawImage);

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
	}
		

	//auto image = Image32::fromFile((PATH "out" + std::to_string(frame / 2) + std::string(".png")).data());
	auto image = Image32::fromFile((PATH "out" + std::to_string(frame / 2) + std::string(".png")).data());
	if (image.has_value()) {
		texture.copyAndResize(*image);

		auto add = [&](i64 centerX, i64 centerY, float scale) {
			auto min = Vec2T<i64>(centerX - 5, centerY - 5).clamped(Vec2T<i64>(0, 0), u.size() - Vec2T<i64>(1));
			auto max = Vec2T<i64>(centerX + 5, centerY + 5).clamped(Vec2T<i64>(0, 0), u.size() - Vec2T<i64>(1));


			//u(centerX, centerY) = 1.0f;
			for (i64 yi = min.y; yi <= max.y; yi++) {
				for (i64 xi = min.x; xi <= max.x; xi++) {
					const auto x = ((xi - centerX));
					const auto y = ((yi - centerY));
					const auto r = Vec2(x, y).length();
					/*u(xi, yi) += (1.0f / sinh(r)) * scale;*/
					u(xi, yi) += (1.0f / cosh(r * 1.0f)) * scale;
					//u_t(xi, yi) += r;
				}
			}
		};

		if (!paused) {
			/*for (i64 y = 0; y < texture.size().y; y++) {
				for (i64 x = 0; x < texture.size().x; x++) {
					u(x, y) = 0.0f;
				}
			}*/

			for (i64 y = 0; y < texture.size().y; y++) {
				for (i64 x = 0; x < texture.size().x; x++) {
					u_t(x, y) *= 0.99f;
					u(x, y) *= 0.97f;
				}
			}

			for (i64 y = 0; y < texture.size().y; y++) {
				for (i64 x = 0; x < texture.size().x; x++) {
					textureFloat(x, y) = texture(x, y).r / 255.0f;
				}
			}

			std::vector<MarchingSquaresLine> lines;
			marchingSquares2(lines, textureFloat.span2d().asConst(), 0.5f, true);

			static bool set = false;
			const auto center = u.size() / 2;
			/*if (!set) {
				add(center.x, center.y, 200.0f);
				set = true;
			}*/
			for (const auto& line : lines) {
				const Vec2T<i64> pos(((line.a + line.b) / 2.0f).applied(floor));
				/*add(pos.x, pos.y, 0.01f);*/
				//add(pos.x, pos.y, 0.01f);
				const auto v = u(pos.x, pos.y);
				if (v < 1.0f) {
					add(pos.x, pos.y, 1.0f - v);
				}
					
			}

			//for (i64 y = 0; y < texture.size().y; y++) {
			//	for (i64 x = 0; x < texture.size().x; x++) {
			//		bool set = texture(x, y).r > 127;
			//		if (set) {
			//			add(x, y, 1.0f - u(x, y));
			//			//add(x, y, 0.1f);
			//		}
			//	}
			//}

			auto get = [&](i64 x, i64 y) {
				/*if (x < 0 || y < 0 || x >= u.size().x || y >= u.size().y) {
					return 0.0f;
				}*/
				x = std::clamp(x, 0ll, u.size().x - 1);
				y = std::clamp(y, 0ll, u.size().y - 1);
				return u(x, y);
			};
			const double dt = 1.0f / 180.0f;

			for (int i = 0; i < 3; i++) {
				for (i64 y = 0; y < u.size().y; y++) {
					for (i64 x = 0; x < u.size().x; x++) {

						//double c = pow(40.0, 2.0);
						///*float u_xx = get(x + 1, y) - 2.0f * get(x, y) + get(x - 1, y);
						//float u_yy = get(x, y + 1) - 2.0f * get(x, y) + get(x, y - 1);*/
						//double u_xx = static_cast<double>(get(x - 1, y)) - 2.0 * static_cast<double>(get(x, y)) + static_cast<double>(get(x + 1, y));
						//double u_yy = static_cast<double>(get(x, y - 1)) - 2.0 * static_cast<double>(get(x, y)) + static_cast<double>(get(x, y + 1));
						//double u_tt = c * (u_xx + u_yy);
						////eulerStep()
						//u_t(x, y) += u_tt * dt;

						//float c = pow(20.0f, 2.0f);
						float c = pow(70.0f, 2.0f);
						/*float u_xx = get(x + 1, y) - 2.0f * get(x, y) + get(x - 1, y);
						float u_yy = get(x, y + 1) - 2.0f * get(x, y) + get(x, y - 1);*/
						float u_xx = get(x - 1, y) - 2.0f * get(x, y) + get(x + 1, y);
						float u_yy = get(x, y - 1) - 2.0f * get(x, y) + get(x, y + 1);
						float u_tt = c * (u_xx + u_yy);
						//eulerStep()
						u_t(x, y) += u_tt * dt;
					}
				}

				for (i64 y = 0; y < u.size().y; y++) {
					for (i64 x = 0; x < u.size().x; x++) {
						u(x, y) += u_t(x, y) * dt;
					}
				}
			}
		}

		auto max = *std::ranges::max_element(u.span());
		auto min = *std::ranges::min_element(u.span());
		for (i64 y = 0; y < texture.size().y; y++) {
			for (i64 x = 0; x < texture.size().x; x++) {
				/*texture(x, texture.size().y - 1 - y) = Pixel32(Vec3(u(x, y) / *max));*/
				/*texture(x, texture.size().y - 1 - y) = Pixel32(Vec3(std::abs(u(x, y) / *max)));*/
				/*texture(x, texture.size().y - 1 - y) = Pixel32(Color3::scientificColoring(u(x, y), min, max));*/
				/*texture(x, texture.size().y - 1 - y) = Pixel32(Color3::scientificColoring(u(x, y), 0.0f, 1.7f));*/
				texture(x, texture.size().y - 1 - y) = Pixel32(Color3::scientificColoring(u(x, y), 0.0f, 1.2f));
				//std::swap(texture(x, y), texture(x, texture.size().y - 1 - y));
			}
		}
		ImGui::Text("max: %g", max);
		ImGui::Text("min: %g", min);
	}

	auto getter = [](void* data, int index) -> float {
		const auto self = reinterpret_cast<MainLoop*>(data);
		return self->u(index, self->u.size().y / 2);
		//return u(index, self->u.size() / 2);
	};

	ImGui::Begin("window");
	ImGui::PlotLines("plot", getter, reinterpret_cast<void*>(this), u.size().y, 0, nullptr, FLT_MAX, FLT_MAX, Vec2(-1.0f));
	ImGui::End();
	glClear(GL_COLOR_BUFFER_BIT);

	auto transform = renderer.camera.makeTransform(gridCenter, 0.0f, gridSize / 2.0f);
	renderer.drawImage(texture.span2d().asConst(), transform);
	/*if (drawImage) {
		renderer.drawImage(texture.span2d().asConst(), transform);
	}*/
	glViewport(0, 0, Window::size().x, Window::size().y);
	renderer.update();
	Dbg::update();
	//Image32 im(Window::size().x, Window::size().y);
	/*glReadPixels(0, 0, Window::size().x, Window::size().y, GL_RGBA, GL_UNSIGNED_BYTE, im.data());
	for (auto& pixel : im) {
		pixel.a = 255;
	}
	for (i64 y = 0; y < im.size().y / 2; y++) {
		for (i64 x = 0; x < im.size().x; x++) {
			std::swap(im(x, y), im(x, im.size().y - 1 - y));
		}
	}*/

	if (frame % 2 == 0) {
		//Timer t;
		//im.saveToBmp(("./output/o/out" + std::to_string(frame / 2) + std::string(".bmp")).data());
		//std::cout << "write" << t.elapsedMilliseconds() << "\n";
		for (i64 y = 0; y < texture.size().y / 2; y++) {
			for (i64 x = 0; x < texture.size().x; x++) {
				std::swap(texture(x, y), texture(x, texture.size().y - 1 - y));
			}
		}
		texture.saveToBmp(("./output/a/out" + std::to_string(frame / 2) + std::string(".bmp")).data());
	}
	timr.guiTookMiliseconds("frame");
}

//MainLoop::MainLoop() {
//
//}
//
//void MainLoop::update() {
//
//}