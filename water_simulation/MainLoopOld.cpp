#include <water_simulation/MainLoop.hpp>
#include <water_simulation/Shaders/texturedQuadData.hpp>
#include <framework/ShaderManager.hpp>
#include <framework/Instancing.hpp>
#include <StructUtils.hpp>
#include <engine/Math/Random.hpp>
#include <engine/Math/Aabb.hpp>
#include <framework/Camera.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Window.hpp>
#include <engine/Utils/Gui.hpp>
#include <imgui/imgui.h>
#include <glad/glad.h>
#include <iostream>
#include <Bits.hpp>
#include <water_simulation/AdvectionDemo.hpp>
#include <water_simulation/Fluid.hpp>

static bool obstacleSetWall = true;

struct EuleraianFluidSimulator {
	float density;
	i64 numX;
	i64 numY;
	i64 numCells;
	float cellSize;
	std::vector<float> u;
	std::vector<float> v;
	std::vector<float> newU;
	std::vector<float> newV;
	std::vector<float> p;
	//std::vector<float> s; // isNotWall
	std::vector<bool> isWall;
	std::vector<float> r;
	std::vector<float> g;
	std::vector<float> b;
	std::vector<float> newM;

	float overRelaxation = 1.9f;

	EuleraianFluidSimulator(float density, i64 numX, i64 numY, float cellSize)
		: density(density)
		, numX(numX + 2)
		, numY(numY + 2)
		, cellSize(cellSize) {
		numCells = this->numX * this->numY;

		u.resize(numCells);
		v.resize(numCells);
		newU.resize(numCells);
		newV.resize(numCells);
		p.resize(numCells);
		isWall.resize(numCells);
		r.resize(numCells, 0.0f);
		g.resize(numCells, 0.0f);
		b.resize(numCells, 0.0f);
		newM.resize(numCells);
	}

	void integrate(float dt, float gravity) {
		const auto n = numY;
		for (i64 i = 1; i < numX; i++) {
			for (i64 j = 1; j < numY - 1; j++) {
				if (!isWall[i * n + j] && !isWall[i * n + j - 1]) {
					v[i * n + j] += gravity * dt;
				}
					
			}
		}
	}

	void solveIncompressibility(i32 numIters, float dt) {
		const auto n = numY;
		const auto cp = density * cellSize / dt;

		//for (;;) {

		//	//for (i32 iter = 0; iter < numIters; iter++) {

		//		for (i64 i = 1; i < numX - 1; i++) {
		//			for (i64 j = 1; j < numY - 1; j++) {

		//				if (isWall[i * n + j])
		//					continue;

		//				const auto sx0 = !isWall[(i - 1) * n + j];
		//				const auto sx1 = !isWall[(i + 1) * n + j];
		//				const auto sy0 = !isWall[i * n + j - 1];
		//				const auto sy1 = !isWall[i * n + j + 1];
		//				// Redeclaration in original source.
		//				const auto outflowingSidesCount = sx0 + sx1 + sy0 + sy1;
		//				/*if (s == 0.0)
		//					continue;*/
		//				if (outflowingSidesCount == 0)
		//					continue;

		//				// Central difference on staggered grid.
		//				const auto div = u[(i + 1) * n + j] - u[i * n + j] + v[i * n + j + 1] - v[i * n + j];
		//				auto p = -div / outflowingSidesCount;
		//				p *= overRelaxation;
		//				this->p[i * n + j] += cp * p;

		//				u[i * n + j] -= sx0 * p;
		//				u[(i + 1) * n + j] += sx1 * p;
		//				v[i * n + j] -= sy0 * p;
		//				v[i * n + j + 1] += sy1 * p;
		//			}
		//		}
		//	//}
		//	float maxDiv = -std::numeric_limits<float>::infinity();

		//	for (int i = 1; i < numX - 1; i++) {
		//		for (int j = 1; j < numY - 1; j++) {
		//			const auto div = u[(i + 1) * n + j] - u[i * n + j] + v[i * n + j + 1] - v[i * n + j];
		//			if (std::abs(div) > maxDiv) {
		//				maxDiv = std::abs(div);
		//			}
		//		}
		//	}
		//	if (maxDiv < 0.11f) {
		//		std::cout << maxDiv << '\n';
		//		break;
		//	}

		//}

		for (i32 iter = 0; iter < numIters; iter++) {

			for (i64 i = 1; i < numX - 1; i++) {
				for (i64 j = 1; j < numY - 1; j++) {

					if (isWall[i * n + j])
						continue;



					const auto sx0 = !isWall[(i - 1) * n + j];
					const auto sx1 = !isWall[(i + 1) * n + j];
					const auto sy0 = !isWall[i * n + j - 1];
					const auto sy1 = !isWall[i * n + j + 1];
					// Redeclaration in original source.
					const auto outflowingSidesCount = sx0 + sx1 + sy0 + sy1;
					/*if (s == 0.0)
						continue;*/
					if (outflowingSidesCount == 0)
						continue;

					// Central difference on staggered grid.
					const auto div = u[(i + 1) * n + j] - u[i * n + j] + v[i * n + j + 1] - v[i * n + j];

					auto p = -div / outflowingSidesCount;
					p *= overRelaxation;
					this->p[i * n + j] += cp * p;

					u[i * n + j] -= sx0 * p;
					u[(i + 1) * n + j] += sx1 * p;
					v[i * n + j] -= sy0 * p;
					v[i * n + j + 1] += sy1 * p;
				}
			}
		}
	}

	void extrapolate() {
		const auto n = numY;
		for (i64 i = 0; i < numX; i++) {
			u[i * n + 0] = u[i * n + 1];
			u[i * n + numY - 1] = u[i * n + numY - 2];
		}
		for (i64 j = 0; j < numY; j++) {
			v[0 * n + j] = v[1 * n + j];
			v[(numX - 1) * n + j] = v[(numX - 2) * n + j];
		}
	}

