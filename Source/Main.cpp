// rbfx Third-Person Shooter Example
// Main entry point using rbfx/Urho3D APIs

#include <rbfx/Engine.h>
#include <rbfx/Core/Context.h>
#include <rbfx/Scene/Scene.h>
#include <rbfx/Graphics/Octree.h>
#include <rbfx/Graphics/Camera.h>
#include <rbfx/Physics/PhysicsWorld.h>
#include <rbfx/Physics/RigidBody.h>
#include <rbfx/Input/Input.h>
#include <rbfx/UI/UI.h>

using namespace Urho3D;

class TPSGame : public Application
{
public:
    TPSGame(Context* context) : Application(context)
    {
    }

    void Setup() override
    {
        engineParameters_["FullScreen"] = false;
        engineParameters_["WindowResizable"] = true;
        engineParameters_["WindowTitle"] = "rbfx TPS Example";
    }

    void Start() override
    {
        // Create scene
        scene_ = new Scene(context_);
        scene_->CreateComponent<Octree>();

        // Create physics world
        auto* physicsWorld = scene_->CreateComponent<PhysicsWorld>();

        // Create camera
        cameraNode_ = scene_->CreateChild("Camera");
        cameraNode_->SetPosition(Vector3(0, 5, -10));
        auto* camera = cameraNode_->CreateComponent<Camera>();
        camera->SetFarClip(500.0f);

        // Create player
        playerNode_ = scene_->CreateChild("Player");
        playerNode_->SetPosition(Vector3(0, 2, 0));
        auto* rigidBody = playerNode_->CreateComponent<RigidBody>();
        rigidBody->SetMass(1.0f);
        rigidBody->SetLinearDamping(0.8f);

        // Setup input
        auto* input = GetSubsystem<Input>();
        input->SetMouseVisible(true);

        // Subscribe to events
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(TPSGame, HandleUpdate));
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(TPSGame, HandleKeyDown));
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;

        float timeStep = eventData[P_TIMESTEP].GetFloat();

        // Handle player movement
        auto* input = GetSubsystem<Input>();
        if (input->IsKeyDown(KEY_W))
        {
            playerNode_->Translate(Vector3::FORWARD * 5.0f * timeStep);
        }
        if (input->IsKeyDown(KEY_S))
        {
            playerNode_->Translate(Vector3::BACK * 5.0f * timeStep);
        }
        if (input->IsKeyDown(KEY_A))
        {
            playerNode_->Translate(Vector3::LEFT * 5.0f * timeStep);
        }
        if (input->IsKeyDown(KEY_D))
        {
            playerNode_->Translate(Vector3::RIGHT * 5.0f * timeStep);
        }

        // Update camera to follow player
        if (cameraNode_ && playerNode_)
        {
            Vector3 playerPos = playerNode_->GetPosition();
            cameraNode_->SetPosition(playerPos + Vector3(0, 5, -10));
            cameraNode_->LookAt(playerPos);
        }
    }

    void HandleKeyDown(StringHash eventType, VariantMap& eventData)
    {
        using namespace KeyDown;

        int key = eventData[P_KEY].GetInt();

        if (key == KEY_ESCAPE)
        {
            engine_->Exit();
        }
    }

private:
    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    SharedPtr<Node> playerNode_;
};

URHO3D_DEFINE_APPLICATION(TPSGame)
