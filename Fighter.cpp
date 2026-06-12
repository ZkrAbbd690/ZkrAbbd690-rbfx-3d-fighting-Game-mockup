#include "Fighter.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/PrefabResource.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/GraphicsDefs.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Math/MathDefs.h>  // For M_DEGTORAD
#include <cstring>

enum CollisionLayer {
    LAYER_WORLD = 1,
    LAYER_FIGHTER = 2,
    LAYER_ZOMBIE = 4
};

static int g_fighterLogFrame = 0;
static float g_fighterLogTimer = 0.0f;

Fighter::Fighter(Context* context) : LogicComponent(context)
{
    SetUpdateEventMask(USE_FIXEDUPDATE);
    URHO3D_LOGINFO("[FIGHTER] Constructor - Ninja-style physics enabled");
}

void Fighter::RegisterObject(Context* context)
{
    context->RegisterFactory<Fighter>();
}

void Fighter::Initialize(Node* node, const ea::string& prefabPath)
{
    node_ = node;
    idleAnimPath_.clear();
    attackAnimPath_ = "test.glb.d/Animations/Armature_Armature_mixamo.com_Layer0.ani";

    URHO3D_LOGINFOF("[FIGHTER] Initialize() node=%s", node ? node->GetName().c_str() : "NULL");

    if (node_)
    {
        arenaPosition_ = node_->GetPosition();
        auto* cache = GetSubsystem<ResourceCache>();
        if (cache && !prefabPath.empty())
        {
            auto* prefabFile = cache->GetResource<PrefabResource>(prefabPath);
            if (prefabFile)
            {
                meshNode_ = node_->CreateChild("AxeManMesh");
                prefabFile->InstantiateReference(meshNode_);
                meshNode_->SetRotation(Quaternion(0.0f, -90.0f, 0.0f));
                meshNode_->SetScale(5.0f / 9.0f);
                Vector3 pos = meshNode_->GetPosition();
                pos.y_ = 0.05f;
                meshNode_->SetPosition(pos);
                CacheOriginalMaterials();
                animController_ = meshNode_->FindComponent<AnimationController>();
                if (animController_) animController_->StopAll();
            }
        }
        if (meshNode_) ReparentWeapon(meshNode_);
        SetupPhysics();
    }
    Reset();
    URHO3D_LOGINFOF("[FIGHTER] Init complete - mode=%d yaw=%.1f", (int)currentMoveMode_, tpsYaw_);
}

void Fighter::SetupPhysics()
{
    if (!node_) return;
    rigidBody_ = node_->GetOrCreateComponent<RigidBody>();
    if (rigidBody_)
    {
        rigidBody_->SetMass(80.0f);
        rigidBody_->SetUseGravity(true);
        rigidBody_->SetLinearDamping(0.5f);      // NINJA-STYLE: Slight damping for control
        rigidBody_->SetAngularDamping(10.0f);
        rigidBody_->SetKinematic(false);
        rigidBody_->SetCollisionLayer(LAYER_FIGHTER);
        rigidBody_->SetCollisionMask(LAYER_WORLD | LAYER_ZOMBIE);
        rigidBody_->SetAngularFactor(Vector3::ZERO);
        
        // *** NINJA-STYLE CRITICAL: Enable collision events even when resting ***
        rigidBody_->SetCollisionEventMode(COLLISION_ALWAYS);
        
        rigidBody_->SetLinearVelocity(Vector3::ZERO);
        URHO3D_LOGINFO("[FIGHTER] RigidBody configured with COLLISION_ALWAYS (Ninja-style)");
    }
    collisionShape_ = node_->GetOrCreateComponent<CollisionShape>();
    if (collisionShape_) collisionShape_->SetCapsule(0.45f, 1.6f, Vector3(0.0f, 0.8f, 0.0f));
    
    // *** NINJA-STYLE: Subscribe to collision events for ground detection ***
    SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(Fighter, HandleNodeCollision));
}

