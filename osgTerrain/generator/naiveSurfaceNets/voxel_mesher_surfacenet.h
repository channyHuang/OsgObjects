#pragma  once

#include <array>
#include <vector>
#include <fstream>

#include "../voxel_mesher.h"
#include "voxels/voxelBuffer.h"
#include "SurfaceNets.h"
using namespace SURFACE_NETS;
    class VoxelMesherSurfaceNets : public VoxelMesher
    {

    public:
        static const int MIN_PADDING = 2;
        static const int MAX_PADDING = 2;
        static const uint8_t _lod_count = 4;

        VoxelMesherSurfaceNets();

        void build(MeshOutput &output, const MeshInput &input) override;

        VoxelMesher *clone() override;
    private:
        struct ReuseCell
        {
            std::array<int, 4> vertices;
        };

        struct ReuseTransitionCell
        {
            std::array<int, 12> vertices;
        };

        void build_internal(MeshOutput &output, std::shared_ptr<VoxelBuffer> voxels, unsigned int channel, int lod_index,
            const Vector3i &position = Vector3i(0));
        int emit_vertex(const Vector3& primary, const Vector3& normal, uint16_t border_mask,
                        const Vector3& secondary, uint32_t material = 0);
        void clear_output();
        void fill_surface_arrays(Arrays &arrays);
        void vertexMesh2outputArrays(
            std::shared_ptr<VertexMesh> pmesh,
            MeshOutput &output,
            const Vector3i &block_size_scaled,
            uint8_t cell_border_mask = 0,
            const Vector3i &position = Vector3i(0));
        //with water
        void build_internal_with_water(MeshOutput &output, std::shared_ptr<VoxelBuffer> voxels, unsigned int channel, int lod_index,
            const Vector3i &position = Vector3i(0));
        void vertexMesh2OutputArrays_with_water(
            std::shared_ptr<VertexMesh> pmesh,
            MeshOutput &output,
            const Vector3i &block_size_scaled,
            uint8_t cell_border_mask = 0,
            const Vector3i &position = Vector3i(0));
        void outputToObjFile(Arrays &singleArray, const Vector3i &position, bool bIsWater = false);
        bool calcUvs(Arrays& singleArray);
    private:
        std::array<std::vector<ReuseCell>, 2> _cache;
        std::array<std::vector<ReuseTransitionCell>, 2> _cache_2d;
        Vector3i _block_size;

        std::vector<Vector3> _output_vertices;
        std::vector<Vector3> _output_normals;
        std::vector<uint32_t> _output_indices;
        std::vector<uint32_t> _output_materials;
        bool _is_water = false;
    };
