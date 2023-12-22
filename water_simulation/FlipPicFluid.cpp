#include "FlipPicFluid.hpp"

FlipPicFluid::FlipPicFluid(Vec2T<i64> gridSize) 
	: gridSize(gridSize)
	, velXValues(gridSize)
	, velXWeights(gridSize)
	, velYValues(gridSize)
	, velYWeights(gridSize)
	, velX(gridSize)
	, velY(gridSize) {

	const auto cellCount = gridSize.x * gridSize.y;
	isWallValues.resize(cellCount);

	for (i64 x = 0; x < gridSize.x; x++) {
		for (i64 y = 0; y < gridSize.y; y++) {
			if (x > 20 && x < gridSize.x / 2 && y > 20 && y < gridSize.y / 2) {
				particlePositions.push_back(Vec2(x, y) * gridCellSize);
				particleVelocities.push_back(Vec2(0.0f, 0.0f));
			}
		}
	}
	particleRadius = gridCellSize / 2.0f;
}

void FlipPicFluid::update(float dt) {
	integrate(dt);
	resolveCollisions();
	transferVelocitiesToVelocityField();
}

void FlipPicFluid::integrate(float dt) {
	const auto gravity = 1.0f;
	for (i64 i = 0; i < particleCount(); i++) {
		// semi-implicit euler
		particleVelocities[i] += Vec2(0.0f, -gravity) * dt;
		particlePositions[i] += particleVelocities[i] * dt;
	}
}

void FlipPicFluid::resolveCollisions() {
	for (i64 i = 0; i < particleCount(); i++) {
		auto clampPositions = [&](i32 axis) {
			const auto min = particleRadius + gridCellSize;
			if (particlePositions[i][axis] < min) {
				particlePositions[i][axis] = min;
				particleVelocities[i][axis] = 0.0f;
			}
			const auto max = -particleRadius + (gridSize[axis] - 1) * gridCellSize;
			if (particlePositions[i][axis] > max) {
				particlePositions[i][axis] = max;
				particleVelocities[i][axis] = 0.0f;
			}
		};
		clampPositions(0);
		clampPositions(1);
	}
}

void FlipPicFluid::transferVelocitiesToVelocityField() {
	// velX[0, 0] at (0.0, 0.5 * cellSize)
	std::ranges::fill(velXValues.span(), 0.0f);
	std::ranges::fill(velXWeights.span(), 0.0f);
	std::ranges::fill(velYValues.span(), 0.0f);
	std::ranges::fill(velYWeights.span(), 0.0f);

	for (i64 i = 0; i < particleCount(); i++) {
		const auto& particleVel = particleVelocities[i];
		const auto& particlePos = particlePositions[i];
		struct Pos {
			Vec2T<i64> gridPos;
			// offset from bottom left corner
			Vec2 delta;
		};
		auto posInGrid = [this](Vec2 pos, Vec2 gridOffset) -> Pos {
			const auto gridPos = ((pos - gridOffset) / gridCellSize).applied(floor);
			return Pos{
				.gridPos = Vec2T<i64>(gridPos),
				.delta = pos - gridPos * gridCellSize,
			};
		};

		struct Weights {
			// w<dx><dy> = w[x + dx, w + dy]
			float w00; // Bottom left corner of cell.
			float w10;
			float w01;
			float w11;
		};

		auto calculateWeights = [this](Pos pos) -> Weights {
			// The closer to the corner the smaller the delta the bigger the weight should be for that corner.
			return Weights{
				.w00 = (1.0f - pos.delta.x) * (1.0f - pos.delta.y),
				.w10 = pos.delta.x * (1.0f - pos.delta.y),
				.w01 = (1.0f - pos.delta.x) * pos.delta.y,
				.w11 = pos.delta.x * pos.delta.y,
			};
		};

		auto transferToGridFromParticle = [&particlePos, &posInGrid, &calculateWeights](Array2d<float>& weightsGrid, Array2d<float>& valuesGrid, Vec2 gridOffset, float particleValue) {
			const auto pos = posInGrid(particlePos, gridOffset);
			const auto& [gridPos, _] = pos;
			const auto weights = calculateWeights(pos);

			weightsGrid(gridPos.x, gridPos.y) += weights.w00;
			weightsGrid(gridPos.x + 1, gridPos.y) += weights.w10;
			weightsGrid(gridPos.x, gridPos.y + 1) += weights.w01;
			weightsGrid(gridPos.x + 1, gridPos.y + 1) += weights.w11;

			valuesGrid(gridPos.x, gridPos.y) += weights.w00 * particleValue;
			valuesGrid(gridPos.x + 1, gridPos.y) += weights.w10 * particleValue;
			valuesGrid(gridPos.x, gridPos.y + 1) += weights.w01 * particleValue;
			valuesGrid(gridPos.x + 1, gridPos.y + 1) += weights.w11 * particleValue;
		};

		transferToGridFromParticle(velXWeights, velXValues, Vec2(0.0f, 0.5f * gridCellSize), particleVel.x);
		transferToGridFromParticle(velYWeights, velYValues, Vec2(0.5f * gridCellSize, 0.0f), particleVel.y);
	}

	auto calculateGridValues = [](Array2d<float>& destinationGrid, const Array2d<float>& valuesGrid, const Array2d<float>& weightsGrid) {
		const auto& destination = destinationGrid.span();
		const auto& values = valuesGrid.span();
		const auto& weights = weightsGrid.span();
		for (i64 i = 0; i < destination.size(); i++) {
			if (weights[i] == 0.0f) {
				destination[i] = 0.0f;
			} else {
				destination[i] = values[i] / weights[i];
			}
		}
	};
	calculateGridValues(velX, velXValues, velXWeights);
	calculateGridValues(velY, velYValues, velYWeights);
}