	float sampleField(const std::vector<float>& field, float dx, float dy, float x, float y) {
		const auto n = numY;
		//const auto h = this.h;
		const auto h1 = 1.0f / cellSize;
		const auto h2 = 0.5f * cellSize;

		/*x = Math.max(Math.min(x, this.numX * h), h);
		y = Math.max(Math.min(y, this.numY * h), h);*/
		x = std::max(std::min(static_cast<float>(x), numX * cellSize), cellSize);
		y = std::max(std::min(static_cast<float>(y), numY * cellSize), cellSize);

		/*auto dx = 0.0f;
		auto dy = 0.0f;*/

		//std::vector<float>* f = &field;

		/*switch (field) {
		case U_FIELD: f = &u; dy = h2; break;
		// Shouldn't this be dy = y2???????????????????????????????Fix in other function if true
		case V_FIELD: f = &v; dx = h2; break;
		case S_FIELD: f = &m; dx = h2; dy = h2; break;

		}*/

		auto x0 = std::min(floor((x - dx) * h1), static_cast<float>(numX - 1));
		auto tx = ((x - dx) - x0 * cellSize) * h1;
		auto x1 = std::min(x0 + 1, static_cast<float>(numX - 1));

		auto y0 = std::min(floor((y - dy) * h1), static_cast<float>(numY - 1));
		auto ty = ((y - dy) - y0 * cellSize) * h1;
		auto y1 = std::min(y0 + 1, static_cast<float>(numY - 1));

		auto sx = 1.0f - tx;
		auto sy = 1.0f - ty;

		auto val = sx * sy * field[static_cast<i64>(x0 * n + y0)] +
			tx * sy * field[static_cast<i64>(x1 * n + y0)] +
			tx * ty * field[static_cast<i64>(x1 * n + y1)] +
			sx * ty * field[static_cast<i64>(x0 * n + y1)];

		return val;
	}

	float sampleStaggeredField(const std::vector<float>& field, float x, float y) {
		return sampleField(field, cellSize / 2.0f, 0.0f, x, y);
	}

	/*float sampleU(float x, float y) {
		return sampleField(u, cellSize / 2.0f, 0.0f, x, y);
	}

	float sampleV(float x, float y) {
		return sampleField(v, cellSize / 2.0f, 0.0f, x, y);
	}*/

	float sampleSmoke(std::vector<float>& s, float x, float y) {
		return sampleField(s, cellSize / 2.0f, cellSize / 2.0f, x, y);
	}

	float avgU(i64 i, i64 j) {
		auto n = numY;
		auto u0 = (u[i * n + j - 1] + u[i * n + j] +
			u[(i + 1) * n + j - 1] + u[(i + 1) * n + j]) * 0.25f;
		return u0;

	}

	float avgV(i64 i, i64 j) {
		auto n = numY;
		auto v0 = (v[(i - 1) * n + j] + v[i * n + j] +
			v[(i - 1) * n + j + 1] + v[i * n + j + 1]) * 0.25f;
		return v0;
	}

	void advectVel(float dt) {
		for (int i = 0; i < newU.size(); i++) {
			newU[i] = u[i];
		}

		for (int i = 0; i < newV.size(); i++) {
			newV[i] = v[i];
		}

		/*this.newU.set(this.u);
		this.newV.set(this.v);*/

		auto n = numY;
		auto h2 = 0.5 * cellSize;

		for (i64 i = 1; i < numX; i++) {
			for (i64 j = 1; j < numY; j++) {

				//cnt++;

				auto sampleVelocity = [&](Vec2 pos) {
					return Vec2(
						sampleStaggeredField(this->u, pos.x, pos.y),
						sampleStaggeredField(this->v, pos.x, pos.y)
					);
				};

				auto rk4IntegratePosition = [&](Vec2 vel, Vec2 pos) {
					//const auto pos = Vec2(x, y);
					//Vec2 k1(u, v);
					Vec2 k1 = vel;
					Vec2 k2 = sampleVelocity(pos - k1 * (dt / 2.0f));
					Vec2 k3 = sampleVelocity(pos - k2 * (dt / 2.0f));
					Vec2 k4 = sampleVelocity(pos - dt * k3);
					return pos - (dt / 6.0f) * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
				};

				// u component
				if (!isWall[i * n + j] && !isWall[(i - 1) * n + j] && j < numY - 1) {
					auto x = i * cellSize;
					auto y = j * cellSize + h2;
					auto u = this->u[i * n + j];
					auto v = avgV(i, j);

					//						var v = this.sampleField(x,y, V_FIELD);
					x = x - dt * u;
					y = y - dt * v;
					//u = sampleField(x, y, U_FIELD);
					u = sampleStaggeredField(this->u, x, y);
					newU[i * n + j] = u;

					//auto x = i * cellSize;
					//auto y = j * cellSize + h2;
					//auto u = this->u[i * n + j];
					//auto v = avgV(i, j);

					////						var v = this.sampleField(x,y, V_FIELD);
					//Vec2 pos = rk4IntegratePosition(Vec2(u, v), Vec2(x, y));
					////u = sampleField(x, y, U_FIELD);
					//u = sampleStaggeredField(this->u, pos.x, pos.y);
					//newU[i * n + j] = u;
				}
				// v component
				if (!isWall[i * n + j] && !isWall[i * n + j - 1] && i < numX - 1) {
					auto x = i * cellSize + h2;
					auto y = j * cellSize;
					auto u = avgU(i, j);
					//						var u = this.sampleField(x,y, U_FIELD);
					auto v = this->v[i * n + j];
					x = x - dt * u;
					y = y - dt * v;
					/*v = sampleField(x, y, V_FIELD);*/
					v = sampleStaggeredField(this->v, x, y);
					newV[i * n + j] = v;

					//auto x = i * cellSize + h2;
					//auto y = j * cellSize;
					//auto u = avgU(i, j);
					////						var u = this.sampleField(x,y, U_FIELD);
					//auto v = this->v[i * n + j];
					//Vec2 pos = rk4IntegratePosition(Vec2(u, v), Vec2(x, y));
					///*v = sampleField(x, y, V_FIELD);*/
					//v = sampleStaggeredField(this->v, pos.x, pos.y);
					//newV[i * n + j] = v;
				}
			}
		}

		for (int i = 0; i < newU.size(); i++) {
			u[i] = newU[i];
		}

		for (int i = 0; i < newV.size(); i++) {
			v[i] = newV[i];
		}

		/*this->u.set(this->newU);
		this->v.set(this->newV);*/
	}

