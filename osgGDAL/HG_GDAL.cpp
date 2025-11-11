#include "HG_GDAL.h"

#include <iostream>

namespace HG_GDAL {

void translateSystem(const std::string& sFileName) {
    CPLSetConfigOption("GDAL_DATA", sFileName.c_str());
    char* pszWKT = nullptr;
    OGRSpatialReference oInGRS, oOutGRS;
    //oInGRS.SetWellKnownGeogCS("WGS84");
    //oOutGRS.SetWellKnownGeogCS("CGCS2000");

    oInGRS.importFromEPSG(4326); //wgs84
    oInGRS.exportToPrettyWkt(&pszWKT);
    std::cout << "wgs84: " << pszWKT << std::endl;

    oOutGRS.importFromEPSG(4490); //cgcs2000
    oOutGRS.exportToPrettyWkt(&pszWKT);
    std::cout << "cgcs2000: " << pszWKT << std::endl;

    oOutGRS.SetTM(0, 114, 1.0, 38500000, 0);

    OGRCoordinateTransformation* transform = OGRCreateCoordinateTransformation(&oInGRS, &oOutGRS);
    double x = 113.3665, y = 23.2047;
    transform->Transform(1, &x, &y);
    std::cout << x << " " << y << std::endl;
}

bool convertModel(const std::string& inputFile, const std::string& outputDir) {
    // 初始化GDAL
    GDALAllRegister();
    
    // 打开OSGB文件
    GDALDatasetH hSrcDS = GDALOpenEx(
        inputFile.c_str(), 
        GDAL_OF_VECTOR | GDAL_OF_READONLY, 
        NULL, NULL, NULL);
        
    if (!hSrcDS) {
        std::cerr << "无法打开文件: " << inputFile << std::endl;
        return false;
    }
    
    // 检查3D Tiles驱动
    GDALDriverH hDriver = GDALGetDriverByName("3DTiles");
    if (!hDriver) {
        std::cerr << "3D Tiles驱动不可用" << std::endl;
        GDALClose(hSrcDS);
        return false;
    }
    
    // 转换选项
    char** papszOptions = NULL;
    papszOptions = CSLSetNameValue(papszOptions, "FORMAT", "B3DM");
    papszOptions = CSLSetNameValue(papszOptions, "CREATION_OPTIONS", "MAX_FEATURES=1000");
    
    // 执行转换
    GDALDatasetH hDstDS = GDALCreateCopy(
        hDriver, 
        (outputDir + "/tileset.json").c_str(), 
        hSrcDS, 
        FALSE, 
        papszOptions, 
        NULL, NULL);
    
    if (hDstDS) {
        std::cout << "转换成功: " << outputDir << std::endl;
        GDALClose(hDstDS);
    } else {
        std::cerr << "转换失败" << std::endl;
    }
    
    // 清理
    CSLDestroy(papszOptions);
    GDALClose(hSrcDS);
    
    return hDstDS != NULL;
}

}