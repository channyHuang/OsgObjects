#pragma once

#include "trainer.h"
#include "policyNet.h"

#include "dataStruct.h"

class TrainTestCol {
public:
	TrainTestCol();

	void test(Trainer& trainer);
	void train(Trainer& trainer);
	int colliborate();
	void optimize(Trainer& trainer);

	void train(Trainer& trainer, int nNumOfPlatform);

	int Cood2Index(int x, int y, int dim) {
		return abs(x) * dim + abs(y);
	}

	int nTerrainDim = 100;
	int nMaxTerrainHeight = 100;
	uint32_t nStateDim = 5;
};