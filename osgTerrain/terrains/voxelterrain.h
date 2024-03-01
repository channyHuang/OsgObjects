#ifndef VOXELTERRAIN_H
#define VOXELTERRAIN_H

#include <unordered_set>
#include <vector>

#include "commonMath/vector3i.h"
#include "commonMath/vector3.h"
#include "commonMath/boxi.h"

#include "voxeldataloader.h"
#include "voxelblock.h"
#include "voxeltool.h"
#include "voxeltoolterrain.h"
#include "voxelmap.h"
#include "voxels/counting_time.h"
#include "meshGenerator/voxel_mesh_updater.h"

typedef Boxi Rect3i;

struct ProfilingClock {
    uint64_t time_before = 0;

    ProfilingClock() {
        restart();
    }

    uint64_t restart() {
        uint64_t now = CountingTime::getInstance()->get_ticks_usec();
        uint64_t time_spent = now - time_before;
        time_before = CountingTime::getInstance()->get_ticks_usec();
        return time_spent;
    }
};
// Infinite paged terrain made of voxel blocks all with the same level of detail.
// Voxels are polygonized around the viewer by distance in a large cubic space.
// Data is streamed using a VoxelStream.
class VoxelTerrain {
public:
    VoxelTerrain();
    ~VoxelTerrain();

    static VoxelTerrain *getInstance() {
        if (instance == nullptr) {
            instance = new VoxelTerrain;
        }
        return instance;
    }

    unsigned int get_block_size_pow2() const;
    void set_block_size_po2(unsigned int p_block_size_po2);


    void make_block_dirty(Vector3i bpos);
    //void make_blocks_dirty(Vector3i min, Vector3i size);
    void make_voxel_dirty(Vector3i pos);
    void make_area_dirty(Rect3i box);

    void set_generate_collisions(bool enabled);
    bool get_generate_collisions() const { return _generate_collisions; }

    int get_view_distance() const;
    void set_view_distance(int distance_in_voxels);


    VoxelTool* get_voxel_tool();

    void set_run_stream_in_editor(bool enable);
    bool is_stream_running_in_editor() const;

    void restart_stream();

    struct Stats {
        VoxelMeshUpdater::Stats updater;
        int updated_blocks = 0;
        int dropped_block_loads = 0;
        int dropped_block_meshs = 0;
        uint64_t time_detect_required_blocks = 0;
        uint64_t time_request_blocks_to_load = 0;
        uint64_t time_process_load_responses = 0;
        uint64_t time_request_blocks_to_update = 0;
        uint64_t time_process_update_responses = 0;
    };

    void _notification(int p_what);

private:
    static VoxelTerrain* instance;

    void _process();

    void _on_stream_params_changed();
    void _set_block_size_po2(int p_block_size_po2);
    void make_all_view_dirty_deferred();
    void start_updater();
    void stop_updater();
    void start_streamer();
    void stop_streamer();
    void reset_map();



    void immerge_block(Vector3i bpos);
//    void save_all_modified_blocks(bool with_copy);
//    void get_viewer_pos_and_direction(Vector3 &out_pos, Vector3 &out_direction) const;
    void send_block_data_requests();

    void emit_block_loaded(const VoxelBlock *block);
    void emit_block_unloaded(const VoxelBlock *block) {};


    static void _bind_methods();

    // Bindings
    Vector3 _b_voxel_to_block(Vector3 pos);
    Vector3 _b_block_to_voxel(Vector3 pos);
    //void _force_load_blocks_binding(Vector3 center, Vector3 extents) { force_load_blocks(center, extents); }
    void _b_save_modified_blocks();
    void _b_save_block(Vector3 p_block_pos);

    // Voxel storage
    VoxelMap *_map = nullptr;

    // How many blocks to load around the viewer
    int _view_distance_blocks;

    // TODO Terrains only need to handle the visible portion of voxels, which reduces the bounds blocks to handle.
    // Therefore, could a simple grid be better to use than a hashmap?

    std::unordered_set<Vector3i, Vector3iHasher> _loading_blocks;
    std::vector<Vector3i> _blocks_pending_load;
    std::vector<Vector3i> _blocks_pending_update;
    std::vector<VoxelMeshUpdater::OutputBlock> _blocks_pending_main_thread_update;

    std::vector<VoxelDataLoader::InputBlock> _blocks_to_save;

    VoxelDataLoader *_stream_thread;
    VoxelMeshUpdater *_block_updater;

    Vector3i _last_viewer_block_pos;
    int _last_view_distance_blocks;

    bool _generate_collisions = true;
    bool _run_stream_in_editor = true;


    Stats _stats;
};



#endif // VOXELTERRAIN_H
