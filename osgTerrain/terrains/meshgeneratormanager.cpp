#include "meshgeneratormanager.h"

MeshGeneratorManager::MeshGeneratorManager()
{
    pool.setMaxThread(8);
}

MeshGeneratorManager::~MeshGeneratorManager() {
}

void MeshGeneratorManager::process() {
}

void MeshGeneratorManager::sltFinish(OutputBlock output) {
    std::unique_lock<std::mutex> lock(mutex);
    outputBlocks.blocks.push_back(output);
}

void MeshGeneratorManager::push(const Input& input) {
    std::cout << __FUNCTION__ << " " << input.blocks.size() << std::endl;
    for (int i = 0; i < input.blocks.size(); ++i) {
        pool.enqueue([input, this, i]() {
            OutputBlock output;
            MeshInput in = { input.blocks[i].voxels, 0, input.blocks[i].position };
            output.position = input.blocks[i].position;
            VoxelMesherSurfaceNets* mesher = new VoxelMesherSurfaceNets;
            mesher->build(output.smooth_surfaces, in);

            if (output.smooth_surfaces.surfaces.size() > 0)
                this->sigFinish(output);
            else {
                //std::cout << "no mesh  " << output.position.toString().c_str() << std::endl;
            }
            });
    }
}

void MeshGeneratorManager::pop(Output &output) {
    output.blocks.swap(outputBlocks.blocks);
    outputBlocks.blocks.clear();
}
