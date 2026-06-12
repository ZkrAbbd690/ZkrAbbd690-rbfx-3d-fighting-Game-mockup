#include "CombatSystem.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Graphics/Octree.h> // <--- FIXED: Added missing header for Raycast and Octree!
#include <Urho3D/Math/Random.h>
#include <cmath>
#include <cfloat>

CombatSystem::CombatSystem(Context* context)
: Component(context)
, fighter_(nullptr)
, zombie_(nullptr)
, debugText_(nullptr)
, arenaHitFrozen_(false)
, biteDelayTimer_(0.0f)
, biteDelayActive_(false)
, bloodScale_(1.0f)
, bloodInitialized_(false)
, bloodComputeDelay_(0.0f)
, bloodOffset_(-0.02f)
{
zombieOffsetScale_ = 0.820f;
zombieOffsetRotation_ = 78.0f;
fighterOffsetRotation_ = 0.0f;
zombieOffsetPosition_ = Vector3(-0.865f, 0.0f, 0.000f); 
fighterOffsetPosition_ = Vector3(0.000f, 0.0f, 0.000f);
}

Node* CombatSystem::FindClosestFighterBone(const Vector3& worldPos)
{
    if (!fighter_) return nullptr;
    
    Node* fNode = fighter_->GetNode();
    if (!fNode) return nullptr;
    
    Node* fMesh = fNode->GetChild("AxeManMesh");
    if (!fMesh) fMesh = fNode;
    
    AnimatedModel* am = fMesh->FindComponent<AnimatedModel>();
    if (!am) am = fNode->FindComponent<AnimatedModel>();
    if (!am) return fMesh;
    
    Skeleton& skel = am->GetSkeleton();
    Node* closestBone = nullptr;
    float closestDist = FLT_MAX;
    
    for (unsigned i = 0; i < skel.GetNumBones(); ++i)
    {
        Bone* bone = skel.GetBone(i);
        if (!bone || !bone->node_) continue;
        
        float dist = (bone->node_->GetWorldPosition() - worldPos).Length();
        if (dist < closestDist)
        {
            closestDist = dist;
            closestBone = bone->node_;
        }
    }
    
    return closestBone ? closestBone : fMesh;
}

void CombatSystem::Initialize(Fighter* fighter, Zombie* zombie, Text* uiText)
{
fighter_ = fighter;
zombie_ = zombie;
debugText_ = uiText;
arenaHitFrozen_ = false;
biteDelayTimer_ = 0.0f;
biteDelayActive_ = false;
bloodScale_ = 1.0f;
bloodInitialized_ = false;
bloodComputeDelay_ = 0.0f;
bitePaused_ = false;
zombieBiteAnimPath_.clear();
fighterBiteAnimPath_.clear();
bloodOffset_ = -0.02f;
bloodWounds_.clear();
activeWound_.Reset();
ClearBloodWounds();
}

void CombatSystem::SpawnBloodDisc(const Vector3& worldPos, const Vector3& worldNormal, Node* attachNode)
{
Node* parent = fighter_ ? fighter_->GetNode() : nullptr;
if (!parent) return;
Node* meshNode = parent->GetChild("AxeManMesh");
if (!meshNode) meshNode = parent;

Node* effectiveParent = attachNode ? attachNode : meshNode;

Vector3 localPos = effectiveParent->WorldToLocal(worldPos);
Vector3 localNormal = effectiveParent->GetWorldRotation().Inverse() * worldNormal;
localNormal.Normalize();

auto* cache = GetSubsystem<ResourceCache>();

// CREATE THE GROUP NODE (So N/M moves the whole disc together)
Node* woundGroup = effectiveParent->CreateChild("BiteWoundGroup");
woundGroup->SetPosition(localPos);

Quaternion alignRot(Vector3::UP, localNormal);
float mainRot = Random(0.0f, 360.0f);
Quaternion twistRot(mainRot, localNormal);
woundGroup->SetRotation(alignRot * twistRot);

// Apply the depth offset
woundGroup->Translate(Vector3::UP * bloodOffset_, TS_LOCAL);

// CREATE THE SIMPLE RED DISC
Node* woundDisc = woundGroup->CreateChild("BloodDisc");
woundDisc->SetPosition(Vector3::ZERO);

auto* sm = woundDisc->CreateComponent<StaticModel>();
sm->SetModel(cache->GetResource<Model>("Models/Box.mdl"));

if (!bloodMaterial_) {
    bloodMaterial_ = new Material(context_);
    bloodMaterial_->SetTechnique(0, cache->GetResource<Technique>("Techniques/NoTexture.xml"));
    bloodMaterial_->SetShaderParameter("MatDiffColor", Variant(Color(1.0f, 0.0f, 0.0f))); // Bright Red
    bloodMaterial_->SetCullMode(CULL_NONE);
}
sm->SetMaterial(bloodMaterial_);

// Flat disc scale: Wide X, Paper-thin Y, Wide Z
woundDisc->SetWorldScale(Vector3(0.08f, 0.001f, 0.08f) * bloodScale_);

bloodWounds_.push_back(SharedPtr<Node>(woundGroup));
activeWound_ = woundGroup; // Make it the active one

// LOGGING FOR SEQUENCE BAKING
URHO3D_LOGINFOF("[BITE-LOG] BAKE THIS -> Bone: %s | Pos: %s | Rot: %s",
    effectiveParent->GetName().c_str(),
    woundGroup->GetPosition().ToString().c_str(),
    woundGroup->GetRotation().ToString().c_str());
}

