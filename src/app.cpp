module;

#include <print>
#include <expected>
#include <vector>
#include <string>
#include <raylib.h>

export module app;

import base;
import algo;
import core;

export struct ApplicationConfig {
    f32 default_height = 900;
    f32 default_width  = 900;
    bool is_resizable  = true;

    std::string title = "";
};

export struct ApplicationData {
    std::vector<AnchorPointGroup> groups;
    std::vector<std::string>      group_name;
};

export enum class ApplicationMode {
    IDLE,
    SELECTION,
    DRAWING,
};

export struct ApplicationInputContext {
    ApplicationMode current_mode = ApplicationMode::IDLE;
    uid hot_group    = -1;
    uid active_group = -1;
    std::vector<uid> selected_groups = {};
};


Res<void> handle_drawing_mode(ApplicationInputContext& ctx, ApplicationData& data) {
    if (IsKeyDown(KEY_ESCAPE))
    {
        ctx.current_mode = ApplicationMode::IDLE;
        ctx.active_group = -1;

        return {};
    };

    auto res = data.groups[ctx.active_group].process_input();
    if (!res)
        return res;

    return {};
};

export Res<void> process_application_input(ApplicationInputContext& ctx, ApplicationData& data)
{
    Position mouse_pos = GetMousePosition();
    ctx.hot_group = -1;

    if (ctx.current_mode == ApplicationMode::DRAWING && ctx.active_group != -1) 
        return handle_drawing_mode(ctx, data);


    for (size_t id = 0; id < data.groups.size(); ++id) 
    {
        const Rect& bounding_box = data.groups[id].bounding_box;
        const std::string& name  = data.group_name[id];

        const Rect label_pos = { 
            bounding_box.x - BOUNDING_BOX_PADDING, 
            bounding_box.y - BOUNDING_BOX_PADDING - 15, 
            static_cast<float>(MeasureText(name.c_str(), 15)), 
            15 
        };

        if (is_mouse_over_bounding_box(mouse_pos, label_pos)) 
        {
            ctx.hot_group = id;
            break;
        };
    };

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && ctx.hot_group == -1 && ctx.active_group == -1) {
        AnchorPointGroup new_group;
        auto res = new_group.insert_anchor_point(mouse_pos, AnchorPointType::CORNER_POINT);
        if (!res)
            return std::unexpected(res.error());

        uid id = *res;
        new_group.input_ctx.prev_active = id;


        ctx.active_group = data.groups.size();
        data.groups.push_back(new_group);

        auto name = std::format("obj_{}", data.groups.size());

        data.group_name.push_back(name);

        ctx.current_mode = ApplicationMode::DRAWING;
    };

    return {};
};

