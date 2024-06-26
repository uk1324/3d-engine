#pragma once

#include <engine/Math/Vec2.hpp>
#include <engine/Math/Aabb.hpp>
#include <platformer/Level.hpp>

struct GameConstants {
	Vec2 playerSize = Vec2(20.0f, 30.0f);
	f32 cellSize = 20.0f;
	f32 doubleJumpOrbRadius = 10.0f;
	f32 doubleJumpOrbCooldown = 3.0f;
	f32 attractingOrbRadius = 10.0f;
	f32 attractingOrbCooldown = 3.0f;
};

const GameConstants& constants();

Aabb playerAabb(Vec2 playerPosition);
Aabb roomAabb(const LevelRoom& room);
Aabb roomAabb(Vec2T<i32> position, Vec2T<i64> size);