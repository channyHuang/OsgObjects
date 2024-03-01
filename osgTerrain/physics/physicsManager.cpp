#include "physicsManager.h"

#include <iostream>

#ifdef PHYSICS_ON

PhysicsManager* PhysicsManager::instance = nullptr;

PhysicsManager::PhysicsManager() {
    initPhysics();
}

PhysicsManager::~PhysicsManager() {}

void PhysicsManager::initPhysics() {
    
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), true);

    physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

    gScene = gPhysics->createScene(sceneDesc);

    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    physx::PxCookingParams params(gPhysics->getTolerancesScale());
    params.meshWeldTolerance = 0.001f;
    params.meshPreprocessParams = physx::PxMeshPreprocessingFlags(physx::PxMeshPreprocessingFlag::eWELD_VERTICES);
    params.buildGPUData = true; //Enable GRB data being produced in cooking.
    gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, params);
}

physx::PxRigidDynamic* PhysicsManager::createDynamic(const std::string& id, const physx::PxTransform& t, const physx::PxGeometry& geometry, const physx::PxVec3& velocity)
{
    if (mapActors.size() <= 0) {
        //physx::PxRigidStatic* floor = physx::PxCreateStatic(*gPhysics, physx::PxTransform(physx::PxVec3(0, 50, 0)), physx::PxBoxGeometry(100, 4, 100), *gMaterial);
        //gScene->addActor(*floor);
    }
    physx::PxRigidDynamic* dynamic = physx::PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
    dynamic->setAngularDamping(0.5f);
    dynamic->setLinearVelocity(velocity);
    gScene->addActor(*dynamic);

    mapActors[id] = std::make_pair(dynamic, t);

    return dynamic;
}

bool PhysicsManager::addTerrain(const Vector3i& bpos, Arrays surface) {
    physx::PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = surface.positions.size();
    meshDesc.triangles.count = surface.indices.size() / 3;
    meshDesc.points.stride = sizeof(Vector3);
    meshDesc.triangles.stride = sizeof(uint32_t) * 3;
    meshDesc.points.data = &(surface.positions)[0];
    meshDesc.triangles.data = &(surface.indices)[0];
    meshDesc.flags = physx::PxMeshFlags(0);

    physx::PxDefaultMemoryOutputStream stream;
    bool ok = gCooking->cookTriangleMesh(meshDesc, stream);
    if (!ok)
        return false;

    physx::PxDefaultMemoryInputData rb(stream.getData(), stream.getSize());

    physx::PxTriangleMesh* triangleMesh = gPhysics->createTriangleMesh(rb);
    if (!triangleMesh)
        return false;

    physx::PxTriangleMeshGeometry meshGeom(triangleMesh);

    physx::PxShape* shape = gPhysics->createShape(meshGeom, *gMaterial);
    if (terrainActor == nullptr) {
        terrainActor = physx::PxCreateStatic(*gPhysics, physx::PxTransform(physx::PxIdentity), *shape);
        gScene->addActor(*terrainActor);
    }
    else {
        terrainActor->attachShape(*shape);
    }
    return true;
}

void PhysicsManager::removeDynamic(const std::string& id) {
    auto itr = mapActors.find(id);
    if (itr != mapActors.end()) {
        gScene->removeActor(*(itr->second.first));
        mapActors.erase(id);
    }
}

void PhysicsManager::stepPhysics(bool active)
{
    if (!active) return;
    
    gScene->simulate(1.f / 20.f);
    gScene->fetchResults(true);

    //physx::PxActorTypeFlags types = physx::PxActorTypeFlag::eRIGID_DYNAMIC;
    //physx::PxU32 nbActors = gScene->getNbActors(types);
    //if (nbActors)
    //{
    //    std::vector<physx::PxRigidDynamic*> actors(nbActors);
    //    gScene->getActors(types, reinterpret_cast<physx::PxActor**>(&actors[0]), nbActors);
    //    for (physx::PxU32 i = 0; i < nbActors; ++i)
    //    {
    //        physx::PxTransform t = actors[i]->getGlobalPose();
    //        updateActor(id, t.p);
    //    }
    //}
    for (auto itr = mapActors.begin(); itr != mapActors.end(); itr++) {
        physx::PxTransform t = itr->second.first->getGlobalPose();

        physx::PxVec3 rayOrig = itr->second.second.p;
        physx::PxVec3 rayDir = t.p - rayOrig;
        physx::PxRaycastHit hit;
        physx::PxRaycastBuffer hitb;
        gScene->raycast(rayOrig, rayDir.getNormalized(), rayDir.magnitude(), hitb, physx::PxHitFlag::ePOSITION);
        hit = hitb.block;
        if (hit.shape) {
            t.p = hit.position;

            itr->second.first->setGlobalPose(t);
        }
        
        itr->second.second = t;
        
        updateActor(itr->first, t.p);
    }
}

#endif
