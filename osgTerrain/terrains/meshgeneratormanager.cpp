#include "meshgeneratormanager.h"

#include "generator/naiveSurfaceNets/voxel_mesher_surfacenet.h"

MeshGeneratorManager::MeshGeneratorManager() {
    pool.setMaxThread(8);
}

MeshGeneratorManager::~MeshGeneratorManager() {}

void MeshGeneratorManager::process() {
}

void MeshGeneratorManager::sltFinish(OutputBlock output) {
    std::unique_lock<std::mutex> lock(m_mutexOutput);
    outputBlocks.blocks.push_back(output);
}

void MeshGeneratorManager::push(const Input& input) {
    spdlog::get("Terrain")->debug("push {} to mesh generator", input.blocks.size());
    // printf("push %u to mesh generator\n", input.blocks.size());
    for (int i = 0; i < input.blocks.size(); ++i) {
        pool.enqueue([input, this, i]() {
            OutputBlock output;
            MeshInput in = { 0, input.blocks[i].position, input.blocks[i].voxels };
            output.position = input.blocks[i].position;
            std::shared_ptr<VoxelMesherSurfaceNets> pMeshGenerator = std::make_shared<VoxelMesherSurfaceNets>();
            pMeshGenerator->build(output.smooth_surfaces, in);

            if (output.smooth_surfaces.surfaces.size() > 0) {
                this->sigFinish(output);
            }
            else {
                // printf("no mesh  %s \n", output.position.toString().c_str() );
            }
        });
    }
}

void MeshGeneratorManager::pop(Output &output) {
    std::unique_lock<std::mutex> lock(m_mutexOutput);
    output.blocks.swap(outputBlocks.blocks);
    outputBlocks.blocks.clear();
}
