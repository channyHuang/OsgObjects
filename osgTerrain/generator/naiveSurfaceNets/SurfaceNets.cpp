#include "SurfaceNets.h"

#include <algorithm>
#include <array>
#include <numeric>
#include <queue>
#include <unordered_map>
#include <unordered_set>

/*
*
* Visit every voxel in the regular grid with the following configuration
*
* Coordinate frame
*
*       z y
*       |/
*       o--x
*
* Voxel corner indices
*
*        7          6
*        o----------o
*       /|         /|
*     4/ |       5/ |
*     o--|-------o  |
*     |  o-------|--o
*     | /3       | /2
*     |/         |/
*     o----------o
*     0          1
*
* Potentially crossed edges
*
*     4
*     o
*     |  o
*     | /3
*     |/
*     o----------o
*     0          1
*
* For potentially crossed edges, we generate quads connecting the 4 cubes of
* the crossed edge, so the considered neighbors are:
*
*
*           o----------o----------o
*          /|         /|         /|
*         / |   0    / |        / | <---- this is the current active cube (i,j,k) in the above coordinate frame
*        o--|-------o--|-------o  |
*       /|  o------/|--o------/|--o
*      / | /|1    / | /|2    / | /|
*     o--|/-|--5-o--|/-|--4-o  |/ |
*     |  o--|----|--o--|----|--o  |
*     | /|  o----|-/|--o----|-/|--o
*     |/ | /     |/ | /3    |/ | /
*     o--|/------o--|/------o  |/
*        o-------|--o-------|--o
*                | /        | /
*                |/         |/
*                o----------o
*
* Indices of neighbor voxels are written on the top squares of the cubes.
* So upper cubes are 0, 1, 2 and lower cubes are 3, 4, 5.
*/
namespace SURFACE_NETS {

	namespace {
		inline Vector3i index2Position(std::size_t voxelIndex, const Vector3i &vVoxelSize) {
			int i = (voxelIndex) % vVoxelSize.x;
			int j = (voxelIndex / vVoxelSize.x) % vVoxelSize.y;
			int k = (voxelIndex) / (vVoxelSize.x * vVoxelSize.y);
			return Vector3i(i, j, k);
		};

		inline std::size_t const position2Index(const Vector3i& pos, const Vector3i &vVoxelSize) {
			return pos.x + (pos.y * vVoxelSize.x) + (pos.z * vVoxelSize.x * vVoxelSize.y);
		};

		auto const isVoxelLowerBoundary = [](const Vector3i &pos) -> bool {
			return (pos.x == 0 || pos.y == 0 || pos.z == 0);
		};

		auto const isVoxelUpperBoundary = [](const Vector3i &pos, int nUpperBound) -> bool {
			return (pos.x == nUpperBound || pos.y == nUpperBound || pos.z == nUpperBound);
		};
		// since padding is 2, need to weed out same face in different blocks, it should be deal in engine, but...
		// for example, [-16, -1] and [0, 15] will both generate a face if the three vertices are all in [-1, 0)
		auto const isRepeatFace = [](const Vector3i &indexa, const Vector3i &indexb, const Vector3i &indexc, int bound) -> bool {
			return (isVoxelUpperBoundary(indexa, bound) && isVoxelUpperBoundary(indexb, bound) && isVoxelUpperBoundary(indexc, bound));
		};

		auto const isVoxelActive = [](float scalar, float isovalue) -> bool {
			return scalar > isovalue;
		};

		auto const isEdgeActive = [](float scalar1, float scalar2, float isovalue) -> bool
		{
			return isVoxelActive(scalar1, isovalue) != isVoxelActive(scalar2, isovalue);
		};

		// the edges provide indices to the corresponding current cube's vertices (voxel corners)
		std::size_t const edges[12][2] =
		{
			{ 0u, 1u },
			{ 1u, 2u },
			{ 2u, 3u },
			{ 3u, 0u },
			{ 4u, 5u },
			{ 5u, 6u },
			{ 6u, 7u },
			{ 7u, 4u },
			{ 0u, 4u },
			{ 1u, 5u },
			{ 2u, 6u },
			{ 3u, 7u }
		};

        MeshType getBlockMaterial(MaterialType material)
		{
			switch (material)
			{
            case MaterialType::ROAD:
            case MaterialType::BRICK:
            case MaterialType::WOOD:
				return MeshType::MESH_CUBE;
// 			case MaterialType::ICE:
// 			case MaterialType::BASALT:
// 			case MaterialType::ROCK:
// 				return MeshType::MESH_HALFSMOOTH;
            case MaterialType::WATER:
				return MeshType::MESH_WATER;
			default:
				break;
			}
			return MeshType::MESH_UNKNOWN;
		}

		const Vector3i quadNeighbors[3] = {
				{ 0, 1, 2 },
				{ 0, 5, 4 },
				{ 2, 3, 4 }
		};

		const std::array<std::size_t, 3> possibleVerticesOrder[2] = {
			{ 0, 1, 2 },
			{ 2, 1, 0 }
		};

        Vector3i findOriginCorner(const Vector3i &posi, int lod) {
			int offset = (1 << lod);
            Vector3i vCorner = posi;
            vCorner = vCorner - Vector3i(vCorner.x % offset, vCorner.y % offset, vCorner.z % offset);
			return vCorner;
		}
        Vector3i findOriginCorner(const Vector3 &pos, int lod) {
            Vector3 &&vpos = pos.getFloor();
            Vector3i &&vCorner = Vector3i(static_cast<int>(vpos.x), static_cast<int>(vpos.y), static_cast<int>(vpos.z));
            return findOriginCorner(vCorner, lod);
        }

		// 简单对象池
		template<typename PoolT>
		thread_local PoolT local_pool = PoolT();
		template<typename ObjectT>
		std::shared_ptr<ObjectT> get_from_pool()
		{
			struct ObjectPool final
			{
				std::deque<ObjectT*> pool;
				~ObjectPool()
				{
					for (auto &object : pool)
                        delete(object);
				}
			};
			auto &object_pool = local_pool<ObjectPool>.pool;
			ObjectT *object = nullptr;
			if (object_pool.empty())
			{
                object = new(ObjectT);
			}
			else
			{
				object = object_pool.back();
				object_pool.pop_back();
			}
			return std::shared_ptr<ObjectT>(object, [&](ObjectT *_object) {
				object_pool.emplace_back(_object);
			});
		}
	}

