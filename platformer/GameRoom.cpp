#include "GameRoom.hpp"
#include <platformer/Collision.hpp>
#include <platformer/Constants.hpp>

struct Distance {
    f32 left, right, top, bottom;
};

Distance getDistance(const Aabb& a, const Aabb& b, Vec2 aVel) {
    const auto aSize = a.size();
    const auto bSize = b.size();

    return Distance{
        .left = std::abs(a.min.x + aSize.x - b.min.x) - aVel.x,
        .right = std::abs(a.min.x - (b.min.x + bSize.x)) + aVel.x,
        .top = std::abs(a.min.y - (b.min.y + bSize.y)) + aVel.y,
        .bottom = std::abs(a.min.y + aSize.y - b.min.y) - aVel.y,
    };
}

bool blockCollision(
    Player& player,
    Vec2& movement,
    const Aabb& playerAabb,
    const Aabb& blockAabb,
    BlockCollsionDirectionsBitfield collisionDirections,
    Vec2 blockVelocity) {
    /*const auto blockAabb = Aabb(block.position, block.position + Vec2(constants().cellSize));
    const auto blockSize = blockAabb.size();*/

    if (!playerAabb.collides(blockAabb)) {
        return false;
    }

    auto distance = getDistance(playerAabb, blockAabb, player.velocity);

    const auto blockSize = blockAabb.size();
    const auto blockPosition = blockAabb.min;
    const auto playerSize = playerAabb.size();

    using namespace BlockCollisionDirections;
    const auto EPSILON = 0.01f;
    if (distance.left < distance.right) {
        if (collisionDirections & L && distance.left < distance.top && distance.left < distance.bottom) {
            movement.x = 0.0f;
            player.velocity.x = 0.0f;
            player.position.x = blockPosition.x - playerSize.x + constants().playerSize.x / 2.0f - EPSILON;
            player.touchingWallOnRight = true;
            player.blockThatIsBeingTouchedMovementDelta = blockVelocity;
            //blockVelocity.y = 0.0f;
        }
    } else {
        if (collisionDirections & R && distance.right < distance.top && distance.right < distance.bottom) {
            movement.x = 0.0f;
            player.velocity.x = 0.0f;
            player.position.x = blockPosition.x + blockSize.x + constants().playerSize.x / 2.0f + EPSILON;
            player.touchingWallOnLeft = true;
            player.blockThatIsBeingTouchedMovementDelta = blockVelocity;
            //blockVelocity.y = 0.0f;
        }
    }

    if (distance.bottom < distance.top) {
        if (collisionDirections & D && distance.bottom < distance.left && distance.bottom < distance.right) {
            movement.y = 0.0f;
            // Prevent the player from sticking to the ceiling of a moving block, because the velocity is being set to zero.
            if (player.velocity.y > 0.0f) {
                player.velocity.y = 0.0f;
            }
            player.position.y = blockPosition.y - playerSize.y + constants().playerSize.y / 2.0f - EPSILON;
            //blockVelocity.x = 0.0f;
        }
    } else {
        if (collisionDirections & U && distance.top < distance.left && distance.top < distance.right) {
            movement.y = 0.0f;
            player.velocity.y = 0.0f;
            player.position.y = blockPosition.y + blockSize.y + constants().playerSize.y / 2.0f + EPSILON;
            player.isGrounded = true;
            player.blockThatIsBeingTouchedMovementDelta = blockVelocity;
            //blockVelocity.x = 0.0f;
        }
    }

    player.blockThatIsBeingTouchedMovementDelta = blockVelocity;
    return true;
}

