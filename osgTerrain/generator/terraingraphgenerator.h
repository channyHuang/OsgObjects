#ifndef TERRAINGRAPHGENERATOR_H
#define TERRAINGRAPHGENERATOR_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <string>
#include <chrono>

#include "commonMath/vector2.h"
#include "commonMath/vector2i.h"
#include "commonMath/vector3i.h"
#include "commonMath/funcs.h"

#include "commonGeometry/voronoi.h"
#include "commonGeometry/nodemap.h"
#include "commonGeometry/poissonsampler.h"

using namespace Graph_Geometry;

class MapGenerator {
public:
    enum DisableMapType {
        Slope = 0,
        River,
        Contour,
        Cities,
        MaxMapType
    };

    enum SmoothType {
        Normalize = 0,
        Round,
        Relax
    };

    enum CrudeType {
        Rough_Polygon_Up,
        Rough_Polygon_Down,
        Rough_Round,
        Rough_ForMesh
    };

    enum DirectionType {
        Direction_X,
        Direction_Y,
        Direction_Z
    };

    MapGenerator(int nSizex = 10, int nSizey = 10, float fResolution = 3.0f);
    //calculate face and vertices from TerrainGeometry
    void genVoronoiFaceVertices();
    void genVoronoiFaceVertices(GraphGeometry &cVoronoi, int &nFaceSize, int &nVerticesSize, std::vector<std::vector<Vector2> > &vFaceVertices);
    void getCrudeHeight(const Box &cBox, std::vector<std::vector<float> > &vHeights,
        float fMaxHeight, float fStrength, DirectionType eNormalDirectionType = MapGenerator::Direction_Y);

    void initialize();
    void smoothHeightMap(SmoothType eSmoothType);
    void addHill(float px, float py, float radius, float height);
    void addCone(float px, float py, float radius, float height);
    void addSlope(float px, float py, float dirx, float diry,
                  float radius, float height);
    bool erode(float amount = 0.1, int iterations = 3);
    //to do, add city details
    void addCity(std::string cityName, std::string territoryName);
    //randomly add hills in map, nNumOfHills is recoment to be size.x * size.y / 5 ?
    void initializeHeightMap(int nNumOfHills = 2000);
    //output
    void outputVoronoiDiagram(int nFaceSize, int nVerticesSize,
        std::vector<std::vector<Vector2>> &faceVertices, std::vector<float> &facecolors,
        std::string filename = "voronoi.ply");
    bool getNormalizedHeightMap(std::vector<float> &facecolors);



    void setMapType(DisableMapType eType, bool bDisable = true);
    //generate different kinds of rough terrain
    void genRiverHeightMap(std::vector<float> &vHeights, float fMaxHeight = 100.0f, bool bHasRiver = false);

    Vector2i getSize() { return m_vSize; }
    void setVoronoi(GraphGeometry voronoi) { m_cVoronoi = voronoi; }
    void setCrudeType(CrudeType eCrudeType) { m_eCrudeType = eCrudeType; }

    typedef std::vector<Vertex2D> VertexList;

    struct Segment {
        Vector2 p1;
        Vector2 p2;
    };

    struct City {
        std::string cityName;
        std::string territoryName;
        Vector2 position;
        int faceid;
        std::vector<float> movementCosts;
    };

    //init functions
    void initializeVoronoiData();
    void initializeMapData();
    void initializeNeighbourMap();
    void initializeFaceNeighbours();
    void initializeFaceVertices();
    void initializeFaceEdges();

    void initializeLandFaceTable();
    //erosion functions
    void calculateErosionMap(NodeMap<float> &erosionMap);
    void fillDepressions();
    void calculateFlowMap(NodeMap<int> &flowMap);
    void calculateFluxMap(NodeMap<float> &fluxMap);
    float calculateFluxCap(NodeMap<float> &fluxMap);
    void calculateSlopeMap(NodeMap<float> &slopeMap);
    float calculateSlope(int i);
    //river function
    void getRiverDrawData(std::vector<std::vector<Vector2>> &data);
    void getRiverPaths(std::vector<VertexList> &paths);
    void getRiverVertices(VertexList &vertices);
    void getFixedRiverVertices(VertexList &riverVertices, VertexList &fixedVertices);
    //slope function
    void getSlopeDrawData(std::vector<float> &data);
    void getSlopeSegments(std::vector<Segment> &segments);
    void calculateSlopeMap(NodeMap<float> &slopeMap, bool bHorizontal);
    float calculateSlope(int idx, bool bHorizontal);
    //city function
    void getCityLocation(City &cCity);
    void getCityScores(NodeMap<float> &cityScores);
    void updateCityMovementCost(City &city);
    void getCityDrawData(std::vector<Vector2> &data);
    //statistic data
    std::vector<float> computeFaceValues(NodeMap<float> &heightMap);
    std::vector<Vector2> computeFacePositions();
    Vector2 computeFacePosition(int fidx);
    //common function
    void calculateVertexNormal(int vidx, Vector3 &vNormal);
    float getPointDistance(Vector2 &p1, Vector2 &p2);
    float pointToEdgeDistance(Vector2 p);
    bool isEdgeInMap(HalfEdge &h);
    VertexList smoothPath(VertexList &path, float factor);
    //contour function
    void getConnectedFaces(int seed, std::vector<uint8_t> &isLandFace,
        std::vector<uint8_t> &isFaceProcessed,
        std::vector<int> &faces);
    void getContourDrawData(std::vector<std::vector<Vector2> > &data);
    void getContourPaths(std::vector<VertexList> &paths);
    void getContourPath(int seed, std::vector<uint8_t> &isContourVertex,
        std::vector<uint8_t> &isEndVertex,
        std::vector<uint8_t> &isVertexInContour,
        VertexList &path);

