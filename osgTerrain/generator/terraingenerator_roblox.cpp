#include "terraingenerator_roblox.h"

std::shared_ptr<TerrainGenerator_Roblox> TerrainGenerator_Roblox::pInstance_ = nullptr;

std::shared_ptr<TerrainGenerator_Roblox> TerrainGenerator_Roblox::getInstance() {
   if (pInstance_ == nullptr) {
       pInstance_ = std::make_shared<TerrainGenerator_Roblox>();
   }
   return pInstance_;
}

TerrainGenerator_Roblox::TerrainGenerator_Roblox() {
   initData();
}

TerrainGenerator_Roblox::~TerrainGenerator_Roblox() {}

void TerrainGenerator_Roblox::initData() {
   //unsigned seeds = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
   srand(6180339);
   for (unsigned int i = 0; i < 1000; i++) {
       vtheseed_.push_back(std::rand() * 1.f / RAND_MAX);
   }
   ntheseed_size_ = vtheseed_.size();
   cperlin_.load();
}

void TerrainGenerator_Roblox::genTotallyFlat(VoxelBrush* pVoxelTool, int max_lod) {
   float h = std::max(1.0f, _range.vSize.y * 0.5f);
   Box cBox(_range.vStart, _range.vStart + Vector3(_range.vSize.x, h, _range.vSize.z));

   Vector3i &&vmin_pos = MathFuncs::vector3FloorOrCeil(_range.vBox.vMin, true);
   Vector3i &&vmax_pos = MathFuncs::vector3FloorOrCeil(_range.vBox.vMax, false);

   float fnew_sdf = 0.f, brushOccupancy = 1.f;
   MaterialType ematerial_origin = MaterialType::AIR;
   Vector3 pos = Vector3(0), selectionSize = cBox.vMax - cBox.vMin, vOutSize, cellVector;
   Vector3 vhalf_selection_size = selectionSize * .5f;

   for (pos.x = vmin_pos.x; pos.x <= vmax_pos.x; ++pos.x) {
       for (pos.z = vmin_pos.z; pos.z <= vmax_pos.z; ++pos.z) {
           for (pos.y = vmin_pos.y; pos.y <= vmax_pos.y; ++pos.y) {
               cellVector = pos * fresolution_ - cBox.getCenter();
               vOutSize.x = 1.f - std::max(0.f, std::abs(cellVector.x) - vhalf_selection_size.x);
               vOutSize.y = 1.f - std::max(0.f, std::abs(cellVector.y) - vhalf_selection_size.y);
               vOutSize.z = 1.f - std::max(0.f, std::abs(cellVector.z) - vhalf_selection_size.z);
               brushOccupancy = vOutSize.x * vOutSize.y * vOutSize.z;
               fnew_sdf = translateSdfAndOccupancy(brushOccupancy, false);
               if (fnew_sdf < 2.f) {
                   pVoxelTool->set_voxel_f(pos, fnew_sdf, VoxelBuffer::CHANNEL_SDF);
                   if (fnew_sdf <= 0) pVoxelTool->set_voxel(pos, 1, VoxelBuffer::CHANNEL_TYPE);
               }

               //pVoxelTool->set_voxel_f(pos, -1.f, VoxelBuffer::CHANNEL_SDF);
               //pVoxelTool->set_voxel(pos, GRASSLAND, VoxelBuffer::CHANNEL_TYPE);
           }
       }
   }
}

//void TerrainGenerator_Roblox::genTotallyFlat(VoxelToolTerrain* pVoxelTool, int max_lod) {
//   float h = std::max(1.0f, _range.vSize.y * 0.5f);
//   Box cBox(_range.vStart, _range.vStart + Vector3(_range.vSize.x, h, _range.vSize.z));
//
//   Vector3i &&vmin_pos = TerrainMath::vector3FloorOrCeil(_range.vBox.vMin, true);
//   Vector3i &&vmax_pos = TerrainMath::vector3FloorOrCeil(_range.vBox.vMax, false);
//
//   float fnew_sdf = 0.f, brushOccupancy = 1.f;
//   MaterialType ematerial_origin = MaterialType::AIR;
//   Vector3 pos = Vector3(0), selectionSize = cBox.vMax - cBox.vMin, vOutSize, cellVector;
//   Vector3 vhalf_selection_size = selectionSize * .5f;
//
//   for (pos.x = vmin_pos.x; pos.x <= vmax_pos.x; ++pos.x) {
//       for (pos.z = vmin_pos.z; pos.z <= vmax_pos.z; ++pos.z) {
//           for (pos.y = vmin_pos.y; pos.y <= vmax_pos.y; ++pos.y) {
//               cellVector = pos * fresolution_ - cBox.getCenter();
//
//               vOutSize.x = 1.f - std::max(0.f, std::abs(cellVector.x) - vhalf_selection_size.x);
//               vOutSize.y = 1.f - std::max(0.f, std::abs(cellVector.y) - vhalf_selection_size.y);
//               vOutSize.z = 1.f - std::max(0.f, std::abs(cellVector.z) - vhalf_selection_size.z);
//
//               brushOccupancy = vOutSize.x * vOutSize.y * vOutSize.z;
//
//               fnew_sdf = translateSdfAndOccupancy(brushOccupancy, false);
//               //pVoxelTool->set_voxel_f(pos, fnew_sdf, VoxelBuffer::CHANNEL_SDF);
//               //if (fnew_sdf <= 0) pVoxelTool->set_voxel(pos, 1, VoxelBuffer::CHANNEL_TYPE);
//           }
//       }
//   }
//}

