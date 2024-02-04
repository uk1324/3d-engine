#include "PolynomialInterpolationDemo.hpp"
#include <glad/glad.h>
#include <imgui/implot.h>
#include <imgui/imgui_internal.h>
#include <engine/Utils/Span2d.hpp>
#include <engine/Utils/Span.hpp>
#include <engine/Utils/Put.hpp>
#include <engine/Utils/Array2d.hpp>
#include <engine/Math/Random.hpp>
#include <engine/Math/Angles.hpp>

void printMatrix(Span2d<const float> m) {
	for (i64 y = 0; y < m.sizeY(); y++) {
		for (i64 x = 0; x < m.sizeX(); x++) {
			putnn("% ", m(x, y));
		}
		put("");
	}
}

void gaussianEliminationConvertToUpperTriangular(Span2d<float> augmentedMatrix) {
	const auto m = augmentedMatrix.sizeY();
	const auto n = augmentedMatrix.sizeX();

	auto a = [&](i32 y, i32 x) -> float& {
		return augmentedMatrix(x - 1, y - 1);
		if (x < 1 || y < 1 || x > n || y > m) {
			ASSERT_NOT_REACHED();
		}
	};

	auto swap_rows = [&](i32 a, i32 b) {
		if (a == b) {
			return;
		}

		for (i64 i = 0; i < augmentedMatrix.sizeX(); i++) {
			std::swap(augmentedMatrix(i, a - 1), augmentedMatrix(i, b - 1));
		}
	};

	i32 h = 1; /* Initialization of the pivot row */
	i32 k = 1; /* Initialization of the pivot column */

	while (h <= m && k <= n) {
		/* Find the k-th pivot: */
		i32 i_max = h;

		for (i32 i = h; i <= m; i++) {
			if (a(i, k) > std::abs(a(i_max, k))) {
				i_max = i;
			}
		}
		//i_max : = argmax(i = h ... m, abs(A[i, k]))

		if (a(i_max, k) == 0) {
			k += 1;
		} else {
			/* No pivot in this column, pass to next column */
			swap_rows(h, i_max);

			/* Do for all rows below pivot: */
			for (i32 i = h + 1; i <= m; i++) {
				const auto f = a(i, k) / a(h, k);
				/* Fill with zeros the lower part of pivot column: */
				a(i, k) = 0;

				/* Do for all remaining elements in current row: */
				//for j = k + 1 ... n{
				for (i32 j = k + 1; j <= n; j++) {
					a(i, j) = a(i, j) - a(h, j) * f;
				}
			}

			/* Increase pivot row and column */
			h++;
			k++;
		}
		
		
	}
	
}

#include <Dbg.hpp>

[[nodiscard]] bool backSubstituteUpperTriangular(Span2d<const float> augmentedMatrix, Span<float> output) {
	ASSERT(augmentedMatrix.sizeX() == augmentedMatrix.sizeY() + 1);
	ASSERT(output.size() == augmentedMatrix.sizeY());

	for (i32 y = augmentedMatrix.sizeY() - 1; y >= 0; y--) {
		output[y] = augmentedMatrix(augmentedMatrix.sizeX() - 1, y);

		for (i32 x = y + 1; x < augmentedMatrix.sizeX() - 1; x++) {
			output[y] -= augmentedMatrix(x, y) * output[x];
		}

		const auto a = augmentedMatrix(y, y);
		if (a == 0.0f) {
			return false;
		}

		output[y] /= a;
	}

	return true;
}

void calculateInterpolatedPolynomialCoefficients(const std::vector<Vec2T<double>>& controlPoints, std::vector<float>& output) {
	float y = 0.0f; 

	Array2d<float> a(controlPoints.size() + 1, controlPoints.size());
	for (i32 yi = 0; yi < controlPoints.size(); yi++) {
		float x = controlPoints[yi].x;
		float powersOfX = 1.0f;
		for (i32 xi = 0; xi < controlPoints.size(); xi++) {
			a(xi, yi) = powersOfX;
			powersOfX *= x;
		}
		a(controlPoints.size(), yi) = controlPoints[yi].y;
	}

	gaussianEliminationConvertToUpperTriangular(a.span2d());
	backSubstituteUpperTriangular(a.span2d().asConst(), output);
}

