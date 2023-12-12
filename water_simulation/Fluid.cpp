#include "Fluid.hpp"

EulerianFluid::EulerianFluid(Vec2T<i64> gridSize, float cellSpacing, float overRelaxation, float density)
	: gridSize{ gridSize }
	, cellSpacing{ cellSpacing }
	, density{ density }
	, overRelaxation{ overRelaxation } {
	const auto cellCount = gridSize.x * gridSize.y;
	velX.resize(cellCount);
	velY.resize(cellCount);
	oldVelX.resize(cellCount);
	oldVelY.resize(cellCount);
	pressure.resize(cellCount);
	isWallValues.resize(cellCount);
	divergence.resize(cellCount);
	advectedQuantityOld.resize(cellCount);
}

auto EulerianFluid::integrate(float dt, float gravity) -> void {
	for (i64 y = 1; y < gridSize.y - 1; y++) {
		for (i64 x = 1; x < gridSize.x; x++) {
			if (isWall(x, y) || isWall(x, y - 1))
				continue;
			at(velY, x, y) += gravity * dt;
		}
	}
}

EulerianFluid::OpenSides EulerianFluid::getOpenSides(i64 x, i64 y) const {
	return OpenSides{
		.xMinus = !isWall(x - 1, y),
		.xPlus = !isWall(x + 1, y),
		.yMinus = !isWall(x, y - 1),
		.yPlus = !isWall(x, y + 1)
	};
}

auto EulerianFluid::solveIncompressibility(i32 solverIterations, float dt) -> void {
	fill(pressure.begin(), pressure.end(), 0.0f);

	const auto cellMass = density * (cellSpacing * cellSpacing);
	for (i64 iter = 0; iter < solverIterations; iter++) {

		for (i64 y = 1; y < gridSize.y - 1; y++) {
			for (i64 x = 1; x < gridSize.x - 1; x++) {
				if (isWall(x, y))
					continue;

				const auto sides = getOpenSides(x, y);
				const auto openSidesCount = sides.count();

				if (openSidesCount == 0)
					continue;

				// When correcting the divergence the constant cellSpacing can be ignored.
				// This assumes that the velocity at walls is constant. Which should be true, but might cause bugs if it is not.
				const auto divergenceTimesCellSpacing =
					-at(velX, x, y)
					+ at(velX, x + 1, y)
					- at(velY, x, y)
					+ at(velY, x, y + 1);

				const auto totalVelocityChangeToCorrectDivergence = -divergenceTimesCellSpacing * overRelaxation;
				const auto velocityChangePerSide = totalVelocityChangeToCorrectDivergence / openSidesCount;

				at(velX, x, y) -= sides.xMinus * velocityChangePerSide;
				at(velX, x + 1, y) += sides.xPlus * velocityChangePerSide;
				at(velY, x, y) -= sides.yMinus * velocityChangePerSide;
				at(velY, x, y + 1) += sides.yPlus * velocityChangePerSide;

				const auto acceleration = totalVelocityChangeToCorrectDivergence / dt;
				const auto area = cellSpacing * openSidesCount;

				// Not sure if this iterative way of computing the pressure is correct.
				at(pressure, x, y) += acceleration * cellMass / area;
			}
		}
	}
}

void EulerianFluid::computeDivergence() {
	for (i64 y = 1; y < gridSize.y - 1; y++) {
		for (i64 x = 1; x < gridSize.x - 1; x++) {
			at(divergence, x, y) = (at(velX, x + 1, y) - at(velX, x, y) + at(velY, x, y + 1) - at(velY, x, y)) / cellSpacing;
		}
	}
}

float EulerianFluid::sampleField(Span2d<const float> field, Vec2 pos, Vec2 cellOffset) {
	pos.x = std::clamp(pos.x, cellSpacing, gridSize.x * cellSpacing);
	pos.y = std::clamp(pos.y, cellSpacing, gridSize.y * cellSpacing);

	// Could just use a single clamp here and remove the one from the top.
	const auto x0 = std::min(static_cast<i64>(floor((pos.x - cellOffset.x) / cellSpacing)), gridSize.x - 1);
	const auto tx = ((pos.x - cellOffset.x) - x0 * cellSpacing) / cellSpacing;
	const auto x1 = std::min(x0 + 1, gridSize.x - 1);

	const auto y0 = std::min(static_cast<i64>(floor((pos.y - cellOffset.y) / cellSpacing)), gridSize.y - 1);
	const auto ty = ((pos.y - cellOffset.y) - y0 * cellSpacing) / cellSpacing;
	const auto y1 = std::min(y0 + 1, gridSize.y - 1);

	if (field.data() != velX.data() && field.data() != velY.data()) {
		return field(x0, y0);
	}

	
	const auto bilerpedValue =
		(1.0f - tx) * (1.0f - ty) * field(x0, y0) +
		(1.0f - tx) * ty * field(x0, y1) +
		tx * (1.0f - ty) * field(x1, y0) +
		tx * ty * field(x1, y1);

	return bilerpedValue;
}