void CombatSystem::DiscardActiveWound()
{
    if (!activeWound_) return;

    URHO3D_LOGINFO("[BLOOD] Discarding active wound.");

    // Remove from the list
    bloodWounds_.erase(ea::remove(bloodWounds_.begin(), bloodWounds_.end(), activeWound_), bloodWounds_.end());

    // Remove from the scene
    activeWound_->Remove();
    activeWound_ = nullptr;
}

void CombatSystem::AdjustBloodOffset(float delta)
{
bloodOffset_ += delta; // Save to global offset so future spawns inherit it

// NEW: Only move the active wound!
if (activeWound_) {
activeWound_->Translate(Vector3::UP * delta, TS_LOCAL);
URHO3D_LOGINFOF("[BLOOD] Adjusted ACTIVE wound. Offset: %.4f", bloodOffset_);
} else {
URHO3D_LOGINFO("[BLOOD] No active wound selected. Use [ and ] to select one.");
}
}

void CombatSystem::SelectNextWound(bool forward)
{
if (bloodWounds_.empty()) return;

int currentIdx = -1;
for (int i = 0; i < (int)bloodWounds_.size(); ++i) {
if (bloodWounds_[i] == activeWound_) {
currentIdx = i;
break;
}
}

// Calculate next index, wrapping around
if (forward) {
currentIdx = (currentIdx + 1) % bloodWounds_.size();
} else {
currentIdx = (currentIdx - 1 + bloodWounds_.size()) % bloodWounds_.size();
}

activeWound_ = bloodWounds_[currentIdx];

// Optional: Make the selected wound briefly brighter or bigger so you know which one it is
// For now, we just log it.
URHO3D_LOGINFOF("[BLOOD] Selected wound %d / %zu", currentIdx + 1, bloodWounds_.size());
}

void CombatSystem::GrowBloodWounds(float delta)
{
bloodScale_ = Clamp(bloodScale_ + delta, 0.2f, 8.0f);
}

void CombatSystem::ClearBloodWounds()
{
for (auto& wound : bloodWounds_) {
if (wound && wound->GetParent()) {
wound->GetParent()->RemoveChild(wound);
}
}
bloodWounds_.clear();
activeWound_.Reset();
bloodScale_ = 1.0f;
bloodInitialized_ = false;
bloodComputeDelay_ = 0.0f;
}

void CombatSystem::ComputeBloodIntersections()
{
ClearBloodWounds();
Node* fNode = fighter_->GetNode();
Node* zNode = zombie_->GetNode();
if (!fNode || !zNode) return;

auto* octree = GetScene()->GetComponent<Octree>();
if (!octree) return;

Node* fMesh = fNode->GetChild("AxeManMesh");
if (!fMesh) fMesh = fNode;
AnimatedModel* fAm = fMesh->FindComponent<AnimatedModel>();
if (!fAm) fAm = fNode->FindComponent<AnimatedModel>();

Node* zMesh = zNode->GetChild("ZombieMesh");
if (!zMesh) zMesh = zNode;
AnimatedModel* zAm = zMesh->FindComponent<AnimatedModel>();
if (!zAm) zAm = zNode->FindComponent<AnimatedModel>();

if (!zAm || !fAm) return;

Skeleton& zSkel = zAm->GetSkeleton();
Vector3 fCenter = fNode->GetWorldPosition();

for (unsigned i = 0; i < zSkel.GetNumBones(); ++i)
{
Bone* bone = zSkel.GetBone(i);
if (!bone || !bone->node_) continue;
ea::string bName = bone->name_;

bool relevant = false;
if (bName.find("Spine") != ea::string::npos) relevant = true;
if (bName.find("Neck") != ea::string::npos) relevant = true;
if (bName.find("Head") != ea::string::npos) relevant = true;
if (bName.find("Clavicle") != ea::string::npos) relevant = true;
if (bName.find("Shoulder") != ea::string::npos) relevant = true;
if (bName.find("Arm") != ea::string::npos) relevant = true;
if (bName.find("Hand") != ea::string::npos) relevant = true;
if (!relevant) continue;

Vector3 boneWorldPos = bone->node_->GetWorldPosition();
Vector3 dir = (fCenter - boneWorldPos).Normalized();
Ray ray(boneWorldPos, dir);

ea::vector<RayQueryResult> results;
RayOctreeQuery query(results, ray, RAY_TRIANGLE, 2.0f, DRAWABLE_GEOMETRY);
octree->Raycast(query);

for (const auto& res : results)
{
Node* hitNode = res.drawable_->GetNode();
if (hitNode == fMesh || hitNode->GetParent() == fMesh || hitNode == fNode)
{
SpawnBloodDisc(res.position_, res.normal_, fMesh);
break;
}
}
}
}

