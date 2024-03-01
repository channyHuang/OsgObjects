#ifndef PHYSICSMANAGER_H
#define PHYSICSMANAGER_H

//#define PHYSICS_ON

#ifdef PHYSICS_ON

#include <vector>
#include <unordered_map>

#include "PxPhysicsAPI.h"
#include "PxPhysics.h"

#include "commonFunc/SignalSlots.h"
#include "commonMath/Vector3i.h"
#include "terrains/commonStruct.h"

class PhysicsManager {
public:
	static PhysicsManager* getInstance() {
		if (instance == nullptr) {
			instance = new PhysicsManager();
		}
		return instance;
	}

	~PhysicsManager();

	void initPhysics();
	void stepPhysics(bool active);

	physx::PxRigidDynamic* createDynamic(const std::string& id,
										 const physx::PxTransform& t = physx::PxTransform(physx::PxVec3(0)), 
										 const physx::PxGeometry& geometry = physx::PxSphereGeometry(.5f), 
										 const physx::PxVec3& velocity = physx::PxVec3(0));

	void removeDynamic(const std::string& id);

	bool addTerrain(const Vector3i& bpos, Arrays surface);

// signal
	SignalSlot::Signal<void(const std::string&, const physx::PxVec3&)> updateActor;

private:
	PhysicsManager();

private:
	physx::PxMaterial* gMaterial;
	physx::PxFoundation* gFoundation;
	physx::PxPhysics* gPhysics;
	physx::PxScene* gScene;

	static PhysicsManager* instance;

	physx::PxDefaultAllocator gAllocator;
	physx::PxDefaultErrorCallback	gErrorCallback;
	physx::PxDefaultCpuDispatcher* gDispatcher;

	std::unordered_map<std::string, std::pair<physx::PxRigidDynamic*, physx::PxTransform>> mapActors;
	physx::PxRigidStatic* terrainActor = nullptr;
	physx::PxCooking* gCooking;
};

#endif

#endif