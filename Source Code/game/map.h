#ifndef MAP_H
#define MAP_H

#include <vector>
#include <memory>
#include <algorithm>
#include "bahamut.h"
#include "utils.h"

//
//   CONSTANTS
//

//#define DIAGONAL_ASTAR

const i32 TILE_SIZE = 16;
const i32 TILESET_WIDTH = 13; //measured in 16x16 blocked, aka tiles
const i32 SCROLL_SPEED = 4;

const f32 MAX_SPEED = 0.8;
const f32 MAX_FORCE = 1.0;
const f32 MIN_SEPERATION = 10;
const f32 SCALING_FACTOR = (1/15.0f);
const f32 VELOCITY_MINIMUM = 0.95;

//
//   DATA TYPES
//

struct Stats {
    i32 str;
    i32 stam;
    i32 dex;
    i32 intl;
};

enum UnitType {
    UNIT_IMP,
    UNIT_DEMON,
    UNIT_ORC,
    UNIT_SKELETON,
    UNIT_DRAGON,
    UNIT_NECROMANCER,
    UNIT_PLATINO,
    UNIT_TYPE_COUNT
};

enum UnitState {
    UNIT_IDLE,
    UNIT_MINING,
    UNIT_WALKING,
    UNIT_BUSY,
    UNIT_RAIDING
};

struct Unit {
    vec2 pos;
    vec2 velocity;
    vec2 force;
    Rect tilesetpos;

    i32 expToNext;
    Stats stats;
    i32 level;
    i32 hp;
    i32 mana;
    i32 timer;

    UnitType type;
    UnitState state;
    std::vector<vec2> path;
};
typedef std::vector<Unit> UnitList;

struct Map {
    i32* grid;
    u16 width;
    u16 height;
    i32 x;
    i32 y;
};

struct Node;
typedef std::shared_ptr<Node> NodePtr;
struct Node {
    Node(i32 x, i32 y, NodePtr parent, i32 fcost) {
        this->x = x;
        this->y = y;
        this->parent = parent;
        this->fcost = fcost;
    }
    i32 x;
    i32 y;
    NodePtr parent;
    i32 fcost;
};

//
//   HELPER FUNCTIONS
//

static inline
bool blocked_tile(i32 id) {
    if(id % TILESET_WIDTH <= 5)
        return true;
    return false;
}

static inline
void check_blocked_neighbors(Map* map, i32 x, i32 y, bool* up, bool* down, bool* left, bool* right) {
    *up = true;
    *down = true;
    *left = true;
    *right = true;

    if(y-1 >= 0) 
        *up = blocked_tile(map->grid[x + (y-1) * map->width]);
    if(y + 1 <= map->height)
        *down = blocked_tile(map->grid[x + (y+1) * map->width]);
    if(x - 1 >= 0)
        *left = blocked_tile(map->grid[(x-1) + y * map->width]);
    if(x + 1 <= map->width)
        *right = blocked_tile(map->grid[(x+1) + y * map->width]);
}

static inline
vec2 get_unblocked_neighbor(Map* map, vec2 tile) {
    bool upblocked, downblocked, leftblocked, rightblocked;
    check_blocked_neighbors(map, tile.x, tile.y, &upblocked, &downblocked, &leftblocked, &rightblocked);

    if(!upblocked)
        return {(f32)tile.x, (f32)tile.y-1};
    if(!downblocked)
        return {(f32)tile.x, (f32)tile.y+1};
    if(!leftblocked)
        return {(f32)tile.x-1, (f32)tile.y};
    if(!rightblocked)
        return {(f32)tile.x+1, (f32)tile.y};

    return tile;
}

static inline
bool all_sides_blocked(Map* map, i32 x, i32 y) {
    bool upblocked, downblocked, leftblocked, rightblocked;
    check_blocked_neighbors(map, x, y, &upblocked, &downblocked, &leftblocked, &rightblocked);
    if(upblocked && rightblocked && leftblocked && downblocked)
        return true;
    return false;
}

