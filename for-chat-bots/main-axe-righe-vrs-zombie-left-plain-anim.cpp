#include <Urho3D/Urho3D.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/PrefabResource.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/GraphicsDefs.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Font.h> 
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Core/StringUtils.h> 

#include <cstring>

using namespace Urho3D;

int main(int argc, char** argv)
{
    SharedPtr<Context> context(new Context());
    SharedPtr<Engine> engine(new Engine(context));

    StringVariantMap engineParameters;
    engineParameters[EP_WINDOW_TITLE] = "Kilo-Code Battle Arena: Brute Axe vs Zombie";
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

    auto* renderer = engine->GetSubsystem<Renderer>();
    auto* cache    = engine->GetSubsystem<ResourceCache>();
    auto* input    = engine->GetSubsystem<Input>();
    auto* ui       = engine->GetSubsystem<UI>();

    // ---- Fallback UI Diagnostics ----
    auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
    if (font)
    {
        auto* debugText = ui->GetRoot()->CreateChild<Text>();
        debugText->SetText("STAGE ACTIVE: BRUTE AXE (RIGHT) VS RUNNING ZOMBIE (LEFT)");
        debugText->SetFont(font, 16);
        debugText->SetColor(Color::GREEN);
        debugText->SetHorizontalAlignment(HA_CENTER);
        debugText->SetVerticalAlignment(VA_TOP);
    }

    // ---- Scene Setup ----
    SharedPtr<Scene> scene(new Scene(context));
    scene->CreateComponent<Octree>();

    auto* zoneNode = scene->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(1.0f, 1.0f, 1.0f)); 

    // Key light
    auto* lightNode = scene->CreateChild("DirectionalLight");
    auto* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    lightNode->SetDirection(Vector3(0.3f, -1.0f, 0.5f));
    light->SetBrightness(1.5f); 
    light->SetShadowIntensity(0.0f);

    // Enhanced Overhead Face/Head Lighting
    auto* topLightNode = scene->CreateChild("OverheadDiagnosticLight");
    auto* topLight = topLightNode->CreateComponent<Light>();
    topLight->SetLightType(LIGHT_DIRECTIONAL);
    topLightNode->SetDirection(Vector3(0.0f, -1.0f, 0.0f)); 
    topLight->SetBrightness(3.0f); 

    // Camera Configuration 
    auto* cameraNode = scene->CreateChild("Camera");
    cameraNode->SetPosition(Vector3(0.0f, 1.2f, -3.5f)); 
    cameraNode->LookAt(Vector3(0.0f, 0.6f, 1.6f));
    auto* camera = cameraNode->CreateComponent<Camera>();

    SharedPtr<Viewport> viewport(new Viewport(context, scene, camera));
    renderer->SetViewport(0, viewport);

    // ========================================================================
    //  CHARACTER 1: BRUTE AXE MAN (RIGHT SIDE - FACING LEFT SCREEN)
    // ========================================================================
    auto* axeManPivot = scene->CreateChild("AxeManPivot");
    // Shifted from 0.6f to 0.5f to move 1 cm leftwards
    axeManPivot->SetPosition(Vector3(0.59f, 0.0f, 1.6f)); 
    axeManPivot->SetRotation(Quaternion::IDENTITY); 

    auto* axeManNode = axeManPivot->CreateChild("AxeManMesh");
    auto* axePrefab = cache->GetResource<PrefabResource>("test.glb.d/Prefab.prefab");

    if (axePrefab)
    {
        axeManNode->InstantiatePrefab(axePrefab);
        URHO3D_LOGINFO("Axe Man Prefab instantiated successfully.");

        auto* animCtrl = axeManNode->FindComponent<AnimationController>();
        if (animCtrl)
        {
            const ea::string animPath = "test.glb.d/Animations/mixamo.com.ani";
            if (animCtrl->Play(animPath, 0, true, 0.0f))
            {
                animCtrl->SetSpeed(animPath, 2.0f); 
                URHO3D_LOGINFO("Axe Man Animation playing.");
            }
            else
            {
                URHO3D_LOGERROR("Failed to play Axe Man animation!");
            }
        }

        axeManNode->SetRotation(Quaternion(0.0f, -90.0f, 0.0f));
        axeManNode->SetScale(5.0f / 9.0f);
        Vector3 pos = axeManNode->GetPosition();
        pos.y_ -= 0.05f; 
        axeManNode->SetPosition(pos);

        // ---- Weapon reparenting stack routine ----
        auto* animatedModel = axeManNode->FindComponent<AnimatedModel>();
        if (animatedModel)
        {
            animatedModel->SetUpdateInvisible(false);
            
            Skeleton& skeleton = animatedModel->GetSkeleton();
            Bone* rightHandBone = skeleton.GetBone("RightHand");

            if (rightHandBone && rightHandBone->node_)
            {
                URHO3D_LOGINFO("RightHand bone found -- reparenting BattleAxe...");

                ea::vector<Node*> allDescendants;
                ea::vector<Node*> stack;
                stack.push_back(axeManNode);
                while (!stack.empty())
                {
                    Node* current = stack.back();
                    stack.pop_back();
                    for (Node* child : current->GetChildren())
                    {
                        allDescendants.push_back(child);
                        stack.push_back(child);
                    }
                }

                for (Node* child : allDescendants)
                {
                    auto* sm = child->GetComponent<StaticModel>();
                    if (!sm) continue;

                    Model* mdl = sm->GetModel();
                    if (!mdl) continue;

                    const char* mdlName = mdl->GetName().c_str();
                    
                    if (std::strstr(mdlName, "BattleAxe") == nullptr)
                        continue;

                    Vector3 worldPos = child->GetWorldPosition();
                    Quaternion worldRot = child->GetWorldRotation();

                    child->SetParent(rightHandBone->node_);

                    Quaternion handInv = rightHandBone->node_->GetWorldRotation().Inverse();
                    child->SetPosition(handInv * (worldPos - rightHandBone->node_->GetWorldPosition()));
                    child->SetRotation(handInv * worldRot);
                }
            }
        }
    }
    else
    {
        URHO3D_LOGERROR("Could not find test.glb.d/Prefab.prefab!");
        engine->Exit();
        return 2;
    }

    // ========================================================================
    //  CHARACTER 2: RUNNING ZOMBIE (LEFT SIDE - CHARGING RIGHTWARD)
    // ========================================================================
    auto* zombiePivot = scene->CreateChild("ZombiePivot");
    zombiePivot->SetPosition(Vector3(-2.0f, 0.0f, 1.6f)); 
    zombiePivot->SetRotation(Quaternion(0.0f, 90.0f, 0.0f)); 

    auto* zombieNode = zombiePivot->CreateChild("ZombieMesh");
    const ea::string zombieRoot = "firing-running-didn'twork/test_out/test.gltf.d/"; 
    auto* zombiePrefab = cache->GetResource<PrefabResource>(zombieRoot + "Prefab.prefab");

    if (zombiePrefab)
    {
        zombieNode->InstantiatePrefab(zombiePrefab);
        auto* animCtrl = zombieNode->FindComponent<AnimationController>();
        if (animCtrl)
        {
            const ea::string animPath = zombieRoot + "Animations/mixamo.com.ani";
            if (animCtrl->Play(animPath, 0, true, 0.0f))
            {
                animCtrl->SetSpeed(animPath, 0.5f); 
            }
        }
        zombieNode->SetRotation(Quaternion::IDENTITY);
        zombieNode->SetScale(75.0f); // Doubled from 50.0f to 100.0f!
        zombieNode->SetPosition(Vector3::ZERO);
    }

    // ================================================================
    //  MAIN APPLICATION RUNTIME LOOP
    // ================================================================
    while (!engine->IsExiting())
    {
        engine->RunFrame();

        if (input->GetKeyDown(KEY_W)) cameraNode->Translate(Vector3::FORWARD * 0.1f);
        if (input->GetKeyDown(KEY_S)) cameraNode->Translate(Vector3::BACK * 0.1f);
        
        if (input->GetKeyPress(KEY_ESCAPE)) engine->Exit();
    }

    renderer->SetViewport(0, nullptr);
    engine->Exit();

    return 0;
}