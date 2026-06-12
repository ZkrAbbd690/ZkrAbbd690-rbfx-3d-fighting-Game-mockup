# 🎮 Kilo-Code Parameter Mapping Guide
## Visual Behavior → Code Parameters

This guide maps **on-screen visual behaviors** to **exact code parameters** for real-time adjustment in both TPS and SIDE modes.

---

# 📍 TPS MODE PARAMETERS

## 🏃 Movement Speed

### 1. **How Fast Fighter Accelerates**
- **On-Screen:** Fighter speeds up when pressing WASD
- **File:** `Fighter.h`
- **Line:** ~86
- **Parameter:** `TPS_MOVE_FORCE`
- **Current:** `400.0f`
- **Range:** `200.0f - 800.0f`
- **Effect:**
  - ⬆️ **Increase:** Faster acceleration, more responsive
  - ⬇️ **Decrease:** Slower acceleration, heavier feel

```cpp
static constexpr float TPS_MOVE_FORCE = 400.0f;  // ← ADJUST THIS
```

---

### 2. **Maximum Running Speed**
- **On-Screen:** Fastest the fighter can run
- **File:** `Fighter.h`
- **Line:** ~79
- **Parameter:** `TPS_MAX_SPEED`
- **Current:** `5.0f`
- **Range:** `2.0f - 10.0f`
- **Effect:**
  - ⬆️ **Increase:** Fighter runs faster (top speed)
  - ⬇️ **Decrease:** Fighter runs slower (top speed)

```cpp
static constexpr float TPS_MAX_SPEED = 5.0f;  // ← ADJUST THIS
```

---

### 3. **Air Control Strength**
- **On-Screen:** How much fighter can move while jumping/falling
- **File:** `Fighter.h`
- **Line:** ~87
- **Parameter:** `TPS_AIR_FORCE`
- **Current:** `100.0f`
- **Range:** `50.0f - 200.0f`
- **Effect:**
  - ⬆️ **Increase:** More mid-air control (arcade feel)
  - ⬇️ **Decrease:** Less mid-air control (realistic)

```cpp
static constexpr float TPS_AIR_FORCE = 100.0f;  // ← ADJUST THIS
```

---

## 🖱️ Mouse/Camera Control

### 4. **Mouse Sensitivity**
- **On-Screen:** How fast fighter rotates when moving mouse
- **File:** `Fighter.h`
- **Line:** ~81
- **Parameter:** `TPS_MOUSE_SENSITIVITY`
- **Current:** `0.20f`
- **Range:** `0.05f - 0.50f`
- **Effect:**
  - ⬆️ **Increase:** Faster rotation per pixel
  - ⬇️ **Decrease:** Slower rotation per pixel

```cpp
static constexpr float TPS_MOUSE_SENSITIVITY = 0.20f;  // ← ADJUST THIS
```

---

### 5. **Rotation Limits (Left)**
- **On-Screen:** How far fighter can turn left
- **File:** `Fighter.h`
- **Line:** ~83
- **Parameter:** `TPS_YAW_MAX_LEFT`
- **Current:** `-180.0f`
- **Range:** `-180.0f to -30.0f`
- **Effect:**
  - ⬆️ **Increase (toward 0):** Less left rotation
  - ⬇️ **Decrease (toward -180):** More left rotation

```cpp
static constexpr float TPS_YAW_MAX_LEFT = -180.0f;  // ← ADJUST THIS
```

---

### 6. **Rotation Limits (Right)**
- **On-Screen:** How far fighter can turn right
- **File:** `Fighter.h`
- **Line:** ~82
- **Parameter:** `TPS_YAW_MAX_RIGHT`
- **Current:** `180.0f`
- **Range:** `30.0f - 180.0f`
- **Effect:**
  - ⬆️ **Increase (toward 180):** More right rotation
  - ⬇️ **Decrease (toward 0):** Less right rotation

```cpp
static constexpr float TPS_YAW_MAX_RIGHT = 180.0f;  // ← ADJUST THIS
```

---

## ⏱️ Physics Feel

### 7. **Stopping Distance (Damping)**
- **On-Screen:** How quickly fighter stops after releasing keys
- **File:** `Fighter.cpp`
- **Line:** ~61 (in SetupPhysics)
- **Parameter:** `SetLinearDamping()`
- **Current:** `0.5f`
- **Range:** `0.0f - 2.0f`
- **Effect:**
  - ⬆️ **Increase:** Stops faster (ice skating = 0.0, sticky = 2.0)
  - ⬇️ **Decrease:** Slides more after stopping