static inline
void orient_tiles(Map* map) {
    for(i32 x = 0; x < map->width; ++x) {
        for(i32 y = 0; y < map->height; ++y) {
            u32 index = x + y * map->width;
            i32 id = map->grid[index];

            bool upblocked, downblocked, leftblocked, rightblocked;
            check_blocked_neighbors(map, x, y, &upblocked, &downblocked, &leftblocked, &rightblocked);

            if(blocked_tile(id)) {
                if(downblocked && rightblocked)
                    map->grid[index] = 0;

                if(downblocked && leftblocked)
                    map->grid[index] = 2;

                if(upblocked && rightblocked)
                    map->grid[index] = 0 + 2 * TILESET_WIDTH;

                if(upblocked && leftblocked)
                    map->grid[index] = 2 + 2 * TILESET_WIDTH;

                if((!leftblocked || !rightblocked) && upblocked && downblocked) 
                    map->grid[index] = 0 + 1 * TILESET_WIDTH;

                if((!downblocked || !upblocked) && leftblocked && rightblocked)
                    map->grid[index] = 1 + 0 * TILESET_WIDTH;

                if(!upblocked && !downblocked && !rightblocked && !leftblocked)
                    map->grid[index] = 1 + 1 * TILESET_WIDTH;

                if(upblocked && downblocked && rightblocked && leftblocked) {
                    map->grid[index] = 1 + 2 * TILESET_WIDTH;

                    if(x - 1 >= 0 && y - 1 >= 0)
                        if(!blocked_tile(map->grid[(x-1) + (y-1) * map->width]))
                            map->grid[index] = 2 + 2 * TILESET_WIDTH;

                    if(x + 1 <= map->width && y - 1 >= 0)
                        if(!blocked_tile(map->grid[(x+1) + (y-1) * map->width]))
                            map->grid[index] = 0 + 2 * TILESET_WIDTH;

                    if(x - 1 >= 0 && y + 1 <= map->height)
                        if(!blocked_tile(map->grid[(x-1) + (y+1) * map->width]))
                            map->grid[index] = 2 + 0 * TILESET_WIDTH;

                    if(x + 1 <= map->width && y + 1 <= map->height)
                        if(!blocked_tile(map->grid[(x+1) + (y+1) * map->width]))
                            map->grid[index] = 0;
                }

            } else {
                if(leftblocked)
                    map->grid[index] = 6 + 1 * TILESET_WIDTH;

                if(upblocked)
                    map->grid[index] = 7 + 0 * TILESET_WIDTH;

                if(rightblocked)
                    map->grid[index] = 8 + 1 * TILESET_WIDTH;

                if(downblocked)
                    map->grid[index] = 7 + 2 * TILESET_WIDTH;

                if(upblocked && leftblocked)
                    map->grid[index] = 6 + 0 * TILESET_WIDTH;

                if(upblocked && rightblocked)
                    map->grid[index] = 8 + 0 * TILESET_WIDTH;

                if(downblocked && leftblocked)
                    map->grid[index] = 6 + 2 * TILESET_WIDTH;

                if(downblocked && rightblocked)
                    map->grid[index] = 8 + 2 * TILESET_WIDTH;

                if(upblocked && leftblocked && rightblocked)
                    map->grid[index] = 9 + 0 * TILESET_WIDTH;

                if(leftblocked && rightblocked)
                    map->grid[index] = 9 + 1 * TILESET_WIDTH;

                if(downblocked && leftblocked && rightblocked)
                    map->grid[index] = 9 + 2 * TILESET_WIDTH;

                if(upblocked && leftblocked && downblocked)
                    map->grid[index] = 10 + 1 * TILESET_WIDTH;

                if(upblocked && downblocked)
                    map->grid[index] = 11 + 1 * TILESET_WIDTH;

                if(upblocked && rightblocked && downblocked)
                    map->grid[index] = 12 + 1 * TILESET_WIDTH;

                if(upblocked && downblocked && rightblocked && leftblocked)
                    map->grid[index] = 13 + 0 * TILESET_WIDTH;

                if(!upblocked && !downblocked && !rightblocked && !leftblocked)
                    map->grid[index] = 7 + 1 * TILESET_WIDTH;

            }
        }
    }
}

static inline
vec2 get_closest_mineable(std::vector<vec2>& minequeue, vec2 origin, i32* index) {
    vec2 result = {0};
    f32 shortest = INT_MAX;
    *index = 0;

    for(u16 i = 0; i < minequeue.size(); ++i) {
        f32 dist = getDistanceE(minequeue[i].x, minequeue[i].y, origin.x / TILE_SIZE, origin.y / TILE_SIZE);
        if(shortest > dist) {
            shortest = dist;
            result = minequeue[i];
            *index = i;
        }
    }
    return result;
}

