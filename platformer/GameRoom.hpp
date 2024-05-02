#pragma once

#include <vector>
#include <platformer/Entities.hpp>
#include <platformer/Player.hpp>

struct GameRoom {
	std::vector<Block> blocks;
	std::vector<Spike> spikes;
	std::vector<Platform> platforms;
	std::vector<DoubleJumpOrb> doubleJumpOrbs;
	std::vector<MovingBlock> movingBlocks;
};

void collisionDetection(f32 dt, std::vector<GameRoom*> rooms, Player& player);