float EulerianFluid::sampleFieldVelX(const std::vector<float>& field, Vec2 pos) {
	return sampleField(spanFrom(field).asConst(), pos, Vec2(0.0f, cellSpacing / 2.0f));
}

float EulerianFluid::sampleFieldVelY(const std::vector<float>& field, Vec2 pos) {
	return sampleField(spanFrom(field).asConst(), pos, Vec2(cellSpacing / 2.0f, 0.0f));
}

Vec2 EulerianFluid::sampleVel(Vec2 pos) {
	return Vec2(sampleFieldVelX(velX, pos), sampleFieldVelY(velY, pos));
}

float EulerianFluid::sampleQuantity(Span2d<const float> field, Vec2 pos) {
	return sampleField(field, pos, Vec2(cellSpacing / 2.0f));
}

auto EulerianFluid::advectVelocity(float dt) -> void {
	oldVelX = velX;
	oldVelY = velY;

	for (i64 y = 1; y < gridSize.y; y++) {
		for (i64 x = 1; x < gridSize.x; x++) {
			if (isWall(x, y))
				continue;

			// Going back at step in a straight line and sampling the average previous pos to get the new velocity is called semi-lagrangian advection. This introduces viscosity. An alternative to this would be to step the forward and distribute the values to the cells closest to the new position, which would be more difficult to implement especially in a staggered grid.
			if (!isWall(x - 1, y) && y < gridSize.y - 1) {
				const auto pos = Vec2(x + 0.0f, y + 0.5f) * cellSpacing;
				const auto avgVelY = (at(oldVelY, x - 1, y) + at(oldVelY, x, y) + at(oldVelY, x - 1, y + 1) + at(oldVelY, x, y + 1)) / 4.0f;
				const Vec2 vel(at(oldVelX, x, y), avgVelY);
				const auto approximatePreviousPos = pos - vel * dt;
				at(velX, x, y) = sampleFieldVelX(oldVelX, approximatePreviousPos);
			}

			if (!isWall(x, y - 1) && x < gridSize.x - 1) {
				const auto pos = Vec2(x + 0.5f, y + 0.0f) * cellSpacing;
				const auto avgVelX = (at(oldVelX, x, y - 1) + at(oldVelX, x, y) + at(oldVelX, x + 1, y - 1) + at(oldVelX, x + 1, y)) / 4.0f;
				const Vec2 vel(avgVelX, at(oldVelY, x, y));
				const auto approximatePreviousPos = pos - vel * dt;
				at(velY, x, y) = sampleFieldVelY(oldVelY, approximatePreviousPos);
			}
		}
	}
}

void EulerianFluid::advectQuantity(Span2d<float> quantity, float dt) {
	ASSERT(quantity.sizeX() == gridSize.x);
	ASSERT(quantity.sizeY() == gridSize.y);

	memcpy(advectedQuantityOld.data(), quantity.data(), quantity.sizeX() * quantity.sizeY() * sizeof(float));

	for (i64 y = 1; y < gridSize.y - 1; y++) {
		for (i64 x = 1; x < gridSize.x - 1; x++) {
			if (isWall(x, y)) {
				continue;
			}

			// Read advect velocity.
			const auto avgVel = Vec2(at(velX, x, y) + at(velX, x + 1, y), at(velY, x, y) + at(velY, x, y + 1)) / 2.0f;
			const auto pos = (Vec2(Vec2T(x, y)) + Vec2(0.5f)) * cellSpacing;
			const auto approximatePreviousPos = pos - dt * avgVel;
			quantity(x, y) = sampleQuantity(spanFrom(advectedQuantityOld).asConst(), approximatePreviousPos);
		}
	}
}

auto EulerianFluid::update(float dt, float gravity, i32 solverIterations) -> void {
	integrate(dt, gravity);
	solveIncompressibility(solverIterations, dt);
	computeDivergence();
	advectVelocity(dt);
}

auto EulerianFluid::setIsWall(i64 x, i64 y, bool value) -> void {
	isWallValues[y * gridSize.x + x] = value;
}

auto EulerianFluid::isWall(i64 x, i64 y) const -> bool {
	return isWallValues[y * gridSize.x + x];
}

void EulerianFluid::removeVelocityAround(i64 x, i64 y) {
	at(velX, x, y) = 0.0f;
	at(velX, x + 1, y) = 0.0f;
	at(velY, x, y) = 0.0f;
	at(velY, x, y + 1) = 0.0f;
}

i64 EulerianFluid::OpenSides::count() const {
	return xPlus + xMinus + yPlus + yMinus;
}
