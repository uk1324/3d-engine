#include <platformer/Player.hpp>
#include <platformer/Constants.hpp>
#include <engine/Input/Input.hpp>
#include <RefOptional.hpp>
#include <engine/Math/Circle.hpp>
#include <engine/Math/Aabb.hpp>

#include <imgui/imgui.h>

void Player::updateMovement(f32 dt, std::vector<DoubleJumpOrb>& doubleJumpOrbs) {
    const bool left = Input::isKeyHeld(KeyCode::A);
    const bool right = Input::isKeyHeld(KeyCode::D);
    const bool jump = Input::isKeyHeld(KeyCode::SPACE);

    f32 speed = 1.0f / 1.5f;

    if (!grounded) {
        speed *= 0.40f;
    }

    if (left) {
        velocity.x -= speed;
    }
    if (right) {
        velocity.x += speed;
    }

    elapsedSinceLastGrounded += dt;
    if (grounded) {
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
        } else if (!grounded && touchingWallOnLeft && elapsedSinceLastJumped) {
            jumped = true;
            velocity = Vec2(jumpSpeed * 1.55f, jumpSpeed * 1.3f);
            jumpedOffGround = false;
        } else if (!grounded && touchingWallOnRight) {
            velocity = Vec2(-jumpSpeed * 1.55f, jumpSpeed * 1.3f);
            jumped = true;
            jumpedOffGround = false;
        }

        if (jumped) {
            jumpReleased = false;
            grounded = false;
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

    if (!grounded) {
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
    if (grounded) {
        velocity.x *= frictionX;
    } else {
        velocity.x *= 0.94f;
    }

    //this.grounded = false
    grounded = false;
    touchingWallOnLeft = false;
    touchingWallOnRight = false;

    /*if (this.velY > 10) {
        this.velY = 10
    }*/
    velocity.y = std::max(-10.0f, velocity.y);


    //velocity.x *= this.frictionX


    if (!dead) {
        position += velocity;
    }

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


void Player::blockCollision(const std::vector<Block>& blocks) {
    const auto playerAabb = ::playerAabb(position);
    const auto playerSize = playerAabb.size();
    for (const auto& block : blocks) {
        const auto blockAabb = Aabb(block.position, block.position + Vec2(constants().cellSize));
        const auto blockSize = blockAabb.size();
        
        if (!playerAabb.collides(blockAabb)) {
            continue;
        }

        auto distance = getDistance(playerAabb, blockAabb, velocity);
        
        using namespace BlockCollisionDirections;
        if (block.collisionDirections & L && distance.left < distance.top && distance.left < distance.bottom) {
            velocity.x = 0.0f;
            position.x = block.position.x - playerSize.x + constants().playerSize.x / 2.0f;
            touchingWallOnRight = true;
        }
        if (block.collisionDirections & R && distance.right < distance.top && distance.right < distance.bottom) {
            velocity.x = 0.0f;
            position.x = block.position.x + blockSize.x + constants().playerSize.x / 2.0f;
            touchingWallOnLeft = true;
        }
        if (block.collisionDirections & D && distance.bottom < distance.left && distance.bottom < distance.right) {
            velocity.y = 0.0f;
            position.y = block.position.y - playerSize.y + constants().playerSize.y / 2.0f;
        }
        if (block.collisionDirections & U && distance.top < distance.left && distance.top < distance.right) {
            velocity.y = 0.0f;
            position.y = block.position.y + blockSize.y + constants().playerSize.y / 2.0f;
            grounded = true;
        }

    }
}
