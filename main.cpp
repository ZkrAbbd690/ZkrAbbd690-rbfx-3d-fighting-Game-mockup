#include <Urho3D/Urho3D.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/PrefabResource.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/GraphicsDefs.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include "Fighter.h"
#include "Zombie.h"
#include "ZombieAI.h"
#include "ZombieManager.h"
#include "CombatSystem.h"
#include "CameraController.h"

using namespace Urho3D;

int main(int argc, char** argv)
{
SharedPtr<Context> context(new Context());
SharedPtr<Engine> engine(new Engine(context));
StringVariantMap engineParameters;
engineParameters[EP_WINDOW_TITLE] = "Kilo-Code Battle Arena (Ninja-Style Physics)";
engineParameters[EP_RESOURCE_PREFIX_PATHS] = "Data";
engineParameters[EP_FULL_SCREEN] = false;
engineParameters[EP_WINDOW_WIDTH] = 1152;
engineParameters[EP_WINDOW_HEIGHT] = 864;
engineParameters[EP_MULTI_SAMPLE] = 1;
engineParameters[EP_VSYNC] = true;
if (!engine->Initialize(engineParameters, {}))
{
URHO3D_LOGERROR("Could not initialise the engine.");
return 1;
}

context->RegisterFactory<Fighter>();
context->RegisterFactory<Zombie>();
context->RegisterFactory<ZombieAI>();
context->RegisterFactory<ZombieManager>();
context->RegisterFactory<CombatSystem>();
context->RegisterFactory<CameraController>();

auto* renderer = engine->GetSubsystem<Renderer>();
auto* cache = engine->GetSubsystem<ResourceCache>();
auto* input = engine->GetSubsystem<Input>();
auto* ui = engine->GetSubsystem<UI>();
auto* time = engine->GetSubsystem<Time>();
auto* graphics = engine->GetSubsystem<Graphics>();

URHO3D_LOGINFO("[MAIN] === STARTING WITH NINJA-STYLE PHYSICS ===");

// Mouse visible and free for camera control
input->SetMouseMode(MM_ABSOLUTE);
input->SetMouseVisible(true);

// HUD
auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
auto* debugText = ui->GetRoot()->CreateChild<Text>();
if (font && debugText)
{
debugText->SetText("TAB: TPS/SIDE | WASD: MOVE | MOUSE: AIM | SPACE: ATTACK | [Ninja-Style Physics Active]");
debugText->SetFont(font, 16);
debugText->SetColor(Color::GREEN);
debugText->SetHorizontalAlignment(HA_CENTER);
debugText->SetVerticalAlignment(VA_TOP);
}

// Scene setup
SharedPtr<Scene> scene(new Scene(context));
scene->CreateComponent<Octree>();
scene->CreateComponent<DebugRenderer>();
scene->CreateComponent<PhysicsWorld>();

// Ground plane for visible floor and reliable physics ground contacts.
Node* groundNode = scene->CreateChild("Ground");
groundNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
auto* groundPlane = groundNode->CreateComponent<StaticModel>();
groundPlane->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
groundPlane->SetMaterial(cache->GetResource<Material>("Materials/StoneTiled.xml"));
groundNode->SetScale(Vector3(100.0f, 1.0f, 100.0f));

auto* groundRigidBody = groundNode->CreateComponent<RigidBody>();
groundRigidBody->SetMass(0.0f);
auto* groundCollision = groundNode->CreateComponent<CollisionShape>();
groundCollision->SetBox(Vector3(100.0f, 0.1f, 100.0f));

auto* zone = scene->CreateComponent<Zone>();
zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
zone->SetAmbientColor(Color(0.35f, 0.35f, 0.35f));
zone->SetFogColor(Color(0.55f, 0.55f, 0.55f));
zone->SetFogStart(150.0f);
zone->SetFogEnd(250.0f);

// Lights
Node* l = scene->CreateChild("KeyLight");
l->SetDirection(Vector3(0.2f, -0.8f, 0.5f));
auto* dl = l->CreateComponent<Light>();
dl->SetLightType(LIGHT_DIRECTIONAL);
dl->SetColor(Color(0.8f, 0.784f, 0.76f));
dl->SetBrightness(0.72f);
dl->SetCastShadows(false);

Node* fillLightNode = scene->CreateChild("FillLight");
fillLightNode->SetDirection(Vector3(-0.4f, -0.6f, -0.2f));
auto* fillLight = fillLightNode->CreateComponent<Light>();
fillLight->SetLightType(LIGHT_DIRECTIONAL);
fillLight->SetColor(Color(0.4f, 0.4f, 0.44f));
fillLight->SetBrightness(0.32f);

// Camera
 auto* cameraNode = scene->CreateChild("Camera");
 cameraNode->SetPosition(Vector3(0.0f, 1.2f, -3.5f));
 cameraNode->LookAt(Vector3(0.0f, 0.6f, 1.6f));
 auto* camera = cameraNode->CreateComponent<Camera>();

Node* camLightNode = scene->CreateChild("ViewerLight");
camLightNode->SetPosition(cameraNode->GetPosition());
camLightNode->SetRotation(cameraNode->GetRotation());
auto* camLight = camLightNode->CreateComponent<Light>();
camLight->SetLightType(LIGHT_SPOT);
camLight->SetColor(Color(1.2f, 1.12f, 1.04f));
camLight->SetBrightness(3.2f);
camLight->SetRange(40.0f);
camLight->SetFov(72.0f);
camLight->SetCastShadows(false);

SharedPtr<Viewport> viewport(new Viewport(context, scene, camera));
viewport->SetDrawDebug(true);
renderer->SetViewport(0, viewport);

// Create components
 URHO3D_LOGINFO("[MAIN] Creating Fighter...");
 Node* axeManPivot = scene->CreateChild("AxeManPivot");
 axeManPivot->SetPosition(Vector3(0.59f, 0.0f, 1.6f));
 auto* fighterComponent = axeManPivot->CreateComponent<Fighter>();
 fighterComponent->Initialize(axeManPivot, "test.glb.d/Prefab.prefab");

URHO3D_LOGINFO("[MAIN] Creating CameraController...");
auto* camController = cameraNode->CreateComponent<CameraController>();
camController->Initialize(cameraNode, fighterComponent);
fighterComponent->SetMoveMode(MoveMode::TPS);

URHO3D_LOGINFO("[MAIN] Creating ZombieManager...");
Node* zombieMgrNode = scene->CreateChild("ZombieManagerNode");
auto* zombieManager = zombieMgrNode->CreateComponent<ZombieManager>();
zombieManager->Initialize(scene, fighterComponent, 5);
Zombie* zombieComponent = zombieManager->GetFirstZombie();

URHO3D_LOGINFO("[MAIN] Creating CombatSystem...");
Node* coreSystemNode = scene->CreateChild("CombatSystemNode");
auto* combatSystem = coreSystemNode->CreateComponent<CombatSystem>();
combatSystem->Initialize(fighterComponent, zombieComponent, debugText);

URHO3D_LOGINFO("[MAIN] === ALL READY (NINJA-STYLE ACTIVE) ===");

// Main loop
int frameCount = 0;
float statusTimer = 0.0f;

while (!engine->IsExiting())
{
engine->RunFrame();
float dt = time->GetTimeStep();
frameCount++;
statusTimer += dt;

// === NINJA-STYLE: Set controls structure BEFORE physics update ===
if (fighterComponent)
{
    // Clear previous frame's button states (but preserve accumulated yaw/pitch)
    fighterComponent->controls_.Reset();
    
    // === TPS MOUSE YAW (accumulate into controls) ===
    if (fighterComponent->GetMoveMode() == MoveMode::TPS)
    {
        int mouseMoveX = input->GetMouseMoveX();
        if (mouseMoveX != 0)
        {
            float sensitivity = Fighter::TPS_MOUSE_SENSITIVITY;
            fighterComponent->controls_.yaw += (float)mouseMoveX * sensitivity;
            float clampedYaw = fighterComponent->SetTpsYaw(fighterComponent->controls_.yaw);
            
            static int logCounter = 0;
            if (++logCounter % 10 == 0)  // Log every 10 frames with mouse input
            {
                URHO3D_LOGDEBUGF("[MAIN-TPS] MouseX=%d yaw=%.1f (clamped=%.1f)", 
                    mouseMoveX, fighterComponent->controls_.yaw, clampedYaw);
            }
        }
    }

    // === O KEY: STEP ROTATE LEFT 5 DEGREES (for tuning mouse rotation control) ===
    if (input->GetKeyPress(KEY_O))
    {
        float step = -5.0f; // Left rotation step
        fighterComponent->controls_.yaw += step;
        float clampedYaw = fighterComponent->SetTpsYaw(fighterComponent->controls_.yaw);
        URHO3D_LOGINFOF("[MAIN-O] Step rotate LEFT 5.0° | yaw=%.1f clamped=%.1f",
            fighterComponent->controls_.yaw, clampedYaw);
    }

    // === P KEY: STEP ROTATE RIGHT 5 DEGREES (inverse of O) ===
    if (input->GetKeyPress(KEY_P))
    {
        float step = 5.0f; // Right rotation step
        fighterComponent->controls_.yaw += step;
        float clampedYaw = fighterComponent->SetTpsYaw(fighterComponent->controls_.yaw);
        URHO3D_LOGINFOF("[MAIN-P] Step rotate RIGHT 5.0° | yaw=%.1f clamped=%.1f",
            fighterComponent->controls_.yaw, clampedYaw);
    }

    // === L KEY: MARK CURRENT YAW AS MAX RIGHT LIMIT (for permanent limit tuning) ===
    if (input->GetKeyPress(KEY_L))
    {
        float currentYaw = fighterComponent->controls_.yaw;
        URHO3D_LOGINFOF("[MAIN-L] MARKED MAX RIGHT YAW LIMIT = %.1f | (use this for permanent TPS_YAW_MAX_RIGHT)", currentYaw);
    }
    
    // === NINJA-STYLE: Read WASD keys into controls structure ===
    fighterComponent->controls_.forward = input->GetKeyDown(KEY_W);
    fighterComponent->controls_.back = input->GetKeyDown(KEY_S);
    fighterComponent->controls_.left = input->GetKeyDown(KEY_A);
    fighterComponent->controls_.right = input->GetKeyDown(KEY_D);
    fighterComponent->controls_.attack = input->GetKeyPress(KEY_SPACE);
}

// === TAB TOGGLE ===
if (input->GetKeyPress(KEY_TAB))
{
    URHO3D_LOGINFO("[MAIN] TAB pressed");
    if (camController) camController->ToggleMode();
}

// === SPACE ATTACK ===
if (input->GetKeyPress(KEY_SPACE))
{
    URHO3D_LOGINFO("[MAIN] SPACE pressed");
    if (combatSystem->IsArenaHitFrozen())
    {
        combatSystem->ResetSimulation();
        if (zombieManager)
            zombieManager->Reset();
    }
    else if (zombieManager && zombieManager->GetAliveCount() > 0)
    {
        if (fighterComponent && !fighterComponent->IsSwinging())
            fighterComponent->StartAttack();
    }
    else
        combatSystem->HandleManualAttack();
}

// === Z PAUSE BITE ===
if (input->GetKeyPress(KEY_Z))
{
    if (combatSystem) combatSystem->ToggleBitePause();
}

// === T RED TINT ===
if (input->GetKeyPress(KEY_T))
{
    static bool testRed = false;
    testRed = !testRed;
    if (fighterComponent) fighterComponent->SetRedTint(testRed);
    URHO3D_LOGINFOF("[MAIN] Red tint = %s", testRed ? "ON" : "OFF");
}

// === B N/M WOUND DEPTH ===
if (input->GetKeyPress(KEY_B)) { if (combatSystem) combatSystem->DiscardActiveWound(); }
if (input->GetKeyPress(KEY_N)) { if (combatSystem) combatSystem->AdjustBloodOffset(-0.005f); }
if (input->GetKeyPress(KEY_M)) { if (combatSystem) combatSystem->AdjustBloodOffset(0.005f); }
if (input->GetKeyPress(KEY_RIGHTBRACKET)) { if (combatSystem) combatSystem->SelectNextWound(true); }
if (input->GetKeyPress(KEY_LEFTBRACKET)) { if (combatSystem) combatSystem->SelectNextWound(false); }

// === +/- GROW BLOOD ===
bool shrink = input->GetKeyPress(KEY_MINUS) || input->GetKeyPress(KEY_KP_MINUS);
bool grow = input->GetKeyPress(KEY_EQUALS) || input->GetKeyPress(KEY_KP_PLUS);
if (shrink) combatSystem->GrowBloodWounds(-0.25f);
if (grow) combatSystem->GrowBloodWounds( 0.25f);

// Update zombie manager before combat updates.
// In multi-zombie mode, skip the old CombatSystem bite/update path while zombies are alive.
if (zombieManager)
   zombieManager->Update(dt);

// === F1 DEBUG SPHERES ===
if (input->GetKeyPress(KEY_F1))  // CHANGED: GetKeyDown to GetKeyPress (toggle on press)
{
    static bool debugEnabled = false;
    debugEnabled = !debugEnabled;
    viewport->SetDrawDebug(debugEnabled);
    URHO3D_LOGINFOF("[MAIN] F1 Debug rendering: %s", debugEnabled ? "ON" : "OFF");
}

// ALWAYS draw debug when enabled (not just on key down)
if (viewport->GetDrawDebug())
{
    auto* debug = scene->GetComponent<Urho3D::DebugRenderer>();
    if (debug)
    {
        // Draw sphere at each zombie position
        for (int i = 0; i < zombieManager->GetTotalCount(); ++i)
        {
            Zombie* z = zombieManager->GetZombie(i);
            if (z && !zombieManager->IsZombieDead(i))
            {
                Vector3 pos = z->GetPosition();
                pos.y_ += 1.0f;
                debug->AddSphere(Sphere(pos, 0.8f), Color::RED, false);
                debug->AddLine(pos, pos + Vector3::UP * 2.0f, Color::YELLOW, false);
                
                // ADDED: Draw label with zombie index
                URHO3D_LOGDEBUGF("[DEBUG] Zombie %d sphere at %s", i, pos.ToString().c_str());
            }
        }
        
        // Draw sphere at fighter position
        Vector3 fpos = fighterComponent->GetPosition();
        fpos.y_ += 1.0f;
        debug->AddSphere(Sphere(fpos, 0.5f), Color::GREEN, false);
        
        URHO3D_LOGDEBUGF("[DEBUG] Fighter sphere at %s", fpos.ToString().c_str());
    }
}
if (zombieManager && zombieManager->GetAliveCount() == 0)
   combatSystem->UpdateCombat(dt);

// === STATUS LOG EVERY 2 SECONDS ===
if (statusTimer >= 2.0f)
{
    statusTimer = 0.0f;
   URHO3D_LOGINFOF("[CAM] Position: %s Direction: %s",
       cameraNode->GetPosition().ToString().c_str(),
       cameraNode->GetDirection().ToString().c_str());

   if (fighterComponent) {
       URHO3D_LOGINFOF("[MAIN-STATUS] Fighter: pos=%s yaw=%.1f mode=%s ground=%d inAir=%.2f frame=%d zombies_alive=%d/%d",
           fighterComponent->GetPosition().ToString().c_str(),
           fighterComponent->GetTpsYaw(),
           fighterComponent->GetMoveMode() == MoveMode::TPS ? "TPS" : "SIDE",
           fighterComponent->IsOnGround(),
           fighterComponent->GetInAirTime(),
           frameCount,
           zombieManager ? zombieManager->GetAliveCount() : 0,
           zombieManager ? zombieManager->GetTotalCount() : 0);
   }
   if (zombieComponent) {
       URHO3D_LOGINFOF("[MAIN-STATUS] Primary zombie: pos=%s ground=%d",
           zombieComponent->GetPosition().ToString().c_str(),
           zombieComponent->IsOnGround());
   }

   if (fighterComponent && zombieManager && zombieManager->GetTotalCount() > 0)
   {
       const Vector3 zombie0Pos = zombieManager->GetZombiePosition(0);
       URHO3D_LOGINFOF("[MAIN-STATUS] Fighter: %s | Zombie0: %s | Distance: %.2f",
           fighterComponent->GetPosition().ToString().c_str(),
           zombie0Pos.ToString().c_str(),
           (fighterComponent->GetPosition() - zombie0Pos).Length());
   }

    if (zombieManager)
        zombieManager->LogZombiePositions();
}

// Sync camera light
camLightNode->SetPosition(cameraNode->GetPosition());
camLightNode->SetRotation(cameraNode->GetRotation());

if (input->GetKeyPress(KEY_ESCAPE))
{
    URHO3D_LOGINFO("[MAIN] ESC - exiting");
    engine->Exit();
}
}

renderer->SetViewport(0, nullptr);
engine->Exit();
return 0;
}