	bool gen_vertex_by_voxel(
		std::function<float(const Vector3&)> const& sdfFunction,
        std::function<MaterialType(const Vector3&)> const& materialFunction,
        const Vector3i &vpos,
        Vector3 &mesh_vertex,
		bool &bSharePoint,
		const int lod,
		float const isovalue) {
		Vector3 vVoxelCorner[8];
		float vVoxelSdfValue[8];
		bool vEdgeActive[12];
		std::vector<Vector3> vMCPoints;

		int offset = (1 << lod);
        Vector3 pos = Vector3(vpos.x, vpos.y, vpos.z);
		//init corner info
		for (int i = 0; i < 8; ++i) {
            vVoxelCorner[i] = pos + GeometryMath::vCubeVer[i] * (offset * 1.f);
			vVoxelSdfValue[i] = sdfFunction(vVoxelCorner[i]);
		}
		//get active edge
		bool bVoxelActive = false;
		for (unsigned int idx = 0; idx < 12; ++idx) {
			vEdgeActive[idx] = isEdgeActive(vVoxelSdfValue[edges[idx][0]], vVoxelSdfValue[edges[idx][1]], isovalue);
			bVoxelActive = (bVoxelActive || vEdgeActive[idx]);
		}
		if (!bVoxelActive) {
			return false;
		}

		vMCPoints.clear();
        Vector3 vMCPointSum = Vector3(0);
		for (std::size_t e = 0; e < 12; ++e) {
			if (!vEdgeActive[e]) {
				continue;
			}
            const Vector3 p1 = vVoxelCorner[edges[e][0]];
            const Vector3 p2 = vVoxelCorner[edges[e][1]];

            const float s1 = vVoxelSdfValue[edges[e][0]];
            const float s2 = vVoxelSdfValue[edges[e][1]];

            const float t = (isovalue - s1) / (s2 - s1);

			Vector3 vMCPoint = p1 + t * (p2 - p1);
			vMCPoints.emplace_back(vMCPoint);
			vMCPointSum += vMCPoint;
		}

		float const nMCPointCount = static_cast<float>(vMCPoints.size());
		// need to minus Vector3(MIN_PADDING, MIN_PADDING, MIN_PADDING)
		mesh_vertex = vMCPointSum / nMCPointCount;

		// consider materialtype
		// in 8 corners, if one is cube, then generate cube vertex; if there is no smooth material, not share point in diff faces 
		bool bvertex_is_cube = false;
		int nhalfsmooth_count = 0;
		for (unsigned int idx = 0; idx < 8; ++idx) {
            MaterialType eMaterialType = materialFunction(vVoxelCorner[idx]);
			auto mtr = getBlockMaterial(eMaterialType);
			if (mtr != MeshType::MESH_UNKNOWN)
			{
				if (mtr == MeshType::MESH_CUBE)
					bvertex_is_cube = true;
				++nhalfsmooth_count;
			}
            else if (eMaterialType == MaterialType::AIR) {
				++nhalfsmooth_count;
			}
		}
		if (nhalfsmooth_count < 8) bSharePoint = true;

		if (bvertex_is_cube) {
            mesh_vertex = pos + Vector3(fcube_position, fcube_position, fcube_position) * (offset * 1.f);
		}

		//keep vertex totally inside the voxel in case of failed to get material type
        Vector3i vOriginVertex = findOriginCorner(mesh_vertex, lod);
		if (vOriginVertex.x != vpos.x) mesh_vertex.x -= MathFuncs::LOWEPSILON;
		if (vOriginVertex.y != vpos.y) mesh_vertex.y -= MathFuncs::LOWEPSILON;
		if (vOriginVertex.z != vpos.z) mesh_vertex.z -= MathFuncs::LOWEPSILON;

		return true;
	}

	bool gen_avg_vertex_by_voxel(
		std::function<float(const Vector3&)> const& sdfFunction,
        std::function<MaterialType(const Vector3&)> const& materialFunction,
        const Vector3i &vpos,
        Vector3 &mesh_vertex,
		bool &bSharePoint,
		const int lod,
		float const isovalue) {
		int offset = (1 << lod);
		int count = 0;
        Vector3i vmax_pos = vpos + offset;
        mesh_vertex = Vector3(0);
		for (int i = vpos.x; i < vmax_pos.x; ++i) {
			for (int j = vpos.y; j < vmax_pos.y; ++j) {
				for (int k = vpos.z; k < vmax_pos.z; ++k) {
                    Vector3 vertex;
                    Vector3i pos(i, j, k);
					bool bShareSinglePoint = false;
					if (gen_vertex_by_voxel(sdfFunction, materialFunction, pos, vertex, bShareSinglePoint)) {
						if (!bShareSinglePoint) bSharePoint = false;
						mesh_vertex += vertex;
						++count;
					}
				}
			}
		}
		if (count <= 0) return false;
		mesh_vertex /= (count * 1.f);
		return true;
	}

