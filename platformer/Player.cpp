#include <platformer/Player.hpp>
#include <platformer/Constants.hpp>
#include <engine/Input/Input.hpp>
#include <RefOptional.hpp>
#include <engine/Math/Circle.hpp>
#include <engine/Math/Aabb.hpp>
#include <platformer/Collision.hpp>
#include <imgui/imgui.h>


//bool Player::isGrounded() const {
//    return blockThatIsBeingStoodOnVelocity.has_value();
//}

void Player::updateMovement(f32 dt, std::vector<DoubleJumpOrb>& doubleJumpOrbs) {
    const bool left = Input::isKeyHeld(KeyCode::A);
    const bool right = Input::isKeyHeld(KeyCode::D);
    const bool jump = Input::isKeyHeld(KeyCode::SPACE);

    //ImGui::Checkbox("isGrounded", &isGrounded);
    ImGui::InputFloat2("velocity", velocity.data());
    ImGui::Text("is grounded: %s", isGrounded ? "true" : "false");
    ImGui::Text("touching wall on left: %s", touchingWallOnLeft ? "true" : "false");
    ImGui::Text("touching wall on right: %s", touchingWallOnRight ? "true" : "false");
    /*ImGui::Text("touched block position delta: %s");*/
    if (blockThatIsBeingTouchedMovementDelta.has_value()) {
        ImGui::InputFloat2("touched block position delta", blockThatIsBeingTouchedMovementDelta->data());
    } else {
        ImGui::Text("no block touched");
    }

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

    //this.grounded = false
    //grounded = false;

    /*if (this.velY > 10) {
        this.velY = 10
    }*/
    velocity.y = std::max(-10.0f, velocity.y);
    //if (isGrounded) {
    //    velocity.y -= 1.0f;
    //}
    /*if (blockThatIsBeingTouchedMovementDelta.has_value() && isGrounded && blockThatIsBeingTouchedMovementDelta->y) {
        velocity.y = blockThatIsBeingTouchedMovementDelta->y;
    }*/


    //velocity.x *= this.frictionX


    if (!dead) {
        //position += velocity;
        /*if (blockThatIsBeingStoodOnVelocity.has_value()) {
            position += *blockThatIsBeingStoodOnVelocity;
        }*/
    }

    //blockThatIsBeingStoodOnVelocity = std::nullopt;
    isGrounded = false;
    blockThatIsBeingTouchedMovementDelta = std::nullopt;
    touchingWallOnLeft = false;
    touchingWallOnRight = false;
}
#include <framework/Dbg.hpp>

