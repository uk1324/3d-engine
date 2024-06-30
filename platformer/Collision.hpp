#pragma once

#include <engine/Math/Aabb.hpp>
#include <optional>

const f32 EPSILON = 1e-8f;

inline f32 clamp(f32 value, f32 min, f32 max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

inline f32 sign(f32 value) {
    return value < 0 ? -1 : 1;
}

//struct Point {
//    f32 x;
//    f32 y;
//
//    Point(f32 x = 0, f32 y = 0) {
//        this->x = x;
//        this->y = y;
//    }
//
//    f32 normalize() {
//        f32 length = this->x * this->x + this->y * this->y;
//        if (length > 0) {
//            length = sqrt(length);
//            const auto inverseLength = 1.0 / length;
//            this->x *= inverseLength;
//            this->y *= inverseLength;
//        } else {
//            this->x = 1;
//            this->y = 0;
//        }
//        return length;
//    }
//};

struct Hit {
    Vec2 pos;
    Vec2 delta;
    Vec2 normal;
    f32 time;

    Hit() {
        this->pos = Vec2(0.0f);
        this->delta = Vec2(0.0f);
        this->normal = Vec2(0.0f);
        this->time = 0;
    }
};

struct Sweep {
    std::optional<Hit> hit;
    Vec2 pos;
    f32 time;

    Sweep() {
        this->hit = std::nullopt;
        this->pos = Vec2(0.0f);
        this->time = 1;
    }
};

struct AABB {
    Vec2 pos;
    Vec2 half;

    AABB(Vec2 pos, Vec2 half) {
        this->pos = pos;
        this->half = half;
    }

    std::optional<Hit> intersectSegment(
        Vec2 pos,
        Vec2 delta,
        f32 paddingX = 0,
        f32 paddingY = 0
    ) {
        const auto scaleX = 1.0 / delta.x;
        const auto scaleY = 1.0 / delta.y;
        const auto signX = sign(scaleX);
        const auto signY = sign(scaleY);
        const auto nearTimeX =
        (this->pos.x - signX * (this->half.x + paddingX) - pos.x) * scaleX;
        const auto nearTimeY =
        (this->pos.y - signY * (this->half.y + paddingY) - pos.y) * scaleY;
        const auto farTimeX =
        (this->pos.x + signX * (this->half.x + paddingX) - pos.x) * scaleX;
        const auto farTimeY =
        (this->pos.y + signY * (this->half.y + paddingY) - pos.y) * scaleY;
        if (nearTimeX > farTimeY || nearTimeY > farTimeX) {
            return std::nullopt;
        }

        const auto nearTime = nearTimeX > nearTimeY ? nearTimeX : nearTimeY;
        const auto farTime = farTimeX < farTimeY ? farTimeX : farTimeY;
        if (nearTime >= 1 || farTime <= 0) {
        return std::nullopt;
        }

        auto hit = Hit();
        hit.time = clamp(nearTime, 0, 1);
        if (nearTimeX > nearTimeY) {
            hit.normal.x = -signX;
            hit.normal.y = 0;
        } else {
            hit.normal.x = 0;
            hit.normal.y = -signY;
        }
        hit.delta.x = (1.0 - hit.time) * -delta.x;
        hit.delta.y = (1.0 - hit.time) * -delta.y;
        hit.pos.x = pos.x + delta.x * hit.time;
        hit.pos.y = pos.y + delta.y * hit.time;
        return hit;
    }

        std::optional<Hit> intersectAABB(AABB box) {
          const auto dx = box.pos.x - this->pos.x;
          const auto px = box.half.x + this->half.x - abs(dx);
          if (px <= 0) {
            return std::nullopt;
          }

          const auto dy = box.pos.y - this->pos.y;
          const auto py = box.half.y + this->half.y - abs(dy);
          if (py <= 0) {
            return std::nullopt;
          }

          auto hit = Hit();
          if (px < py) {
            const auto sx = sign(dx);
            hit.delta.x = px * sx;
            hit.normal.x = sx;
            hit.pos.x = this->pos.x + this->half.x * sx;
            hit.pos.y = box.pos.y;
          } else {
            const auto sy = sign(dy);
            hit.delta.y = py * sy;
            hit.normal.y = sy;
            hit.pos.x = box.pos.x;
            hit.pos.y = this->pos.y + this->half.y * sy;
          }
          return hit;
    }

        Sweep sweepAABB(AABB box, Vec2 delta) {
          auto sweep = Sweep();
          if (delta.x == 0 && delta.y == 0) {
            sweep.pos.x = box.pos.x;
            sweep.pos.y = box.pos.y;
            sweep.hit = this->intersectAABB(box);
            sweep.time = sweep.hit ? (sweep.hit->time = 0) : 1;
            return sweep;
          }

          sweep.hit = this->intersectSegment(box.pos, delta, box.half.x, box.half.y);
          if (sweep.hit) {
            /*sweep.time = clamp(sweep.hit->time - EPSILON, 0, 1);*/
              sweep.time = clamp(sweep.hit->time, 0, 1);
            sweep.pos.x = box.pos.x + delta.x * sweep.time;
            sweep.pos.y = box.pos.y + delta.y * sweep.time;
            auto direction = delta.normalized();
            //direction.normalize();
            sweep.hit->pos.x = clamp(
              sweep.hit->pos.x + direction.x * box.half.x,
              this->pos.x + this->half.x,
                this->pos.x - this->half.x
            );
            sweep.hit->pos.y = clamp(
              sweep.hit->pos.y + direction.y * box.half.y,
              this->pos.y - this->half.y,
              this->pos.y + this->half.y
            );
          } else {
            sweep.pos.x = box.pos.x + delta.x;
            sweep.pos.y = box.pos.y + delta.y;
            sweep.time = 1;
          }
          return sweep;
    }

};