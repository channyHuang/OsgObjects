#include "MeshPostProcessing.h"

MeshPostProcessing* MeshPostProcessing::instance = nullptr;

static void SetPixel(uint8_t* dest, int destWidth, int x, int y, const uint8_t* color)
{
    uint8_t* pixel = &dest[(x + y * destWidth) * 4];
    pixel[0] = color[0];
    pixel[1] = color[1];
    pixel[2] = color[2];
    pixel[3] = color[3];
}

static void RasterizeTriangle(uint8_t* dest, int destWidth, const int* t0, const int* t1, const int* t2, const uint8_t* color)
{
    if (t0[1] > t1[1]) std::swap(t0, t1);
    if (t0[1] > t2[1]) std::swap(t0, t2);
    if (t1[1] > t2[1]) std::swap(t1, t2);
    int total_height = t2[1] - t0[1];
    for (int i = 0; i < total_height; i++) {
        bool second_half = i > t1[1] - t0[1] || t1[1] == t0[1];
        int segment_height = second_half ? t2[1] - t1[1] : t1[1] - t0[1];
        float alpha = (float)i / total_height;
        float beta = (float)(i - (second_half ? t1[1] - t0[1] : 0)) / segment_height;
        int A[2], B[2];
        for (int j = 0; j < 2; j++) {
            A[j] = int(t0[j] + (t2[j] - t0[j]) * alpha);
            B[j] = int(second_half ? t1[j] + (t2[j] - t1[j]) * beta : t0[j] + (t1[j] - t0[j]) * beta);
        }
        if (A[0] > B[0]) std::swap(A, B);
        for (int j = A[0]; j <= B[0]; j++)
            SetPixel(dest, destWidth, j, t0[1] + i, color);
    }
}

bool MeshPostProcessing::unwrap() {
    bProcessingFinish = false;
    if (sFileName.length() <= 0) sFileName = "E:/thirdLibs/CGAL-5.4-beta1/data/meshes/sphere.ply";
    output_mesh.clear();
    if (!CGAL::IO::read_polygon_mesh(sFileName.c_str(), output_mesh)) {
        std::cerr << "Error: read file " << sFileName << " points = " << output_mesh.number_of_vertices() << std::endl;
    }

    std::vector<float> position;

    for (vertex_descriptor vd : vertices(output_mesh)) {
        position.push_back(output_mesh.point(vd).x());
        position.push_back(output_mesh.point(vd).y());
        position.push_back(output_mesh.point(vd).z());
    }

    indices.clear();
    for (auto& f : output_mesh.faces()) {
        for (auto vi : CGAL::vertices_around_face(output_mesh.halfedge(f), output_mesh))
            indices.push_back(vi.idx());
    }

    atlas = xatlas::Create();
    xatlas::MeshDecl meshDecl;
    meshDecl.vertexCount = output_mesh.number_of_vertices();
    meshDecl.vertexPositionData = position.data();
    meshDecl.vertexPositionStride = sizeof(float) * 3;
    meshDecl.indexCount = indices.size();
    meshDecl.indexData = indices.data();
    meshDecl.indexFormat = xatlas::IndexFormat::UInt32;
    xatlas::AddMeshError error = xatlas::AddMesh(atlas, meshDecl, (uint32_t)1);
    if (error != xatlas::AddMeshError::Success) {
        xatlas::Destroy(atlas);
        return false;
    }

    xatlas::Generate(atlas);

    if (atlas->meshCount <= 0) return false;
    uvWidth = atlas->width;
    uvHeight = atlas->height;

    std::vector<int> positionColor;
    positionColor.resize(position.size());

    if (1) {
        chartsTextureData.clear();
        chartsTextureData.resize(atlas->width * atlas->height * 4);
        memset(chartsTextureData.data(), 0, chartsTextureData.size());
        // only deal with first mesh
        const xatlas::Mesh& mesh = atlas->meshes[0];
        
        for (uint32_t j = 0; j < mesh.chartCount; j++) {
            const xatlas::Chart* chart = &mesh.chartArray[j];
            uint8_t color[4] = { rand() % 256, rand() % 256, rand() % 256, 255 };
            // for each chart face
            for (uint32_t k = 0; k < chart->faceCount; k++) {
                int verts[3][2];
                // for each vertex in each chart face
                for (int l = 0; l < 3; l++) {
                    const xatlas::Vertex& v = mesh.vertexArray[mesh.indexArray[chart->faceArray[k] * 3 + l]];
                    verts[l][0] = int(v.uv[0]);
                    verts[l][1] = int(v.uv[1]);
                    for (int m = 0; m < 3; ++m) {
                        positionColor[v.xref * 3 + m] = color[m];
                    }
                }
                RasterizeTriangle(chartsTextureData.data(), atlas->width, verts[0], verts[1], verts[2], color);
            }
        }
    }

    if (1) {
        std::ofstream ofs("chartColor.ply");
        ofs << "ply\nformat ascii 1.0\nelement vertex " << output_mesh.number_of_vertices() <<
            "\nproperty double x\nproperty double y\nproperty double z\n";// property uchar red\nproperty uchar green\nproperty uchar blue\n";
        ofs << "element face " << indices.size() / 3 << std::endl;
        ofs << "property list uchar int vertex_indices\n";
        ofs << "property uchar red\nproperty uchar green\nproperty uchar blue" << std::endl;
        ofs << "end_header\n";

        for (vertex_descriptor vd : vertices(output_mesh)) {
            ofs << output_mesh.point(vd).x()  << " " << output_mesh.point(vd).z() << " " << output_mesh.point(vd).y() << " ";
            //ofs << positionColor[vd] << " " << positionColor[vd + 1] << " " << positionColor[vd + 2];
            ofs << std::endl;
        }
        for (auto& f : output_mesh.faces()) {
            ofs << "3 ";
            int idx = 0;
            for (auto vi : CGAL::vertices_around_face(output_mesh.halfedge(f), output_mesh)) {
                ofs << vi.idx() << " ";
                idx = vi.idx();
            }
            ofs << positionColor[idx * 3] << " " << positionColor[idx * 3 + 1] << " " << positionColor[idx * 3 + 2] << std::endl;
        }
        ofs.close();
    }

    xatlas::Destroy(atlas);
    bProcessingFinish = true;
    return true;
}

