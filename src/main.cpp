#include <expected>
#include <map>
#include <print>
#include <raylib.h>
#include <vector>
#include "types.h"

constexpr Color PORCELEAN = Color{254, 254, 250, 255};
constexpr Color LAVENDER_MIST = Color{248, 244, 255, 255};

constexpr Color SOFT_BLUSH = Color{255, 229, 236, 255};     // BG
constexpr Color CHERRY_BLOSSOM = Color{255, 179, 198, 255}; // ACTIVE
constexpr Color COTTON_CANDY = Color{255, 143, 171, 255};   // HOT
constexpr Color PETAL_ROUGE = Color{251, 111, 146, 255};    // IDLE

const float RADIUS = 10.0f;
const float DRAG_THRESHOLD = 10.0f;
const u8 BEZIER_SEGMENT_SIZE = 100;

struct InputContext {
    uid hot = -1;
    uid active = -1;
    uid prev_active = -1;
};

enum class AnchorPointState {
    IDLE,
    HOVER,
    LEFT_CLICK,
    DRAG,
    RIGHT_CLICK,
};

enum class AnchorPointType {
    CORNER_POINT,
    SMOOTH_ANCHOR_POINT,
    // SYMMETRICAL_ANCHOR_POINT,
};

struct AnchorControlHandle {
    Position prev;
    Position next;
};

struct AnchorPointSystem {
    std::vector<Position> anchor_point_position = {};
    std::vector<AnchorPointState> anchor_point_state = {};
    std::vector<AnchorPointType> anchor_point_type = {};
    std::map<uid, AnchorControlHandle> anchor_control_handle = {};

    std::vector<Position> bezier_path = {};

    bool is_mouse_over_anchor_point(Position pos, Position mouse) {
        return CheckCollisionPointCircle(mouse, pos, RADIUS);
    }

    Res<uid> insert_anchor_point(Position anchor_pos, AnchorPointType anchor_type) {
        if (auto res = sanity_check(); !res)
            return std::unexpected(res.error());

        anchor_point_position.push_back(anchor_pos);
        anchor_point_state.push_back(AnchorPointState::IDLE);
        anchor_point_type.push_back(anchor_type);

        return anchor_point_position.size() - 1;
    }

    Res<void> remove_anchor_point(uid id) {
        return {};
    }

    Res<void> process_input(InputContext& input_ctx) {
        if (auto res = sanity_check(); !res)
            return res;

        Position mouse_pos = GetMousePosition();
        Position mouse_deta = GetMouseDelta();
        input_ctx.hot = -1;

        for (auto& state : anchor_point_state)
            state = AnchorPointState::IDLE;

        for (uid id = 0; id < anchor_point_position.size(); ++id) {
            if (auto res = validate_id(id); !res)
                return res;

            if (is_mouse_over_anchor_point({anchor_point_position[id].x, anchor_point_position[id].y}, mouse_pos)) {
                input_ctx.hot = id;
                anchor_point_state[id] = AnchorPointState::HOVER;
                break;
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && input_ctx.hot == -1 && input_ctx.active == -1) {
            auto res = insert_anchor_point(mouse_pos, AnchorPointType::CORNER_POINT);
            if (!res)
                return std::unexpected(res.error());

            uid id = *res;
            input_ctx.prev_active = id;
        }

        auto compute_mirror_from_custom_orgin = [](Position P, Position O) {
            float X = (2 * O.x) - P.x;
            float Y = (2 * O.y) - P.y;
            return Position{X, Y};
        };

        bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && ctrlDown && input_ctx.hot != -1 && input_ctx.active == -1) {
            input_ctx.active = input_ctx.hot;
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && ctrlDown && input_ctx.active != -1) {
            DrawText("MOVING", 0, 0, 25, BLACK);
            Position& anchor_position = anchor_point_position[input_ctx.active];
            AnchorPointType& anchor_type = anchor_point_type[input_ctx.active];

            anchor_point_state[input_ctx.active] = AnchorPointState::LEFT_CLICK;

            anchor_position.x += mouse_deta.x;
            anchor_position.y += mouse_deta.y;
            if (anchor_type == AnchorPointType::SMOOTH_ANCHOR_POINT) {
                anchor_control_handle[input_ctx.active].next.x += mouse_deta.x;
                anchor_control_handle[input_ctx.active].next.y += mouse_deta.y;
                anchor_control_handle[input_ctx.active].prev.x += mouse_deta.x;
                anchor_control_handle[input_ctx.active].prev.y += mouse_deta.y;
            }
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && input_ctx.prev_active != -1) {
            Position anchor_position = anchor_point_position[input_ctx.prev_active];
            if (distance_between_points_squared(anchor_position, mouse_pos) > DRAG_THRESHOLD) {
                anchor_point_type[input_ctx.prev_active] = AnchorPointType::SMOOTH_ANCHOR_POINT;
                anchor_control_handle[input_ctx.prev_active].next = mouse_pos;
                anchor_control_handle[input_ctx.prev_active].prev = compute_mirror_from_custom_orgin(mouse_pos, anchor_position);
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            input_ctx.prev_active = -1;
            input_ctx.active = -1;
        }

        return {};
    }

    float distance_between_points_squared(Position v1, Position v2) {
        float dx = v2.x - v1.x;
        float dy = v2.y - v1.y;
        return dx * dx + dy * dy;
    }

    Res<void> compute_bezier_path() {
        if (anchor_point_position.empty())
            return {};

        bezier_path.clear();

        bezier_path.push_back(anchor_point_position[0]);
        for (uid id = 1; id < anchor_point_position.size(); ++id) {
            if (anchor_point_type[id] == AnchorPointType::CORNER_POINT)
                bezier_path.push_back(anchor_point_position[id]);
            else if (anchor_point_type[id] == AnchorPointType::SMOOTH_ANCHOR_POINT && anchor_point_type[id - 1] == AnchorPointType::CORNER_POINT)
                auto res = compute_quadratic_bezier(anchor_point_position[id - 1], anchor_point_position[id], anchor_control_handle[id].prev);
            else if (anchor_point_type[id] == AnchorPointType::SMOOTH_ANCHOR_POINT && anchor_point_type[id - 1] == AnchorPointType::SMOOTH_ANCHOR_POINT)
                auto res = compute_cubic_bezier(anchor_point_position[id - 1], anchor_point_position[id], anchor_control_handle[id - 1].next, anchor_control_handle[id].prev);
        }

        return {};
    }

    Res<void> compute_cubic_bezier(Position p1, Position p2, Position h1, Position h2) {
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
        }

        return {};
    }