```cpp
rigidBody_->SetLinearDamping(0.5f);  // ← ADJUST THIS
```

---

### 8. **Coyote Time (Grace Period)**
- **On-Screen:** How long after leaving ground can still move
- **File:** `Fighter.h`
- **Line:** ~89
- **Parameter:** `IN_AIR_THRESHOLD`
- **Current:** `0.3f`
- **Range:** `0.1f - 0.5f`
- **Effect:**
  - ⬆️ **Increase:** Longer grace period (arcade platformer)
  - ⬇️ **Decrease:** Shorter grace period (realistic)

```cpp
static constexpr float IN_AIR_THRESHOLD = 0.3f;  // ← ADJUST THIS (seconds)
```

---

# 📍 SIDE MODE PARAMETERS

## 🏃 Movement Speed

### 9. **Horizontal Movement Force (A/D)**
- **On-Screen:** How fast fighter accelerates left/right
- **File:** `Fighter.h`
- **Line:** ~88
- **Parameter:** `SIDE_MOVE_FORCE`
- **Current:** `300.0f`
- **Range:** `150.0f - 600.0f`
- **Effect:**
  - ⬆️ **Increase:** Faster left/right acceleration
  - ⬇️ **Decrease:** Slower, heavier feel

```cpp
static constexpr float SIDE_MOVE_FORCE = 300.0f;  // ← ADJUST THIS
```

---

### 10. **Maximum Side Speed**
- **On-Screen:** Fastest fighter can move left/right
- **File:** `Fighter.h`
- **Line:** ~80
- **Parameter:** `SIDE_MAX_SPEED`
- **Current:** `2.5f`
- **Range:** `1.0f - 5.0f`
- **Effect:**
  - ⬆️ **Increase:** Faster max horizontal speed
  - ⬇️ **Decrease:** Slower max horizontal speed

```cpp
static constexpr float SIDE_MAX_SPEED = 2.5f;  // ← ADJUST THIS
```

---

### 11. **Return-to-Center Spring Strength**
- **On-Screen:** How strongly fighter pulls back to center when idle
- **File:** `Fighter.cpp`
- **Line:** ~333 (in SIDE MODE section)
- **Parameter:** Spring force multiplier
- **Current:** `200.0f`
- **Range:** `50.0f - 500.0f`
- **Effect:**
  - ⬆️ **Increase:** Pulls to center faster
  - ⬇️ **Decrease:** Pulls to center slower

```cpp
moveForce = xDiff * 200.0f;  // ← ADJUST THIS (spring force)
```

---

### 12. **Center Dead Zone**
- **On-Screen:** How close to center before stopping pull
- **File:** `Fighter.cpp`
- **Line:** ~332 (in SIDE MODE section)
- **Parameter:** Dead zone threshold
- **Current:** `0.05f`
- **Range:** `0.01f - 0.2f`
- **Effect:**
  - ⬆️ **Increase:** Larger "center zone" (less precise)
  - ⬇️ **Decrease:** Smaller "center zone" (more precise)

```cpp
if (Abs(xDiff) > 0.05f) {  // ← ADJUST THIS (dead zone)
```

---

# 🧟 ZOMBIE PARAMETERS

## 🏃 Movement

### 13. **Zombie Walk Force**
- **On-Screen:** How fast zombie accelerates toward fighter
- **File:** `Zombie.h`
- **Line:** ~75
- **Parameter:** `WALK_FORCE`
- **Current:** `200.0f`
- **Range:** `100.0f - 2000.0f`
- **Effect:**
  - ⬆️ **Increase:** Zombie catches up faster
  - ⬇️ **Decrease:** Zombie slower, easier to evade

```cpp
static constexpr float WALK_FORCE = 200.0f;  // ← ADJUST THIS
```

---

### 14. **Zombie Max Speed**
- **On-Screen:** Fastest zombie can move
- **File:** `Zombie.h`
- **Line:** ~77
- **Parameter:** `WALK_SPEED`
- **Current:** `1.8f`
- **Range:** `0.5f - 20.0f`
- **Effect:**
  - ⬆️ **Increase:** Zombie runs faster
  - ⬇️ **Decrease:** Zombie shambles slower

