#pragma once
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Scene/Node.h>
#include "Fighter.h"

using namespace Urho3D;

enum class CameraMode { TPS, SIDE, TRANSITIONING };

class CameraController : public LogicComponent
{
    URHO3D_OBJECT(CameraController, LogicComponent);

public:
    CameraController(Context* context);
    ~CameraController() override = default;

    static void RegisterObject(Context* context);

    void Initialize(Node* cameraNode, Fighter* fighter);
    void Update(float timeStep) override;      // Fallback
    void PostUpdate(float timeStep) override;   // <-- PRIMARY: runs AFTER physics
    void ToggleMode();
    
    // Reset orbit offset when entering TPS mode - ensures camera starts correctly
    void ResetTpsOffset() { tpsOrbitOffset_ = 0.0f; }

    CameraMode GetCurrentMode() const { return currentMode_; }

private:
    void UpdateTPS(float timeStep);
    void UpdateSide(float timeStep);
    void UpdateTransition(float timeStep);

    WeakPtr<Node> cameraNode_;
    WeakPtr<Fighter> fighter_;
    Camera* camera_ = nullptr;
    Input* input_ = nullptr;

    CameraMode currentMode_ = CameraMode::TPS;

    // Transition state
    CameraMode targetMode_ = CameraMode::TPS;
    float transitionTimer_ = 0.0f;
    float transitionDuration_ = 0.15f;     // <-- FASTER: 0.15s snap
    
    Vector3 startPos_;
    Vector3 targetPos_;

    // TPS Settings
    float tpsFov_ = 60.0f;
    float tpsDistance_ = 3.5f;
    float tpsHeight_ = 1.5f;
    float tpsOrbitSpeed_ = 90.0f;
    float tpsOrbitOffset_ = 0.0f;          // offset FROM fighter's back
    float fighterYaw_ = 0.0f;              // cached yaw from fighter, synced every frame

    // Side Settings  (PERSPECTIVE — no ortho!)
    float sideDistance_ = 5.0f;            // X offset, not Z
};