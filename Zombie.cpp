#include "Zombie.h"
#include "Fighter.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/PrefabResource.h>
#include <Urho3D/Scene/Scene.h>         // <--- STRICT FIX 1: Added missing header for Scene
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <cmath>

Zombie::Zombie(Context* context) : LogicComponent(context)
    , onGround_(false)
    , wasOnGround_(false)
    , isSliding_(false)
    , inAirTime_(0.0f)
{
    SetUpdateEventMask(USE_FIXEDUPDATE);
    URHO3D_LOGINFO("[ZOMBIE] Constructor - Ninja Snow War style AI Controller enabled");
}

void Zombie::RegisterObject(Context* context)
{
    context->RegisterFactory<Zombie>();
}

void Zombie::Initialize(Node* node, const ea::string& prefabPath)
{
    node_ = node;
    if (!node_) return;

    AnimatedModel* animModel = nullptr;
    AnimationController* animCtrl = nullptr;

    startPosition_ = node_->GetPosition();
    node_->SetRotation(Quaternion(0.0f, 180.0f, 0.0f));

    size_t lastSlash = prefabPath.find_last_of('/');
    runAnimPath_ = (lastSlash != ea::string::npos) ? 
        prefabPath.substr(0, lastSlash) + "/Animations/mixamo.com.ani" : prefabPath;

    auto* cache = GetSubsystem<ResourceCache>();
    if (lastSlash != ea::string::npos)
    {
        ea::string animDir = prefabPath.substr(0, lastSlash) + "/Animations/";
        ea::vector<ea::string> possibleAttacks = {
            animDir + "Zombie_Attack.ani",
            animDir + "Attack.ani",
            animDir + "Bite.ani",
            animDir + "Slash.ani",
            animDir + "Throw.ani"
        };
        for (const auto& p : possibleAttacks)
        {
            if (cache->Exists(p))
            {
                attackAnimPath_ = p;
                URHO3D_LOGINFOF("[ZOMBIE] Found Attack Animation: %s", p.c_str());
                break;
            }
        }
    }

    meshNode_ = node_->CreateChild("ZombieMesh");
    auto* prefab = cache->GetResource<PrefabResource>(prefabPath);

    if (prefab)
    {
        prefab->InstantiateReference(meshNode_);

        ea::vector<Node*> stack;
        stack.push_back(meshNode_);
        while (!stack.empty())
        {
            Node* current = stack.back();
            stack.pop_back();
            if (!animModel) animModel = current->GetComponent<AnimatedModel>();
            if (!animCtrl) animCtrl = current->GetComponent<AnimationController>();
            if (animModel && animCtrl) break;
            for (Node* child : current->GetChildren()) stack.push_back(child);
        }
    }

    if (animModel)
    {
        modelNode_ = animModel->GetNode();
        animModel->SetCastShadows(true);
        animModel->SetUpdateInvisible(true);
        animModel->SetEnabled(true);
        animModel->SetViewMask(0xFFFFFFFF);
        
        // One-third size
        modelNode_->SetScale(66.67f);
        modelNode_->SetPosition(Vector3::ZERO);
    }

    if (animCtrl) animController_ = animCtrl;

    SetupPhysics();
    URHO3D_LOGINFO("[ZOMBIE] Initialization complete");

    Reset();
}

void Zombie::SetupPhysics()
{
    if (!node_) return;
    rigidBody_ = node_->GetOrCreateComponent<RigidBody>();
    if (rigidBody_)
    {
        rigidBody_->SetMass(60.0f);
        rigidBody_->SetUseGravity(true);
        rigidBody_->SetLinearDamping(0.5f);   
        rigidBody_->SetAngularDamping(10.0f);
        rigidBody_->SetKinematic(false);
        rigidBody_->SetCollisionLayer(Z_LAYER_ZOMBIE);
        rigidBody_->SetCollisionMask(Z_LAYER_WORLD | Z_LAYER_FIGHTER);
        rigidBody_->SetAngularFactor(Vector3::ZERO);
        rigidBody_->SetCollisionEventMode(COLLISION_ALWAYS);
        rigidBody_->SetRestitution(0.0f);
        rigidBody_->SetFriction(0.6f);
    }
    collisionShape_ = node_->GetOrCreateComponent<CollisionShape>();
    if (collisionShape_)
        collisionShape_->SetCapsule(0.45f, 1.6f, Vector3(0.0f, 0.8f, 0.0f));
        
    SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(Zombie, HandleNodeCollision));
}