```cpp
static constexpr float WALK_SPEED = 1.8f;  // ← ADJUST THIS
```

---

### 15. **Zombie Contact Distance (Neck Bite Trigger)**
- **On-Screen:** How close zombie must be to trigger bite
- **File:** `Zombie.h`
- **Line:** ~78
- **Parameter:** `STOP_THRESHOLD`
- **Current:** `0.94f`
- **Range:** `0.5f - 2.0f`
- **Effect:**
  - ⬆️ **Increase:** Zombie stops farther away
  - ⬇️ **Decrease:** Zombie gets closer before stopping

```cpp
static constexpr float STOP_THRESHOLD = 0.94f;  // ← ADJUST THIS
```

---

### 16. **Zombie Animation Speed**
- **On-Screen:** How fast zombie animation plays
- **File:** `Zombie.cpp`
- **Line:** ~175 (in UpdateAnimation)
- **Parameter:** Animation speed multiplier
- **Current:** `12.5f`
- **Range:** `5.0f - 150.0f`
- **Effect:**
  - ⬆️ **Increase:** Animation plays faster
  - ⬇️ **Decrease:** Animation plays slower

```cpp
float animSpeed = (speed / WALK_SPEED) * 12.5f;  // ← ADJUST THIS
```

---

# 📷 CAMERA PARAMETERS

## 🎥 TPS Camera

### 17. **Camera Distance**
- **On-Screen:** How far camera is behind fighter
- **File:** `CameraController.h`
- **Line:** ~34 (private members)
- **Parameter:** `tpsDistance_`
- **Current:** `3.5f`
- **Range:** `1.5f - 10.0f`
- **Effect:**
  - ⬆️ **Increase:** Camera farther back (wider view)
  - ⬇️ **Decrease:** Camera closer (tighter view)

```cpp
float tpsDistance_ = 3.5f;  // ← ADJUST THIS
```

---

### 18. **Camera Height**
- **On-Screen:** How high camera is above fighter
- **File:** `CameraController.h`
- **Line:** ~35
- **Parameter:** `tpsHeight_`
- **Current:** `1.5f`
- **Range:** `0.5f - 3.0f`
- **Effect:**
  - ⬆️ **Increase:** Camera higher (top-down view)
  - ⬇️ **Decrease:** Camera lower (eye-level view)

```cpp
float tpsHeight_ = 1.5f;  // ← ADJUST THIS
```

---

### 19. **Camera FOV**
- **On-Screen:** Field of view (wide angle vs zoom)
- **File:** `CameraController.h`
- **Line:** ~33
- **Parameter:** `tpsFov_`
- **Current:** `60.0f`
- **Range:** `45.0f - 90.0f`
- **Effect:**
  - ⬆️ **Increase:** Wider view (fish-eye)
  - ⬇️ **Decrease:** Narrower view (zoom)

```cpp
float tpsFov_ = 60.0f;  // ← ADJUST THIS
```

---

### 20. **Q/E Orbit Speed**
- **On-Screen:** How fast Q/E keys rotate camera
- **File:** `CameraController.h`
- **Line:** ~37
- **Parameter:** `tpsOrbitSpeed_`
- **Current:** `90.0f`
- **Range:** `30.0f - 180.0f`
- **Effect:**
  - ⬆️ **Increase:** Faster orbit rotation
  - ⬇️ **Decrease:** Slower orbit rotation

```cpp
float tpsOrbitSpeed_ = 90.0f;  // ← ADJUST THIS (degrees/sec)
```

---

## 🎥 SIDE Camera

### 21. **Side Camera Distance**
- **On-Screen:** How far camera is to the right
- **File:** `CameraController.h`
- **Line:** ~41
- **Parameter:** `sideDistance_`
- **Current:** `5.0f`
- **Range:** `3.0f - 10.0f`
- **Effect:**
  - ⬆️ **Increase:** Camera farther right
  - ⬇️ **Decrease:** Camera closer

```cpp
float sideDistance_ = 5.0f;  // ← ADJUST THIS
```

---

# ⚔️ COMBAT PARAMETERS

