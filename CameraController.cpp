#include "CameraController.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/IO/Log.h> 

CameraController::CameraController(Context* context) : LogicComponent(context)
{
    // BOTH masks so Update runs as fallback, PostUpdate runs after physics
    SetUpdateEventMask(USE_UPDATE | USE_POSTUPDATE);
    transitionDuration_ = 0.15f;
}

void CameraController::RegisterObject(Context* context)
{
    context->RegisterFactory<CameraController>();
}

void CameraController::Initialize(Node* cameraNode, Fighter* fighter)
{
    cameraNode_ = cameraNode;
    fighter_ = fighter;
    camera_ = cameraNode->GetComponent<Camera>();
    input_ = GetSubsystem<Input>();

    if (camera_)
    {
        camera_->SetFov(tpsFov_);
        camera_->SetOrthographic(false);   // <-- LOCK: never use ortho
    }
    
    if (fighter_ && fighter_->GetNode())
    {
        tpsOrbitOffset_ = 0.0f;
        Vector3 fighterPos = fighter_->GetPosition();
        float fighterYaw = fighter_->GetTpsYaw();
        float rad = fighterYaw * M_DEGTORAD;
        Vector3 idealPos = fighterPos + Vector3(-Sin(rad) * tpsDistance_, tpsHeight_, -Cos(rad) * tpsDistance_);
        cameraNode_->SetPosition(idealPos);
        cameraNode_->LookAt(fighterPos + Vector3::UP * 1.0f);
    }
    
    URHO3D_LOGINFOF("[CAM-CTRL] Init: FOV=%.1f dist=%.1f ortho=OFF", tpsFov_, tpsDistance_);
}

void CameraController::ToggleMode()
{
    if (currentMode_ == CameraMode::TRANSITIONING) 
    {
        URHO3D_LOGINFO("[CAM-CTRL] Toggle ignored: mid-transition.");
        return;
    }

    targetMode_ = (currentMode_ == CameraMode::TPS) ? CameraMode::SIDE : CameraMode::TPS;
    currentMode_ = CameraMode::TRANSITIONING;
    transitionTimer_ = 0.0f;
    startPos_ = cameraNode_->GetPosition();
    
    // Reset TPS offset when entering TPS mode - fixes camera starting at wrong angle
    if (targetMode_ == CameraMode::TPS)
    {
        ResetTpsOffset();
    }
    
    URHO3D_LOGINFOF("[CAM-CTRL] >>> ToggleMode START -> %s", 
        targetMode_ == CameraMode::SIDE ? "SIDE" : "TPS");
}

// Fallback - delegates to PostUpdate logic
void CameraController::Update(float timeStep)
{
    // Empty: all work done in PostUpdate so camera follows physics-updated fighter
}

void CameraController::PostUpdate(float timeStep)
{
    if (!cameraNode_ || !camera_ || !fighter_ || !fighter_->GetNode())
    {
        return;
    }

    if (currentMode_ == CameraMode::TRANSITIONING)
        UpdateTransition(timeStep);
    else if (currentMode_ == CameraMode::TPS)
        UpdateTPS(timeStep);
    else
        UpdateSide(timeStep);
}

