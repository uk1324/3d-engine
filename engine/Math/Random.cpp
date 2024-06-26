#include "Random.hpp"
#include "Angles.hpp"
#include <random>

float random01() {
	std::random_device rd;
	std::mt19937 e(rd());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	return dist(e);
}

float randomFromRange(f32 min, f32 max) {
	return min + (max - min) * random01();
}

Vec3 random01Vec3() {
	return Vec3(random01(), random01(), random01());
}

Vec2 randomPointInUnitCircle() {
	const auto r = sqrt(random01());
	const auto angle = random01() * TAU<float>;
	return Vec2::oriented(angle) * r;
}