### 22. **Attack Lunge Distance**
- **On-Screen:** How far fighter lunges during attack
- **File:** `CombatSystem.cpp`
- **Line:** ~218 (HandleManualAttack)
- **Parameter:** Lunge calculation
- **Current:** `dist - 0.9f + 0.01f`
- **Range:** `0.5f - 1.5f` offset
- **Effect:**
  - ⬆️ **Increase offset:** Shorter lunge
  - ⬇️ **Decrease offset:** Longer lunge

```cpp
float step = dist - 0.9f + 0.01f;  // ← ADJUST 0.9f (lunge distance)
```

---

### 23. **Attack Lunge Speed**
- **On-Screen:** How fast fighter lunges
- **File:** `Fighter.cpp`
- **Line:** ~300 (lunge force)
- **Parameter:** Lunge force
- **Current:** `800.0f`
- **Range:** `400.0f - 1600.0f`
- **Effect:**
  - ⬆️ **Increase:** Faster lunge
  - ⬇️ **Decrease:** Slower lunge

```cpp
rigidBody_->ApplyForce(toTarget * 800.0f);  // ← ADJUST THIS
```

---

### 24. **Neck Bite Delay**
- **On-Screen:** Time before neck bite triggers in SIDE mode
- **File:** `CombatSystem.cpp`
- **Line:** ~145 (UpdateCombat)
- **Parameter:** Timer threshold
- **Current:** `2.0f`
- **Range:** `1.0f - 5.0f`
- **Effect:**
  - ⬆️ **Increase:** Longer wait before bite
  - ⬇️ **Decrease:** Shorter wait before bite

```cpp
if (biteDelayTimer_ > 2.0f)  // ← ADJUST THIS (seconds)
```

---

# 🔄 MOTION DIRECTION INVERTERS

## ⚠️ ADVANCED: Flip Movement Axes

These parameters **invert** the direction of movement on specific axes. Useful if your world coordinates are flipped or you need reversed controls.

---

### 27. **TPS Forward/Backward Direction**
- **On-Screen:** W/S moves backward instead of forward (inverted)
- **File:** `Fighter.cpp`
- **Line:** ~414-415 (TPS section)
- **Parameter:** Sign multipliers in world direction calculation
- **Current:** `+moveZ`
- **Invert:** `-moveZ`
- **Effect:**
  - ⬆️ **Normal:** W = forward, S = backward
  - ⬇️ **Inverted:** W = backward, S = forward

```cpp
// NORMAL (current):
float worldX = moveX * cosY + moveZ * sinY;
float worldZ = moveZ * cosY - moveX * sinY;

// INVERTED FORWARD/BACK (flip moveZ signs):
float worldX = moveX * cosY - moveZ * sinY;  // ← Changed + to -
float worldZ = -moveZ * cosY - moveX * sinY; // ← Changed moveZ to -moveZ
```

---

### 28. **TPS Strafe Left/Right Direction**
- **On-Screen:** A/D moves right instead of left (inverted)
- **File:** `Fighter.cpp`
- **Line:** ~414-415 (TPS section)
- **Parameter:** Sign multipliers for moveX
- **Current:** `+moveX`
- **Invert:** `-moveX`
- **Effect:**
  - ⬆️ **Normal:** A = left, D = right
  - ⬇️ **Inverted:** A = right, D = left

```cpp
// NORMAL (current):
float worldX = moveX * cosY + moveZ * sinY;
float worldZ = moveZ * cosY - moveX * sinY;

// INVERTED STRAFE (flip moveX signs):
float worldX = -moveX * cosY + moveZ * sinY;  // ← Changed moveX to -moveX
float worldZ = moveZ * cosY + moveX * sinY;   // ← Changed - to +
```

---

### 29. **TPS X-Axis World Direction**
- **On-Screen:** All movement along X axis reversed
- **File:** `Fighter.cpp`
- **Line:** ~422, 426 (force application)
- **Parameter:** worldX sign in ApplyForce
- **Current:** `worldX * TPS_MOVE_FORCE`
- **Invert:** `-worldX * TPS_MOVE_FORCE`
- **Effect:**
  - ⬆️ **Normal:** Calculated worldX direction
  - ⬇️ **Inverted:** Opposite worldX direction

```cpp
// NORMAL (current):
rigidBody_->ApplyForce(Vector3(worldX * TPS_MOVE_FORCE, 0.0f, worldZ * TPS_MOVE_FORCE));

// INVERTED X-AXIS:
rigidBody_->ApplyForce(Vector3(-worldX * TPS_MOVE_FORCE, 0.0f, worldZ * TPS_MOVE_FORCE));
//                              ↑ Added negative sign
```