	void advectSmoke(std::vector<float>& m, float dt) {


		//newM.set(this.m);
		for (int i = 0; i < newM.size(); i++) {
			newM[i] = m[i];
		}

		auto n = this->numY;
		//auto h = this.h;
		auto h2 = 0.5 * cellSize;

		for (auto i = 1; i < numX - 1; i++) {
			for (auto j = 1; j < numY - 1; j++) {

				if (!isWall[i * n + j]) {
					auto u = (this->u[i * n + j] + this->u[(i + 1) * n + j]) * 0.5;
					auto v = (this->v[i * n + j] + this->v[i * n + j + 1]) * 0.5;
					auto x = i * cellSize + h2 - dt * u;
					auto y = j * cellSize + h2 - dt * v;

					/*newM[i * n + j] = sampleField(x, y, S_FIELD);*/
					newM[i * n + j] = sampleSmoke(m, x, y);
				}
			}
		}

		//this.m.set(this.newM);
		for (int i = 0; i < newM.size(); i++) {
			m[i] = newM[i];
		}
	}


	void simulate(float dt, float gravity, float numIters) {

		integrate(dt, gravity);

		//p.fill(0.0);
		for (int i = 0; i < p.size(); i++) {
			p[i] = 0.0f;
		}
		solveIncompressibility(numIters, dt);

		extrapolate();
		advectVel(dt);
		advectSmoke(r, dt);
		advectSmoke(g, dt);
		advectSmoke(b, dt);
	}

	float lastObstacleX = -1.0f;
	float lastObstacleY = -1.0f;

	const float obstacleRadius = 0.1f;
	i64 frameNumber = 0;

	void setObstacle(float x, float y, bool reset, float dt) {
		frameNumber++;

		auto vx = 0.0f;
		auto vy = 0.0f;

		if (!reset && lastObstacleX != -1.0f) {
			vx = (x - lastObstacleX) / dt;
			vy = (y - lastObstacleY) / dt;
		}

		lastObstacleX = x;
		lastObstacleY = y;
		auto r = obstacleRadius;
		//auto f = scene.fluid;
		auto& f = *this;
		auto n = f.numY;
		auto cd = sqrt(2) * f.cellSize;

		for (i64 i = 1; i < f.numX - 2; i++) {
			for (i64 j = 1; j < f.numY - 2; j++) {

				//f.s[i * n + j] = 1.0;
				f.isWall[i * n + j] = false;

				auto dx = (i + 0.5) * f.cellSize - x;
				auto dy = (j + 0.5) * f.cellSize - y;

				if (dx * dx + dy * dy < r * r) {
					/*if (obstacleSetWall) {
						f.isWall[i * n + j] = true;
						
						f.u[i * n + j] = vx;
						f.u[(i + 1) * n + j] = vx;
						f.v[i * n + j] = vy;
						f.v[i * n + j + 1] = vy;
					} else {
						f.u[i * n + j] += vx;
						f.u[(i + 1) * n + j] += vx;
						f.v[i * n + j] += vy;
						f.v[i * n + j + 1] += vy;
					}*/
					
					/*if (obstacleSetWall) {
						f.isWall[i * n + j] = true;
					} 

					f.u[i * n + j] = vx;
					f.u[(i + 1) * n + j] = vx;
					f.v[i * n + j] = vy;
					f.v[i * n + j + 1] = vy;*/

					if (obstacleSetWall) {
						f.isWall[i * n + j] = true;
						
						f.u[i * n + j] = vx;
						f.u[(i + 1) * n + j] = vx;
						f.v[i * n + j] = vy;
						f.v[i * n + j + 1] = vy;
					} else {
						f.u[i * n + j] += vx;
						f.u[(i + 1) * n + j] += vx;
						f.v[i * n + j] += vy;
						f.v[i * n + j + 1] += vy;
					}
				}
			}
		}

		//scene.showObstacle = true;
	}
};

struct Fluid {
	float density;
	i64 numX;
	i64 numY;
	i64 numCells;
	// cell size
	float h;
	std::vector<float> u;
	std::vector<float> v;
	std::vector<float> newU;
	std::vector<float> newV;
	std::vector<float> p;
	std::vector<float> s; // isNotWall
	std::vector<float> r;
	std::vector<float> g;
	std::vector<float> b;
	std::vector<float> newM;

	float overRelaxation = 1.9f;

	Fluid(float density, i64 numX, i64 numY, float h) 
		: density(density)
		, numX(numX + 2)
		, numY(numY + 2)
		, h(h) {
		numCells = this->numX * this->numY;

		u.resize(numCells);
		v.resize(numCells);
		newU.resize(numCells);
		newV.resize(numCells);
		p.resize(numCells);
		s.resize(numCells);
		r.resize(numCells, 0.0f);
		g.resize(numCells, 0.0f);
		b.resize(numCells, 0.0f);
		newM.resize(numCells);
	}

	void integrate(float dt, float gravity) {
		const auto n = numY;
		for (i64 i = 1; i < numX; i++) {
			for (i64 j = 1; j < numY - 1; j++) {
				if (s[i * n + j] != 0.0 && s[i * n + j - 1] != 0.0)
					v[i * n + j] += gravity * dt;
			}
		}
	}

