#include "generator_recon.h"

std::shared_ptr<Generator_Recon> Generator_Recon::pInstance_ = nullptr;

std::shared_ptr<Generator_Recon> Generator_Recon::getInstance() {
   if (pInstance_ == nullptr) {
       pInstance_ = std::make_shared<Generator_Recon>();
   }
   return pInstance_;
}

Generator_Recon::Generator_Recon() {

}

Generator_Recon::~Generator_Recon() {}

// type 0: adverage; 1: min; 2: max
float computeAdverageSpacing(std::vector<vcg::Point3<float>>& vPoints, int type = 0) {
    vcg::KdTree<float> tree(vcg::ConstDataWrapper<vcg::Point3<float>>(vPoints.data(), vPoints.size()));
    vcg::KdTree<float>::PriorityQueue nq;
    int k = 10;
    float fAdverageSpace = 0;
    switch (type) {
    case 1:
    {
        float fSpace = (std::numeric_limits<float>::max)();
        for (auto &p : vPoints) {
            tree.doQueryK(p, 2, nq);
            if (fSpace > nq.getTopWeight()) fSpace = nq.getTopWeight();
        }
        fAdverageSpace = std::sqrt(fSpace);
    }
    break;
    case 2:
    {
        float fSpace = 0;
        for (auto &p : vPoints) {
            tree.doQueryK(p, 2, nq);
            if (fSpace < nq.getTopWeight()) fSpace = nq.getTopWeight();
        }
        fAdverageSpace = std::sqrt(fSpace);
    }
    break;
    case 0:
    default:
    {
        for (auto &p : vPoints) {
            float fSpace = 0;
            tree.doQueryK(p, k, nq);
            int neighbours = nq.getNofElements();
            for (int j = 0; j < neighbours; j++)
            {
                fSpace += std::sqrt(nq.getWeight(j));
            }
            fAdverageSpace += (fSpace / k);
        }
        fAdverageSpace /= vPoints.size();
    }
    break;
    }

    return fAdverageSpace;
}

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vcg/complex/allocate.h>
#include <vcg/complex/algorithms/pointcloud_normal.h>
#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/export_obj.h>

//#include "commonCGAL.h"

class MyEdge;
class MyFace;
class MyVertex;
struct MyUsedTypes : public vcg::UsedTypes<vcg::Use<MyVertex>   ::AsVertexType,
    vcg::Use<MyEdge>     ::AsEdgeType,
    vcg::Use<MyFace>     ::AsFaceType> {};

class MyVertex : public vcg::Vertex<MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::BitFlags  > {};
class MyFace : public vcg::Face< MyUsedTypes, vcg::face::FFAdj, vcg::face::VertexRef, vcg::face::BitFlags > {};
class MyEdge : public vcg::Edge<MyUsedTypes> {};
class MyMesh : public vcg::tri::TriMesh< std::vector<MyVertex>, std::vector<MyFace>, std::vector<MyEdge>  > {};
typedef typename MyMesh::VertexType::CoordType   Point3x;

Vector3 real2grid(Vector3 pos, Vector3 center, Vector3 space) {
    return (pos - center) / space;
}

