#include "voxelterrain.h"

#include "voxels/common_enum.h"
#include "voxels/utility.h"
const uint32_t MAIN_THREAD_MESHING_BUDGET_MS = 8;

VoxelTerrain *VoxelTerrain::instance = nullptr;

VoxelTerrain::VoxelTerrain() {
    _map = VoxelMap::getInstance();

    _view_distance_blocks = 8;
    _last_view_distance_blocks = 0;

    _block_updater = nullptr;

    const bool updater_was_running = (_block_updater != nullptr);
    stop_updater();

    if (updater_was_running) {
        start_updater();
    }

    make_all_view_dirty_deferred();
}

VoxelTerrain::~VoxelTerrain() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
    if (_block_updater != nullptr) {
        delete _block_updater;
        _block_updater = nullptr;
    }
    if (_stream_thread != nullptr) {
        delete _stream_thread;
        _stream_thread = nullptr;
    }
}

void VoxelTerrain::_set_block_size_po2(int p_block_size_po2) {
    _map->create(p_block_size_po2, 0);
}

unsigned int VoxelTerrain::get_block_size_pow2() const {
    return _map->get_block_size_pow2();
}

void VoxelTerrain::make_block_dirty(Vector3i bpos) {
    VoxelBlock *block = _map->get_block(bpos);
    if (block == nullptr) {
        if (_loading_blocks.find(bpos) == _loading_blocks.end()) {
            _blocks_pending_load.push_back(bpos);
            _loading_blocks.insert(bpos);
        }
    } else if (block->get_mesh_state() != VoxelBlock::MESH_UPDATE_NOT_SENT) {
        block->set_mesh_state(VoxelBlock::MESH_UPDATE_NOT_SENT);
        _blocks_pending_update.push_back(bpos);

        if (!block->is_modified()) {
            block->set_modified(true);
        }
    }
}


//void VoxelTerrain::make_blocks_dirty(Vector3i min, Vector3i size) {
//	Vector3i max = min + size;
//	Vector3i pos;
//	for (pos.z = min.z; pos.z < max.z; ++pos.z) {
//		for (pos.y = min.y; pos.y < max.y; ++pos.y) {
//			for (pos.x = min.x; pos.x < max.x; ++pos.x) {
//				make_block_dirty(pos);
//			}
//		}
//	}
//}

void VoxelTerrain::make_all_view_dirty_deferred() {
    _last_view_distance_blocks = 0;
}

void VoxelTerrain::start_updater() {
    // set mesh generation params
    VoxelMeshUpdater::MeshingParams params;
    _block_updater = new VoxelMeshUpdater(1, params);
}

void VoxelTerrain::stop_updater() {
    struct ResetMeshStateAction {
        void operator()(VoxelBlock *block) {
            if (block->get_mesh_state() == VoxelBlock::MESH_UPDATE_SENT) {
                block->set_mesh_state(VoxelBlock::MESH_UPDATE_NOT_SENT);
            }
        }
    };

    if (_block_updater != nullptr) {
        delete(_block_updater);
        _block_updater = nullptr;
    }

    _blocks_pending_main_thread_update.clear();
    _blocks_pending_update.clear();

    ResetMeshStateAction a;
    _map->for_all_blocks(a);
}

void VoxelTerrain::start_streamer() {
    _stream_thread = new VoxelDataLoader(1, get_block_size_pow2());
}

void VoxelTerrain::stop_streamer() {
    if (_stream_thread) {
        delete(_stream_thread);
        _stream_thread = nullptr;
    }
    _loading_blocks.clear();
    _blocks_pending_load.clear();
}


void VoxelTerrain::reset_map() {
    // Don't reset while streaming, the result can be dirty

    _map->for_all_blocks([this](VoxelBlock *block) {
        emit_block_unloaded(block);
    });
    _map->create(get_block_size_pow2(), 0);

    // To force queries to happen again, because we only listen for viewer position changes
    make_all_view_dirty_deferred();
}

namespace {
struct ScheduleSaveAction {
    std::vector<VoxelDataLoader::InputBlock> &blocks_to_save;
    bool with_copy;

    void operator()(VoxelBlock *block) {
        if (block->is_modified()) {
            VoxelDataLoader::InputBlock b;
            // to do
            b.data.voxels_to_save = /*with_copy ? block->voxels->duplicate() :*/ block->voxels;
            b.position = block->position;
            b.can_be_discarded = false;
            blocks_to_save.push_back(b);
            block->set_modified(false);
        }
    }
};
} // namespace