void TerrainGenerator_Roblox::setTerrainGeneratorSeed(std::string seedNumberStr) {
   long long compositeNumber = 0, number;
   for (size_t i = 0; i < seedNumberStr.length(); ++i) {
       number = seedNumberStr[i] - '0';
       if (number >= 0 && number <= 9) {
           compositeNumber = (compositeNumber + 6) * (number + 5);
       }
       else {
           compositeNumber = (compositeNumber + 7) * (seedNumberStr[i] + 3);
       }
       compositeNumber %= 61803;
   }
   nmaster_seed_ = compositeNumber;
}

float TerrainGenerator_Roblox::getNoise(int x, int y, int z, int seed1) {
   long long wtf = x + y + z + seed1 + nmaster_seed_ + (nmaster_seed_ - x) * (seed1 + z) + (seed1 - y) * (nmaster_seed_ + z);
   int index = std::floor(wtf % ntheseed_size_) + 1;
   index = (index + ntheseed_size_) % ntheseed_size_;
   return vtheseed_[index];
}

float TerrainGenerator_Roblox::getNoise(int x, int z) {
   return getNoise(x, 0, z);
}

float TerrainGenerator_Roblox::getPerlin(float x, float y, float z, int seed, float scale, bool raw) {
   if (!raw) {
       return cperlin_.noise(x * 1.f / scale + seed * 17 + nmaster_seed_, y / scale - nmaster_seed_, z / scale - seed * seed) * .5 + .5;
   }
   return cperlin_.noise(x / scale + seed * 17 + nmaster_seed_, y / scale - nmaster_seed_, z / scale - seed * seed);
}

