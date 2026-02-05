#include <climits>
module;

#include <print>
#include <vector>
#include <map>
#include <raylib.h>
#include <expected>

export module core;

import base;
import algo;

const float DRAG_THRESHOLD = 10.0f;


export enum class AnchorPointState {
    IDLE,
    HOVER,
    LEFT_CLICK,
    DRAG,
    RIGHT_CLICK,
};

export enum class AnchorPointType {
    CORNER_POINT,
    SMOOTH_ANCHOR_POINT,
    // SYMMETRICAL_ANCHOR_POINT,
};

struct AnchorControlHandle {
    Position prev;
    Position next;
};

export struct AnchorPointGroup {
    std::vector<Position> anchor_point_position = {};
    std::vector<AnchorPointState> anchor_point_state = {};
    std::vector<AnchorPointType> anchor_point_type = {};
    std::map<uid, AnchorControlHandle> anchor_control_handle = {};

    struct InputContext {
        uid hot = -1;
        uid active = -1;
        uid prev_active = -1;
    } input_ctx;

    std::vector<Position> bezier_path = {};

    Rect bounding_box = {INT_MAX, INT_MAX, INT_MIN, INT_MIN};

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

    Res<void> process_input() {
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

    Res<void> compute_bounding_box() {
        if (anchor_point_position.empty())
            return {};

        for (const auto [x, y]: bezier_path) 
        {
            bounding_box.x      = std::min(bounding_box.x, x);
            bounding_box.y      = std::min(bounding_box.y, y);
            bounding_box.width  = std::max(bounding_box.width,  x);
            bounding_box.height = std::max(bounding_box.height, y);
        };

        // std::println("{} {} {} {}", bounding_box.x, bounding_box.y, bounding_box.height, bounding_box.width);

        return {};
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
                auto res = compute_quadratic_bezier(bezier_path, anchor_point_position[id - 1], anchor_point_position[id], anchor_control_handle[id].prev);
            else if (anchor_point_type[id] == AnchorPointType::SMOOTH_ANCHOR_POINT && anchor_point_type[id - 1] == AnchorPointType::SMOOTH_ANCHOR_POINT)
                auto res = compute_cubic_bezier(bezier_path, anchor_point_position[id - 1], anchor_point_position[id], anchor_control_handle[id - 1].next, anchor_control_handle[id].prev);
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
