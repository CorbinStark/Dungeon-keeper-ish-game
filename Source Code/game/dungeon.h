#ifndef DUNGEON_H
#define DUNGEON_H

#include "bahamut.h"
#include "map.h"

const i32 WALL_HP = 40;
const i32 MINE_COST = 50;
const i32 EXP_FOR_LEVEL = 100;
const i32 EXP_MULTIPLIER = 2;


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

const i32 MAX_ABILITIES = 5;

struct UnitData {
    vec2 tilesetpos;
    u16 stamina_gain;
    u16 strength_gain;
    u16 dexterity_gain;
    u16 intelligence_gain;
    u16 cost;
    Ability abilities[MAX_ABILITIES];
};

const UnitData IMP         = { {0, 0}, 1, 1, 1, 0, MINE_COST, { {ABILITY_KICK, 1}                                                                                         } };
const UnitData DEMON       = { {1, 0}, 2, 2, 1, 0, 100,       { {ABILITY_KICK, 1},       {ABILITY_CLEAVE, 3},        {ABILITY_WING_ATTACK, 5}                             } };
const UnitData ORC         = { {2, 0}, 3, 2, 0, 0, 110,       { {ABILITY_BITE, 1},       {ABILITY_CLEAVE, 3},        {ABILITY_SWEEP, 5}                                   } };
const UnitData SKELETON    = { {3, 0}, 1, 2, 1, 0, 60,        { {ABILITY_BITE, 1},       {ABILITY_SWEEP, 3},         {ABILITY_KICK, 5},      {ABILITY_RAISE_SKELETON, 12} } };
const UnitData DRAGON      = { {4, 0}, 3, 3, 2, 3, 200,       { {ABILITY_BITE, 1},       {ABILITY_WING_ATTACK, 3},   {ABILITY_FIREBALL, 5}                                } };
const UnitData NECROMANCER = { {5, 0}, 2, 0, 0, 3, 150,       { {ABILITY_SHADOWBOLT, 1}, {ABILITY_RAISE_SKELETON, 3}                                                      } };
const UnitData PLATINO     = { {0, 1}, 5, 5, 5, 5, 1000,      { {ABILITY_FIREBALL, 1},   {ABILITY_SHADOWBOLT, 2},    {ABILITY_RAISE_SKELETON, 3}, {ABILITY_BITE, 4}, {ABILITY_WING_ATTACK, 5}}};

struct DungeonMap {
    Map map;
    u32 timer;
    std::vector<Unit> units;
    std::vector<vec2> minequeue;
};

struct DungeonScene {
    Texture unitset[2];
    Texture tileset;
};

static inline
DungeonScene load_dungeon_scene() {
    DungeonScene scene = {0};

    scene.tileset = load_texture("data/art/tileset.png", GL_NEAREST);
    scene.unitset[0] = load_texture("data/art/unitset.png", GL_NEAREST);
    scene.unitset[1] = load_texture("data/art/unitset2.png", GL_NEAREST);

    return scene;
}

static inline
void draw_units(RenderBatch* batch, DungeonMap* map, DungeonScene* scene) {
    for(u16 i = 0; i < map->units.size(); ++i) {
        Unit* unit = &map->units.at(i);
        draw_texture_EX(batch, map->timer % 100 < 50 ? scene->unitset[0] : scene->unitset[1], unit->tilesetpos, {unit->pos.x, unit->pos.y, (f32)TILE_SIZE, (f32)TILE_SIZE});

        if(unit->path.size() > 0) {
            vec2 seek = calculate_seek({(unit->path.back().x * TILE_SIZE)+(TILE_SIZE/2), (unit->path.back().y * TILE_SIZE)+(TILE_SIZE/2)}, unit);
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

static inline
void dungeon(RenderBatch* batch, DungeonMap* map, DungeonScene* scene, vec2 mouse) {
    draw_map(batch, &map->map, scene->tileset);
    map->timer++;

    draw_units(batch, map, scene);

    if(is_button_down(MOUSE_BUTTON_LEFT)) {
        add_minequeue_no_copies(mouse, map);
    }

    //DEBUG
    for(u16 i = 0; i < map->minequeue.size(); ++i)
        draw_rectangle(batch, map->minequeue[i].x * TILE_SIZE, map->minequeue[i].y * TILE_SIZE, TILE_SIZE, TILE_SIZE, {255, 100, 100, 100});
    for(u16 i = 0; i < map->units.back().path.size(); ++i)
        draw_rectangle(batch, map->units.back().path[i].x * TILE_SIZE, map->units.back().path[i].y * TILE_SIZE, TILE_SIZE, TILE_SIZE, {100, 255, 100, 100});

    //update units position based on velocity vector
    for(u16 i = 0; i < map->units.size(); ++i) {
        Unit* unit = &map->units[i];
        unit->velocity = unit->velocity + (SCALING_FACTOR * unit->force);
        truncate(&unit->velocity, MAX_SPEED);
        unit->pos = unit->pos + unit->velocity;

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

        if(unit->type == UNIT_IMP && unit->state == UNIT_IDLE && unit->path.size() == 0 && map->minequeue.size() > 0) {
            for(u32 j = 0; j < map->minequeue.size(); ++j) {
                if(!all_sides_blocked(&map->map, map->minequeue[j].x, map->minequeue[j].y))
                    unit->path = pathfind_astar(&map->map, {unit->pos.x / TILE_SIZE, unit->pos.y / TILE_SIZE}, {map->minequeue[j].x, map->minequeue[j].y});
            }
        }

        //check for reaching destination
        if(unit->path.size() > 0 && getDistanceE(unit->pos.x, unit->pos.y, (unit->path.back().x * TILE_SIZE) + (TILE_SIZE/2), (unit->path.back().y * TILE_SIZE) + (TILE_SIZE/2)) < (TILE_SIZE/1.5)) {
            u32 index = (i32)(unit->pos.x / TILE_SIZE) + (i32)(unit->pos.y / TILE_SIZE) * map->map.width;
            //reached destination
            if(abs(unit->velocity.x) < VELOCITY_MINIMUM && abs(unit->velocity.y) < VELOCITY_MINIMUM && !blocked_tile(map->map.grid[index]) && unit->path.size() == 1) {
                unit->path.clear();
                unit->velocity = {0, 0};
                unit->force = {0, 0};

                if(unit->type == UNIT_IMP)
                    unit->state = UNIT_MINING;
                else {

                }
            }

            if(unit->path.size() > 1)
                unit->path.pop_back();
        }
    }
}

#endif