---

### 30. **TPS Z-Axis World Direction**
- **On-Screen:** All movement along Z axis reversed
- **File:** `Fighter.cpp`
- **Line:** ~422, 426 (force application)
- **Parameter:** worldZ sign in ApplyForce
- **Current:** `worldZ * TPS_MOVE_FORCE`
- **Invert:** `-worldZ * TPS_MOVE_FORCE`
- **Effect:**
  - ⬆️ **Normal:** Calculated worldZ direction
  - ⬇️ **Inverted:** Opposite worldZ direction

```cpp
// NORMAL (current):
rigidBody_->ApplyForce(Vector3(worldX * TPS_MOVE_FORCE, 0.0f, worldZ * TPS_MOVE_FORCE));

// INVERTED Z-AXIS:
rigidBody_->ApplyForce(Vector3(worldX * TPS_MOVE_FORCE, 0.0f, -worldZ * TPS_MOVE_FORCE));
//                                                              ↑ Added negative sign
```

---

### 31. **SIDE Left/Right Direction**
- **On-Screen:** A moves right, D moves left (inverted)
- **File:** `Fighter.cpp`
- **Line:** ~326-327 (SIDE section)
- **Parameter:** Force sign
- **Current:** `left = -SIDE_MOVE_FORCE, right = +SIDE_MOVE_FORCE`
- **Invert:** Swap signs
- **Effect:**
  - ⬆️ **Normal:** A = left (-X), D = right (+X)
  - ⬇️ **Inverted:** A = right (+X), D = left (-X)

```cpp
// NORMAL (current):
if (controls_.left) moveForce = -SIDE_MOVE_FORCE;  // Negative = left
if (controls_.right) moveForce = SIDE_MOVE_FORCE;  // Positive = right

// INVERTED SIDE:
if (controls_.left) moveForce = SIDE_MOVE_FORCE;   // ← Swapped signs
if (controls_.right) moveForce = -SIDE_MOVE_FORCE; // ← Swapped signs
```

---

### 32. **SIDE X-Axis World Direction**
- **On-Screen:** Entire SIDE mode movement direction flipped
- **File:** `Fighter.cpp`
- **Line:** ~341, 345 (force application)
- **Parameter:** moveForce sign in ApplyForce
- **Current:** `moveForce`
- **Invert:** `-moveForce`
- **Effect:**
  - ⬆️ **Normal:** moveForce direction
  - ⬇️ **Inverted:** Opposite moveForce direction

```cpp
// NORMAL (current):
rigidBody_->ApplyForce(Vector3(moveForce, 0.0f, 0.0f));

// INVERTED SIDE X-AXIS:
rigidBody_->ApplyForce(Vector3(-moveForce, 0.0f, 0.0f));
//                              ↑ Added negative sign
```

---

### 33. **Zombie X-Axis Walk Direction**
- **On-Screen:** Zombie walks backward (away from fighter)
- **File:** `Zombie.cpp`
- **Line:** ~154, 158 (UpdateMovement)
- **Parameter:** WALK_FORCE sign
- **Current:** `+WALK_FORCE`
- **Invert:** `-WALK_FORCE`
- **Effect:**
  - ⬆️ **Normal:** Zombie walks toward fighter (+X)
  - ⬇️ **Inverted:** Zombie walks away from fighter (-X)

```cpp
// NORMAL (current):
rigidBody_->ApplyForce(Vector3(WALK_FORCE, 0.0f, 0.0f));

// INVERTED ZOMBIE DIRECTION:
rigidBody_->ApplyForce(Vector3(-WALK_FORCE, 0.0f, 0.0f));
//                              ↑ Added negative sign
```

---

### 34. **Mouse Rotation Direction**
- **On-Screen:** Mouse left rotates right (inverted)
- **File:** `Fighter.h`
- **Line:** ~81 (sensitivity constant)
- **Parameter:** TPS_MOUSE_SENSITIVITY sign
- **Current:** `+0.20f`
- **Invert:** `-0.20f`
- **Effect:**
  - ⬆️ **Normal:** Move mouse right → turn right
  - ⬇️ **Inverted:** Move mouse right → turn left

