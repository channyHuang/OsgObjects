#include "terraingraphgenerator.h"

MapGenerator::MapGenerator(int nSizex, int nSizey, float fResolution) :
                               m_fResolution(fResolution)
{
    if (m_fResolution <= 0) m_fResolution = 1.0f;
    if (nSizex < m_fResolution || nSizey < m_fResolution) {
        nSizex = nSizey = (int)std::ceil(m_fResolution);
    }
    m_vSize = Vector2i(nSizex, nSizey);
    m_cBoundingBox = Box(0.f, 0.f, -1.f, (float)m_vSize.x, (float)m_vSize.y, 1.f);
}

void MapGenerator::initialize() {
    //set seed
    unsigned seeds = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
    srand(seeds);

    initializeVoronoiData();
    initializeMapData();
    m_bInitialized = true;

    //init
    initializeHeightMap();
    //erode
    erode(0.1, 1);
}

void MapGenerator::smoothHeightMap(SmoothType eSmoothType) {
    if (!m_bInitialized) {
        return;
    }
    switch (eSmoothType) {
    case SmoothType::Normalize:
        m_vHeightMap.normalize();
        break;
    case SmoothType::Round:
        m_vHeightMap.round();
        break;
    case SmoothType::Relax:
        m_vHeightMap.relax();
        break;
    default:
        break;
    }
}

void MapGenerator::addHill(float px, float py, float r, float height) {
    if (!m_bInitialized) {
        return;
    }
    // Create a rounded hill where height falls off smoothly
    // coefficients for the Wyvill kernel function
    float rsq = r * r;
    float coef1 = (4.0f / 9.0f)*(1.0f / (r*r*r*r*r*r));
    float coef2 = (17.0f / 9.0f)*(1.0f / (r*r*r*r));
    float coef3 = (22.0f / 9.0f)*(1.0f / (r*r));

    Vector2 v;
    for (unsigned int i = 0; i < m_vHeightMap.size(); i++) {
        v = m_vVertexMap.vertices[i].vPos;
        Vector2 vDist = v - Vector2(px, py);
        float dsq = vDist.lenSqr();
        if (dsq < rsq) {
            float kernel = 1.0f - coef1*dsq*dsq*dsq + coef2*dsq*dsq - coef3*dsq;
            float hval = m_vHeightMap(i);
            m_vHeightMap.set(i, hval + height*kernel);
        }
    }
}

void MapGenerator::addCone(float px, float py, float radius, float height) {
    if (!m_bInitialized) {
        return;
    }
    //    Create a cone where height falls off linearly
    float invradius = 1.0f / radius;
    float rsq = radius * radius;
    Vector2 v;
    for (unsigned int i = 0; i < m_vHeightMap.size(); i++) {
        v = m_vVertexMap.vertices[i].vPos;
        float dx = v.x - px;
        float dy = v.y - py;
        float dsq = dx*dx + dy*dy;
        if (dsq < rsq) {
            float dist = sqrt(dsq);
            float kernel = 1.0f - dist * invradius;
            float hval = m_vHeightMap(i);
            m_vHeightMap.set(i, hval + height*kernel);
        }
    }
}

/*
    Add a slope that runs parallel to the line with direction (dirx, diry)
    and position (px, py). (dirx, diry) must be a unit vector. The slope
    varies in height linearly from 0 to height from the left side of the
    direction vector to the right side of the direction vector.
*/
void MapGenerator::addSlope(float px, float py, float dirx, float diry,
                                 float radius, float height) {
    if (!m_bInitialized) {
        return;
    }

    Vector2 v;
    for (unsigned int i = 0; i < m_vHeightMap.size(); i++) {
        v = m_vVertexMap.vertices[i].vPos;
        Vector2 vDir = Vector2(dirx, diry);
        Vector2 vDist = Vector2(px, py) - v;
        float fDistDot = vDist.dot(vDir);
        Vector2 vNewDist = vDist - vDir * fDistDot;
        float dist = fmin(vNewDist.len(), radius);
        float fCross = vDist.cross(vDir);

        float minn = 0.5f * height;
        float maxn = (fCross < 0 ? 0 : height);

        float fieldval = minn + (dist / radius)*(maxn - minn);
        float hval = m_vHeightMap(i);
        m_vHeightMap.set(i, hval + fieldval);
    }
}

bool MapGenerator::erode(float amount, int iterations)  {
    if (!m_bInitialized) {
        return false;
    }

    if (iterations <= 0) {
        return true;
    }

    NodeMap<float> erosionMap(&m_vVertexMap, 0.0f);
    calculateErosionMap(erosionMap);

    for (unsigned int i = 0; i < m_vHeightMap.size(); i++) {
        float currlevel = m_vHeightMap(i);
        float newlevel = currlevel - amount * erosionMap(i);
        m_vHeightMap.set(i, newlevel);
    }

    m_bHeightMapEroded = true;

    return erode(amount, iterations - 1);
}

void MapGenerator::addCity(std::string cityName, std::string territoryName) {
    if (!m_bInitialized) {
        return;
    }

    if (!m_bHeightMapEroded) {
        erode(0.0);
    }

    City city;
    city.cityName = cityName;
    city.territoryName = territoryName;
    getCityLocation(city);

    updateCityMovementCost(city);
    m_vCities.push_back(city);
}

void MapGenerator::genVoronoiFaceVertices() {
    faceVertices.clear();
    genVoronoiFaceVertices(m_cVoronoi, m_nFaceSize, m_nVerticesSize, faceVertices);
}

void MapGenerator::genVoronoiFaceVertices(GraphGeometry &cVoronoi, int &nFaceSize, int &nVerticesSize, std::vector<std::vector<Vector2> > &vFaceVertices) {
    vFaceVertices.clear();
    nFaceSize = (int)cVoronoi.faces.size();
    nVerticesSize = 0;
    for (unsigned int i = 0; i < cVoronoi.faces.size(); i++) {
        Face f = cVoronoi.faces[i];
        if (f.outerComponent == -1) {
            continue;
        }
        HalfEdge h = cVoronoi.outerComponent(f);
        int startRef = h.id;
        std::vector<Vector2> vertices;
        Vertex2D v;
        do {
            v = cVoronoi.origin(h);
            vertices.push_back(v.vPos);
            h = cVoronoi.next(h);
        } while (h.id != startRef);
        vFaceVertices.push_back(vertices);

        if (vertices.size() >= 3) {
            nVerticesSize += (int)vertices.size();
        }
        else {
            nFaceSize--;
        }
    }
}

