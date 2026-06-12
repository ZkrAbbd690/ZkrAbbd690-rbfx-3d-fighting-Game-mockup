#include "ZombieAI.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Math/MathDefs.h>

ZombieAI::ZombieAI(Context* context) : Component(context)
{
}

void ZombieAI::RegisterObject(Context* context)
{
    context->RegisterFactory<ZombieAI>();
}

void ZombieAI::Initialize(Zombie* zombie, Fighter* target)
{
    zombie_ = zombie;
    target_ = target;
    currentState_ = ZombieAIState::IDLE;
    isActive_ = true;
    
    ResetRandomization();
    URHO3D_LOGINFO("[ZOMBIE-AI] Initialized Non-Stopping Randomized Middle-Screen AI");
}

void ZombieAI::ResetRandomization()
{
    // Every game session gets unique movement & reordering parameters
    weaveSpeed_ = Random(1.5f, 4.5f);
    weaveWidth_ = Random(2.0f, 5.0f);
    timePhase_ = Random(0.0f, 360.0f);
    reorderTimer_ = Random(0.0f, 2.0f);
    randomXGoal_ = Random(-3.5f, 3.5f);
    randomZGoal_ = Random(3.0f, 8.0f);
}

void ZombieAI::Reset()
{
    currentState_ = ZombieAIState::IDLE;
    isActive_ = true;
    ResetRandomization();
    if (zombie_) zombie_->controls_.Reset();
}

void ZombieAI::Update(float timeStep)
{
    if (!zombie_ || !target_) return;
    
    zombie_->controls_.Reset();
    
    if (!isActive_)
    {
        if (currentState_ != ZombieAIState::IDLE)
            TransitionTo(ZombieAIState::IDLE);
        return;
    }
    
    if (currentState_ == ZombieAIState::DEAD) return;
    
    if (target_->GetMoveMode() == MoveMode::SIDE)
    {
        isActive_ = false;
        return;
    }
    
    // --- 1. PERIODIC RANDOMIZED REORDERING ---
    reorderTimer_ -= timeStep;
    if (reorderTimer_ <= 0.0f)
    {
        // Pick new random goals in the middle corridor to constantly swap and reorder
        randomXGoal_ = Random(-3.8f, 3.8f);
        randomZGoal_ = Random(2.5f, 8.5f);
        weaveSpeed_ = Random(2.0f, 5.0f);
        reorderTimer_ = Random(1.5f, 4.0f);
    }

    Vector3 zombiePos = zombie_->GetPosition();
    Vector3 fighterPos = target_->GetPosition();
    
    // --- 2. CALCULATE DYNAMIC MIDDLE-SCREEN GOAL ---
    // Smooth sinusoidal weaving blended with the live randomized goal
    float systemTimeSec = Time::GetSystemTime() * 0.001f;
    float dynamicWeaveX = Sin((systemTimeSec * weaveSpeed_ + timePhase_) * 50.0f) * weaveWidth_;
    
    // Final goal in the middle of the screen in front of Fighter
    float goalX = fighterPos.x_ + (randomXGoal_ * 0.5f + dynamicWeaveX * 0.5f);
    float goalZ = fighterPos.z_ + randomZGoal_;
    
    Vector3 liveGoal(goalX, zombiePos.y_, goalZ);
    Vector3 toGoal = liveGoal - zombiePos;
    toGoal.y_ = 0.0f;
    
    // Continually steer towards liveGoal
    if (toGoal.Length() > 0.05f)
    {
        float targetYaw = Atan2(toGoal.x_, toGoal.z_) * M_RADTODEG;
        zombie_->controls_.yaw = targetYaw;
    }
    
    // --- 3. CRITICAL REQUIREMENT: THEY SHOULD NOT STOP ---
    // Always keep moving forward! Never set controls_.forward = false.
    zombie_->controls_.forward = true;

    // Periodically throw snowballs while running
    Vector3 toFighter = fighterPos - zombiePos;
    toFighter.y_ = 0.0f;
    if (toFighter.Length() < ATTACK_RANGE)
    {
        zombie_->controls_.attack = true; 
    }
}

void ZombieAI::TransitionTo(ZombieAIState newState)
{
    if (currentState_ == newState) return;
    currentState_ = newState;

    if (newState == ZombieAIState::DEAD)
    {
        if (zombie_)
        {
            zombie_->controls_.Reset();
            if (target_) zombie_->HandleHit(target_.Get());
        }
    }
}

void ZombieAI::Kill()
{
    TransitionTo(ZombieAIState::DEAD);
}

void ZombieAI::UpdatePursuing(float timeStep) {}
void ZombieAI::UpdateAttacking(float timeStep) {}