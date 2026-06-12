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
        engineParameters_[EP_WINDOW_TITLE] = "rbfx Spine Aiming Test";
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

        // Environment
        Node* zoneNode = scene_->CreateChild("Zone");
        auto* zone = zoneNode->CreateComponent<Zone>();
        zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
        zone->SetAmbientColor(Color(0.3f, 0.3f, 0.3f));

        Node* dirLightNode = scene_->CreateChild("DirLight");
        dirLightNode->SetDirection(Vector3(0.3f, -1.0f, 0.5f));
        auto* dirLight = dirLightNode->CreateComponent<Light>();
        dirLight->SetLightType(LIGHT_DIRECTIONAL);
        dirLight->SetBrightness(0.5f);

        // Floor
        Node* floorNode = scene_->CreateChild("Floor");
        floorNode->SetPosition(Vector3(0.0f, -0.01f, 0.0f));
        floorNode->SetScale(Vector3(50.0f, 1.0f, 50.0f));
        auto* floorModel = floorNode->CreateComponent<StaticModel>();
        floorModel->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
        floorModel->SetMaterial(cache->GetResource<Material>("Materials/DefaultGrey.xml"));

        // Fighter - FIXED in place, NO ANIMATION
        fighterNode_ = scene_->CreateChild("Fighter");
        auto* prefab = cache->GetResource<PrefabResource>("test.glb.d/Prefab.prefab");

        if (prefab) {
            fighterNode_->InstantiatePrefab(prefab);

            // STOP any auto-playing animation from the prefab
            auto* animCtrl = fighterNode_->FindComponent<AnimationController>();
            if (animCtrl) {
                animCtrl->StopAll();
                URHO3D_LOGINFO("Animation stopped - character in bind pose");
            }

            // Position and scale
            fighterNode_->SetRotation(Quaternion(0.0f, -90.0f, 0.0f));
            fighterNode_->SetScale(1.0f / 3.0f);
            Vector3 pos = fighterNode_->GetPosition();
            pos.y_ -= 0.05f;
            fighterNode_->SetPosition(pos);

            // Get animated model for bone access
            auto* animatedModel = fighterNode_->FindComponent<AnimatedModel>();
            if (animatedModel) {
                Skeleton& skeleton = animatedModel->GetSkeleton();

                // --- SPINE BONE (for aiming) ---
                spineBone_ = skeleton.GetBone("Spine");
                if (spineBone_ && spineBone_->node_) {
                    URHO3D_LOGINFOF("Spine bone found at world pos: %.2f, %.2f, %.2f",
                        spineBone_->node_->GetWorldPosition().x_,
                        spineBone_->node_->GetWorldPosition().y_,
                        spineBone_->node_->GetWorldPosition().z_);
                } else {
                    URHO3D_LOGERROR("Spine bone not found!");
                }

                // --- RIGHT HAND BONE (weapon anchor) ---
                Bone* rightHandBone = skeleton.GetBone("RightHand");
                if (rightHandBone && rightHandBone->node_) {
                    URHO3D_LOGINFO("RightHand bone found – reparenting weapon parts...");

                    // Collect every descendant node first (so we don't modify containers while iterating them)
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
                            continue;   // Body part (hair, face, body, etc.) – skip

                        // Preserve exact world position / rotation when reparenting
                        Vector3 worldPos = child->GetWorldPosition();
                        Quaternion worldRot = child->GetWorldRotation();

                        child->SetParent(rightHandBone->node_);

                        Quaternion handInv = rightHandBone->node_->GetWorldRotation().Inverse();
                        child->SetPosition(handInv * (worldPos - rightHandBone->node_->GetWorldPosition()));
                        child->SetRotation(handInv * worldRot);

                        URHO3D_LOGINFOF("Reparented '%s' to RightHand", mdlName);
                    }
                } else {
                    URHO3D_LOGERROR("RightHand bone not found – weapon will not stick to hand!");
                }
            }
        } else {
            URHO3D_LOGERROR("Could not find test.glb.d/Prefab.prefab!");
        }

        // Camera - FIXED behind character
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

        // Mouse setup - ABSOLUTE mode
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
        auto* cam = cameraNode_->GetComponent<Camera>();
        auto* graphics = GetSubsystem<Graphics>();
        if (!cam || !graphics || !spineBone_ || !spineBone_->node_) return;

        // Crosshair follows mouse
        IntVector2 mousePos = input->GetMousePosition();
        crosshair_->SetPosition(mousePos.x_ - 8, mousePos.y_ - 16);

        // Get aim point in world space
        float normX = (float)mousePos.x_ / (float)graphics->GetWidth();
        float normY = (float)mousePos.y_ / (float)graphics->GetHeight();
        Ray cameraRay = cam->GetScreenRay(normX, normY);
        
        // Aim at a plane 5 units in front of camera
        Vector3 aimPoint = cameraRay.origin_ + cameraRay.direction_ * 5.0f;

        // SPINE AIMING
        Vector3 spinePos = spineBone_->node_->GetWorldPosition();
        
        // Direction from spine to aim point
        Vector3 toTarget = aimPoint - spinePos;
        toTarget.Normalize();

        // The spine is a child of Hips. We need to set its rotation in HIPS-LOCAL space.
        Node* hipsNode = spineBone_->node_->GetParent();
        if (!hipsNode) return;

        // Convert world direction to hips-local direction
        Quaternion hipsWorldInv = hipsNode->GetWorldRotation().Inverse();
        Vector3 hipsLocalTarget = hipsWorldInv * toTarget;

        // Extract yaw (left/right) and pitch (up/down)
        float localYaw = Atan2(hipsLocalTarget.x_, hipsLocalTarget.z_);
        float localPitch = -Asin(hipsLocalTarget.y_);

        // Clamp to reasonable spine limits
        localYaw = Clamp(localYaw, -M_PI * 0.7f, M_PI * 0.7f);    // +/- 126 degrees
        localPitch = Clamp(localPitch, -M_PI * 0.5f, M_PI * 0.5f); // +/- 90 degrees

        // Apply rotation - CONVERT RADIANS TO DEGREES for the Quaternion constructor
        spineBone_->node_->SetRotation(Quaternion(localPitch * M_RADTODEG, localYaw * M_RADTODEG, 0.0f));

        // Log every 30 frames to avoid spam
        static int frameCount = 0;
        if (++frameCount % 30 == 0) {
            URHO3D_LOGINFOF("AIM: yaw=%.1f deg, pitch=%.1f deg", localYaw * M_RADTODEG, localPitch * M_RADTODEG);
        }
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
    Bone* spineBone_ = nullptr;
};

URHO3D_DEFINE_APPLICATION_MAIN(FightScene)