#ifndef DUNGEON_H
#define DUNGEON_H

#include "bahamut.h"
#include "map.h"

//
//   CONSTANTS
//

const i32 WALL_HP = 100;
const i32 MINE_COST = 50;
const i32 EXP_FOR_LEVEL = 100;
const i32 EXP_MULTIPLIER = 2;
const i32 MAX_ABILITIES = 5;

//
//   STRUCTS
//

enum DungeonState {
    DUNGEON_IDLE,
    DUNGEON_BUILD,
    DUNGEON_MINE,
    DUNGEON_SUMMARY,
};

enum AbilityType {
    ABILITY_FIREBALL,
    ABILITY_KICK,
    ABILITY_CLEAVE,
    ABILITY_RAISE_SKELETON,
    ABILITY_SWEEP,
    ABILITY_WING_ATTACK,
    ABILITY_BITE,
    ABILITY_SHADOWBOLT
};

struct Ability {
    AbilityType type;
    u16 levelReq;
};

struct UnitData {
    vec2 tilesetpos;
    u16 stamina_gain;
    u16 strength_gain;
    u16 dexterity_gain;
    u16 intelligence_gain;
    u16 cost;
    Ability abilities[MAX_ABILITIES];
};

const UnitData UNIT_TYPES[UNIT_TYPE_COUNT] = {
    /*imp*/        { {0, 0}, 1, 1, 1, 0, MINE_COST, { {ABILITY_KICK, 1}                                                                                         } },
    /*demon*/      { {1, 0}, 2, 2, 1, 0, 100,       { {ABILITY_KICK, 1},       {ABILITY_CLEAVE, 3},        {ABILITY_WING_ATTACK, 5}                             } },
    /*orc*/        { {2, 0}, 3, 2, 0, 0, 110,       { {ABILITY_BITE, 1},       {ABILITY_CLEAVE, 3},        {ABILITY_SWEEP, 5}                                   } },
    /*skeleton*/   { {3, 0}, 1, 2, 1, 0, 60,        { {ABILITY_BITE, 1},       {ABILITY_SWEEP, 3},         {ABILITY_KICK, 5},      {ABILITY_RAISE_SKELETON, 12} } },
    /*dragon*/     { {4, 0}, 3, 3, 2, 3, 200,       { {ABILITY_BITE, 1},       {ABILITY_WING_ATTACK, 3},   {ABILITY_FIREBALL, 5}                                } },
    /*necromancer*/{ {5, 0}, 2, 0, 0, 3, 150,       { {ABILITY_SHADOWBOLT, 1}, {ABILITY_RAISE_SKELETON, 3}                                                      } },

    /*platino*/    { {0, 1}, 5, 5, 5, 5, 1000,      { {ABILITY_FIREBALL, 1},   {ABILITY_SHADOWBOLT, 2},    {ABILITY_RAISE_SKELETON, 3}, {ABILITY_BITE, 4}, {ABILITY_WING_ATTACK, 5}}}
};

struct DungeonMap {
    Map map;
    u32 timer;
    std::vector<Unit> units;
    std::vector<vec2> minequeue;

    //heart of dungeon
    i32 hp;
    i32 mana;
    i32 exp;
};

struct DungeonScene {
    Texture unitset[2];
    Texture tileset;
    Texture hpbar;
    Texture menubar;
    Texture redbar;
    Texture bluebar;
    Texture purplebar;
};

static inline
DungeonScene load_dungeon_scene() {
    DungeonScene scene = {0};

    scene.tileset = load_texture("data/art/tileset.png", GL_NEAREST);
    scene.unitset[0] = load_texture("data/art/unitset.png", GL_NEAREST);
    scene.unitset[1] = load_texture("data/art/unitset2.png", GL_NEAREST);
    scene.menubar = load_texture("data/art/menubars.png", GL_NEAREST);
    scene.hpbar = load_texture("data/art/healthbar.png", GL_NEAREST);
    scene.redbar = load_texture("data/art/redbar.png", GL_NEAREST);
    scene.bluebar = load_texture("data/art/bluebar.png", GL_NEAREST);
    scene.purplebar = load_texture("data/art/purplebar.png", GL_NEAREST);

    return scene;
}

//
//   HELPER FUNCTIONS
//

static inline
void swap(vec2* a, vec2* b) {
    vec2 temp = *a;
    *a = *b;
    *b = temp;
}

static inline
void sort_by_dist(std::vector<vec2>& queue, vec2 origin) {
    for(u32 i = 0; i < queue.size()-1; ++i)
        for(u32 j = 0; j < queue.size()-1; ++j)
            if(getDistanceE(queue[j].x, queue[j].y, origin.x, origin.y) < getDistanceE(queue[j+1].x, queue[j+1].y, origin.x, origin.y))
                swap(&queue[j], &queue[j+1]);
}

static inline
void draw_units(RenderBatch* batch, DungeonMap* map, DungeonScene* scene) {
    for(u16 i = 0; i < map->units.size(); ++i) {
        Unit* unit = &map->units.at(i);
        draw_texture_EX(batch, map->timer % 100 < 50 ? scene->unitset[0] : scene->unitset[1], unit->tilesetpos, {unit->pos.x + map->map.x, unit->pos.y + map->map.y, (f32)TILE_SIZE, (f32)TILE_SIZE});

        if(unit->path.size() > 0) {
            vec2 seek = calculate_seek({(unit->path.back().x * TILE_SIZE), (unit->path.back().y * TILE_SIZE)}, unit);
            vec2 seperation = calculate_seperation(&map->map, &map->units, unit);
            unit->force = seek + seperation;
            truncate(&unit->force, MAX_FORCE);
        }
    }
}

