#include "ImguiMainPage.h"

#include "osgManager.h"

#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h"

ImguiMainPage::ImguiMainPage() {
    cFileName = new char[nMaxFileNameLength];
    cOtherFileName = new char[nMaxFileNameLength];
}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer) {
    pviewer = &viewer;

    OsgManager::getInstance()->setViewer(viewer);
}

ImguiMainPage::~ImguiMainPage() {
    pviewer = nullptr;
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("GDAL");
    if (ImGui::Button("Switch Scene")) {
    }
    if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {
    }
    if (ImGui::InputTextWithHint("path", "gdal-data", cOtherFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {
    }
    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("GIS functions")) {
            if (ImGui::Button("translate geometric system")) {
                sFileName = std::string(cFileName);
                std::string sOtherFileName = std::string(cOtherFileName);
                if (sOtherFileName.length() <= 0) {
                    sOtherFileName = "E:/thirdLibs/3rdparty/gdal-data";
                }
                CPLSetConfigOption("GDAL_DATA", sOtherFileName.c_str());

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

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
