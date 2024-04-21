#include <platformer/Player.hpp>
#include <platformer/Constants.hpp>
#include <engine/Input/Input.hpp>
#include <RefOptional.hpp>
#include <engine/Math/Circle.hpp>
#include <engine/Math/Aabb.hpp>
#include <platformer/Collision.hpp>
#include <imgui/imgui.h>


bool Player::isGrounded() const {
    return blockThatIsBeingStoodOnVelocity.has_value();
}

void Player::updateMovement(f32 dt, std::vector<DoubleJumpOrb>& doubleJumpOrbs) {
    const bool left = Input::isKeyHeld(KeyCode::A);
    const bool right = Input::isKeyHeld(KeyCode::D);
    const bool jump = Input::isKeyHeld(KeyCode::SPACE);

    f32 speed = 1.0f / 1.5f;
    //f32 speed = 40.0f / 1.5f;

    if (!isGrounded()) {
        speed *= 0.40f;
    }

    if (left) {
        velocity.x -= speed;
    }
    if (right) {
        velocity.x += speed;
    }

    elapsedSinceLastGrounded += dt;
    if (isGrounded()) {
        elapsedSinceLastGrounded = 0.0f;
    }
    /*if (this.grounded) {
        this.lastGrounded = Date.now()
    }*/
    elapsedSinceJumpPressed += dt;
    if (jump && jumpReleased) {
        elapsedSinceJumpPressed = 0.0f;
    }
    /*if (this.controller.jump && this.jumpReleased) {
        this.jumpLastPressed = Date.now()
    }*/

    if (!jump) {
        jumpReleased = true;
    }

    /*if (!this.controller.jump) {
        this.jumpReleased = true
    }*/

    const auto jumpPressedRememberTime = 0.1f;
    const auto coyoteTime = 0.1f;
    const auto jumpSpeed = 4.1f;

    std::optional<DoubleJumpOrb&> touchedDoubleJumpOrb;
    for (auto& orb : doubleJumpOrbs) {
        if (!orb.isActive()) {
            continue;
        }

        if (circleAabbCollision(orb.position, constants().doubleJumpOrbRadius, playerAabb(position))) {
            touchedDoubleJumpOrb = orb;
            break;
        }
    }

    if (elapsedSinceJumpPressed < jumpPressedRememberTime && jumpReleased) {
        bool jumped = false;

        if (velocity.y <= 0 && elapsedSinceLastGrounded < coyoteTime) {
            jumped = true;
            jumpedOffGround = true;
            velocity.y = jumpSpeed;
        } else if (touchedDoubleJumpOrb.has_value()) {
            touchedDoubleJumpOrb->elapsedSinceUsed = 0.0f;
            jumped = true;
            jumpedOffGround = false;
            // The orb gives a bigger jump, but doesn't allow holding space to extend it, because it the holding with the orb felt like flying.
            velocity.y = jumpSpeed * 1.3f;
        } else if (!isGrounded() && touchingWallOnLeft && elapsedSinceLastJumped) {
            jumped = true;
            velocity = Vec2(jumpSpeed * 1.55f, jumpSpeed * 1.3f);
            jumpedOffGround = false;
        } else if (!isGrounded() && touchingWallOnRight) {
            velocity = Vec2(-jumpSpeed * 1.55f, jumpSpeed * 1.3f);
            jumped = true;
            jumpedOffGround = false;
        }

        if (jumped) {
            jumpReleased = false;
            blockThatIsBeingStoodOnVelocity = std::nullopt;
            elapsedSinceJumpPressed = 0.0f;
            elapsedSinceLastJumped = 0.0f;
        }
    }
    elapsedSinceLastJumped += dt;

    if (jump && jumpedOffGround && elapsedSinceLastJumped < 0.07f && velocity.y > 0.0f) {
        velocity.y += 0.5f;
    }

    const auto gravity = 0.3f;

    const auto holdingOntoLeftWall = touchingWallOnLeft && left;
    const auto holdingOntoRightWall = touchingWallOnRight && right;

    if (!isGrounded()) {
        const auto jumpGravityMultiplier = 0.8f;
        const auto fallGravityMultiplier = 1.5f;

        const auto jumping = (jump && velocity.y >= 0) && !holdingOntoLeftWall && !holdingOntoRightWall;

        const auto gravityMultiplier = jumping
            ? jumpGravityMultiplier
            : fallGravityMultiplier;

        velocity.y -= gravity * gravityMultiplier;
    }

    if (!(jump && velocity.y >= 0) && (holdingOntoLeftWall || holdingOntoRightWall)) {
        velocity.y *= 0.80f;
    }

    const auto frictionX = 0.85;
    if (isGrounded()) {
        velocity.x *= frictionX;
    } else {
        velocity.x *= 0.94f;
    }

    //this.grounded = false
    //grounded = false;

    /*if (this.velY > 10) {
        this.velY = 10
    }*/
    velocity.y = std::max(-10.0f, velocity.y);


    //velocity.x *= this.frictionX


    if (!dead) {
        //position += velocity;
        if (blockThatIsBeingStoodOnVelocity.has_value()) {
            position += *blockThatIsBeingStoodOnVelocity;
        }
    }

    blockThatIsBeingStoodOnVelocity = std::nullopt;
    touchingWallOnLeft = false;
    touchingWallOnRight = false;
}

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

