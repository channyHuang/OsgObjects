#pragma once

#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"

namespace HG_GDAL {
    void translateSystem(const std::string& sFileName);
    bool convertModel(const std::string& inputFile, const std::string& outputDir);

    void Init(OGRCoordinateTransformation *pOgrCT, double *Origin);
};
