#pragma once
#include "Zombie.h"
#include "Fighter.h"

using namespace Urho3D;

enum class ZombieAIState
{
    IDLE,
    PURSUING,
    ATTACKING,
    DEAD
};

class ZombieAI : public Component
{
    URHO3D_OBJECT(ZombieAI, Component);

public:
    ZombieAI(Context* context);
    ~ZombieAI() override = default;

    static void RegisterObject(Context* context);

    void Initialize(Zombie* zombie, Fighter* target);
    void Update(float timeStep);
    void Kill();
    void Reset();

    bool IsDead() const { return currentState_ == ZombieAIState::DEAD; }
    bool IsActive() const { return isActive_; }
    void SetActive(bool active) { isActive_ = active; }

private:
    void UpdatePursuing(float timeStep);
    void UpdateAttacking(float timeStep);
    void TransitionTo(ZombieAIState newState);
    void ResetRandomization();

    WeakPtr<Zombie> zombie_;
    WeakPtr<Fighter> target_;
    
    ZombieAIState currentState_{ZombieAIState::IDLE};
    bool isActive_{true};  
    
    static constexpr float PURSUE_RANGE = 50.0f;
    static constexpr float ATTACK_RANGE = 15.0f; // Range to periodically trigger snowball throws
    static constexpr float ROTATION_SPEED = 240.0f;  

    // Dynamic randomization parameters for continuous non-stopping motion
    float weaveSpeed_{2.0f};
    float weaveWidth_{3.0f};
    float timePhase_{0.0f};
    float targetZOffset_{5.0f};
    float reorderTimer_{0.0f};
    float randomXGoal_{0.0f};
    float randomZGoal_{5.0f};
};