    std::shared_ptr<VertexMesh> surface_nets(
        std::function<float(const Vector3i&)> const& sdfFunction,
        std::function<MaterialType(const Vector3&)> const& materialFunction,
        const Vector3i &vVoxelSize,
		const int nMinPadding, const int nMaxPadding,
		const int lod,
        const Vector3i &start_position,
		float const isovalue)
	{
        std::shared_ptr<VertexMesh> newmesh = std::make_shared<VertexMesh>();
        //auto &&pmesh = get_from_pool<VertexMesh>();
        auto &&pmesh = newmesh;
		VertexMesh &mesh = *pmesh;
		mesh.reserve(1680);
		mesh.clear();

        std::shared_ptr<std::unordered_map<std::size_t, std::uint64_t>> newIndex = std::make_shared<std::unordered_map<std::size_t, std::uint64_t>>();
        //auto &&pVoxelIndex2MeshIndex = get_from_pool<std::unordered_map<std::size_t, std::uint64_t>>();
        auto &&pVoxelIndex2MeshIndex = newIndex;
		std::unordered_map<std::size_t, std::uint64_t> &voxelIndex2MeshIndex = *pVoxelIndex2MeshIndex;
		voxelIndex2MeshIndex.reserve(1680);
		voxelIndex2MeshIndex.clear();

		float nMCPointCount = 0;
		// coordinates of voxel corners in the mesh's coordinate frame
		Vector3 vVoxelCorner[8];
		float vVoxelSdfValue[8];
		bool vEdgeActive[12];

        Vector3 pos = Vector3(0);
        Vector3i posi = Vector3i(0);
		for (pos.x = 0, posi.x = 0; pos.x < vVoxelSize.x - 1; ++pos.x, ++posi.x) {
			for (pos.y = 0, posi.y = 0; pos.y < vVoxelSize.y - 1; ++pos.y, ++posi.y) {
				for (pos.z = 0, posi.z = 0; pos.z < vVoxelSize.z - 1; ++pos.z, ++posi.z) {
					//init corner info
					for (int i = 0; i < 8; ++i) {
                        vVoxelCorner[i] = pos + GeometryMath::vCubeVer[i];
						vVoxelSdfValue[i] = sdfFunction(vVoxelCorner[i]);
					}			
					//get active edge
					bool bVoxelActive = false;
					for (unsigned int idx = 0; idx < 12; ++idx) {
						vEdgeActive[idx] = isEdgeActive(vVoxelSdfValue[edges[idx][0]], vVoxelSdfValue[edges[idx][1]], isovalue);
						bVoxelActive = (bVoxelActive || vEdgeActive[idx]);
					}
					if (!bVoxelActive) {
						continue;
					}

					nMCPointCount = 0;
                    Vector3 vMCPointSum = Vector3(0);
					for (std::size_t e = 0; e < 12; ++e) {
						if (!vEdgeActive[e]) {
							continue;
						}
						auto const p1 = vVoxelCorner[edges[e][0]];
						auto const p2 = vVoxelCorner[edges[e][1]];

						auto const s1 = vVoxelSdfValue[edges[e][0]];
						auto const s2 = vVoxelSdfValue[edges[e][1]];

						auto const t = (isovalue - s1) / (s2 - s1);

						Vector3 &&vMCPoint = p1 + t * (p2 - p1);
						++nMCPointCount;
						vMCPointSum += vMCPoint;
					}

					// need to minus Vector3(MIN_PADDING, MIN_PADDING, MIN_PADDING)
					Vector3 &&mesh_vertex = vMCPointSum / nMCPointCount;
					std::size_t const voxelIndex = position2Index(posi, vVoxelSize);
					std::uint64_t const vertex_index = mesh.vertices_.size();
					voxelIndex2MeshIndex[voxelIndex] = vertex_index;

					// consider materialtype
					// in 8 corners, if one is cube, then generate cube vertex; if there is no smooth material, not share point in diff faces 
					bool bvertex_is_cube = false;
					int nhalfsmooth_count = 0;
					for (unsigned int idx = 0; idx < 8; ++idx) {
                        MaterialType eMaterialType = materialFunction(vVoxelCorner[idx]);
						auto mtr = getBlockMaterial(eMaterialType);
						if (mtr != MeshType::MESH_UNKNOWN)
						{
							if (mtr == MeshType::MESH_CUBE)
								bvertex_is_cube = true;
							++nhalfsmooth_count;
						}
                        else if (eMaterialType == MaterialType::AIR) {
							++nhalfsmooth_count;
						}
					}
					if (bvertex_is_cube) {
                        mesh_vertex = pos + Vector3(fcube_position, fcube_position, fcube_position);
					}

					//keep vertex totally inside the voxel in case of failed to get material type
                    if (std::floor(mesh_vertex.x) != pos.x) mesh_vertex.x -= MathFuncs::LOWEPSILON;
                    if (std::floor(mesh_vertex.y) != pos.y) mesh_vertex.y -= MathFuncs::LOWEPSILON;
                    if (std::floor(mesh_vertex.z) != pos.z) mesh_vertex.z -= MathFuncs::LOWEPSILON;
					// mesh_vertex -= Vector3((float)nMinPadding, (float)nMinPadding, (float)nMinPadding);

					mesh.vertices_.emplace_back(mesh_vertex);
                    mesh.materials_.emplace_back(0);
					mesh.vertexValid_.emplace_back(!isVoxelLowerBoundary(posi) && !isVoxelUpperBoundary(posi, vVoxelSize.y - 2));
					mesh.share_point_.emplace_back(nhalfsmooth_count < 8);
				}
			}
		}

        mesh.normals_.resize(mesh.vertices_.size(), Vector3(0));
		auto &&itIndexEnd = voxelIndex2MeshIndex.end();
		//Triangles
		for (auto const& key_value : voxelIndex2MeshIndex)
		{
			std::size_t   const voxelIndex = key_value.first;
			std::uint64_t const vertex_index = key_value.second;

			auto const pos = index2Position(voxelIndex, vVoxelSize);
			int i = pos.x, j = pos.y, k = pos.z;

			if (isVoxelLowerBoundary(pos))
				continue;
		
			Vector3i edgeNeighborPos[6] = {
				{ i - 1, j    , k     },
				{ i - 1, j - 1, k     },
				{ i    , j - 1, k     },
				{ i    , j - 1, k - 1 },
				{ i    , j    , k - 1 },
				{ i - 1, j    , k - 1 }
			};

			const Vector3i vVoxelCornersInterest[4] =
			{
				{i, j, k},
				{i, j, k + 1},
				{i, j + 1, k},
				{i + 1, j, k}
			};

			const float edgeSdfValues[3][2] =
			{
				// directed edge (0,4)
				{
					sdfFunction(vVoxelCornersInterest[0]),
					sdfFunction(vVoxelCornersInterest[1])
				},
				// directed edge (3,0)
				{
					sdfFunction(vVoxelCornersInterest[2]),
					sdfFunction(vVoxelCornersInterest[0])
				},
				// directed edge (0,1)
				{
					sdfFunction(vVoxelCornersInterest[0]),
					sdfFunction(vVoxelCornersInterest[3])
				}
			};

			for (std::size_t idxi = 0; idxi < 3; ++idxi) {
				if ((edgeSdfValues[idxi][0] > 0 && edgeSdfValues[idxi][1] > 0) || (edgeSdfValues[idxi][0] <= 0 && edgeSdfValues[idxi][1] <= 0)) continue;

				int neighbors[3] = { 0, 0, 0 };
				for (std::size_t idxj = 0; idxj < 3; ++idxj) {
					neighbors[idxj] = position2Index(edgeNeighborPos[quadNeighbors[idxi][idxj]], vVoxelSize);
				}

				if (voxelIndex2MeshIndex.find(neighbors[0]) == itIndexEnd ||
					voxelIndex2MeshIndex.find(neighbors[1]) == itIndexEnd ||
					voxelIndex2MeshIndex.find(neighbors[2]) == itIndexEnd)
					continue;

				uint64_t const neighbor_vertices[3] = {
					voxelIndex2MeshIndex.at(neighbors[0]),
					voxelIndex2MeshIndex.at(neighbors[1]),
					voxelIndex2MeshIndex.at(neighbors[2])
				};

				auto verticesOrder =
					edgeSdfValues[idxi][1] > edgeSdfValues[idxi][0] ?
					possibleVerticesOrder[0] :
					possibleVerticesOrder[1];

				if (edgeSdfValues[idxi][0] == 0 && edgeSdfValues[idxi][1] == 0) {
					if (idxi == 0) verticesOrder = sdfFunction(vVoxelCornersInterest[2]) <= 0 ? possibleVerticesOrder[0] : possibleVerticesOrder[1];
					if (idxi == 2) verticesOrder = sdfFunction(vVoxelCornersInterest[2]) <= 0 ? possibleVerticesOrder[0] : possibleVerticesOrder[1];
				}

				const int v0 = static_cast<int>(vertex_index);
				const int v1 = static_cast<int>(neighbor_vertices[verticesOrder[0]]);
				const int v2 = static_cast<int>(neighbor_vertices[verticesOrder[1]]);
				const int v3 = static_cast<int>(neighbor_vertices[verticesOrder[2]]);

				Vector3i &&vPos1 = index2Position(neighbors[0], vVoxelSize);
				Vector3i &&vPos2 = index2Position(neighbors[1], vVoxelSize);
				Vector3i &&vPos3 = index2Position(neighbors[2], vVoxelSize);

                Vector3 &&face_normal_1 = Triangle(mesh.vertices_[v0], mesh.vertices_[v1], mesh.vertices_[v2]).getNormal();
				mesh.normals_[v0] += face_normal_1;
				mesh.normals_[v1] += face_normal_1;
				mesh.normals_[v2] += face_normal_1;

                Vector3 &&face_normal_2 = Triangle(mesh.vertices_[v0], mesh.vertices_[v2], mesh.vertices_[v3]).getNormal();
				mesh.normals_[v0] += face_normal_2;
				mesh.normals_[v2] += face_normal_2;
				mesh.normals_[v3] += face_normal_2;


                if (!isVoxelLowerBoundary(vPos1) && !isVoxelLowerBoundary(vPos2)
                    && !isVoxelUpperBoundary(pos, vVoxelSize.y - 2)
                    && !isVoxelUpperBoundary(vPos1, vVoxelSize.y - 2)
                    && !isVoxelUpperBoundary(vPos2, vVoxelSize.y - 2)
                    && !isRepeatFace(pos, vPos1, vPos2, 1)) {
                    mesh.faces_[0].emplace_back(Vector3i(v0, v1, v2));

                    mesh.face_normals_.emplace_back(face_normal_1);
                }
                else {
                    mesh.edge_faces_.emplace_back(Vector3i(v0, v2, v1));
                }
                if (!isVoxelLowerBoundary(vPos2) && !isVoxelLowerBoundary(vPos3)
                    && !isVoxelUpperBoundary(pos, vVoxelSize.y - 2)
                    && !isVoxelUpperBoundary(vPos2, vVoxelSize.y - 2)
                    && !isVoxelUpperBoundary(vPos3, vVoxelSize.y - 2)
                    && !isRepeatFace(pos, vPos2, vPos3, 1)) {
                    mesh.faces_[0].emplace_back(Vector3i(v0, v2, v3));

                    mesh.face_normals_.emplace_back(face_normal_2);
                }
                else {
                    mesh.edge_faces_.emplace_back(Vector3i(v0, v3, v2));
                }
			}
		}
		return pmesh;
    }