// NINJA-STYLE: Ground detection via collision events (not raycasting)
void Fighter::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    using namespace NodeCollision;
    
    // Get collision contact data
    MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
    
    while (!contacts.IsEof())
    {
        Vector3 contactPosition = contacts.ReadVector3();
        Vector3 contactNormal = contacts.ReadVector3();
        /*float contactDistance = */contacts.ReadFloat();
        /*float contactImpulse = */contacts.ReadFloat();
        
        // NINJA-STYLE: Ground detection - contact below center with mostly vertical normal
        // contactNormal.y_ > 0.75 means slope angle < 41 degrees
        if (contactPosition.y_ < (node_->GetPosition().y_ + 0.5f))
        {
            if (contactNormal.y_ > 0.75f)  // Vertical threshold
            {
                onGround_ = true;
            }
        }
    }
}

void Fighter::CacheOriginalMaterials()
{
    materialSlots_.clear();
    if (!meshNode_) return;
    static const ea::vector<ea::string> diffuseUnitNames = {"DiffMap", "Albedo", "AlbedoMap", "BaseColor", "BaseColorMap", "Diffuse"};
    ea::vector<Node*> allNodes;
    ea::vector<Node*> stack = { meshNode_ };
    while (!stack.empty()) {
        Node* current = stack.back(); stack.pop_back(); allNodes.push_back(current);
        for (Node* child : current->GetChildren()) stack.push_back(child);
    }
    for (Node* n : allNodes) {
        const auto& components = n->GetComponents();
        for (unsigned c = 0; c < components.size(); ++c) {
            StaticModel* sm = dynamic_cast<StaticModel*>(components[c].Get());
            if (!sm) continue;
            for (unsigned g = 0; g < sm->GetNumGeometries(); ++g) {
                Material* mat = sm->GetMaterial(g);
                if (!mat) continue;
                MaterialSlot slot;
                slot.node = n; slot.drawable = sm; slot.geoIndex = g; slot.originalMaterial = mat;
                for (const auto& unitName : diffuseUnitNames) {
                    Texture* tex = mat->GetTexture(unitName);
                    if (!tex) continue;
                    ea::string texName = tex->GetName();
                    if ((texName.find("Body") != ea::string::npos || texName.find("body") != ea::string::npos) &&
                        (texName.find("diffuse") != ea::string::npos || texName.find("Diffuse") != ea::string::npos) &&
                        texName.find("normal") == ea::string::npos && texName.find("Normal") == ea::string::npos) {
                        slot.isBodyDiffuse_ = true; slot.diffuseUnitName_ = unitName; slot.originalDiffuse_ = tex; break;
                    }
                }
                materialSlots_.push_back(slot);
            }
        }
    }
    int bodyCount = 0;
    for (const auto& s : materialSlots_) if (s.isBodyDiffuse_) ++bodyCount;
    URHO3D_LOGINFOF("[BITE-TEX] Slots: %zu total, %d body-diffuse", materialSlots_.size(), bodyCount);
}

void Fighter::SetBiteTexture(bool enable)
{
    if (biteTextureActive_ == enable) return;
    biteTextureActive_ = enable;
    auto* cache = GetSubsystem<ResourceCache>();
    if (enable && !biteTexture_) biteTexture_ = cache->GetResource<Texture2D>("test.glb.d/Textures/MaleBruteA_Body_diffuse1_bite1.jpg");
    for (auto& slot : materialSlots_) {
        if (!slot.drawable) continue;
        bool needsBite = biteTextureActive_ && slot.isBodyDiffuse_ && biteTexture_;
        bool needsRed = redTintActive_;
        if (!needsBite && !needsRed) {
            slot.drawable->SetMaterial(slot.geoIndex, slot.originalMaterial);
            slot.activeMaterial.Reset();
        } else {
            SharedPtr<Material> cloned = slot.originalMaterial->Clone();
            if (needsBite) {
                if (!slot.diffuseUnitName_.empty()) cloned->SetTexture(slot.diffuseUnitName_, biteTexture_);
                cloned->SetTexture("DiffMap", biteTexture_);
            }
            if (needsRed) cloned->SetShaderParameter("MatDiffColor", Color(1.0f, 0.0f, 0.0f, 1.0f));
            slot.drawable->SetMaterial(slot.geoIndex, cloned);
            slot.activeMaterial = cloned;
        }
    }
}

