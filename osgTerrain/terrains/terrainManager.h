#pragma once

#include <unordered_set>
#include <iostream>

#include "commonMath/vector3i.h"
#include "commonMath/boxi.h"

#include "voxelmap.h"
#include "meshgeneratormanager.h"
#include "voxels/common_enum.h"

// manage the whole terrain -- update
class TerrainManager {
public:
    TerrainManager();
    ~TerrainManager();

    static TerrainManager *getInstance() {
        if (m_pInstance == nullptr) {
            m_pInstance = std::make_unique<TerrainManager>();
        }
        return m_pInstance.get();
    }

    void make_block_dirty(const Vector3i& bpos);
    void make_voxel_dirty(const Vector3i& pos);
    void make_area_dirty(const Boxi& box);

// signals
    SignalSlot::Signal<void(Arrays, Vector3i)> sigGenerateMeshSuc;

// slots
    void _notification(Notification_Event p_what);

private:
    void _process();
    void start_updater();
    void stop_updater();

    
private:
    static std::unique_ptr<TerrainManager> m_pInstance;
    std::mutex m_mutexUpdate;

    VoxelMap *_map = nullptr;
    MeshGeneratorManager *_block_updater = nullptr;
    std::unordered_set<Vector3i, Vector3iHash> _blocks_pending_update;
    std::vector<OutputBlock> _blocks_pending_main_thread_update;
    Vector3 viewer_pos;
    Vector3 viewer_direction;
};