void Player::collision(
    f32 dt,
    const std::vector<Block>& blocks, 
    const std::vector<Platform>& platforms,
    const std::vector<MovingBlock>& movingBlocks) {
    static constexpr i32 AXIS_COUNT = 2;
    // For sliding to work with swept collision first the motion is done until some object is hit. After that the movement delta in the direction of collision is set to zero and the swept collision for the new movement delta is performed.
    Vec2 movementDelta = velocity;

    //if (blockThatIsBeingStoodOnVelocity.has_value()) {
    //    position += *blockThatIsBeingStoodOnVelocity;
    //}
    if (Input::isKeyDown(KeyCode::K)) {
        int x = 5;
    }

    //const auto playerAabb = ::playerAabb(position);
    //for (const auto& block : blocks) {
    //    const auto blockAabb = Aabb(block.position, block.position + Vec2(constants().cellSize));;
    //    blockCollision(playerAabb, blockAabb, block.collisionDirections, Vec2(0.0f));
    //}
    //for (const auto& block : movingBlocks) {
    //    const auto blockAabb = Aabb(block.position(), block.position() + block.size);
    //    using namespace BlockCollisionDirections;
    //    auto blockCopy = block;
    //    //blockCopy.update(dt);
    //    blockCollision(playerAabb, blockAabb, L | R | U | D, blockCopy.positionDelta);
    //}
    std::optional<i32> indexToIgnore;

    for (i32 _ = 0; _ < AXIS_COUNT; _++) {
        i32 index = 0;
        const auto playerAabb = ::playerAabb(position);
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

        for (const auto& block : blocks) {
            index++;
            if (index == indexToIgnore) {
                continue;
            }
            const auto blockAabb = Aabb(block.position, block.position + Vec2(constants().cellSize));

            auto bAabb = AABB(blockAabb.center(), blockAabb.size() / 2.0f);

            auto result = bAabb.sweepAABB(pAabb, movementDelta);
            checkHit(result.hit);
        }

        for (const auto& platform : platforms) {
            index++;
            if (index == indexToIgnore) {
                continue;
            }
            const auto playerBottomY = position.y - constants().playerSize.y / 2.0f;
            const auto platformY = platform.position.y;
            //Dbg::drawAabb(platformAabb1, Vec3(1.0f), 2.0f); 

            if (velocity.y > 0.0f || playerBottomY < platformY) {
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

        //for (const auto& block : movingBlocks) {
        //    index++;
        //    if (index == indexToIgnore) {
        //        continue;
        //    }

        //    const auto blockAabb = Aabb(block.position(), block.position() + block.size);
        //    auto bAabb = AABB(blockAabb.center(), blockAabb.size() / 2.0f);

        //    auto blockCopy = block;
        //    blockCopy.update(dt);

        //    auto result = bAabb.sweepAABB(pAabb, movementDelta - blockCopy.positionDelta);
        //    checkHit(result.hit, blockCopy.positionDelta);
        //}

        if (!closestHit.has_value()) {
            position += movementDelta;
            break;
        }
        indexToIgnore = closestHitIndex;

        //if (closestHit->time == 0.0f) {
        //    break;
        //}

        //ImGui::InputFloat2("normal", closestHit->normal.data());

        const auto epsilon = 0.01f;
        //const auto epsilon = 0.0f;
        const auto movement = (std::clamp(closestHit->time - epsilon, 0.0f, 1.0f)) * movementDelta;
        position += movement;
        movementDelta -= movement;

        if (closestHit->time == 0.0f) {
            //position += closestHit->normal * 0.03f;
            //break;
            const auto playerAabb = ::playerAabb(position);
            for (const auto& block : blocks) {
                const auto blockAabb = Aabb(block.position, block.position + Vec2(constants().cellSize));;
                blockCollision(movementDelta, playerAabb, blockAabb, block.collisionDirections, Vec2(0.0f));
            }
            for (const auto& block : movingBlocks) {
                auto blockCopy = block;
                blockCopy.update(dt);
                //const auto blockAabb = Aabb(block.position(), block.position() + block.size);
                const auto blockAabb = Aabb(blockCopy.position(), blockCopy.position() + blockCopy.size);
                using namespace BlockCollisionDirections;
                blockCollision(movementDelta, playerAabb, blockAabb, L | R | U | D, blockCopy.positionDelta);
            }
            ImGui::Text("already colliding");
            continue;
        }

        if (closestHit->normal.x != 0) {
            movementDelta.x = 0.0f;
            velocity.x = 0.0f;
            //velocity.x = closestHitPositionDelta.x;
            closestHitPositionDelta.y = 0.0f;
        } else if (closestHit->normal.y != 0) {
            movementDelta.y = 0.0f;
            //velocity.y = closestHitPositionDelta.y;
            velocity.y = 0.0f;
            //closestHitPositionDelta.x = 0.0f;
        }

        if (closestHit->normal.y == 1.0f) {
            isGrounded = true;
        }
        if (closestHit->normal.x == 1.0f) {
            touchingWallOnLeft = true;
        }
        if (closestHit->normal.x == -1.0f) {
            touchingWallOnRight = true;
        }

        //if (closestHit->normal.x != 0) {
        //    movementDelta.x = 0.0f;
        //    velocity.x = 0.0f;
        //    //velocity.x = closestHitPositionDelta.x;
        //} else if (closestHit->normal.y != 0) {
        //    movementDelta.y = 0.0f;
        //    velocity.y = 0.0f;
        //    if (closestHitPositionDelta != Vec2(0.0f)) {
        //        int x = 5;
        //    }
        //    //velocity.y = closestHitPositionDelta.y;
        //}
        ////if (closestHit->time == 0.0f) {
        ////    position += closestHit->normal * 0.01f;
        ////}
        //if (closestHit->normal.y == 1.0f) {
        //    //blockThatIsBeingStoodOnVelocity = closestHitPositionDelta;
        //    isGrounded = true;
        //}
    }

    {
        const auto playerAabb = ::playerAabb(position);
        /*for (const auto& block : blocks) {
            const auto blockAabb = Aabb(block.position, block.position + Vec2(constants().cellSize));;
            blockCollision(movementDelta, playerAabb, blockAabb, block.collisionDirections, Vec2(0.0f));
        }*/
        for (const auto& block : movingBlocks) {
            auto blockCopy = block;
            blockCopy.update(dt);
            //const auto blockAabb = Aabb(block.position(), block.position() + block.size);
            const auto blockAabb = Aabb(blockCopy.position(), blockCopy.position() + blockCopy.size);
            using namespace BlockCollisionDirections;
            blockCollision(movementDelta, playerAabb, blockAabb, L | R | U | D, blockCopy.positionDelta);
        }
    }

    if (blockThatIsBeingTouchedMovementDelta.has_value()) {
        position += *blockThatIsBeingTouchedMovementDelta;
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
            //blockThatIsBeingStoodOnVelocity = block.positionDelta;
            //blockThatIsBeingTouchedMovementDelta = 
        }
    }
}

void Player::blockCollision(
    Vec2& movement,
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
    const auto EPSILON = 0.01f;
    if (distance.left < distance.right) {
        if (collisionDirections & L && distance.left < distance.top && distance.left < distance.bottom) {
            movement.x = 0.0f;
            velocity.x = 0.0f;
            position.x = blockPosition.x - playerSize.x + constants().playerSize.x / 2.0f - EPSILON;
            touchingWallOnRight = true;
            blockThatIsBeingTouchedMovementDelta = blockVelocity;
            //blockVelocity.y = 0.0f;
        }
    } else {
        if (collisionDirections & R && distance.right < distance.top && distance.right < distance.bottom) {
            movement.x = 0.0f;
            velocity.x = 0.0f;
            position.x = blockPosition.x + blockSize.x + constants().playerSize.x / 2.0f + EPSILON;
            touchingWallOnLeft = true;
            blockThatIsBeingTouchedMovementDelta = blockVelocity;
            //blockVelocity.y = 0.0f;
        }
    }
    
    if (distance.bottom < distance.top) {
        if (collisionDirections & D && distance.bottom < distance.left && distance.bottom < distance.right) {
            movement.y = 0.0f;
            // Prevent the player from sticking to the ceiling of a moving block, because the velocity is being set to zero.
            if (velocity.y > 0.0f) {
                velocity.y = 0.0f;
            }
            position.y = blockPosition.y - playerSize.y + constants().playerSize.y / 2.0f - EPSILON;
            //blockVelocity.x = 0.0f;
        }
    } else {
        if (collisionDirections & U && distance.top < distance.left && distance.top < distance.right) {
            movement.y = 0.0f;
            velocity.y = 0.0f;
            position.y = blockPosition.y + blockSize.y + constants().playerSize.y / 2.0f + EPSILON;
            isGrounded = true;
            blockThatIsBeingTouchedMovementDelta = blockVelocity;
            //blockVelocity.x = 0.0f;
        }
    }
  
   
    blockThatIsBeingTouchedMovementDelta = blockVelocity;
}