```cpp
// NORMAL (current):
static constexpr float TPS_MOUSE_SENSITIVITY = 0.20f;

// INVERTED MOUSE:
static constexpr float TPS_MOUSE_SENSITIVITY = -0.20f;  // ← Negative
```

**OR** invert in main.cpp input reading:
```cpp
// In main.cpp (alternative method):
fighterComponent->controls_.yaw -= (float)mouseMoveX * sensitivity;  // ← Changed + to -
```

---

### 35. **Camera Orbit Direction (Q/E)**
- **On-Screen:** Q orbits right instead of left
- **File:** `CameraController.cpp`
- **Line:** ~117-118 (UpdateTPS)
- **Parameter:** tpsOrbitOffset_ increment sign
- **Current:** `Q = +speed, E = -speed`
- **Invert:** Swap signs
- **Effect:**
  - ⬆️ **Normal:** Q = left orbit, E = right orbit
  - ⬇️ **Inverted:** Q = right orbit, E = left orbit

```cpp
// NORMAL (current):
if (input_->GetKeyDown(KEY_Q)) tpsOrbitOffset_ += tpsOrbitSpeed_ * timeStep;
if (input_->GetKeyDown(KEY_E)) tpsOrbitOffset_ -= tpsOrbitSpeed_ * timeStep;

// INVERTED ORBIT:
if (input_->GetKeyDown(KEY_Q)) tpsOrbitOffset_ -= tpsOrbitSpeed_ * timeStep;  // Swapped
if (input_->GetKeyDown(KEY_E)) tpsOrbitOffset_ += tpsOrbitSpeed_ * timeStep;  // Swapped
```

---

## 🎯 DIRECTION INVERSION MATRIX

Quick reference for all invertible axes:

| Motion Type | Axis | File | Line | Invert By | Effect |
|-------------|------|------|------|-----------|--------|
| **TPS Forward/Back** | Z local | Fighter.cpp | 414-415 | Flip moveZ signs | W↔S swap |
| **TPS Strafe** | X local | Fighter.cpp | 414-415 | Flip moveX signs | A↔D swap |
| **TPS World X** | X world | Fighter.cpp | 422/426 | Add - to worldX | Flip X axis |
| **TPS World Z** | Z world | Fighter.cpp | 422/426 | Add - to worldZ | Flip Z axis |
| **SIDE Left/Right** | X control | Fighter.cpp | 326-327 | Swap -/+ signs | A↔D swap |
| **SIDE World X** | X world | Fighter.cpp | 341/345 | Add - to moveForce | Flip X axis |
| **Zombie Walk** | X world | Zombie.cpp | 154/158 | Add - to WALK_FORCE | Walk backward |
| **Mouse Rotation** | Yaw | Fighter.h | 81 | Negate sensitivity | Left↔Right |
| **Camera Orbit** | Q/E | CameraController.cpp | 117-118 | Swap +/- signs | Q↔E swap |

---

## 💡 COMMON INVERSION SCENARIOS

### Scenario 1: **Entire World X-Axis Flipped**
If your world has X pointing opposite direction:
```cpp
// Fighter.cpp, lines 422 & 426 (TPS mode):
rigidBody_->ApplyForce(Vector3(-worldX * TPS_MOVE_FORCE, 0.0f, worldZ * TPS_MOVE_FORCE));
//                              ↑ Add negative

// Fighter.cpp, lines 341 & 345 (SIDE mode):
rigidBody_->ApplyForce(Vector3(-moveForce, 0.0f, 0.0f));
//                              ↑ Add negative

// Zombie.cpp, lines 154 & 158:
rigidBody_->ApplyForce(Vector3(-WALK_FORCE, 0.0f, 0.0f));
//                              ↑ Add negative
```

---

### Scenario 2: **Camera Rotation Feels Wrong**
Invert mouse sensitivity:
```cpp
// Fighter.h, line 81:
static constexpr float TPS_MOUSE_SENSITIVITY = -0.20f;  // ← Negative
```

---

### Scenario 3: **Controls Feel Backwards**
Swap WASD mappings in direction calculation:
```cpp
// Fighter.cpp, lines 414-415:
// Instead of: worldX = moveX * cosY + moveZ * sinY
float worldX = -moveX * cosY - moveZ * sinY;  // Flip both
float worldZ = -moveZ * cosY + moveX * sinY;  // Flip both
```

