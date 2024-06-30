#include "MarchingSquares.hpp"
#include "Utils.hpp"

std::vector<std::vector<Vec2>> marchingSquares(Span2d<const float> grid, bool pixelPerfect, bool conntectDiagonals, float boundaryValue) {
	auto isOutOfRange = [&](i64 x, i64 y) {
		return x < 0 || y < 0 || x >= grid.size().x || y >= grid.size().y;
	};

	auto at = [&](i64 x, i64 y) -> bool {
		// TODO: Could add an option for chosing what to do in the case of out of range.
		if (isOutOfRange(x, y)) {
			return false;
		}
		return grid.get(x, grid.size().y - 1 - y) > boundaryValue;
	};

	auto value = [&](i64 x, i64 y) -> i32 {
		i32 value = at(x, y) << 0;
		value += at(x + 1, y) << 1;
		value += at(x, y + 1) << 2;
		value += at(x + 1, y + 1) << 3;
		return value;
	};

	std::vector<bool> visited;
	visited.resize(grid.size().x * grid.size().y, false);

	auto setVisited = [&](i64 x, i64 y) -> void {
		if (isOutOfRange(x, y)) {
			return;
		}
		visited[y * grid.size().x + x] = true;
	};

	auto getVisited = [&](i64 x, i64 y) -> bool {
		if (isOutOfRange(x, y)) {
			return true;
		}
		return visited[y * grid.size().x + x];
	};

	std::vector<std::vector<Vec2>> polygons;
	Vec2T<i64> startPos{ 0, 0 };

	for (;;) {
		for (i64 y = startPos.y; y < grid.sizeY(); y++) {
			for (i64 x = startPos.x; x < grid.sizeX(); x++) {
				if (getVisited(x, y)) {
					continue;
				}

				setVisited(x, y);

				const auto v = value(x, y);
				const auto atLeastOneIsWhiteButNotAll = v > 0 && !(v == (1 | 2 | 4 | 8));
				if (const auto liesOnTheBoundary = atLeastOneIsWhiteButNotAll) {
					startPos = Vec2T{ x, y };
					goto foundStaringPoint;
				}
			}
			if (y == grid.sizeY() - 1) {
				return polygons;
			}
			startPos.x = 0;
		}

	foundStaringPoint:
		Vec2T<i64> current = startPos;

		Vec2T<i64> previousMove{ 0, 0 };
		Vec2 previousTranslation{ 0.0f };
		std::vector<Vec2> verts;
		for (;;) {
			// Don't know what is a good way to explain why these directions are chosen. Some cases for example the diagonals or 3 true cell ones might be weird so it's best to just drawing them by hand. It is simpler to understand the pixel perfect version.
			Vec2 pos{ current };
			Vec2T<i64> move;
			/*
			4 8
			1 2
			*/
			const auto v = value(current.x, current.y);
			std::optional<Vec2> vert;
			switch (v) {
				/*
				0 0 | 4 0 | 4 8
				1 0 | 1 0 | 1 0
				*/
			case 1:
			case 1 | 4:
			case 1 | 4 | 8:
				move = { 0, -1 };
				break;

				/*
				0 0 | 0 0 | 4 0
				0 2 | 1 2 | 1 2
				*/
			case 2:
			case 1 | 2:
			case 1 | 2 | 4:
				move = { 1, 0 };
				break;

				/*
				4 0 | 4 8 | 4 8
				0 0 | 0 0 | 0 2
				*/
			case 4:
			case 4 | 8:
			case 2 | 4 | 8:
				move = { -1, 0 };
				break;

				/*
				0 8 | 0 8 | 0 8
				0 0 | 0 2 | 1 2
				*/
			case 8:
			case 2 | 8:
			case 1 | 2 | 8:
				move = { 0, 1 };
				break;

				/*
				0 8
				1 0
				*/
			case 1 | 8:
				if (conntectDiagonals) {
					if (previousMove == Vec2T<i64>{ -1, 0 }) {
						move = { 0, -1 };
					} else {
						move = { 0, 1 };
					}
				} else {
					if (previousMove == Vec2T<i64>{ -1, 0 }) {
						move = { 0, 1 };
					} else {
						move = { 0, -1 };
					}
				}

				break;

				/*
				4 0
				0 2
				*/
			case 2 | 4:
				if (conntectDiagonals) {
					if (previousMove == Vec2T<i64>{ 0, -1 }) {
						move = { 1, 0 };
					} else {
						move = { -1, 0 };
					}
				} else {
					if (previousMove == Vec2T<i64>{ 0, -1 }) {
						move = { -1, 0 };
					} else {
						move = { 1, 0 };
					}
				}
				break;

			default:
				ASSERT_NOT_REACHED();
				return polygons;
			}

			if (pixelPerfect) {
				verts.push_back(Vec2{ current } + Vec2{ 1.0f });
				/*if (previousMove == move) {
					verts.pop_back();
				}*/
			} else {
				auto nextPos = Vec2{ current } + Vec2{ 1.0f } + Vec2{ move } / 2.0f;
				/*if (verts.size() >= 1) {
					const auto translation = nextPos - verts.back();
					if (translation == previousTranslation) {
						verts.pop_back();
					}
					previousTranslation = translation;
				}*/
				verts.push_back(nextPos);
			}

			current += move;
			setVisited(current.x, current.y);

			if (current == startPos)
				break;

			previousMove = move;
		}

		// TODO: This logic can definitely be simplified by changing the order in which things are added in the loop. One of these ifs could be removed then. 
		if (verts.size() >= 3) {
			if (const auto isColinear = (verts.back() - verts[verts.size() - 2]).applied(signOrZero) == (verts[0] - verts.back()).applied(signOrZero)) {
				verts.erase(verts.end() - 1);
			}
		}
		if (verts.size() >= 3) {
			if (const auto isColinear = (verts[0] - verts.back()).applied(signOrZero) == (verts[1] - verts[0]).applied(signOrZero)) {
				verts.erase(verts.begin());
			}
			polygons.push_back(std::move(verts));
		}
	}

	return polygons;
}

