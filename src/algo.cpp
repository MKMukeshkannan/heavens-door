module;

#include <vector>
#include <raylib.h>

export module algo;

import base;

constexpr size_t BEZIER_SEGMENT_SIZE=100;

export Res<void> compute_cubic_bezier(std::vector<Position> &bezier_path, Position p1, Position p2, Position h1, Position h2) {
    auto compute_cubic_bezier_t = [](Position p1, Position p2, Position h1, Position h2, float t) -> Position {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        return {uuu * p1.x + 3 * uu * t * h1.x + 3 * u * tt * h2.x + ttt * p2.x,
            uuu * p1.y + 3 * uu * t * h1.y + 3 * u * tt * h2.y + ttt * p2.y};
    };

    for (u8 i = 0; i < BEZIER_SEGMENT_SIZE; ++i) {
        float t = (float)i / BEZIER_SEGMENT_SIZE;
        Position pt = compute_cubic_bezier_t(p1, p2, h1, h2, t);
        bezier_path.push_back(pt);
    };

    return {};
}

export Res<void> compute_quadratic_bezier(std::vector<Position> &bezier_path, Position p1, Position p2, Position h1) {
    auto compute_quadratic_bezier_t = [](Position p1, Position p2, Position h1, float t) -> Position {
        float u = 1.0f - t;
        return {u * u * p1.x + 2 * u * t * h1.x + t * t * p2.x,
            u * u * p1.y + 2 * u * t * h1.y + t * t * p2.y};
    };

    for (u8 i = 0; i < BEZIER_SEGMENT_SIZE; ++i) {
        float t = (float)i / BEZIER_SEGMENT_SIZE;
        Position pt = compute_quadratic_bezier_t(p1, p2, h1, t);
        bezier_path.push_back(pt);
    };

    return {};
}

export f32 distance_between_points_squared(Position v1, Position v2) {
    f32 dx = v2.x - v1.x;
    f32 dy = v2.y - v1.y;
    return dx * dx + dy * dy;
}

export Position compute_mirror_from_custom_orgin(Position P, Position O) {
    float X = (2 * O.x) - P.x;
    float Y = (2 * O.y) - P.y;
    return Position{X, Y};
};

export bool is_mouse_over_anchor_point(Position pos, Position mouse) {
    bool collision = false;
    float distanceSquared = (mouse.x - pos.x)*(mouse.x - pos.x) + (mouse.y - pos.y)*(mouse.y - pos.y);
    if (distanceSquared <= RADIUS*RADIUS) collision = true;

    return collision;
};


export bool is_mouse_over_bounding_box(Position point, Rect rec) {
    bool collision = false;

    if ((point.x >= rec.x) && (point.x < (rec.x + rec.width)) && (point.y >= rec.y) && (point.y < (rec.y + rec.height))) collision = true;

    return collision;
};