static inline
void truncate(vec2* v, f32 max) {
    f32 i = 0;
    i = max / length(*v);
    i = i < 1.0 ? i : 1.0;
    v->x *= i;
    v->y *= i;
}

static inline
void clamp(i32* in, i32 min, i32 max) {
    if(*in > max) *in = max;
    if(*in < min) *in = min;
}

static inline
void clamp(f32* in, f32 min, f32 max) {
    if(*in > max) *in = max;
    if(*in < min) *in = min;
}

//
//   PATH STEERING
//

static inline
vec2 calculate_seek(vec2 dest, Unit* unit) {
    vec2 desired = MAX_SPEED * normalize(dest - unit->pos);
    return (desired*2) - unit->velocity;
}

static inline
vec2 calculate_seperation(Map* map, UnitList* list, Unit* unit) {
    vec2 total = {0};

    // for(u32 i = 0; i < list->size(); ++i) {
    //     Unit* a = &(*list)[i];
    //     if(a != unit) {
    //         f32 dist = getDistanceE(unit->pos.x, unit->pos.y, a->pos.x, a->pos.y);
    //         if(dist < MIN_SEPERATION && dist >= 0) {
    //             vec2 push = unit->pos - a->pos;
    //             total = total + (push/14.0f);
    //         }
    //     }
    // }

    i32 x0 = (unit->pos.x / TILE_SIZE) - 1;
    i32 x1 = (unit->pos.x / TILE_SIZE) + 1;
    i32 y0 = (unit->pos.y / TILE_SIZE) - 1;
    i32 y1 = (unit->pos.y / TILE_SIZE) + 1;

    clamp(&x0, 0, map->width);
    clamp(&x1, 0, map->width);
    clamp(&y0, 0, map->height);
    clamp(&y1, 0, map->height);

    for(u32 x = x0; x < x1; ++x) {
        for(u32 y = y0; y < y1; ++y) {
            u32 index = x + y * map->width;

            if(blocked_tile(map->grid[index])) {
                f32 dist = getDistanceE(unit->pos.x, unit->pos.y, x * TILE_SIZE, y * TILE_SIZE);
                if(dist <= TILE_SIZE-2 && dist >= 0) {
                    vec2 push = unit->pos - V2(x * TILE_SIZE, y * TILE_SIZE);
                    total = total + (push/8.5);
                }
            }
        }
    }
    return MAX_FORCE * total;
}

//
//   PATHFINDING
//

static inline
bool compare_ptr_to_node(NodePtr a, NodePtr b) {
    return (a->fcost >= b->fcost);
}

static inline
void process_successor(i32 x, i32 y, i32 fcost, NodePtr parent, std::vector<NodePtr>& open, std::vector<NodePtr>& closed) {
    if(x < 0 || y < 0) return;

    for(u16 i = 0; i < open.size(); ++i) {
        i32 nodex = open[i]->x;
        i32 nodey = open[i]->y;
        if(nodex == x && nodey == y) {
            if(open[i]->fcost <= fcost)
                return;
            break;
        }
    }

    for(u16 i = 0; i < closed.size(); ++i) {
        i32 nodex = closed[i]->x;
        i32 nodey = closed[i]->y;
        if(nodex == x && nodey == y) {
            if(closed[i]->fcost <= fcost)
                return;
            break;
        }
    }
    open.push_back(NodePtr(new Node(x, y, parent, fcost)));
}

static inline
std::vector<vec2> reconstruct_path(NodePtr current) {
    std::vector<vec2> path;
    while(current->parent != NULL) {
        path.push_back({(f32)current->x, (f32)current->y});
        current = current->parent;
    }
    path.push_back({(f32)current->x, (f32)current->y});
    return path;
}

static inline
bool jump_search_horizontal(Map* map, NodePtr current, std::vector<vec2>& successors, i32 dx, vec2 dest) {
    vec2 currpt = {(f32)current->x, (f32)current->y};

    for(;;) {
        currpt.x+=dx;
        //if reached destination
        if(currpt.x == dest.x && currpt.y == dest.y) {
            successors.push_back(currpt);
            return false;
        }
        //if a wall or edge of map reached, exit
        u32 index3 = currpt.x + currpt.y * map->width;
        if(currpt.x < 0 || currpt.x > map->width-1 || blocked_tile(map->grid[index3]))
            return true;

        //if there are forced neighbors up or down, add successor and exit
        u32 index = currpt.x + (currpt.y-1) * map->width;
        u32 index2 = currpt.x + (currpt.y+1) * map->width;
        if(currpt.y - 1 < 0 || currpt.y + 1 > map->height-1) {
            successors.push_back(currpt);
            return false;
        }
        if(blocked_tile(map->grid[index]) || blocked_tile(map->grid[index2])) {
            successors.push_back({currpt.x, currpt.y});
            return false;
        }
    }
}

