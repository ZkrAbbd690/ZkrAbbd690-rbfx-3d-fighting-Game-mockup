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

#include <cstring>

using namespace Urho3D;

int main(int argc, char** argv)
{
    SharedPtr<Context> context(new Context());
    SharedPtr<Engine> engine(new Engine(context));

    StringVariantMap engineParameters;
    engineParameters[EP_WINDOW_TITLE] = "Brute Axe Animation";
    engineParameters[EP_RESOURCE_PREFIX_PATHS] = "Data";
    engineParameters[EP_FULL_SCREEN] = false;
    
    engineParameters[EP_WINDOW_WIDTH] = 800;
    engineParameters[EP_WINDOW_HEIGHT] = 450;
    
    engineParameters[EP_MULTI_SAMPLE] = 1;
    engineParameters[EP_VSYNC] = false; 

    if (!engine->Initialize(engineParameters, {}))
    {
        URHO3D_LOGERROR("Could not initialise the engine.");
        return 1;
    }

    auto* renderer = engine->GetSubsystem<Renderer>();
    auto* cache    = engine->GetSubsystem<ResourceCache>();
    auto* input    = engine->GetSubsystem<Input>();

    // ---- Scene ----
    SharedPtr<Scene> scene(new Scene(context));
    scene->CreateComponent<Octree>();

    auto* zoneNode = scene->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(1.0f, 1.0f, 1.0f)); // High ambient for visibility

    // Key light
    auto* lightNode = scene->CreateChild("DirectionalLight");
    auto* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    lightNode->SetDirection(Vector3(0.3f, -1.0f, 0.5f));
    light->SetBrightness(1.5f); // 3x brightness boost
    light->SetShadowIntensity(0.0f); 

    // Camera
    auto* cameraNode = scene->CreateChild("Camera");
    cameraNode->SetPosition(Vector3(0.0f, 1.5f, -5.0f));
    cameraNode->LookAt(Vector3(0.0f, 1.0f, 0.0f));
    auto* camera = cameraNode->CreateComponent<Camera>();

    SharedPtr<Viewport> viewport(new Viewport(context, scene, camera));
    renderer->SetViewport(0, viewport);

    // ================================================================
    //  CHARACTER (Using test.glb.d)
    // ================================================================
    auto* characterNode = scene->CreateChild("Character");
    // FIXED: Pointing directly to your test directory asset
    auto* prefab = cache->GetResource<PrefabResource>("test.glb.d/Prefab.prefab");

    if (prefab)
    {
        characterNode->InstantiatePrefab(prefab);
        URHO3D_LOGINFO("Prefab instantiated successfully.");

        auto* animCtrl = characterNode->FindComponent<AnimationController>();
        if (animCtrl)
        {
            // FIXED: Updated animation path path
            const ea::string animPath = "test.glb.d/Animations/mixamo.com.ani";
            if (animCtrl->Play(animPath, 0, true, 0.0f))
            {
                animCtrl->SetSpeed(animPath, 2.0f); // 2x slower speed
                URHO3D_LOGINFO("Animation playing at 0.5x speed.");
            }
            else
            {
                URHO3D_LOGERROR("Failed to play animation!");
            }
        }

        characterNode->SetRotation(Quaternion(0.0f, -90.0f, 0.0f));
        characterNode->SetScale(5.0f / 9.0f);
        Vector3 pos = characterNode->GetPosition();
        pos.y_ -= 0.05f;
        characterNode->SetPosition(pos);

        // ---- Weapon reparenting ----
        auto* animatedModel = characterNode->FindComponent<AnimatedModel>();
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
                stack.push_back(characterNode);
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
                    
                    // FIXED: Target BattleAxe_GEO specifically for detachment/reparenting
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

    // ================================================================
    //  MAIN LOOP
    // ================================================================
    while (!engine->IsExiting())
    {
        engine->RunFrame();

        if (input->GetKeyPress(KEY_ESCAPE))
            engine->Exit();
    }

    renderer->SetViewport(0, nullptr);
    engine->Exit();

    return 0;
}