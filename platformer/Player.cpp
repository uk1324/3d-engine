#include <platformer/Player.hpp>
#include <platformer/Constants.hpp>
#include <engine/Input/Input.hpp>
#include <RefOptional.hpp>
#include <engine/Math/Circle.hpp>
#include <engine/Math/Random.hpp>
#include <engine/Math/Aabb.hpp>
#include <platformer/Collision.hpp>
#include <imgui/imgui.h>
#include <platformer/Assets.hpp>

#include <framework/Dbg.hpp>

void Player::updateVelocity(
    const GameInput& input,
    f32 dt, 
    std::vector<DoubleJumpOrb>& doubleJumpOrbs, 
    std::vector<AttractingOrb>& attractingOrbs,
    GameAudio& audio) {

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

    if (input.left) {
        velocity.x -= speed;
    }
    if (input.right) {
        velocity.x += speed;
    }

    elapsedSinceLastGrounded += dt;
    if (isGrounded) {
        elapsedSinceLastGrounded = 0.0f;
    }
    
    elapsedSinceTouchedWallOnLeft += dt;
    if (touchingWallOnLeft) {
        elapsedSinceTouchedWallOnLeft = 0.0f;
    }

    elapsedSinceTouchedWallOnRight += dt;
    if (touchingWallOnRight) {
        elapsedSinceTouchedWallOnRight = 0.0f;
    }

    elapsedSinceJumpPressed += dt;
    if (input.jump && jumpReleased) {
        elapsedSinceJumpPressed = 0.0f;
    }

    if (!input.jump) {
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

    bool used = false;
    if (input.use) {
        used = true;
        for (auto& orb : attractingOrbs) {
            //Dbg::drawCircle(orb.position, constants().cellSize * 2.0f, Vec3(1.0f, 0.0f, 0.0f));
            const auto fromPlayerToOrb = orb.position - position;
            const auto distance = fromPlayerToOrb.length();
            const auto direction = fromPlayerToOrb / distance;
            auto scale = [](f32 t) -> f32 {
                // move the maximum to 1.0f
                static const f32 maximumT = sqrt(0.5f);
                t *= maximumT;

                // move the maximum to 3 * cellSize
                t /= constants().cellSize * 3.0f;

                f32 value = t * exp(-t * t);

                // make the maximum value 1.0f
                const auto maximumValue = sqrt(1 / (2.0f * exp(1.0f)));
                value /= maximumValue;

                return value;
            };
            /*velocity += direction * distance * exp(-distance * distance / 4000.0f) / 20.0f;*/
            const auto acceleration = scale(distance);
            /*ImGui::Text("accleration: %g", acceleration);
            ImGui::Text("distance: %g", distance);
            ImGui::Text("velocity: %g, %g", velocity.x, velocity.y);*/
            velocity += direction * acceleration;
            velocity *= 0.99f;
            /*if (isnan(position.x) || isnan(position.y)) {
                int x = 5;
            }*/
        }
    }
    // Istead of having coyote time could make it so you have to be within a certain distance from a wall to wall jump. This is done in celeste.
    const auto wallJumpCoyoteTime = 0.05f;
    if (elapsedSinceJumpPressed < jumpPressedRememberTime && jumpReleased) {
        bool jumped = false;

        bool playJumpSound = false;

        if (velocity.y <= 0 && elapsedSinceLastGrounded < coyoteTime) {
            jumped = true;
            jumpedOffGround = true;
            velocity.y = jumpSpeed;
            playJumpSound = true;
        } else if (touchedDoubleJumpOrb.has_value()) {
            touchedDoubleJumpOrb->onUse(audio);
            jumped = true;
            jumpedOffGround = false;
            // The orb gives a bigger jump, but doesn't allow holding space to extend it, because it the holding with the orb felt like flying.
            velocity.y = jumpSpeed * 1.3f;
        } else if (!isGrounded && elapsedSinceTouchedWallOnLeft < wallJumpCoyoteTime) {
            jumped = true;
            velocity = Vec2(jumpSpeed * 1.55f, jumpSpeed * 1.3f);
            jumpedOffGround = false;
            playJumpSound = true;
        } else if (!isGrounded && elapsedSinceTouchedWallOnRight < wallJumpCoyoteTime) {
            velocity = Vec2(-jumpSpeed * 1.55f, jumpSpeed * 1.3f);
            jumped = true;
            jumpedOffGround = false;
            playJumpSound = true;
        }

        if (playJumpSound) {
            audio.playSoundEffect(assets->jumpSound, randomFromRange(0.4, 1.4f));
        }

        if (jumped) {
            jumpReleased = false;
            isGrounded = false;
            blockThatIsBeingTouchedMovementDelta = std::nullopt;
            elapsedSinceJumpPressed = 0.0f;
            elapsedSinceLastJumped = 0.0f;
        }
    }
    elapsedSinceLastJumped += dt;

    if (input.jump && jumpedOffGround && elapsedSinceLastJumped < 0.07f && velocity.y > 0.0f && !used) {
        velocity.y += 0.5f;
    }

    const auto gravity = 0.3f;

    const auto holdingOntoLeftWall = touchingWallOnLeft && input.left;
    const auto holdingOntoRightWall = touchingWallOnRight && input.right;

    /*if (!isGrounded)*/ {
        const auto jumpGravityMultiplier = 0.8f;
        const auto fallGravityMultiplier = 1.5f;

        const auto jumping = (input.jump && velocity.y >= 0) && !holdingOntoLeftWall && !holdingOntoRightWall;

        const auto gravityMultiplier = jumping
            ? jumpGravityMultiplier
            : fallGravityMultiplier;

        velocity.y -= gravity * gravityMultiplier;
    }

    if (!(input.jump && velocity.y >= 0) && (holdingOntoLeftWall || holdingOntoRightWall)) {
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

Aabb Player::aabb() const {
    return playerAabb(position);
}
