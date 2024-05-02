#include <platformer/Player.hpp>
#include <platformer/Constants.hpp>
#include <engine/Input/Input.hpp>
#include <RefOptional.hpp>
#include <engine/Math/Circle.hpp>
#include <engine/Math/Aabb.hpp>
#include <platformer/Collision.hpp>
#include <imgui/imgui.h>

void Player::updateVelocity(f32 dt, std::vector<DoubleJumpOrb>& doubleJumpOrbs) {
    const bool left = Input::isKeyHeld(KeyCode::A);
    const bool right = Input::isKeyHeld(KeyCode::D);
    const bool jump = Input::isKeyHeld(KeyCode::SPACE);

    //ImGui::Checkbox("isGrounded", &isGrounded);
    //ImGui::InputFloat2("velocity", velocity.data());
    //ImGui::Text("is grounded: %s", isGrounded ? "true" : "false");
    //ImGui::Text("touching wall on left: %s", touchingWallOnLeft ? "true" : "false");
    //ImGui::Text("touching wall on right: %s", touchingWallOnRight ? "true" : "false");
    ///*ImGui::Text("touched block position delta: %s");*/
    //if (blockThatIsBeingTouchedMovementDelta.has_value()) {
    //    ImGui::InputFloat2("touched block position delta", blockThatIsBeingTouchedMovementDelta->data());
    //} else {
    //    ImGui::Text("no block touched");
    //}

    f32 speed = 1.0f / 1.5f;
    //f32 speed = 40.0f / 1.5f;

    if (!isGrounded) {
        speed *= 0.40f;
    }

    if (left) {
        velocity.x -= speed;
    }
    if (right) {
        velocity.x += speed;
    }

    elapsedSinceLastGrounded += dt;
    if (isGrounded) {
        elapsedSinceLastGrounded = 0.0f;
    }

    elapsedSinceJumpPressed += dt;
    if (jump && jumpReleased) {
        elapsedSinceJumpPressed = 0.0f;
    }

    if (!jump) {
        jumpReleased = true;
    }

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
        } else if (!isGrounded && touchingWallOnLeft && elapsedSinceLastJumped) {
            jumped = true;
            velocity = Vec2(jumpSpeed * 1.55f, jumpSpeed * 1.3f);
            jumpedOffGround = false;
        } else if (!isGrounded && touchingWallOnRight) {
            velocity = Vec2(-jumpSpeed * 1.55f, jumpSpeed * 1.3f);
            jumped = true;
            jumpedOffGround = false;
        }

        if (jumped) {
            jumpReleased = false;
            //blockThatIsBeingStoodOnVelocity = std::nullopt;
            isGrounded = false;
            blockThatIsBeingTouchedMovementDelta = std::nullopt;
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

    /*if (!isGrounded)*/ {
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
    if (isGrounded) {
        velocity.x *= frictionX;
    } else {
        velocity.x *= 0.94f;
    }

    velocity.y = std::max(-10.0f, velocity.y);

    isGrounded = false;
    blockThatIsBeingTouchedMovementDelta = std::nullopt;
    touchingWallOnLeft = false;
    touchingWallOnRight = false;
}
#include <framework/Dbg.hpp>

//void Player::collision(
//    f32 dt,
//    const std::vector<Block>& blocks, 
//    const std::vector<Platform>& platforms,
//    const std::vector<MovingBlock>& movingBlocks) {
//    
//}

//void Player::checkIfPlayerIsStandingOnMovingBlocks(const std::vector<MovingBlock>& movingBlocks) {
//    const auto playerAabb = ::playerAabb(position);
//    for (const auto& block : movingBlocks) {
//        const auto position = block.position();
//        const auto blockAabb = Aabb(position, position + block.size);
//
//        if (!playerAabb.collides(blockAabb)) {
//            continue;
//        }
//
//        auto distance = getDistance(playerAabb, blockAabb, velocity);
//        if (distance.top < distance.left && distance.top < distance.right) {
//            //blockThatIsBeingStoodOnVelocity = block.positionDelta;
//            //blockThatIsBeingTouchedMovementDelta = 
//        }
//    }
//}

Aabb Player::aabb() const {
    return playerAabb(position);
}