void Fighter::SetRedTint(bool enable) { if (redTintActive_ != enable) { redTintActive_ = enable; SetBiteTexture(biteTextureActive_); } }

void Fighter::SetMoveMode(MoveMode mode)
{
    if (currentMoveMode_ == mode) return;
    currentMoveMode_ = mode;
    if (rigidBody_) { rigidBody_->SetLinearVelocity(Vector3::ZERO); rigidBody_->SetAngularVelocity(Vector3::ZERO); }
    if (mode == MoveMode::SIDE) {
        lastTpsYaw_ = tpsYaw_;
        tpsYaw_ = 0.0f; // Face +Z in SIDE mode (not -Z)
        if (node_) node_->SetRotation(Quaternion(0.0f, tpsYaw_, 0.0f));
        URHO3D_LOGINFOF("[FIGHTER] -> SIDE mode (A/D only) yaw=%.1f", tpsYaw_);
    } else {
        tpsYaw_ = 0.0f; // Face +Z when entering TPS from SIDE
        if (node_) {
            node_->SetRotation(Quaternion(0.0f, tpsYaw_, 0.0f));
            if (meshNode_) meshNode_->SetRotation(Quaternion(0.0f, -90.0f, 0.0f)); // Reset mesh to TPS default
            URHO3D_LOGINFOF("[FIGHTER] -> TPS mode (yaw=%.1f)", tpsYaw_);
        }
    }
}

void Fighter::Reset()
{
    isSwinging_ = false; isBiteSynced_ = false; isLunging_ = false; lungeDistCovered_ = 0.0f; swingTimer_ = 0.0f;
    biteTextureActive_ = false; redTintActive_ = false;
    onGround_ = false; inAirTime_ = 0.0f;  // Reset Ninja-style state
    controls_.Reset();  // Clear controls
    
    for (auto& slot : materialSlots_) { if (slot.drawable && slot.originalMaterial) slot.drawable->SetMaterial(slot.geoIndex, slot.originalMaterial); slot.activeMaterial.Reset(); }
    if (node_) {
        node_->SetPosition(arenaPosition_); 
        tpsYaw_ = 30.0f; // Start rotated 30 degrees right
        node_->SetRotation(Quaternion(0.0f, tpsYaw_, 0.0f));
        if (rigidBody_) { rigidBody_->SetKinematic(false); rigidBody_->SetAngularFactor(Vector3::ZERO); rigidBody_->SetPosition(arenaPosition_); rigidBody_->SetRotation(Quaternion(0.0f, tpsYaw_, 0.0f)); rigidBody_->SetLinearVelocity(Vector3::ZERO); rigidBody_->SetAngularVelocity(Vector3::ZERO); rigidBody_->Activate(); }
    }
    if (meshNode_) { meshNode_->SetRotation(Quaternion(0.0f, -90.0f, 0.0f)); meshNode_->SetScale(5.0f / 9.0f); meshNode_->SetPosition(Vector3(0.0f, 0.05f, 0.0f)); }
    if (animController_) animController_->StopAll();
}

void Fighter::StartAttack()
{
    if (isSwinging_ || isBiteSynced_ || !animController_) return;
    isSwinging_ = true; swingTimer_ = 0.0f; lungeDistCovered_ = 0.0f;
    if (lungeTarget_.Equals(Vector3::ZERO)) isLunging_ = false;
    if (rigidBody_) rigidBody_->SetUseGravity(false);
    animController_->StopAll();
    AnimationParameters params(context_, attackAnimPath_); params.looped_ = false;
    animController_->PlayNewExclusive(params);
    animController_->SetTime(attackAnimPath_, 0.0f);
    animController_->SetSpeed(attackAnimPath_, 1.0f);
}

