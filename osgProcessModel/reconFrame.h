#ifndef RECONFRAME_H
#define RECONFRAME_H

#include <iostream>

#include "commonOsg/commonOsg.h"

class ReconFrame {
public:
    static ReconFrame* getInstance() {
        if (instance == nullptr) {
            instance = new ReconFrame;
        }
        return instance;
    }
    
    ~ReconFrame();

    void reconFrame(const std::string& sFilePath);

private:
    ReconFrame();

    static ReconFrame* instance;
};

#endif