inline int getPointInConvexPolygonIndex(const Vector2 &pos, const std::vector<std::vector<Vector2> > &vFaceVertices) {
    unsigned int j = 0;
    for (; j < vFaceVertices.size(); j++) {
        if (GeometryMath::PointInConvexPolygon(pos, (int)vFaceVertices[j].size(), vFaceVertices[j]) > 0) break;
    }
    if (j >= vFaceVertices.size()) {
        return -1;
    }
    return j;
}

void MapGenerator::getCrudeHeight(const Box &cBox, std::vector<std::vector<float> > &vHeights,
    float fMaxHeight, float fStrength, DirectionType eNormalDirectionType) {
    Vector3i vMinPos = cBox.vMin.getFloor();
    Vector3i vMaxPos((int)std::ceil(cBox.vMax.x), (int)std::ceil(cBox.vMax.y), (int)std::ceil(cBox.vMax.z));
    Vector3i vSize = vMaxPos - vMinPos + Vector3i(1);

    switch (eNormalDirectionType) {
    case Direction_Y:
        vHeights.resize(vSize.x, std::vector<float>(vSize.z, 0.f));
        break;
    case Direction_X:
        vHeights.resize(vSize.y, std::vector<float>(vSize.z, 0.f));
        break;
    case Direction_Z:
        vHeights.resize(vSize.x, std::vector<float>(vSize.y, 0.f));
    default:
        break;
    }
    for (unsigned int x = 0; x < vHeights.size(); x++) {
        for (unsigned int z = 0; z < vHeights[0].size(); z++) {
            Vector2 pos;
            switch (eNormalDirectionType) {
            case Direction_Y:
                pos = Vector2((float)x + std::floor(cBox.vMin.x), (float)z + std::floor(cBox.vMin.z));
                break;
            case Direction_X:
                pos = Vector2((float)x + std::floor(cBox.vMin.y), (float)z + std::floor(cBox.vMin.z));
                break;
            case Direction_Z:
                pos = Vector2((float)x + std::floor(cBox.vMin.x), (float)z + std::floor(cBox.vMin.y));
                break;
            default:
                break;
            }

            int idx = getPointInConvexPolygonIndex(pos, faceVertices);
            if (idx == -1) {
                vHeights[x][z] = 0;
                continue;
            }

            float fMinDist = MathFuncs::POS_INFINITY;
            for (unsigned int k = 0; k < faceVertices[idx].size(); k++) {
                fMinDist = std::min(fMinDist,
                    GeometryMath::distancePointSegment(pos, faceVertices[idx][k], faceVertices[idx][(k + 1) % faceVertices[idx].size()]));
            }

            float height = 0;
            if (m_eCrudeType == Rough_Polygon_Down) {
                height = (fMinDist < fStrength ? std::max(0.5f, fMinDist) * fMaxHeight : 0);
            }
            else if (m_eCrudeType == Rough_Polygon_Up) {
                height = (fMinDist < fStrength ? std::pow(fMinDist, 0.5f) * fMaxHeight : std::pow(fMinDist, 0.2f) * fMaxHeight);
            }
            vHeights[x][z] = height;
        }
    }
}

void MapGenerator::outputVoronoiDiagram(int nFaceSize, int nVerticesSize,
    std::vector<std::vector<Vector2>> &faceVertices, std::vector<float> &facecolors,
    std::string filename) {

    if (facecolors.size() > 0) {
        getRiverDrawData(m_vRiverData);
    }
    //output
    std::ofstream ofs(filename.c_str());
    //header
    ofs << "ply\nformat ascii 1.0" << std::endl;
    ofs << "element Vertex2D " << nVerticesSize << std::endl;
    ofs << "property float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue" << std::endl;
    ofs << "element face " << nFaceSize << "\nproperty list uchar int vertex_index" << std::endl;
    ofs << "end_header" << std::endl;
    //vertices
    int index = 0;
    for (unsigned int i = 0; i < m_cVoronoi.faces.size(); i++) {
        if (faceVertices[i].size() < 3) continue;
        for (unsigned int j = 0; j < faceVertices[i].size(); j++) {
            ofs << faceVertices[i][j].x << " 0 " << faceVertices[i][j].y << " ";

            int nColor = (facecolors.size() > 0 ? (int)(facecolors[i] * 255) : 255);
            unsigned int x, y;
            for (x = 0; x < m_vRiverData.size(); x++) {
                for (y = 0; y < m_vRiverData[x].size(); y++) {
                    if (GeometryMath::PointInConvexPolygon(m_vRiverData[x][y], (int)faceVertices[i].size(), faceVertices[i]) > 0) {
                        ofs << "255 0 0" << std::endl;
                        break;
                    }
                }
                if (y < m_vRiverData[x].size()) {
                    break;
                }
            }
            if (x >= m_vRiverData.size()) {
                ofs << nColor << " " << nColor << " " << nColor << std::endl;
            }
        }
    }
    //face
    for (unsigned int i = 0; i < m_cVoronoi.faces.size(); i++) {
        if (faceVertices[i].size() < 3) continue;

        ofs << faceVertices[i].size() << " ";
        for (unsigned int j = 0; j < faceVertices[i].size(); j++) {
            ofs << index + j << " ";
        }
        ofs << std::endl;
        index += (int)faceVertices[i].size();
    }

    ofs.close();
}

bool MapGenerator::getNormalizedHeightMap(std::vector<float> &facecolors) {
    if (!m_bInitialized) {
        return false;
    }

    facecolors = computeFaceValues(m_vHeightMap);
    float min = facecolors[0];
    float max = facecolors[0];
    for (unsigned int i = 0; i < facecolors.size(); i++) {
        min = fmin(min, facecolors[i]);
        max = fmax(max, facecolors[i]);
    }

    for (unsigned int i = 0; i < facecolors.size(); i++) {
        facecolors[i] = (facecolors[i] - min) / (max - min);
    }

    return true;
}

void MapGenerator::setMapType(DisableMapType eType, bool bDisable) {
    if (eType < Slope || eType >= MaxMapType) return;
    if (bDisable) {
        m_nDisabledMapType |= (1 << eType);
    } else if (m_nDisabledMapType & (1 << eType)) {
        m_nDisabledMapType ^= (1 << eType);
    }
}

void MapGenerator::initializeVoronoiData() {
    float pad = m_fSamplePadFactor * m_fResolution;
    Box sampleExtents(m_cBoundingBox.vMin.x - pad, m_cBoundingBox.vMin.y - pad, -1,
                            m_cBoundingBox.vMax.x + pad, m_cBoundingBox.vMax.y + pad, 1);

    std::vector<Vector2> samples;
    samples = PoissonDiscSampler::generateSamples(sampleExtents,
                                                  m_fResolution,
                                                  m_nPoissonSamplerKValue);

    Graph_Geometry::Voronoi cVoronoiGenerator;
    m_cVoronoi = cVoronoiGenerator.voronoi(samples);
}