// ============================================================================
// NINJA-STYLE: Force-Based Movement with Ground Detection
// ============================================================================
void Fighter::FixedUpdate(float timeStep)
{
    g_fighterLogTimer += timeStep;
    bool doLog = (g_fighterLogTimer >= 1.0f);
    if (doLog) { g_fighterLogTimer = 0.0f; g_fighterLogFrame++; }

    // === CRITICAL: Sync node position from rigidbody each frame ===
    if (rigidBody_ && node_) {
        Vector3 bodyPos = rigidBody_->GetPosition();
        
        // In SIDE mode: lock Z to arena position
        if (currentMoveMode_ == MoveMode::SIDE) {
            bodyPos.z_ = arenaPosition_.z_;
        }
        
        Vector3 correctedPos(bodyPos.x_, arenaPosition_.y_, bodyPos.z_);
        node_->SetPosition(correctedPos);
        node_->SetRotation(Quaternion(0.0f, tpsYaw_, 0.0f));
    }

    // === NINJA-STYLE: Ground state update ===
    if (onGround_)
    {
        inAirTime_ = 0.0f;
    }
    else
    {
        inAirTime_ += timeStep;
    }

    Vector3 pos = node_->GetPosition();

    // FALLBACK: If Y position is at floor level, assume grounded.
    if (pos.y_ <= 0.05f && inAirTime_ < 0.1f)
    {
        onGround_ = true;
        inAirTime_ = 0.0f;
    }

    // Reset for next frame, but keep floor-level fallback true until height changes.
    if (pos.y_ > 0.1f)
        onGround_ = false;

    // === ANTI-SINK: Zero out Y velocity ===
    if (rigidBody_) {
        Vector3 vel = rigidBody_->GetLinearVelocity();
        if (vel.y_ != 0.0f) { 
            vel.y_ = 0.0f;
            rigidBody_->SetLinearVelocity(vel); 
        }
    }

    // === CHECK FOR MOVEMENT LOCK ===
    if (isBiteSynced_) return;

    // === SWINGING ===
    if (isSwinging_) {
        swingTimer_ += timeStep;
        if (isLunging_ && rigidBody_ && swingTimer_ <= 0.45f) {
            Vector3 toTarget = lungeTarget_ - node_->GetPosition(); toTarget.y_ = 0.0f;
            float dist = toTarget.Length();
            if (dist > 0.02f) { 
                toTarget.Normalize(); 
                // NINJA-STYLE: Use force for lunge
                rigidBody_->ApplyForce(toTarget * 800.0f); 
            }
            else { isLunging_ = false; }
        } else if (isLunging_) { isLunging_ = false; }
        
        if (!animController_->IsPlaying(attackAnimPath_)) {
            isSwinging_ = false; isLunging_ = false; swingTimer_ = 0.0f; lungeDistCovered_ = 0.0f;
            if (rigidBody_) { rigidBody_->SetUseGravity(true); rigidBody_->Activate(); }
            if (meshNode_) { meshNode_->SetRotation(Quaternion(0.0f, -90.0f, 0.0f)); meshNode_->SetScale(5.0f / 9.0f); meshNode_->SetPosition(Vector3(0.0f, 0.05f, 0.0f)); }
        }
        return;
    }

    if (!rigidBody_) return;

    if (doLog) {
        URHO3D_LOGINFOF("[FIGHTER] frame=%d mode=%d ground=%d inAir=%.2f pos=%s", 
            g_fighterLogFrame, (int)currentMoveMode_, onGround_, inAirTime_, node_->GetPosition().ToString().c_str());
    }

    // === SIDE MODE: Force-Based Movement ===
    if (currentMoveMode_ == MoveMode::SIDE) {
        float moveForce = 0.0f;
        
        // Read from controls structure
        if (controls_.left) moveForce = -SIDE_MOVE_FORCE;
        if (controls_.right) moveForce = SIDE_MOVE_FORCE;
        
        // Return-to-center spring when idle
        if (moveForce == 0.0f && !isBiteSynced_) {
            Vector3 bodyPos = rigidBody_->GetPosition();
            float xDiff = arenaPosition_.x_ - bodyPos.x_;
            if (Abs(xDiff) > 0.05f) {
                moveForce = xDiff * 200.0f;  // Spring force
            }
        }

        // NINJA-STYLE: Apply force based on ground state
        if (inAirTime_ < IN_AIR_THRESHOLD)  // On ground or grace period
        {
            rigidBody_->ApplyForce(Vector3(moveForce, 0.0f, 0.0f));
        }
        else  // In air - reduced control
        {
            rigidBody_->ApplyForce(Vector3(moveForce * 0.3f, 0.0f, 0.0f));
        }
        
        // NINJA-STYLE: Velocity clamping
        Vector3 vel = rigidBody_->GetLinearVelocity();
        float hSpeed = sqrtf(vel.x_ * vel.x_ + vel.z_ * vel.z_);
        if (hSpeed > SIDE_MAX_SPEED) {
            vel.x_ *= (SIDE_MAX_SPEED / hSpeed);
            vel.z_ *= (SIDE_MAX_SPEED / hSpeed);
            rigidBody_->SetLinearVelocity(vel);
        }

        // Lock Z position
        Vector3 bodyPosCheck = rigidBody_->GetPosition();
        if (Abs(bodyPosCheck.z_ - arenaPosition_.z_) > 0.001f) {
            bodyPosCheck.z_ = arenaPosition_.z_;
            rigidBody_->SetPosition(bodyPosCheck);
        }

        // Face direction: +Z (0°) or -Z (180°) instead of +X/-X
        float sideYaw = 0.0f;
        if (vel.x_ < -0.1f) sideYaw = 180.0f;
        else if (vel.x_ > 0.1f) sideYaw = 0.0f;
        else if (auto* zNode = node_->GetScene()->GetChild("ZombiePivot")) {
            float zDir = zNode->GetWorldPosition().x_ - rigidBody_->GetPosition().x_;
            sideYaw = (zDir > 0.0f) ? 0.0f : 180.0f;
        }
        tpsYaw_ = sideYaw;

        if (meshNode_) {
            meshNode_->SetRotation(Quaternion(0.0f, -tpsYaw_, 0.0f));
            meshNode_->SetScale(5.0f / 9.0f);
            Vector3 mp = meshNode_->GetPosition();
            mp.y_ = 0.05f;
            meshNode_->SetPosition(mp);
        }

        if (doLog) URHO3D_LOGINFOF("[FIGHTER-SIDE] vel=(%.2f, 0, 0) force=%.1f yaw=%.1f ground=%d", 
            vel.x_, moveForce, tpsYaw_, onGround_);
        return;
    }

    // === TPS MODE: FREE 360° MOVEMENT ON XZ PLANE ===
    float moveX = 0.0f;
    float moveZ = 0.0f;
    
    // Read from controls structure
    if (controls_.forward) moveZ += 1.0f;  // W - Forward
    if (controls_.back) moveZ -= 1.0f;     // S - Backward
    if (controls_.left) moveX -= 1.0f;     // A - Strafe Left
    if (controls_.right) moveX += 1.0f;    // D - Strafe Right

    // Normalize diagonal movement (prevents faster diagonal movement)
    float len = sqrtf(moveX * moveX + moveZ * moveZ);
    if (len > 0.001f) {
        moveX /= len;
        moveZ /= len;
    }

    if (doLog) URHO3D_LOGINFOF("[FIGHTER-TPS] localDir=(%.2f, %.2f)", moveX, moveZ);

    // Convert local WASD direction to WORLD direction based on camera/fighter yaw
    // This allows movement in ANY direction on the XZ plane
    float yawRad = tpsYaw_ * M_DEGTORAD;
    float sinY = sinf(yawRad);
    float cosY = cosf(yawRad);
    
    // Forward/Back (moveZ): moves along the direction we're facing
    // Strafe Left/Right (moveX): moves perpendicular to facing direction
    float worldX = moveX * cosY + moveZ * sinY;
    float worldZ = moveZ * cosY - moveX * sinY;

    if (doLog) URHO3D_LOGINFOF("[FIGHTER-TPS] worldDir=(%.3f, %.3f) yaw=%.1f", worldX, worldZ, tpsYaw_);

    // NINJA-STYLE: Apply force based on ground state
    if (inAirTime_ < IN_AIR_THRESHOLD)  // On ground or grace period
    {
        rigidBody_->ApplyForce(Vector3(worldX * TPS_MOVE_FORCE, 0.0f, worldZ * TPS_MOVE_FORCE));
    }
    else  // In air - reduced control
    {
        rigidBody_->ApplyForce(Vector3(worldX * TPS_AIR_FORCE, 0.0f, worldZ * TPS_AIR_FORCE));
    }
    
    // NINJA-STYLE: Velocity clamping (prevents infinite acceleration)
    Vector3 vel = rigidBody_->GetLinearVelocity();
    float hSpeed = sqrtf(vel.x_ * vel.x_ + vel.z_ * vel.z_);
    if (hSpeed > TPS_MAX_SPEED) {
        vel.x_ *= (TPS_MAX_SPEED / hSpeed);
        vel.z_ *= (TPS_MAX_SPEED / hSpeed);
        rigidBody_->SetLinearVelocity(vel);
    }

    // Sync rotation (fighter faces mouse direction)
    Quaternion aimQuat(0.0f, tpsYaw_, 0.0f);
    rigidBody_->SetRotation(aimQuat);
    node_->SetRotation(aimQuat);

    if (meshNode_) {
        meshNode_->SetScale(5.0f / 9.0f);
        Vector3 mp = meshNode_->GetPosition();
        mp.y_ = 0.05f;
        meshNode_->SetPosition(mp);
        
        if (doLog) URHO3D_LOGINFOF("[FIGHTER-TPS] pivotYaw=%.1f meshWorldYaw=%.1f", 
            tpsYaw_, tpsYaw_ - 90.0f);
    }

    if (doLog) {
        URHO3D_LOGINFOF("[FIGHTER-TPS] yaw=%.1f local=(%.2f,%.2f) world=(%.3f,%.3f) vel=(%.3f,%.3f) ground=%d",
            tpsYaw_, moveX, moveZ, worldX, worldZ, vel.x_, vel.z_, onGround_);
    }
}