TerrainGenerator_Roblox::PointVoxelInfo TerrainGenerator_Roblox::findBiomeInfo(TerrainBiomes choiceBiome, int x, int y, int z, float verticalGradientTurbulence) {
   PointVoxelInfo cCurVoxelInfo;

   cCurVoxelInfo.foccupancy_ = 0.5f;
   cCurVoxelInfo.eSurfaceMaterial_ = MaterialType::GRASS;
   cCurVoxelInfo.eFillMaterial_ = MaterialType::ROCK;

   switch (choiceBiome) {
   case TerrainBiomes::Water:
   {
       cCurVoxelInfo.foccupancy_ = .36f + getPerlin(x, y, z, 2, 50) * .08f;
       cCurVoxelInfo.eSurfaceMaterial_ = (1 - verticalGradientTurbulence < .44f ? MaterialType::STONE : MaterialType::SAND); // not proper material: STONE -> SLATE
   }
   break;
   case TerrainBiomes::Marsh:
   {
       float preLedge = getPerlin(x + getPerlin(x, 0, z, 5, 7, true) * 10 + getPerlin(x, 0, z, 6, 30, true) * 50, 0, z + getPerlin(x, 0, z, 9, 7, true) * 10 + getPerlin(x, 0, z, 10, 30, true) * 50, 2, 70);
       float grassyLedge = thresholdFilter(preLedge, .65f, 0);
       float largeGradient = getPerlin(x, y, z, 4, 100), smallGradient = getPerlin(x, y, z, 3, 20);
       float smallGradientThreshold = thresholdFilter(smallGradient, .5f, 0);
       cCurVoxelInfo.foccupancy_ = fwater_level_ - .04f
           + preLedge * grassyLedge*.025f
           + largeGradient * .035f
           + smallGradient * .025f;
       cCurVoxelInfo.eSurfaceMaterial_ = (grassyLedge >= 1 ? MaterialType::GRASSLAND
           : (1 - verticalGradientTurbulence < fwater_level_ - .01f ? MaterialType::MUD
           : (1 - verticalGradientTurbulence < fwater_level_ + .01f ?
               MaterialType::SOIL : MaterialType::GRASSLAND)));
       cCurVoxelInfo.eFillMaterial_ = MaterialType::STONE;
   }
   break;
   case TerrainBiomes::Plains:
   {
       float rivulet = ridgedFlippedFilter(getPerlin(x + getPerlin(x, y, z, 17, 40) * 25, 0, z + getPerlin(x, y, z, 19, 40) * 25, 2, 200));
       float rivuletThreshold = thresholdFilter(rivulet, .01f, 0);

       float rockMap = thresholdFilter(ridgedFlippedFilter(getPerlin(x, 0, z, 101, 7)), .3f, .7f)
           * thresholdFilter(getPerlin(x, 0, z, 102, 50), .6f, .05f);

       cCurVoxelInfo.foccupancy_ = .5f
           + getPerlin(x, y, z, 2, 100) * .02f
           + rivulet * .05f
           + rockMap * .05f
           + rivuletThreshold * .005f;

       float verticalGradient = 1 - (y - 1.f) / (_range.vSize.y - 1.f);
       float surfaceGradient = verticalGradient * .5 + cCurVoxelInfo.foccupancy_ * .5f;
       bool thinSurface = (surfaceGradient > .5f - fsurface_thickness_ * .4f) && (surfaceGradient < .5f + fsurface_thickness_ * .4f);
       cCurVoxelInfo.eSurfaceMaterial_ =
           rockMap > 0 ? MaterialType::ROCK :
           (!thinSurface ? MaterialType::MUD
               : (thinSurface && rivuletThreshold <= 0 ? MaterialType::WATER
                   : (1 - verticalGradientTurbulence < fwater_level_ - .01f ? MaterialType::SAND : MaterialType::GRASSLAND)));
       cCurVoxelInfo.eFillMaterial_ =
           (rockMap > 0 ? MaterialType::ROCK : MaterialType::SANDSTONE);
   }
   break;
   case TerrainBiomes::Canyons:
   {
       float canyonNoise = ridgedFlippedFilter(getPerlin(x, 0, z, 2, 200));
       float canyonNoiseTurbed = ridgedFlippedFilter(getPerlin(x + getPerlin(x, 0, z, 5, 20, true) * 20, 0, z + getPerlin(x, 0, z, 9, 20, true) * 20, 2, 200));
       float sandbank = thresholdFilter(canyonNoiseTurbed, 0, .05f);
       float canyonTop = thresholdFilter(canyonNoiseTurbed, .125f, 0);
       float mesaSlope = thresholdFilter(canyonNoise, .33f, .12f);
       float mesaTop = thresholdFilter(canyonNoiseTurbed, .49f, 0);
       cCurVoxelInfo.foccupancy_ = .42f
           + getPerlin(x, y, z, 2, 70) * .05f
           + canyonNoise * .05f
           + sandbank * .04f
           + thresholdFilter(canyonNoiseTurbed, .05f, 0)*.08f
           + thresholdFilter(canyonNoiseTurbed, .05f, .075f)*.04f
           + canyonTop * .01f

           + thresholdFilter(canyonNoiseTurbed, .0575f, .2725f)*.01f

           + mesaSlope * .06f
           + thresholdFilter(canyonNoiseTurbed, .45f, 0)*.14f
           + thresholdFilter(canyonNoiseTurbed, .45f, .04)*.025f
           + mesaTop * .02f;
       cCurVoxelInfo.eSurfaceMaterial_ =
           (1 - verticalGradientTurbulence < fwater_level_ + .015f ? MaterialType::SAND
               : (sandbank > 0 && sandbank < 1 ? MaterialType::SAND
                   : MaterialType::SANDSTONE));
       MaterialType canyonBandingMaterial[] = { MaterialType::ROCK, MaterialType::MUD,  MaterialType::SAND,  MaterialType::SAND,
                                                MaterialType::SANDSTONE, MaterialType::SANDSTONE, MaterialType::SANDSTONE, MaterialType::SANDSTONE,
                                                MaterialType::SANDSTONE, MaterialType::SANDSTONE };
       int index = std::ceil((1.f - getNoise(1, y, 2)) * 10.f);
       cCurVoxelInfo.eFillMaterial_ = canyonBandingMaterial[index >= 10 || index < 0 ? 0 : index];
   }
   break;
   case TerrainBiomes::Hills:
   {
       float rivulet = std::pow(ridgedFlippedFilter(getPerlin(x + getPerlin(x, y, z, 17, 20) * 20, 0, z + getPerlin(x, y, z, 19, 20) * 20, 2, 200)), 0.5f);
       float largeHills = getPerlin(x, y, z, 3, 60);
       cCurVoxelInfo.foccupancy_ = .48f
           + largeHills * .05f
           + (.05f
               + largeHills * .1
               + getPerlin(x, y, z, 4, 25)*.125f)
           *rivulet;
       float surfaceMaterialGradient = (1 - verticalGradientTurbulence)*.9f + rivulet * .1f;
       cCurVoxelInfo.eSurfaceMaterial_ =
           (surfaceMaterialGradient < fwater_level_ - .015f ? MaterialType::MUD
               : (surfaceMaterialGradient < fwater_level_ ? MaterialType::SOIL
                   : MaterialType::GRASSLAND));
       cCurVoxelInfo.eFillMaterial_ = MaterialType::STONE;
   }
   break;
   case TerrainBiomes::Dunes:
   {
       float duneTurbulence = getPerlin(x, 0, z, 227, 20) * 24;
       float layer1 = ridgedFilter(getPerlin(x, 0, z, 201, 40));
       float layer2 = ridgedFilter(getPerlin(x / 10 + duneTurbulence, 0, z + duneTurbulence, 200, 48));
       cCurVoxelInfo.foccupancy_ = .4f + .1f * (layer1 + layer2);
       cCurVoxelInfo.eSurfaceMaterial_ = MaterialType::SAND;
       cCurVoxelInfo.eFillMaterial_ = MaterialType::SANDSTONE;
   }
   break;
   case TerrainBiomes::Mountains:
   {
       float rivulet = ridgedFlippedFilter(getPerlin(x + getPerlin(x, y, z, 17, 20) * 20, 0, z + getPerlin(x, y, z, 19, 20) * 20, 2, 200));
       cCurVoxelInfo.foccupancy_ = -.4f
           //+ fractalize(mountainsOperation, x, y / 20, z, 8, .65) * 1.2
           + fractalize([&](int x, int y, int z, int i) {
           return ridgedFilter(getPerlin(x, y, z, 100 + i, (1.f / i) * 160));
               }, x, y / 20, z, 8, .65f) * 1.2f
           + rivulet * .2f;
               cCurVoxelInfo.eSurfaceMaterial_ =
                   (verticalGradientTurbulence < .275f ? MaterialType::SNOW
                       : (verticalGradientTurbulence < .35f ? MaterialType::ROCK
                           : (verticalGradientTurbulence < .4f ? MaterialType::SOIL
                               : (1 - verticalGradientTurbulence < fwater_level_ ? MaterialType::ROCK
                                   : (1 - verticalGradientTurbulence < fwater_level_ + .01f ? MaterialType::MUD
                                       : (1 - verticalGradientTurbulence < fwater_level_ + .015f ? MaterialType::SOIL
                                           : MaterialType::GRASSLAND))))));
   }
   break;
   case TerrainBiomes::Lavaflow:
   {
       float crackX = x + getPerlin(x, y*.25f, z, 21, 8, true) * 5;
       float crackY = y + getPerlin(x, y*.25f, z, 22, 8, true) * 5;
       float crackZ = z + getPerlin(x, y*.25f, z, 23, 8, true) * 5;
       float crack1 = ridgedFilter(getPerlin(crackX + getPerlin(x, y, z, 22, 30, true) * 30, crackY, crackZ + getPerlin(x, y, z, 24, 30, true) * 30, 2, 120));
       float crack2 = ridgedFilter(getPerlin(crackX, crackY, crackZ, 3, 40))*(crack1*.25f + .75f);
       float crack3 = ridgedFilter(getPerlin(crackX, crackY, crackZ, 4, 20))*(crack2*.25f + .75f);

       float generalHills = thresholdFilter(getPerlin(x, y, z, 9, 40), .25f, .5f)*getPerlin(x, y, z, 10, 60);

       float cracks = std::max(0., 1. - thresholdFilter(crack1, .975f, 0) - thresholdFilter(crack2, .925f, 0) - thresholdFilter(crack3, .9f, 0));

       Vector3 spireVec = Vector3(.7, .7, 0) * Vector3(crackX, crackY, crackZ);
       float spires = thresholdFilter(getPerlin(spireVec.x / 40, spireVec.y / 300, spireVec.z / 30, 123, 1), .6f, .4f);

       cCurVoxelInfo.foccupancy_ = fwater_level_ + .02f
           + cracks * (.5 + generalHills * .5f)*.02f
           + generalHills * .05f
           + spires * .3f
           + ((1 - verticalGradientTurbulence > fwater_level_ + .01 || spires > 0) ? .04f : 0);

       cCurVoxelInfo.eFillMaterial_ = (spires > 0 ? MaterialType::ROCK : (cracks < 1 ? MaterialType::LAVA : MaterialType::BASALT));
       cCurVoxelInfo.eSurfaceMaterial_ = (cCurVoxelInfo.eFillMaterial_ == MaterialType::LAVA && 1 - verticalGradientTurbulence < fwater_level_ ? MaterialType::BASALT : cCurVoxelInfo.eFillMaterial_);
   }
   break;
   case TerrainBiomes::Arctic:
   {
       float preBoundary = getPerlin(x + getPerlin(x, 0, z, 5, 8, true) * 5, y / 8, z + getPerlin(x, 0, z, 9, 8, true) * 5, 2, 20);
       float roughChunks = getPerlin(x, y / 4, z, 436, 2);
       float boundary = ridgedFilter(preBoundary);
       float boundaryMask = thresholdFilter(boundary, .8f, .1f);
       float boundaryTypeMask = getPerlin(x, 0, z, 6, 74) - .5f;
       float boundaryComp = 0;
       if (boundaryTypeMask < 0)
           boundaryComp = (boundary > (1 + boundaryTypeMask * .5f) ? -.17f : 0);
       else
           boundaryComp = boundaryMask * .1*roughChunks * boundaryTypeMask;
       cCurVoxelInfo.foccupancy_ = .55f
           + boundary * .05f * boundaryTypeMask
           + boundaryComp
           + getPerlin(x, 0, z, 123, 25) * .025f;

       cCurVoxelInfo.eSurfaceMaterial_ = (1 - verticalGradientTurbulence < fwater_level_ - .1f ? MaterialType::ICE : (boundaryMask > .6f && boundaryTypeMask > .1f && roughChunks > .5f ? MaterialType::ICE : MaterialType::SNOW));
       cCurVoxelInfo.eFillMaterial_ = MaterialType::ICE;
   }
   break;
   default:
       break;
   }
   return cCurVoxelInfo;
}

