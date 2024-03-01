#ifndef COMMON_DEF_H
#define COMMON_DEF_H

#include <vector>
#include <string>

namespace ReconSpace
{
    // recon
    enum AlgoSimplify {
        SimplifyGrid,
        SimplifyWLOP,
        SimplifyHierarchy,
        SimplifyRandom
    };

    enum AlgoSmooth {
        SmoothJet,
        SmoothBilateral,
        SmoothWeightPCA
    };

    enum AlgoReconstruct {
        ReconPoisson,
        ReconAdvancingFront,
        ReconSpace
    };

    enum ReconStep {
        Step_OutlierRemove = 0,
        Step_Cluster,
        Step_Simplify,
        Step_Smooth,
        Step_Normal,
        Step_Reconstruct,
        Step_Reconstruct_All,
        Step_Show,
        Step_Unwrap,
        Step_Max_Count
    };

    static std::vector<std::string> vAlgoReconStr = { "Poission", "Advancing Front", "Scale Spacing" };
    static std::vector<std::string> vAlgoSmoothStr = { "JetSmooth", "Bilateral", "weightPCA" };
    static std::vector<std::string> vAlgoSimplifyStr = { "Grid", "WLOP", "Hierarchy", "Random" };
    static bool vCheckRecon[3] = { true, false, false };

    // terrain
    static std::vector<std::string> vBiomeStr = { "Water", "Marsh", "Plains", "Hills", "Dunes", "Canyons", "Mountains", "Lavaflow", "Arctic" };
    static bool vBiomeCheck[9] = { false, false, false, true };
    static std::vector<std::string> vBrushStr = { "Sphere", "Square" };
    static std::vector<std::string> vModifyStr = { "Add", "Reduce" };
    static std::vector<std::string> vMaterialStr = { "GRASS", "GRASSLAND", "Wood"};

    struct ReconStepParam {
        ReconStepParam() {}
        ReconStepParam(bool _bUse, int _nStepEnum, int _nAlgorithmEnum)
            : bUse(_bUse), nStepEnum(_nStepEnum), nAlgorithmEnum(_nAlgorithmEnum) {}

        bool bUse = false;
        int nStepEnum = 0;
        int nAlgorithmEnum = 0;
    };

    struct ReconParams {
        ReconParams() {}

        ReconStep eStepEnum;
        std::string sFileName;
        ReconStepParam stStepParam[Step_Max_Count];
    };

    // data
    static ReconParams g_stReconParams;
}

#endif