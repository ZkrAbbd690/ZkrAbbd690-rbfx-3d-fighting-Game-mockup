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
#include <Urho3D/UI/Font.h> // Fixed: Included explicitly so GetResource<Font> compiles!
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Core/StringUtils.h> // Fixed: For FormatString utility

#include <cstring>

using namespace Urho3D;

int main(int argc, char** argv)
{
    SharedPtr<Context> context(new Context());
    SharedPtr<Engine> engine(new Engine(context));

    StringVariantMap engineParameters;
    engineParameters[EP_WINDOW_TITLE] = "Kilo-Code Animation Diagnostic";
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

    // ---- 1. Fallback UI Diagnostic Render ----
    auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
    if (font)
    {
        auto* debugText = ui->GetRoot()->CreateChild<Text>();
        debugText->SetText("DIAGNOSTIC ACTIVE: IF YOU CAN SEE THIS, THE ENGINE WINDOW IS RENDERING OK.");
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
     zone->SetAmbientColor(Color(0.7f, 0.7f, 0.7f)); // Increased from 0.6 to 0.7 for brighter ambient shadows

     // Baseline Directional Light
     auto* lightNode = scene->CreateChild("DirectionalLight");
     auto* light = lightNode->CreateComponent<Light>();
     light->SetLightType(LIGHT_DIRECTIONAL);
     lightNode->SetDirection(Vector3(0.4f, -1.0f, 0.4f));
     light->SetBrightness(2.5f); // Increased from 2.0f to 2.5f for a crisper highlight

      // Overhead Diagnostic Light
      auto* topLightNode = scene->CreateChild("OverheadDiagnosticLight");
      auto* topLight = topLightNode->CreateComponent<Light>();
      topLight->SetLightType(LIGHT_DIRECTIONAL);
      topLightNode->SetDirection(Vector3(0.0f, -1.0f, 0.0f)); 
      topLight->SetBrightness(3.0f); // Increased to 3.0f to strongly illuminate the head

    // Camera Node configuration
    auto* cameraNode = scene->CreateChild("Camera");
    cameraNode->SetPosition(Vector3(0.0f, 0.5f, -1.5f)); // Brought lower and closer for the smaller scale
    cameraNode->LookAt(Vector3(0.0f, 0.4f, 0.0f));
    auto* camera = cameraNode->CreateComponent<Camera>();

    SharedPtr<Viewport> viewport(new Viewport(context, scene, camera));
    renderer->SetViewport(0, viewport);

    // ========================================================================
    //  CHARACTER SPARK ROUTINE (WITH PARENT PIVOT CONTROL)
    // ========================================================================
    // 1. Create an invisible parent platform node to handle scene placement
    auto* characterPivotNode = scene->CreateChild("CharacterPivot");
    
    // CONTROL WHERE THE ANIMATION STARTS FROM HERE:
    // Pushes the entire system 1.6 units back on the Z axis
    characterPivotNode->SetPosition(Vector3(-2.0f, 0.0f, 1.6f)); 
    
    // ROTATE 15 DEGREES CLOCKWISE ON Y:
    // In Urho3D's left-handed coordinate system, a positive value rotates clockwise looking down
    characterPivotNode->SetRotation(Quaternion(0.0f, 110.0f, 0.0f));

    // 2. Attach the actual character asset as a child of our pivot platform
    auto* characterNode = characterPivotNode->CreateChild("CharacterMesh");
    const ea::string assetRoot = "firing-running-didn'twork/test_out/test.gltf.d/";
    auto* prefab = cache->GetResource<PrefabResource>(assetRoot + "Prefab.prefab");

    if (prefab)
    {
        characterNode->InstantiatePrefab(prefab);
        URHO3D_LOGINFO("--- DIAGNOSTIC: Prefab node mapping hooked up ---");

        auto* animCtrl = characterNode->FindComponent<AnimationController>();
        if (animCtrl)
        {
            const ea::string animPath = assetRoot + "Animations/mixamo.com.ani";
            if (animCtrl->Play(animPath, 0, true, 0.0f))
            {
                animCtrl->SetSpeed(animPath, 0.5f); 
                URHO3D_LOGINFO("--- DIAGNOSTIC: Animation tracked to memory loop ---");
            }
        }

        // Keep the local mesh scale intact, but leave local position/rotation zeroed 
        // out so it obeys the parent pivot platform completely.
        characterNode->SetRotation(Quaternion::IDENTITY);
        characterNode->SetScale(50.0f);
        characterNode->SetPosition(Vector3::ZERO);

        // ---- DIAGNOSTIC MODEL BOUNDS READOUT ----
        auto* animatedModel = characterNode->FindComponent<AnimatedModel>();
        if (animatedModel)
        {
            animatedModel->SetUpdateInvisible(true); 
            BoundingBox box = animatedModel->GetBoundingBox();
            Vector3 center = box.Center();
            Vector3 size = box.Size();
            
            char logBuffer[256];
            snprintf(logBuffer, sizeof(logBuffer), 
                "--- MODEL BOUNDS INFO: Center=(%.3f, %.3f, %.3f) Size=(%.3f, %.3f, %.3f) ---", 
                center.x_, center.y_, center.z_, size.x_, size.y_, size.z_);
            URHO3D_LOGINFO(ea::string(logBuffer));
        }
    }
    else
    {
        URHO3D_LOGERROR("Diagnostic failed: Prefab target unreachable.");
        engine->Exit();
        return 2;
    }

    // ================================================================
    //  MAIN EXECUTION LOOP WITH RUNTIME DEBUG TOOLS
    // ================================================================
    while (!engine->IsExiting())
    {
        engine->RunFrame();

        // RUNTIME CAMERA MOVEMENT FOR HUNTING: Hold W/S to push/pull camera depth
        if (input->GetKeyDown(KEY_W))
        {
            cameraNode->Translate(Vector3::FORWARD * 0.1f);
        }
        if (input->GetKeyDown(KEY_S))
        {
            cameraNode->Translate(Vector3::BACK * 0.1f);
        }

        if (input->GetKeyPress(KEY_ESCAPE))
            engine->Exit();
    }

    renderer->SetViewport(0, nullptr);
    engine->Exit();

    return 0;
}