	std::shared_ptr<VertexMesh> surface_nets_reduce_surface(
        std::function<float(const Vector3i&)> const& sdfFunction,
        std::function<MaterialType(const Vector3&)> const& materialFunction,
        const Vector3i &vVoxelSize,
		const int nMinPadding, const int nMaxPadding,
		const int lod,
        const Vector3i &position,
		float const isovalue) {
        std::shared_ptr<VertexMesh> &&pmesh = surface_nets(sdfFunction, materialFunction, vVoxelSize, nMinPadding, nMaxPadding, lod);
		VertexMesh &mesh = *pmesh;
		
		if (lod <= 0) {
			return pmesh;
		}
		//if calculation is too large, try to modify downscale_lod = lod;
		int downscale_lod = 1;

		std::unordered_map < std::size_t, std::uint32_t > new_voxelIndex2MeshIndex{};
		while (downscale_lod <= lod) {
			auto pnew_mesh = surface_nets_lod_from_upper_lod(pmesh, sdfFunction, materialFunction, 
				vVoxelSize, nMinPadding, nMaxPadding, downscale_lod);
			VertexMesh &new_mesh = *pnew_mesh;

			mesh.swap(new_mesh);
			downscale_lod++;
		}

		return pmesh;
	}

	std::shared_ptr<VertexMesh> surface_nets_lod_from_upper_lod(
		std::shared_ptr<VertexMesh> pmesh, 
		std::function<float(const Vector3&)> const& sdfFunction,
        std::function<MaterialType(const Vector3&)> const& materialFunction,
        const Vector3i &vVoxelSize,
		const int nMinPadding, const int nMaxPadding,
		int downscale_lod) {
		VertexMesh &mesh = *pmesh;
        std::shared_ptr<VertexMesh> pnew_mesh = std::make_shared<VertexMesh>();
        //auto &&pnew_mesh = get_from_pool<VertexMesh>();
		VertexMesh &new_mesh = *pnew_mesh;
		new_mesh.reserve(1680);
		new_mesh.clear();

		std::unordered_map < std::size_t, std::uint32_t > new_voxelIndex2MeshIndex{};
		int min_pos = nMinPadding, max_pos = vVoxelSize.y - nMaxPadding - (1 << downscale_lod);
		new_voxelIndex2MeshIndex.clear();

        for (unsigned int idx = 0; idx < mesh.faces_[0].size(); ++idx) {
                    Vector3i face_idx = mesh.faces_[0][idx];

			Vector3i new_face_idx;
			for (int axis = 0; axis < 3; ++axis) {
				Vector3i &&vCorner = findOriginCorner(mesh.vertices_[face_idx[axis]], 0);
				int i = vCorner.x, j = vCorner.y, k = vCorner.z;

				//keep point
				if (i < min_pos || j < min_pos || k < min_pos || i >= max_pos || j >= max_pos || k >= max_pos || !mesh.share_point_[face_idx[axis]]) {
					std::size_t const voxelIndex = position2Index(vCorner, vVoxelSize);
					auto itr = new_voxelIndex2MeshIndex.find(voxelIndex);
					if (itr == new_voxelIndex2MeshIndex.end()) {
						uint32_t vertex_index = new_mesh.vertices_.size();
						new_voxelIndex2MeshIndex[voxelIndex] = vertex_index;
						new_mesh.vertices_.emplace_back(mesh.vertices_[face_idx[axis]]);
                        new_mesh.normals_.emplace_back(Vector3(0));
						new_mesh.share_point_.emplace_back(mesh.share_point_[face_idx[axis]]);
						new_face_idx[axis] = vertex_index;
					}
					else {
						uint32_t vertex_index = itr->second;
						new_face_idx[axis] = vertex_index;
						if (mesh.share_point_[face_idx[axis]]) {
							new_mesh.share_point_[vertex_index] = true;
						}
					}
				} //reduce face to new point
				else {
					Vector3i vLodCorner = findOriginCorner(vCorner - nMinPadding, downscale_lod) + nMinPadding;
					std::size_t const lodVoxelIndex = position2Index(vLodCorner, vVoxelSize);

					auto &&itr = new_voxelIndex2MeshIndex.find(lodVoxelIndex);
					if (itr == new_voxelIndex2MeshIndex.end()) {
						Vector3 new_vertex;
						bool bSharePoint = false;
						bool bPointValid = gen_avg_vertex_by_voxel(sdfFunction, materialFunction, vLodCorner, new_vertex, bSharePoint, downscale_lod);
						if (!bPointValid) {
                            std::cout << "surface point in invalid pos = (" << vLodCorner.x << vLodCorner.y << vLodCorner.z << ")";
						}
						uint32_t vertex_index = new_mesh.vertices_.size();
						new_voxelIndex2MeshIndex[lodVoxelIndex] = vertex_index;
						new_mesh.vertices_.emplace_back(new_vertex);
                        new_mesh.normals_.emplace_back(Vector3(0));
						new_mesh.share_point_.emplace_back(mesh.share_point_[face_idx[axis]]);
						new_face_idx[axis] = vertex_index;
					}
					else {
						uint32_t const vertex_index = itr->second;
						new_face_idx[axis] = vertex_index;
						if (mesh.share_point_[face_idx[axis]]) {
							new_mesh.share_point_[vertex_index] = true;
						}
					}
				}
			}
			//reduce invalid face
			if (new_face_idx[0] == new_face_idx[1] || new_face_idx[0] == new_face_idx[2] || new_face_idx[1] == new_face_idx[2]) continue;

            new_mesh.faces_[0].emplace_back(new_face_idx);
			//calc normals
            Vector3 face_normal = Triangle(new_mesh.vertices_[new_face_idx.x], new_mesh.vertices_[new_face_idx.z], new_mesh.vertices_[new_face_idx.y]).getNormal();
			new_mesh.normals_[new_face_idx.x] += face_normal;
			new_mesh.normals_[new_face_idx.y] += face_normal;
			new_mesh.normals_[new_face_idx.z] += face_normal;
			new_mesh.face_normals_.emplace_back(face_normal);
		}
		for (unsigned int idx = 0; idx < mesh.edge_faces_.size(); ++idx) {
			Vector3i face_idx = mesh.edge_faces_[idx];
            const Vector3 edge_face_normal = Triangle(mesh.vertices_[face_idx.x], mesh.vertices_[face_idx.z], mesh.vertices_[face_idx.y]).getNormal();
			for (int axis = 0; axis < 3; ++axis) {
				Vector3i vCorner = findOriginCorner(mesh.vertices_[face_idx[axis]], 0);
				std::size_t const voxelIndex = position2Index(vCorner, vVoxelSize);
				int i = vCorner.x, j = vCorner.y, k = vCorner.z;

				auto itr = new_voxelIndex2MeshIndex.find(voxelIndex);
				if (itr != new_voxelIndex2MeshIndex.end()) {
					new_mesh.normals_[itr->second] += edge_face_normal;
				}
			}
		}

		new_mesh.vertexValid_.resize(new_mesh.vertices_.size(), true);
		new_mesh.edge_faces_.resize(0);
	
		return pnew_mesh;
	}