void VoxelTerrain::immerge_block(Vector3i bpos) {
    _map->remove_block(bpos, [this, bpos](VoxelBlock *block) {
        emit_block_unloaded(block);
        ScheduleSaveAction{ _blocks_to_save, false }(block);
    });

    _loading_blocks.erase(bpos);
}

inline int get_border_index(int x, int max) {
    return x == 0 ? 0 : x != max ? 1 : 2;
}

void VoxelTerrain::make_voxel_dirty(Vector3i pos) {
    // Update the block in which the voxel is
    const Vector3i bpos = _map->voxel_to_block(pos);
    make_block_dirty(bpos);
    //OS::get_singleton()->print("Dirty (%i, %i, %i)\n", bpos.x, bpos.y, bpos.z);

    // Update neighbor blocks if the voxel is touching a boundary

    const Vector3i rpos = _map->to_local(pos);

    // TODO Thread-safe way of getting this parameter
    const bool check_corners = true; //_mesher->get_occlusion_enabled();

    const int max = _map->get_block_size() - 1;

    if (rpos.x == 0) {
        make_block_dirty(bpos - Vector3i(1, 0, 0));
    } else if (rpos.x == max) {
        make_block_dirty(bpos + Vector3i(1, 0, 0));
    }

    if (rpos.y == 0) {
        make_block_dirty(bpos - Vector3i(0, 1, 0));
    } else if (rpos.y == max) {
        make_block_dirty(bpos + Vector3i(0, 1, 0));
    }

    if (rpos.z == 0) {
        make_block_dirty(bpos - Vector3i(0, 0, 1));
    } else if (rpos.z == max) {
        make_block_dirty(bpos + Vector3i(0, 0, 1));
    }

    // We might want to update blocks in corners in order to update ambient occlusion
    if (check_corners) {

        //       24------25------26
        //       /|              /|
        //      / |             / |
        //    21  |           23  |
        //    /  15           /  17
        //   /    |          /    |
        // 18------19------20     |
        //  |     |         |     |
        //  |     6-------7-|-----8
        //  |    /          |    /
        //  9   /          11   /
        //  |  3            |  5
        //  | /             | /      y z
        //  |/              |/       |/
        //  0-------1-------2        o--x

        // I'm not good at writing piles of ifs

        static const int normals[27][3] = {
            { -1, -1, -1 }, { 0, -1, -1 }, { 1, -1, -1 },
            { -1, -1, 0 }, { 0, -1, 0 }, { 1, -1, 0 },
            { -1, -1, 1 }, { 0, -1, 1 }, { 1, -1, 1 },

            { -1, 0, -1 }, { 0, 0, -1 }, { 1, 0, -1 },
            { -1, 0, 0 }, { 0, 0, 0 }, { 1, 0, 0 },
            { -1, 0, 1 }, { 0, 0, 1 }, { 1, 0, 1 },

            { -1, 1, -1 }, { 0, 1, -1 }, { 1, 1, -1 },
            { -1, 1, 0 }, { 0, 1, 0 }, { 1, 1, 0 },
            { -1, 1, 1 }, { 0, 1, 1 }, { 1, 1, 1 }
        };
        static const int ce_counts[27] = {
            4, 1, 4,
            1, 0, 1,
            4, 1, 4,

            1, 0, 1,
            0, 0, 0,
            1, 0, 1,

            4, 1, 4,
            1, 0, 1,
            4, 1, 4
        };
        static const int ce_indexes_lut[27][4] = {
            { 0, 1, 3, 9 }, { 1 }, { 2, 1, 5, 11 },
            { 3 }, {}, { 5 },
            { 6, 3, 7, 15 }, { 7 }, { 8, 7, 5, 17 },

            { 9 }, {}, { 11 },
            {}, {}, {},
            { 15 }, {}, { 17 },

            { 18, 9, 19, 21 }, { 19 }, { 20, 11, 19, 23 },
            { 21 }, {}, { 23 },
            { 24, 15, 21, 25 }, { 25 }, { 26, 17, 23, 25 }
        };

        const int m = get_border_index(rpos.x, max) + 3 * get_border_index(rpos.z, max) + 9 * get_border_index(rpos.y, max);

        const int *ce_indexes = ce_indexes_lut[m];
        const int ce_count = ce_counts[m];
        //OS::get_singleton()->print("m=%i, rpos=(%i, %i, %i)\n", m, rpos.x, rpos.y, rpos.z);

        for (int i = 0; i < ce_count; ++i) {
            // TODO Because it's about ambient occlusion across 1 voxel only,
            // we could optimize it even more by looking at neighbor voxels,
            // and discard the update if we know it won't change anything
            const int *normal = normals[ce_indexes[i]];
            const Vector3i nbpos(bpos.x + normal[0], bpos.y + normal[1], bpos.z + normal[2]);
            //OS::get_singleton()->print("Corner dirty (%i, %i, %i)\n", nbpos.x, nbpos.y, nbpos.z);
            make_block_dirty(nbpos);
        }
    }
}