	void solveIncompressibility(i32 numIters, float dt) {

		const auto n = numY;
		const auto cp = density * h / dt;

		for (i32 iter = 0; iter < numIters; iter++) {

			for (i64 i = 1; i < numX - 1; i++) {
				for (i64 j = 1; j < numY - 1; j++) {

					if (s[i * n + j] == 0.0)
						continue;

					//const auto s = this->s[i * n + j];
					const auto sx0 = this->s[(i - 1) * n + j];
					const auto sx1 = this->s[(i + 1) * n + j];
					const auto sy0 = this->s[i * n + j - 1];
					const auto sy1 = this->s[i * n + j + 1];
					// Redeclaration in original source.
					const auto s = sx0 + sx1 + sy0 + sy1;
					if (s == 0.0)
						continue;

					// Central difference on staggered grid.
					const auto div = u[(i + 1) * n + j] - u[i * n + j] + v[i * n + j + 1] - v[i * n + j];

					auto p = -div / s;
					p *= overRelaxation;
					this->p[i * n + j] += cp * p;

					u[i * n + j] -= sx0 * p;
					u[(i + 1) * n + j] += sx1 * p;
					v[i * n + j] -= sy0 * p;
					v[i * n + j + 1] += sy1 * p;
				}
			}
		}
	}

	void extrapolate() {
		const auto n = numY;
		for (i64 i = 0; i < numX; i++) {
			u[i * n + 0] = u[i * n + 1];
			u[i * n + numY - 1] = u[i * n + numY - 2];
		}
		for (i64 j = 0; j < numY; j++) {
			v[0 * n + j] = v[1 * n + j];
			v[(numX - 1) * n + j] = v[(numX - 2) * n + j];
		}
	}

	float sampleField(std::vector<float>& field, float dx, float dy, float x, float y) {
		const auto n = numY;
		//const auto h = this.h;
		const auto h1 = 1.0f / h;
		const auto h2 = 0.5f * h;

		/*x = Math.max(Math.min(x, this.numX * h), h);
		y = Math.max(Math.min(y, this.numY * h), h);*/
		x = std::max(std::min(static_cast<float>(x), numX * h), h);
		y = std::max(std::min(static_cast<float>(y), numY * h), h);

		/*auto dx = 0.0f;
		auto dy = 0.0f;*/

		std::vector<float>* f = &field;

		/*switch (field) {
		case U_FIELD: f = &u; dy = h2; break;
		// Shouldn't this be dy = y2???????????????????????????????Fix in other function if true
		case V_FIELD: f = &v; dx = h2; break;
		case S_FIELD: f = &m; dx = h2; dy = h2; break;

		}*/

		auto x0 = std::min(floor((x - dx) * h1), static_cast<float>(numX - 1));
		auto tx = ((x - dx) - x0 * h) * h1;
		auto x1 = std::min(x0 + 1, static_cast<float>(numX - 1));

		auto y0 = std::min(floor((y - dy) * h1), static_cast<float>(numY - 1));
		auto ty = ((y - dy) - y0 * h) * h1;
		auto y1 = std::min(y0 + 1, static_cast<float>(numY - 1));

		auto sx = 1.0f - tx;
		auto sy = 1.0f - ty;

		auto val = sx * sy * (*f)[static_cast<i64>(x0 * n + y0)] +
			tx * sy * (*f)[static_cast<i64>(x1 * n + y0)] +
			tx * ty * (*f)[static_cast<i64>(x1 * n + y1)] +
			sx * ty * (*f)[static_cast<i64>(x0 * n + y1)];

		return val;
	}

	float sampleU(float x, float y) {
		return sampleField(u, h / 2.0f, 0.0f, x, y);
	}

	float sampleV(float x, float y) {
		return sampleField(v, h / 2.0f, 0.0f, x, y);
	}

	float sampleSmoke(std::vector<float>& s, float x, float y) {
		return sampleField(s, h / 2.0f, h / 2.0f, x, y);
	}

	float avgU(i64 i, i64 j) {
		auto n = numY;
		auto u0 = (u[i * n + j - 1] + u[i * n + j] +
			u[(i + 1) * n + j - 1] + u[(i + 1) * n + j]) * 0.25f;
		return u0;

	}

	float avgV(i64 i, i64 j) {
		auto n = numY;
		auto v0 = (v[(i - 1) * n + j] + v[i * n + j] +
			v[(i - 1) * n + j + 1] + v[i * n + j + 1]) * 0.25f;
		return v0;
	}

	void advectVel(float dt) {
		for (int i = 0; i < newU.size(); i++) {
			newU[i] = u[i];
		}

		for (int i = 0; i < newV.size(); i++) {
			newV[i] = v[i];
		}

		/*this.newU.set(this.u);
		this.newV.set(this.v);*/

		auto n = numY;
		auto h = this->h;
		auto h2 = 0.5 * h;

		for (i64 i = 1; i < numX; i++) {
			for (i64 j = 1; j < numY; j++) {

				//cnt++;

				// u component
				if (s[i * n + j] != 0.0 && s[(i - 1) * n + j] != 0.0 && j < numY - 1) {
					auto x = i * h;
					auto y = j * h + h2;
					auto u = this->u[i * n + j];
					auto v = avgV(i, j);
					//						var v = this.sampleField(x,y, V_FIELD);
					x = x - dt * u;
					y = y - dt * v;
					//u = sampleField(x, y, U_FIELD);
					u = sampleU(x, y);
					newU[i * n + j] = u;
				}
				// v component
				if (s[i * n + j] != 0.0 && s[i * n + j - 1] != 0.0 && i < numX - 1) {
					auto x = i * h + h2;
					auto y = j * h;
					auto u = avgU(i, j);
					//						var u = this.sampleField(x,y, U_FIELD);
					auto v = this->v[i * n + j];
					x = x - dt * u;
					y = y - dt * v;
					/*v = sampleField(x, y, V_FIELD);*/
					v = sampleV(x, y);
					newV[i * n + j] = v;
				}
			}
		}

		for (int i = 0; i < newU.size(); i++) {
			u[i] = newU[i];
		}

		for (int i = 0; i < newV.size(); i++) {
			v[i] = newV[i];
		}

		/*this->u.set(this->newU);
		this->v.set(this->newV);*/
	}