static inline
void add_minequeue_no_copies(vec2 mouse, DungeonMap* map) {
    vec2 v = V2((i32)(mouse.x / TILE_SIZE), (i32)(mouse.y / TILE_SIZE));
    bool exists = false;
    for(u32 i = 0; i < map->minequeue.size(); ++i)
        if(map->minequeue[i] == v)
            exists = true;

    u32 index = v.x + v.y * map->map.width;
    if(!exists && blocked_tile(map->map.grid[index]))
        map->minequeue.push_back(v);
}

//
//   UPDATE GAME SYSTEMS
//

static inline
void update_units(DungeonMap* map, Unit* unit) {
    if(unit->state == UNIT_MINING) {
        unit->timer++;
        if(unit->timer > WALL_HP){
            unit->timer = 0;
            unit->path.clear();
            i32 tileindex;
            vec2 nearest = get_closest_mineable(map->minequeue, unit->pos, &tileindex);
            if(map->minequeue.size() > 0)
                map->minequeue.erase(map->minequeue.begin() + tileindex);
            u32 index = nearest.x + nearest.y * map->map.width;
            map->map.grid[index] = 8;
            orient_tiles(&map->map);
            unit->state = UNIT_IDLE;
        }
    }

    if(unit->type != UNIT_IMP) {
        if(unit->state == UNIT_IDLE) {
            
        }
    }

    if(unit->type == UNIT_IMP && unit->state == UNIT_IDLE && unit->path.size() == 0 && map->minequeue.size() > 0) {
        sort_by_dist(map->minequeue, V2(unit->pos.x/TILE_SIZE, unit->pos.y/TILE_SIZE));
        for(u32 j = 0; j < map->minequeue.size(); ++j) {
            if(!all_sides_blocked(&map->map, map->minequeue[j].x, map->minequeue[j].y))
                unit->path = pathfind_astar(&map->map, {unit->pos.x / TILE_SIZE, unit->pos.y / TILE_SIZE}, map->minequeue[j]);
            //unit->path = pathfind(&map->map, {unit->pos.x / TILE_SIZE, unit->pos.y / TILE_SIZE}, get_unblocked_neighbor(&map->map, map->minequeue[j]));
        }
    }

}

static inline
void unit_reached_destination(DungeonMap* map, Unit* unit) {
    if(unit->type == UNIT_IMP)
        unit->state = UNIT_MINING;
    else {

    }
}

//
//   MAIN DUNGEON GAME LOOP
//

static inline
void dungeon(RenderBatch* batch, DungeonMap* map, DungeonScene* scene, vec2 mouse) {
    draw_map(batch, &map->map, scene->tileset);
    map->timer++;

    draw_units(batch, map, scene);

    if(is_button_down(MOUSE_BUTTON_LEFT)) {
        add_minequeue_no_copies(V2(mouse.x - map->map.x, mouse.y - map->map.y), map);
    }

    //DEBUG
    for(u16 i = 0; i < map->minequeue.size(); ++i)
        draw_rectangle(batch, (map->minequeue[i].x * TILE_SIZE) + map->map.x, (map->minequeue[i].y * TILE_SIZE) + map->map.y, TILE_SIZE, TILE_SIZE, {211, 125, 44, 130});

    draw_texture(batch, scene->hpbar, 10, 10);
    draw_texture_EX(batch, scene->redbar, {0, 0, 52, 6}, {10 + 37, 10 + 5, (f32)((f32)map->hp / 100) * 52, 6});
    draw_texture_EX(batch, scene->bluebar, {0, 0, 52, 6}, {10 + 37, 10 + 15, (f32)((f32)map->mana / 100) * 52, 6});
    draw_texture_EX(batch, scene->purplebar, {0, 0, 52, 6}, {10 + 37, 10 + 25, (f32)((f32)map->exp / 100) * 52, 6});
    draw_texture(batch, scene->menubar, (get_virtual_width() / 2) - (scene->menubar.width/2), 330);

    //update units position based on velocity vector
    for(u16 i = 0; i < map->units.size(); ++i) {
        Unit* unit = &map->units[i];
        unit->velocity = unit->velocity + (SCALING_FACTOR * unit->force);
        truncate(&unit->velocity, MAX_SPEED);
        unit->pos = unit->pos + unit->velocity;

        update_units(map, unit);

        //check for reaching destination
        if(unit->path.size() > 0 && getDistanceE(unit->pos.x, unit->pos.y, (unit->path.back().x * TILE_SIZE), (unit->path.back().y * TILE_SIZE)) < (TILE_SIZE/1.5)) {
            u32 index = (i32)(unit->pos.x / TILE_SIZE) + (i32)(unit->pos.y / TILE_SIZE) * map->map.width;
            //reached destination
            if(abs(unit->velocity.x) < VELOCITY_MINIMUM && abs(unit->velocity.y) < VELOCITY_MINIMUM && !blocked_tile(map->map.grid[index]) && unit->path.size() == 1) {
                unit->path.clear();
                unit->velocity = {0, 0};
                unit->force = {0, 0};

                unit_reached_destination(map, unit);
            }

            if(unit->path.size() > 1)
                unit->path.pop_back();
        }
    }
}

#endif