	//with water
	void prepareTables() {
		for (int i0 = 0; i0 < 81; ++i0) {
			for (int i1 = 0; i1 < 81; ++i1) {
				int t[2][2][2] = {
					{
						{ i0 % 3, (i0 / 3) % 3 },
						{ (i0 / 9) % 3, (i0 / 27) % 3 }
					}, {
						{ i1 % 3, (i1 / 3) % 3 },
						{ (i1 / 9) % 3, (i1 / 27) % 3 }
					}
				};

				int edgemask = 0;
				for (int i = 0; i < 12; ++i) {
					const unsigned char(&e)[2][3] = kEdgeVertexTable[i];
					int p0x = e[0][0], p0y = e[0][1], p0z = e[0][2], p1x = e[1][0], p1y = e[1][1], p1z = e[1][2];
					if (t[p0x][p0y][p0z] != t[p1x][p1y][p1z]) {
						edgemask |= 1 << i;
					}
				}

				gEdgeTable[i0 + 81 * i1] = edgemask;
			}
		}
	}

	static Vector3 computePoint(const Vector3 &vsmooth, float cellSize, MeshType ematerialType, size_t seed) {
		Vector3 &&vcube_offset = Vector3(cellSize, cellSize, cellSize) * 0.2f;
		Vector3 &&vcenter = Vector3(cellSize, cellSize, cellSize) * 0.5f;
		switch (ematerialType)
		{
		case MeshType::MESH_CUBE:
			return vcube_offset;
		case MeshType::MESH_HALFSMOOTH:
		{
            Vector3 offset = (Vector3((seed & 255) / 255.f, ((seed >> 8) & 255) / 255.f, ((seed >> 16) & 255) / 255.f) * 2.f - Vector3(1)) * (0.3f * cellSize);
			offset += vsmooth;
            offset.x = MathFuncs::clamp(offset.x, 0.f, 1.f);
            offset.y = MathFuncs::clamp(offset.y, 0.f, 1.f);
            offset.z = MathFuncs::clamp(offset.z, 0.f, 1.f);
			return offset;
		}
		case MeshType::MESH_WATER:
            return (vsmooth.y >= 0.5f ? vsmooth : vsmooth + Vector3(0, 1, 0) * 0.5f);
        case MeshType::MESH_NOTSMOOTH:
            return round((vsmooth - vcenter) / 0.5f) * 0.5f + vcenter;
		default:
			break;
		}
		return vsmooth;
	}

