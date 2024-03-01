#include "voxel_mesher_surfacenet.h"

#include "xatlas/xatlas.h"
//#define DEBUG_SINGLE_BUILD_TIME
//#define DEBUG_SINGLE_BUILD_MESHS
    namespace {
        static const float TRANSITION_CELL_SCALE = 0.25;
        static const bool MESH_SHARE_POINTS = true;
        static const bool BMESH_SUPPORT_WATER = true;

        Vector3 get_border_offset(const Vector3 &pos, const int lod_index, const Vector3i &block_size)
        {
            Vector3 delta;

            const float p2k = 1.0f * (1 << lod_index); // 2 ^ lod
            const float p2mk = 1.f / p2k;     // 2 ^ (-lod)

            const float wk = TRANSITION_CELL_SCALE * p2k; // 2 ^ (lod - 2), if scale is 0.25

            for (unsigned int i = 0; i < 3; ++i)
            {

                const float p = pos[i];
                const float s = static_cast<float>(block_size[i]);

                if (p < p2k)
                {
                    // The vertex is inside the minimum cell.
                    delta[i] = (1.0f - p2mk * p) * wk;
                }
                else if (p > (p2k * (s - 1)))
                {
                    // The vertex is inside the maximum cell.
                    delta[i] = ((p2k * s) - 1.0f - p) * wk;
                }
            }

            return delta;
        }

        inline Vector3 project_border_offset(const Vector3 &delta, const Vector3 &normal)
        {
            return Vector3(
                (1 - normal.x * normal.x) * delta.x /**/ - normal.y * normal.x * delta.y /*     */ - normal.z * normal.x * delta.z,
                /**/ -normal.x * normal.y * delta.x + (1 - normal.y * normal.y) * delta.y /*    */ - normal.z * normal.y * delta.z,
                /**/ -normal.x * normal.z * delta.x /**/ - normal.y * normal.z * delta.y /**/ + (1 - normal.z * normal.z) * delta.z);
        }

        inline Vector3 get_secondary_position(const Vector3 &primary, const Vector3 &normal, const int lod_index, const Vector3i &block_size)
        {
            Vector3 delta = get_border_offset(primary, lod_index, block_size);
            delta = project_border_offset(delta, normal);
            return primary + delta;
        }

        inline float get_normal_angle(const Vector3 &normal1, const Vector3 &normal2) {
            if (normal1 == Vector3(0) || normal2 == Vector3(0)) return 0.f;
            float cos_theta = normal1.dot(normal2); // / (normal1.len() * normal2.len());
            return acos(cos_theta); //[0,pi]
        }
    }

    VoxelMesherSurfaceNets::VoxelMesherSurfaceNets()
    {
        set_padding(MIN_PADDING, MAX_PADDING);
    }

    void VoxelMesherSurfaceNets::build(MeshOutput &output, const MeshInput &input)
    {
        //corresponding to VoxelDataMap._block_size_pow2, which is 4 currently, if input.lod == 4, return all level
        if (input.lod > _lod_count || input.lod < 0) {
            return;
        }
#ifdef DEBUG_SINGLE_BUILD_TIME
        ui32 time_start = Root::Instance()->getCurrentTime();
#endif
        int channel = VoxelBuffer::CHANNEL_SDF;
        // Initialize dynamic memory:
        // These vectors are re-used.
        // We don't know in advance how much geometry we are going to produce.
        // Once capacity is big enough, no more memory should be allocated
        clear_output();

        std::shared_ptr<VoxelBuffer> voxels = input.voxels;

        if (BMESH_SUPPORT_WATER) {
            build_internal_with_water(output, voxels, channel, input.lod, input.position);
        }
        else {
            build_internal(output, voxels, channel, input.lod, input.position);
        }

        if (output.surfaces.size() > 0) {
            calcUvs(output.surfaces[0]);
        }

#ifdef DEBUG_SINGLE_BUILD_TIME
        ui32 time_end = Root::Instance()->getCurrentTime();
        LordLogError("VoxelMesherSurfaceNets::build duration = %I32d(ms)", time_end - time_start);
#endif
    }

    bool VoxelMesherSurfaceNets::calcUvs(Arrays& singleArray) {
        singleArray.uvs.resize(singleArray.positions.size(), Vector2(rand() / RAND_MAX, rand() / RAND_MAX));

        xatlas::Atlas* atlas = xatlas::Create();
        xatlas::MeshDecl meshDecl;
        meshDecl.vertexCount = singleArray.positions.size();
        meshDecl.vertexPositionData = singleArray.positions.data();
        meshDecl.vertexPositionStride = sizeof(Vector3);
        meshDecl.indexCount = singleArray.indices.size();
        meshDecl.indexData = singleArray.indices.data();
        meshDecl.indexFormat = xatlas::IndexFormat::UInt32;
        xatlas::AddMeshError error = xatlas::AddMesh(atlas, meshDecl, (uint32_t)1);
        if (error != xatlas::AddMeshError::Success) {
            xatlas::Destroy(atlas);
            return false;
        }

        xatlas::Generate(atlas);

        if (atlas->meshCount <= 0) return false;
        float uvWidth = 1.f / atlas->width;
        float uvHeight = 1.f / atlas->height;
        const xatlas::Mesh& mesh = atlas->meshes[0];
        for (uint32_t j = 0; j < mesh.chartCount; j++) {
            const xatlas::Chart* chart = &mesh.chartArray[j];
            for (uint32_t k = 0; k < chart->faceCount; k++) {
                int verts[3][2];
                for (int l = 0; l < 3; l++) {
                    const xatlas::Vertex& v = mesh.vertexArray[mesh.indexArray[chart->faceArray[k] * 3 + l]];
                    singleArray.uvs[v.xref] = Vector2(uvWidth * (int)v.uv[0], uvHeight * (int)v.uv[1]);
                }
            }
        }
    }

    VoxelMesher *VoxelMesherSurfaceNets::clone()
    {
        return new VoxelMesherSurfaceNets;
    }

    void VoxelMesherSurfaceNets::build_internal(MeshOutput &output, std::shared_ptr<VoxelBuffer> voxels,
        unsigned int channel, int lod_index, const Vector3i &position)
    {
        uint8_t cell_border_mask = 0;

        const Vector3i block_size_with_padding = voxels->get_size();
        const Vector3i block_size = block_size_with_padding - Vector3i(MIN_PADDING + MAX_PADDING);
        const Vector3i block_size_scaled = block_size << lod_index;

        auto const funVoxelSdf = [&](const Vector3i &pos) -> float const {
            return voxels->get_voxel_f(pos, VoxelBuffer::CHANNEL_SDF);
        };
        auto const funVoxelMaterial = [&](const Vector3 &pos)->MaterialType const {
            return (MaterialType)voxels->get_voxel(pos, VoxelBuffer::CHANNEL_TYPE);
        };

        Vector3i vVoxelSize(block_size_with_padding.x, block_size_with_padding.y, block_size_with_padding.z);
        std::shared_ptr<VertexMesh> &&pmesh = surface_nets_reduce_surface(funVoxelSdf, funVoxelMaterial, vVoxelSize,
            MIN_PADDING, MAX_PADDING, 0, position);

        vertexMesh2outputArrays(pmesh, output, block_size_scaled, cell_border_mask, position);
        if (lod_index == 0) return;
        //insure lod_index in [0, _lod_count]
        for (int downscale_lod = 1; downscale_lod < _lod_count; ++downscale_lod) {
            clear_output();

            std::shared_ptr<VertexMesh> &&pnew_mesh = surface_nets_lod_from_upper_lod(
                pmesh, funVoxelSdf, funVoxelMaterial, vVoxelSize, MIN_PADDING, MAX_PADDING, downscale_lod);

            if (lod_index == downscale_lod) {
                output.surfaces.clear();
                vertexMesh2outputArrays(pnew_mesh, output, block_size_scaled, cell_border_mask, position);
                return;
            }

            vertexMesh2outputArrays(pnew_mesh, output, block_size_scaled, cell_border_mask, position);
            pmesh.swap(pnew_mesh);
        }
    }

    void VoxelMesherSurfaceNets::build_internal_with_water(MeshOutput &output,
        std::shared_ptr<VoxelBuffer> voxels,
        unsigned int channel,
        int lod_index,
        const Vector3i &position) {
        const Vector3i block_size_with_padding = voxels->get_size();
        if (block_size_with_padding.x <= 2 || block_size_with_padding.y <= 2 || block_size_with_padding.z <= 2) {
            return;
        }
        const Vector3i block_size = block_size_with_padding - Vector3i(MIN_PADDING + MAX_PADDING);
        const Vector3i block_size_scaled = block_size << lod_index;
        uint8_t cell_border_mask = 0;

        auto const funVoxelSdf = [&](const Vector3 &pos) -> float {
            return voxels->get_voxel_f(pos, VoxelBuffer::CHANNEL_SDF);
        };
        auto const funVoxelMaterial = [&](const Vector3 &pos)->MaterialType {
            return (MaterialType)voxels->get_voxel(pos, VoxelBuffer::CHANNEL_TYPE);
        };
        auto const funVoxelOccupancy = [&](const Vector3i &posi) -> float {
            float sdf = voxels->get_voxel_f(posi, VoxelBuffer::CHANNEL_SDF);
            return (sdf > 0 ? 0 : -sdf);
        };
        //get voxel tag, 0: air; 1: water; 2: others
        auto const funVoxelTag = [&](const Vector3i &posi) -> int {
            MaterialType material = (MaterialType)voxels->get_voxel(Vector3i(posi), VoxelBuffer::CHANNEL_TYPE);
            return (material == MaterialType::AIR ? 0 : (material == MaterialType::WATER ? 1 : 2));
        };

        auto const funVoxelOccupancyAndTag = [&](const Vector3i &posi) -> std::pair<int, int> {
            float sdf = voxels->get_voxel_f(posi, VoxelBuffer::CHANNEL_SDF);
            MaterialType material = (MaterialType)voxels->get_voxel(Vector3i(posi), VoxelBuffer::CHANNEL_TYPE);
            int occupancy = static_cast<int>(sdf > 0 ? 0 : -sdf);
            int tag = (sdf > 0 ? 0 : (material == MaterialType::WATER ? 1 : 2));
            return std::pair<int, int>(occupancy, tag);
        };

        Vector3i vVoxelSize(block_size_with_padding.x, block_size_with_padding.y, block_size_with_padding.z);

        std::shared_ptr<VertexMesh> &&pmesh = surface_nets_with_water(funVoxelSdf, funVoxelMaterial, funVoxelOccupancyAndTag, funVoxelTag, vVoxelSize, 0);
        
        separateSolidAndWaterFaces(pmesh, lod_index);

        clear_output();
        vertexMesh2OutputArrays_with_water(pmesh, output, block_size_scaled, cell_border_mask, position);

        // response for engine's request, did not use lod of mesh anymore in version 2.0, annotate in 2021/10/29
#ifdef USE_MESH_LOD
        if (lod_index == 0) return;
        //insure lod_index in [0, _lod_count]
        for (int downscale_lod = 1; downscale_lod < _lod_count; ++downscale_lod) {
            clear_output();

            std::shared_ptr<VertexMesh> &&pnew_mesh = surface_nets_lod_from_upper_lod(
                pmesh, funVoxelSdf, funVoxelMaterial, vVoxelSize, MIN_PADDING, MAX_PADDING, downscale_lod);
            separateSolidAndWaterFaces(pnew_mesh);

            if (lod_index == downscale_lod) {
                output.surfaces.clear();
                vertexMesh2OutputArrays_with_water(pnew_mesh, output, block_size_scaled, cell_border_mask, position);
                return;
            }

            vertexMesh2OutputArrays_with_water(pnew_mesh, output, block_size_scaled, cell_border_mask, position);
            pmesh.swap(pnew_mesh);
        }
#endif
    }

    int VoxelMesherSurfaceNets::emit_vertex(const Vector3& primary, const Vector3& normal, uint16_t border_mask, const Vector3& secondary, uint32_t material)
    {

        int vi = _output_vertices.size();

        _output_vertices.push_back(primary);
        _output_normals.push_back(normal);
        _output_materials.push_back(material);

        return vi;
    }

    void VoxelMesherSurfaceNets::clear_output()
    {
        _output_indices.clear();
        _is_water = false;
        _output_normals.clear();
        _output_vertices.clear();
        _output_materials.clear();
    }

    void VoxelMesherSurfaceNets::fill_surface_arrays(Arrays &arrays)
    {
        arrays.positions.swap(_output_vertices);
        arrays.normals.swap(_output_normals);
        arrays.indices.swap(_output_indices);
        arrays.materials.swap(_output_materials);
        arrays.isWater = _is_water;
    }

    void VoxelMesherSurfaceNets::vertexMesh2outputArrays(
        std::shared_ptr<VertexMesh> pmesh,
        MeshOutput &output,
        const Vector3i &block_size_scaled,
        uint8_t cell_border_mask,
        const Vector3i &position) {
        VertexMesh &mesh = *pmesh;
        if (mesh.vertices_.size() <= 0 || mesh.faces_[0].size() <= 0) {
            return;
        }

        Vector3 secondary;
        std::vector<int> cell_vertex_indices(mesh.vertices_.size(), 0);
        for (unsigned int i = 0; i < mesh.vertices_.size(); i++) {
            if (!mesh.vertexValid_[i]) continue;

            mesh.normals_[i].normalize();

            Vector3 primary = mesh.vertices_[i] - Vector3(2) + position.to_vec3() * 16.f;
            secondary = get_secondary_position(primary, mesh.normals_[i], 0, block_size_scaled);
            cell_vertex_indices[i] = emit_vertex(primary, mesh.normals_[i], cell_border_mask, secondary, mesh.materials_[i]);
        }

        Vector3 offset = vector3i2Vector3(position) * 16.f - Vector3(2);
        for (unsigned int t = 0; t < mesh.faces_[0].size(); ++t)
        {
            for (int i = 0; i < 3; i++) {
                uint32_t index = (uint32_t)(cell_vertex_indices[(int)mesh.faces_[0][t][i]]);

                if (MESH_SHARE_POINTS) {
                    //if normal is ZERO or material belongs to regular materials, do not share face vertex
                    if (_output_normals[index] == Vector3(0) || !mesh.share_point_[(int)mesh.faces_[0][t][i]]) {
                        Vector3 primary = mesh.vertices_[(int)mesh.faces_[0][t][i]] + offset;
                        secondary = get_secondary_position(primary, mesh.face_normals_[t], 0, block_size_scaled);
                        index = emit_vertex(primary, mesh.face_normals_[t], cell_border_mask, secondary, mesh.materials_[(int)mesh.faces_[0][t][i]]);
                    }

                    _output_indices.emplace_back(index);
                }
                else {
                    //if the angle of vertex normal and face normal > threshold, do not share face vertex
                    float normal_angle = get_normal_angle(_output_normals[index], mesh.face_normals_[t]);
                    if (_output_normals[index] == Vector3(0) || !mesh.share_point_[(int)mesh.faces_[0][t][i]]
                        || normal_angle >  MathFuncs::PI / 4.f) {
                        Vector3 primary = mesh.vertices_[(int)mesh.faces_[0][t][i]] + offset;
                        secondary = get_secondary_position(primary, mesh.face_normals_[t], 0, block_size_scaled);
                        index = emit_vertex(primary, mesh.face_normals_[t], cell_border_mask, secondary, mesh.materials_[(int)mesh.faces_[0][t][i]]);
                    }

                    _output_indices.emplace_back(index);
                }
            }
        }

        Arrays regular_arrays;
        fill_surface_arrays(regular_arrays);
        output.surfaces.push_back(regular_arrays);
#ifdef DEBUG_SINGLE_BUILD_MESHS
        outputToObjFile(output.surfaces[0], position, false);
#endif
    }

    // as engine's request, output.arrays return 2 arrays, one for non-water mesh, the other for water mesh
    // non mesh: output.arrays.size() == 0
    // only non-water mesh or only water mesh: output.arrays.size() == 1, use isWater
    // both non-water mesh and water mesh: output.arrays.size() == 2
    void VoxelMesherSurfaceNets::vertexMesh2OutputArrays_with_water(
        std::shared_ptr<VertexMesh> pmesh,
        MeshOutput &output,
        const Vector3i &block_size_scaled,
        uint8_t cell_border_mask,
        const Vector3i &position) {
        VertexMesh &mesh = *pmesh;
        size_t nvertices_count = mesh.vertices_.size();
        size_t nsolid_faces_count = mesh.solid_faces_.size(), nwater_faces_count = mesh.water_faces_.size();
        if (nvertices_count < 3 || (nsolid_faces_count <= 0 && nwater_faces_count <= 0)) return;

        std::vector<int> cell_vertex_indices(nvertices_count, -1);
        Vector3 primary, secondary, normal;

        Vector3 offset = position.to_vec3() * 16.f - Vector3(2.f);
        if (nsolid_faces_count > 0) {
            //non water mesh
            _is_water = false;
            for (size_t i = 0; i < nsolid_faces_count; ++i) {
                for (int j = 0; j < 3; ++j) {
                    int vertex_index = mesh.solid_faces_[i][j];
                    int possible_index = cell_vertex_indices[vertex_index];
                    //did not push to _output_vertices yeah
                    if (possible_index < 0) {
                        primary = mesh.vertices_[vertex_index] + offset;
                        normal = mesh.normals_[vertex_index];
                        secondary = get_secondary_position(primary, normal, 0, block_size_scaled);
                        cell_vertex_indices[vertex_index] = emit_vertex(primary, normal, cell_border_mask, secondary);
                    }

                    uint32_t index = (uint32_t)(cell_vertex_indices[vertex_index]);
                    if (_output_normals[index] == Vector3(0) || !mesh.share_point_[vertex_index]) {
                        normal = (mesh.vertices_[mesh.solid_faces_[i].y] - mesh.vertices_[mesh.solid_faces_[i].x]).cross(mesh.vertices_[mesh.solid_faces_[i].z] - mesh.vertices_[mesh.solid_faces_[i].x]);

                        primary = mesh.vertices_[vertex_index] + offset;
                        secondary = get_secondary_position(primary, normal, 0, block_size_scaled);
                        index = emit_vertex(primary, normal, cell_border_mask, secondary);
                    }

                    _output_indices.emplace_back(index);
                }
            }

            Arrays regular_arrays;
            fill_surface_arrays(regular_arrays);
            output.surfaces.push_back(regular_arrays);
            clear_output();
            std::vector<int>(nvertices_count, -1).swap(cell_vertex_indices);
        }

        if (nwater_faces_count > 0) {
            //water mesh
            _is_water = true;
            for (size_t i = 0; i < nwater_faces_count; ++i) {
                for (int j = 0; j < 3; ++j) {
                    int vertex_index = mesh.water_faces_[i][j];
                    int possible_index = cell_vertex_indices[vertex_index];
                    //did not push to _output_vertices yeah
                    if (possible_index < 0) {
                        primary = mesh.vertices_[vertex_index];
                        normal = mesh.normals_[vertex_index];
                        secondary = get_secondary_position(primary, normal, 0, block_size_scaled);
                        cell_vertex_indices[vertex_index] = emit_vertex(primary, normal, cell_border_mask, secondary);
                    }

                    uint32_t index = (uint32_t)(cell_vertex_indices[vertex_index]);
                    if (_output_normals[index] == Vector3(0) || !mesh.share_point_[vertex_index]) {
                        normal = (mesh.vertices_[mesh.water_faces_[i].y] - mesh.vertices_[mesh.water_faces_[i].x]).cross(mesh.vertices_[mesh.water_faces_[i].z] - mesh.vertices_[mesh.water_faces_[i].x]);

                        primary = mesh.vertices_[(int)mesh.water_faces_[i][j]];
                        secondary = get_secondary_position(primary, normal, 0, block_size_scaled);
                        index = emit_vertex(primary, normal, cell_border_mask, secondary);
                    }
                    //seperate water mesh and non water mesh
                    _output_indices.emplace_back(index);
                }
            }

            Arrays water_arrays;
            fill_surface_arrays(water_arrays);
            output.surfaces.push_back(water_arrays);
        }

#ifdef DEBUG_SINGLE_BUILD_MESHS
        if (output.surfaces.size() == 1) {
            outputToObjFile(output.surfaces[0], position, output.surfaces[0].isWater);
        }
        else if (output.surfaces.size() == 2) {
            outputToObjFile(output.surfaces[0], position, false);
            outputToObjFile(output.surfaces[1], position, true);
        }

        std::string sFileName = ("side_mesh_") + std::to_string(position.x) + "_" + std::to_string(position.y) + "_" + std::to_string(position.z) + ".obj";
        bool bhasface = false;
        for (int e = 0; e < Cube_Count; ++e) {
            if (mesh.transition_faces_[1][e].size() > 0) {
                bhasface = true;
                break;
            }
        }
        if (bhasface) {
            std::ofstream ofs(sFileName);
            for (size_t i = 0; i < mesh.vertices_.size(); i++) {
                ofs << "v " << mesh.vertices_[i][0] + offset.x << " " << mesh.vertices_[i][1] + offset.y << " " << mesh.vertices_[i][2] + offset.z << std::endl;
            }
            for (int e = 0; e < Cube_Count; ++e) {
                if (mesh.transition_faces_[1][e].size() > 0) {
                    for (size_t i = 0; i < mesh.transition_faces_[1][e].size(); ++i) {
                        ofs << "f " << mesh.transition_faces_[1][e][i].x + 1 << " " << mesh.transition_faces_[1][e][i].y + 1 << " " << mesh.transition_faces_[1][e][i].z + 1 << std::endl;
                    }
                }
            }
            ofs.close();
        }
#endif
    }

    void VoxelMesherSurfaceNets::outputToObjFile(Arrays &singleArray, const Vector3i &position, bool bIsWater) {
        if (singleArray.indices.size() <= 0 || singleArray.positions.size() <= 0) {
            return;
        }
        std::string sFileName = (bIsWater ? "water_mesh_" : "solid_mesh_") + std::to_string(position.x) + "_" + std::to_string(position.y) + "_" + std::to_string(position.z) + ".obj";
        std::ofstream ofs(sFileName);
        for (size_t i = 0; i < singleArray.positions.size(); i++) {
            ofs << "v " << singleArray.positions[i][0] << " " << singleArray.positions[i][1] << " " << singleArray.positions[i][2] << std::endl;
            ofs << "vn " << singleArray.normals[i].x << " " << singleArray.normals[i].y << " " << singleArray.normals[i].z << std::endl;
        }
        for (size_t i = 0; i < singleArray.indices.size(); i += 3) {
            ofs << "f " << singleArray.indices[i] + 1 << " " << singleArray.indices[i + 1] + 1 << " " << singleArray.indices[i + 2] + 1 << std::endl;
        }
        ofs.close();
    }