void CombatSystem::ToggleBitePause()
{
if (!zombie_ || !zombie_->IsNeckBiting()) return;
bitePaused_ = !bitePaused_;
Node* zNode = zombie_->GetNode();
Node* fNode = fighter_ ? fighter_->GetNode() : nullptr;
auto setAnimSpeed = [](Node* node, const ea::string& animPath, float speed) {
if (!node || animPath.empty()) return;
AnimationController* anim = node->GetComponent<AnimationController>();
if (!anim) {
ea::vector<Node*> children;
node->GetChildrenWithComponent<AnimationController>(children, true);
if (!children.empty() && children.front()) anim = children.front()->GetComponent<AnimationController>();
}
if (anim) anim->SetSpeed(animPath, speed);
};
float speed = bitePaused_ ? 0.0f : 1.0f;
setAnimSpeed(zNode, zombieBiteAnimPath_, speed);
setAnimSpeed(fNode, fighterBiteAnimPath_, speed);
}

void CombatSystem::UpdateCombat(float timeStep)
{
if (!fighter_ || !zombie_) return;
if (!arenaHitFrozen_ && !zombie_->IsNeckBiting()) zombie_->CheckCollision(fighter_);
if (!arenaHitFrozen_ && zombie_->IsContactFrozen() && !zombie_->IsNeckBiting() && !zombie_->IsFrozen())
{
    // Only accumulate bite delay in SIDE mode
    if (fighter_->GetMoveMode() == MoveMode::SIDE)
    {
        if (!biteDelayActive_) { biteDelayActive_ = true; biteDelayTimer_ = 0.0f; }
        if (!fighter_->IsMidSwing()) biteDelayTimer_ += timeStep;
    }
    else
    {
        biteDelayActive_ = false;
        biteDelayTimer_ = 0.0f;
    }
}
else
{
    biteDelayActive_ = false;
    biteDelayTimer_ = 0.0f;
}

if (fighter_->IsMidSwing() && !zombie_->IsFrozen() && !zombie_->IsNeckBiting() && !arenaHitFrozen_)
{
float deltaX = std::abs(fighter_->GetPosition().x_ - zombie_->GetPosition().x_);
if (deltaX < 1.4f)
{
arenaHitFrozen_ = true; biteDelayActive_ = false; biteDelayTimer_ = 0.0f;
zombie_->HandleHit(fighter_);
if (debugText_) { debugText_->SetText("CHOP DETECTED!"); debugText_->SetColor(Color::RED); }
return;
}
}

// ONLY auto-trigger in SIDE mode
if (NECKBITE_DEBUG_ENABLED && fighter_->GetMoveMode() == MoveMode::SIDE && biteDelayActive_ && biteDelayTimer_ > 2.0f && !fighter_->IsMidSwing())
{
biteDelayActive_ = false; biteDelayTimer_ = 0.0f; arenaHitFrozen_ = true;
zombie_->StartNeckBite(fighter_->GetNode()); zombie_->SetBiteSynced(true); fighter_->SetBiteSynced(true);
if (fighter_) fighter_->SetBiteTexture(true);
ClearBloodWounds();
if (debugText_) { debugText_->SetText("NECK BITE SEQUENCING ACTIVE."); debugText_->SetColor(Color::YELLOW); }
Node* fNode = fighter_->GetNode(); Node* zNode = zombie_->GetNode();
if (fNode && zNode)
{
if (fighter_->GetRigidBody()) { fighter_->GetRigidBody()->SetKinematic(true); fighter_->GetRigidBody()->SetLinearVelocity(Vector3::ZERO); fighter_->GetRigidBody()->SetAngularVelocity(Vector3::ZERO); }
if (zombie_->GetRigidBody()) { zombie_->GetRigidBody()->SetKinematic(true); zombie_->GetRigidBody()->SetLinearVelocity(Vector3::ZERO); zombie_->GetRigidBody()->SetAngularVelocity(Vector3::ZERO); }
fNode->SetRotation(Quaternion(0.0f, fighterOffsetRotation_, 0.0f)); zNode->SetRotation(Quaternion(0.0f, zombieOffsetRotation_, 0.0f));
Vector3 currentFighterPos = fNode->GetPosition(); currentFighterPos.y_ = 0.0f; fNode->SetPosition(currentFighterPos);
Vector3 targetZombiePos = currentFighterPos + zombieOffsetPosition_; zNode->SetPosition(targetZombiePos);
if (fighter_->GetRigidBody()) fighter_->GetRigidBody()->SetPosition(currentFighterPos);
if (zombie_->GetRigidBody()) zombie_->GetRigidBody()->SetPosition(targetZombiePos);

AnimationController* fAnim = nullptr; ea::vector<Node*> fChildren; fNode->GetChildrenWithComponent<AnimationController>(fChildren, true);
if (auto* topF = fNode->GetComponent<AnimationController>()) fAnim = topF; else if (!fChildren.empty() && fChildren.front()) fAnim = fChildren.front()->GetComponent<AnimationController>();
if (fAnim) { ea::string path = "fighter-nek-bite/test.glb.d/Animations/Armature.001_Armature.001_mixamo.com_Layer0.ani"; fAnim->StopAll(); AnimationParameters params(context_, path); params.looped_ = true; fAnim->PlayNewExclusive(params); fighterBiteAnimPath_ = path; }
AnimationController* zAnim = nullptr; ea::vector<Node*> zChildren; zNode->GetChildrenWithComponent<AnimationController>(zChildren, true);
if (auto* topZ = zNode->GetComponent<AnimationController>()) zAnim = topZ; else if (!zChildren.empty() && zChildren.front()) zAnim = zChildren.front()->GetComponent<AnimationController>();
if (zAnim) { ea::string path = "zombie-nek-bite/test.glb.d/Animations/Armature_Armature_mixamo.com_Layer0_0.ani"; zAnim->StopAll(); AnimationParameters params(context_, path); params.looped_ = true; zAnim->PlayNewExclusive(params); zombieBiteAnimPath_ = path; }
}
}
if (arenaHitFrozen_ && zombie_->IsNeckBiting())
{
if (!bloodInitialized_) { bloodComputeDelay_ += timeStep; if (bloodComputeDelay_ > 0.40f) { ComputeBloodIntersections(); bloodInitialized_ = true; bloodComputeDelay_ = 0.0f; } }
Node* fNode = fighter_->GetNode(); Node* zNode = zombie_->GetNode();
if (fNode && zNode)
{
fNode->SetRotation(Quaternion(0.0f, fighterOffsetRotation_, 0.0f)); zNode->SetRotation(Quaternion(0.0f, zombieOffsetRotation_, 0.0f));
Vector3 fPos = fNode->GetPosition(); fPos.y_ = 0.0f; fNode->SetPosition(fPos);
Vector3 zPos = fPos + zombieOffsetPosition_; zNode->SetPosition(zPos);
if (fighter_->GetRigidBody()) { fighter_->GetRigidBody()->SetKinematic(true); fighter_->GetRigidBody()->SetPosition(fPos); fighter_->GetRigidBody()->SetLinearVelocity(Vector3::ZERO); fighter_->GetRigidBody()->SetAngularVelocity(Vector3::ZERO); }
if (zombie_->GetRigidBody()) { zombie_->GetRigidBody()->SetKinematic(true); zombie_->GetRigidBody()->SetPosition(zPos); zombie_->GetRigidBody()->SetLinearVelocity(Vector3::ZERO); zombie_->GetRigidBody()->SetAngularVelocity(Vector3::ZERO); }
}
}
}

void CombatSystem::HandleManualAttack()
{
if (!fighter_ || !zombie_) return;
if (zombie_->IsFrozen() || zombie_->IsNeckBiting()) { ResetSimulation(); return; }
Vector3 fPos = fighter_->GetPosition(); Vector3 zPos = zombie_->GetPosition(); Vector3 delta = zPos - fPos; delta.y_ = 0.0f; float dist = delta.Length();
float step = dist - 0.9f + 0.01f; if (step < 0.0f) step = 0.0f; if (step > 0.35f) step = 0.35f;
Vector3 lungeGoal = fPos + delta.Normalized() * step; lungeGoal.y_ = fPos.y_; fighter_->SetLungeTarget(lungeGoal); fighter_->StartAttack();
}

void CombatSystem::ResetSimulation()
{
ClearBloodWounds(); arenaHitFrozen_ = false; biteDelayTimer_ = 0.0f; biteDelayActive_ = false; bitePaused_ = false; zombieBiteAnimPath_.clear(); fighterBiteAnimPath_.clear();
if (fighter_) fighter_->Reset(); if (zombie_) zombie_->Reset();
if (debugText_) { debugText_->SetText("STAGE ACTIVE: RUNNING..."); debugText_->SetColor(Color::GREEN); }
}