#include <platformer/Collision.hpp>
#include <algorithm>

//const auto EPSILON = 1e-8f;
//
//static f32 sign(f32 v) {
//    return v < 0 ? -1.0f : 1.0f;
//}
//#include <framework/Dbg.hpp>
//std::optional<Hit> intersectSegment(
//    const Aabb& aabb,
//    Vec2 pos,
//    Vec2 delta,
//    f32 paddingX,
//    f32 paddingY) {
//
//    Dbg::drawAabb(aabb);
//
//    const auto thisPos = aabb.center();
//    const auto half = aabb.size() / 2.0f;
//
//    const auto scaleX = 1.0f / delta.x;
//    const auto scaleY = 1.0f / delta.y;
//    const auto signX = sign(scaleX);
//    const auto signY = sign(scaleY);
//    const auto nearTimeX =
//        (thisPos.x - signX * (half.x + paddingX) - pos.x) * scaleX;
//    const auto nearTimeY =
//        (thisPos.y - signY * (half.y + paddingY) - pos.y) * scaleY;
//    const auto farTimeX =
//        (thisPos.x + signX * (half.x + paddingX) - pos.x) * scaleX;
//    const auto farTimeY =
//        (thisPos.y + signY * (half.y + paddingY) - pos.y) * scaleY;
//    if (nearTimeX > farTimeY || nearTimeY > farTimeX) {
//        return std::nullopt;
//    }
//
//    const auto nearTime = nearTimeX > nearTimeY ? nearTimeX : nearTimeY;
//    const auto farTime = farTimeX < farTimeY ? farTimeX : farTimeY;
//    if (nearTime >= 1 || farTime <= 0) {
//        return std::nullopt;
//    }
//
//    //const hit = new Hit(this);
//    auto hit = Hit(aabb);
//
//    hit.time = std::clamp(nearTime, 0.0f, 1.0f);
//    if (nearTimeX > nearTimeY) {
//        hit.normal.x = -signX;
//        hit.normal.y = 0;
//    } else {
//        hit.normal.x = 0;
//        hit.normal.y = -signY;
//    }
//    hit.delta.x = (1.0 - hit.time) * -delta.x;
//    hit.delta.y = (1.0 - hit.time) * -delta.y;
//    hit.pos.x = pos.x + delta.x * hit.time;
//    hit.pos.y = pos.y + delta.y * hit.time;
//    return hit;
//}
//
//std::optional<Hit> aabbVsAabbCollision(const Aabb& a, const Aabb& box) {
//    const auto boxPos = box.center();
//    const auto boxHalf = box.size() / 2.0f;
//
//    const auto pos = a.center();
//    const auto half = a.size() / 2.0f;
//
//    const auto dx = boxPos.x - pos.x;
//    const auto px = boxHalf.x + half.x - abs(dx);
//    if (px <= 0) {
//        return std::nullopt;
//    }
//
//    const auto dy = boxPos.y - pos.y;
//    const auto py = boxHalf.y + half.y - abs(dy);
//    if (py <= 0) {
//        return std::nullopt;
//    }
//
//    if (px < py) {
//        const auto sx = sign(dx);
//        auto hit = Hit(a);
//        hit.pos = Vec2(pos.x + half.x * sx, boxPos.y);
//        hit.delta = Vec2(px * sx, 0.0f);
//        hit.normal = Vec2(sx, 0.0f);
//        /*return Hit{
//            .collider = a,
//            .pos = Vec2(pos.x + half.x * sx, boxPos.y),
//            .delta = Vec2(px * sx, 0.0f),
//            .normal = Vec2(sx, 0.0f),
//        };*/
//        /*hit.delta.x = px * sx;
//        hit.normal.x = sx;
//        hit.pos.x = this.pos.x + this.half.x * sx;
//        hit.pos.y = box.pos.y;*/
//    } else {
//        const auto sy = sign(dy);
//        auto hit = Hit(a);
//        //hit.collider = a;
//        hit.pos = Vec2(boxPos.x, pos.y + half.y * sy);
//        hit.delta = Vec2(0.0f, py * sy);
//        hit.normal = Vec2(0.0f, sy);
//        return hit;
//        /*hit.delta.y = py * sy;
//        hit.normal.y = sy;
//        hit.pos.x = box.pos.x;
//        hit.pos.y = this.pos.y + this.half.y * sy;*/
//
//        /*const sy = sign(dy);
//        hit.delta.y = py * sy;
//        hit.normal.y = sy;
//        hit.pos.x = box.pos.x;
//        hit.pos.y = this.pos.y + this.half.y * sy;*/
//    }
//}
//
//Sweep aabbVsSweeptAabbCollision(const Aabb& a, const Aabb& aabb, Vec2 delta) {
//    auto sweep = Sweep();
//    sweep.hit = std::nullopt;
//    sweep.pos = Vec2(0.0f);
//    sweep.time = 1;
//
//    const auto boxPos = aabb.center();
//    const auto boxHalf = aabb.size() / 2.0f;
//
//    const auto pos = a.center();
//    const auto half = a.size() / 2.0f;
//
//    if (delta.x == 0.0f && delta.y == 0.0f) {
//        sweep.pos.x = boxPos.x;
//        sweep.pos.y = boxPos.y;
//        /*sweep.hit = this.intersectAABB(box);*/
//        sweep.hit = aabbVsAabbCollision(a, aabb);
//        sweep.time = sweep.hit ? (sweep.hit->time = 0) : 1;
//        return sweep;
//    }
//
//    sweep.hit = intersectSegment(aabb, boxPos, delta, boxHalf.x, boxHalf.y);
//    if (sweep.hit) {
//        sweep.time = std::clamp(sweep.hit->time - EPSILON, 0.0f, 1.0f);
//        sweep.pos.x = boxPos.x + delta.x * sweep.time;
//        sweep.pos.y = boxPos.y + delta.y * sweep.time;
//        /*const direction = delta.clone();
//        direction.normalize();*/
//        const auto direction = delta.normalized();
//        sweep.hit->pos.x = std::clamp(
//            sweep.hit->pos.x + direction.x * boxHalf.x,
//            pos.x - half.x,
//            pos.x + half.x
//        );
//        sweep.hit->pos.y = std::clamp(
//            sweep.hit->pos.y + direction.y * boxHalf.y,
//            pos.y - half.y,
//            pos.y + half.y
//        );
//    } else {
//        sweep.pos.x = boxPos.x + delta.x;
//        sweep.pos.y = boxPos.y + delta.y;
//        sweep.time = 1;
//    }
//    return sweep;
//}
//
////Sweep sweepInto(staticColliders: Collider[], delta : Point) {
////    let nearest = new Sweep();
////    nearest.time = 1;
////    nearest.pos.x = this.pos.x + delta.x;
////    nearest.pos.y = this.pos.y + delta.y;
////    for (let i = 0, il = staticColliders.length; i < il; i++) {
////    const sweep = staticColliders[i].sweepAABB(this, delta);
////    if (sweep.time < nearest.time) {
////        nearest = sweep;
////    }
////    }
////    return nearest;
////}