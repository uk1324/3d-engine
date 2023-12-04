#include "Fluid.hpp"

EulerianFluid::EulerianFluid(Vec2T<i64> gridSize, float cellSpacing, float overRelaxation, float density)
	: gridSize{ gridSize }
	, cellSpacing{ cellSpacing }
	, density{ density }
	, overRelaxation{ overRelaxation } {
	const auto cellCount = gridSize.x * gridSize.y;
	velX.resize(cellCount);
	velY.resize(cellCount);
	newVelX.resize(cellCount);
	newVelY.resize(cellCount);
	pressure.resize(cellCount);
	smoke.resize(cellCount, 1.0f);
	isWallValues.resize(cellCount);
	newSmoke.resize(cellCount);
	divergence.resize(cellCount);

	// Place border walls.
	for (i64 x = 0; x < gridSize.x; x++) {
		for (i64 y = 0; y < gridSize.y; y++) {
			bool wall = false;
			if (x == 0 || x == gridSize.x - 1 || y == 0 || y == gridSize.y - 1)
				wall = true;
			setIsWall(x, y, wall);
		}
	}
}

auto EulerianFluid::integrate(float dt, float gravity) -> void {
	for (i64 x = 1; x < gridSize.x; x++) {
		for (i64 y = 1; y < gridSize.y - 1; y++) {
			if (isWall(x, y) || isWall(x, y - 1))
				continue;
			at(velY, x, y) += gravity * dt;
		}
	}
}

auto EulerianFluid::solveIncompressibility(i32 solverIterations, float dt) -> void {
	const auto pressureWithoutVelocity = density * (cellSpacing * cellSpacing) / dt;
	// If a fluid is incompressible it has to have a divergence of 0 at each point. The amount of fluid going out of a point has to be equal to the amount going in. A divergence of zero means that no fluid is created. If divergence were to be positive (in an compressible fluid) then the fluid would need to be created out of nothing and if negative then matter would need to disappear. Solve using projection gauss seidel. To find approximate the global solution solve each cell separately multiple times.
	// Incompressible fluids are a good approximation of for example water.
	// I think the mathematical term for the way to remove the divergence from a vector field is caleld Hodge decomposition. This is mentioned in https://damassets.autodesk.net/content/dam/autodesk/research/publications-assets/pdf/realtime-fluid-dynamics-for.pdf. Not sure if the method described there is the same as this one. The implementation shown seems quite different, but it should do the same thing.
	// "Hodge decomposition: every velocity field is the sum of a mass conserving field and a gradient field"
	// Mass conserving means with zero divergence and gradient field means the just the curl of the vector field.
	for (i64 iter = 0; iter < solverIterations; iter++) {

		for (i64 x = 1; x < gridSize.x - 1; x++) {
			for (i64 y = 1; y < gridSize.y - 1; y++) {
				if (isWall(x, y))
					continue;

				// std::vector<bool> is really slow in debug mode.
				const auto
					sx0 = !isWall(x - 1, y),
					sx1 = !isWall(x + 1, y),
					sy0 = !isWall(x, y - 1),
					sy1 = !isWall(x, y + 1);
				const auto outflowingSidesCount = sx0 + sx1 + sy0 + sy1;

				if (outflowingSidesCount == 0.0)
					continue;

				// The cooridinates don't represent the cells around the [x, y] so they don't need to be set to zero if there is a wall. These velocites belong to the cell not the cells around them.
				// Total outflow.
				const auto divergence =
					-at(velX, x, y)
					+ at(velX, x + 1, y)
					- at(velY, x, y)
					+ at(velY, x, y + 1);

				// Outflow to each surrouding cell evenly.
				const auto correctedOutflow = (-divergence / outflowingSidesCount) * overRelaxation;
				at(pressure, x, y) += pressureWithoutVelocity * correctedOutflow;
				at(velX, x, y) -= sx0 * correctedOutflow;
				at(velX, x + 1, y) += sx1 * correctedOutflow;
				at(velY, x, y) -= sy0 * correctedOutflow;
				at(velY, x, y + 1) += sy1 * correctedOutflow;
			}
		}
	}

	for (i64 x = 1; x < gridSize.x - 1; x++) {
		for (i64 y = 1; y < gridSize.y - 1; y++) {
			if (isWall(x, y))
				continue;

			const auto
				sx0 = !isWall(x - 1, y),
				sx1 = !isWall(x + 1, y),
				sy0 = !isWall(x, y - 1),
				sy1 = !isWall(x, y + 1);
			const auto outflowingSidesCount = sx0 + sx1 + sy0 + sy1;

			if (outflowingSidesCount == 0.0)
				continue;

			const auto divergence =
				-at(velX, x, y)
				+ at(velX, x + 1, y)
				- at(velY, x, y)
				+ at(velY, x, y + 1);

			at(this->divergence, x, y) = divergence;
		}
	}
}

