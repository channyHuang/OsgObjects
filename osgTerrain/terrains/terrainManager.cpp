#include "terrainManager.h"

#include "voxels/common_enum.h"
#include "generator/voxel_mesher.h"

TerrainManager *TerrainManager::instance = nullptr;

TerrainManager::TerrainManager() {
    _map = VoxelMap::getInstance();
    _blocks_pending_update.clear();
    stop_updater();
}

TerrainManager::~TerrainManager() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
    if (_block_updater != nullptr) {
        delete _block_updater;
        _block_updater = nullptr;
    }
}

void TerrainManager::make_block_dirty(const Vector3i& bpos) {
    VoxelBlock *block = _map->get_block(bpos);
    if (block == nullptr) {
    } else if (block->get_mesh_state() != VoxelBlock::MESH_NEED_UPDATE) {
        block->set_mesh_state(VoxelBlock::MESH_NEED_UPDATE);
        _blocks_pending_update.insert(bpos);
    }
}

void TerrainManager::make_voxel_dirty(const Vector3i& pos) {
    const Vector3i bpos = _map->voxel_to_block(pos);
    make_block_dirty(bpos);
}

void TerrainManager::make_area_dirty(const Boxi& box) {
    const Vector3i min_block_pos = _map->voxel_to_block(box.vMin);
    const Vector3i max_block_pos = _map->voxel_to_block(box.vMax);

    Vector3i bpos;
    for (bpos.z = min_block_pos.z; bpos.z <= max_block_pos.z; ++bpos.z) {
        for (bpos.x = min_block_pos.x; bpos.x <= max_block_pos.x; ++bpos.x) {
            for (bpos.y = min_block_pos.y; bpos.y <= max_block_pos.y; ++bpos.y) {
                make_block_dirty(bpos);
            }
        }
    }
}

void TerrainManager::_notification(Notification_Event p_what) {
    switch (p_what) {
    case Notification_Enter:
        if (_block_updater == nullptr) {
            start_updater();
        }
        break;
    case Notification_Process:
        _process();
        break;
    case Notification_Exit:
        stop_updater();
        default:
            break;
    }
}

void TerrainManager::_process() {
    // send update request
#ifdef DEBUG_INFO
    if (_blocks_pending_update.size()) 
        std::cout << __FUNCTION__ << " in " << _blocks_pending_update.size() << std::endl;
#endif
    {
        Input input;

        for (auto itr = _blocks_pending_update.begin(); itr != _blocks_pending_update.end(); itr++) {
            Vector3i block_pos = (*itr);
            VoxelBlock *block = _map->get_block(block_pos);

            std::shared_ptr<VoxelBuffer> nbuffer = std::make_shared<VoxelBuffer>();

            unsigned int block_size = _map->get_block_size();
            unsigned int min_padding = _block_updater->get_minimum_padding();
            unsigned int max_padding = _block_updater->get_maximum_padding();
            nbuffer->create(Vector3i(block_size + min_padding + max_padding));

            unsigned int channels_mask = (1 << VoxelBuffer::CHANNEL_TYPE) | (1 << VoxelBuffer::CHANNEL_SDF);
            _map->get_buffer_copy(_map->block_to_voxel(block_pos) - Vector3i(min_padding), nbuffer, channels_mask);

            InputBlock iblock;
            iblock.voxels = nbuffer;
            iblock.position = block_pos;
            input.blocks.push_back(iblock);

            block->set_mesh_state(VoxelBlock::MESH_UPDATE_SENT);
        }
        if (!input.is_empty()) {
            _block_updater->push(input);
        }
        _blocks_pending_update.clear();
    }

    // receive updated mesh
    {

        Output output;
        _block_updater->pop(output);

#ifdef DEBUG_INFO
        if (output.blocks.size())
            std::cout << __FUNCTION__ << " out " << output.blocks.size() << std::endl;
#endif
        sets.clear();
        for (int i = 0; i < output.blocks.size(); ++i) {
            const OutputBlock &data = output.blocks[i];
            for (int j = 0; j < data.smooth_surfaces.surfaces.size(); ++j) {

                Arrays surface = data.smooth_surfaces.surfaces[j];
                if (surface.empty()) {
                    continue;
                }

                sigGenerateMeshSuc(surface, output.blocks[i].position);

                auto itr = sets.find(output.blocks[i].position);
                if (itr == sets.end()) sets.insert(output.blocks[i].position);
                else std::cout << __FUNCTION__ << " out replete " << output.blocks[i].position.toString().c_str() << std::endl;

                break;
            }
        }
    }
}

void TerrainManager::start_updater() {
    //VoxelMeshUpdater::MeshingParams params;
    //_block_updater = new VoxelMeshUpdater(1, params);
    _block_updater = new MeshGeneratorManager();
    _block_updater->sigFinish.connect(_block_updater, &MeshGeneratorManager::sltFinish);
}

void TerrainManager::stop_updater() {
    if (_block_updater != nullptr) {
        delete(_block_updater);
        _block_updater = nullptr;
    }

    _blocks_pending_update.clear();
    _map->for_all_blocks([=](VoxelBlock *block) {
        if (block->get_mesh_state() == VoxelBlock::MESH_UPDATE_SENT) {
            block->set_mesh_state(VoxelBlock::MESH_NEED_UPDATE);
        }
    });
}
