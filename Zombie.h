#pragma once
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/AnimatedModel.h>

using namespace Urho3D;

class Fighter;

enum ZombieCollisionLayer {
    Z_LAYER_WORLD = 1,
    Z_LAYER_FIGHTER = 2,
    Z_LAYER_ZOMBIE = 4
};

// Controls structure for Ninja Snow War style AI Controller
struct ZombieControls
{
    bool forward = false;
    bool attack = false;
    float yaw = 0.0f;
    void Reset() { forward = attack = false; }
};

class Zombie : public LogicComponent
{
    URHO3D_OBJECT(Zombie, LogicComponent);

public:
     Zombie(Context* context);
     ~Zombie() override = default;

     static void RegisterObject(Context* context);

     void Initialize(Node* node, const ea::string& prefabPath);
     void Reset();
     void FixedUpdate(float timeStep) override;

     void TriggerNeckBite();
     void StartNeckBite(Node* targetNode);
     void ApplyBoundaryFreeze();
     void ClearBoundaryFreeze();
     void CheckCollision(Fighter* fighter);
     void HandleHit(Fighter* fighter);

     // Added for Urho3D Ninja sample snowball attack
     void PerformAttack(); 
     void HandleProjectileCollision(StringHash eventType, VariantMap& eventData); 

     void SetBiteSynced(bool synced) { isBiteSynced_ = synced; }
     void SetFighter(Fighter* fighter) { fighter_ = fighter; }
     void SetMovementDirection(const Vector3& direction);
     bool IsFrozen() const { return isFrozen_; }
     bool IsContactFrozen() const { return isContactFrozen_; }
     bool IsNeckBiting() const { return isNeckBiting_; }
     Vector3 GetPosition() const { return node_ ? node_->GetPosition() : Vector3::ZERO; }
     RigidBody* GetRigidBody() const { return rigidBody_; }

     bool IsOnGround() const { return onGround_; }

     void SetSideMode(bool side) { isSideMode_ = side; }

     // Public controls for ZombieAI to drive (Exactly like Fighter / AIController)
     ZombieControls controls_;

private:
     void SetupPhysics();
     void UpdateMovement(float timeStep);
     void UpdateAnimation();
     
     void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

     SharedPtr<Node> node_;
     SharedPtr<Node> meshNode_;
     SharedPtr<Node> modelNode_;  
     SharedPtr<AnimationController> animController_;
     SharedPtr<RigidBody> rigidBody_;
     SharedPtr<CollisionShape> collisionShape_;

     ea::string runAnimPath_;
     ea::string attackAnimPath_; // Added for attack animation

     Vector3 startPosition_{0.59f, 0.0f, 3.2f};  
     Vector3 sideStartPosition_{2.0f, 0.0f, 0.0f};  

     bool isFrozen_{false};
     bool isContactFrozen_{false};
     bool isNeckBiting_{false};
     bool isBiteSynced_{false};
     bool isSideMode_{false};

     WeakPtr<Fighter> fighter_;  

     float health_{100.0f};
     const float maxHealth_{100.0f};

     static constexpr float WALK_FORCE = 2000.0f;   
     static constexpr float AIR_FORCE  = 800.0f;
     static constexpr float WALK_SPEED = 3.5f;
     static constexpr float STOP_THRESHOLD = 0.94f;   

     bool hasBeenHit_ = false;

     bool onGround_{false};
     bool wasOnGround_{false};
     bool isSliding_{false};
     float inAirTime_{0.0f};
     static constexpr float IN_AIR_THRESHOLD = 0.3f;  

     Vector3 forcedMovementDirection_{Vector3::ZERO};
     bool hasForcedMovementDirection_{false};

     // Ninja Snow War attack cooldown & parameters
     float attackCooldown_{0.0f};
     static constexpr float ZOMBIE_ATTACK_DELAY = 1.5f;
};