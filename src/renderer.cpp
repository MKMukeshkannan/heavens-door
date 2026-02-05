module;

#include <raylib.h>
#include <cstddef>
#include <string>

export module renderer;

import app;
import base;
import core;

export Res<void> draw_application_context(ApplicationInputContext& input) 
{
    int screen_height = GetScreenHeight();
    int screen_width = GetScreenWidth();
    const int status_height = 32;
    const int status_width = 201;
    const int padding = 10;

    DrawRectangle(screen_width - status_width - padding, screen_height - status_height - padding, status_width + padding, status_height + padding, BLACK);
    switch (input.current_mode) {
        case ApplicationMode::DRAWING: 
            DrawText("DRAW", screen_width - status_width, screen_height - status_height, status_height, WHITE);
            return {};
        case ApplicationMode::IDLE: 
            DrawText("IDLE", screen_width - status_width, screen_height - status_height, status_height, WHITE);
            return {};
        case ApplicationMode::SELECTION: 
            DrawText("SELE", screen_width - status_width, screen_height - status_height, status_height, WHITE);
            return {};
    }

    return {};
};

export {
    using DrawProperty = u8;
    constexpr u8 CURRENTLY_DRAWING = (1 << 0);
    constexpr u8 GROUP_HOVERED     = (1 << 1);
};

export Res<void> draw_anchor_group(AnchorPointGroup& group, const std::string& name, DrawProperty property) {
    if (auto res = group.sanity_check(); !res)
        return res;

    if (group.anchor_point_position.size() > 1) {
        if (auto res = group.compute_bezier_path(); !res)
            return res;

        if (auto res = group.compute_bounding_box(); !res)
            return res;

        DrawLineStrip(group.bezier_path.data(), group.bezier_path.size(), BLACK);
        if (property & CURRENTLY_DRAWING)
            DrawLine((group.anchor_point_position.end() - 1)->x, (group.anchor_point_position.end() - 1)->y, GetMousePosition().x, GetMousePosition().y, BLACK);

        // DrawRectangleLines(
        //         group.bounding_box.x - BOUNDING_BOX_PADDING, 
        //         group.bounding_box.y - BOUNDING_BOX_PADDING, 
        //         group.bounding_box.width - group.bounding_box.x + (2 * BOUNDING_BOX_PADDING), 
        //         group.bounding_box.height - group.bounding_box.y + (2 * BOUNDING_BOX_PADDING), 
        //         BLUE);
        DrawRectangle(
                group.bounding_box.x - BOUNDING_BOX_PADDING, 
                group.bounding_box.y - BOUNDING_BOX_PADDING - 15, 
                MeasureText(name.c_str(), 15), 
                15,
                (property & GROUP_HOVERED) ? BLUE: GRAY);
        DrawText(name.c_str(), 
                group.bounding_box.x - BOUNDING_BOX_PADDING, 
                group.bounding_box.y - BOUNDING_BOX_PADDING - 15, 
                15, BLACK);
    }

    for (uid id = 0; id < group.anchor_point_position.size(); ++id) {
        Color applied_color;
        switch (group.anchor_point_state[id]) {
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

        if (property & CURRENTLY_DRAWING)
        {
            DrawRectangle(
                    group.anchor_point_position[id].x - RADIUS / 2,
                    group.anchor_point_position[id].y - RADIUS / 2,
                    RADIUS, RADIUS, applied_color);
            DrawRectangleLines(
                    group.anchor_point_position[id].x - RADIUS / 2,
                    group.anchor_point_position[id].y - RADIUS / 2,
                    RADIUS, RADIUS, BLACK);
        };
    }

    if (property & CURRENTLY_DRAWING)
        for (auto [id, handle] : group.anchor_control_handle) {
            Position anchor_position = group.anchor_point_position[id];

            DrawCircle(handle.next.x, handle.next.y, 5, RED);
            DrawCircle(handle.prev.x, handle.prev.y, 5, RED);

            DrawLineV(anchor_position, handle.next, BLACK);
            DrawLineV(anchor_position, handle.prev, BLACK);
        }

    return {};
}
