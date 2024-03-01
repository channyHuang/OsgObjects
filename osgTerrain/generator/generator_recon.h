#ifndef GENERATOR_RECON_H
#define GENERATOR_RECON_H

#include <vector>
#include <functional>
#include <chrono>
#include <fstream>
#include <queue>

#include "commonMath/vector3.h"
#include "commonMath/vector3i.h"
//#include "terrains/voxeltoolterrain.h"
#include "voxels/common_enum.h"
#include "commonMath/funcs.h"
#include "terrains/voxelBrush.h"

#include "solver/kdtree.h"

class Generator_Recon
{
public:

    Generator_Recon();
    ~Generator_Recon();

    static std::shared_ptr<Generator_Recon> getInstance();

    void setRange(const Vector3& vStart, const Vector3i& vSize) {
        _range.vStart = vStart - vSize.to_vec3() * 0.5f;
        _range.vSize = vSize;
        _range.vBox = Box(_range.vStart, _range.vStart + _range.vSize.to_vec3());
    }

    //void loadFile(const std::string& fileName);

public:
    struct {
        Vector3i vSize = Vector3i(100);
        Vector3 vStart = Vector3(0);
        Box vBox = Box(Vector3(-10.f), Vector3(10.f));
    } _range;

private:
    
    static std::shared_ptr<Generator_Recon> pInstance_;
    std::vector<Vector3> m_vVertex;
    std::vector<Vector3> m_vNormal;

    std::vector<vcg::Point3<float>> m_vPoints;
    std::vector<vcg::Point3<float>> m_vNormals;
};

#endif // TERRAINGENERATOR_RECON_H