void marchingSquares2(std::vector<MarchingSquaresLine>& output, Span2d<const float> grid, float boundaryValue, bool lerp) {
	output.clear();

	auto smallerThanBoundaryValue = [&grid, &boundaryValue](i64 x, i64 y) {
		return grid(x, y) < boundaryValue;
	};

	for (i64 yi = 0; yi < grid.sizeY() - 1; yi++) {
		for (i64 xi = 0; xi < grid.sizeX() - 1; xi++) {
			auto add = [&output, &xi, &yi](Vec2 a, Vec2 b) {
				output.push_back(MarchingSquaresLine{ a, b, Vec2T<i32>(xi, yi)});
			};
			/*
			x, y+1 | x+1, y+1
			-----------------
			x, y   | x+1, y
			configuration = x, y+1 | x+1, y+1 | x, y | x+1, y 
			*/
			//bool interpolate = lerp;
			//if (isnan(grid(xi, yi)) || isnan(grid(xi + 1, yi)) || isnan(grid(xi, yi + 1)) || isnan(grid(xi + 1, yi + 1))) {
			//	//interpolate = false;
			//	//continue;
			//}

			const auto configuration =
				(static_cast<i32>(smallerThanBoundaryValue(xi, yi + 1)) << 3)
				| (static_cast<i32>(smallerThanBoundaryValue(xi + 1, yi + 1)) << 2) 
				| (static_cast<i32>(smallerThanBoundaryValue(xi, yi)) << 1)
				| static_cast<i32>(smallerThanBoundaryValue(xi + 1, yi));

			auto at = [&](i64 x, i64 y) {
				return grid(x, y) - boundaryValue;
			};
			/*
			Interpolation
			Example
			x x
			(-)
			o o
			The (-) symbol is supposed to indicate the the line starts at x of the center of the left cells and goes to the x of the center of the right cells.
			Because the value is smaller than 0 at 'x's and bigger than zero at 'o's somewhere between them lies f(pos) = 0.
			If you are at [xi, yi] then the center of that cell is [xi + 0.5, yi + 0.5].
			In the example configuration above the y value should be between yi + 0.5 and yi + 1.5.
			Without interpolation you assume that the value is exacly between them so it is at yi + 1.0.
			Lets consider only the left side so x = xi.
			With interpolation you want to calculate where would f(xi, y) be 0 assuming the the values between f(xi, yi) and f(xi, yi + 1) differ linearly.
			dy = (yi + 1) - (yi)
			slope = (f(xi, yi + 1) - f(xi, yi)) / dy
			Lets assume y = 0 at yi, because that is the bottom cell center.
			Then to make it go throught the correct points it has to look like this
			f(xi, yi) + y * slope
			Setting equal to zero gives:
			y = -f(xi, yi) / slope
			This is the offest from the center of yi to the zero point. So the zero is at yi + 0.5 + y.

			|
			|      /f(xi, yi + 1)
			|     /
			|    /
			+---/------
			^  /   ^
			yi/	 yi+1
			f(xi, yi)

			In configurations with diagonals you can see in which direction should the endpoint be able to move based on in which direction could a line connected to it move.
			*/

			auto lerpY = [&](i64 x) {
				if (!lerp || isnan(grid(x, yi)) || isnan(grid(x, yi + 1))) {
					return 0.5f;
				}
				const auto dy = 1.0f;
				float slope = (at(x, yi + 1) - at(x, yi)) / dy;
				const auto y = -at(x, yi) / slope;
				return y;
			};

			auto lerpX = [&](i64 y) {
				if (!lerp || isnan(grid(xi, y)) || isnan(grid(xi + 1, y))) {
					return 0.5f;
				}
				const auto dx = 1.0f;
				float slope = (at(xi + 1, y) - at(xi, y)) / dx;
				const auto x = -at(xi, y) / slope;
				return x;
			};

			switch (configuration) {
			// oo xx
			// oo xx
			case 0b0000:
			case 0b1111:
				break;

			// xo ox
			// oo xx
			case 0b1000:
			case 0b0111:
				add(Vec2(xi + 0.5f, yi + 0.5f + lerpY(xi)), Vec2(xi + 0.5f + lerpX(yi + 1), yi + 1.5f));
				break;

			// ox xo
			// oo xx
			case 0b0100:
			case 0b1011: {
				add(Vec2(xi + 0.5f + lerpX(yi + 1), yi + 1.5f), Vec2(xi + 1.5f, yi + 0.5f + lerpY(xi + 1)));
				break;
			}
			

			// oo xx
			// xo ox
			case 0b0010:
			case 0b1101:
				add(Vec2(xi + 0.5f, yi + 0.5f + lerpY(xi)), Vec2(xi + 0.5f + lerpX(yi), yi + 0.5f));
				break;

			// oo xx
			// ox xo
			case 0b0001:
			case 0b1110:
				add(Vec2(xi + 0.5f + lerpX(yi), yi + 0.5f), Vec2(xi + 1.5f, yi + 0.5f + lerpY(xi + 1)));
				break;

			// xx oo
			// oo xx
			case 0b1100:
			case 0b0011:
				add(Vec2(xi + 0.5f, yi + 0.5f + lerpY(xi)), Vec2(xi + 1.5f, yi + 0.5f + lerpY(xi + 1)));
				break;

			// xo ox
			// xo ox
			case 0b1010:
			case 0b0101:
				add(Vec2(xi + 0.5 + lerpX(yi), yi + 0.5f), Vec2(xi + 0.5f + lerpX(yi + 1), yi + 1.5f));
				break;

			/*
			'x's disconnected.
			x  / /
			  / / 
			 / /  x
			It doesn't make sense to choose one over the other, because if you just flipped the sign the configuration would change.
			*/
			// xo
			// ox
			case 0b1001:
				add(Vec2(xi + 0.5f, yi + 1.0f), Vec2(xi + 1.0f, yi + 1.5f));
				add(Vec2(xi + 1.0f, yi + 0.5f), Vec2(xi + 1.5f, yi + 1.0f));
				break;

			// ox
			// xo
			case 0b0110:
				add(Vec2(xi + 1.0f, yi + 0.5f), Vec2(xi + 0.5f, yi + 1.0f));
				add(Vec2(xi + 1.0f, yi + 1.5f), Vec2(xi + 1.5f, yi + 1.0f));
				break;

			default:
				ASSERT_NOT_REACHED();
				break;
			}
		}
	}
}