static inline
bool jump_search_vertical(Map* map, NodePtr current, std::vector<vec2>& successors, i32 dy, vec2 dest) {
    vec2 currpt = {(f32)current->x, (f32)current->y};

    for(;;) {
        currpt.y+=dy;
        //if reached destination
        if(currpt.x == dest.x && currpt.y == dest.y) {
            successors.push_back(currpt);
            return false;
        }
        //if a wall or edge of map reached, exit
        u32 index3 = currpt.x + currpt.y * map->width;
        if(currpt.y < 0 || currpt.y > map->height-1 || blocked_tile(map->grid[index3]))
            return true;

        //if there are forced neighbors left or right, add successor and exit
        u32 index = (currpt.x-1) + currpt.y * map->width;
        u32 index2 = (currpt.x+1) + currpt.y * map->width;
        if(currpt.x - 1 < 0 || currpt.x + 1 > map->width-1) {
            successors.push_back(currpt);
            return false;
        }
        if(blocked_tile(map->grid[index]) || blocked_tile(map->grid[index2])) {
            successors.push_back({currpt.x, currpt.y});
            return false;
        }
    }
}

static inline
std::vector<vec2> find_successors(Map* map, NodePtr current, vec2 start, vec2 dest) {
    std::vector<vec2> successors;
    //go in all four cardinal directions, if all of them
    //hit walls, with no forced neighbors, then it is safe to traverse
    //diagonally.
    //if there are forced neighbors at any point (traversing diagonally
    //or cardinally), then add that node to the open list (successors in
    //this func) and continue with the A* processing.

    bool left_hit_wall = jump_search_horizontal(map, current, successors, -1, dest);
    bool right_hit_wall = jump_search_horizontal(map, current, successors, 1, dest);

    bool up_hit_wall = jump_search_vertical(map, current, successors, -1, dest);
    bool down_hit_wall = jump_search_vertical(map, current, successors,1, dest);

    if(right_hit_wall && up_hit_wall) {
        //SAFE to go up and right
        u32 index = (current->x + 1) + (current->y - 1) * map->width;
        if(current->x + 1 < map->width - 1 && current->y - 1 > 0 && !blocked_tile(map->grid[index]))
            successors.push_back({(f32)current->x + 1, (f32)current->y - 1});
    }

    if(up_hit_wall && left_hit_wall) {
        //SAFE to go up and left
        u32 index = (current->x - 1) + (current->y - 1) * map->width;
        if(current->x - 1 > 0 && current->y - 1 > 0 && !blocked_tile(map->grid[index]))
            successors.push_back({(f32)current->x - 1, (f32)current->y - 1});
    }

    if(left_hit_wall && down_hit_wall) {
        //SAFE to go down and left
        u32 index = (current->x - 1) + (current->y + 1) * map->width;
        if(current->x - 1 > 0 && current->y + 1 < map->height-1 && !blocked_tile(map->grid[index]))
            successors.push_back({(f32)current->x - 1, (f32)current->y + 1});
    }

    if(down_hit_wall && right_hit_wall) {
        //SAFE to go down and right
        u32 index = (current->x + 1) + (current->y + 1) * map->width;
        if(current->x + 1 < map->width-1 && current->y + 1 < map->height-1 && !blocked_tile(map->grid[index]))
            successors.push_back({(f32)current->x + 1, (f32)current->y + 1});
    }

    return successors;
}