void CameraController::UpdateTPS(float timeStep)
{
    if (!fighter_ || !fighter_->GetNode()) return;
    
    // Sync yaw from fighter every frame (prevents drift)
    fighterYaw_ = fighter_->GetTpsYaw();
    
    // Optional Q/E orbit
    if (input_->GetKeyDown(KEY_Q)) tpsOrbitOffset_ += tpsOrbitSpeed_ * timeStep;
    if (input_->GetKeyDown(KEY_E)) tpsOrbitOffset_ -= tpsOrbitSpeed_ * timeStep;

    float wheel = input_->GetMouseMoveWheel();
    if (wheel != 0.0f) {
        tpsDistance_ -= wheel * 0.5f;
        tpsDistance_ = Clamp(tpsDistance_, 1.5f, 10.0f);
    }

    Vector3 fighterPos = fighter_->GetPosition();
    float fighterYaw = fighterYaw_; // Use cached value
    float camYaw = fighterYaw + tpsOrbitOffset_;
    float rad = camYaw * M_DEGTORAD;

    Vector3 idealPos = fighterPos + Vector3(-Sin(rad) * tpsDistance_, tpsHeight_, -Cos(rad) * tpsDistance_);
    cameraNode_->SetPosition(idealPos);
    cameraNode_->LookAt(fighterPos + Vector3::UP * 1.0f);
    
    static float logTimer = 0;
    logTimer += timeStep;
    if (logTimer > 1.0f)
    {
        URHO3D_LOGINFOF("[CAM-CTRL] TPS: fighterYaw=%.1f orbit=%.1f cam=%s",
            fighterYaw, tpsOrbitOffset_, idealPos.ToString().c_str());
        logTimer = 0;
    }
}

void CameraController::UpdateSide(float timeStep)
{
    if (!fighter_ || !fighter_->GetNode()) return;
    
    Vector3 fighterPos = fighter_->GetPosition();
    // True side view: camera sits to the right of the fighter, looking left
    Vector3 idealPos = Vector3(fighterPos.x_ + sideDistance_, tpsHeight_, fighterPos.z_);
    cameraNode_->SetPosition(idealPos);
    cameraNode_->LookAt(Vector3(fighterPos.x_, tpsHeight_, fighterPos.z_));
    
    static float logTimer = 0;
    logTimer += timeStep;
    if (logTimer > 1.0f)
    {
        URHO3D_LOGINFOF("[CAM-CTRL] SIDE: cam=%s fighter=%s",
            idealPos.ToString().c_str(), fighterPos.ToString().c_str());
        logTimer = 0;
    }
}

void CameraController::UpdateTransition(float timeStep)
{
    transitionTimer_ += timeStep;
    float t = transitionTimer_ / transitionDuration_;
    
    // HARD SNAP at end - no smoothstep to avoid freeze
    if (t >= 1.0f)
    {
        t = 1.0f;
        currentMode_ = targetMode_;
        
        if (fighter_) 
        {
            fighter_->SetMoveMode(targetMode_ == CameraMode::SIDE ? MoveMode::SIDE : MoveMode::TPS);
            // Sync fighterYaw_ from fighter when switching TO TPS
            if (targetMode_ == CameraMode::TPS)
            {
                fighterYaw_ = fighter_->GetTpsYaw();
                // Reset orbit offset on TPS entry to prevent camera offset issues
                ResetTpsOffset();
            }
            URHO3D_LOGINFOF("[CAM-CTRL] >>> Toggle DONE -> %s",
                targetMode_ == CameraMode::SIDE ? "SIDE" : "TPS");
        }
    }

    Vector3 fighterPos = fighter_->GetPosition();
    
    // Recalculate target live every frame
    Vector3 targetPos;
    if (targetMode_ == CameraMode::SIDE)
    {
        targetPos = Vector3(fighterPos.x_ + sideDistance_, tpsHeight_, fighterPos.z_);
    }
    else
    {
        float fighterYaw = fighter_->GetTpsYaw();
        float rad = (fighterYaw + tpsOrbitOffset_) * M_DEGTORAD;
        targetPos = fighterPos + Vector3(-Sin(rad) * tpsDistance_, tpsHeight_, -Cos(rad) * tpsDistance_);
    }

    // Linear lerp only - no smoothstep
    Vector3 currentPos = startPos_.Lerp(targetPos, t);
    cameraNode_->SetPosition(currentPos);
    cameraNode_->LookAt(fighterPos + Vector3::UP * 1.0f);

    // Log every frame during transition
    URHO3D_LOGINFOF("[CAM-CTRL] Transition: t=%.2f timer=%.3f dur=%.2f", t, transitionTimer_, transitionDuration_);
}