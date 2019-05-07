#include "bahamut.h"
#include "utils.h"
#include "map.h"
#include "dungeon.h"

enum MainState {
    GOTO_TITLE,
    MAIN_TITLE,

    GOTO_DUNGEON,
    MAIN_DUNGEON,
    MAIN_OVERWORLD,

    GOTO_SUMMARY,
    MAIN_SUMMARY,

    MAIN_EXIT
};

struct TitleScene {
    Texture mountain1;
    Texture mountain2;
    Texture mountain3;
    Texture sky;
    Texture scroll;
    BitmapFont big;
    BitmapFont small;
    Sound bgm;
};

static inline
TitleScene load_title_scene() {
    TitleScene scene = {0};
    
    scene.big = load_neighbors_font(3);
    scene.small = load_neighbors_font(2);
    scene.mountain1 = load_texture("data/art/mountain1.png", GL_NEAREST);
    scene.mountain2 = load_texture("data/art/mountain2.png", GL_NEAREST);
    scene.mountain3 = load_texture("data/art/mountain3.png", GL_NEAREST);
    scene.sky = load_texture("data/art/sky.png", GL_NEAREST);
    scene.scroll = load_texture("data/art/scroll.png", GL_NEAREST);

    scene.bgm = load_sound("data/sound/Soliloquy.wav");
    set_sound_looping(scene.bgm, true);
    play_sound(scene.bgm);

    return scene;
}

static inline
void dispose_title_scene(TitleScene* scene) {
    dispose_texture(scene->mountain1);
    dispose_texture(scene->mountain2);
    dispose_texture(scene->mountain3);
    dispose_texture(scene->sky);
    dispose_texture(scene->scroll);

    dispose_sound(scene->bgm);
}

static inline
bool text_button(RenderBatch* batch, BitmapFont* font, std::string str, i32* yInitial, vec2 mouse) {
    f32 height = 16 * 2;
    f32 width = get_string_width(font, str.c_str());
    f32 xPos = (get_virtual_width() / 2) - (width / 2);
    f32 yPos = *yInitial += (16*2);
    bool collided = colliding({ xPos, yPos, width, height }, { mouse.x, mouse.y });
    if (collided) {
        str[0] = '>';
        str[1] = ' ';
        draw_text(batch, font, str.c_str(), xPos, yPos);
    }
    else {
        str[0] = ' ';
        str[1] = ' ';
        draw_text(batch, font, str.c_str(), xPos, yPos);
    }

    return collided & is_button_released(MOUSE_BUTTON_LEFT);
}

static inline
void title_screen(RenderBatch* batch, TitleScene* scene, MainState* mainstate, vec2 mouse) {
    static f32 xoffsets[6] = {0, 0, 0, 1280, 1280, 1280};

    for(u8 i = 0; i < 6; ++i) {
        xoffsets[i] -= .25 * ((i%3)+1);
        if((i32)xoffsets[i] <= (i32)-scene->mountain1.width)
            xoffsets[i] = (i32)scene->mountain1.width;
    }

    draw_texture(batch, scene->sky, 0, 0);

    draw_texture(batch, scene->mountain3, xoffsets[0], 60);
    draw_texture(batch, scene->mountain3, xoffsets[3], 60);

    draw_texture(batch, scene->mountain2, xoffsets[1], 60);
    draw_texture(batch, scene->mountain2, xoffsets[4], 60);

    draw_texture(batch, scene->mountain1, xoffsets[2], 60);
    draw_texture(batch, scene->mountain1, xoffsets[5], 60);

    draw_text(batch, &scene->big, "Monster Manager", (get_virtual_width()/2) - (get_string_width(&scene->big, "Monster Manager")/2), 10);

    i32 yInitial = 160;

    if(text_button(batch, &scene->small, "  Play Game", &yInitial, mouse)) {
        *mainstate = GOTO_DUNGEON;
    }
    if(text_button(batch, &scene->small, "  Options", &yInitial, mouse)) {

    }
    if(text_button(batch, &scene->small, "  Credits", &yInitial, mouse)) {

    }
    if(text_button(batch, &scene->small, "  Quit", &yInitial, mouse)) {
        *mainstate = MAIN_EXIT;
    }

    //draw_texture(batch, scene->scroll, 100, 100);
}

int main() {
    printf("\n/////////////////////////////////\nPROGRAM STARTING\n/////////////////////////////////\n\n");
    init_window(640, 360, "Monster Manager", false, true, true);
    init_audio();
    set_fps_cap(60);
    set_master_volume(100);
    set_vsync(true);
    set_mouse_state(MOUSE_HIDDEN);

    RenderBatch * batch = &create_batch();
    Shader basic = load_default_shader_2D();
    MainState state = GOTO_TITLE;

    DungeonMap dungeonMap = {0};
    dungeonMap.hp = 100;
    dungeonMap.mana = 50;
    dungeonMap.exp = 15;
    dungeonMap.map = load_random_map(100, 90);
    
    Texture cursor = load_texture("data/art/cursor.png", GL_NEAREST);

    Unit unit = {0};
    unit.pos = {15 * TILE_SIZE, 15 * TILE_SIZE};
    unit.tilesetpos = { 0 * TILE_SIZE, 0 * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    unit.hp = 10;
    unit.mana = 10;
    unit.type = UNIT_IMP;
    dungeonMap.units.push_back(unit);
    unit.type = UNIT_DEMON;
    unit.tilesetpos = { 1 * TILE_SIZE, 0 * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    dungeonMap.units.push_back(unit);
    unit.type = UNIT_ORC;
    unit.pos = {15 * TILE_SIZE, 16 * TILE_SIZE};
    unit.tilesetpos = { 2 * TILE_SIZE, 0 * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    dungeonMap.units.push_back(unit);
    unit.type = UNIT_SKELETON;
    unit.pos = {15 * TILE_SIZE, 17 * TILE_SIZE};
    unit.tilesetpos = { 3 * TILE_SIZE, 0 * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    dungeonMap.units.push_back(unit);
    unit.type = UNIT_DRAGON;
    unit.pos = {15 * TILE_SIZE, 18 * TILE_SIZE};
    unit.tilesetpos = { 4 * TILE_SIZE, 0 * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    dungeonMap.units.push_back(unit);

    start_shader(basic);
    upload_mat4(basic, "projection", orthographic_projection(0, 0, get_window_width(), get_window_height(), -1, 1));
    stop_shader();

    DungeonScene dungeonScene;
    TitleScene titlescene; 

    while(window_open()) {
        Rect view = fit_aspect_ratio(1.777777777777778);
        vec2 mouse = get_mouse_pos(view);
        set_viewport(view.x, view.y, view.width, view.height);

        begin_drawing();
        begin2D(batch, basic);
            
            if(state == GOTO_TITLE) {
                titlescene = load_title_scene();
                state = MAIN_TITLE;
            }
            if(state == MAIN_TITLE) {
                title_screen(batch, &titlescene, &state, mouse);
            }
            if(state == GOTO_DUNGEON) {
                dungeonScene = load_dungeon_scene();
                dispose_title_scene(&titlescene);
                state = MAIN_DUNGEON;
            }
            if(state == MAIN_DUNGEON) {
                dungeon(batch, &dungeonMap, &dungeonScene, mouse);
            }
            if(state == MAIN_EXIT) 
                exit(0);

            draw_texture(batch, cursor, mouse.x, mouse.y);
            
        end2D(batch);
        end_drawing();
    }

    dispose_batch(batch);
    dispose_window();

    return 0;
}