	void advectSmoke(std::vector<float>& m, float dt) {


		//newM.set(this.m);
		for (int i = 0; i < newM.size(); i++) {
			newM[i] = m[i];
		}

		auto n = this->numY;
		//auto h = this.h;
		auto h2 = 0.5 * h;

		for (auto i = 1; i < numX - 1; i++) {
			for (auto j = 1; j < numY - 1; j++) {

				if (s[i * n + j] != 0.0) {
					auto u = (this->u[i * n + j] + this->u[(i + 1) * n + j]) * 0.5;
					auto v = (this->v[i * n + j] + this->v[i * n + j + 1]) * 0.5;
					auto x = i * h + h2 - dt * u;
					auto y = j * h + h2 - dt * v;

					/*newM[i * n + j] = sampleField(x, y, S_FIELD);*/
					newM[i * n + j] = sampleSmoke(m, x, y);
				}
			}
		}

		//this.m.set(this.newM);
		for (int i = 0; i < newM.size(); i++) {
			m[i] = newM[i];
		}
	}


	void simulate(float dt, float gravity, float numIters) {

		integrate(dt, gravity);

		//p.fill(0.0);
		for (int i = 0; i < p.size(); i++) {
			p[i] = 0.0f;
		}
		solveIncompressibility(numIters, dt);

		extrapolate();
		advectVel(dt);
		advectSmoke(r, dt);
		advectSmoke(g, dt);
		advectSmoke(b, dt);
	}

	float lastObstacleX = -1.0f;
	float lastObstacleY = -1.0f;

	const float obstacleRadius = 0.1f;
	i64 frameNumber = 0;

	void setObstacle(float x, float y, bool reset, float dt) {
		frameNumber++;

		auto vx = 0.0f;
		auto vy = 0.0f;

		if (!reset && lastObstacleX != -1.0f) {
			vx = (x - lastObstacleX) / dt;
			vy = (y - lastObstacleY) / dt;
		}

		lastObstacleX = x;
		lastObstacleY = y;
		auto r = obstacleRadius;
		//auto f = scene.fluid;
		auto& f = *this;
		auto n = f.numY;
		auto cd = sqrt(2) * f.h;

		for (i64 i = 1; i < f.numX - 2; i++) {
			for (i64 j = 1; j < f.numY - 2; j++) {

				f.s[i * n + j] = 1.0;

				auto dx = (i + 0.5) * f.h - x;
				auto dy = (j + 0.5) * f.h - y;

				if (dx * dx + dy * dy < r * r) {
					/*if (obstacleSetWall) {
						f.s[i * n + j] = 0.0;
						
						f.u[i * n + j] = vx;
						f.u[(i + 1) * n + j] = vx;
						f.v[i * n + j] = vy;
						f.v[i * n + j + 1] = vy;
					} else {
						f.u[i * n + j] += vx;
						f.u[(i + 1) * n + j] += vx;
						f.v[i * n + j] += vy;
						f.v[i * n + j + 1] += vy;
					}*/

					if (obstacleSetWall) {
						f.s[i * n + j] = 0.0;

						f.u[i * n + j] = vx;
						f.u[(i + 1) * n + j] = vx;
						f.v[i * n + j] = vy;
						f.v[i * n + j + 1] = vy;
						
					} else {
						f.u[i * n + j] += vx;
						f.u[(i + 1) * n + j] += vx;
						f.v[i * n + j] += vy;
						f.v[i * n + j + 1] += vy;
					}
					

					/*if (scene.sceneNr == 2)
						f.m[i * n + j] = 0.5 + 0.5 * Math.sin(0.1 * scene.frameNr)*/
					/*else
						f.m[i * n + j] = 1.0;*/
					//f.m[i * n + j] = 0.5 + 0.5 * sin(0.1 * frameNumber);
					
				}
			}
		}

		//scene.showObstacle = true;
	}
};

static constexpr TexturedQuadVertex fullscreenQuadVerts[]{
	{ Vec2{ -1.0f, 1.0f }, Vec2{ 0.0f, 1.0f } },
	{ Vec2{ 1.0f, 1.0f }, Vec2{ 1.0f, 1.0f } },
	{ Vec2{ -1.0f, -1.0f }, Vec2{ 0.0f, 0.0f } },
	{ Vec2{ 1.0f, -1.0f }, Vec2{ 1.0f, 0.0f } },
};

static constexpr u32 fullscreenQuadIndices[]{
	0, 1, 2, 2, 1, 3
};

//Fluid fluid(1000.0f, 200, 100, 1.0f / 100.0f);
EuleraianFluidSimulator fluid0(5000.0f, 200, 100, 1.0f / 100.0f);
Fluid fluid1(5000.0f, 200, 100, 1.0f / 100.0f);

//EulerianFluid eulerianFluid(Vec2T<i64>(200, 100), 1.0f / 100.0f);