void MapGenerator::initializeHeightMap(int nNumOfHills) {
    float pad = 5.0;
    Box expandedExtents(m_cBoundingBox.vMin.x - pad, m_cBoundingBox.vMin.y - pad, -1,
        m_cBoundingBox.vMax.x + pad, m_cBoundingBox.vMax.y + pad, 1);
    Vector2 vRadiusRange(1.0, 8.0);

    //add hill or cone
    for (int i = 0; i < nNumOfHills; i++) {
        Vector2 p = GeometryMath::randomPoint(expandedExtents);
        float r = MathFuncs::IntervalRandom(vRadiusRange.x, vRadiusRange.y);
        float strength = MathFuncs::IntervalRandom(0.5f, 1.5);
        if (MathFuncs::IntervalRandom(0, 1) > 0.5) {
            addHill(p.x, p.y, r, strength);
        } else {
            addCone(p.x, p.y, r, strength);
        }
    }

    if (MathFuncs::IntervalRandom(0, 1) > 0.5) {
        Vector2 p = GeometryMath::randomPoint(expandedExtents);
        float r = MathFuncs::IntervalRandom(6.0f, 12.0);
        float strength = MathFuncs::IntervalRandom(1.0f, 3.0);
        addCone(p.x, p.y, r, strength);
    }

    //add slope
    if (false && MathFuncs::IntervalRandom(0, 1) > 0.1) {
        Vector2 dir = GeometryMath::randomDirection();
        Vector2 lp = GeometryMath::randomPoint(m_cBoundingBox);
        float slopewidth = MathFuncs::IntervalRandom(0.5f, 5.0);
        float strength = MathFuncs::IntervalRandom(2.0f, 3.0);
        addSlope(lp.x, lp.y, dir.x, dir.y, slopewidth, strength);
    }

    //smooth
    {
        if (MathFuncs::IntervalRandom(0, 1) > 0.5) {
            smoothHeightMap(SmoothType::Normalize);
        } else {
            smoothHeightMap(SmoothType::Round);
        }

        if (MathFuncs::IntervalRandom(0, 1) > 0.5) {
            smoothHeightMap(SmoothType::Relax);
        }
    }
}

void MapGenerator::initializeMapData() {
    //Initializing map data...
    m_vVertexMap = VertexMap(&m_cVoronoi, m_cBoundingBox);
    m_vHeightMap = NodeMap<float>(&m_vVertexMap, 0);
    initializeNeighbourMap();
    initializeFaceNeighbours();
    initializeFaceVertices();
    initializeFaceEdges();
}

void MapGenerator::initializeNeighbourMap() {
    m_vNeighbourMap = NodeMap<std::vector<int> >(&m_vVertexMap);
    std::vector<int> indices;
    for (unsigned int i = 0; i < m_vNeighbourMap.size(); i++) {
        indices.clear();
        m_vVertexMap.getNeighbourIndices(m_vVertexMap.vertices[i], indices);
        m_vNeighbourMap.set(i, indices);
    }
}

void MapGenerator::initializeFaceNeighbours() {
    m_vFaceNeighbours.reserve(m_cVoronoi.faces.size());
    Face f;
    HalfEdge h;
    std::vector<HalfEdge> outerComponents;
    std::vector<int> faceindices;
    for (unsigned int i = 0; i < m_cVoronoi.faces.size(); i++) {
        f = m_cVoronoi.faces[i];

        outerComponents.clear();
        m_cVoronoi.getOuterComponents(f, outerComponents);
        faceindices.clear();
        for (unsigned int nidx = 0; nidx < outerComponents.size(); nidx++) {
            h = m_cVoronoi.twin(outerComponents[nidx]);
            int nfidx = h.incidentFace;
            if (nfidx != -1) {
                faceindices.push_back(nfidx);
            }
        }

        m_vFaceNeighbours.push_back(std::vector<int>(faceindices.size(), -1));
        for (unsigned int idx = 0; idx < faceindices.size(); idx++) {
            m_vFaceNeighbours[i][idx] = faceindices[idx];
        }
    }
}

void MapGenerator::initializeFaceVertices() {
    m_vFaceVertices.reserve(m_cVoronoi.faces.size());
    Face f;
    Vertex2D v;
    std::vector<HalfEdge> edges;
    for (unsigned int i = 0; i < m_cVoronoi.faces.size(); i++) {
        f = m_cVoronoi.faces[i];

        edges.clear();
        m_cVoronoi.getOuterComponents(f, edges);
        m_vFaceVertices.push_back(std::vector<int>(edges.size(), -1));
        for (unsigned int eidx = 0; eidx < edges.size(); eidx++) {
            v = m_cVoronoi.origin(edges[eidx]);
            m_vFaceVertices[i][eidx] = v.id;
        }
    }
}

void MapGenerator::initializeFaceEdges() {
    m_vFaceEdges.reserve(m_cVoronoi.faces.size());
    Face f;
    std::vector<HalfEdge> edges;
    for (unsigned int i = 0; i < m_cVoronoi.faces.size(); i++) {
        f = m_cVoronoi.faces[i];

        edges.clear();
        m_cVoronoi.getOuterComponents(f, edges);
        m_vFaceEdges.push_back(std::vector<int>(edges.size(), -1));
        for (unsigned int eidx = 0; eidx < edges.size(); eidx++) {
            m_vFaceEdges[i][eidx] = edges[eidx].id;
        }
    }
}

std::vector<float> MapGenerator::computeFaceValues(NodeMap<float> &heightMap) {
    std::vector<float> faceheights;
    faceheights.reserve(m_cVoronoi.faces.size());

    Vertex2D v;
    for (unsigned int j = 0; j < m_cVoronoi.faces.size(); j++) {
        float sum = 0.0;
        for (unsigned int i = 0; i < m_vFaceVertices[j].size(); i++) {
            v = m_cVoronoi.getVertex(m_vFaceVertices[j][i]);
            if (heightMap.isNode(v)) {
                sum += heightMap(v);
            }
        }
        float avg = sum / m_vFaceVertices[j].size();
        faceheights.push_back(avg);
    }

    return faceheights;
}

std::vector<Vector2> MapGenerator::computeFacePositions() {
    std::vector<Vector2> positions;
    positions.reserve(m_cVoronoi.faces.size());

    Vector2 p;
    for (unsigned int j = 0; j < m_cVoronoi.faces.size(); j++) {
        positions.push_back(computeFacePosition(j));
    }

    return positions;
}

