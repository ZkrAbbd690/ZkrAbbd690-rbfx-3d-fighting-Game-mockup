#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputConstants.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/PrefabResource.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>

#include <vector>
#include <cstring>

using namespace Urho3D;

class FightScene : public Application {
    URHO3D_OBJECT(FightScene, Application);
public:
    explicit FightScene(Context* context) : Application(context) {}

    void Setup() override {
        engineParameters_[EP_WINDOW_TITLE] = "rbfx Animation Test";
        engineParameters_[EP_RESOURCE_PATHS] = "Data;CoreData";
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_LOG_NAME] = "Fighter.log";
    }

    void Start() override {
        auto* cache = GetSubsystem<ResourceCache>();
        auto* ui = GetSubsystem<UI>();
        auto* input = GetSubsystem<Input>();
        scene_ = new Scene(context_);
        scene_->CreateComponent<Octree>();

        Node* zoneNode = scene_->CreateChild("Zone");
        auto* zone = zoneNode->CreateComponent<Zone>();
        zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
        zone->SetAmbientColor(Color(0.3f, 0.3f, 0.3f));

        Node* dirLightNode = scene_->CreateChild("DirLight");
        dirLightNode->SetDirection(Vector3(0.3f, -1.0f, 0.5f));
        auto* dirLight = dirLightNode->CreateComponent<Light>();
        dirLight->SetLightType(LIGHT_DIRECTIONAL);
        dirLight->SetBrightness(0.5f);

        Node* floorNode = scene_->CreateChild("Floor");
        floorNode->SetPosition(Vector3(0.0f, -0.01f, 0.0f));
        floorNode->SetScale(Vector3(50.0f, 1.0f, 50.0f));
        auto* floorModel = floorNode->CreateComponent<StaticModel>();
        floorModel->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
        floorModel->SetMaterial(cache->GetResource<Material>("Materials/DefaultGrey.xml"));

        fighterNode_ = scene_->CreateChild("Fighter");
        auto* prefab = cache->GetResource<PrefabResource>("Firing Rifle.fbx.d/Prefab.prefab");

        if (prefab) {
            fighterNode_->InstantiatePrefab(prefab);

            auto* animCtrl = fighterNode_->FindComponent<AnimationController>();
            if (animCtrl) {
                const char* animPath = "Firing Rifle.fbx.d/Animations/mixamo.com.ani";
                bool played = animCtrl->Play(animPath, 0, true, 0.0f);

                if (played) {
                    // The editor saved a slow default speed in the prefab.
                    // Force a multiplier to match Assimp Viewer playback.
                    // Tune this value by eye: 1.0 = normal, 3.0 = 3x faster, etc.
                    animCtrl->SetSpeed(animPath, 3.0f);

                    URHO3D_LOGINFO("Animation playing - speed forced to 3.0x");
                } else {
                    URHO3D_LOGERROR("Play() returned false - animation failed to start!");
                }
            }

            // Scale: 1/3 of the previous 5/3 size = 5/9
            fighterNode_->SetRotation(Quaternion(0.0f, -90.0f, 0.0f));
            fighterNode_->SetScale(5.0f / 9.0f);
            Vector3 pos = fighterNode_->GetPosition();
            pos.y_ -= 0.05f;
            fighterNode_->SetPosition(pos);

            // Weapon reparenting
            auto* animatedModel = fighterNode_->FindComponent<AnimatedModel>();
            if (animatedModel) {
                Skeleton& skeleton = animatedModel->GetSkeleton();

                Bone* rightHandBone = skeleton.GetBone("RightHand");
                if (rightHandBone && rightHandBone->node_) {
                    URHO3D_LOGINFO("RightHand bone found -- reparenting weapon parts...");

                    std::vector<Node*> allDescendants;
                    std::vector<Node*> stack;
                    stack.push_back(fighterNode_);
                    while (!stack.empty()) {
                        Node* current = stack.back();
                        stack.pop_back();
                        for (Node* child : current->GetChildren()) {
                            allDescendants.push_back(child);
                            stack.push_back(child);
                        }
                    }

                    for (Node* child : allDescendants) {
                        auto* sm = child->GetComponent<StaticModel>();
                        if (!sm) continue;

                        Model* mdl = sm->GetModel();
                        if (!mdl) continue;

                        const char* mdlName = mdl->GetName().c_str();
                        if (std::strncmp(mdlName, "_2nrt", 5) == 0)
                            continue;

                        Vector3 worldPos = child->GetWorldPosition();
                        Quaternion worldRot = child->GetWorldRotation();

                        child->SetParent(rightHandBone->node_);

                        Quaternion handInv = rightHandBone->node_->GetWorldRotation().Inverse();
                        child->SetPosition(handInv * (worldPos - rightHandBone->node_->GetWorldPosition()));
                        child->SetRotation(handInv * worldRot);

                        URHO3D_LOGINFOF("Reparented '%s' to RightHand", mdlName);
                    }
                } else {
                    URHO3D_LOGERROR("RightHand bone not found -- weapon will not stick to hand!");
                }
            }
        } else {
            URHO3D_LOGERROR("Could not find Firing Rifle.fbx.d/Prefab.prefab!");
        }

        // Camera
        cameraNode_ = scene_->CreateChild("Camera");
        cameraNode_->SetPosition(Vector3(0.0f, 1.5f, -5.0f));
        cameraNode_->LookAt(Vector3(0.0f, 1.0f, 0.0f));
        cameraNode_->CreateComponent<Camera>();

        // Spotlight from camera
        Node* spotLightNode = cameraNode_->CreateChild("SpotLight");
        auto* spotLight = spotLightNode->CreateComponent<Light>();
        spotLight->SetLightType(LIGHT_SPOT);
        spotLight->SetBrightness(3.0f);
        spotLight->SetRange(25.0f);
        spotLight->SetFov(50.0f);
        spotLight->SetColor(Color(1.0f, 0.95f, 0.85f));

        GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));

        // Mouse setup
        input->SetMouseMode(MM_ABSOLUTE);
        input->SetMouseVisible(true);

        // Crosshair UI
        crosshair_ = ui->GetRoot()->CreateChild<Text>("Crosshair");
        crosshair_->SetText("+");
        crosshair_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 32);
        crosshair_->SetHorizontalAlignment(HA_LEFT);
        crosshair_->SetVerticalAlignment(VA_TOP);
        crosshair_->SetColor(Color(1.0f, 0.0f, 0.0f));

        // Events
        SubscribeToEvent(E_KEYDOWN, &FightScene::HandleKeyDown);
        SubscribeToEvent(E_UPDATE, &FightScene::HandleUpdate);
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData) {
        auto* input = GetSubsystem<Input>();
        auto* graphics = GetSubsystem<Graphics>();
        if (!graphics) return;

        IntVector2 mousePos = input->GetMousePosition();
        crosshair_->SetPosition(mousePos.x_ - 8, mousePos.y_ - 16);
    }

    void HandleKeyDown(StringHash eventType, VariantMap& eventData) {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE) engine_->Exit();
    }

private:
    SharedPtr<Scene> scene_;
    Node* cameraNode_ = nullptr;
    Node* fighterNode_ = nullptr;
    Text* crosshair_ = nullptr;
};

URHO3D_DEFINE_APPLICATION_MAIN(FightScene)