void collisionDetection(f32 dt, std::vector<GameRoom*> rooms, Player& player) {
    // For sliding to work with swept collision first the motion is done until some object is hit. After that the movement delta in the direction of collision is set to zero and the swept collision for the new movement delta is performed.
    Vec2 movementDelta = player.velocity;

    std::optional<i32> indexToIgnore;

    auto resolveCollisions = [&]() {
        const auto playerAabb = player.aabb();
        for (const auto& room : rooms) {
            for (const auto& block : room->blocks) {
                const auto blockAabb = Aabb(block.position, block.position + Vec2(constants().cellSize));
                blockCollision(player, movementDelta, playerAabb, blockAabb, block.collisionDirections, Vec2(0.0f));
            }
            for (auto& block : room->movingBlocks) {
                auto blockCopy = block;
                blockCopy.update(dt);
                //const auto blockAabb = Aabb(block.position(), block.position() + block.size);
                const auto blockAabb = Aabb(blockCopy.position(), blockCopy.position() + blockCopy.size);
                using namespace BlockCollisionDirections;
                const auto collisionHappened = blockCollision(player, movementDelta, playerAabb, blockAabb, L | R | U | D, blockCopy.positionDelta);
                if (collisionHappened) {
                    block.onPlayerCollision();
                }
            }
        }
    };

    static constexpr i32 AXIS_COUNT = 2;
    for (i32 _ = 0; _ < AXIS_COUNT; _++) {
        i32 index = 0;
        const auto playerAabb = player.aabb();
        const auto pAabb = AABB(playerAabb.center(), playerAabb.size() / 2.0f);

        std::optional<Hit> closestHit;
        Vec2 closestHitPositionDelta = Vec2(0.0f);
        i32 closestHitIndex = 0;

        auto checkHit = [&](const std::optional<Hit>& hit, Vec2 positionDelta = Vec2(0.0f)) -> void {
            if (!hit.has_value()) {
                return;
            }

            /*using namespace BlockCollisionDirections;
            if ((hit->normal.x == 1.0f && !(collisionDirections & R)) ||
                hit->normal.x == -1.0f && !(collisionDirections & L) ||
                hit->normal.y == 1.0f && !(collisionDirections & U) ||
                hit->normal.y == -1.0f && !(collisionDirections & D)) {
                return;
            }*/

            if (!closestHit.has_value() || hit->time < closestHit->time) {
                closestHit = hit;
                closestHitPositionDelta = positionDelta;
                closestHitIndex = index;
            }
        };

        for (const auto& room : rooms) {
            for (const auto& block : room->blocks) {
                index++;
                if (index == indexToIgnore) {
                    continue;
                }
                const auto blockAabb = Aabb(block.position, block.position + Vec2(constants().cellSize));

                auto bAabb = AABB(blockAabb.center(), blockAabb.size() / 2.0f);

                auto result = bAabb.sweepAABB(pAabb, movementDelta);
                checkHit(result.hit);
            }

            for (const auto& platform : room->platforms) {
                index++;
                if (index == indexToIgnore) {
                    continue;
                }
                const auto playerBottomY = player.position.y - constants().playerSize.y / 2.0f;
                const auto platformY = platform.position.y;

                if (player.velocity.y > 0.0f || playerBottomY < platformY) {
                    continue;
                }

                const auto platformAabb1 = Aabb(
                    Vec2(platform.position.x, platformY),
                    Vec2(platform.position.x + constants().cellSize, platformY + 1.0f));
                auto platformAabb = AABB(platformAabb1.center(), platformAabb1.size() / 2.0f);

                auto result = platformAabb.sweepAABB(pAabb, movementDelta);
                if (!result.hit.has_value() || result.hit->normal.x != 0.0f) {
                    continue;
                }
                checkHit(result.hit);
            }
       }

        if (!closestHit.has_value()) {
            player.position += movementDelta;
            break;
        }

        indexToIgnore = closestHitIndex;

        const auto epsilon = 0.01f;
        const auto movement = (std::clamp(closestHit->time - epsilon, 0.0f, 1.0f)) * movementDelta;
        player.position += movement;
        movementDelta -= movement;

        if (closestHit->time == 0.0f) {
            resolveCollisions();
            continue;
        }

        if (closestHit->normal.x != 0) {
            movementDelta.x = 0.0f;
            player.velocity.x = 0.0f;
            closestHitPositionDelta.y = 0.0f;
        } else if (closestHit->normal.y != 0) {
            movementDelta.y = 0.0f;
            player.velocity.y = 0.0f;
        }

        if (closestHit->normal.y == 1.0f) {
            player.isGrounded = true;
        }
        if (closestHit->normal.x == 1.0f) {
            player.touchingWallOnLeft = true;
        }
        if (closestHit->normal.x == -1.0f) {
            player.touchingWallOnRight = true;
        }

    }

    {
        const auto playerAabb = ::playerAabb(player.position);
        for (const auto& room : rooms) {
            for (auto& block : room->movingBlocks) {
                auto blockCopy = block;
                blockCopy.update(dt);
                //const auto blockAabb = Aabb(block.position(), block.position() + block.size);
                const auto blockAabb = Aabb(blockCopy.position(), blockCopy.position() + blockCopy.size);
                using namespace BlockCollisionDirections;
                const auto collisionHappened = blockCollision(player, movementDelta, playerAabb, blockAabb, L | R | U | D, blockCopy.positionDelta);
                if (collisionHappened) {
                    block.onPlayerCollision();
                }
            }
        }
        
    }

    if (player.blockThatIsBeingTouchedMovementDelta.has_value()) {
        player.position += *player.blockThatIsBeingTouchedMovementDelta;
    }
}