auto EulerianFluid::sampleField(Vec2 pos, FieldType type) -> float {
	pos.x = std::clamp(pos.x, cellSpacing, gridSize.x * cellSpacing);
	pos.y = std::clamp(pos.y, cellSpacing, gridSize.y * cellSpacing);

	std::vector<float>* field = nullptr;

	Vec2 cellOffset{ 0.0f };
	switch (type) {
	case FieldType::VEL_X:
		field = &velX;
		cellOffset.y = cellSpacing / 2.0f;
		break;
	case FieldType::VEL_Y:
		field = &velY;
		cellOffset.x = cellSpacing / 2.0f;
		break;
	case FieldType::SMOKE:
		field = &smoke;
		cellOffset = Vec2{ cellSpacing / 2.0f };
		break;
	}

	// Could just use a single clamp here and remove the one from the top.
	const auto x0 = std::min(static_cast<i64>(floor((pos.x - cellOffset.x) / cellSpacing)), gridSize.x - 1);
	const auto tx = ((pos.x - cellOffset.x) - x0 * cellSpacing) / cellSpacing;
	const auto x1 = std::min(x0 + 1, gridSize.x - 1);

	const auto y0 = std::min(static_cast<i64>(floor((pos.y - cellOffset.y) / cellSpacing)), gridSize.y - 1);
	const auto ty = ((pos.y - cellOffset.y) - y0 * cellSpacing) / cellSpacing;
	const auto y1 = std::min(y0 + 1, gridSize.y - 1);

	const auto bilerpedValue =
		(1.0f - tx) * (1.0f - ty) * at(*field, x0, y0) +
		(1.0f - tx) * ty * at(*field, x0, y1) +
		tx * (1.0f - ty) * at(*field, x1, y0) +
		tx * ty * at(*field, x1, y1);

	return bilerpedValue;
}

auto EulerianFluid::advectVelocity(float dt) -> void {
	newVelX = velX;
	newVelY = velY;

	for (i64 x = 1; x < gridSize.x; x++) {
		for (i64 y = 1; y < gridSize.y; y++) {
			if (isWall(x, y))
				continue;

			// Going back at step in a straight line and sampling the average previous pos to get the new velocity is called semi-lagrangian advection. This introduces viscosity. An alternative to this would be to step the forward and distribute the values to the cells closest to the new position, which would be more difficult to implement especially in a staggered grid.
			if (!isWall(x - 1, y) && y < gridSize.y - 1) {
				const auto pos = Vec2{ x + 0.0f, y + 0.5f } *cellSpacing;
				const auto avgVelY = (at(velY, x - 1, y) + at(velY, x, y) + at(velY, x - 1, y + 1) + at(velY, x, y + 1)) / 4.0f;
				const Vec2 vel{ at(velX, x, y), avgVelY };
				const auto approximatePreviousPos = pos - vel * dt;
				at(newVelX, x, y) = sampleField(approximatePreviousPos, FieldType::VEL_X);
			}

			if (!isWall(x, y - 1) && x < gridSize.x - 1) {
				const auto pos = Vec2{ x + 0.5f, y + 0.0f } *cellSpacing;
				const auto avgVelX = (at(velX, x, y - 1) + at(velX, x, y) + at(velX, x + 1, y - 1) + at(velX, x + 1, y)) / 4.0f;
				const Vec2 vel{ avgVelX, at(velY, x, y) };
				const auto approximatePreviousPos = pos - vel * dt;
				at(newVelY, x, y) = sampleField(approximatePreviousPos, FieldType::VEL_Y);
			}
		}
	}

	velX = newVelX;
	velY = newVelY;
}