Vector2 MapGenerator::computeFacePosition(int fidx) {
    Vector2 vSum = Vector2(0, 0);
    Vector2 p;
    for (unsigned int i = 0; i < m_vFaceVertices[fidx].size(); i++) {
        p = m_cVoronoi.getVertex(m_vFaceVertices[fidx][i]).vPos;
        vSum += p;
    }
    return vSum / (float)m_vFaceVertices[fidx].size();
}

bool MapGenerator::isEdgeInMap(HalfEdge &h) {
    Vertex2D v1 = m_cVoronoi.origin(h);
    Vertex2D v2 = m_cVoronoi.origin(m_cVoronoi.twin(h));

    return m_vVertexMap.isVertex(v1) && m_vVertexMap.isVertex(v2);
}

bool MapGenerator::isContourEdge(HalfEdge &h,
                                       std::vector<float> &faceheights,
                                       float isolevel) {
    Face f1 = m_cVoronoi.incidentFace(h);
    Face f2 = m_cVoronoi.incidentFace(m_cVoronoi.twin(h));
    float iso1 = faceheights[f1.id];
    float iso2 = faceheights[f2.id];
    bool hasInside = iso1 < isolevel || iso2 < isolevel;
    bool hasOutside = iso1 >= isolevel || iso2 >= isolevel;

    return hasInside && hasOutside;
}

void MapGenerator::calculateErosionMap(NodeMap<float> &erosionMap) {
    fillDepressions();

    NodeMap<float> fluxMap(&m_vVertexMap, 0.0);
    calculateFluxMap(fluxMap);

    NodeMap<float> slopeMap(&m_vVertexMap, 0.0);
    calculateSlopeMap(slopeMap);

    for (unsigned int i = 0; i < erosionMap.size(); i++) {
        float flux = fluxMap(i);
        float slope = slopeMap(i);
        float river = m_fErosionRiverFactor * sqrt(flux) * slope;
        float creep = m_fErosionCreepFactor * slope * slope;
        float erosion = fmin(river + creep, m_fmaxErosionRate);
        erosionMap.set(i, erosion);
    }

    erosionMap.normalize();
}

void MapGenerator::fillDepressions() {
    NodeMap<float> finalHeightMap(&m_vVertexMap, m_vHeightMap.max());
    Vertex2D v;
    for (unsigned int i = 0; i < m_vVertexMap.edge.size(); i++) {
        v = m_vVertexMap.edge[i];
        finalHeightMap.set(v, m_vHeightMap(v));
    }

    float eps = 1e-5;
    std::vector<int> *neighbours;
    for (;;) {
        bool heightUpdated = false;
        for (unsigned int i = 0; i < m_vHeightMap.size(); i++) {
            if (m_vHeightMap(i) == finalHeightMap(i)) {
                continue;
            }

            neighbours = m_vNeighbourMap.getPointer(i);
            for (unsigned int nidx = 0; nidx < neighbours->size(); nidx++) {
                float nval = finalHeightMap(neighbours->at(nidx));
                if (m_vHeightMap(i) >= nval + eps) {
                    finalHeightMap.set(i, m_vHeightMap(i));
                    heightUpdated = true;
                    break;
                }

                float hval = nval + eps;
                if ((finalHeightMap(i) > hval) && (hval > m_vHeightMap(i))) {
                    finalHeightMap.set(i, hval);
                    heightUpdated = true;
                }
            }
        }

        if (!heightUpdated) {
            break;
        }
    }

    m_vHeightMap = finalHeightMap;
}
//calculate the lowest neighbour Vertex2D of each interior vertex, higher to lower
void MapGenerator::calculateFlowMap(NodeMap<int> &flowMap) {
    Vertex2D v, n;
    std::vector<int> *nbs;
    for (unsigned int i = 0; i < m_vVertexMap.interior.size(); i++) {
        v = m_vVertexMap.interior[i];

        nbs = m_vNeighbourMap.getPointer(v);
        Vertex2D minVertex;
        float minHeight = m_vHeightMap(v);
        for (unsigned int nidx = 0; nidx < nbs->size(); nidx++) {
            n = m_vVertexMap.vertices[nbs->at(nidx)];
            if (!m_vVertexMap.isVertex(n)) {
                continue;
            }

            if (m_vHeightMap(n) < minHeight) {
                minHeight = m_vHeightMap(n);
                minVertex = n;
            }
        }

        flowMap.set(v, flowMap.getNodeIndex(minVertex));
    }
}

void MapGenerator::calculateFluxMap(NodeMap<float> &fluxMap) {
    NodeMap<int> flowMap(&m_vVertexMap, -1);
    calculateFlowMap(flowMap);

    for (unsigned int i = 0; i < flowMap.size(); i++) {
        int next = i;
        while (next != -1) {
            fluxMap.set(next, fluxMap(next) + 1.0f);
            next = flowMap(next);
        }
    }

    float maxFlux = calculateFluxCap(fluxMap);
    for (unsigned int i = 0; i < fluxMap.size(); i++) {
        float f = fluxMap(i);
        f = fmin(maxFlux, f);
        f /= maxFlux;
        fluxMap.set(i, f);
    }

    m_vFluxMap = fluxMap;
    m_vFlowMap = flowMap;
}

float MapGenerator::calculateFluxCap(NodeMap<float> &fluxMap) {
    float max = fluxMap.max();

    int nbins = 1000;
    std::vector<int> bins(nbins, 0);

    float step = (float)max / (float)nbins;
    float invstep = 1.0f / step;
    for (unsigned int i = 0; i < fluxMap.size(); i++) {
        float f = fluxMap(i);
        int binidx = (int)floor(f * invstep);
        if (binidx >= (int)bins.size()) {
            binidx = (int)bins.size() - 1;
        }
        bins[binidx]++;
    }

    float acc = 0.0;
    float maxflux = 0.0;
    for (int i = 0; i < nbins; i++) {
        float pct = (float)bins[i] / (float)fluxMap.size();
        acc += pct;
        if (acc > m_fluxCapPercentile) {
            maxflux = (i + 1)*step;
            break;
        }
    }

    return maxflux;
}

void MapGenerator::calculateSlopeMap(NodeMap<float> &slopeMap) {
    for (unsigned int i = 0; i < slopeMap.size(); i++) {
        slopeMap.set(i, calculateSlope(i));
    }
}

float MapGenerator::calculateSlope(int i) {
    Vertex2D v = m_vVertexMap.vertices[i];
    if (!m_vVertexMap.isInterior(v)) {
        return 0.0;
    }

    Vector3 vNormal;
    calculateVertexNormal(i, vNormal);
    return sqrt(vNormal.x * vNormal.x + vNormal.y * vNormal.y);
}

