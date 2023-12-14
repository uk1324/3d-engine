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

// TODO: Where is [0, 0]? Does samping work correctly does sampling velocity at [cellSize, cellSize] / 2 actually return the velocity there?
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

std::optional<float> EulerianFluid::sampleQuantity(Span2d<const float> field, Vec2 pos) {
	const auto cellOffset = Vec2(cellSpacing / 2.0f);

	pos.x = std::clamp(pos.x, cellSpacing, gridSize.x * cellSpacing);
	pos.y = std::clamp(pos.y, cellSpacing, gridSize.y * cellSpacing);

	// Could just use a single clamp here and remove the one from the top.
	auto x0 = std::min(static_cast<i64>(floor((pos.x - cellOffset.x) / cellSpacing)), gridSize.x - 1);
	const auto tx = ((pos.x - cellOffset.x) - x0 * cellSpacing) / cellSpacing;
	auto x1 = std::min(x0 + 1, gridSize.x - 1);

	auto y0 = std::min(static_cast<i64>(floor((pos.y - cellOffset.y) / cellSpacing)), gridSize.y - 1);
	const auto ty = ((pos.y - cellOffset.y) - y0 * cellSpacing) / cellSpacing;
	auto y1 = std::min(y0 + 1, gridSize.y - 1);

	/*
	v1
	if (isWall(x0, y0) || isWall(x1, y0) || isWall(x0, y1) || isWall(x1, y1)) {
		return std::nullopt;
	}*/

	/*
	v2
	if (isWall(x0, y0)) {
		return std::nullopt;
	}

	if (isWall(x1, y1)) {
		x1 = x0;
		y1 = y0;
	}

	if (isWall(x1, y0)) {
		x1 = x0;
	}

	if (isWall(x0, y1)) {
		y1 = y0;
	}*/


	/*if (isWall(x0, y0)) {
		if (!isWall(x1, y0)) {
			x0 = x1;
		}
		if (!isWall(x0, y1)) {
			y0 = y1;
		}

	} else {
		if (isWall(x1, y1)) {
			x1 = x0;
			y1 = y0;
		}

		if (isWall(x1, y0)) {
			x1 = x0;
		}

		if (isWall(x0, y1)) {
			y1 = y0;
		}
	}*/

	
	/*
	There is a problem with semi-lagrangian advection, because it advects from the approximate previous position. There can be wall around the previous position which would cause the values at walls to be interpolated. The way to fully solve it would be to raycast so the sample doesn't go through a wall and to prevent the samples being taken from walls you need to consider all the 2^4 combinations of blocks that can be sampled from. 
	w - wa
	For example in the configutation
	xx
	00
	then 
	y1 = y0
	Raycasting might be expensive and shouldn't be nescessary most of the time, because most of the time walls are too thick to go through.
	Without doing raycasting it might be best to just not advect the wall at all in cases where it is hard to determine from where should the advection take place. Like all walls or diagonal configurations.
	*/
	// Not sure if the previous pos being inside walls would only be caused by not accurate enough integration or could it happen, because of the brush changing velocity.
	// TODO: Could implement the same fix for velocity advection.
	/*
	x0 y1 | x1 y1
	-------------
	x0 y0 | x1 y0
	configuration = x0 y1 | x1 y1 | x0 y0 | x1 y0
	*/
	const auto configuration = 
		(static_cast<u32>(isWall(x0, y1)) << 3) 
		| static_cast<u32>(isWall(x1, y1) << 2) 
		| static_cast<u32>(isWall(x0, y0) << 1) 
		| static_cast<u32>(isWall(x1, y0));
	// xx ox xo
	// xx xo ox
	if (configuration == 0b1111 || configuration == 0b0110 || configuration == 0b1001) {
		return std::nullopt;
	}

	// ox
	// xx
	else if (configuration == 0b0111) {
		return field(x0, y1);
	}

	// xo
	// xx
	else if (configuration == 0b1011) {
		return field(x1, y1);
	}

	// xx
	// ox
	else if (configuration == 0b1101) {
		return field(x0, y0);
	}

	// xx
	// xo
	else if (configuration == 0b1110) {
		return field(x1, y0);
	}

	// xx
	// oo
	else if (configuration == 0b1100) {
		y1 = y0;
	}

	// oo
	// xx
	else if (configuration == 0b0011) {
		y0 = y1;
	}
	
	// xo
	// xo
	else if (configuration == 0b1010) {
		x0 = x1;
	}

	// ox
	// ox
	else if (configuration == 0b0101) {
		x1 = x0;
	}

	// xo
	// oo
	else if (configuration == 0b1000) {
		return field(x1, y0);
	}

	// ox
	// oo
	else if (configuration == 0b0100) {
		return field(x0, y0);
	}

	// oo
	// xo
	else if (configuration == 0b0010) {
		return field(x1, y1);
	}

	// oo
	// ox
	else if (configuration == 0b0001) {
		return field(x0, y1);
	}

	// oo
	// oo

	const auto bilerpedValue =
		(1.0f - tx) * (1.0f - ty) * field(x0, y0) +
		(1.0f - tx) * ty * field(x0, y1) +
		tx * (1.0f - ty) * field(x1, y0) +
		tx * ty * field(x1, y1);

	return bilerpedValue;

	//return sampleField(field, pos, Vec2(cellSpacing / 2.0f));
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
			const auto optValueAtPreviousPos = sampleQuantity(spanFrom(advectedQuantityOld).asConst(), approximatePreviousPos);
			if (optValueAtPreviousPos.has_value()) {
				quantity(x, y) = *optValueAtPreviousPos;
			}
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