---

## 📊 UPDATED PARAMETER COUNT

**35 Total Parameters:**
- Original 26 parameters (speeds, forces, camera, etc.)
- **+9 Direction Inverters** (axis flipping, control swap)

---

# 🎨 VISUAL PARAMETERS

### 25. **Blood Disc Size**
- **On-Screen:** Size of blood wound decals
- **File:** `CombatSystem.cpp`
- **Line:** ~92 (SpawnBloodDisc)
- **Parameter:** Scale multiplier
- **Current:** `0.08f` base
- **Range:** `0.04f - 0.20f`
- **Effect:**
  - ⬆️ **Increase:** Larger blood wounds
  - ⬇️ **Decrease:** Smaller blood wounds

```cpp
woundDisc->SetWorldScale(Vector3(0.08f, 0.001f, 0.08f) * bloodScale_);
// ← ADJUST 0.08f (base size)
```

---

### 26. **Blood Depth Offset**
- **On-Screen:** How deep blood sits in skin
- **File:** `CombatSystem.cpp`
- **Line:** ~86 (initial offset)
- **Parameter:** `bloodOffset_`
- **Current:** `-0.02f`
- **Range:** `-0.05f - 0.01f`
- **Effect:**
  - ⬆️ **Increase (toward 0):** Blood sticks out more
  - ⬇️ **Decrease (toward -0.05):** Blood sinks deeper

```cpp
bloodOffset_ = -0.02f;  // ← ADJUST THIS
```

---

# 📊 KILO-CODE IMPLEMENTATION TABLE

## Quick Reference for Visual Editor

| Parameter | File | Line | Variable | Min | Max | Default |
|-----------|------|------|----------|-----|-----|---------|
| **TPS Acceleration** | Fighter.h | 86 | TPS_MOVE_FORCE | 200 | 800 | 400 |
| **TPS Max Speed** | Fighter.h | 79 | TPS_MAX_SPEED | 2.0 | 10.0 | 5.0 |
| **TPS Air Control** | Fighter.h | 87 | TPS_AIR_FORCE | 50 | 200 | 100 |
| **Mouse Sensitivity** | Fighter.h | 81 | TPS_MOUSE_SENSITIVITY | 0.05 | 0.50 | 0.20 |
| **Rotation Left** | Fighter.h | 83 | TPS_YAW_MAX_LEFT | -180 | -30 | -180 |
| **Rotation Right** | Fighter.h | 82 | TPS_YAW_MAX_RIGHT | 30 | 180 | 180 |
| **Damping** | Fighter.cpp | 61 | SetLinearDamping() | 0.0 | 2.0 | 0.5 |
| **Coyote Time** | Fighter.h | 89 | IN_AIR_THRESHOLD | 0.1 | 0.5 | 0.3 |
| **SIDE Acceleration** | Fighter.h | 88 | SIDE_MOVE_FORCE | 150 | 600 | 300 |
| **SIDE Max Speed** | Fighter.h | 80 | SIDE_MAX_SPEED | 1.0 | 5.0 | 2.5 |
| **Center Spring** | Fighter.cpp | 333 | xDiff * multiplier | 50 | 500 | 200 |
| **Center Dead Zone** | Fighter.cpp | 332 | Abs(xDiff) > threshold | 0.01 | 0.2 | 0.05 |
| **Zombie Force** | Zombie.h | 75 | WALK_FORCE | 100 | 2000 | 200 |
| **Zombie Speed** | Zombie.h | 77 | WALK_SPEED | 0.5 | 20.0 | 1.8 |
| **Bite Distance** | Zombie.h | 78 | STOP_THRESHOLD | 0.5 | 2.0 | 0.94 |
| **Zombie Anim Speed** | Zombie.cpp | 175 | speed multiplier | 5.0 | 150 | 12.5 |
| **Cam Distance** | CameraController.h | 34 | tpsDistance_ | 1.5 | 10.0 | 3.5 |
| **Cam Height** | CameraController.h | 35 | tpsHeight_ | 0.5 | 3.0 | 1.5 |
| **Cam FOV** | CameraController.h | 33 | tpsFov_ | 45 | 90 | 60 |
| **Orbit Speed** | CameraController.h | 37 | tpsOrbitSpeed_ | 30 | 180 | 90 |
| **Side Cam Distance** | CameraController.h | 41 | sideDistance_ | 3.0 | 10.0 | 5.0 |
| **Lunge Distance** | CombatSystem.cpp | 218 | dist - offset | 0.5 | 1.5 | 0.9 |
| **Lunge Force** | Fighter.cpp | 300 | ApplyForce multiplier | 400 | 1600 | 800 |
| **Bite Delay** | CombatSystem.cpp | 145 | timer threshold | 1.0 | 5.0 | 2.0 |
| **Blood Size** | CombatSystem.cpp | 92 | SetWorldScale base | 0.04 | 0.20 | 0.08 |
| **Blood Depth** | CombatSystem.cpp | 86 | bloodOffset_ | -0.05 | 0.01 | -0.02 |