float polynomialEvaluateHorner(Span<float> coefficients, float x) {
	/*if (coefficients.size() == 0) {
		return 0.0f;
	}
	float output = x * coefficients[0];
	for (int i = 1; i < coefficients.size(); i++) {
		output += coefficients[i];
		output *= x;
	}
	return output;*/

	if (coefficients.size() == 0) {
		return 0.0f;
	}
	float output = coefficients[coefficients.size() - 1];
	for (int i = coefficients.size() - 2; i >= 0; i--) {
		output = output * x + coefficients[i];
	}
	return output;
}

PolynomialInterpolationDemo::PolynomialInterpolationDemo() {
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	Polynomial p;
	const auto count = 5;
	float scale = 12.0f;
	for (int i = 1; i <= count; i++) {
		float x = cos((2 * i - 1) / float(2 * count) * PI<float>) * scale + scale + 1.0f;
		p.controlPoints.push_back(Vec2T<double>(x, log(x)));
	}
	p.coefficients.resize(p.controlPoints.size());
	p.color = Vec3(1.0f, 0.0f, 0.0f);
	p.recalculateCoefficients = true;
	//polynomials.push_back(p);
}


void PolynomialInterpolationDemo::update() {
	glClear(GL_COLOR_BUFFER_BIT);

	using namespace ImGui;

	dragPointsIds = 0;
	// https://github.com/ocornut/imgui/issues/2583
	auto id = DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoTabBar);

	static bool firstFrame = true;
	if (firstFrame) {
		// https://gist.github.com/AidanSun05/953f1048ffe5699800d2c92b88c36d9f
		DockBuilderRemoveNode(id);
		DockBuilderAddNode(id);

		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
		const auto rightId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);

		DockBuilderDockWindow("plot", rightId);
		DockBuilderDockWindow("plot settings", leftId);

		DockBuilderFinish(id);
		firstFrame = false;
	}
	
	Begin("plot", nullptr, ImGuiWindowFlags_NoTitleBar);

	if (ImPlot::BeginPlot("##main plot", ImVec2(-1.0f, -1.0f))) {
		const auto plotRect = ImPlot::GetPlotLimits();

		for (auto& polynomial : polynomials) {
			for (int i = 0; i < polynomial.controlPoints.size(); i++) {
				auto& point = polynomial.controlPoints[i];
				const auto modified = ImPlot::DragPoint(dragPointsIds, &point.x, &point.y, polynomial.imColor());
				dragPointsIds++;
				if (modified) {
					polynomial.recalculateCoefficients = true;
				}
			}

			if (polynomial.recalculateCoefficients) {
				calculateInterpolatedPolynomialCoefficients(polynomial.controlPoints, polynomial.coefficients);
			}

			std::vector<float> xs;
			std::vector<float> ys;
			const auto count = 400;
			for (int i = 0; i < count; i++) {
				float x = plotRect.X.Min + (plotRect.X.Max - plotRect.X.Min) * (i / float(count - 1));
				float y = polynomialEvaluateHorner(polynomial.coefficients, x);
				xs.push_back(x);
				ys.push_back(y);
			}
			ImPlot::PushStyleColor(ImPlotCol_Line, polynomial.imColor());
			ImPlot::PlotLine("test", xs.data(), ys.data(), xs.size());
			ImPlot::PopStyleColor();
		}

		ImPlot::EndPlot();
	}

	End();

	Begin("plot settings");

	if (ImGui::Button("create new polynomial")) {
		polynomials.push_back(Polynomial{ .color = random01Vec3() });
	}

	for (int i = 0; i < polynomials.size(); i++) {
		auto& polynomial = polynomials[i];
		Separator();
		ColorEdit3("color", polynomial.color.data());
		PushID(i);
		InputFloat2("new point position", polynomial.inputPoint.data());
		if (Button("add new point")) {
			polynomial.controlPoints.push_back(Vec2T<double>(polynomial.inputPoint));
			polynomial.coefficients.resize(polynomial.controlPoints.size());
			polynomial.recalculateCoefficients = true;
			polynomial.inputPoint = Vec2(0.0f);
		}
		PopID();
	}

	End();
}

ImVec4 PolynomialInterpolationDemo::Polynomial::imColor() const {
	return ImVec4(color.x, color.y, color.z, 1.0f);
}
