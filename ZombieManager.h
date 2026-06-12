#pragma once
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include "Zombie.h"
#include "ZombieAI.h"
#include "Fighter.h"

using namespace Urho3D;

class ZombieManager : public Component
{
    URHO3D_OBJECT(ZombieManager, Component);

public:
    ZombieManager(Context* context);
    ~ZombieManager() override = default;

    static void RegisterObject(Context* context);

    void Initialize(Scene* scene, Fighter* fighter, int zombieCount = 5);
    void Update(float timeStep);
    void Reset();
    void CheckFighterCollisions();

    void LogZombiePositions() const;
    void DrawDebug(Urho3D::DebugRenderer* debug) const;
    Vector3 GetZombiePosition(unsigned index) const;
    Zombie* GetZombie(unsigned index) const;
    bool IsZombieDead(int index) const;
    int GetAliveCount() const;
    int GetTotalCount() const { return (int)zombies_.size(); }
    
    // Get first zombie for backwards compatibility with CombatSystem
    Zombie* GetFirstZombie() const;

private:
    struct ZombieEntry
    {
        SharedPtr<Node> node;
        SharedPtr<Zombie> zombie;
        SharedPtr<ZombieAI> ai;
        Vector3 spawnPosition;
    };

    void SpawnZombie(const Vector3& position);
    void UpdateModeState();

    ea::vector<ZombieEntry> zombies_;
    WeakPtr<Scene> scene_;
    WeakPtr<Fighter> fighter_;
    
    bool isSideMode_{false};
};