---

# 🎛️ SUGGESTED KILO-CODE UI LAYOUT

```
┌─────────────────────────────────────────────────┐
│  FIGHTER MOVEMENT (TPS)                         │
├─────────────────────────────────────────────────┤
│  Acceleration:    [====|====] 400               │
│  Max Speed:       [====|====] 5.0               │
│  Air Control:     [====|====] 100               │
│  Mouse Sens:      [====|====] 0.20              │
│  Rotation Range:  [====|====] ±180°             │
│  Damping:         [====|====] 0.5               │
└─────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────┐
│  FIGHTER MOVEMENT (SIDE)                        │
├─────────────────────────────────────────────────┤
│  Acceleration:    [====|====] 300               │
│  Max Speed:       [====|====] 2.5               │
│  Center Spring:   [====|====] 200               │
│  Dead Zone:       [====|====] 0.05              │
└─────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────┐
│  ZOMBIE BEHAVIOR                                │
├─────────────────────────────────────────────────┤
│  Walk Force:      [====|====] 200               │
│  Max Speed:       [====|====] 1.8               │
│  Bite Distance:   [====|====] 0.94              │
│  Bite Delay:      [====|====] 2.0s              │
└─────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────┐
│  CAMERA                                         │
├─────────────────────────────────────────────────┤
│  TPS Distance:    [====|====] 3.5               │
│  TPS Height:      [====|====] 1.5               │
│  FOV:             [====|====] 60°               │
│  SIDE Distance:   [====|====] 5.0               │
└─────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────┐
│  DIRECTION INVERTERS (Checkboxes)               │
├─────────────────────────────────────────────────┤
│  ☐ Invert TPS Forward/Back                      │
│  ☐ Invert TPS Strafe                            │
│  ☐ Invert TPS World X-Axis                      │
│  ☐ Invert TPS World Z-Axis                      │
│  ☐ Invert SIDE Left/Right                       │
│  ☐ Invert SIDE World X-Axis                     │
│  ☐ Invert Zombie Walk Direction                 │
│  ☐ Invert Mouse Rotation                        │
│  ☐ Invert Camera Orbit Q/E                      │
└─────────────────────────────────────────────────┘
```

---

# 🔧 REAL-TIME MODIFICATION WORKFLOW

For **kilo-code** to modify parameters in real-time:

1. **Read current value** from file
2. **Display slider/input** with min/max range
3. **On change**, update value in memory OR write to file
4. **Hot-reload** (if supported) OR require rebuild

**Example kilo-code pseudo-code:**
```python
# Read from Fighter.h
current_speed = read_constant("Fighter.h", "TPS_MAX_SPEED")  # → 5.0

# Display slider
new_speed = slider("Max Speed", current_speed, min=2.0, max=10.0)

# Write back
if new_speed != current_speed:
    write_constant("Fighter.h", "TPS_MAX_SPEED", new_speed)
    notify("Rebuild required")
```

---

## 📊 SUMMARY

**35 Total Parameters Mapped:**
- ✅ 26 Standard adjustable values (speeds, forces, camera, etc.)
- ✅ 9 Direction inverters (axis flipping, control reversal)

**All parameters include:**
- Exact file location
- Line numbers
- Variable names
- Min/Max ranges
- Default values
- Visual effect descriptions

**Additional Resources:**
- See **DIRECTION_INVERTERS_QUICK_REF.md** for detailed inversion examples
- See **TPS_FREE_MOVEMENT_FIX.md** for recent movement improvements

---

**This mapping gives kilo-code everything it needs to visually adjust gameplay! 🎮✨**