static inline
std::vector<vec2> pathfind(Map* map, vec2 start, vec2 dest) {
    std::vector<NodePtr> open;
    std::vector<NodePtr> closed;
    NodePtr current;
    open.push_back(NodePtr(new Node(start.x, start.y, NULL, 0)));

    u32 tries = 0;
    while(!open.empty() && tries < 1000) {
        tries++;

        std::sort(open.begin(), open.end(), compare_ptr_to_node);
        current = open.back();
        open.pop_back();
        closed.push_back(current);

        if(current->x == dest.x && current->y == dest.y)
            return reconstruct_path(current);

        //now use JUMP POINT SEARCH to eliminate nodes that can be skipped, and return new nodes
        std::vector<vec2> successors = find_successors(map, current, start, dest);
        for(u32 i = 0; i < successors.size(); ++i) {
            vec2 currpt = successors[i];
            process_successor(currpt.x, currpt.y, current->fcost + getDistanceE(currpt.x, currpt.y, dest.x, dest.y), current, open, closed);
        }
    }

    std::vector<vec2> blank;
    blank.push_back(start);
    return blank;
}


static inline
std::vector<vec2> pathfind_astar(Map* map, vec2 start, vec2 dest) {
    if (start.x < 0 || start.y < 0 || start.x > map->width - 1 || start.y > map->height - 1)
        BMT_LOG(FATAL_ERROR, "vec2 start was out of bounds. start = (%f, %f)", start.x, start.y);
    if (dest.x < 0 || dest.y < 0 || dest.x > map->width - 1 || dest.y > map->height - 1)
        BMT_LOG(FATAL_ERROR, "vec2 dest was out of bounds. dest = (%f, %f)", dest.x, dest.y);

    std::vector<NodePtr> open;
    std::vector<NodePtr> closed;
    NodePtr current;
    open.push_back(NodePtr(new Node(start.x, start.y, NULL, 0)));

    u16 tries = 0;
    while (!open.empty() && tries < 650) {
        //tries++;

        std::sort(open.begin(), open.end(), compare_ptr_to_node);
        current = open.back();
        open.pop_back();
        closed.push_back(current);

        if (current->x == dest.x && current->y == dest.y) {
            return reconstruct_path(current);
        }

        //NON-DIAGONALS
        //if heuristic cost = -1 then the tile is blocked and cant be move to.
        int x = current->x - 1;
        int y = current->y;
        if (x > map->width) continue;
        if (y > map->height) continue;
        if(!blocked_tile(map->grid[x + y * map->width]))
            process_successor(x, y, current->fcost + getDistanceE(x, y, dest.x, dest.y), current, open, closed);
        else if(dest.x == x && dest.y == y)
            return reconstruct_path(current);

        x = current->x + 1;
        y = current->y;
        if (x > map->width) continue;
        if (y > map->height) continue;
        if(!blocked_tile(map->grid[x + y * map->width]))
            process_successor(x, y, current->fcost + getDistanceE(x, y, dest.x, dest.y), current, open, closed);
        else if(dest.x == x && dest.y == y)
            return reconstruct_path(current);

        x = current->x;
        y = current->y - 1;
        if (x > map->width) continue;
        if (y > map->height) continue;
        if(!blocked_tile(map->grid[x + y * map->width]))
            process_successor(x, y, current->fcost + getDistanceE(x, y, dest.x, dest.y), current, open, closed);
        else if(dest.x == x && dest.y == y)
            return reconstruct_path(current);

        x = current->x;
        y = current->y + 1;
        if (x > map->width) continue;
        if (y > map->height) continue;
        if(!blocked_tile(map->grid[x + y * map->width]))
            process_successor(x, y, current->fcost + getDistanceE(x, y, dest.x, dest.y), current, open, closed);
        else if(dest.x == x && dest.y == y)
            return reconstruct_path(current);

#ifdef DIAGONAL_ASTAR
        //DIAGONALS
        x = current->x + 1;
        y = current->y + 1;
        if (x > map->width) continue;
        if (y > map->height) continue;
        if(!blocked_tile(map->grid[x + y * map->width]))
            process_successor(x, y, current->fcost + getDistanceE(x, y, dest.x, dest.y), current, open, closed);
        else if(dest.x == x && dest.y == y)
            return reconstruct_path(current);

        x = current->x - 1;
        y = current->y - 1;
        if (x > map->width) continue;
        if (y > map->height) continue;
        if(!blocked_tile(map->grid[x + y * map->width]))
            process_successor(x, y, current->fcost + getDistanceE(x, y, dest.x, dest.y), current, open, closed);
        else if(dest.x == x && dest.y == y)
            return reconstruct_path(current);

        x = current->x - 1;
        y = current->y + 1;
        if (x > map->width) continue;
        if (y > map->height) continue;
        if(!blocked_tile(map->grid[x + y * map->width]))
            process_successor(x, y, current->fcost+ getDistanceE(x, y, dest.x, dest.y), current, open, closed);
        else if(dest.x == x && dest.y == y)
            return reconstruct_path(current);

        x = current->x + 1;
        y = current->y - 1;
        if (x > map->width) continue;
        if (y > map->height) continue;
        if(!blocked_tile(map->grid[x + y * map->width]))
            process_successor(x, y, current->fcost + getDistanceE(x, y, dest.x, dest.y), current, open, closed);
        else if(dest.x == x && dest.y == y)
            return reconstruct_path(current);
#endif

    }
    if (tries == 650) {
        return reconstruct_path(current);
    }
    std::vector<vec2> blank;
    blank.push_back(start);
    return blank;
}