MainLoop MainLoop::make() {

	auto image = Image32::fromFile("assets/image.png");
	Image32 resized(fluid0.numX, fluid0.numY);
	resized.copyAndResize(*image);
	/*for (int i = 0; i < fluid.r.size(); i++) {
		fluid.r[i] = random01();
		fluid.g[i] = random01();
		fluid.b[i] = random01();
	}*/

	/*for (int i = 1; i < fluid.m.size(); i++) {
		
	}*/

	for (int x = 0; x < fluid0.numX; x++) {
		for (int y = 0; y < fluid0.numY; y++) {
			if ((x == 0 || y == 0 || x == fluid0.numX - 1 || y == fluid0.numY - 1)) {
				fluid0.isWall[x * fluid0.numY + y] = true;
				fluid1.s[x * fluid0.numY + y] = 0.0f;
			} else {
				fluid0.isWall[x * fluid0.numY + y] = false;
				fluid1.s[x * fluid0.numY + y] = 1.0f;
			}
		}
	}

	auto setColor = [&](i64 x, i64 y, Vec3 color) {
		const auto off = (x * fluid0.numY + (fluid0.numY - y - 1));

		/*fluid0.r[off] = resized(x, y).r / 255.0f;
		fluid0.g[off] = resized(x, y).g / 255.0f;
		fluid0.b[off] = resized(x, y).b / 255.0f;
		fluid1.r[off] = resized(x, y).r / 255.0f;
		fluid1.g[off] = resized(x, y).g / 255.0f;
		fluid1.b[off] = resized(x, y).b / 255.0f;*/
		fluid0.r[off] = color.x / 255.0f;
		fluid0.g[off] = color.y / 255.0f;
		fluid0.b[off] = color.z / 255.0f;

		fluid1.r[off] = color.x / 255.0f;
		fluid1.g[off] = color.y / 255.0f;
		fluid1.b[off] = color.z / 255.0f;

		//eulerianFluid.at(eulerianFluid.smoke, x, y) = color.x / 255.0f;
	};

	for (int x = 0; x < fluid0.numX; x++) {
		for (int y = 0; y < fluid0.numY; y++) {

			const auto gridSize = 10;
			const auto ix = x / gridSize;
			const auto iy = y / gridSize;
			if (ix % 2 == iy % 2) {
				setColor(x, y, Vec3(0.0f));
			} else {
				setColor(x, y, Vec3(255.0f));
			}
			
		}
	}

	//for (i64 i = 0; i < fluid.numX; i++) {
	//	for (i64 j = 0; j < fluid.numY; j++) {
	//		auto s = 1.0;	// fluid
	//		if (i == 0 || i == fluid.numX - 1 || j == 0 || j == fluid.numY - 1)
	//			s = 0.0;	// solid
	//		fluid.s[i * fluid.numY + j] = s;
	//	}
	//}

	auto texture = Texture::generate();
	texture.bind();
	
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		//GL_R32F,
		GL_RGB32F,
		/*GRID_SIZE.x,
		GRID_SIZE.y,*/
		fluid0.numX,
		fluid0.numY,
		0,
		//GL_RED,
		GL_RGB,
		GL_FLOAT,
		nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	auto instancesVbo = Vbo(1024ull * 10);

	/*Array2d<float> array(GRID_SIZE);
	for (int y = 0; y < array.size().y; y++) {
		for (int x = 0; x < array.size().x; x++) {
			array(x, y) = random01();
		}
	}*/
	Array2d<float> array(GRID_SIZE);

	for (int x = 0; x < array.size().x; x++) {
		array(x, 0) = 1.0f;
		array(x, array.size().y - 1) = 1.0f;
	}

	for (int y = 0; y < array.size().y; y++) {
		array(0, y) = 1.0f;
		array(array.size().x - 1, y) = 1.0f;
	}

	for (int y = 0; y < array.size().y; y++) {
		for (int x = 0; x < array.size().x; x++) {
			if (x % 2 == y % 2) 
				array(x, y) = 0.5f;
		}
	}

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Vbo texturedQuadVbo(fullscreenQuadVerts, sizeof(fullscreenQuadVerts));
	//Ibo texturedQuadIbo(fullscreenQuadIndices, sizeof(fullscreenQuadIndices));
	//Vao texturedQuadVao = Vao::generate();
	////texturedQuadVao.bind();
	//texturedQuadVao.bind();
	//TexturedQuadInstances::addAttributesToVao(texturedQuadVao, texturedQuadVbo, instancesVbo);
	//texturedQuadVbo.bind();
	//texturedQuadIbo.bind();
	//Vao::unbind();
	//Ibo::unbind();

	auto texturedQuadVbo = Vbo(fullscreenQuadVerts, sizeof(fullscreenQuadVerts));
	auto texturedQuadIbo = Ibo(fullscreenQuadIndices, sizeof(fullscreenQuadIndices));
	auto texturedQuadVao = Vao::generate();
	TexturedQuadInstances::addAttributesToVao(texturedQuadVao, texturedQuadVbo, instancesVbo);
	texturedQuadVao.bind();
	texturedQuadIbo.bind();
	Vao::unbind();
	//Ibo::unbind();

	

	return MainLoop{
		MOVE(array),
		MOVE(texture),
		MOVE(instancesVbo),
		.texturedQuadShader = MAKE_GENERATED_SHADER(TEXTURED_QUAD),
		MOVE(texturedQuadVbo),
		MOVE(texturedQuadIbo),
		MOVE(texturedQuadVao),
	};
}

auto demo = AdvectionDemo::make();