void marchingSquares3(
	std::vector<MarchingSquares3Line>& linesOut,
	Span2d<MarchingSquaresGridCell> gridCellToStoredLinesOut,
	Span2d<const float> grid,
	float boundaryValue,
	bool lerp) {

	std::fill(
		gridCellToStoredLinesOut.span().begin(),
		gridCellToStoredLinesOut.span().end(), 
		MarchingSquaresGridCell{ 
			.line1Index = MarchingSquaresGridCell::EMPTY, 
			.line2Index = MarchingSquaresGridCell::EMPTY,
		});

	auto smallerThanBoundaryValue = [&grid, &boundaryValue](i64 x, i64 y) {
		return grid(x, y) < boundaryValue;
	};

	ASSERT(gridCellToStoredLinesOut.size() == grid.size() - Vec2T<i64>(1));

	for (i64 yi = 0; yi < grid.sizeY() - 1; yi++) {
		for (i64 xi = 0; xi < grid.sizeX() - 1; xi++) {
			auto add = [&linesOut, &xi, &yi](Vec2 a, Vec2 b) {
				linesOut.push_back(MarchingSquares3Line{ a, b, Vec2T<i32>(xi, yi)});
			};
			auto setLineIndexInGrid = [&]() {
				gridCellToStoredLinesOut(xi, yi).line1Index = linesOut.size();
				gridCellToStoredLinesOut(xi, yi).line2Index = MarchingSquaresGridCell::EMPTY;
			};
			auto setLineIndexInGridTwice = [&]() {
				gridCellToStoredLinesOut(xi, yi).line1Index = linesOut.size();
				gridCellToStoredLinesOut(xi, yi).line2Index = linesOut.size() + 1;
			};

			const auto configuration =
				(static_cast<i32>(smallerThanBoundaryValue(xi, yi + 1)) << 3)
				| (static_cast<i32>(smallerThanBoundaryValue(xi + 1, yi + 1)) << 2) 
				| (static_cast<i32>(smallerThanBoundaryValue(xi, yi)) << 1)
				| static_cast<i32>(smallerThanBoundaryValue(xi + 1, yi));

			auto at = [&](i64 x, i64 y) {
				return grid(x, y) - boundaryValue;
			};

			auto lerpY = [&](i64 x) {
				if (!lerp || isnan(grid(x, yi)) || isnan(grid(x, yi + 1))) {
					return 0.5f;
				}
				const auto dy = 1.0f;
				float slope = (at(x, yi + 1) - at(x, yi)) / dy;
				const auto y = -at(x, yi) / slope;
				return y;
			};

			auto lerpX = [&](i64 y) {
				if (!lerp || isnan(grid(xi, y)) || isnan(grid(xi + 1, y))) {
					return 0.5f;
				}
				const auto dx = 1.0f;
				float slope = (at(xi + 1, y) - at(xi, y)) / dx;
				const auto x = -at(xi, y) / slope;
				return x;
			};

			switch (configuration) {
			// oo xx
			// oo xx
			case 0b0000:
			case 0b1111:
				break;

			// xo ox
			// oo xx
			case 0b1000:
			case 0b0111:
				setLineIndexInGrid();
				add(Vec2(xi + 0.5f, yi + 0.5f + lerpY(xi)), Vec2(xi + 0.5f + lerpX(yi + 1), yi + 1.5f));
				break;

			// ox xo
			// oo xx
			case 0b0100:
			case 0b1011: {
				setLineIndexInGrid();
				add(Vec2(xi + 0.5f + lerpX(yi + 1), yi + 1.5f), Vec2(xi + 1.5f, yi + 0.5f + lerpY(xi + 1)));
				break;
			}
			

			// oo xx
			// xo ox
			case 0b0010:
			case 0b1101:
				setLineIndexInGrid();
				add(Vec2(xi + 0.5f, yi + 0.5f + lerpY(xi)), Vec2(xi + 0.5f + lerpX(yi), yi + 0.5f));
				break;

			// oo xx
			// ox xo
			case 0b0001:
			case 0b1110:
				setLineIndexInGrid();
				add(Vec2(xi + 0.5f + lerpX(yi), yi + 0.5f), Vec2(xi + 1.5f, yi + 0.5f + lerpY(xi + 1)));
				break;

			// xx oo
			// oo xx
			case 0b1100:
			case 0b0011:
				setLineIndexInGrid();
				add(Vec2(xi + 0.5f, yi + 0.5f + lerpY(xi)), Vec2(xi + 1.5f, yi + 0.5f + lerpY(xi + 1)));
				break;

			// xo ox
			// xo ox
			case 0b1010:
			case 0b0101:
				setLineIndexInGrid();
				add(Vec2(xi + 0.5 + lerpX(yi), yi + 0.5f), Vec2(xi + 0.5f + lerpX(yi + 1), yi + 1.5f));
				break;

			// xo
			// ox
			case 0b1001:
				setLineIndexInGridTwice();
				add(Vec2(xi + 0.5f, yi + 1.0f), Vec2(xi + 1.0f, yi + 1.5f));
				add(Vec2(xi + 1.0f, yi + 0.5f), Vec2(xi + 1.5f, yi + 1.0f));
				break;

			// ox
			// xo
			case 0b0110:
				setLineIndexInGridTwice();
				add(Vec2(xi + 1.0f, yi + 0.5f), Vec2(xi + 0.5f, yi + 1.0f));
				add(Vec2(xi + 1.0f, yi + 1.5f), Vec2(xi + 1.5f, yi + 1.0f));
				break;

			default:
				ASSERT_NOT_REACHED();
				break;
			}
		}
	}
}

