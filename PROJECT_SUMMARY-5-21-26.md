# Kilo-Code Battle Arena — Complete Project Summary
## As of 2026-05-20 / 2026-05-21

---

## 1. PROJECT OVERVIEW

A 3D arena combat game built on **rbfx** (Urho3D Rebel Fork) using MinGW 64-bit.
Two characters face off: a **Fighter** (Axe Man) and a **Zombie** (runner).

**Core loop:**
1. Zombie runs from left (`x = -4.5`) toward Fighter (`x = 0.59`)
2. Zombie stops body-to-body at Fighter (`0.85f` distance)
3. Player presses **SPACE** — Fighter swings axe
4. If swing connects during active window (`0.20s–0.60s`), Zombie freezes (dead)
5. Press **SPACE** again to reset / spawn next runner

---

## 2. ARCHITECTURE

```
Combatant (abstract base)
├── Fighter (hero, right side, faces left)
└── Zombie (villain, left side, runs right)

CombatSystem (Component)
├── arbitrates encounter
├── handles input (SPACE)
└── manages arena state (hit-frozen, reset)
```

### File Structure
| File | Purpose |
|------|---------|
| `Combatant.h` | Abstract base: node, animController, health, side, freeze state |
| `Fighter.h/.cpp` | Hero: axe swing animation, mid-swing detection, weapon reparenting |
| `Zombie.h/.cpp` | Villain: run-in, contact freeze, treadmill root-motion cancel, hit reaction |
| `CombatSystem.h/.cpp` | Master arbitrator: hit validation, input routing, reset |
| `main.cpp` | Engine init, scene setup, lighting, camera, main loop |

---

## 3. CURRENT STATE (WORKING)

### Fighter (Axe Man)
- **Position:** `x = 0.59, y = 0, z = 1.6`
- **Faces:** Left (rotation Y = -90°)
- **Scale:** `5.0f / 9.0f`
- **Prefab:** `test.glb.d/Prefab.prefab` (MaleBruteA model)
- **Animation:** `test.glb.d/Animations/mixamo.com.ani`
- **Weapon:** BattleAxe reparented to RightHand bone at runtime
- **Swing window:** `0.20s` to `0.60s` of animation (IsMidSwing)
- **Spam guard:** Cannot start new swing while already swinging
- **Movement:** Attack animation causes slight forward lunge (desired — no Blender edit needed)

### Zombie (Runner)
- **Start position:** `x = -4.5, y = 0, z = 1.6`
- **Faces:** Right (rotation Y = 90°)
- **Scale:** `75.0f` (mesh node)
- **Prefab:** `firing-running-didn'twork/test_out/test.gltf.d/Prefab.prefab`
- **Animation:** `firing-running-didn'twork/test_out/test.gltf.d/Animations/mixamo.com.ani`
- **Speed:** `3.0f` units/sec
- **Treadmill:** Root-motion cancellation via `mixamorig:Hips` bone tracking
- **Stop distance:** `0.85f` from Fighter (body-to-body, no overlap)
- **Hit reaction:** Head tilts back `-15°` on impact, animation stops

### Combat Flow
1. Zombie runs in → stops at `fighter.x - 0.85f`
2. Legs keep animating (treadmill) — zombie runs in place
3. Player presses SPACE → Fighter starts attack
4. `0.00s–0.20s`: Wind-up — no damage
5. `0.20s–0.60s`: **Active swing window** — axe can connect
6. If `deltaX < 1.1f` during window → **HIT** → Zombie freezes
7. Press SPACE again → reset both, zombie respawns at `-4.5`

---

## 4. TECHNICAL DETAILS

### Root-Motion Cancellation (Treadmill)
```cpp
// Every frame in Zombie::Update()
Vector3 rootWorld = rootBone_->node_->GetWorldPosition();
Vector3 nodeWorld = node_->GetWorldPosition();
Vector3 desired(nodeWorld.x_, nodeWorld.y_ + baselineY_, nodeWorld.z_);
Vector3 delta = desired - rootWorld;
meshNode_->Translate(delta, TS_WORLD);  // Cancel root motion
```

### Hit Validation
```cpp
// In CombatSystem::UpdateCombat()
if (fighter_->IsMidSwing() &&           // 0.20s <= swingTimer <= 0.60s
    !zombie_->IsFrozen() &&             // Not already dead
    zombie_->IsContactFrozen() &&       // Zombie is at stopping distance
    !arenaHitFrozen_)                   // Not already registered
{
    float deltaX = abs(fighterPos.x - zombiePos.x);
    if (deltaX < 1.1f)                  // Axe range
    {
        zombie_->HandleHit(fighter_);   // FREEZE
    }
}
```

### Dynamic Boundary
```cpp
// CheckCollision recalculates boundary every frame from Fighter's LIVE position
float boundaryX = targetPos.x_ - solidStopThreshold_;  // 0.85f
if (myPos.x_ >= boundaryX) { /* clamp and freeze contact */ }
```

---

## 5. KNOWN ISSUES & FIXED BUGS (HISTORY)