void Fighter::ReparentWeapon(Node* meshNode)
{
    if (!meshNode) return;
    auto* animatedModel = meshNode->FindComponent<AnimatedModel>();
    if (!animatedModel) return;
    Skeleton& skeleton = animatedModel->GetSkeleton();
    Bone* rightHandBone = skeleton.GetBone("RightHand");
    if (rightHandBone && rightHandBone->node_) {
        ea::vector<Node*> allDescendants;
        ea::vector<Node*> stack = { meshNode };
        while (!stack.empty()) {
            Node* current = stack.back(); stack.pop_back();
            for (Node* child : current->GetChildren()) { allDescendants.push_back(child); stack.push_back(child); }
        }
        for (Node* child : allDescendants) {
            auto* sm = child->GetComponent<StaticModel>();
            if (!sm) continue;
            Model* mdl = sm->GetModel();
            if (!mdl || std::strstr(mdl->GetName().c_str(), "BattleAxe") == nullptr) continue;
            Vector3 worldPos = child->GetWorldPosition();
            Quaternion worldRot = child->GetWorldRotation();
            child->SetParent(rightHandBone->node_);
            Quaternion handInv = rightHandBone->node_->GetWorldRotation().Inverse();
            child->SetPosition(handInv * (worldPos - rightHandBone->node_->GetWorldPosition()));
            child->SetRotation(handInv * worldRot);
        }
    }
}