void Zombie::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    using namespace NodeCollision;

    RigidBody* otherBody = static_cast<RigidBody*>(eventData[P_OTHERBODY].GetPtr());
    if (!otherBody) return;

    bool isGround = (otherBody->GetCollisionLayer() == Z_LAYER_WORLD) ||
                    (otherBody->GetMass() == 0.0f && otherBody->IsKinematic());

    if (isGround)
    {
        MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
        while (!contacts.IsEof())
        {
            Vector3 contactPosition = contacts.ReadVector3();
            Vector3 contactNormal = contacts.ReadVector3();
            contacts.ReadFloat(); 
            contacts.ReadFloat(); 

            if (contactPosition.y_ < node_->GetPosition().y_ + 0.5f && 
                contactNormal.y_ > 0.75f)
            {
                onGround_ = true;
            }
        }
    }
}

void Zombie::FixedUpdate(float timeStep)
{
    if (fighter_)
        isSideMode_ = (fighter_->GetMoveMode() == MoveMode::SIDE);

    float floorY = isSideMode_ ? sideStartPosition_.y_ : startPosition_.y_;
    if (node_)
    {
        Vector3 pos = node_->GetPosition();
        if (std::abs(pos.y_ - floorY) > 0.0001f)
        {
            pos.y_ = floorY;
            node_->SetPosition(pos);
        }
    }
    if (rigidBody_)
    {
        Vector3 rbPos = rigidBody_->GetPosition();
        if (std::abs(rbPos.y_ - floorY) > 0.0001f)
        {
            rbPos.y_ = floorY;
            rigidBody_->SetPosition(rbPos);
        }
        Vector3 vel = rigidBody_->GetLinearVelocity();
        if (vel.y_ != 0.0f)
        {
            vel.y_ = 0.0f;
            rigidBody_->SetLinearVelocity(vel);
        }
    }

    if (isBiteSynced_ || health_ <= 0 || isFrozen_ || isNeckBiting_) return;

    wasOnGround_ = onGround_;
    onGround_ = false;  
    isSliding_ = false;

    Vector3 pos = node_->GetPosition();
    if (pos.y_ <= 0.05f && !wasOnGround_)
    {
        pos.y_ = 0.0f;
        node_->SetPosition(pos);
        if (rigidBody_)
        {
            Vector3 vel = rigidBody_->GetLinearVelocity();
            if (vel.y_ < 0) vel.y_ = 0;
            rigidBody_->SetLinearVelocity(vel);
            rigidBody_->SetPosition(pos);
        }
        onGround_ = true;
        wasOnGround_ = true;
    }

    if (attackCooldown_ >= 0.0f)
        attackCooldown_ -= timeStep;

    if (!isContactFrozen_ && !isFrozen_ && controls_.attack && attackCooldown_ <= 0.0f)
    {
        PerformAttack();
        attackCooldown_ = ZOMBIE_ATTACK_DELAY;
    }

    if (!isContactFrozen_ && !isFrozen_ && node_)
    {
        Quaternion aimRot(0.0f, controls_.yaw, 0.0f);
        node_->SetRotation(aimRot);
        if (rigidBody_) rigidBody_->SetRotation(aimRot);
    }

    UpdateMovement(timeStep);
    UpdateAnimation();
}

void Zombie::PerformAttack()
{
    if (!node_ || !node_->GetScene()) return;

    URHO3D_LOGINFO("[ZOMBIE] Launching Projectile exact at Fighter!");

    auto* cache = GetSubsystem<ResourceCache>();

    if (animController_)
    {
        if (!attackAnimPath_.empty())
        {
            animController_->PlayExclusive(attackAnimPath_, 1, false, 0.1f);
            animController_->SetTime(attackAnimPath_, 0.0f);
        }
        else if (!runAnimPath_.empty())
        {
            animController_->PlayExclusive(runAnimPath_, 1, false, 0.1f);
            animController_->SetTime(runAnimPath_, 0.0f);
            animController_->SetSpeed(runAnimPath_, 2.5f);
        }
    }

    Node* projNode = node_->GetScene()->CreateChild("SnowBall");
    
    // --- CRITICAL AIM FIX: Launch exactly towards Fighter ---
    Vector3 myPos = node_->GetPosition();
    Vector3 targetPos = fighter_ ? fighter_->GetPosition() : myPos + Vector3(-1.0f, 0.0f, 0.0f);
    
    // Target chest height
    Vector3 aimVec = (targetPos + Vector3::UP * 1.0f) - (myPos + Vector3::UP * 1.0f);
    aimVec.Normalize();

    Quaternion aimRot(Vector3::FORWARD, aimVec);
    
    // Spawn projectile in front of zombie
    Vector3 spawnPos = myPos + Vector3::UP * 1.0f + aimVec * 0.8f;
    projNode->SetPosition(spawnPos);
    projNode->SetRotation(aimRot);
    projNode->SetScale(0.35f);

    auto* sm = projNode->CreateComponent<StaticModel>();
    Model* sphereModel = cache->GetResource<Model>("Models/Sphere.mdl");
    if (sphereModel) sm->SetModel(sphereModel);

    Material* baseMat = cache->GetResource<Material>("Materials/StoneTiled.xml");
    if (baseMat)
    {
        SharedPtr<Material> projMat = baseMat->Clone();
        projMat->SetShaderParameter("MatDiffColor", Color(0.1f, 0.8f, 0.1f, 1.0f)); 
        sm->SetMaterial(projMat);
    }

    auto* body = projNode->CreateComponent<RigidBody>();
    body->SetMass(1.0f);
    body->SetUseGravity(true);
    body->SetCollisionLayer(Z_LAYER_ZOMBIE);
    body->SetCollisionMask(Z_LAYER_WORLD | Z_LAYER_FIGHTER);
    body->SetCollisionEventMode(COLLISION_ALWAYS);

    auto* shape = projNode->CreateComponent<CollisionShape>();
    shape->SetSphere(0.35f);

    // ninjaThrowVelocity exactly like Ninja Snow War sample
    Vector3 throwVel = aimVec * 20.0f + Vector3::UP * 2.0f;
    body->SetLinearVelocity(throwVel);

    SubscribeToEvent(projNode, E_NODECOLLISION, URHO3D_HANDLER(Zombie, HandleProjectileCollision));
}