bool MeshPostProcessing::unwrap(const std::string& sFileName, std::vector<std::pair<float, float>>& uvs) {
    std::vector<float> position;

    if (!CGAL::IO::read_polygon_mesh(sFileName.c_str(), output_mesh)) {
        std::cerr << "Error: read file " << sFileName << " points = " << output_mesh.number_of_vertices() << std::endl;
    }

    for (vertex_descriptor vd : vertices(output_mesh)) {
        position.push_back(output_mesh.point(vd).x());
        position.push_back(output_mesh.point(vd).y());
        position.push_back(output_mesh.point(vd).z());
    }

    indices.clear();
    for (auto& f : output_mesh.faces()) {
        for (auto vi : CGAL::vertices_around_face(output_mesh.halfedge(f), output_mesh))
            indices.push_back(vi.idx());
    }

    atlas = xatlas::Create();
    xatlas::MeshDecl meshDecl;
    meshDecl.vertexCount = output_mesh.number_of_vertices();
    meshDecl.vertexPositionData = position.data();
    meshDecl.vertexPositionStride = sizeof(float) * 3;
    meshDecl.indexCount = indices.size();
    meshDecl.indexData = indices.data();
    meshDecl.indexFormat = xatlas::IndexFormat::UInt32;
    xatlas::AddMeshError error = xatlas::AddMesh(atlas, meshDecl, (uint32_t)1);
    if (error != xatlas::AddMeshError::Success) {
        xatlas::Destroy(atlas);
        return false;
    }

    xatlas::Generate(atlas);

    if (atlas->meshCount <= 0) return false;

    uvs.resize(position.size());
    const xatlas::Mesh& mesh = atlas->meshes[0];
    float invWidth = 1.f / atlas->width;
    float invHeight = 1.f / atlas->height;
    for (uint32_t j = 0; j < mesh.chartCount; j++) {
        const xatlas::Chart* chart = &mesh.chartArray[j];
        for (uint32_t k = 0; k < chart->faceCount; k++) {
            for (int l = 0; l < 3; l++) {
                const xatlas::Vertex& v = mesh.vertexArray[mesh.indexArray[chart->faceArray[k] * 3 + l]];
                uvs[v.xref] = std::make_pair(v.uv[0] * invWidth, v.uv[1] * invHeight);
            }
        }
    }
}