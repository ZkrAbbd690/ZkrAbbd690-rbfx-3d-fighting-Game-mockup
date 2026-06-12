#pragma once
#include <Urho3D/Scene/Component.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/CustomGeometry.h>
#include "Fighter.h"
#include "Zombie.h"

using namespace Urho3D;

class CombatSystem : public Component
{
URHO3D_OBJECT(CombatSystem, Component);
public:
CombatSystem(Context* context);
~CombatSystem() override = default;
void Initialize(Fighter* fighter, Zombie* zombie, Text* uiText);
void UpdateCombat(float timeStep);
void HandleManualAttack();
void ResetSimulation();
void ComputeBloodIntersections();
void SpawnBloodDisc(const Vector3& worldPos, const Vector3& worldNormal, Node* attachNode);
void GrowBloodWounds(float delta);
void ClearBloodWounds();
void ToggleBitePause();
void AdjustBloodOffset(float delta);
void SelectNextWound(bool forward); 
Node* FindClosestFighterBone(const Vector3& worldPos);
void DiscardActiveWound(); // ADDED BACK

bool IsArenaHitFrozen() const { return arenaHitFrozen_; }
static constexpr bool NECKBITE_DEBUG_ENABLED = true; 

private:
Fighter* fighter_;
Zombie* zombie_;
Text* debugText_;
bool arenaHitFrozen_;
float biteDelayTimer_ = 0.0f;
bool biteDelayActive_ = false;
float zombieOffsetScale_;
float zombieOffsetRotation_;
float fighterOffsetRotation_;
Vector3 zombieOffsetPosition_;
Vector3 fighterOffsetPosition_;
ea::vector<SharedPtr<Node>> bloodWounds_;
float bloodScale_ = 1.0f;
bool bloodInitialized_ = false;
float bloodComputeDelay_ = 0.0f;

bool bitePaused_{false};
ea::string zombieBiteAnimPath_;
ea::string fighterBiteAnimPath_;

float bloodOffset_{-0.02f}; 
SharedPtr<Node> activeWound_; 
SharedPtr<Material> bloodMaterial_; // ADDED BACK
};