    Res<void> compute_quadratic_bezier(Position p1, Position p2, Position h1) {
        auto compute_quadratic_bezier_t = [](Position p1, Position p2, Position h1, float t) -> Position {
            float u = 1.0f - t;
            return {u * u * p1.x + 2 * u * t * h1.x + t * t * p2.x,
                    u * u * p1.y + 2 * u * t * h1.y + t * t * p2.y};
        };

        for (u8 i = 0; i < BEZIER_SEGMENT_SIZE; ++i) {
            float t = (float)i / BEZIER_SEGMENT_SIZE;
            Position pt = compute_quadratic_bezier_t(p1, p2, h1, t);
            bezier_path.push_back(pt);
        }

        return {};
    }

    Res<void> draw_anchor_points() {
        if (auto res = sanity_check(); !res)
            return res;

        if (anchor_point_position.size() > 1) {
            if (auto res = compute_bezier_path(); !res)
                return res;

            DrawLineStrip(bezier_path.data(), bezier_path.size(), BLACK);
            DrawLine((anchor_point_position.end() - 1)->x, (anchor_point_position.end() - 1)->y, GetMousePosition().x, GetMousePosition().y, BLACK);
        }

        for (uid id = 0; id < anchor_point_position.size(); ++id) {
            Color applied_color;
            switch (anchor_point_state[id]) {
                case AnchorPointState::IDLE:
                    applied_color = PETAL_ROUGE;
                    break;

                case AnchorPointState::HOVER:
                    applied_color = COTTON_CANDY;
                    break;

                case AnchorPointState::LEFT_CLICK:
                case AnchorPointState::DRAG:
                case AnchorPointState::RIGHT_CLICK:
                    applied_color = CHERRY_BLOSSOM;
                    break;

                default:
                    applied_color = RED;
                    break;
            }

            DrawRectangle(
                anchor_point_position[id].x - RADIUS / 2,
                anchor_point_position[id].y - RADIUS / 2,
                RADIUS, RADIUS, applied_color);
            DrawRectangleLines(
                anchor_point_position[id].x - RADIUS / 2,
                anchor_point_position[id].y - RADIUS / 2,
                RADIUS, RADIUS, BLACK);
        }

        for (auto [id, handle] : anchor_control_handle) {
            Position anchor_position = anchor_point_position[id];

            DrawCircle(handle.next.x, handle.next.y, 5, RED);
            DrawCircle(handle.prev.x, handle.prev.y, 5, RED);

            DrawLineV(anchor_position, handle.next, BLACK);
            DrawLineV(anchor_position, handle.prev, BLACK);
        }

        return {};
    }

    inline Res<void> sanity_check() {
        if (anchor_point_position.size() == anchor_point_state.size())
            return {};
        return Err("[FAILED SANITY CHECK] there is size mismatch in system arrays....");
    }

    inline Res<void> validate_id(uid id) {
        if (id >= anchor_point_position.size())
            return Err("[FAILED ID VALIDATION] invalid ID...");
        return {};
    }
};

Res<void> draw_status_bar(InputContext& input_ctx) {
    int screen_height = GetScreenHeight();
    int screen_width = GetScreenWidth();
    const int status_height = 32;
    const int status_width = 200;
    const int padding = 10;

    DrawRectangle(screen_width - status_width - padding, screen_height - status_height - padding, status_width + padding, status_height + padding, BLACK);

    Position mouse_deta = GetMouseDelta();
    if (input_ctx.active != -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && mouse_deta.x == 0 && mouse_deta.y == 0)
        DrawText("CLICKED", screen_width - status_width, screen_height - status_height, status_height, WHITE);
    else if (input_ctx.active != -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        DrawText("DRAGGING", screen_width - status_width, screen_height - status_height, status_height, WHITE);
    else if (input_ctx.hot != -1)
        DrawText("HOVEREd", screen_width - status_width, screen_height - status_height, status_height, WHITE);
    else
        DrawText("JUST MOVING AROUND", screen_width - status_width, screen_height - status_height, status_height, WHITE);

    return {};
}

int main() {
    AnchorPointSystem ap_system;
    InputContext input_ctx;

    InitWindow(900, 900, "BEZIER CURVE");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        ClearBackground(SOFT_BLUSH);

        if (auto res = ap_system.process_input(input_ctx); !res)
            return std::print("ERRROR {}", res.error()), -1;

        BeginDrawing();
        if (auto res = ap_system.draw_anchor_points(); !res)
            return std::print("{}", res.error()), -1;

        if (auto res = draw_status_bar(input_ctx); !res)
            return std::print("{}", res.error()), -1;
        EndDrawing();
    }

    return 0;
}