auto EulerianFluid::advectSmoke(float dt) -> void {
	newSmoke = smoke;

	for (i64 x = 1; x < gridSize.x - 1; x++) {
		for (i64 y = 1; y < gridSize.y - 1; y++) {
			if (isWall(x, y))
				continue;

			// Read advect velocity.
			const auto avgVel = Vec2{ at(velX, x, y) + at(velX, x + 1, y), at(velY, x, y) + at(velY, x, y + 1) } / 2.0f;
			const auto pos = (Vec2{ Vec2T{ x, y } } + Vec2{ 0.5f })* cellSpacing;
			const auto approximatePreviousPos = pos - dt * avgVel;
			at(newSmoke, x, y) = sampleField(approximatePreviousPos, FieldType::SMOKE);
		}
	}

	smoke = newSmoke;
}

auto EulerianFluid::update(float dt, float gravity, i32 solverIterations) -> void {
	integrate(dt, gravity);
	fill(pressure.begin(), pressure.end(), 0.0f);
	solveIncompressibility(solverIterations, dt);
	advectVelocity(dt);
	advectSmoke(dt);
}

auto EulerianFluid::setIsWall(i64 x, i64 y, bool value) -> void {
	isWallValues[x * gridSize.y + y] = value;
}

auto EulerianFluid::isWall(i64 x, i64 y) const -> bool {
	return isWallValues[x * gridSize.y + y];
}