//
//   MAP
//

static inline
void place_random_room(Map* map, u32 width, u32 height, vec2 origin, i32* x, i32* y) {
    *x = 0;
    *y = 0;
    do {
        *x = random_int(2, map->width-height-2);
        *y = random_int(2, map->height-width-2);
    } while(*x + width > map->width && *y + height > map->height&& getDistanceE(*x, *y, origin.x, origin.y) < 6 && getDistanceE(*x, *y, origin.x, origin.y) > 4);

    if(origin.x != -1 && origin.y != -1) {
        std::vector<vec2> path = pathfind_astar(map, {(f32)*x, (f32)*y}, origin);
        for(u32 i = 0; i < path.size(); ++i){
            u32 index = path[i].x + path[i].y * map->width;
            map->grid[index] = 0;
        }
    }

    for(u32 i = *x; i < *x + width; ++i)
        for(u32 j = *y; j < *y + height; ++j)
            map->grid[i + j * map->width] = 0;
}

static inline
Map load_random_map(i32 width, i32 height) {
    Map map = {0};

    map.width = width;
    map.height = height;
    map.grid = (i32*)malloc(sizeof(i32) * (map.width * map.height));

    for(u32 i = 0; i < width * height; ++i)
        map.grid[i] = 8;

    i32 x = 0;
    i32 y = 0;
    place_random_room(&map, 5, 5, {-1, -1}, &x, &y);
    place_random_room(&map, 8, 8, {(f32)x, (f32)y}, &x, &y);

    for(u32 i = 0; i < width*height; ++i) {
        if(map.grid[i] == 8)
            map.grid[i] = 0;
        else
            map.grid[i] = 8;
    }

    orient_tiles(&map);

    return map;
}

static inline
void draw_map(RenderBatch* batch, Map* map, Texture tileset) {
    i32 x0 = (-map->x / TILE_SIZE);
    i32 x1 = (-map->x / TILE_SIZE) + (get_virtual_width() / TILE_SIZE) + 1;
    i32 y0 = (-map->y / TILE_SIZE);
    i32 y1 = (-map->y / TILE_SIZE) + (get_virtual_height() / TILE_SIZE) + 2;

    clamp(&x0, 0, map->width);
    clamp(&x1, 0, map->width);
    clamp(&y0, 0, map->height);
    clamp(&y1, 0, map->height);

    Rect dest = {0, 0, (f32)TILE_SIZE, (f32)TILE_SIZE};
    Rect src = {0, 0, (f32)TILE_SIZE, (f32)TILE_SIZE};

    for(u32 x = x0; x < x1; ++x) {
        for(u32 y = y0; y < y1; ++y) {
            i32 id = map->grid[x + y * map->width];

            dest.x = (x * TILE_SIZE) + map->x;
            dest.y = (y * TILE_SIZE) + map->y;
            src.x = (id % TILESET_WIDTH) * TILE_SIZE;
            src.y = (id / TILESET_WIDTH) * TILE_SIZE;

            draw_texture_EX(batch, tileset, src, dest);
        }
    }
    if(is_key_down(KEY_LEFT))
        map->x += SCROLL_SPEED;
    if(is_key_down(KEY_RIGHT))
        map->x -= SCROLL_SPEED;
    if(is_key_down(KEY_DOWN))
        map->y -= SCROLL_SPEED;
    if(is_key_down(KEY_UP))
        map->y += SCROLL_SPEED;

}

static inline
void dispose_map(Map* map) {
    free(map->grid);
}

#endif
