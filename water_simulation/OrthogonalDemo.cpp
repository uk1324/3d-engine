#include "OrthogonalDemo.hpp"
#include <engine/Math/Utils.hpp>
#include <implot.h>
#include <engine/Math/MarchingSquares.hpp>
#include <engine/Input/Input.hpp>

Vec2 min(0.0f);
Vec2 max(1.0f);
OrthogonalDemo::OrthogonalDemo() 
	: noise(1) {

	for (i32 yi = 0; yi < array.size().y; yi++) {
		for (i32 xi = 0; xi < array.size().x; xi++) {
			const f32 xt = f32(xi) / f32(array.size().x - 1);
			const f32 yt = f32(yi) / f32(array.size().y - 1);
			/*const Vec2 pos = lerp(min, max, Vec2(xt, yt));*/
			const Vec2 pos = Vec2(
				lerp(min.x, max.x, xt),
				lerp(min.y, max.y, yt)
			);
			array(xi, yi) = sampleNoise(pos);
			int s = 6;
			if (xi % s == 0 && yi % s == 0) {
				points.push_back(Point{ .p = pos, .direction = 1.0f });
				points.push_back(Point{ .p = pos, .direction = -1.0f });
			}
		}
	}
	std::vector<MarchingSquaresLine> lines;
	const auto isolineCount = 10;
	const auto minmax = std::ranges::minmax(array.span());
	for (i32 i = 0; i < isolineCount; i++) {
		const auto t = f32(i) / f32(isolineCount - 1);
		const auto v = lerp(minmax.min, minmax.max, t);
		marchingSquares2(lines, array.span2d().asConst(), v, true);
		for (const auto& line : lines) {
			Vec2 a = rescaleMarchingSquaresPoint(line.a, Vec2(array.size()), min, max);
			Vec2 b = rescaleMarchingSquaresPoint(line.b, Vec2(array.size()), min, max);
			isolines.push_back(a);
			isolines.push_back(b);
			//rescaleMarchingSquaresLines(lines, array.size(), min, max);
		}
	}

	for (i32 yi = 0; yi < grid.size().y; yi++) {
		for (i32 xi = 0; xi < grid.size().x; xi++) {
			const f32 xt = f32(xi) / f32(grid.size().x - 1);
			const f32 yt = f32(yi) / f32(grid.size().y - 1);
			grid(xi, yi) = sampleVectorField(gridPos(Vec2(xt, yt)));
		}
	}
}

#include "PlotUtils.hpp"
#include <glad/glad.h>

void OrthogonalDemo::update() {
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui::ShowDemoWindow();
	return;
	/*update2();
	return;*/
	//ImPlot::ShowDemoWindow();
	if (ImPlot::BeginPlot("plot", Vec2(-1.0f))) {
		ImPlot::PushColormap(ImPlotColormap_Greys);
		//ImPlot::PlotHeatmap("heatmap", array.data(), array.size().x, array.size().y, 0.0f, 0.0f, nullptr, ImPlotPoint(min.x, min.y), ImPlotPoint(max.x, max.y));
		ImPlot::PopColormap();

		Input::ignoreImGuiWantCapture = true;
		if (Input::isMouseButtonDown(MouseButton::LEFT)) {
			const auto mousePos = ImPlot::GetPlotMousePos();
			points.push_back(Point{ .p = Vec2(mousePos.x, mousePos.y) });
		}
		Input::ignoreImGuiWantCapture = false;

		//ImPlot::PlotScatter("points", &points.data()->p.x, &points.data()->p.y, points.size(), 0, 0, sizeof(Point));
		for (const auto& point : points) {
			plotVec2Line("line", point.history);
		}
		plotVec2LineSegments("line2", isolines);
		//for (const auto& point : points) {
		//	//ImPlot::DragPoint
		//}

		ImPlot::EndPlot();
	}

	auto grad = [this](Vec2 p) -> Vec2 {
		const auto h = 0.001f;
		const auto dfdx = (sampleNoise(p + Vec2(h, 0.0f)) - sampleNoise(p - Vec2(h, 0.0f))) / (2.0f * h);
		const auto dfdy = (sampleNoise(p + Vec2(0.0f, h)) - sampleNoise(p - Vec2(0.0f, h))) / (2.0f * h);
		return Vec2(dfdx, dfdy);
	};
	ImGui::Checkbox("paused", &paused);
	if (!paused) {
		for (auto& point : points) {
			point.history.push_back(point.p);
			Vec2 g = grad(point.p);
			/*f32 dx = 0.01f;
			point.p.y += (g.y / g.x) * dx;
			point.p.x += dx;*/
			point.p += g * 0.001f * point.direction;
			/*point.p.y += ;
			point.p.x += dx;*/
		}
	}
	
}