//#include "Fluid.hpp"
//
//EulerianFluid::EulerianFluid(Vec2T<i64> gridSize, float cellSpacing, float overRelaxation, float density)
//	: gridSize{ gridSize }
//	, cellSpacing{ cellSpacing }
//	, density{ density }
//	, overRelaxation{ overRelaxation } {
//	const auto cellCount = gridSize.x * gridSize.y;
//	velX.resize(cellCount);
//	velY.resize(cellCount);
//	newVelX.resize(cellCount);
//	newVelY.resize(cellCount);
//	oldVelX.resize(cellCount);
//	oldVelY.resize(cellCount);
//	pressure.resize(cellCount);
//	isWallValues.resize(cellCount);
//	divergence.resize(cellCount);
//	advectedQuantityOld.resize(cellCount);
//
//	// Place border walls.
//	for (i64 x = 0; x < gridSize.x; x++) {
//		for (i64 y = 0; y < gridSize.y; y++) {
//			bool wall = false;
//			if (x == 0 || x == gridSize.x - 1 || y == 0 || y == gridSize.y - 1)
//				wall = true;
//			setIsWall(x, y, wall);
//		}
//	}
//}
//
//auto EulerianFluid::integrate(float dt, float gravity) -> void {
//	for (i64 y = 1; y < gridSize.y - 1; y++) {
//		for (i64 x = 1; x < gridSize.x; x++) {
//			if (isWall(x, y) || isWall(x, y - 1))
//				continue;
//			at(velY, x, y) += gravity * dt;
//		}
//	}
//}
//
//auto EulerianFluid::solveIncompressibility(i32 solverIterations, float dt) -> void {
//	const auto pressureWithoutVelocity = density * (cellSpacing * cellSpacing) / dt;
//	// If a fluid is incompressible it has to have a divergence of 0 at each point. The amount of fluid going out of a point has to be equal to the amount going in. A divergence of zero means that no fluid is created. If divergence were to be positive (in an compressible fluid) then the fluid would need to be created out of nothing and if negative then matter would need to disappear. Solve using projection gauss seidel. To find approximate the global solution solve each cell separately multiple times.
//	// Incompressible fluids are a good approximation of for example water.
//	// I think the mathematical term for the way to remove the divergence from a vector field is caleld Hodge decomposition. This is mentioned in https://damassets.autodesk.net/content/dam/autodesk/research/publications-assets/pdf/realtime-fluid-dynamics-for.pdf. Not sure if the method described there is the same as this one. The implementation shown seems quite different, but it should do the same thing.
//	// "Hodge decomposition: every velocity field is the sum of a mass conserving field and a gradient field"
//	// Mass conserving means with zero divergence and gradient field means the just the curl of the vector field.
//	for (i64 iter = 0; iter < solverIterations; iter++) {
//
//		for (i64 y = 1; y < gridSize.y - 1; y++) {
//			for (i64 x = 1; x < gridSize.x - 1; x++) {
//				if (isWall(x, y))
//					continue;
//
//				// std::vector<bool> is really slow in debug mode.
//				const auto
//					sx0 = !isWall(x - 1, y),
//					sx1 = !isWall(x + 1, y),
//					sy0 = !isWall(x, y - 1),
//					sy1 = !isWall(x, y + 1);
//				const auto outflowingSidesCount = sx0 + sx1 + sy0 + sy1;
//
//				if (outflowingSidesCount == 0.0)
//					continue;
//
//				// The cooridinates don't represent the cells around the [x, y] so they don't need to be set to zero if there is a wall. These velocites belong to the cell not the cells around them.
//				// Total outflow.
//				const auto divergence =
//					-at(velX, x, y)
//					+ at(velX, x + 1, y)
//					- at(velY, x, y)
//					+ at(velY, x, y + 1);
//
//				// Outflow to each surrouding cell evenly.
//				const auto correctedOutflow = (-divergence / outflowingSidesCount) * overRelaxation;
//				at(pressure, x, y) += pressureWithoutVelocity * correctedOutflow;
//				at(velX, x, y) -= sx0 * correctedOutflow;
//				at(velX, x + 1, y) += sx1 * correctedOutflow;
//				at(velY, x, y) -= sy0 * correctedOutflow;
//				at(velY, x, y + 1) += sy1 * correctedOutflow;
//			}
//		}
//	}
//
//	for (i64 y = 1; y < gridSize.y - 1; y++) {
//		for (i64 x = 1; x < gridSize.x - 1; x++) {
//			if (isWall(x, y))
//				continue;
//
//			const auto
//				sx0 = !isWall(x - 1, y),
//				sx1 = !isWall(x + 1, y),
//				sy0 = !isWall(x, y - 1),
//				sy1 = !isWall(x, y + 1);
//			const auto outflowingSidesCount = sx0 + sx1 + sy0 + sy1;
//
//			if (outflowingSidesCount == 0.0)
//				continue;
//
//			const auto divergence =
//				-at(velX, x, y)
//				+ at(velX, x + 1, y)
//				- at(velY, x, y)
//				+ at(velY, x, y + 1);
//
//			at(this->divergence, x, y) = divergence;
//		}
//	}
//}
//
////auto EulerianFluid::sampleField(Vec2 pos, FieldType type) -> float {
////	//pos.x = std::clamp(pos.x, cellSpacing, gridSize.x * cellSpacing);
////	//pos.y = std::clamp(pos.y, cellSpacing, gridSize.y * cellSpacing);
////
////	//std::vector<float>* field = nullptr;
////
////	//Vec2 cellOffset{ 0.0f };
////	//switch (type) {
////	//case FieldType::VEL_X:
////	//	field = &velX;
////	//	cellOffset.y = cellSpacing / 2.0f;
////	//	break;
////	//case FieldType::VEL_Y:
////	//	field = &velY;
////	//	cellOffset.x = cellSpacing / 2.0f;
////	//	break;
////	//case FieldType::SMOKE:
////	//	field = &smoke;
////	//	cellOffset = Vec2{ cellSpacing / 2.0f };
////	//	break;
////	//}
////
////	//// Could just use a single clamp here and remove the one from the top.
////	//const auto x0 = std::min(static_cast<i64>(floor((pos.x - cellOffset.x) / cellSpacing)), gridSize.x - 1);
////	//const auto tx = ((pos.x - cellOffset.x) - x0 * cellSpacing) / cellSpacing;
////	//const auto x1 = std::min(x0 + 1, gridSize.x - 1);
////
////	//const auto y0 = std::min(static_cast<i64>(floor((pos.y - cellOffset.y) / cellSpacing)), gridSize.y - 1);
////	//const auto ty = ((pos.y - cellOffset.y) - y0 * cellSpacing) / cellSpacing;
////	//const auto y1 = std::min(y0 + 1, gridSize.y - 1);
////
////	//const auto bilerpedValue =
////	//	(1.0f - tx) * (1.0f - ty) * at(*field, x0, y0) +
////	//	(1.0f - tx) * ty * at(*field, x0, y1) +
////	//	tx * (1.0f - ty) * at(*field, x1, y0) +
////	//	tx * ty * at(*field, x1, y1);
////
////	//return bilerpedValue;
////}
//
//float EulerianFluid::sampleField(Span2d<const float> field, Vec2 pos, Vec2 cellOffset) {
//	pos.x = std::clamp(pos.x, cellSpacing, gridSize.x * cellSpacing);
//	pos.y = std::clamp(pos.y, cellSpacing, gridSize.y * cellSpacing);
//
//	// Could just use a single clamp here and remove the one from the top.
//	const auto x0 = std::min(static_cast<i64>(floor((pos.x - cellOffset.x) / cellSpacing)), gridSize.x - 1);
//	const auto tx = ((pos.x - cellOffset.x) - x0 * cellSpacing) / cellSpacing;
//	const auto x1 = std::min(x0 + 1, gridSize.x - 1);
//
//	const auto y0 = std::min(static_cast<i64>(floor((pos.y - cellOffset.y) / cellSpacing)), gridSize.y - 1);
//	const auto ty = ((pos.y - cellOffset.y) - y0 * cellSpacing) / cellSpacing;
//	const auto y1 = std::min(y0 + 1, gridSize.y - 1);
//
//	const auto bilerpedValue =
//		(1.0f - tx) * (1.0f - ty) * field(x0, y0) +
//		(1.0f - tx) * ty * field(x0, y1) +
//		tx * (1.0f - ty) * field(x1, y0) +
//		tx * ty * field(x1, y1);
//		/*(1.0f - tx) * (1.0f - ty) * at(*field, x0, y0) +
//		(1.0f - tx) * ty * at(*field, x0, y1) +
//		tx * (1.0f - ty) * at(*field, x1, y0) +
//		tx * ty * at(*field, x1, y1);*/
//
//	return bilerpedValue;
//}
//
//float EulerianFluid::sampleFieldVelX(const std::vector<float>& field, Vec2 pos) {
//	return sampleField(spanFrom(field).asConst(), pos, Vec2(0.0f, cellSpacing / 2.0f));
//}
//
//float EulerianFluid::sampleFieldVelY(const std::vector<float>& field, Vec2 pos) {
//	return sampleField(spanFrom(field).asConst(), pos, Vec2(cellSpacing / 2.0f, 0.0f));
//}
//
//float EulerianFluid::sampleQuantity(Span2d<const float> field, Vec2 pos) {
//	return sampleField(field, pos, Vec2(cellSpacing / 2.0f));
//}
//
////auto EulerianFluid::advectVelocity(float dt) -> void {
////	newVelX = velX;
////	newVelY = velY;
////
////	for (i64 x = 1; x < gridSize.x; x++) {
////		for (i64 y = 1; y < gridSize.y; y++) {
////			if (isWall(x, y))
////				continue;
////
////			// Going back at step in a straight line and sampling the average previous pos to get the new velocity is called semi-lagrangian advection. This introduces viscosity. An alternative to this would be to step the forward and distribute the values to the cells closest to the new position, which would be more difficult to implement especially in a staggered grid.
////			if (!isWall(x - 1, y) && y < gridSize.y - 1) {
////				const auto pos = Vec2{ x + 0.0f, y + 0.5f } *cellSpacing;
////				const auto avgVelY = (at(velY, x - 1, y) + at(velY, x, y) + at(velY, x - 1, y + 1) + at(velY, x, y + 1)) / 4.0f;
////				const Vec2 vel{ at(velX, x, y), avgVelY };
////				const auto approximatePreviousPos = pos - vel * dt;
////				at(newVelX, x, y) = sampleField(approximatePreviousPos, FieldType::VEL_X);
////			}
////
////			if (!isWall(x, y - 1) && x < gridSize.x - 1) {
////				const auto pos = Vec2{ x + 0.5f, y + 0.0f } *cellSpacing;
////				const auto avgVelX = (at(velX, x, y - 1) + at(velX, x, y) + at(velX, x + 1, y - 1) + at(velX, x + 1, y)) / 4.0f;
////				const Vec2 vel{ avgVelX, at(velY, x, y) };
////				const auto approximatePreviousPos = pos - vel * dt;
////				at(newVelY, x, y) = sampleField(approximatePreviousPos, FieldType::VEL_Y);
////			}
////		}
////	}
////
////	velX = newVelX;
////	velY = newVelY;
////}
//
//auto EulerianFluid::advectVelocity(float dt) -> void {
//
//	//newVelX = velX;
//	//newVelY = velY;
//
//	//for (i64 x = 1; x < gridSize.x; x++) {
//	//	for (i64 y = 1; y < gridSize.y; y++) {
//	//		if (isWall(x, y))
//	//			continue;
//
//	//		// Going back at step in a straight line and sampling the average previous pos to get the new velocity is called semi-lagrangian advection. This introduces viscosity. An alternative to this would be to step the forward and distribute the values to the cells closest to the new position, which would be more difficult to implement especially in a staggered grid.
//	//		if (!isWall(x - 1, y) && y < gridSize.y - 1) {
//	//			const auto pos = Vec2{ x + 0.0f, y + 0.5f } *cellSpacing;
//	//			const auto avgVelY = (at(velY, x - 1, y) + at(velY, x, y) + at(velY, x - 1, y + 1) + at(velY, x, y + 1)) / 4.0f;
//	//			const Vec2 vel{ at(velX, x, y), avgVelY };
//	//			const auto approximatePreviousPos = pos - vel * dt;
//	//			at(newVelX, x, y) = sampleFieldVelX(velX, approximatePreviousPos);
//	//		}
//
//	//		if (!isWall(x, y - 1) && x < gridSize.x - 1) {
//	//			const auto pos = Vec2{ x + 0.5f, y + 0.0f } *cellSpacing;
//	//			const auto avgVelX = (at(velX, x, y - 1) + at(velX, x, y) + at(velX, x + 1, y - 1) + at(velX, x + 1, y)) / 4.0f;
//	//			const Vec2 vel{ avgVelX, at(velY, x, y) };
//	//			const auto approximatePreviousPos = pos - vel * dt;
//	//			at(newVelY, x, y) = sampleFieldVelX(velY, approximatePreviousPos);
//	//		}
//	//	}
//	//}
//
//	//velX = newVelX;
//	//velY = newVelY;
//
//	oldVelX = velX;
//	oldVelY = velY;
//
//	for (i64 y = 1; y < gridSize.y; y++) {
//		for (i64 x = 1; x < gridSize.x; x++) {
//			if (isWall(x, y))
//				continue;
//
//			// Going back at step in a straight line and sampling the average previous pos to get the new velocity is called semi-lagrangian advection. This introduces viscosity. An alternative to this would be to step the forward and distribute the values to the cells closest to the new position, which would be more difficult to implement especially in a staggered grid.
//			if (!isWall(x - 1, y) && y < gridSize.y - 1) {
//				const auto pos = Vec2(x + 0.0f, y + 0.5f) * cellSpacing;
//				const auto avgVelY = (at(oldVelY, x - 1, y) + at(oldVelY, x, y) + at(oldVelY, x - 1, y + 1) + at(oldVelY, x, y + 1)) / 4.0f;
//				const Vec2 vel(at(oldVelX, x, y), avgVelY);
//				const auto approximatePreviousPos = pos - vel * dt;
//				at(velX, x, y) = sampleFieldVelX(oldVelX, approximatePreviousPos);
//			}
//
//			if (!isWall(x, y - 1) && x < gridSize.x - 1) {
//				const auto pos = Vec2(x + 0.5f, y + 0.0f) * cellSpacing;
//				const auto avgVelX = (at(oldVelX, x, y - 1) + at(oldVelX, x, y) + at(oldVelX, x + 1, y - 1) + at(oldVelX, x + 1, y)) / 4.0f;
//				const Vec2 vel(avgVelX, at(oldVelY, x, y));
//				const auto approximatePreviousPos = pos - vel * dt;
//				at(velY, x, y) = sampleFieldVelY(oldVelY, approximatePreviousPos);
//			}
//		}
//	}
//}
//
//void EulerianFluid::advectQuantity(Span2d<float> quantity, float dt) {
//	ASSERT(quantity.sizeX() == gridSize.x);
//	ASSERT(quantity.sizeY() == gridSize.y);
//
//	memcpy(advectedQuantityOld.data(), quantity.data(), quantity.sizeX() * quantity.sizeY() * sizeof(float));
//
//	for (i64 y = 1; y < gridSize.y - 1; y++) {
//		for (i64 x = 1; x < gridSize.x - 1; x++) {
//			if (isWall(x, y))
//				continue;
//
//			// Read advect velocity.
//			const auto avgVel = Vec2{ at(velX, x, y) + at(velX, x + 1, y), at(velY, x, y) + at(velY, x, y + 1) } / 2.0f;
//			const auto pos = (Vec2{ Vec2T{ x, y } } + Vec2{ 0.5f })* cellSpacing;
//			const auto approximatePreviousPos = pos - dt * avgVel;
//			/*at(newSmoke, x, y) = sampleField(approximatePreviousPos, FieldType::SMOKE);*/
//			/*at(newSmoke, x, y) = sampleField(quantity.asConst(), approximatePreviousPos, Vec2(cellSpacing / 2.0f));*/
//			//quantity(newSmoke, x, y) = sampleField(spanFrom(advectedQuantityOld).asConst(), approximatePreviousPos, Vec2(cellSpacing / 2.0f));
//			quantity(x, y) = sampleQuantity(spanFrom(advectedQuantityOld).asConst(), approximatePreviousPos);
//		}
//	}
//
//	//for (i64 x = 1; x < gridSize.x - 1; x++) {
//	//	for (i64 y = 1; y < gridSize.y - 1; y++) {
//	//		if (isWall(x, y))
//	//			continue;
//
//	//		// Read advect velocity.
//	//		const auto avgVel = Vec2{ at(velX, x, y) + at(velX, x + 1, y), at(velY, x, y) + at(velY, x, y + 1) } / 2.0f;
//	//		const auto pos = (Vec2{ Vec2T{ x, y } } + Vec2{ 0.5f }) * cellSpacing;
//	//		const auto approximatePreviousPos = pos - dt * avgVel;
//	//		/*at(newSmoke, x, y) = sampleField(approximatePreviousPos, FieldType::SMOKE);*/
//	//		/*at(newSmoke, x, y) = sampleField(quantity.asConst(), approximatePreviousPos, Vec2(cellSpacing / 2.0f));*/
//	//		//quantity(newSmoke, x, y) = sampleField(spanFrom(advectedQuantityOld).asConst(), approximatePreviousPos, Vec2(cellSpacing / 2.0f));
//	//		quantity(x, y) = sampleQuantity(spanFrom(advectedQuantityOld).asConst(), approximatePreviousPos);
//	//	}
//	//}
//}
//
//auto EulerianFluid::update(float dt, float gravity, i32 solverIterations) -> void {
//	integrate(dt, gravity);
//	fill(pressure.begin(), pressure.end(), 0.0f);
//	solveIncompressibility(solverIterations, dt);
//	advectVelocity(dt);
//}
//
//auto EulerianFluid::setIsWall(i64 x, i64 y, bool value) -> void {
//	/*isWallValues[x * gridSize.y + y] = value;*/
//	isWallValues[y * gridSize.x + x] = value;
//}
//
//auto EulerianFluid::isWall(i64 x, i64 y) const -> bool {
//	return isWallValues[y * gridSize.x + x];
//}