Vector3 grid2real(Vector3 pos, Vector3 center, Vector3 space) {
    return (pos * space) + center;
}
/*
void Generator_Recon::loadFile(const std::string& fileName) {
    Surface_mesh mesh;
    int len = fileName.length();
    std::string refFileName = fileName.substr(0, len - 4) + "_res.ply";
    if (!CGAL::IO::read_polygon_mesh(refFileName, mesh)) {
        std::cout << " load failed " << std::endl;
    }

    Tree ctree(faces(mesh).first, faces(mesh).second, mesh);

    const aiScene* scene = aiImportFile(fileName.c_str(), aiProcess_GenUVCoords);
    if (scene == nullptr) return;

    for (int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];

        if (mesh->mNumVertices) {

            for (int i = 0; i < mesh->mNumVertices; ++i) {
                Vector3 p(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
                m_vVertex.push_back(p);
                m_vPoints.push_back(vcg::Point3f(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
                if (mesh->HasNormals()) {
                    Vector3 n(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
                    m_vNormal.push_back(n);
                }
            }
        }
    }

    if (m_vNormal.size() < m_vVertex.size()) {
        m_vNormal.clear();
        MyMesh vcgMesh;
        for (auto& p : m_vPoints) {
            vcg::tri::Allocator<MyMesh>::AddVertex(vcgMesh, p);
        }

        vcg::tri::PointCloudNormal<MyMesh>::Param par;
        par.fittingAdjNum = 10;
        par.useViewPoint = true;
        vcg::tri::PointCloudNormal<MyMesh>::Compute(vcgMesh, par);
        vcg::tri::io::ExporterOBJ<MyMesh>::Save(vcgMesh, "recon_normal.obj", vcg::tri::io::Mask::IOM_VERTNORMAL);

        m_vVertex.clear();
        m_vPoints.clear();
        for (auto& ver : vcgMesh.vert) {
            m_vPoints.push_back(ver.P());
            m_vVertex.push_back(Vector3(ver.P().X(), ver.P().X(), ver.P().Z()));
            m_vNormal.push_back(Vector3(0) - Vector3(ver.N().X(), ver.N().Y(), ver.N().Z()));
        }
    }

    vcg::KdTree<float> tree(vcg::ConstDataWrapper<vcg::Point3<float>>(m_vPoints.data(), m_vPoints.size()));
    float fMinSpace = computeAdverageSpacing(m_vPoints, 1);
    vcg::Box3<float> box = tree._getAABBox();
    int nMaxDimIdx = box.MaxDim();

    Vector3 vCenter = Vector3(box.Center().X(), box.Center().Y(), box.Center().Z());

    Box oBox;
    oBox.vMin = Vector3(box.min.X(), box.min.Y(), box.min.Z());
    oBox.vMax = Vector3(box.max.X(), box.max.Y(), box.max.Z());
   
    std::cout << __FUNCTION__ << " " << fMinSpace << " (" << box.min.X() << " " << box.min.Y() << " " << box.min.Z() << ") (" << box.max.X() << " " << box.max.Y() << " " << box.max.Z() << " )" << std::endl;
    
    float density = (box.DimX() * box.DimY() + box.DimX() * box.DimZ() + box.DimZ() * box.DimY() ) * 2.f / m_vVertex.size();
    float space = std::sqrt(density);
    Vector3 vSpace(space);
    Box vBox(real2grid(oBox.vMin, vCenter, vSpace), real2grid(oBox.vMax, vCenter, vSpace));

    std::cout << __FUNCTION__ << " " << vSpace.toString().c_str() << " " << vBox.vMin.toString().c_str() << " " << vBox.vMax.toString().c_str() << std::endl;

    VoxelBrush* brush = new VoxelBrush(TerrainManager::getInstance(), VoxelMap::getInstance());

    Vector3i&& vmin_pos = MathFuncs::vector3FloorOrCeil(vBox.vMin, true);
    Vector3i&& vmax_pos = MathFuncs::vector3FloorOrCeil(vBox.vMax, false);

    Vector3 pos;
    for (pos.x = vmin_pos.x; pos.x <= vmax_pos.x; ++pos.x) {
        for (pos.z = vmin_pos.z; pos.z <= vmax_pos.z; ++pos.z) {
            for (pos.y = vmin_pos.y; pos.y <= vmax_pos.y; ++pos.y) {
                Vector3 opos = grid2real(pos, vCenter, vSpace);

                unsigned int idx = INT_MAX;
                float dist = 0;
                tree.doQueryClosest(vcg::Point3f(opos.x, opos.y, opos.z), idx, dist);
                Vector3 neighbor = real2grid(m_vVertex[idx], vCenter, vSpace);
                float fSdfValue = pos.distanceTo(neighbor);

                float fsdf = fSdfValue;
                if (0) {
                    float fDotValue = (pos - neighbor).dot(m_vNormal[idx]);
                    fsdf = (fDotValue > 0 ? fSdfValue : -fSdfValue);
                }

             
                Point_3 a(opos.x, opos.y, opos.z);
                Point_3 b(m_vVertex[idx].x, m_vVertex[idx].y, m_vVertex[idx].z);

                Vector3 dir = m_vVertex[idx] - opos;
                dir = dir * (vBox.getDMax() * 2.f);

                Vector3 ae = opos + dir;

                Kernel::Segment_3 query(a, Point_3(ae.x, ae.y, ae.z));
                std::list<Segment_intersection> out;
                ctree.all_intersections(query, std::back_inserter(out));
                int count = out.size();
                if (count & 1) {
                    fsdf = -fsdf;
                }

                if (fsdf > 2) continue;
                brush->set_voxel_f(pos, fsdf, VoxelBuffer::CHANNEL_SDF);
                if (fsdf <= 0) brush->set_voxel(pos, 1, VoxelBuffer::CHANNEL_TYPE);
            }
        }
    }
    std::cout << __FUNCTION__ << " end " << std::endl;
}
*/