Vec2 rescaleMarchingSquaresPoint(Vec2 p, Vec2 gridSize, Vec2 min, Vec2 max) {
	p /= gridSize;
	p.x = lerp(min.x, max.x, p.x);
	p.y = lerp(min.y, max.y, p.y);
	return p;
}

void rescaleMarchingSquaresLines(std::vector<MarchingSquares3Line>& lines, Vec2 gridSize, Vec2 min, Vec2 max) {
	for (auto& segment : lines) {
		auto scale = [&](Vec2 pos) -> Vec2 {
			pos /= gridSize;
			pos.x = lerp(min.x, max.x, pos.x);
			pos.y = lerp(min.y, max.y, pos.y);
			return pos;
		};
		segment.a = scale(segment.a);
		segment.b = scale(segment.b);
	}
}

void marchingSquresLinesToVectorOfEndpoints(const std::vector<MarchingSquares3Line>& lines, std::vector<Vec2>& endpoints) {
	for (const auto segment : lines) {
		endpoints.push_back(segment.a);
		endpoints.push_back(segment.b);
	}
}

void rescaleMarchingSquaresLinesAndConvertToVectorOfEndpoints(std::vector<MarchingSquares3Line>& lines, std::vector<Vec2>& endpointsOut, Vec2 gridSize, Vec2 min, Vec2 max) {
	rescaleMarchingSquaresLines(lines, gridSize, min, max);
	marchingSquresLinesToVectorOfEndpoints(lines, endpointsOut);
}

Vec2T<i32> MarchingSquaresLine::topLeftIndex() const {
	return Vec2T<i32>(bottomLeftIndex.x, bottomLeftIndex.y + 1);
}

Vec2T<i32> MarchingSquaresLine::bottomRightIndex() const {
	return Vec2T<i32>(bottomLeftIndex.x + 1, bottomLeftIndex.y);
}

Vec2T<i32> MarchingSquaresLine::topRightIndex() const {
	return Vec2T<i32>(bottomLeftIndex.x + 1, bottomLeftIndex.y + 1);
}