void VoxelTerrain::make_area_dirty(Rect3i box) {
    Vector3i min_pos = box.vMin;
    Vector3i max_pos = box.vMax - Vector3i(1, 1, 1);

    const bool check_corners = true;

    if (check_corners) {
        min_pos -= Vector3i(1, 1, 1);
        max_pos += Vector3i(1, 1, 1);

    } else {
        Vector3i min_rpos = _map->to_local(min_pos);
        if (min_rpos.x == 0) {
            --min_pos.x;
        }
        if (min_rpos.y == 0) {
            --min_pos.y;
        }
        if (min_rpos.z == 0) {
            --min_pos.z;
        }

        const int max = _map->get_block_size() - 1;
        const Vector3i max_rpos = _map->to_local(max_pos);
        if (max_rpos.x == max) {
            ++max_pos.x;
        }
        if (max_rpos.y == max) {
            ++max_pos.y;
        }
        if (max_rpos.z == max) {
            ++max_pos.z;
        }
    }

    const Vector3i min_block_pos = _map->voxel_to_block(min_pos);
    const Vector3i max_block_pos = _map->voxel_to_block(max_pos);

    Vector3i bpos;
    for (bpos.z = min_block_pos.z; bpos.z <= max_block_pos.z; ++bpos.z) {
        for (bpos.x = min_block_pos.x; bpos.x <= max_block_pos.x; ++bpos.x) {
            for (bpos.y = min_block_pos.y; bpos.y <= max_block_pos.y; ++bpos.y) {
                make_block_dirty(bpos);
            }
        }
    }
}


static void remove_positions_outside_box(
        std::vector<Vector3i> &positions,
        Rect3i box,
        std::unordered_set<Vector3i, Vector3iHasher> &loading_set) {

    for (int i = 0; i < positions.size(); ++i) {
        const Vector3i bpos = positions[i];
        if (!box.contains(bpos)) {
            const int last = positions.size() - 1;
            positions[i] = positions[last];
            positions.resize(last);
            loading_set.erase(bpos);
            --i;
        }
    }
}

void VoxelTerrain::send_block_data_requests() {
    VoxelDataLoader::Input input;

    Vector3 viewer_pos;
    //get_viewer_pos_and_direction(viewer_pos, input.priority_direction);
    input.priority_position = _map->voxel_to_block(Vector3i(viewer_pos));

    for (int i = 0; i < _blocks_pending_load.size(); ++i) {
        VoxelDataLoader::InputBlock input_block;
        input_block.position = _blocks_pending_load[i];
        input_block.lod = 0;
        input.blocks.push_back(input_block);
    }

    for (unsigned int i = 0; i < _blocks_to_save.size(); ++i) {
        input.blocks.push_back(_blocks_to_save[i]);
    }

    //print_line(String("Sending {0} block requests").format(varray(input.blocks_to_emerge.size())));
    _blocks_pending_load.clear();
    _blocks_to_save.clear();

}

VoxelTool* VoxelTerrain::get_voxel_tool() {
    VoxelTool *tool = new VoxelToolTerrain(this, _map);
    return tool;
}

void VoxelTerrain::_notification(int p_what) {
    switch (p_what) {
    case Notification_Enter:
        if (_block_updater == nullptr) {
            start_updater();
        }
        if (_stream_thread == nullptr) {
            start_streamer();
        }
        break;
    case Notification_Process:
        _process();
        break;
    case Notification_Exit:
        stop_updater();
        stop_streamer();
        default:
            break;
    }
}