float TerrainGenerator_Roblox::findBiomeTransitionValue(TerrainBiomes biome, float weight, float value, float averageValue) {
   if (biome == TerrainBiomes::Arctic) {
       return (weight > .2f ? 1 : 0) * value;
   }
   else if (biome == TerrainBiomes::Canyons) {
       return (weight > .7f ? 1 : 0) * value;
   }
   else if (biome == TerrainBiomes::Mountains) {
       weight = std::pow(weight, 3);
       return averageValue * (1 - weight) + value * weight;
   }
   return averageValue * (1 - weight) + value * weight;
}

void TerrainGenerator_Roblox::generateTerrainByBiomes(VoxelBrush* pBrush, BiomesParam& biomeParams, int max_lod) {
    if (!biomeParams.use_biomes) {
        //terrain version 1.0
        genTotallyFlat(pBrush);

        progress = 100;
        return;
    }
    //terrain version 2.0, maybe 3.0
    if (biomeParams.biomes_be_checked != nbiomes_be_checked_) {
        setBiomesBeChecked(biomeParams.biomes_be_checked);
    }
    setIncludeCaves(biomeParams.generate_caves);
    setBiomeSize(biomeParams.biome_size);
    setTerrainGeneratorSeed(biomeParams.seed);

    float foccupancy_scale = .5f / _range.vSize.y;

    //init data
    if (vbiomes_.size() <= 0) {
        vbiomes_.push_back(TerrainBiomes::Hills);
    }

    // middle result
    TerrainBiomes biome; //current biome type
    std::vector<PointDistNoiseInfo> biomePoints;
    std::unordered_map<TerrainBiomes, PointVoxelInfo> weightPoints;
    // bsurface: if current position is surface of one biome
    bool bBiomeNoCave = false, bis_biome_surface = false;
    Vector3i posi;
    float closestDistance, dist, weightTotal, preCaveComp = 0.f, comp = 0.f;
    Vector3 cellToBiome = Vector3(0), gridPoint = Vector3(0), point = Vector3(0);
    uint32_t nbiomesSize = vbiomes_.size();
    // final result
    float verticalGradient, fcaves, verticalGradientTurbulence, faverage_occupancy, value;
    PointVoxelInfo cCurVoxelInfo;
    float fnew_occupancy = 0.f, fnew_sdf = 0.f;
    MaterialType fnew_material = MaterialType::AIR;

    //for each voxel
    for (posi.x = 1; posi.x <= _range.vSize.x; ++posi.x) {
        for (posi.z = 1; posi.z <= _range.vSize.z; ++posi.z) {
            bBiomeNoCave = false;
            cellToBiome.x = posi.x * 1.f / nbiome_size_ + getPerlin(posi.x, 0, posi.z, 233, nbiome_size_ * .3f) * .25f + getPerlin(posi.x, 0, posi.z, 235, nbiome_size_ * .05f) * .075f;
            cellToBiome.z = posi.z * 1.f / nbiome_size_ + getPerlin(posi.x, 0, posi.z, 234, nbiome_size_ * .3f) * .25f + getPerlin(posi.x, 0, posi.z, 236, nbiome_size_ * .05f) * .075f;

            closestDistance = 1000000;
            biomePoints.clear();
            weightTotal = 0;

            for (int vx = -1; vx <= 1; ++vx) {
                for (int vz = -1; vz <= 1; ++vz) {
                    gridPoint.x = std::floor(cellToBiome.x + vx + .5f);
                    gridPoint.z = std::floor(cellToBiome.z + vz + .5f);

                    point.x = gridPoint.x + (getNoise(gridPoint.x, gridPoint.z, 53) - .5f) * .75f;
                    point.z = gridPoint.z + (getNoise(gridPoint.x, gridPoint.z, 73) - .5f) * .75f;

                    dist = point.distanceTo(cellToBiome);
                    if (dist < closestDistance) {
                        closestDistance = dist;
                    }

                    biomePoints.push_back(PointDistNoiseInfo(dist, getNoise(gridPoint.x, gridPoint.z)));
                }
            }
            weightPoints.clear();
            for (auto point : biomePoints) {
                float weight = ((point.fdist_ == closestDistance) ? 1 : (closestDistance / point.fdist_ - fbiome_blend_percent_inverse_) / fbiome_blend_percent_);
                if (weight > 0) {
                    weight = std::pow(weight, 2.1);
                    weightTotal += weight;
                    int index = std::ceil(nbiomesSize * (1.f - point.fbiome_noise_));
                    biome = vbiomes_[index >= nbiomesSize ? index - 1 : index];

                    if (weightPoints.find(biome) != weightPoints.end()) {
                        weightPoints[biome].fweight_ += weight;
                    }
                    else {
                        weightPoints[biome] = PointVoxelInfo(weight);
                    }
                }
            }

            for (auto itr = weightPoints.begin(); itr != weightPoints.end(); itr++) {
                itr->second.fweight_ /= weightTotal;
                if (itr->first == TerrainBiomes::Arctic) {
                    bBiomeNoCave = true;
                }
            }

            for (posi.y = 1; posi.y <= _range.vSize.y; ++posi.y) {
                verticalGradient = 1.f - (posi.y - 1.f) / (_range.vSize.y - 1.f);
                verticalGradientTurbulence = verticalGradient * .9f + .1f * getPerlin(posi.x, posi.y, posi.z, 107, 15);
                cCurVoxelInfo.foccupancy_ = 0.f;
                cCurVoxelInfo.eSurfaceMaterial_ = MaterialType::LAVA;
                cCurVoxelInfo.eFillMaterial_ = MaterialType::ROCK;
                fcaves = 0.f;

                //surface of every biome
                if (verticalGradient > .65f || verticalGradient < .1f) {
                    cCurVoxelInfo.foccupancy_ = .5f;
                }
                else if (nbiomesSize == 1) {
                    cCurVoxelInfo = findBiomeInfo(vbiomes_[0], posi.x, posi.y, posi.z, verticalGradientTurbulence);
                }
                else {
                    faverage_occupancy = 0.f;
                    for (auto itr = weightPoints.begin(); itr != weightPoints.end(); itr++) {
                        auto &&info = findBiomeInfo(itr->first, posi.x, posi.y, posi.z, verticalGradientTurbulence);
                        itr->second.foccupancy_ = info.foccupancy_;
                        itr->second.eSurfaceMaterial_ = info.eSurfaceMaterial_;
                        itr->second.eFillMaterial_ = info.eFillMaterial_;
                        faverage_occupancy += info.foccupancy_ * itr->second.fweight_;
                    }
                    for (auto itr = weightPoints.begin(); itr != weightPoints.end(); itr++) {
                        value = findBiomeTransitionValue(itr->first, itr->second.fweight_, itr->second.foccupancy_, faverage_occupancy);
                        if (value > cCurVoxelInfo.foccupancy_) {
                            cCurVoxelInfo.foccupancy_ = value;
                            cCurVoxelInfo.eSurfaceMaterial_ = itr->second.eSurfaceMaterial_;
                            cCurVoxelInfo.eFillMaterial_ = itr->second.eFillMaterial_;
                        }
                    }
                }

                preCaveComp = verticalGradient * .5f + cCurVoxelInfo.foccupancy_ * .5f;

                bis_biome_surface = ((preCaveComp > .5f - fsurface_thickness_) && (preCaveComp < .5f + fsurface_thickness_));

                if (binclude_caves_
                    && (!bBiomeNoCave || verticalGradient > .65f)
                    && !(bis_biome_surface && (1 - verticalGradient) < fwater_level_ + .005f)
                    && !(bis_biome_surface && (1 - verticalGradient) > fwater_level_ + .58f)) {
                    float ridged[3], caves[3];
                    fcaves = 1.f;
                    for (int i = 0; i < 3; ++i) {
                        ridged[i] = ridgedFilter(getPerlin(posi.x, posi.y, posi.z, 4 + i, _range.vSize.x));
                        caves[i] = thresholdFilter(ridged[i], .84f, .01f);
                        fcaves *= caves[i];
                    }

                    if (bis_biome_surface) {
                        fcaves -= thresholdFilter(getPerlin(posi.x, posi.y, posi.z, 143, 62), .35f, 0);
                    }
                    fcaves = (fcaves < 0 ? 0 : (fcaves > 1 ? 1 : fcaves));
                }

                comp = preCaveComp - fcaves;
                fnew_occupancy = thresholdFilter(comp, .5f, foccupancy_scale);
                // below water level and above surface, has no terrain
                if (1.f - verticalGradient < fwater_level_ && preCaveComp <= .5f && fnew_occupancy <= 0) {
                    fnew_occupancy = 1.f;
                    cCurVoxelInfo.eSurfaceMaterial_ = MaterialType::WATER;
                    cCurVoxelInfo.eFillMaterial_ = MaterialType::WATER;
                    bis_biome_surface = true;
                }

                fnew_sdf = (posi.y == 1 ? -1 : translateSdfAndOccupancy(fnew_occupancy, false));
                if (fnew_sdf <= 0) {
                    fnew_material = (posi.y == 1 ? MaterialType::LAVA : (bis_biome_surface ? cCurVoxelInfo.eSurfaceMaterial_ : cCurVoxelInfo.eFillMaterial_));
                }
                else {
                    fnew_material = MaterialType::AIR;
                }
                pBrush->set_voxel_info(Vector3i(posi - Vector3(1)).to_vec3() + _range.vStart, fnew_sdf, fnew_material);

            }
        }

        progress = posi.x / _range.vSize.x;
    }

    progress = 100;
}