	static std::pair<Vector3, MeshType> gen_avg_vertex_by_voxel(
		std::function<float(const Vector3&)> const& sdfFunction,
        std::function<MaterialType(const Vector3&)> const& materialFunction,
        std::function<std::pair<int, int>(const Vector3i&)> const& funVoxelOccupancyAndTag,
        const Vector3i &vVoxelSize,
		const int lod,
        const Vector3i &vposi,
		const int edgemask) {
        float cellSize = /*(1 << lod) **/ 1.0f;
		Vector3 &&corner = Vector3(vposi.x * cellSize, vposi.y * cellSize, vposi.z * cellSize);
        float nMCPointCount = 0;
        Vector3 vMCPointSum = Vector3(0);
		float occScale = 1.f / (1 << 8);
		// calculate vertices
        Vector3i p0i, p1i;
        Vector3 p0, p1;
		for (int i = 0; i < 12; ++i) {
			if (edgemask & (1 << i)) {
				const unsigned char(&e)[2][3] = kEdgeVertexTable[i];
                p0i = Vector3i(e[0][0], e[0][1], e[0][2]);
                p1i = Vector3i(e[1][0], e[1][1], e[1][2]);
				p0 = vector3i2Vector3(p0i);
				p1 = vector3i2Vector3(p1i);

				const std::pair<int, int> &g0 = funVoxelOccupancyAndTag(vposi + p0i);
				const std::pair<int, int> &g1 = funVoxelOccupancyAndTag(vposi + p1i);
				
				float t = g0.second > g1.second ? (g0.first + 1) * occScale : 1 - (g1.first + 1) * occScale;
                vMCPointSum += MathFuncs::Lerp(p0 * cellSize, p1 * cellSize, t);
				nMCPointCount += 1.0f;
			}
        }

		// get materials in 8 corners of one voxel
		bool bvertex_is_cube = false, bvertex_is_water = false;
		int nhalfsmooth_count = 0;
		MeshType emeshType = MeshType::MESH_UNKNOWN;
		for (int i = 0; i < 8; ++i) {
            Vector3 pos = vector3i2Vector3(vposi + Vector3i(kVertexIndexTable[i][0], kVertexIndexTable[i][1], kVertexIndexTable[i][2]));
            MaterialType eCurrentVoxelMaterial = materialFunction(pos);
			auto mtr = getBlockMaterial(eCurrentVoxelMaterial);
			if (mtr != MeshType::MESH_UNKNOWN) {
				if (mtr == MeshType::MESH_CUBE) {
					bvertex_is_cube = true;
				}
				else if (mtr == MeshType::MESH_WATER) {
					bvertex_is_water = true;
				}
				++nhalfsmooth_count;
			}
            else if (eCurrentVoxelMaterial == MaterialType::AIR) {
				++nhalfsmooth_count;
			}
		}
		if (emeshType != MeshType::MESH_WATER) {
			if (nhalfsmooth_count >= 8) {
				if (bvertex_is_cube) emeshType = MeshType::MESH_CUBE;
				else if (bvertex_is_water) emeshType = MeshType::MESH_WATER;
				else emeshType = MeshType::MESH_HALFSMOOTH;
			}
			else {
				emeshType = MeshType::MESH_SMOOTH;
			}
		}

		// change voxel pos in input buffer to voxel pos in its own block
		// here min_padding = 2, max_padding = 2, block size = 16. Keep an eye in here if these values be changed
        Vector3i vblock_posi = vposi - Vector3i(2, 2, 2);
		for (int axis = 0; axis < 3; ++axis) {
			if (vblock_posi[axis] < 0) vblock_posi[axis] += 16;
			else if (vblock_posi[axis] >= 16) vblock_posi[axis] -= 16;
		}
		size_t seed = computeSeed(vblock_posi);
		Vector3 &&point = computePoint(vMCPointSum / nMCPointCount, cellSize, emeshType, seed);
		Vector3 &&position = corner + point;
		
		if (std::floor(position.x) != vposi.x) position.x -= MathFuncs::LOWEPSILON;
		if (std::floor(position.y) != vposi.y) position.y -= MathFuncs::LOWEPSILON;
		if (std::floor(position.z) != vposi.z) position.z -= MathFuncs::LOWEPSILON;
		// for debug
		//if (position.x >= corner.x + 1 || position.y >= corner.y + 1 || position.z >= corner.z + 1
		//	|| position.x < corner.x || position.y < corner.y || position.z < corner.z) {
		//	LordLogError("SurfaceNets.gen_avg_vertex_by_voxel pos = (%f %f %f) outside the voxel", position.x, position.y, position.z);
		//}

		return std::pair<Vector3, MeshType>(position, emeshType);
	}