void MapGenerator::getContourDrawData(std::vector<std::vector<Vector2> > &data) {
    std::vector<VertexList> paths;
    getContourPaths(paths);

    Vector2 inv = Vector2(1.0f / m_cBoundingBox.getDX(), 1.0f / m_cBoundingBox.getDY());
    for (unsigned int j = 0; j < paths.size(); j++) {
        std::vector<Vector2> drawPath;
        drawPath.reserve(paths[j].size());
        for (unsigned int i = 0; i < paths[j].size(); i++) {
            Vertex2D v = paths[j][i];
            Vector2 vn = (v.vPos - Vector2(m_cBoundingBox.vMin.x, m_cBoundingBox.vMin.z));
            vn.x *= inv.x;
            vn.y *= inv.y;

            drawPath.push_back(vn);
        }
        data.push_back(drawPath);
    }
}

void MapGenerator::getContourPaths(std::vector<VertexList> &paths) {
    std::vector<int> adjacentEdgeCounts(m_vVertexMap.vertices.size(), 0);
    std::vector<uint8_t> isEdgeVisited(m_cVoronoi.edges.size(), false);
    HalfEdge h;
    Vertex2D v1, v2;
    for (unsigned int i = 0; i < m_cVoronoi.edges.size(); i++) {
        h = m_cVoronoi.edges[i];
        if (!isEdgeInMap(h) || isEdgeVisited[h.id]) {
            continue;
        }

        if (!isContourEdge(h)) {
            continue;
        }

        v1 = m_cVoronoi.origin(h);
        v2 = m_cVoronoi.origin(m_cVoronoi.twin(h));
        int idx1 = m_vVertexMap.getVertexIndex(v1);
        int idx2 = m_vVertexMap.getVertexIndex(v2);
        adjacentEdgeCounts[idx1]++;
        adjacentEdgeCounts[idx2]++;

        isEdgeVisited[h.id] = true;
        isEdgeVisited[h.twin] = true;
    }

    std::vector<uint8_t> isEndVertex(m_vVertexMap.vertices.size(), false);
    std::vector<uint8_t> isContourVertex(m_vVertexMap.vertices.size(), false);
    for (unsigned int i = 0; i < adjacentEdgeCounts.size(); i++) {
        if (adjacentEdgeCounts[i] == 1) {
            isEndVertex[i] = true;
            isContourVertex[i] = true;
        } else if (adjacentEdgeCounts[i] == 2) {
            isContourVertex[i] = true;
        }
    }

    std::vector<uint8_t> isVertexInContour(m_vVertexMap.vertices.size(), false);
    for (unsigned int i = 0; i < isEndVertex.size(); i++) {
        if (!isEndVertex[i] || isVertexInContour[i]) {
            continue;
        }

        VertexList path;
        getContourPath(i, isContourVertex, isEndVertex, isVertexInContour, path);
        paths.push_back(path);
    }

    for (unsigned int i = 0; i < isContourVertex.size(); i++) {
        if (!isContourVertex[i] || isVertexInContour[i]) {
            continue;
        }

        VertexList path;
        getContourPath(i, isContourVertex, isEndVertex, isVertexInContour, path);
        paths.push_back(path);
    }
}

bool MapGenerator::isLandFace(int fidx) {
    if (!m_bLandFaceTableInitialized) {
        initializeLandFaceTable();
    }

    return m_vIsLandFaceTable[fidx];
}

void MapGenerator::initializeLandFaceTable() {
    getLandFaces(m_vIsLandFaceTable);
    m_bLandFaceTableInitialized = true;
}

void MapGenerator::getLandFaces(std::vector<uint8_t> &isLandFace) {
    std::vector<float> faceHeights = computeFaceValues(m_vHeightMap);

    isLandFace.clear();
    isLandFace.reserve(m_cVoronoi.faces.size());
    for (unsigned int i = 0; i < faceHeights.size(); i++) {
        isLandFace.push_back(isLand(faceHeights[i]));
    }

    cleanupLandFaces(isLandFace);
}

bool MapGenerator::isLand(float seaLevel) {
    return seaLevel >= m_seaLevel;
}

void MapGenerator::cleanupLandFaces(std::vector<uint8_t> &isLandFace) {
    std::vector<std::vector<int> > islands;
    std::vector<uint8_t> isFaceProcessed(m_cVoronoi.faces.size(), false);
    std::vector<int> connectedFaces;
    for (unsigned int i = 0; i < isLandFace.size(); i++) {
        if (isFaceProcessed[i]) {
            continue;
        }

        connectedFaces.clear();
        getConnectedFaces(i, isLandFace, isFaceProcessed, connectedFaces);
        islands.push_back(connectedFaces);
    }

    for (unsigned int j = 0; j < islands.size(); j++) {
        if (islands[j].size() >= m_fminIslandFaceThreshold) {
            continue;
        }

        for (unsigned int i = 0; i < islands[j].size(); i++) {
            int fidx = islands[j][i];
            isLandFace[fidx] = !isLandFace[fidx];
        }
    }
}

void MapGenerator::getConnectedFaces(int seed,
                                           std::vector<uint8_t> &isLandFace,
                                           std::vector<uint8_t> &isFaceProcessed,
                                           std::vector<int> &faces) {
    std::vector<int> queue;
    queue.push_back(seed);
    isFaceProcessed[seed] = true;

    while (!queue.empty()) {
        int fidx = queue.back();
        queue.pop_back();

        uint8_t faceType = isLandFace[fidx];
        for (unsigned int nidx = 0; nidx < m_vFaceNeighbours[fidx].size(); nidx++) {
            int nfidx = m_vFaceNeighbours[fidx][nidx];
            if ((isLandFace[nfidx] == faceType) && !isFaceProcessed[nfidx]) {
                queue.push_back(nfidx);
                isFaceProcessed[nfidx] = true;
            }
        }

        faces.push_back(fidx);
    }
}


bool MapGenerator::isContourEdge(HalfEdge &h) {
    Face f1 = m_cVoronoi.incidentFace(h);
    Face f2 = m_cVoronoi.incidentFace(m_cVoronoi.twin(h));
    return (isLandFace(f1.id) && !isLandFace(f2.id)) ||
           (isLandFace(f2.id) && !isLandFace(f1.id));
}

bool MapGenerator::isContourEdge(Vertex2D &v1, Vertex2D &v2) {
    std::vector<HalfEdge> edges;
    edges.reserve(3);
    m_cVoronoi.getIncidentEdges(v1, edges);

    HalfEdge h;
    Vertex2D v;
    for (unsigned int i = 0; i < edges.size(); i++) {
        h = edges[i];
        v = m_cVoronoi.origin(m_cVoronoi.twin(h));
        if (v.id == v2.id) {
            return isContourEdge(h);
        }
    }

    return false;
}