void MainLoop::update() {
	glClear(GL_COLOR_BUFFER_BIT);

	/*demo.update();

	return;*/
	Camera camera;
	camera.aspectRatio = Window::aspectRatio();

	const auto iterationCount = 40;

	//for (int x = 0; x < fluid0.numX; x++) {
	//	for (int y = 0; y < fluid0.numY; y++) {
	//		const auto i = x, j = y;
	//		const auto off = (x + 1) * fluid0.numY + y;
	//		const auto n = fluid0.numY;
	//		if (fluid0.isWall[off]) {

	//			fluid0.u[i * n + j] = 0.0f;
	//			fluid0.u[(i + 1) * n + j] = 0.0f;
	//			fluid0.v[i * n + j] = 0.0f;
	//			fluid0.v[i * n + j + 1] = 0.0f;

	//			fluid1.u[i * n + j] = 0.0f;
	//			fluid1.u[(i + 1) * n + j] = 0.0f;
	//			fluid1.v[i * n + j] = 0.0f;
	//			fluid1.v[i * n + j + 1] = 0.0f;

	//			/*fluid0.u[off] = 0.0f;
	//			fluid0.v[off] = 0.0f;
	//			fluid1.u[off] = 0.0f;
	//			fluid1.v[off] = 0.0f;*/
	//		}
	//	}
	//}

	ImGui::Checkbox("obstalce set wall", &obstacleSetWall);


	fluid0.simulate(1.0f / 60.0f, 0.0f, iterationCount);
	fluid1.simulate(1.0f / 60.0f, 0.0f, iterationCount);
	//eulerianFluid.update(1.0f / 60.0f, 0.0f, 40);

	static Array2d<float> divergence(fluid0.numX, fluid0.numY);
	const auto n = fluid0.numY;

	for (int i = 0; i < fluid0.numX; i++) {
		for (int j = 0; j < fluid0.numY; j++) {
			divergence(i, j) = 0.0f;
		}
	}

	{
		float maxDivergence = -std::numeric_limits<float>::infinity();
		float minDivergence = std::numeric_limits<float>::infinity();
		for (int i = 1; i < fluid0.numX - 1; i++) {
			for (int j = 1; j < fluid0.numY - 1; j++) {
				const auto div = fluid0.u[(i + 1) * n + j] - fluid1.u[i * n + j] + fluid1.v[i * n + j + 1] - fluid1.v[i * n + j];
				if (div > maxDivergence) {
					maxDivergence = div;
				}
				if (div < minDivergence) {
					minDivergence = div;
				}
			}
		}

		Gui::put("max divergence %: ", maxDivergence);
		Gui::put("min divergence %: ", minDivergence);
	}

	{
		float maxDivergence = -std::numeric_limits<float>::infinity();
		float minDivergence = std::numeric_limits<float>::infinity();
		for (int i = 1; i < fluid0.numX - 1; i++) {
			for (int j = 1; j < fluid0.numY - 1; j++) {
				const auto div = fluid0.u[(i + 1) * n + j] - fluid0.u[i * n + j] + fluid0.v[i * n + j + 1] - fluid0.v[i * n + j];
				if (div > maxDivergence) {
					maxDivergence = div;
				}
				if (div < minDivergence) {
					minDivergence = div;
				}
				divergence(i, j) = div;
			}
		}

		Gui::put("max divergence %: ", maxDivergence);
		Gui::put("min divergence %: ", minDivergence);
	}

	const auto CELL_SIZE = 0.1f;

	/*const auto gridSize = CELL_SIZE * Vec2{ GRID_SIZE };*/
	const auto gridSize = CELL_SIZE * Vec2(fluid0.numX, fluid0.numY);
	const auto gridCenter = gridSize / 2.0f;
	camera.pos = gridCenter;
	camera.changeSizeToFitBox(gridSize);

	texturedQuadShader.use();
	/*glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GRID_SIZE.x, GRID_SIZE.y, GL_RED, GL_FLOAT, array.data());*/

	static std::vector<float> f;
	f.resize(fluid1.r.size() * 3);

	//auto& sampledField = fluid.m;
	/*ASSERT(fluid0.u == fluid1.u);
	ASSERT(fluid0.v == fluid1.v);*/

	/*for (int x = 0; x < fluid0.numX; x++) {
		for (int y = 0; y < fluid0.numY; y++) {
			const auto off = (x * fluid0.numY + (fluid0.numY - y - 1));
			ASSERT(!fluid0.isWall[off] == fluid1.s[off]);
		}
	}*/
	ASSERT(fluid0.u == fluid1.u);
	ASSERT(fluid0.v == fluid1.v);
	ASSERT(fluid0.p == fluid1.p);
	ASSERT(fluid0.r == fluid1.r);
	ASSERT(fluid0.g == fluid1.g);
	ASSERT(fluid0.b == fluid1.b);
	ASSERT(fluid0.newM == fluid1.newM);

	static bool showDivergence = false;
	ImGui::Checkbox("show divergence", &showDivergence);

	for (int x = 0; x < fluid1.numX; x++) {
		for (int y = 0; y < fluid1.numY; y++) {
			/*f[y * fluid.numX + x] = 1.0f - sampledField[x * fluid.numY + y];*/
			/*f[y * fluid.numX + x] = sampledField[x * fluid.numY + y];*/
			const auto offset = (y * fluid1.numX + x) * 3.0f;
			const auto a = x * fluid1.numY + y;
			//f[offset] = fluid0.isWall[a];
			//f[offset] = 1.0f - fluid1.s[a];
			if (showDivergence) {
				f[offset] = divergence(x, y);
				f[offset + 1] = 0.0f;
				f[offset + 2] = fluid0.isWall[a]; 
			} else {
				/*f[offset] = fluid1.r[a];
				f[offset + 1] = fluid1.g[a];
				f[offset + 2] = fluid1.b[a];*/
				f[offset] = fluid0.r[a];
				f[offset + 1] = fluid0.g[a];
				f[offset + 2] = fluid0.b[a];
				/*f[offset] = eulerianFluid.at(eulerianFluid.smoke, x, y);
				f[offset + 1] = eulerianFluid.at(eulerianFluid.smoke, x, y);
				f[offset + 2] = eulerianFluid.at(eulerianFluid.smoke, x, y);*/
			}
			
			//setObstacle(
		}
	}

	/*glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fluid.numX, fluid.numY, GL_RED, GL_FLOAT, f.data());*/
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fluid1.numX, fluid1.numY, GL_RGB, GL_FLOAT, f.data());
	texturedQuadShader.setTexture("renderedTexture", 0, texture);

	static std::vector<TexturedQuadInstance> instances;
	/*instances.push_back(TexturedQuadInstance{ .transform = Mat3x2::identity });*/
	instances.push_back(TexturedQuadInstance{ camera.makeTransform(gridCenter, 0.0f, gridSize / 2.0f) });

	const auto c = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
	const auto cursorPos = Vec2T<i64>{ (c / CELL_SIZE).applied(floor) };

	static Vec2 obstaclePos(0.0f);

	if (Input::isMouseButtonHeld(MouseButton::LEFT)) {
		if (cursorPos.x == array.clampX(cursorPos.x) && cursorPos.y == array.clampY(cursorPos.y)) {
			//array.at(cursorPos) = 0.75f;
			//sampledField[cursorPos.x * fluid.numY + cursorPos.y] = 5.0f;
			fluid0.setObstacle(cursorPos.x * (1.0f / 100.0f), cursorPos.y * (1.0f / 100.0f), false, 1.0f / 60.0f);
			fluid1.setObstacle(cursorPos.x * (1.0f / 100.0f), cursorPos.y * (1.0f / 100.0f), false, 1.0f / 60.0f);
			//fluid.setObstacle(cursorPos.x, cursorPos.y, true, 1.0f / 60.0f);

			const auto gridPos = cursorPos;
			const auto SPACE_BETWEEN_CELLS = 1.0f / 100.0f;
			const auto obstacleRadius = 0.05f;
			Vec2 newPos = Vec2{ gridPos } *SPACE_BETWEEN_CELLS;
			auto vel = (newPos - obstaclePos) / (1.0f / 60.0f);
			obstaclePos = newPos;

			//elapsed += Time::deltaTime();
			//auto& fluid = eulerianFluid;
			//// Skip borders
			//for (i64 x = 1; x < fluid.gridSize.x - 2; x++) {
			//	for (i64 y = 1; y < fluid.gridSize.y - 2; y++) {
			//		const auto cellPos = Vec2{ Vec2T{ x, y } } *fluid.cellSpacing;
			//		const auto n = fluid.gridSize.y;
			//		if (!fluid.isWall(x, y) && (cellPos - obstaclePos).lengthSq() < pow(obstacleRadius, 2.0f)) {
			//			// Smoke doesn't impact velocities it only advectes (moves) with them. It is used to visualize how the fluid moves. The color doesn't represent the amount of fluid at a point. The fluid is incompressible so it has the same amount everywhere. The smoke is kind of like a fluid moving inside a fluid and it can vary from place to place. 

			//			// This code just sets the value of the smoke to a changing value to better show changes and it also looks cool. Could just have different smokes values at different points and just move them.
			//			//fluid.at(fluid.smoke, x, y) = 0.5f + 0.5f * sin(elapsed / (1.0f / 60.0f) * 0.1f);
			//			fluid.at(fluid.velX, x, y) = vel.x;
			//			fluid.at(fluid.velX, x + 1, y) = vel.x;
			//			fluid.at(fluid.velY, x, y) = vel.y;
			//			fluid.at(fluid.velY, x, y + 1) = vel.y;
			//		}

			//	}
			//}

		}
	}
	if (Input::isMouseButtonUp(MouseButton::LEFT)) {
		fluid0.setObstacle(fluid0.lastObstacleX, fluid0.lastObstacleY, true, 1.0f / 60.0f);
		fluid0.lastObstacleX = -1.0f;
		fluid0.lastObstacleY = -1.0f;

		fluid1.setObstacle(fluid1.lastObstacleX, fluid1.lastObstacleY, true, 1.0f / 60.0f);
		fluid1.lastObstacleX = -1.0f;
		fluid1.lastObstacleY = -1.0f;
	}

	drawInstances(texturedQuadVao, instancesVbo, instances, [](usize instanceCount) {
		glDrawElementsInstanced(GL_TRIANGLES, std::size(fullscreenQuadIndices), GL_UNSIGNED_INT, nullptr, instanceCount);
	});

	instances.clear();

	//const Vec2 windowSize = Window::size();

	//auto aspectRatioConstraint = [](ImGuiSizeCallbackData* data) -> void {
	//	// https://github.com/ocornut/imgui/issues/6210
	//	float aspect_ratio = *reinterpret_cast<float*>(data->UserData);
	//	data->DesiredSize.x = std::max(data->DesiredSize.x, data->DesiredSize.y);
	//	data->DesiredSize.y = (float)(int)(data->DesiredSize.x / aspect_ratio);
	//};
	//const auto textureSize = GRID_SIZE;
	//float aspectRatio = textureSize.xOverY();
	//ImGui::SetNextWindowSizeConstraints(Vec2{ 0.0f }, Vec2{ INFINITY }, aspectRatioConstraint, &aspectRatio);

	////const auto textureSize = textureSize;
	//ImGui::SetNextWindowSize(Vec2{ windowSize.x * 0.3f, windowSize.x * 0.3f / textureSize.xOverY() }, ImGuiCond_Appearing);
	//ImGui::Begin("name");

	//const auto sceneWindowWindowSpace = Aabb::fromCorners(
	//	Vec2{ ImGui::GetWindowPos() } + ImGui::GetWindowContentRegionMin(),
	//	Vec2{ ImGui::GetWindowPos() } + ImGui::GetWindowContentRegionMax()
	//);
	//const auto sceneWindowSize = sceneWindowWindowSpace.size();
	//ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture.handle())), sceneWindowSize);
	//ImGui::End();
	

	//for (auto& img : debuggedImages) {
	//	if (!img.isWindowOpen) {
	//		continue;
	//	}

	//	auto aspectRatioConstraint = [](ImGuiSizeCallbackData* data) -> void {
	//		// https://github.com/ocornut/imgui/issues/6210
	//		float aspect_ratio = *reinterpret_cast<float*>(data->UserData);
	//		data->DesiredSize.x = std::max(data->DesiredSize.x, data->DesiredSize.y);
	//		data->DesiredSize.y = (float)(int)(data->DesiredSize.x / aspect_ratio);
	//	};
	//	float aspectRatio = textureSize(img.texture).xOverY();
	//	SetNextWindowSizeConstraints(Vec2{ 0.0f }, Vec2{ INFINITY }, aspectRatioConstraint, &aspectRatio);

	//	const auto textureSize = ::textureSize(img.texture);
	//	SetNextWindowSize(Vec2{ windowSize.x * 0.3f, windowSize.x * 0.3f / textureSize.xOverY() }, ImGuiCond_Appearing);
	//	Begin(img.windowName.c_str(), &img.isWindowOpen);

	//	const auto sceneWindowWindowSpace = Aabb::fromCorners(
	//		Vec2{ ImGui::GetWindowPos() } + ImGui::GetWindowContentRegionMin(),
	//		Vec2{ ImGui::GetWindowPos() } + ImGui::GetWindowContentRegionMax()
	//	);
	//	const auto sceneWindowSize = sceneWindowWindowSpace.size();
	//	ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(img.texture.id)), sceneWindowSize);
	//	End();
	//	if (img.autoRefresh) {
	//		img.refresh();
	//	}
	//}

	//ImGui::Image(reinterpret_cast<void*>(texture.handle()), ImVec2());
}