// <--- STRICT FIX 2: Using Flawless P_BODY / P_OTHERBODY keys
void Zombie::HandleProjectileCollision(StringHash eventType, VariantMap& eventData)
{
    using namespace NodeCollision;
    RigidBody* projBody = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());
    RigidBody* otherBody = static_cast<RigidBody*>(eventData[P_OTHERBODY].GetPtr());

    if (!projBody || !otherBody) return;

    Node* projNode = projBody->GetNode();
    Node* otherNode = otherBody->GetNode();
    if (!projNode || !otherNode) return;

    if (otherBody->GetCollisionLayer() == Z_LAYER_ZOMBIE) return; 

    URHO3D_LOGINFOF("[ZOMBIE-HIT] Projectile successfully hit: %s", otherNode->GetName().c_str());

    if (otherNode->GetName() == "AxeManPivot" || otherNode->GetName() == "AxeManMesh")
    {
        // Push fighter when hit
        otherBody->ApplyImpulse(projNode->GetWorldRotation() * Vector3(0.0f, 50.0f, 100.0f));
    }

    projNode->Remove();
}

void Zombie::UpdateMovement(float timeStep)
{
    if (!rigidBody_ || !node_) return;

    if (onGround_) inAirTime_ = 0.0f;
    else inAirTime_ += timeStep;

    if (!isContactFrozen_ && !isFrozen_)
    {
        if (controls_.forward)
        {
            Vector3 moveDir = Quaternion(0.0f, controls_.yaw, 0.0f) * Vector3::FORWARD;
            float force = (inAirTime_ < IN_AIR_THRESHOLD) ? WALK_FORCE : AIR_FORCE;
            rigidBody_->ApplyForce(moveDir * force);

            Vector3 vel = rigidBody_->GetLinearVelocity();
            Vector3 hVel(vel.x_, 0.0f, vel.z_);
            if (hVel.Length() > WALK_SPEED)
            {
                hVel = hVel.Normalized() * WALK_SPEED;
                rigidBody_->SetLinearVelocity(Vector3(hVel.x_, vel.y_, hVel.z_));
            }
        }
        else
        {
            Vector3 vel = rigidBody_->GetLinearVelocity();
            rigidBody_->ApplyImpulse(Vector3(-5.0f * vel.x_ * timeStep, 0.0f, -5.0f * vel.z_ * timeStep));
        }

        Vector3 velCheck = rigidBody_->GetLinearVelocity();
        if (Abs(velCheck.y_) < 0.05f) 
        {
            velCheck.y_ = 0.0f;
            rigidBody_->SetLinearVelocity(velCheck);
        }
    }
    else
    {
        rigidBody_->SetLinearVelocity(Vector3::ZERO);
        rigidBody_->SetAngularVelocity(Vector3::ZERO);
    }
}

void Zombie::UpdateAnimation()
{
    if (!animController_) return;

    if (isContactFrozen_ || isFrozen_)
    {
        if (!isFrozen_)
            animController_->SetSpeed(runAnimPath_, 0.0f);
    }
    else if (rigidBody_)
    {
        Vector3 vel = rigidBody_->GetLinearVelocity();
        float speed = Vector3(vel.x_, 0.0f, vel.z_).Length();

        float animSpeed = (speed / WALK_SPEED) * 6.25f;
        animSpeed = Clamp(animSpeed, 0.0f, 5.0f);
        animController_->SetSpeed(runAnimPath_, animSpeed);
    }
}

