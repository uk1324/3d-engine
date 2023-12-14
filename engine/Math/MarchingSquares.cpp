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
				if (previousMove == move) {
					verts.pop_back();
				}
			} else {
				auto nextPos = Vec2{ current } + Vec2{ 1.0f } + Vec2{ move } / 2.0f;
				if (verts.size() >= 1) {
					const auto translation = nextPos - verts.back();
					if (translation == previousTranslation) {
						verts.pop_back();
					}
					previousTranslation = translation;
				}
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

std::vector<MarchingSquaresLine> marchingSquares2(Span2d<const float> grid, float boundaryValue) {
	std::vector<MarchingSquaresLine> output;

	auto smallerThanBoundaryValue = [&grid, &boundaryValue](i64 x, i64 y) {
		return grid(x, y) < boundaryValue;
	};

	auto add = [&output](Vec2 a, Vec2 b) {
		output.push_back(MarchingSquaresLine{ a, b });
	};

	for (i64 yi = 0; yi < grid.sizeY() - 1; yi++) {
		for (i64 xi = 0; xi < grid.sizeX() - 1; xi++) {
			/*
			x, y+1 | x+1, y+1
			-----------------
			x, y   | x+1, y
			configuration = x, y+1 | x+1, y+1 | x, y | x+1, y 
			*/
			const auto configuration =
				(static_cast<i32>(smallerThanBoundaryValue(xi, yi + 1)) << 3)
				| (static_cast<i32>(smallerThanBoundaryValue(xi + 1, yi + 1)) << 2) 
				| (static_cast<i32>(smallerThanBoundaryValue(xi, yi)) << 1)
				| static_cast<i32>(smallerThanBoundaryValue(xi + 1, yi));

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
				add(Vec2(xi + 0.5f, yi + 1.0f), Vec2(xi + 1.0f, yi + 1.5f));
				break;

			// ox xo
			// oo xx
			case 0b0100:
			case 0b1011:
				add(Vec2(xi + 1.0f, yi + 1.5f), Vec2(xi + 1.5f, yi + 1.0f));
				break;

			// oo xx
			// xo ox
			case 0b0010:
			case 0b1101:
				add(Vec2(xi + 0.5f, yi + 1.0f), Vec2(xi + 1.0f, yi + 0.5f));
				break;

			// oo xx
			// ox xo
			case 0b0001:
			case 0b1110:
				add(Vec2(xi + 1.0f, yi + 0.5f), Vec2(xi + 1.5f, yi + 1.0f));
				break;

			// xx oo
			// oo xx
			case 0b1100:
			case 0b0011:
				add(Vec2(xi + 0.5f, yi + 1.0f), Vec2(xi + 1.5f, yi + 1.0f));
				break;

			// xo ox
			// xo ox
			case 0b1010:
			case 0b0101:
				add(Vec2(xi + 1.0f, yi + 0.5f), Vec2(xi + 1.0f, yi + 1.5f));
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

	return output;
}
