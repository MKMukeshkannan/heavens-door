#include <cstddef>
#include <raylib.h>
#include <iostream>

import base;
import app;
import core;
import renderer;

int main() {
    ApplicationConfig app_config;
    ApplicationInputContext app_ctx;
    ApplicationData app_data;

    if (app_config.is_resizable) 
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);


    InitWindow(app_config.default_width, app_config.default_height, app_config.title.c_str());
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        ClearBackground(SOFT_BLUSH);

        if (auto res = process_application_input(app_ctx, app_data); !res)
            return std::cout << "[ERROR]" << res.error().c_str() << std::endl, -1;

        BeginDrawing();
        for (uid id = 0; id < (uid)app_data.groups.size(); ++id)
        {
            DrawProperty props = 0;

            if (app_ctx.current_mode == ApplicationMode::DRAWING && app_ctx.active_group == id)
                props |= CURRENTLY_DRAWING;

            if (app_ctx.current_mode == ApplicationMode::IDLE && app_ctx.hot_group == id)
                props |= GROUP_HOVERED;

            AnchorPointGroup& group = app_data.groups[id];
            std::string group_name = app_data.group_name[id];
            if (auto res = draw_anchor_group(group, group_name, props); !res)
                return std::cout << "[ERROR]" << res.error().c_str() << std::endl, -1;
        };

        if (auto res = draw_application_context(app_ctx); !res)
            return std::cout << "[ERROR]" << res.error().c_str() << std::endl, -1;

        EndDrawing();
    }

    return 0;
}
