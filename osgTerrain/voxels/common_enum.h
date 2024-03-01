#ifndef COMMON_ENUM_H
#define COMMON_ENUM_H

enum Notification_Event {
    Notification_Enter = 1,
    Notification_Exit,
    Notification_Process
};

enum MaterialType {
    AIR = 0,
    BASALT,
    BRICK,
    GRASS,
    GRASSLAND,
    ICE,
    LAVA,
    MUD,
    ROCK,
    SAND,
    SANDSTONE,
    SNOW,
    SOIL,
    STONE,
    WATER,
    WOOD,
    ROAD,
    Material_MAX
};

enum TerrainBrushType {
    BrushSphere,
    BrushSquare
};

enum TerrainModifyType {
    Modify_Add,
    Modify_Reduce,
    Modify_Flat,
    Modify_Replace,
    Modify_Increase,
    Modify_Decrease
};

#endif // COMMON_ENUM_H
