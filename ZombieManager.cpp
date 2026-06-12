#include "ZombieManager.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Math/MathDefs.h>

ZombieManager::ZombieManager(Context* context) : Component(context)
{
}

void ZombieManager::RegisterObject(Context* context)
{
    context->RegisterFactory<ZombieManager>();
}

void ZombieManager::Initialize(Scene* scene, Fighter* fighter, int zombieCount)
{
    scene_ = scene;
    fighter_ = fighter;
    
    if (!scene || !fighter) 
    {
        URHO3D_LOGERROR("[ZOMBIE-MGR] Invalid inputs");
        return;
    }

    // Seed Urho3D random number generator so every game session is unique
    SetRandomSeed(Time::GetSystemTime());
    
    Vector3 fighterPos = fighter->GetPosition();
    URHO3D_LOGINFOF("[ZOMBIE-MGR] Spawning %d zombies from both LEFT and RIGHT of screen", zombieCount);

    for (int i = 0; i < zombieCount; ++i)
    {
        // Come from both left and right of screen
        bool spawnOnLeft = (i % 2 == 0); 
        float sideOffset = Random(4.5f, 8.0f); 
        float spawnX = fighterPos.x_ + (spawnOnLeft ? -sideOffset : sideOffset);
        
        // Stagger their Z distance so they rush in beautifully
        float spawnZ = fighterPos.z_ + 12.0f + Random(-2.0f, 3.0f);
        
        Vector3 spawnPos(spawnX, 0.0f, spawnZ);

        URHO3D_LOGINFOF("[ZOMBIE-MGR] Zombie %d spawn (%s): %s", 
            i, spawnOnLeft ? "LEFT" : "RIGHT", spawnPos.ToString().c_str());
        
        SpawnZombie(spawnPos);
    }
}

void ZombieManager::SpawnZombie(const Vector3& position)
{
    if (!scene_) return;

    ZombieEntry entry;
    entry.spawnPosition = position;

    entry.node = scene_->CreateChild("ZombiePivot");
    entry.node->SetPosition(position);

    entry.zombie = entry.node->CreateComponent<Zombie>();
    entry.zombie->Initialize(entry.node, "firing-running-didn'twork/test_out/test.gltf.d/Prefab.prefab");
    entry.zombie->SetFighter(fighter_.Get());

    entry.ai = entry.node->CreateComponent<ZombieAI>();
    entry.ai->Initialize(entry.zombie, fighter_.Get());

    zombies_.push_back(entry);
}

void ZombieManager::Update(float timeStep)
{
    UpdateModeState();

    for (auto& entry : zombies_)
    {
        if (entry.ai && !entry.ai->IsDead())
            entry.ai->Update(timeStep);
    }

    CheckFighterCollisions();
}

void ZombieManager::UpdateModeState()
{
    if (!fighter_) return;

    const bool newSideMode = (fighter_->GetMoveMode() == MoveMode::SIDE);

    if (newSideMode != isSideMode_)
    {
        isSideMode_ = newSideMode;

        for (auto& entry : zombies_)
        {
            if (entry.ai)
                entry.ai->SetActive(!isSideMode_);

            const bool isDead = entry.ai && entry.ai->IsDead();
            if (entry.zombie && isSideMode_ && !isDead)
                entry.zombie->ApplyBoundaryFreeze();
            else if (entry.zombie && !isDead)
            {
                entry.zombie->SetSideMode(false);
                entry.zombie->Reset();
            }
        }
    }
}

void ZombieManager::CheckFighterCollisions()
{
    if (!fighter_ || isSideMode_) return;
    if (!fighter_->IsMidSwing()) return;

    const Vector3 fighterPos = fighter_->GetPosition();
    const float swingRange = 1.4f;

    for (auto& entry : zombies_)
    {
        if (!entry.zombie || !entry.ai || entry.ai->IsDead()) continue;

        const Vector3 zombiePos = entry.zombie->GetPosition();
        const float distance = (fighterPos - zombiePos).Length();

        if (distance < swingRange)
            entry.ai->Kill();
    }
}

void ZombieManager::Reset()
{
    const bool sideMode = fighter_ && fighter_->GetMoveMode() == MoveMode::SIDE;

    if (fighter_ && !zombies_.empty())
    {
        Vector3 fighterPos = fighter_->GetPosition();
        int count = (int)zombies_.size();
        
        for (int i = 0; i < count; ++i)
        {
            // Re-randomize start positions on both left and right
            bool spawnOnLeft = (i % 2 == 0);
            float sideOffset = Random(4.5f, 8.0f);
            float spawnX = fighterPos.x_ + (spawnOnLeft ? -sideOffset : sideOffset);
            float spawnZ = fighterPos.z_ + 12.0f + Random(-2.0f, 3.0f);
            Vector3 spawnPos(spawnX, 0.0f, spawnZ);
            
            zombies_[i].spawnPosition = spawnPos;
            
            if (zombies_[i].zombie && zombies_[i].node)
            {
                zombies_[i].node->SetPosition(spawnPos);
                zombies_[i].zombie->SetSideMode(sideMode);
                zombies_[i].zombie->Reset();
                if (zombies_[i].zombie->GetRigidBody())
                {
                    zombies_[i].zombie->GetRigidBody()->SetPosition(spawnPos);
                    zombies_[i].zombie->GetRigidBody()->SetLinearVelocity(Vector3::ZERO);
                    zombies_[i].zombie->GetRigidBody()->SetAngularVelocity(Vector3::ZERO);
                }
            }

            if (zombies_[i].ai)
                zombies_[i].ai->Reset();
        }
    }

    URHO3D_LOGINFO("[ZOMBIE-MGR] All zombies reset to come from left and right");
}

int ZombieManager::GetAliveCount() const
{
    int count = 0;
    for (const auto& entry : zombies_)
    {
        if (entry.ai && !entry.ai->IsDead())
            ++count;
    }
    return count;
}

Vector3 ZombieManager::GetZombiePosition(unsigned index) const
{
    return index < zombies_.size() && zombies_[index].zombie ? zombies_[index].zombie->GetPosition() : Vector3::ZERO;
}

Zombie* ZombieManager::GetZombie(unsigned index) const
{
    return index < zombies_.size() ? zombies_[index].zombie.Get() : nullptr;
}

bool ZombieManager::IsZombieDead(int index) const
{
    if (index < 0 || index >= (int)zombies_.size()) return true;
    return zombies_[index].ai ? zombies_[index].ai->IsDead() : true;
}

Zombie* ZombieManager::GetFirstZombie() const
{
    return GetZombie(0);
}

void ZombieManager::LogZombiePositions() const {}
void ZombieManager::DrawDebug(Urho3D::DebugRenderer* debug) const {}