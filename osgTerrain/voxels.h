#pragma once

#include <unordered_map>

#include "commonMath/vector3i.h"

const int BLOCKBIT = 4;
const int CHUNKBIT = (BLOCKBIT + BLOCKBIT);
const int BLOCK (1 << BLOCKBIT)
const int CHUNK (1 << CHUNKBIT)

enum MaterialType : class uint8_t {
	MaterialMud,
	MaterialGlass
};

Vector3i coord2ChunkGlobal(Vector3i vPos) {
	Vector3i vPosChunkGlobal;
	vPosChunkGlobal.x = (vPos.x >> CHUNKBIT);
	vPosChunkGlobal.y = (vPos.y >> CHUNKBIT);
	vPosChunkGlobal.z = (vPos.z >> CHUNKBIT);
	return vPosChunkGlobal;
}

Vector3i coord2ChunkLocal(Vector3i vPos) {
	Vector3i vPosChunkLocal;
	vPosChunkLocal.x = (vPos.x % CHUNK);
	vPosChunkLocal.y = (vPos.y % CHUNK);
	vPosChunkLocal.z = (vPos.z % CHUNK);
	return vPosChunkLocal;
}

Vector3i coordChunk2BlockGlobal(Vector3i vPosChunkLocal) {
	Vector3i vPosBlockGlobal;
	vPosBlockGlobal.x = (vPosChunkLocal.x >> BLOCKBIT);
	vPosBlockGlobal.y = (vPosChunkLocal.y >> BLOCKBIT);
	vPosBlockGlobal.z = (vPosChunkLocal.z >> BLOCKBIT);
	return vPosBlockGlobal;
}

Vector3i coordChunk2BlockLocal(Vector3i vPosChunkLocal) {
	Vector3i vPosBlockLocal;
	vPosBlockLocal.x = (vPosChunkLocal.x % BLOCK);
	vPosBlockLocal.y = (vPosChunkLocal.y % BLOCK);
	vPosBlockLocal.z = (vPosChunkLocal.z % BLOCK);
	return vPosBlockLocal;
}

int coord2Index(Vector3i vPos) {
	int index = vPos.x;
	index = (index << BIT) + vPos.y;
	index = (index << BIT) + vPos.z;
	return index;
}

// [-1, 1] -> [255, 0]
uint8_t fSdf2nSdf(float fSdf) {
	if (fSdf >= 1) return 0;
	if (fSdf <= -1) return 255;
	return (1 - fSdf) * 127.5;
}

float nSdf2fSdf(uint8_t nSdf) {
	return 1 - nSdf / 127.5;
}
// sdf [0, 255]
class Voxel {
public:
	Voxel() {}
	~Voxel() {}

	uint8_t nSdf;
	MaterialType mEnumMaterial;
};

// (x,y,z) [0,2^BIT]
class Block {
public:
	Block(const Vector3i& vBlockPos) : mVBlockPos(vBlockPos) {}
	~Block() {
		mMapVoxels.clear();
	}

	void setVoxel(Vector3i vPos, float fSdf) {
		int index = coord2Index(vPos);
		auto itr = mMapVoxels.find(index);
		if (itr == mMapVoxels.end()) {
			if (nSdf == 0) return;
			mMapVoxels.insert(index, nSdf);
			if (nSdf <= 127) mNOutsideCount++;
			else mNInsideCount++;
		}
		else {
			if (itr->second <= 127) mNOutsideCount--;
			else mNInsideCount--;

			if (nSdf == 0) {
				mMapVoxels.erase(index);
			}
			else {
				mMapVoxels[index] = nSdf;
				if (nSdf <= 127) mNOutsideCount++;
				else mNInsideCount++;
			}
		}
	}

private:
	Vector3i mVBlockPos;
	std::unordered_map<int, Voxel> mMapVoxels;
	int mNOutsideCount;
	int mNInsideCount;
};
// (x,y,z) [0,2^(2*BIT)]
class Chunk {
public:
	Chunk(const Vector3i& vChunkPos) : mVChunkPos(vChunkPos) {}
	~Chunk() {}

	void setVoxel(Vector3i vChunkPos, uint8_t nSdf) {
		Vector3i vPosBlockGlobal = coord2BlockGlobal(vChunkPos);
		Vector3i vPosBlockLocal = coord2BlockLocal(vChunkPos);

		auto itr = mMapBlocks.find(vPosBlockGlobal);
		if (itr == mMapBlocks.end()) {
			if (nSdf == 0) return;
			Block block(vPosBlockGlobal);
			block.setVoxel(vPosBlockLocal, nSdf);
		}
		else {
			if (itr->second <= 127) mNOutsideCount--;
			else mNInsideCount--;

			if (nSdf == 0) {
				mMapVoxels.erase(index);
			}
			else {
				mMapVoxels[index] = nSdf;
				if (nSdf <= 127) mNOutsideCount++;
				else mNInsideCount++;
			}
		}
	}

private:
	Vector3i mVChunkPos;
	std::unordered_map<int, Block> mMapBlocks;
};

class ModifyTool {
public:
	void setVoxel(Vector3i vOriginPos, float fSdf) {
		uint8_t nSdf = fSdf2nSdf(fSdf);
		Vector3i vPosChunkGlobal = coord2ChunkGlobal(vOriginPos);
		Vector3i vPosChunkLocal = coord2ChunkLocal(vOriginPos);

		auto itr = mMapChunks.find(vPos);
		if (itr == mMapChunks.end()) {
			Chunk chunk(vPosChunkGlobal);
			chunk.setVoxel(vPosChunkLocal, nSdf);
			mMapChunks.insert(vPosChunkGlobal, chunk);
		}
		else {
			itr->second.setVoxel(vPosChunkLocal, nSdf);
		}
	}

	void reset() {
		mMapChunks.clear();
	}

private:
	std::unordered_map<Vector3i, Chunk, Vector3iHash> mMapChunks;
};