void FlipPicFluid::makeIncompressible() {
	for (i64 iter = 0; iter < 40; iter++) {

		for (i64 y = 1; y < gridSize.y - 1; y++) {
			for (i64 x = 1; x < gridSize.x - 1; x++) {
				if (isWall(x, y))
					continue;

				//const float xMinus = isWallFloat(x - 1, y);
				//const float xPlus = isWallFloat(x + 1, y);
				//const float yMinus = isWallFloat(x, y - 1);
				//const float yPlus = isWallFloat(x, y + 1);
				//const float openSidesCount = xMinus + xPlus + yMinus + yPlus;

				//if (openSidesCount == 0.0f)
				//	continue;

				//// When correcting the divergence the constant cellSpacing can be ignored.
				//// This assumes that the velocity at walls is constant. Which should be true, but might cause bugs if it is not.
				//const auto divergenceTimesCellSpacing =
				//	-at(velX, x, y)
				//	+ at(velX, x + 1, y)
				//	- at(velY, x, y)
				//	+ at(velY, x, y + 1);

				//const auto totalVelocityChangeToCorrectDivergence = -divergenceTimesCellSpacing * overRelaxation;
				//const auto velocityChangePerSide = totalVelocityChangeToCorrectDivergence / openSidesCount;

				//at(velX, x, y) -= xMinus * velocityChangePerSide;
				//at(velX, x + 1, y) += xPlus * velocityChangePerSide;
				//at(velY, x, y) -= yMinus * velocityChangePerSide;
				//at(velY, x, y + 1) += yPlus * velocityChangePerSide;

				//const auto acceleration = totalVelocityChangeToCorrectDivergence / dt;
				//const auto area = cellSpacing * openSidesCount;

				//// Not sure if this iterative way of computing the pressure is correct.
				//at(pressure, x, y) += acceleration * cellMass / area;
			}
		}
	}
}

i64 FlipPicFluid::particleCount() {
	return particlePositions.size();
}

void FlipPicFluid::setIsWall(i64 x, i64 y, bool value) {
	isWallValues[x * gridSize.y + y] = value;
}

bool FlipPicFluid::isWall(i64 x, i64 y) {
	return isWallValues[x * gridSize.y + y];
}
