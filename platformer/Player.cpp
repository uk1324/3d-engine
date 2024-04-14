#include <platformer/Player.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Aabb.hpp>

Aabb Player::aabb(const PlayerSettings& settings) {
    return Aabb(position - settings.size / 2.0f, position + settings.size / 2.0f);
}

void Player::updateMovement(f32 dt) {
    const bool left = Input::isKeyHeld(KeyCode::A);
    const bool right = Input::isKeyHeld(KeyCode::D);
    const bool jump = Input::isKeyHeld(KeyCode::SPACE);

    if (jump) {
        float test = 5.0f;
    }

    f32 speed = 1.0f / 1.5f;

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
    const auto jumpSpeed = 3.9f;

    if (elapsedSinceJumpPressed < jumpPressedRememberTime 
        && velocity.y <= 0 && jumpReleased 
        && elapsedSinceLastGrounded < coyoteTime) {

        velocity.y = jumpSpeed;
        jumpReleased = false;
        grounded = false;
        elapsedSinceJumpPressed = 0.0f;
        elapsedSinceLastJumped = 0.0f;
    }
    elapsedSinceLastJumped += dt;

    /*if (Date.now() - this.jumpLastPressed < this.jumpPressedRememberTime && this.velY >= 0 && this.jumpReleased && Date.now() - this.lastGrounded < this.coyoteTime) {
        this.velY = -this.jumpSpeed
            this.jumpReleased = false
            this.grounded = false
            this.jumpLastPressed = null
            this.lastJumped = Date.now()
    }*/

    /*if (this.controller.jump && Date.now() - this.lastJumped < 100 && this.velY < 0) {
        this.velY -= (50 / 100)
    }*/
    // Allow jumping by holding jump when already in air.
    if (jump && elapsedSinceLastJumped < 0.1f && velocity.y > 0.0f) {
        velocity.y += 0.5f;
    }

    /*if (!this.grounded) {
        let gravityMultiplier = (this.controller.jump && this.velY <= 0) ? this.jumpGravityMultiplier : this.fallGravityMultiplier
            this.velY += this.gameData.world.gravity * gravityMultiplier
    }*/

    const auto gravity = 0.3f;

    if (!grounded) {
        const auto jumpGravityMultiplier = 0.8f;
        const auto fallGravityMultiplier = 1.5f;

        const auto gravityMultiplier = (jump && velocity.y >= 0)
            ? jumpGravityMultiplier
            : fallGravityMultiplier;
        velocity.y -= gravity * gravityMultiplier;
    }

    //this.grounded = false
    grounded = false;

    /*if (this.velY > 10) {
        this.velY = 10
    }*/
    velocity.y = std::max(-10.0f, velocity.y);

    const auto frictionX = 0.85;

    //velocity.x *= this.frictionX
    velocity.x *= frictionX;

    if (!dead) {
        position += velocity;
    }

    /*this.posX = Math.floor(this.x / this.gameData.block.size)
    this.posY = Math.floor(this.y / this.gameData.block.size)*/
    /*this.posX = Math.floor(this.x / this.gameData.block.size)
    this.posY = Math.floor(this.y / this.gameData.block.size)*/

	/*f32 movement = 0.0f;
	if (Input::isKeyHeld(KeyCode::D)) {
		movement += 1.0f;
	}
	if (Input::isKeyHeld(KeyCode::A)) {
		movement -= 1.0f;
	}
	velocity.x = movement;

	f32 gravity = 1.0f;
	velocity.y -= gravity * dt;

	position += velocity * dt;*/
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


void Player::blockCollision(const PlayerSettings& settings, const std::vector<Block>& blocks, f32 cellSize) {
    //f32 playerRadius = 30.0f;
    const auto playerAabb = aabb(settings);
    const auto playerSize = playerAabb.size();
    for (const auto& block : blocks) {
        const auto blockAabb = Aabb(block.position, block.position + Vec2(cellSize));
        const auto blockSize = blockAabb.size();
        
        if (!playerAabb.collides(blockAabb)) {
            continue;
        }

        auto distance = getDistance(playerAabb, blockAabb, velocity);
        
        using namespace BlockCollisionDirections;
        if (block.collisionDirections & L && distance.left < distance.top && distance.left < distance.bottom) {
            velocity.x = 0.0f;
            position.x = block.position.x - playerSize.x + settings.size.x / 2.0f;
        }
        if (block.collisionDirections & R && distance.right < distance.top && distance.right < distance.bottom) {
            velocity.x = 0.0f;
            position.x = block.position.x + blockSize.x + settings.size.x / 2.0f;
        }
        if (block.collisionDirections & D && distance.bottom < distance.left && distance.bottom < distance.right) {
            velocity.y = 0.0f;
            position.y = block.position.y - playerSize.y + settings.size.y / 2.0f;
        }
        if (block.collisionDirections & U && distance.top < distance.left && distance.top < distance.right) {
            velocity.y = 0.0f;
            position.y = block.position.y + blockSize.y + settings.size.y / 2.0f;
            grounded = true;
        }

    }
}