void MapGenerator::getContourPath(int seed, std::vector<uint8_t> &isContourVertex,
                                                  std::vector<uint8_t> &isEndVertex,
                                                  std::vector<uint8_t> &isVertexInContour,
                                                  VertexList &path) {
    Vertex2D v = m_vVertexMap.vertices[seed];
    Vertex2D lastVertex = v;

    std::vector<int> *nbs;
    for (;;) {
        path.push_back(v);
        isVertexInContour[m_vVertexMap.getVertexIndex(v)] = true;

        nbs = m_vNeighbourMap.getPointer(v);
        bool isFound = false;
        for (unsigned int i = 0; i < nbs->size(); i++) {
            Vertex2D n = m_vVertexMap.vertices[nbs->at(i)];
            int nidx = m_vVertexMap.getVertexIndex(n);
            if (n.id != lastVertex.id && isContourVertex[nidx] &&
                    isContourEdge(v, n)) {
                lastVertex = v;
                v = n;
                isFound = true;
                break;
            }
        }

        if (!isFound) {
            break;
        }

        int vidx = m_vVertexMap.getVertexIndex(v);
        if (isEndVertex[vidx] || isVertexInContour[vidx]) {
            path.push_back(v);
            isVertexInContour[vidx] = true;
            break;
        }
    }
}

void MapGenerator::getRiverDrawData(std::vector<std::vector<Vector2>> &data) {
    if (!m_bHeightMapEroded) {
        erode();
        if (!m_bHeightMapEroded) return;
    }

    std::vector<VertexList> riverPaths;
    getRiverPaths(riverPaths);

    for (unsigned int i = 0; i < riverPaths.size(); i++) {
        riverPaths[i] = smoothPath(riverPaths[i], m_fRiverSmoothingFactor);
    }

    float invwidth = 1.0f / m_cBoundingBox.getDX();
    float invheight = 1.0f / m_cBoundingBox.getDY();
    for (unsigned int j = 0; j < riverPaths.size(); j++) {
        std::vector<Vector2> drawPath;
        drawPath.reserve(riverPaths[j].size());
        for (unsigned int i = 0; i < riverPaths[j].size(); i++) {
            Vertex2D v = riverPaths[j][i];
            float nx = (v.vPos.x - m_cBoundingBox.vMin.x) * invwidth;
            float ny = (v.vPos.y - m_cBoundingBox.vMin.y) * invheight;

            drawPath.push_back(Vector2(nx * m_vSize.x, ny * m_vSize.y));
        }
        data.push_back(drawPath);
    }
}

void MapGenerator::getRiverPaths(std::vector<VertexList> &riverPaths) {
    VertexList riverVertices;
    getRiverVertices(riverVertices);

    std::vector<uint8_t> isVertexInRiver(m_vVertexMap.vertices.size(), false);
    for (unsigned int i = 0; i < riverVertices.size(); i++) {
        int idx = m_vVertexMap.getVertexIndex(riverVertices[i]);
        isVertexInRiver[idx] = true;
    }

    VertexList fixedVertices;
    getFixedRiverVertices(riverVertices, fixedVertices);
    std::vector<uint8_t> isFixedVertex(m_vVertexMap.vertices.size(), false);
    for (unsigned int i = 0; i < fixedVertices.size(); i++) {
        int vidx = m_vVertexMap.getVertexIndex(fixedVertices[i]);
        isFixedVertex[vidx] = true;
    }

    VertexList path;
    for (unsigned int i = 0; i < isFixedVertex.size(); i++) {
        if (!isFixedVertex[i]) {
            continue;
        }

        path.clear();
        int next = i;
        while (isVertexInRiver[next]) {
            path.push_back(m_vVertexMap.vertices[next]);
            next = m_vFlowMap(next);

            if (next == -1) { break; }
            if (isFixedVertex[next]) {
                path.push_back(m_vVertexMap.vertices[next]);
                break;
            }
        }

        if (path.size() >= 2) {
            riverPaths.push_back(path);
        }
    }
}

void MapGenerator::getRiverVertices(VertexList &vertices) {
    std::vector<uint8_t> isVertexAdded(m_vVertexMap.vertices.size(), false);

    Vertex2D v;
    VertexList pathVertices;
    for (unsigned int i = 0; i < m_vVertexMap.vertices.size(); i++) {
        v = m_vVertexMap.vertices[i];
        if (m_vFluxMap(v) < m_fRiverFluxThreshold || isCoastVertex(i)) {
            continue;
        }

        int next = m_vFlowMap.getNodeIndex(v);
        pathVertices.clear();
        while (next != -1) {
            pathVertices.push_back(m_vVertexMap.vertices[next]);
            if (isCoastVertex(next)) {
                break;
            }
            if (!isLandVertex(next)) {
                pathVertices.clear();
                break;
            }
            next = m_vFlowMap(next);
        }

        if (pathVertices.empty()) { continue; }

        for (unsigned int i = 0; i < pathVertices.size(); i++) {
            int idx = m_vVertexMap.getVertexIndex(pathVertices[i]);
            if (!isVertexAdded[idx]) {
                vertices.push_back(pathVertices[i]);
                isVertexAdded[idx] = true;
            }
        }
    }
}

bool MapGenerator::isLandVertex(int vidx) {
    std::vector<Face> faces;
    faces.reserve(6);
    m_cVoronoi.getIncidentFaces(m_vVertexMap.vertices[vidx], faces);

    for (unsigned int i = 0; i < faces.size(); i++) {
        if (isLandFace(faces[i].id)) {
            return true;
        }
    }

    return false;
}

bool MapGenerator::isCoastVertex(int vidx) {
    std::vector<Face> faces;
    faces.reserve(6);
    m_cVoronoi.getIncidentFaces(m_vVertexMap.vertices[vidx], faces);

    bool hasLand = false;
    bool hasSea = false;
    for (unsigned int i = 0; i < faces.size(); i++) {
        if (isLandFace(faces[i].id)) {
            hasLand = true;
        } else {
            hasSea = true;
        }
    }

    return hasLand && hasSea;
}