	static void gen_faces(VertexMesh &mesh,
        const Vector3i &vvoxel_size,
        const int lod,
        std::function<std::pair<int, int>(const Vector3i&)> const& funVoxelOccupancyAndTag,
		std::unordered_map<std::size_t, std::size_t> &mvoxelIndex2MeshIndex) {
        auto const getVertexIndexByLod = [&mesh, &mvoxelIndex2MeshIndex](const Vector3i& posi, const Vector3i &vvoxel_size, const int getLod) -> int {
            int nstep = (1 << getLod);
            int voxelIdx = position2Index(posi, vvoxel_size);
            if (mvoxelIndex2MeshIndex.find(voxelIdx) == mvoxelIndex2MeshIndex.end()) return 0;
            if (getLod == 0) return mvoxelIndex2MeshIndex[voxelIdx];

            if (mesh.mesh_type_[mvoxelIndex2MeshIndex[voxelIdx]] == MeshType::MESH_CUBE) return mvoxelIndex2MeshIndex[voxelIdx];
#ifdef LOD_OPTION_SECOND
            Vector3i vposi = (posi - 1) / nstep * nstep + 1;
#else //LOD_OPTION_FIRST
            // do not reduce vertex in edge since there is no lod info of neighbors
            if (posi.x <= 1 || posi.x >= vvoxel_size.x - 3 || posi.y <= 1 || posi.y >= vvoxel_size.y - 3 || posi.z <= 1 || posi.z >= vvoxel_size.z - 3) return mvoxelIndex2MeshIndex[voxelIdx];
            Vector3i vposi = (posi - 2) / nstep * nstep + 2;
#endif
            Vector3i vpos_cur;
            for (vpos_cur.x = vposi.x; vpos_cur.x <= posi.x; ++vpos_cur.x) {
                for (vpos_cur.y = vposi.y; vpos_cur.y <= posi.y; ++vpos_cur.y) {
                    for (vpos_cur.z = vposi.z; vpos_cur.z <= posi.z; ++vpos_cur.z) {
                        int nvoxelIdx = position2Index(vpos_cur, vvoxel_size);
                        if (mvoxelIndex2MeshIndex.find(nvoxelIdx) != mvoxelIndex2MeshIndex.end()) {
                            return mvoxelIndex2MeshIndex[nvoxelIdx];
                        }
                    }
                }
            }
            return mvoxelIndex2MeshIndex[voxelIdx];
        };

        int mesh_index_lod[8] = { 0 }, mesh_index_upper_lod[8] = { 0 };
        int nstep = (1 << lod);
        Vector3i posi, vposi;
        for (posi.y = 1; posi.y + 1 < vvoxel_size.y; ++posi.y) {
            for (posi.z = 1; posi.z + 1 < vvoxel_size.z; ++posi.z) {
                for (posi.x = 1; posi.x + 1 < vvoxel_size.x; ++posi.x) {
                    const std::pair<float, int> &v000 = funVoxelOccupancyAndTag(posi);
                    const std::pair<float, int> &v100 = funVoxelOccupancyAndTag(posi + Vector3i(1, 0, 0));
                    const std::pair<float, int> &v010 = funVoxelOccupancyAndTag(posi + Vector3i(0, 1, 0));
                    const std::pair<float, int> &v001 = funVoxelOccupancyAndTag(posi + Vector3i(0, 0, 1));
					
                    for (int i = 0; i < 8; ++i) {
                        vposi = posi - GeometryMath::vCubeVer[i];
                        mesh_index_lod[i] = getVertexIndexByLod(vposi, vvoxel_size, lod);
                        mesh_index_upper_lod[i] = getVertexIndexByLod(vposi, vvoxel_size, lod - 1);
                    }
                    vposi = (posi - 1) / nstep * nstep + 1;
                    // add quad faces
                    if (v000.second != v100.second) {
                        pushQuad(mesh, mesh_index_lod[0], mesh_index_lod[3], mesh_index_lod[7], mesh_index_lod[4], v000.second > v100.second, lod);
                        // add transition index, axis x
                        if (lod > 0 && isVoxelUpperBoundary(vposi, 1) || isVoxelUpperBoundary(posi, vvoxel_size.y - 3)) {
                            if (vposi.y == 1) {
                                pushTransition(mesh, mesh_index_lod[0], mesh_index_upper_lod[0], mesh_index_upper_lod[4], mesh_index_lod[4],
                                    lod, Cube_Y_Neg, v000.second > v100.second);
                            }
                            if (posi.z == vvoxel_size.y - 3) {
                                pushTransition(mesh, mesh_index_lod[3], mesh_index_upper_lod[3], mesh_index_upper_lod[0], mesh_index_lod[0],
                                    lod, Cube_Z_Pos, v000.second > v100.second);
                            }
                        }
                    }

                    if (v000.second != v010.second) {
                        pushQuad(mesh, mesh_index_lod[0], mesh_index_lod[1], mesh_index_lod[5], mesh_index_lod[4], v000.second < v010.second, lod);
                        // add transition index, axis y
                        if (lod > 0 && isVoxelUpperBoundary(vposi, 1) || isVoxelUpperBoundary(posi, vvoxel_size.y - 3)) {
                            if (vposi.z == 1) {
                                pushTransition(mesh, mesh_index_lod[1], mesh_index_upper_lod[1], mesh_index_upper_lod[0], mesh_index_lod[0],
                                    lod, Cube_Z_Neg, v000.second < v010.second);
                            }
                            if (posi.x == vvoxel_size.y - 3) {
                                pushTransition(mesh, mesh_index_lod[0], mesh_index_upper_lod[0], mesh_index_upper_lod[4], mesh_index_lod[4],
                                    lod, Cube_X_Pos, v000.second < v010.second);
                            }
                        }
                    }

                    if (v000.second != v001.second) {
                        pushQuad(mesh, mesh_index_lod[0], mesh_index_lod[1], mesh_index_lod[2], mesh_index_lod[3], v000.second > v001.second, lod);
                        // add transition index, axis z
                        if (lod > 0 && isVoxelUpperBoundary(vposi, 1) || isVoxelUpperBoundary(posi, vvoxel_size.y - 3)) {
                            if (vposi.x == 1) {
                                pushTransition(mesh, mesh_index_lod[0], mesh_index_upper_lod[0], mesh_index_upper_lod[1], mesh_index_lod[1],
                                    lod, Cube_X_Neg, v000.second > v001.second);
                            }
                            if (posi.y == vvoxel_size.y - 3) {
                                pushTransition(mesh, mesh_index_lod[0], mesh_index_upper_lod[0], mesh_index_upper_lod[3], mesh_index_lod[3],
                                    lod, Cube_Y_Pos, v000.second > v001.second);
                            }
                        }
                    }
                }
            }
        }
    }

