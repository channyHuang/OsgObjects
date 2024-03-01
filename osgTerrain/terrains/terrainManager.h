#ifndef TERRAINMANAGER_H
#define TERRAINMANAGER_H

#include <unordered_set>
#include <iostream>

#include "commonMath/vector3i.h"
#include "commonMath/boxi.h"
#include "voxelmap.h"
#include "meshgeneratormanager.h"

class TerrainManager {
public:

    ~TerrainManager();

    static TerrainManager *getInstance() {
        if (instance == nullptr) {
            instance = new TerrainManager;
        }
        return instance;
    }

    void make_block_dirty(const Vector3i& bpos);
    void make_voxel_dirty(const Vector3i& pos);
    void make_area_dirty(const Boxi& box);

// signals
    SignalSlot::Signal<void(Arrays, Vector3i)> sigGenerateMeshSuc;

// slots
    void _notification(Notification_Event p_what);

private:
    TerrainManager();

    void _process();
    void start_updater();
    void stop_updater();

    static TerrainManager* instance;
    VoxelMap *_map = nullptr;
    MeshGeneratorManager *_block_updater = nullptr;
    std::unordered_set<Vector3i, Vector3iHash> _blocks_pending_update;
    std::vector<OutputBlock> _blocks_pending_main_thread_update;
    Vector3 viewer_pos;
    Vector3 viewer_direction;

    std::unordered_set<Vector3i, Vector3iHash> sets;
};

#endif