void OrthogonalDemo::update2() {
	const auto cellSize = (max - min) / Vec2(grid.size());

	auto copy = Array2d<Vec2>(grid);
	auto sampleGrid = [&](i64 x, i64 y) -> Vec2 {
		if (x < 0 || y < 0 || x >= grid.size().x || y >= grid.size().y) {
			const auto vt = Vec2(x, y) / Vec2(grid.size());
			const auto pos = gridPos(vt);
			return sampleVectorField(pos);
		}
		return copy(x, y);
	};

	auto sampleGrid2 = [&](i64 x, i64 y) -> Vec2 {
		if (x < 0) {
			x += grid.size().x;
		}
		if (y < 0) {
			y += grid.size().y;
		}
		if (x >= grid.size().x) {
			x -= grid.size().x;
		}
		if (y >= grid.size().y) {
			y -= grid.size().y;
		}
		return copy(x, y);
	};

	auto setGrid = [&](i64 x, i64 y, Vec2 value) -> void {
		if (x < 0 || y < 0 || x >= grid.size().x || y >= grid.size().y) {
			return;
		}
		grid(x, y) = value;
	};

	auto setGridX = [&](i64 x, i64 y, f32 value) -> void {
		if (x < 0 || y < 0 || x >= grid.size().x || y >= grid.size().y) {
			return;
		}
		grid(x, y).x = value;
	};

	auto setGridY = [&](i64 x, i64 y, f32 value) -> void {
		if (x < 0 || y < 0 || x >= grid.size().x || y >= grid.size().y) {
			return;
		}
		grid(x, y).y = value;
	};

	// The solution isn't perfect, because not every function has a harmonic conjugate (and also probably, beucase I don't know what I am doing). For example x^2 doesn't have a harmonic conjugate beucase it doesn't satisfy laplace's equation. I guess you could make a function that does satisfy it by finding the steady state solution to the heat equation.
	// https://math.stackexchange.com/questions/2852574/does-every-function-has-a-harmonic-conjugate

	// This might not actually be possible, because
	/*
	Harmonic functions satisfy the following maximum principle: if K is a nonempty compact subset of U, then f restricted to K attains its maximum and minimum on the boundary of K. If U is connected, this means that f cannot have local maxima or minima, other than the exceptional case where f is constant. Similar properties can be shown for subharmonic functions. 
	https://en.wikipedia.org/wiki/Harmonic_function#Maximum_principle
	*/

	//for (i32 yi = 0; yi < grid.size().y; yi++) {
	//	for (i32 xi = 0; xi < grid.size().x; xi++) {
	//		const auto dx = cellSize.x;
	//		const auto dy = cellSize.y;

	//		const auto dpdyy = (sampleGrid2(xi, yi + 1) - 2.0f * sampleGrid2(xi, yi) + sampleGrid2(xi, yi - 1)) / (dy * dy);
	//		const auto dpdxx = (sampleGrid2(xi + 1, yi) - 2.0f * sampleGrid2(xi, yi) + sampleGrid2(xi - 1, yi)) / (dx * dx);
	//		const auto dt = 1.0f / 600.0f;
	//		setGridX(xi, yi, sampleGrid2(xi, yi).x + (dpdxx.x + dpdyy.x) * dt);
	//		//setGridX(xi, yi, sampleGrid(xi, yi).x);
	//	}
	//}

	//copy = Array2d<Vec2>(grid);

	for (i32 yi = 0; yi < grid.size().y; yi++) {
		for (i32 xi = 0; xi < grid.size().x; xi++) {
			const f32 xt = f32(xi) / f32(grid.size().x - 1);
			const f32 yt = f32(yi) / f32(grid.size().y - 1);

			const auto u = grid(xi, yi).x;
			const auto v = grid(xi, yi).y;

			const auto dx = cellSize.x;
			const auto dy = cellSize.y;



			/*const auto dpdx = (sampleGrid(xi + 1, yi) - sampleGrid(xi, yi)) / dx;
			const auto dpdy = (sampleGrid(xi, yi + 1) - sampleGrid(xi, yi)) / dy;*/
			const auto dpdx = (sampleGrid(xi + 1, yi) - sampleGrid(xi - 1, yi)) / (3.0f * dx);
			const auto dpdy = (sampleGrid(xi, yi + 1) - sampleGrid(xi, yi - 1)) / (3.0f * dy);
			const auto dudx = dpdx.x;
			const auto dudy = dpdy.x;
			const auto dvdx = dpdx.y;
			const auto dvdy = dpdy.y;
			//{
			//	const auto a = dudx * dy + sampleGrid(xi, yi - 1).y;
			//	const auto b = dudy * dx + sampleGrid(xi + 1, yi).y;
			//	grid(xi, yi).y = (a + b) / 2.0f;
			//	//grid(xi, yi).y = a;
			//}
			

			//{
			//	const auto a = -dudx * dy + sampleGrid(xi, yi + 1).y;
			//	const auto b = dudy * dx + sampleGrid(xi + 1, yi).y;
			//	grid(xi, yi).y = (a + b) / 2.0f;
			//	//grid(xi, yi).y = a;
			//}

			{
				const auto a = -dudx * dy + sampleGrid(xi, yi + 1).y;
				const auto b = dudy * dx + sampleGrid(xi + 1, yi).y;
				grid(xi, yi).y = (a + b) / 2.0f;
				//grid(xi, yi).y = a;
			}

			//{
			//	const auto c1 = dudy + dvdx;
			//	const auto k = c1 * 3.0f * dx - dvdy * 3.0f * dx;
			//	setGridY(xi - 1, yi, sampleGrid(xi - 1, yi).y - k / 2.0f);
			//	setGridY(xi + 1, yi, sampleGrid(xi + 1, yi).y + k / 2.0f);
			//}




			//const auto dpdx = (sampleGrid(xi + 1, yi) - sampleGrid(xi - 1, yi)) / (3.0f * dx);
			//const auto dpdy = (sampleGrid(xi, yi + 1) - sampleGrid(xi, yi - 1)) / (3.0f * dy);
			//const auto dudx = dpdx.x;
			//const auto dudy = dpdy.x;
			//const auto dvdx = dpdx.y;
			//const auto dvdy = dpdy.y;
			////{
			////	const auto c1 = dudx - dvdy;
			////	const auto k = c1 * 3.0f * dy - dudx * 3.0f * dy;
			////	setGridY(xi, yi - 1, sampleGrid(xi, yi - 1).y - k / 2.0f);
			////	setGridY(xi, yi + 1, sampleGrid(xi, yi + 1).y + k / 2.0f);
			////}
			//{
			//	const auto c1 = dudy + dvdx;
			//	const auto k = c1 * 3.0f * dx - dvdy * 3.0f * dx;
			//	setGridY(xi - 1, yi, sampleGrid(xi - 1, yi).y - k / 2.0f);
			//	setGridY(xi + 1, yi, sampleGrid(xi + 1, yi).y + k / 2.0f);
			//}
			//array(xi, yi) = sampleNoise();
		}
	}

	std::vector<f32> data;
	//for (i32 yi = 0; yi < grid.size().y; yi++) {
	for (i32 yi = i64(grid.size().y) - 1; yi >= 0; yi--) {
		for (i32 xi = 0; xi < grid.size().x; xi++) {
		/*for (i32 xi = i64(grid.size().x) - 1; xi >= 0; xi--) {*/
			/*data.push_back(grid(xi, yi).y);*/
			//data.push_back(grid(xi, yi).x);
			data.push_back(grid(xi, yi).y);
		}
	}

	auto drawIsolines = [](const char* name, const Array2d<f32>& data) {
		std::vector<Vec2> iso;
		std::vector<MarchingSquaresLine> lines;
		const auto isolineCount = 10;
		const auto minmax = std::ranges::minmax(data.span());
		for (i32 i = 0; i < isolineCount; i++) {
			const auto t = f32(i) / f32(isolineCount - 1);
			const auto v = lerp(minmax.min, minmax.max, t);
			marchingSquares2(lines, data.span2d().asConst(), v, true);
			for (const auto& line : lines) {
				Vec2 a = rescaleMarchingSquaresPoint(line.a, Vec2(data.size()), min, max);
				Vec2 b = rescaleMarchingSquaresPoint(line.b, Vec2(data.size()), min, max);
				iso.push_back(a);
				iso.push_back(b);
			}
		}
		plotVec2LineSegments(name, iso);
	};

	if (ImPlot::BeginPlot("plot", Vec2(-1.0f))) {
		ImPlot::PushColormap(ImPlotColormap_Greys);
		/*ImPlot::PlotHeatmap("heatmap", data.data(), grid.size().x, grid.size().y, 0.0f, 0.0f, nullptr, ImPlotPoint(min.x, min.y), ImPlotPoint(max.x, max.y));*/
		ImPlot::PlotHeatmap("heatmap", data.data(), grid.size().x, grid.size().y, 0.0f, 0.0f, nullptr, ImPlotPoint(min.x, min.y), ImPlotPoint(max.x, max.y));
		ImPlot::PopColormap();

		Array2d<f32> temp(grid.size());

		for (i32 yi = 0; yi < grid.size().y; yi++) {
			for (i32 xi = 0; xi < grid.size().x; xi++) {
				temp(xi, yi) = grid(xi, yi).x;
			}
		}
		drawIsolines("x iso", temp);

		for (i32 yi = 0; yi < grid.size().y; yi++) {
			for (i32 xi = 0; xi < grid.size().x; xi++) {
				temp(xi, yi) = grid(xi, yi).y;
			}
		}
		drawIsolines("y iso", temp);

		ImPlot::EndPlot();
	}
}

Vec2 OrthogonalDemo::gridPos(Vec2 vt) {
	return Vec2(
		lerp(min.x, max.x, vt.x),
		lerp(min.y, max.y, vt.y)
	);
}

f32 OrthogonalDemo::sampleNoise(Vec2 p) {
	/*return noise.value2d01(p * 10.0f) + 5.0f;*/
	return noise.value2d01(p * 10.0f);
}

Vec2 OrthogonalDemo::sampleVectorField(Vec2 p) {
	/*return Vec2(sampleNoise(p), sampleNoise(p + Vec2(123.32f, 2.0f)));*/
	return Vec2(sampleNoise(p), 0.0f);
}