#pragma once

#include "gdal/gdal.h"
#include "gdal/gdal_priv.h"
#include "gdal/ogr_spatialref.h"
#include <gdal/ogrsf_frmts.h>

namespace HG_GDAL {
    void translateSystem(const std::string& sFileName);
    bool convertModel(const std::string& inputFile, const std::string& outputDir);

    void Init(OGRCoordinateTransformation *pOgrCT, double *Origin);
};
