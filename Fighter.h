#pragma once
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Container/Str.h>

using namespace Urho3D;

enum class MoveMode { TPS, SIDE };

// NINJA-STYLE: Controls structure for clean input management
struct FighterControls
{
    bool forward = false;
    bool back = false;
    bool left = false;
    bool right = false;
    bool attack = false;
    float yaw = 0.0f;      // Accumulated yaw from mouse
    float pitch = 0.0f;    // For future camera pitch
    
    void Reset()
    {
        forward = back = left = right = attack = false;
        // Don't reset yaw/pitch - they accumulate
    }
};

class Fighter : public LogicComponent
{
    URHO3D_OBJECT(Fighter, LogicComponent);

public:
     Fighter(Context* context);
     ~Fighter() override = default;

     static void RegisterObject(Context* context);

     void Initialize(Node* node, const ea::string& prefabPath);
     void Initialize(Node* node, const ea::string& idleAnimPath, const ea::string& attackAnimPath);
     void Reset();
     void StartAttack();
     void FixedUpdate(float timeStep) override;

     void SetBiteSynced(bool synced) { isBiteSynced_ = synced; }
     bool IsSwinging() const { return isSwinging_; }
     bool IsMidSwing() const { return isSwinging_ && (swingTimer_ >= 0.20f && swingTimer_ <= 0.60f); }
     Vector3 GetPosition() const { return node_ ? node_->GetPosition() : Vector3::ZERO; }
     Quaternion GetRotation() const { return node_ ? node_->GetRotation() : Quaternion::IDENTITY; }
     RigidBody* GetRigidBody() const { return rigidBody_; }
     void SetLungeTarget(const Vector3& target) { lungeTarget_ = target; isLunging_ = true; }
     void ClearLungeTarget() { lungeTarget_ = Vector3::ZERO; isLunging_ = false; }
     void SetBiteTexture(bool enable);
     void SetRedTint(bool enable);
     void CacheOriginalMaterials();
     bool IsBiteTextureActive() const { return biteTextureActive_; }
     bool IsRedTintActive() const { return redTintActive_; }

     void SetMoveMode(MoveMode mode);
     MoveMode GetMoveMode() const { return currentMoveMode_; }

     float SetTpsYaw(float yaw) { 
        // Clamp rotation to -180° to +180°
        if (yaw > TPS_YAW_MAX_RIGHT) yaw = TPS_YAW_MAX_RIGHT;
        if (yaw < TPS_YAW_MAX_LEFT) yaw = TPS_YAW_MAX_LEFT;
        tpsYaw_ = yaw; 
        return yaw;
    }
     float GetTpsYaw() const { return tpsYaw_; }

     // NINJA-STYLE: Ground state accessors
     bool IsOnGround() const { return onGround_; }
     float GetInAirTime() const { return inAirTime_; }

     // Movement speeds
     static constexpr float TPS_MAX_SPEED = 5.0f;        // Units per second
     static constexpr float SIDE_MAX_SPEED = 2.5f;      // Units per second
     static constexpr float TPS_MOUSE_SENSITIVITY = 0.5f; // Degrees per pixel (halved from 1.0)
     static constexpr float TPS_YAW_MAX_RIGHT = 100.5f;  // Permanent right limit from log
     static constexpr float TPS_YAW_MAX_LEFT = 27.0f;    // Permanent left limit from log
     
     // NINJA-STYLE: Force constants
     static constexpr float TPS_MOVE_FORCE = 400.0f;     // Ground movement force
     static constexpr float TPS_AIR_FORCE = 100.0f;      // Air control force
     static constexpr float SIDE_MOVE_FORCE = 300.0f;    // Side mode force
     static constexpr float IN_AIR_THRESHOLD = 0.3f;     // Grace period (seconds)

     // Public controls for main.cpp to set
     FighterControls controls_;

private:
     void ReparentWeapon(Node* meshNode);
     void SetupPhysics();
     void ApplyMaterialOverrides();
     
     // NINJA-STYLE: Collision event handler
     void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

     SharedPtr<Node> node_;
     SharedPtr<Node> meshNode_;
     SharedPtr<AnimationController> animController_;
     SharedPtr<RigidBody> rigidBody_;
     SharedPtr<CollisionShape> collisionShape_;

     ea::string idleAnimPath_;
     ea::string attackAnimPath_;

     bool isSwinging_{false};
     bool isBiteSynced_{false};
     float swingTimer_{0.0f};
     Vector3 arenaPosition_{Vector3::ZERO};
     Vector3 lungeTarget_{Vector3::ZERO};
     bool isLunging_{false};
     float lungeDistCovered_ = 0.0f;

     struct MaterialSlot {
         WeakPtr<Node> node;
         WeakPtr<StaticModel> drawable;
         unsigned geoIndex = 0;
         SharedPtr<Material> originalMaterial;
         SharedPtr<Material> activeMaterial;
         bool isBodyDiffuse_ = false;
         ea::string diffuseUnitName_;
         SharedPtr<Texture> originalDiffuse_;
     };
     ea::vector<MaterialSlot> materialSlots_;
     SharedPtr<Texture> biteTexture_;
     bool biteTextureActive_ = false;
     bool redTintActive_ = false;

     MoveMode currentMoveMode_ = MoveMode::TPS;
     float tpsYaw_ = 0.0f;
     float lastTpsYaw_ = 0.0f; // Preserve yaw across TPS<->SIDE switches
     
     // NINJA-STYLE: Ground detection state
     bool onGround_{false};
     float inAirTime_{0.0f};
};