    bool isContourEdge(HalfEdge &h,
        std::vector<float> &faceheights,
        float isolevel);
    bool isContourEdge(HalfEdge &h);
    bool isContourEdge(Vertex2D &v1, Vertex2D &v2);
    bool isLandFace(int fidx);
    void getLandFaces(std::vector<uint8_t> &isLandFace);
    bool isLand(float seaLevel);
    void cleanupLandFaces(std::vector<uint8_t> &isLandFace);

    bool isLandVertex(int vidx);
    bool isCoastVertex(int vidx);

private:
    CrudeType m_eCrudeType = Rough_Polygon_Down;
    //status
    bool m_bInitialized = false;
    int m_nDisabledMapType = 0;
    bool m_bHeightMapEroded = false;
    bool m_bLandFaceTableInitialized = false;

    //size
    Vector2i m_vSize = Vector2i(1920, 1080);
    Box m_cBoundingBox = Box(0.0f, 0.0f, -1.0f, (float)m_vSize.x, (float)m_vSize.y, 1.0f);

    //structures and datas
    GraphGeometry m_cVoronoi;
    VertexMap m_vVertexMap;
    NodeMap<std::vector<int> > m_vNeighbourMap;
    std::vector<std::vector<int> > m_vFaceNeighbours;
    std::vector<std::vector<int> > m_vFaceVertices;
    std::vector<std::vector<int> > m_vFaceEdges;
    NodeMap<float> m_vHeightMap;
    NodeMap<float> m_vFluxMap;
    NodeMap<int> m_vFlowMap;
    std::vector<uint8_t> m_vIsLandFaceTable;
    std::vector<City> m_vCities;
    std::vector<std::vector<Vector2> > faceVertices;
    int m_nVerticesSize = 0, m_nFaceSize = 0;
    //mid data
    std::vector<std::vector<Vector2>> m_vRiverData;

    //params
    //sample params
    //float m_fResolution = 0.1; //size of polygon in voronoi
    float m_fResolution = 5.0f;
    int m_nPoissonSamplerKValue = 25;
    float m_fSamplePadFactor = 3.5;
    //erosion params
    float m_fluxCapPercentile = 0.995;
    float m_fErosionRiverFactor = 500.0;
    float m_fErosionCreepFactor = 500.0;
    float m_fRiverFluxThreshold = 0.06;
    float m_fRiverSmoothingFactor = 0.5;
    float m_fmaxErosionRate = 50.0;
    //slope params
    float m_fminSlopeThreshold = 0.07;
    Vector2 m_vSlopeRange = Vector2(0.0, 0.7);
    Vector2 m_vSlopeAngleRange = Vector2(0.2, 1.5);
    Vector2 m_vSlopeLengthRange = Vector2(0.75, 1.3);
    Vector2 m_vVerticalSlopeRange = Vector2(-0.25, 0.25);
    //city params
    float m_fluxScoreBonus = 2.0;
    float m_fNearEdgeScorePenalty = 0.5;
    float m_fNearCityScorePenalty = 2.0;
    float m_fmaxPenaltyDistance = 4.0;
    float m_fLandDistanceCost = 0.2;
    float m_fSeaDistanceCost = 0.4;
    float m_fUphillCost = 0.1;
    float m_fDownhillCost = 1.0;
    float m_fluxCost = 0.8;
    float m_fLandTransitionCost = 0.0;
    //contour params
    float m_fminIslandFaceThreshold = 35;
    float m_seaLevel = 0.0;
};

#endif