    std::shared_ptr<VertexMesh> surface_nets_with_water(
		std::function<float(const Vector3&)> const& sdfFunction,
        std::function<MaterialType(const Vector3&)> const& materialFunction,
        std::function<std::pair<int, int>(const Vector3i&)> const& funVoxelOccupancyAndTag,
		std::function<int(const Vector3i&)> const& funVoxelTag,
        const Vector3i &vVoxelSize,
		const int lod) {
        std::shared_ptr<VertexMesh> pmesh = std::make_shared<VertexMesh>();
        //auto &&pmesh = get_from_pool<VertexMesh>();
		VertexMesh &mesh = *pmesh;
		mesh.reserve(1680);
		mesh.clear();

		prepareTables();
        std::shared_ptr<std::unordered_map<std::size_t, std::size_t>> pVoxelIndex2MeshIndex = std::make_shared<std::unordered_map<std::size_t, std::size_t>>();
        //auto &&pVoxelIndex2MeshIndex = get_from_pool<std::unordered_map<std::size_t, std::size_t>>();
		std::unordered_map<std::size_t, std::size_t> &voxelIndex2MeshIndex = *pVoxelIndex2MeshIndex;
		voxelIndex2MeshIndex.reserve(1680);
		voxelIndex2MeshIndex.clear();
		
        Vector3i vposi = Vector3i(0);
		for (vposi.y = 0; vposi.y + 1 < vVoxelSize.y; ++vposi.y) {
			for (vposi.z = 0; vposi.z + 1 < vVoxelSize.z; ++vposi.z) {
                int tagi0 = funVoxelTag(Vector3i(0, vposi.y, vposi.z))
                    + 3 * funVoxelTag(Vector3i(0, vposi.y, vposi.z + 1))
                    + 9 * funVoxelTag(Vector3i(0, vposi.y + 1, vposi.z))
                    + 27 * funVoxelTag(Vector3i(0, vposi.y + 1, vposi.z + 1));

				for (vposi.x = 0; vposi.x + 1 < vVoxelSize.x; ++vposi.x) {
                    int tagi1 = funVoxelTag(vposi + Vector3i(1, 0, 0))
                        + 3 * funVoxelTag(vposi + Vector3i(1, 0, 1))
                        + 9 * funVoxelTag(vposi + Vector3i(1, 1, 0))
                        + 27 * funVoxelTag(vposi + Vector3i(1));

					int edgemask = gEdgeTable[tagi0 + 81 * tagi1];
					tagi0 = tagi1;

					if (edgemask != 0) {
						auto info = gen_avg_vertex_by_voxel(sdfFunction, materialFunction, funVoxelOccupancyAndTag, vVoxelSize, lod, vposi, edgemask);
						voxelIndex2MeshIndex[position2Index(vposi, vVoxelSize)] = mesh.vertices_.size();

						mesh.vertices_.push_back(info.first);

                        Vector3i posFloor = MathFuncs::vector3FloorOrCeil(info.first);
                        uint32_t material = 0;
                        for (int i = 0; i < GeometryMath::vCubeVer.size(); ++i) {
                            MaterialType type = materialFunction((vposi + GeometryMath::vCubeVer[i]).to_vec3());
                            if (type != MaterialType::AIR)
                                material |= (1 << type);
                        }

                        mesh.materials_.push_back(material);
						mesh.mesh_type_.push_back(info.second);
						//mark border (invalid vertex)
						mesh.vertexValid_.push_back(!isVoxelLowerBoundary(vposi) && !isVoxelUpperBoundary(vposi, vVoxelSize.y - 2));
						//mark if share point
						mesh.share_point_.push_back(info.second == MeshType::MESH_SMOOTH || info.second == MeshType::MESH_WATER);
					}
				}
			}
		}
        gen_faces(mesh, vVoxelSize, lod, funVoxelOccupancyAndTag, voxelIndex2MeshIndex);
		return pmesh;
	}

    void separateSolidAndWaterFaces(std::shared_ptr<VertexMesh> pmesh, const int lod_index) {
        VertexMesh &meshWithWater = *pmesh;
        if (meshWithWater.faces_[lod_index].empty()) {
			return;
		}

		size_t nvertex_size = meshWithWater.vertices_.size();
        size_t nface_size = meshWithWater.faces_[lod_index].size();
		std::vector<uint8_t> bIsWater(nface_size);
		meshWithWater.normals_.resize(nvertex_size);
		meshWithWater.face_normals_.resize(nface_size);
		Vector3i face_index;
		//calculate normals
		for (size_t i = 0; i < nface_size; ++i) {
            face_index = meshWithWater.faces_[lod_index][i];
			meshWithWater.face_normals_[i] = (meshWithWater.vertices_[face_index.y] - meshWithWater.vertices_[face_index.x]).cross(meshWithWater.vertices_[face_index.z] - meshWithWater.vertices_[face_index.x]);
			meshWithWater.face_normals_[i].normalize();
			for (int j = 0; j < 3; ++j) {
				meshWithWater.normals_[face_index[j]] += meshWithWater.face_normals_[i];
			}
			bIsWater[i] = meshWithWater.isWater(face_index);
		}
		for (size_t i = 0; i < nvertex_size; ++i) {
			meshWithWater.normals_[i].normalize();
		}
		//separate solid and water faces
		for (size_t i = 0; i < nface_size; ++i) {
            face_index = meshWithWater.faces_[lod_index][i];
			if (meshWithWater.vertexValid_[face_index.x] && meshWithWater.vertexValid_[face_index.y] && meshWithWater.vertexValid_[face_index.z]) {
				if (isRepeatFace(MathFuncs::vector3FloorOrCeil(meshWithWater.vertices_[face_index.x], true),
					MathFuncs::vector3FloorOrCeil(meshWithWater.vertices_[face_index.y], true),
					MathFuncs::vector3FloorOrCeil(meshWithWater.vertices_[face_index.z], true), 1)) {
					continue;
				}

				if (bIsWater[i]) {
					meshWithWater.water_faces_.push_back(face_index);
				}
				else {
					meshWithWater.solid_faces_.push_back(face_index);
					if (bIsWater[i ^ 1]) {
                        meshWithWater.solid_faces_.push_back(Vector3i(face_index.x, face_index.z, face_index.y));
					}
				}
			}
		}
	}
}
