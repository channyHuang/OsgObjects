#ifndef MESHGENERATORMANAGER_H
#define MESHGENERATORMANAGER_H

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <mutex>
#include <unordered_set>

#include "spdlog/spdlog.h"

#include "commonFunc/threadPool.h"
#include "commonFunc/signalSlots.h"

#include "terrains/commonStruct.h"

class MeshGeneratorManager {
public:
    explicit MeshGeneratorManager();
    ~MeshGeneratorManager();

    void push(const Input& input);
    void pop(Output &output);

    int get_minimum_padding() const { return _minimum_padding; }
    int get_maximum_padding() const { return _maximum_padding; }


    void process();
    void sltFinish(OutputBlock output);
    
// signal
    SignalSlot::Signal<void(OutputBlock)> sigFinish;

private:
    Output outputBlocks;
    int index = 0;
    int _minimum_padding = 2;
    int _maximum_padding = 2;
    std::mutex m_mutexOutput;

    ThreadPool pool;

    std::unordered_set<Vector3i, Vector3iHash> sets;
};

#endif // MESHGENERATORMANAGER_H
