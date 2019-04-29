#include "bahamut.h"
#include "map.h"
#include "dungeon.h"

enum MainState {
    MAIN_TITLE,
    MAIN_DUNGEON,
    MAIN_OVERWORLD,
    MAIN_SUMMARY,
    MAIN_EXIT
};

int main() {
    init_window(640, 360, "Monster Manager", false, true, true);
    set_fps_cap(60);
    set_vsync(true);

    RenderBatch * batch = &create_batch();
    Shader basic = load_default_shader_2D();
    MainState state = MAIN_TITLE;

    DungeonScene dungeonScene = load_dungeon_scene();
    DungeonMap dungeonMap = {0};
    dungeonMap.map = load_random_map(50, 50);

    Unit unit = {0};
    unit.pos = {15 * TILE_SIZE, 15 * TILE_SIZE};
    unit.tilesetpos = { 0 * TILE_SIZE, 0 * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    unit.dmg = 5;
    unit.hp = 10;
    unit.mana = 10;
    unit.dodge = 5;
    unit.type = UNIT_IMP;
    dungeonMap.units.push_back(unit);

    start_shader(basic);
    upload_mat4(basic, "projection", orthographic_projection(0, 0, get_window_width(), get_window_height(), -1, 1));
    stop_shader();

    while(window_open()) {
        Rect view = fit_aspect_ratio(1.777777777777778);
        vec2 mouse = get_mouse_pos(view);
        set_viewport(view.x, view.y, view.width, view.height);

        begin_drawing();
        begin2D(batch, basic);

            if(state == MAIN_TITLE) {
                dungeon(batch, &dungeonMap, &dungeonScene, mouse);
            }
            if(state == MAIN_EXIT) 
                exit(0);
            
        end2D(batch);
        end_drawing();
    }

    dispose_batch(batch);
    dispose_window();

    return 0;
}