void MapGenerator::getFixedRiverVertices(VertexList &riverVertices,
                                               VertexList &fixedVertices) {
    std::vector<uint8_t> isVertexInRiver(m_vVertexMap.vertices.size(), false);
    for (unsigned int i = 0; i < riverVertices.size(); i++) {
        int idx = m_vVertexMap.getVertexIndex(riverVertices[i]);
        isVertexInRiver[idx] = true;
    }

    std::vector<uint8_t> isEdgeProcessed(m_cVoronoi.edges.size(), false);
    std::vector<int> adjacentEdgeCounts(m_vVertexMap.vertices.size(), 0);

    HalfEdge h;
    Vertex2D p1, p2;
    for (unsigned int i = 0; i < m_cVoronoi.edges.size(); i++) {
        h = m_cVoronoi.edges[i];
        if (!isEdgeInMap(h) || isEdgeProcessed[h.id]) {
            continue;
        }

        p1 = m_cVoronoi.origin(h);
        p2 = m_cVoronoi.origin(m_cVoronoi.twin(h));
        int idx1 = m_vVertexMap.getVertexIndex(p1);
        int idx2 = m_vVertexMap.getVertexIndex(p2);

        if (!isVertexInRiver[idx1] || !isVertexInRiver[idx2]) {
            continue;
        }

        adjacentEdgeCounts[idx1]++;
        adjacentEdgeCounts[idx2]++;

        isEdgeProcessed[h.id] = true;
        isEdgeProcessed[h.twin] = true;
    }

    for (unsigned int i = 0; i < adjacentEdgeCounts.size(); i++) {
        if (adjacentEdgeCounts[i] == 1 || adjacentEdgeCounts[i] == 3) {
            fixedVertices.push_back(m_vVertexMap.vertices[i]);
        }
    }
}

MapGenerator::VertexList MapGenerator::smoothPath(VertexList &path, float factor) {
    if (path.size() < 2) {
        return path;
    }

    VertexList smoothedPath = path;
    for (unsigned int i = 1; i < path.size() - 1; i++) {
        Vector2 v0 = path[i-1].vPos;
        Vector2 v1 = path[i].vPos;
        Vector2 v2 = path[i+1].vPos;

        v1.x = (1 - factor)*v1.x + factor * 0.5f *(v0.x + v2.x);
        v1.y = (1-factor)*v1.y + factor * 0.5f *(v0.y + v2.y);

        smoothedPath[i].vPos = v1;
    }

    return smoothedPath;
}

void MapGenerator::getSlopeDrawData(std::vector<float> &data) {
    std::vector<Segment> slopeSegments;
    getSlopeSegments(slopeSegments);

    float invwidth = 1.0f / (m_cBoundingBox.vMax.x - m_cBoundingBox.vMin.x);
    float invheight = 1.0f / (m_cBoundingBox.vMax.y - m_cBoundingBox.vMin.y);
    for (unsigned int i = 0; i < slopeSegments.size(); i++) {
        Segment s = slopeSegments[i];

        float nx1 = (s.p1.x - m_cBoundingBox.vMin.x) * invwidth;
        float ny1 = (s.p1.y - m_cBoundingBox.vMin.y) * invheight;
        float nx2 = (s.p2.x - m_cBoundingBox.vMin.x) * invwidth;
        float ny2 = (s.p2.y - m_cBoundingBox.vMin.y) * invheight;

        data.push_back(nx1);
        data.push_back(ny1);
        data.push_back(nx2);
        data.push_back(ny2);
    }
}

void MapGenerator::getSlopeSegments(std::vector<Segment> &segments) {
    NodeMap<float> slopeMap(&m_vVertexMap, 0.0);
    calculateSlopeMap(slopeMap, true);

    NodeMap<float> nearSlopeMap(&m_vVertexMap, 0.0);
    calculateSlopeMap(nearSlopeMap, false);

    std::vector<float> faceSlopes = computeFaceValues(slopeMap);
    std::vector<float> nearSlopes = computeFaceValues(nearSlopeMap);
    std::vector<Vector2> facePositions = computeFacePositions();

    for (unsigned int i = 0; i < faceSlopes.size(); i++) {
        float slope = faceSlopes[i];
        if (!isLandFace(i) || fabs(slope) < m_fminSlopeThreshold) {
            continue;
        }

        float factor = (fabs(slope) - m_vSlopeRange.x) / (m_vSlopeRange.y - m_vSlopeRange.x);
        factor = MathFuncs::Clamp(factor, 0.0f, 1.0f);

        float angle = m_vSlopeAngleRange.x + factor * (m_vSlopeAngleRange.y - m_vSlopeAngleRange.x);
        angle = slope < 0 ? angle : -angle;

        Vector2 vDir = Vector2(cos(angle), sin(angle));
        Vector2 vLengthRange = m_vSlopeLengthRange * m_fResolution;

        float vslope = nearSlopes[i];
        float nf = (vslope - m_vVerticalSlopeRange.x) / (m_vVerticalSlopeRange.y - m_vVerticalSlopeRange.x);
        nf = MathFuncs::Clamp(nf, 0.0f, 1.0f);

        float length = vLengthRange.x + nf * (vLengthRange.y - vLengthRange.x);
        Segment s;
        s.p1 = facePositions[i];
        s.p2 = s.p1 + vDir * length;
        segments.push_back(s);
    }
}

void MapGenerator::calculateSlopeMap(NodeMap<float> &slopeMap, bool bHorizontal) {
    for (unsigned int i = 0; i < slopeMap.size(); i++) {
        slopeMap.set(i, calculateSlope(i, bHorizontal));
    }
}

float MapGenerator::calculateSlope(int idx, bool bHorizontal) {
    Vertex2D v = m_vVertexMap.vertices[idx];
    if (!m_vVertexMap.isInterior(v)) {
        return 0;
    }
    Vector3 vNormal;
    calculateVertexNormal(idx, vNormal);
    return (bHorizontal ? vNormal.x : vNormal.y);
}

void MapGenerator::calculateVertexNormal(int vidx, Vector3 &vNormal)
{
    std::vector<int> *nbs = m_vNeighbourMap.getPointer(vidx);
    if (nbs->size() != 3) {
        return;
    }

    Vector2 p0 = m_vVertexMap.vertices[nbs->at(0)].vPos;
    Vector2 p1 = m_vVertexMap.vertices[nbs->at(1)].vPos;
    Vector2 p2 = m_vVertexMap.vertices[nbs->at(2)].vPos;

    Vector3 v0 = Vector3(p1.x - p0.x, p1.y - p0.y, m_vHeightMap(nbs->at(1)) - m_vHeightMap(nbs->at(0)));
    Vector3 v1 = Vector3(p2.x - p0.x, p2.y - p0.y, m_vHeightMap(nbs->at(2)) - m_vHeightMap(nbs->at(0)));
    Vector3 vn = v0.cross(v1);
    float invlen = 1.0f / vn.len();
    vNormal = vn * invlen;
}