//void TerrainGenerator_Roblox::generateTerrainByBiomes(VoxelToolTerrain* pVoxelTool, BiomesParam& biomeParams, int max_lod) {
//   if (!biomeParams.use_biomes) {
//       //terrain version 1.0
//       genTotallyFlat(pVoxelTool);
//       return;
//   }
//   //terrain version 2.0, maybe 3.0
//   if (biomeParams.biomes_be_checked != nbiomes_be_checked_) {
//       setBiomesBeChecked(biomeParams.biomes_be_checked);
//   }
//   setIncludeCaves(biomeParams.generate_caves);
//   setBiomeSize(biomeParams.biome_size);
//   setTerrainGeneratorSeed(biomeParams.seed);
//
//   float foccupancy_scale = .5f / _range.vSize.y;
//
//   //init data
//   if (vbiomes_.size() <= 0) {
//       vbiomes_.push_back(TerrainBiomes::Hills);
//   }
//
//   // middle result
//   TerrainBiomes biome; //current biome type
//   std::vector<PointDistNoiseInfo> biomePoints;
//   std::unordered_map<TerrainBiomes, PointVoxelInfo> weightPoints;
//   // bsurface: if current position is surface of one biome
//   bool bBiomeNoCave = false, bis_biome_surface = false;
//   Vector3i posi;
//   float closestDistance, dist, weightTotal, preCaveComp = 0.f, comp = 0.f;
//   Vector3 cellToBiome = Vector3(0), gridPoint = Vector3(0), point = Vector3(0);
//   uint32_t nbiomesSize = vbiomes_.size();
//   // final result
//   float verticalGradient, fcaves, verticalGradientTurbulence, faverage_occupancy, value;
//   PointVoxelInfo cCurVoxelInfo;
//   float fnew_occupancy = 0.f, fnew_sdf = 0.f;
//   MaterialType fnew_material = MaterialType::AIR;
//
//   //for each voxel
//   for (posi.x = 1; posi.x <= _range.vSize.x; ++posi.x) {
//       for (posi.z = 1; posi.z <= _range.vSize.z; ++posi.z) {
//           bBiomeNoCave = false;
//           cellToBiome.x = posi.x * 1.f / nbiome_size_ + getPerlin(posi.x, 0, posi.z, 233, nbiome_size_ * .3f) * .25f + getPerlin(posi.x, 0, posi.z, 235, nbiome_size_ * .05f) * .075f;
//           cellToBiome.z = posi.z * 1.f / nbiome_size_ + getPerlin(posi.x, 0, posi.z, 234, nbiome_size_ * .3f) * .25f + getPerlin(posi.x, 0, posi.z, 236, nbiome_size_ * .05f) * .075f;
//
//           closestDistance = 1000000;
//           biomePoints.clear();
//           weightTotal = 0;
//
//           for (int vx = -1; vx <= 1; ++vx) {
//               for (int vz = -1; vz <= 1; ++vz) {
//                   gridPoint.x = std::floor(cellToBiome.x + vx + .5f);
//                   gridPoint.z = std::floor(cellToBiome.z + vz + .5f);
//
//                   point.x = gridPoint.x + (getNoise(gridPoint.x, gridPoint.z, 53) - .5f) * .75f;
//                   point.z = gridPoint.z + (getNoise(gridPoint.x, gridPoint.z, 73) - .5f) * .75f;
//
//                   dist = point.distanceTo(cellToBiome);
//                   if (dist < closestDistance) {
//                       closestDistance = dist;
//                   }
//
//                   biomePoints.push_back(PointDistNoiseInfo(dist, getNoise(gridPoint.x, gridPoint.z)));
//               }
//           }
//           weightPoints.clear();
//           for (auto point : biomePoints) {
//               float weight = ((point.fdist_ == closestDistance) ? 1 : (closestDistance / point.fdist_ - fbiome_blend_percent_inverse_) / fbiome_blend_percent_);
//               if (weight > 0) {
//                   weight = std::pow(weight, 2.1);
//                   weightTotal += weight;
//                   int index = std::ceil(nbiomesSize * (1.f - point.fbiome_noise_));
//                   biome = vbiomes_[index >= nbiomesSize ? index - 1 : index];
//
//                   if (weightPoints.find(biome) != weightPoints.end()) {
//                       weightPoints[biome].fweight_ += weight;
//                   }
//                   else {
//                       weightPoints[biome] = PointVoxelInfo(weight);
//                   }
//               }
//           }
//
//           for (auto itr = weightPoints.begin(); itr != weightPoints.end(); itr++) {
//               itr->second.fweight_ /= weightTotal;
//               if (itr->first == TerrainBiomes::Arctic) {
//                   bBiomeNoCave = true;
//               }
//           }
//
//           for (posi.y = 1; posi.y <= _range.vSize.y; ++posi.y) {
//               verticalGradient = 1.f - (posi.y - 1.f) / (_range.vSize.y - 1.f);
//               verticalGradientTurbulence = verticalGradient * .9f + .1f * getPerlin(posi.x, posi.y, posi.z, 107, 15);
//               cCurVoxelInfo.foccupancy_ = 0.f;
//               cCurVoxelInfo.eSurfaceMaterial_ = MaterialType::LAVA;
//               cCurVoxelInfo.eFillMaterial_ = MaterialType::ROCK;
//               fcaves = 0.f;
//
//               //surface of every biome
//               if (verticalGradient > .65f || verticalGradient < .1f) {
//                   cCurVoxelInfo.foccupancy_ = .5f;
//               }
//               else if (nbiomesSize == 1) {
//                   cCurVoxelInfo = findBiomeInfo(vbiomes_[0], posi.x, posi.y, posi.z, verticalGradientTurbulence);
//               }
//               else {
//                   faverage_occupancy = 0.f;
//                   for (auto itr = weightPoints.begin(); itr != weightPoints.end(); itr++) {
//                       auto &&info = findBiomeInfo(itr->first, posi.x, posi.y, posi.z, verticalGradientTurbulence);
//                       itr->second.foccupancy_ = info.foccupancy_;
//                       itr->second.eSurfaceMaterial_ = info.eSurfaceMaterial_;
//                       itr->second.eFillMaterial_ = info.eFillMaterial_;
//                       faverage_occupancy += info.foccupancy_ * itr->second.fweight_;
//                   }
//                   for (auto itr = weightPoints.begin(); itr != weightPoints.end(); itr++) {
//                       value = findBiomeTransitionValue(itr->first, itr->second.fweight_, itr->second.foccupancy_, faverage_occupancy);
//                       if (value > cCurVoxelInfo.foccupancy_) {
//                           cCurVoxelInfo.foccupancy_ = value;
//                           cCurVoxelInfo.eSurfaceMaterial_ = itr->second.eSurfaceMaterial_;
//                           cCurVoxelInfo.eFillMaterial_ = itr->second.eFillMaterial_;
//                       }
//                   }
//               }
//
//               preCaveComp = verticalGradient * .5f + cCurVoxelInfo.foccupancy_ * .5f;
//
//               bis_biome_surface = ((preCaveComp > .5f - fsurface_thickness_) && (preCaveComp < .5f + fsurface_thickness_));
//
//               if (binclude_caves_
//                   && (!bBiomeNoCave || verticalGradient > .65f)
//                   && !(bis_biome_surface && (1 - verticalGradient) < fwater_level_ + .005f)
//                   && !(bis_biome_surface && (1 - verticalGradient) > fwater_level_ + .58f)) {
//                   float ridged[3], caves[3];
//                   fcaves = 1.f;
//                   for (int i = 0; i < 3; ++i) {
//                       ridged[i] = ridgedFilter(getPerlin(posi.x, posi.y, posi.z, 4 + i, 30));
//                       caves[i] = thresholdFilter(ridged[i], .84f, .01f);
//                       fcaves *= caves[i];
//                   }
//
//                   if (bis_biome_surface) {
//                       fcaves -= thresholdFilter(getPerlin(posi.x, 0, posi.z, 143, 62), .35f, 0);
//                   }
//                   fcaves = (fcaves < 0 ? 0 : (fcaves > 1 ? 1 : fcaves));
//               }
//
//               comp = preCaveComp - fcaves;
//               fnew_occupancy = thresholdFilter(comp, .5f, foccupancy_scale);
//               // below water level and above surface, has no terrain
//               if (1.f - verticalGradient < fwater_level_ && preCaveComp <= .5f && fnew_occupancy <= 0) {
//                   fnew_occupancy = 1.f;
//                   cCurVoxelInfo.eSurfaceMaterial_ = MaterialType::WATER;
//                   cCurVoxelInfo.eFillMaterial_ = MaterialType::WATER;
//                   bis_biome_surface = true;
//               }
//
//               fnew_sdf = (posi.y == 1 ? -1 : translateSdfAndOccupancy(fnew_occupancy, false));
//               if (fnew_sdf <= 0) {
//                   fnew_material = (posi.y == 1 ? MaterialType::LAVA : (bis_biome_surface ? cCurVoxelInfo.eSurfaceMaterial_ : cCurVoxelInfo.eFillMaterial_));
//               }
//               else {
//                   fnew_material = MaterialType::AIR;
//               }
//               pVoxelTool->set_voxel_info(Vector3i(posi - Vector3(1)).to_vec3() + _range.vStart, fnew_sdf, fnew_material);
//
//               if ((rand() % 10 == 0) && (fnew_sdf > 0) && (pVoxelTool->get_voxel_f(Vector3i(posi - Vector3(1)) - Vector3(0, 1, 0), VoxelBuffer::CHANNEL_SDF) <= 0)) {
//                    m_quTreeRoot.push(Vector3i(posi - Vector3(1)).to_vec3() - Vector3(0, 1, 0));
//               }
//           }
//       }
//   }
//   //generateTrees(pVoxelTool);
//}

//void TerrainGenerator_Roblox::readDataFromFiles(std::string sDataFileName, VoxelToolTerrain* pVoxelTool) {
//   std::ifstream ifs(sDataFileName);
//   if (!ifs.is_open()) {
//       return;
//   }
//   Vector3 pos;
//   MaterialType material;
//   float sdf;
//   std::string value;
//   while (std::getline(ifs, value)) {
//       sscanf(value.c_str(), "%f %f %f %f %hhd", &pos.x, &pos.y, &pos.z, &sdf, &material);
//       sdf = (sdf <= 0 ? 1 : -sdf);
//       pVoxelTool->set_voxel_f(pos, sdf);
//   }
//   ifs.close();
//}


