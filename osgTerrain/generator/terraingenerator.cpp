#include "terraingenerator.h"

TerrainGenerator::TerrainGenerator()
{

}

TerrainGenerator::~TerrainGenerator() {}

void TerrainGenerator::generateTerrain(VoxelBuffer &buffer, TerrainType eTerrainType) {
    Vector3i vBufferSize = buffer.get_size();
    Vector3 vScale = 3.0 / vBufferSize.to_vec3();
    Box box(Vector3(5), Vector3(vBufferSize.x - 10, vBufferSize.y >> 1, vBufferSize.z - 10));
    Vector3 vpos, derivs;
    switch (eTerrainType) {
    case Terrain_Moutain:
    {
        float amplitude = 1.f, frequency = 2.f;
        for (unsigned int i = 0; i < vBufferSize.x; ++i) {
            for (unsigned int k = 0; k < vBufferSize.z; ++k) {
                Vector3 p(i * vScale.x, 0, k * vScale.z);
                float value = noise_lite.fractal(p, derivs, 7, amplitude, frequency);
                float h = (value + 1.0f) * 0.5f * vBufferSize.y;
                for (unsigned int j = 0; j < vBufferSize.y; ++j) {
                    buffer.set_voxel_f(j - h, i, j, k, VoxelBuffer::CHANNEL_SDF);
                }
            }
        }
        break;
    }
    case Terrain_Flat:
    default:
        for (unsigned int i = 0; i < vBufferSize.x; ++i) {
            for (unsigned int j = 0; j < vBufferSize.y; ++j) {
                for (unsigned int k = 0; k < vBufferSize.z; ++k) {
                    vpos = Vector3(i, j, k);
                    float sdf =  MathFuncs::minDistToCube(vpos, box);
                    buffer.set_voxel_f(sdf, i, j, k, VoxelBuffer::CHANNEL_SDF);
                }
            }
        }
        break;
    }
}

void test()
{
    std::shared_ptr<VoxelBuffer> buffer = std::make_shared<VoxelBuffer>();
    Vector3i vSize = Vector3i(100);
    buffer->create(100, 100, 100);

    TerrainGenerator generator;
    generator.generateTerrain(*buffer.get());

    auto const sdfFunction = [&](float x, float y, float z) -> float {
        return buffer->get_voxel_f(x, y, z, VoxelBuffer::CHANNEL_SDF);
    };

//    SurfaceNets surfaceNets;
//    TriMesh mesh = surfaceNets.surfaceNets(sdfFunction, vSize);

//    output2File(mesh);
}