void MapGenerator::getCityDrawData(std::vector<Vector2> &data) {
    float invwidth = 1.0f / m_cBoundingBox.getDX();
    float invheight = 1.0f / m_cBoundingBox.getDY();
    for (unsigned int i = 0; i < m_vCities.size(); i++) {
        Vector2 p = m_vCities[i].position;
        float nx = (p.x - m_cBoundingBox.vMin.x) * invwidth;
        float ny = (p.y - m_cBoundingBox.vMin.y) * invheight;
        data.push_back(Vector2(nx, ny));
    }
}

void MapGenerator::getCityLocation(City &cCity) {
    NodeMap<float> cityScores(&m_vVertexMap, 0.0);
    getCityScores(cityScores);

    std::vector<float> faceScores = computeFaceValues(cityScores);
    std::vector<Vector2> facePositions = computeFacePositions();
    float maxScore = -std::numeric_limits<float>::infinity();
    int cityfidx = -1;
    for (unsigned int i = 0; i < faceScores.size(); i++) {
        Vector2 fp = facePositions[i];
        if (m_cBoundingBox.isPointInside(Vector3(fp.x, fp.y, 0)) && faceScores[i] > maxScore) {
            maxScore = faceScores[i];
            cityfidx = i;
        }
    }

    cCity.position = computeFacePosition(cityfidx);
    cCity.faceid = cityfidx;
}

void MapGenerator::getCityScores(NodeMap<float> &cityScores) {
    NodeMap<float> fluxMap = m_vFluxMap;
    fluxMap.relax();

    float neginf = -1e2;
    float eps = 1e-6;
    Vector2 p;
    for (unsigned int i = 0; i < cityScores.size(); i++) {
        float score = 0.0f;
        if (!isLandVertex(i) || isCoastVertex(i)) {
            score += neginf;
        }

        score += m_fluxScoreBonus * sqrt(fluxMap(i));

        p = m_vVertexMap.vertices[i].vPos;
        float extentsDist = fmax(0.0f, pointToEdgeDistance(p));
        score -= m_fNearEdgeScorePenalty * (1.0f / (extentsDist + eps));

        for (unsigned int cidx = 0; cidx < m_vCities.size(); cidx++) {
            Vector2 cp = m_vCities[cidx].position;
            float dist = fmin(getPointDistance(p, cp), m_fmaxPenaltyDistance);
            float distfactor = 1 - dist / m_fmaxPenaltyDistance;
            score -= m_fNearCityScorePenalty * distfactor;
        }

        score = fmax(neginf, score);
        cityScores.set(i, score);
    }
}

float MapGenerator::getPointDistance(Vector2 &p1, Vector2 &p2) {
    return (p1 - p2).len();
}

float MapGenerator::pointToEdgeDistance(Vector2 p) {
    float mindist = std::numeric_limits<float>::infinity();
    mindist = fmin(mindist, p.x - m_cBoundingBox.vMin.x);
    mindist = fmin(mindist, m_cBoundingBox.vMax.x - p.x);
    mindist = fmin(mindist, p.y - m_cBoundingBox.vMin.y);
    mindist = fmin(mindist, m_cBoundingBox.vMax.y - p.y);

    return mindist;
}

void MapGenerator::updateCityMovementCost(City &city) {
    std::vector<float> faceHeights = computeFaceValues(m_vHeightMap);
    std::vector<float> faceFlux = computeFaceValues(m_vFluxMap);
    std::vector<Vector2> facePositions = computeFacePositions();
    std::vector<uint8_t> isFaceInMap(m_cVoronoi.faces.size(), false);
    for (unsigned int i = 0; i < m_cVoronoi.faces.size(); i++) {
        isFaceInMap[i] = m_cBoundingBox.isPointInside(Vector3(facePositions[i].x, facePositions[i].y, 0));
    }

    float inf = std::numeric_limits<float>::infinity();
    std::vector<float> movementCosts(m_cVoronoi.faces.size(), inf);
    std::vector<int> parents(m_cVoronoi.faces.size(), -1);
    int rootid = city.faceid;
    movementCosts[rootid] = 0;
    std::queue<int> queue;
    queue.push(rootid);

    while (!queue.empty()) {
        int fidx = queue.front();
        queue.pop();

        for (unsigned int idx = 0; idx < m_vFaceNeighbours[fidx].size(); idx++) {
            int nidx = m_vFaceNeighbours[fidx][idx];
            if (!isFaceInMap[nidx] || movementCosts[nidx] != inf) {
                continue;
            }

            float cost = 0.0;
            float hdist = getPointDistance(facePositions[nidx], facePositions[fidx]);
            float hcost = isLandFace(nidx) ? m_fLandDistanceCost : m_fSeaDistanceCost;
            cost += hcost * hdist;

            if (isLandFace(nidx)) {
                float udist = faceHeights[nidx] - faceHeights[fidx];
                float ucost = udist > 0.0 ? m_fUphillCost : m_fDownhillCost;
                cost += (udist / hdist) * (udist / hdist) * ucost;
                cost += sqrt(faceFlux[nidx]) * m_fluxCost;
            }

            if ((isLandFace(fidx) && !isLandFace(nidx)) ||
                (isLandFace(nidx) && !isLandFace(fidx))) {
                cost += m_fLandTransitionCost;
            }

            movementCosts[nidx] = movementCosts[fidx] + cost;
            parents[nidx] = fidx;
            queue.push(nidx);
        }
    }

    city.movementCosts = movementCosts;
}

void MapGenerator::genRiverHeightMap(std::vector<float> &vHeights, float fMaxHeight, bool bHasRiver) {
    //height color
    std::vector<float> facecolors;
    getNormalizedHeightMap(facecolors);

    int nMesh = m_vSize.x * m_vSize.y;
    vHeights.reserve(nMesh);
#pragma omp parallel for
    for (int i = 0; i < nMesh; i++) {
        int x = i % m_vSize.x;
        int z = i / m_vSize.x;

        unsigned int j = 0;
        for (; j < m_cVoronoi.faces.size(); j++) {
            if (GeometryMath::PointInConvexPolygon(Vector2((float)x, (float)z), (int)faceVertices[j].size(), faceVertices[j]) > 0) break;
        }

        float height = (j < m_cVoronoi.faces.size() ? facecolors[j] * fMaxHeight : 0);

        if (bHasRiver) {
            bool bRiver = false;
            for (unsigned int i = 0; i < m_vRiverData.size(); i++) {
                for (unsigned int j = 0; j < m_vRiverData[i].size(); j++) {
                    if (fabs(m_vRiverData[i][j].x - x) < 1 && fabs(m_vRiverData[i][j].y - z) < 1) {
                        bRiver = true;
                        break;
                    }
                }
            }
            if (bRiver) {
                height -= 3;
            }
        }

        vHeights.push_back(height);
    }
}