void VoxelTerrain::_process() {
    ProfilingClock profiling_clock;

    _stats.dropped_block_loads = 0;
    _stats.dropped_block_meshs = 0;

    Vector3 viewer_pos;
    Vector3 viewer_direction;
    // to do
    //get_viewer_pos_and_direction(viewer_pos, viewer_direction);
    Vector3i viewer_block_pos = _map->voxel_to_block(Vector3i(viewer_pos));

    {
        Rect3i new_box = Rect3i::from_center_extents(viewer_block_pos, Vector3i(_view_distance_blocks));
        Rect3i prev_box = Rect3i::from_center_extents(_last_viewer_block_pos, Vector3i(_last_view_distance_blocks));

        if (prev_box != new_box) {
            prev_box.difference(new_box, [this](Rect3i out_of_range_box) {
                out_of_range_box.for_each_cell([=](Vector3i bpos) {
                    immerge_block(bpos);
                });
            });

            new_box.difference(prev_box, [this](Rect3i box_to_load) {
                box_to_load.for_each_cell([=](Vector3i bpos) {
                    make_block_dirty(bpos);
                });
            });
        }

        remove_positions_outside_box(_blocks_pending_load, new_box, _loading_blocks);
        remove_positions_outside_box(_blocks_pending_update, new_box, _loading_blocks);
    }

    _stats.time_detect_required_blocks = profiling_clock.restart();

    _last_view_distance_blocks = _view_distance_blocks;
    _last_viewer_block_pos = viewer_block_pos;

    // to do

    _stats.time_request_blocks_to_load = profiling_clock.restart();

    // to do

    _stats.time_process_load_responses = profiling_clock.restart();

    {
        VoxelMeshUpdater::Input input;
        input.priority_position = viewer_block_pos;
        input.priority_direction = viewer_direction;

        for (int i = 0; i < _blocks_pending_update.size(); ++i) {
            Vector3i block_pos = _blocks_pending_update[i];

            // to do

            VoxelBlock *block = _map->get_block(block_pos);

            VoxelBuffer* nbuffer = new VoxelBuffer;

            unsigned int block_size = _map->get_block_size();
            unsigned int min_padding = _block_updater->get_minimum_padding();
            unsigned int max_padding = _block_updater->get_maximum_padding();
            nbuffer->create(Vector3i(block_size + min_padding + max_padding));

            unsigned int channels_mask = (1 << VoxelBuffer::CHANNEL_TYPE) | (1 << VoxelBuffer::CHANNEL_SDF);
            _map->get_buffer_copy(_map->block_to_voxel(block_pos) - Vector3i(min_padding), *nbuffer, channels_mask);

            VoxelMeshUpdater::InputBlock iblock;
            iblock.data.voxels = nbuffer;
            iblock.position = block_pos;
            input.blocks.push_back(iblock);

            block->set_mesh_state(VoxelBlock::MESH_UPDATE_SENT);
        }

        _block_updater->push(input);

        _blocks_pending_update.clear();
    }

    _stats.time_request_blocks_to_update = profiling_clock.restart();

    {
        {
            VoxelMeshUpdater::Output output;
            _block_updater->pop(output);

            _stats.updater = output.stats;
            _stats.updated_blocks = output.blocks.size();

            _blocks_pending_main_thread_update.insert(_blocks_pending_main_thread_update.end(), output.blocks.begin(), output.blocks.end());
        }

        const uint32_t timeout = CountingTime::get_ticks_msec() + MAIN_THREAD_MESHING_BUDGET_MS;
        int queue_index = 0;

        for (; queue_index < _blocks_pending_main_thread_update.size() && CountingTime::getInstance()->get_ticks_msec() < timeout; ++queue_index) {
            const VoxelMeshUpdater::OutputBlock &ob = _blocks_pending_main_thread_update[queue_index];

            VoxelBlock *block = _map->get_block(ob.position);
            if (block == nullptr) {
                ++_stats.dropped_block_meshs;
                continue;
            }

            if (ob.drop_hint) {
                ++_stats.dropped_block_meshs;
                continue;
            }
            // to do


            int surface_index = 0;
            const VoxelMeshUpdater::OutputBlockData &data = ob.data;

            for (int i = 0; i < data.smooth_surfaces.surfaces.size(); ++i) {

                Arrays surface = data.smooth_surfaces.surfaces[i];
                if (surface.empty()) {
                    continue;
                }

                ++surface_index;
            }

            // to do
            //block->set_mesh(mesh, this, _generate_collisions, collidable_surfaces, get_tree()->is_debugging_collisions_hint());

            //block->set_parent_visible(is_visible());
        }

        shift_up(_blocks_pending_main_thread_update, queue_index);
    }

    _stats.time_process_update_responses = profiling_clock.restart();
}

Vector3 VoxelTerrain::_b_voxel_to_block(Vector3 pos) {
    return Vector3i(_map->voxel_to_block(pos)).to_vec3();
}

Vector3 VoxelTerrain::_b_block_to_voxel(Vector3 pos) {
    return Vector3i(_map->block_to_voxel(pos)).to_vec3();
}

void VoxelTerrain::_b_save_modified_blocks() {
    //save_all_modified_blocks(true);
}