#include <framework/Dbg.hpp>
void Player::blockCollision(const std::vector<Block>& blocks) {
    auto vToP = [](Vec2 v) {
        return Point(v.x, v.y);
    };

    auto pToV = [](Point v) {
        return Vec2(v.x, v.y);
    };

    //Dbg::drawAabb(playerAabb, Vec3(1.0f, 0.0f, 0.0f), 2.0f);
    Vec2 delta = velocity;
    for (i32 i = 0; i < 2; i++) {
        const auto playerAabb = ::playerAabb(position);
        std::optional<Hit> closestHit;
        for (const auto& block : blocks) {
            const auto blockAabb = Aabb(block.position, block.position + Vec2(constants().cellSize));

            auto pAabb = AABB(vToP(playerAabb.center()), vToP(playerAabb.size() / 2.0f));
            auto bAabb = AABB(vToP(blockAabb.center()), vToP(blockAabb.size() / 2.0f));

            /*auto result = bAabb.sweepAABB(pAabb, vToP(velocity));*/
            auto result = bAabb.sweepAABB(pAabb, vToP(velocity));
            //const auto result = aabbVsSweeptAabbCollision(pAabb, bAabb, vToP(delta));
// 
            //const auto result = aabbVsSweeptAabbCollision(blockAabb, playerAabb, delta);
            if (!result.hit.has_value()) {
                continue;
            }
            Dbg::drawAabb(blockAabb, Vec3(1.0f), 2.0f);
            //Dbg::drawDisk(result.hit->pos, 4.0f, Vec3(1.0f));
            //Dbg::drawDisk(result.pos, 4.0f, Vec3(1.0f, 0.0f, 0.0f));
            //Dbg::drawDisk(position + result.hit->delta, 4.0f, Vec3(1.0f, 0.0f, 0.0f));
            if (!closestHit.has_value() || result.hit->time < closestHit->time) {
                closestHit = result.hit;
            }
        }

        if (!closestHit.has_value()) {
            position += delta;
            return;
        }

        if (delta.x != 0.0f) {
            int x = 5;
        }

        const auto epsilon = 1e-3f;
        const auto movement = (std::clamp(closestHit->time - epsilon, 0.0f, 1.0f)) * delta;
        position += movement;
        delta -= movement;
        if (closestHit->normal.x != 0) {
            delta.x = 0.0f;
            velocity.x = 0.0f;
        } else if (closestHit->normal.y != 0) {
            delta.y = 0.0f;
            velocity.y = 0.0f;
        }
        if (closestHit->time == 0.0f) {
            //position = pToV(closestHit->pos);
            position += pToV(closestHit->normal) * 0.01f;
            int x = 5;
        }

        //position += pToV(closestHit->delta);

        if (closestHit->normal.y == 1.0f) {
            blockThatIsBeingStoodOnVelocity = Vec2(0.0f);
        }
    }
}

void Player::movingBlockCollision(const std::vector<MovingBlock>& movingBlocks) {
    const auto playerAabb = ::playerAabb(position);
    for (const auto& block : movingBlocks) {
        const auto position = block.position();
        const auto blockAabb = Aabb(position, position + block.size);
        using namespace BlockCollisionDirections;
        blockCollision(playerAabb, blockAabb, L | R | U | D, block.positionDelta);
    }
}

void Player::checkIfPlayerIsStandingOnMovingBlocks(const std::vector<MovingBlock>& movingBlocks) {
    const auto playerAabb = ::playerAabb(position);
    for (const auto& block : movingBlocks) {
        const auto position = block.position();
        const auto blockAabb = Aabb(position, position + block.size);

        if (!playerAabb.collides(blockAabb)) {
            continue;
        }

        auto distance = getDistance(playerAabb, blockAabb, velocity);
        if (distance.top < distance.left && distance.top < distance.right) {
            blockThatIsBeingStoodOnVelocity = block.positionDelta;
        }
    }
}

void Player::blockCollision(
    const Aabb& playerAabb,
    const Aabb& blockAabb,
    BlockCollsionDirectionsBitfield collisionDirections,
    Vec2 blockVelocity) {
    /*const auto blockAabb = Aabb(block.position, block.position + Vec2(constants().cellSize));
    const auto blockSize = blockAabb.size();*/

    if (!playerAabb.collides(blockAabb)) {
        return;
    }

    auto distance = getDistance(playerAabb, blockAabb, velocity);

    const auto blockSize = blockAabb.size();
    const auto blockPosition = blockAabb.min;
    const auto playerSize = playerAabb.size();

    using namespace BlockCollisionDirections;
    if (collisionDirections & L && distance.left < distance.top && distance.left < distance.bottom) {
        velocity.x = 0.0f;
        position.x = blockPosition.x - playerSize.x + constants().playerSize.x / 2.0f;
        touchingWallOnRight = true;
    }
    if (collisionDirections & R && distance.right < distance.top && distance.right < distance.bottom) {
        velocity.x = 0.0f;
        position.x = blockPosition.x + blockSize.x + constants().playerSize.x / 2.0f;
        touchingWallOnLeft = true;
    }
    if (collisionDirections & D && distance.bottom < distance.left && distance.bottom < distance.right) {
        velocity.y = 0.0f;
        position.y = blockPosition.y - playerSize.y + constants().playerSize.y / 2.0f;
    }
    if (collisionDirections & U && distance.top < distance.left && distance.top < distance.right) {
        velocity.y = 0.0f;
        position.y = blockPosition.y + blockSize.y + constants().playerSize.y / 2.0f;
        blockThatIsBeingStoodOnVelocity = blockVelocity;
        //grounded = true;
    }
}
