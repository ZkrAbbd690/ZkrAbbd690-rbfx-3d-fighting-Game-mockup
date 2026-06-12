#pragma once

#include <Urho3D/Scene/Component.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/StringUtils.h>

using namespace Urho3D;

enum SIDE { SIDE_HERO = 0, SIDE_VILLAIN = 1 };

class Combatant : public Component
{
    URHO3D_OBJECT(Combatant, Component);

public:
    Combatant(Context* context) 
        : Component(context)
        , node_(nullptr)
        , animController_(nullptr)
        , isFrozen_(false)
        , freezeTimer_(0.0f)
        , side_(0)
        , maxHealth_(100)
        , health_(100)
    {}

    virtual void Initialize(Node* node, const ea::string& prefabPath) = 0;
    virtual void Update(float timeStep) = 0;
    virtual bool CheckCollision(Combatant* other) = 0;
    virtual void HandleHit(Combatant* attacker) = 0;

    // Make SetFrozen VIRTUAL so Zombie can override it
    virtual void SetFrozen(bool freeze, bool lockAnimation = true) = 0;

    virtual void Reset() = 0;

    // Common utility accessors
    virtual float GetForwardOffset() const { return 0.0f; } // Added for lunge tracking

    Node* GetNode() const { return node_; }
    Vector3 GetPosition() const { return node_ ? node_->GetWorldPosition() : Vector3::ZERO; }
    void SetPosition(const Vector3& pos) { if (node_) node_->SetWorldPosition(pos); }

    bool IsFrozen() const { return isFrozen_; }

protected:
    Node* node_;
    AnimationController* animController_;
    bool isFrozen_;
    float freezeTimer_;
    int side_;
    int maxHealth_;
    int health_;
};