void Zombie::CheckCollision(Fighter* fighter)
{
    if (!node_ || !fighter || isFrozen_ || health_ <= 0 || isNeckBiting_ || isBiteSynced_) return;

    isSideMode_ = (fighter->GetMoveMode() == MoveMode::SIDE);

    Vector3 myPos = rigidBody_ ? rigidBody_->GetPosition() : node_->GetPosition();
    Vector3 targetPos = fighter->GetPosition();
    const float floorY = isSideMode_ ? sideStartPosition_.y_ : startPosition_.y_;

    if (isSideMode_)
    {
        float xDist = fabsf(myPos.x_ - targetPos.x_);
        if (xDist > 0.5f) return;

        if (myPos.x_ <= targetPos.x_ + STOP_THRESHOLD)
        {
            Vector3 clampedPos(targetPos.x_ + STOP_THRESHOLD, floorY, myPos.z_);
            node_->SetPosition(clampedPos);
            rigidBody_->SetPosition(clampedPos);
        }
    }
    else
    {
        float zDist = fabsf(myPos.z_ - targetPos.z_);
        if (zDist > 0.5f) return;

        if (myPos.z_ <= targetPos.z_ + STOP_THRESHOLD)
        {
            Vector3 clampedPos(myPos.x_, floorY, targetPos.z_ + STOP_THRESHOLD);
            node_->SetPosition(clampedPos);
            rigidBody_->SetPosition(clampedPos);
        }
    }
}

void Zombie::HandleHit(Fighter* fighter)
{
    if (hasBeenHit_) return;
    hasBeenHit_ = true;
    isFrozen_ = true;
    if (animController_) animController_->StopAll();
    if (rigidBody_)
    {
        rigidBody_->SetLinearVelocity(Vector3::ZERO);
        rigidBody_->SetAngularVelocity(Vector3::ZERO);
        rigidBody_->SetKinematic(true);
        Vector3 pos = rigidBody_->GetPosition();
        pos.y_ = startPosition_.y_;
        rigidBody_->SetPosition(pos);
    }
    if (node_)
    {
        Vector3 pos = node_->GetPosition();
        pos.y_ = startPosition_.y_;
        node_->SetPosition(pos);
    }
}

void Zombie::ApplyBoundaryFreeze()
{
    if (isContactFrozen_) return;
    isContactFrozen_ = true;
    if (rigidBody_) rigidBody_->SetLinearVelocity(Vector3::ZERO);
}

void Zombie::ClearBoundaryFreeze()
{
    if (!isContactFrozen_) return;
    isContactFrozen_ = false;
    if (rigidBody_) rigidBody_->Activate();
}

void Zombie::SetMovementDirection(const Vector3& direction)
{
    forcedMovementDirection_ = direction;
    forcedMovementDirection_.y_ = 0.0f;
    hasForcedMovementDirection_ = forcedMovementDirection_.Length() > 0.001f;
}

void Zombie::TriggerNeckBite()
{
    if (isNeckBiting_) return;
    isNeckBiting_ = true;
    if (rigidBody_) rigidBody_->SetLinearVelocity(Vector3::ZERO);
}

void Zombie::StartNeckBite(Node* targetNode)
{
    TriggerNeckBite();
}

void Zombie::Reset()
{
    isFrozen_ = false;
    isContactFrozen_ = false;
    isNeckBiting_ = false;
    isBiteSynced_ = false;
    hasBeenHit_ = false;
    health_ = maxHealth_;
    onGround_ = false;
    wasOnGround_ = false;
    isSliding_ = false;
    inAirTime_ = 0.0f;
    attackCooldown_ = 0.0f;

    if (node_)
    {
        Vector3 resetPos = isSideMode_ ? sideStartPosition_ : startPosition_;
        Quaternion resetRot = isSideMode_ ? 
            Quaternion(0.0f, 270.0f, 0.0f) : Quaternion(0.0f, 180.0f, 0.0f);

        node_->SetPosition(resetPos);
        node_->SetRotation(resetRot);

        if (rigidBody_)
        {
            rigidBody_->SetKinematic(false);
            rigidBody_->SetAngularFactor(Vector3::ZERO);
            rigidBody_->SetPosition(resetPos);
            rigidBody_->SetRotation(resetRot);
            rigidBody_->SetLinearVelocity(Vector3::ZERO);
            rigidBody_->SetAngularVelocity(Vector3::ZERO);
            rigidBody_->ResetForces();
            rigidBody_->Activate();
        }
    }

    if (meshNode_)
    {
        meshNode_->SetPosition(Vector3::ZERO);
        meshNode_->SetRotation(Quaternion::IDENTITY);
    }

    if (animController_ && !runAnimPath_.empty())
    {
        animController_->StopAll();
        AnimationParameters params(context_, runAnimPath_);
        params.looped_ = true;
        params.layer_ = 0;
        animController_->PlayNewExclusive(params);
        animController_->SetTime(runAnimPath_, 0.0f);
        animController_->SetSpeed(runAnimPath_, 1.0f);
    }
}