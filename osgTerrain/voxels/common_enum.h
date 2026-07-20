#pragma once

#include <stdint.h>

enum Notification_Event {
    Notification_Enter = 1,
    Notification_Exit,
    Notification_Process
};

enum MaterialType : uint32_t {
    AIR = 0,
    BASALT,
    BRICK,
    GRASS,
    GRASSLAND,
    ICE,
    LAVA,
    MUD,
    ROAD,
    ROCK,
    SAND,
    SANDSTONE,
    SNOW,
    SOIL,
    STONE,
    WATER,
    WOOD,
    Material_MAX
};

enum TerrainBrushType : int {
    BrushSphere = 0,
    BrushSquare
};

enum TerrainModifyType : int {
    Modify_Add = 0,
    Modify_Reduce,
    Modify_Flat,
    Modify_Replace,
    Modify_Increase,
    Modify_Decrease
};