| Issue | Status | Fix |
|-------|--------|-----|
| Zombie falls through ground (no PhysicsWorld) | ✅ Fixed | Added PhysicsWorld, disabled gravity, used direct translation |
| Zombie merging with fighter | ✅ Fixed | Hard boundary clamp, dynamic recalculation |
| Zombie sliding without animation | ✅ Fixed | Animation speed matched to movement (`1.0x`) |
| Zombie floating/spinning during neck bite | ✅ Fixed | Removed neck bite system entirely |
| Zombie freezes head-down | ✅ Fixed | Head tilts back on impact, not forward |
| Zombie passes through on 2nd run | ✅ Fixed | `CheckCollision()` runs every frame, not just once |
| `TU_DIFFUSE` not found in rbfx | ✅ Fixed | `#ifndef TU_DIFFUSE #define TU_DIFFUSE 0` |
| `Matrix3` constructor differs in rbfx | ✅ Fixed | 9 float params instead of 3 Vector3s |
| `AddDecal` signature differs | ✅ Fixed | Removed extra `nearClip` param |
| `Image` header path wrong | ✅ Fixed | `Resource/Image.h` not `Graphics/Image.h` |
| `SetFrozen` not virtual in Combatant | ✅ Fixed | Made pure virtual `= 0` |
| Member shadowing (Zombie::node_ vs Combatant::node_) | ✅ Fixed | Removed duplicates from Zombie.h |
| DecalSet GPU crash (`IsLocked()`) | ✅ Fixed | Removed DecalSet entirely |
| Neck bite infinite loop | ✅ Fixed | Removed neck bite system |
| Excessive debug logging (10 FPS) | ✅ Fixed | Removed all debug logging |

---

## 6. REMOVED FEATURES

| Feature | Why Removed | Future Plan |
|---------|-------------|-------------|
| Neck bite animation | Too buggy, caused infinite loops | Will be replaced with Blender-made bite animation |
| Scar decals (DecalSet) | GPU crash in rbfx | Will use temporary mesh or different approach |
| RigidBody physics | Unnecessary complexity, caused merging | Direct translation + boundary clamp is sufficient |
| Blood FX / particles | Not yet implemented | Planned |
| Hit sound | Not yet implemented | Planned |

---

## 7. NEXT STEPS (USER'S PLANS)

1. **Blender animation for zombie neck bite** — when zombie reaches fighter without being hit, play a bite animation where zombie kills fighter
2. **Fighter death animation** — when bitten, fighter dies
3. **Blood FX / particle effects** — on axe hit
4. **Hit sound** — `SpawnSound` or `PlaySound`
5. **Scar decals** — safer approach than DecalSet (temporary colored plane mesh)
6. **Expanded HUD / debug text**
7. **Further input handling refinements**

---

## 8. KEY CONSTANTS & TUNABLES

```cpp
// Zombie
startPosition_      = Vector3(-4.5f, 0.0f, 1.6f);
solidStopThreshold_ = 0.85f;        // Stopping distance from fighter
moveSpeed           = 3.0f;         // Units/sec
animSpeed           = 1.0f;         // Animation playback speed

// Fighter
attackAnimPath_     = "test.glb.d/Animations/mixamo.com.ani";
swingWindowStart    = 0.20f;        // Seconds into anim
swingWindowEnd      = 0.60f;        // Seconds into anim

// Combat
hitRange            = 1.1f;         // Axe connection distance
```

---

## 9. ENGINE & BUILD INFO

- **Engine:** rbfx (Urho3D Rebel Fork)
- **Compiler:** MinGW 64-bit
- **Renderer:** OpenGL 4.5 (Mesa via ZINK)
- **Window:** 1152x864, borderless, VSYNC on
- **Resource path:** `Data/`

---

## 10. CRITICAL API DIFFERENCES (rbfx vs Urho3D)

| Feature | Urho3D | rbfx |
|---------|--------|------|
| `Image` header | `<Urho3D/Graphics/Image.h>` | `<Urho3D/Resource/Image.h>` |
| `TU_DIFFUSE` | Defined in `GraphicsDefs.h` | Not defined — use `0` |
| `Matrix3` constructor | `Matrix3(Vector3, Vector3, Vector3)` | `Matrix3(float, float, ...)` (9 floats) |
| `AddDecal` | 11 params + nearClip | 11 params, no nearClip |
| `AnimationParameters` | `AnimationParameters(context, path)` | `AnimationParameters(context_, path)` |

---

## 11. COMPLETE FILE LIST (Current Working Version)

### Headers
- `Combatant.h` — Pure virtual base, `SetFrozen` is pure virtual
- `Fighter.h` — Overrides all Combatant methods
- `Zombie.h` — No shadowing members, neck bite removed
- `CombatSystem.h` — Component arbitrator

### Source
- `main.cpp` — Engine init, scene, lighting, camera rig, main loop
- `Fighter.cpp` — Attack, swing detection, weapon reparenting
- `Zombie.cpp` — Run, treadmill, boundary clamp, hit reaction
- `CombatSystem.cpp` — Hit validation, input, reset

---

## 12. HOW TO BUILD

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
./bin/fighterproject
```

---

*End of summary. This project is actively developed — check latest conversation for most recent changes.*
