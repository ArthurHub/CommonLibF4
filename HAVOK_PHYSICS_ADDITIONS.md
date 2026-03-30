# Havok Physics API Additions for CommonLibF4VR

All addresses confirmed via Ghidra RE on `Fallout4VR.exe.unpacked.exe`. REL::IDs verified against `fo4_database.csv`.

## Overview

This documents the comprehensive Havok 2014 (New Physics / hknp) API additions made to CommonLibF4VR. These additions provide the first complete, open-source Havok physics interface for Fallout 4 / FO4VR modding, enabling:

- **Raycasting** — world-space ray queries with collision filtering
- **Body creation/destruction** — create custom physics bodies (hand colliders, projectiles, etc.)
- **Velocity & impulse** — push, throw, and move objects physically
- **Motion type switching** — toggle STATIC/DYNAMIC/KEYFRAMED at runtime
- **Constraint systems** — hinge, ball-socket, 6-DOF constraints between bodies
- **Contact events** — detect and respond to collisions
- **VR controller tracking** — smooth, solver-friendly body tracking via ComputeHardKeyFrame
- **Direct body/motion access** — type-safe access to internal Havok arrays

## New Headers

### `RE/Havok/hknpBody.h` (NEW)
Body descriptor struct (0x90 bytes per body). Bodies live in a flat array at hknpWorld+0x20.

Key fields:
- `+0x28` = shape pointer (hknpShape*)
- `+0x40` = userData (often TESObjectREFR*, MUST validate before dereferencing)
- `+0x68` = motionIndex (indexes into motion array, 0 = shared static)
- `+0x72` = motionPropertiesId (lower byte: 0=STATIC, 1=DYNAMIC, 2=KEYFRAMED)

Helper functions: `IsBodyDynamic()`, `IsBodyKeyframed()`, `IsBodyStatic()`

**IMPORTANT**: body+0x00 is AABB data, NOT position. Real position lives in the motion struct.

### `RE/Havok/hknpMotion.h` (NEW)
Motion data struct (0x80 bytes per motion). Motions live at hknpWorld+0xE0.

Key fields:
- `+0x00` = position (hkVector4f, Havok space)
- `+0x10` = orientation quaternion
- `+0x20` = linear velocity
- `+0x30` = angular velocity
- `+0x60` = inverse mass and inertia
- `+0x7C` = timeFactor (0 = frozen, 1 = normal)

Motion index 0 is the shared static motion (all static bodies use it).

### `RE/Havok/hknpConstraintCinfo.h` (NEW)
Constraint creation info + constraint data types. RE'd from `hknpWorld::createConstraint` at VR 0x1415469b0 and `hknpConstraint::init` at VR 0x1417e39c0.

Defines:
- **hknpConstraintCinfo** (0x18 bytes) — creation parameters: constraintData*, bodyIdA, bodyIdB, flags
- **hknpConstraint** (0x38 bytes) — runtime constraint in the world's array (world+0x128, stride 0x38)
- **hkpConstraintData** (0x18 bytes) — abstract base for constraint data (vtable-driven)
- **hkpBallAndSocketConstraintData** (0x70 bytes) — simplest constraint, pivotA at +0x30, pivotB at +0x40
- **hkpRagdollConstraintData** (0x1A0 bytes) — full ragdoll limits (cone, twist, plane)
- **ConstraintType enum** — BallAndSocket=0, Hinge=1, LimitedHinge=2, Ragdoll=7, Fixed=0xB, etc.

FO4 uses OLD Havok (hkp*) constraint data wrapped in the hknp world. Available types:
hkpBallAndSocketConstraintData, hkpHingeConstraintData, hkpLimitedHingeConstraintData,
hkpRagdollConstraintData, hkpFixedConstraintData, hkpStiffSpringConstraintData,
hkpPrismaticConstraintData, hknpBreakableConstraintData, hknpMalleableConstraintData.

VR addresses:
- BallAndSocket ctor: 0x1419af690, setPivots: 0x1419af6e0, vtable: 0x142e17a58
- Ragdoll ctor: 0x1419b1d50, vtable: 0x142e18298

### `RE/Bethesda/bhkPhysicsSystem.h` (NEW)
Bethesda's physics body group manager. RE'd from methods at VR 0x141e0c320..0x141e0c740.

Defines:
- **bhkPhysicsSystem** — groups related bodies and constraints (doors, ragdolls, etc.)
- **hknpPhysicsSystemInstance** (0x30 bytes) — runtime state with body IDs in current world
- **hknpPhysicsSystemData** — serialized template from .nif files
- **hknpPhysicsSystemConstraintDesc** (0x18 bytes) — constraint descriptor in system data

Methods (7 total, all with REL::IDs):
- `CreateInstance(bhkWorld&, hkTransformf&)` — REL::ID(1245299)
- `HasInstanceInWorld(bhkWorld&)` — REL::ID(792872)
- `GetBodyId(hknpBodyId*, int)` — REL::ID(526734)
- `ContainsBodyId(hknpBodyId)` — REL::ID(260599)
- `AddToWorld()` — REL::ID(512878)
- `ChangeWorld(bhkWorld&, hkTransformf&)` — REL::ID(153808)
- `GetMaterialCinfo(int)` — REL::ID(139822)

Key layout:
- +0x10 = hknpPhysicsSystemData* (serialized template)
- +0x18 = hknpPhysicsSystemInstance* (runtime, has bodyIds array at +0x20, count at +0x28)
- Instance+0x18 = hknpWorld* (for world validation)

### `RE/Havok/hknpContactEvent.h` (NEW)
Contact/collision event struct (0x40 bytes). Fired when bodies collide.

Key fields:
- `+0x00` = contact point (hkVector4f, world-space)
- `+0x10` = contact normal
- `+0x20` = bodyIdA
- `+0x24` = separating velocity (negative = approaching)
- `+0x28` = bodyIdB

### `RE/Havok/hknpWorld.h` (ENHANCED)
Complete hknpWorld class with 30+ methods, now including type-safe body/motion access:

**New convenience methods:**
- `GetBodyArray()` — direct access to body array (world+0x20)
- `GetBody(bodyId)` — get specific body by ID
- `GetMotionArray()` — direct access to motion array (world+0xE0)
- `GetBodyMotion(bodyId)` — get the motion for a specific body (with bounds check)
- `GetBodyPosition(bodyId)` — get world-space position from motion

**Collision Queries:**
- `CastRay(query, collector)` — REL::ID(1440166)
- `CastShape(query, shapeInfo, collector, startCollector)` — REL::ID(110287) — shape sweep (= linearCast in Havok 2014)
- `GetClosestPoints(query, transform, collector)` — REL::ID(1053489)
- `QueryAabb(query, collector)` — REL::ID(990501)

**Motion Allocation:**
- `AllocateMotion()` — allocate a motion slot (MUST call before CreateBody for non-static bodies)

**Body Management:**
- `CreateBody(cinfo, additionMode, flags)` — REL::ID(72830) — returns hknpBodyId
- `RemoveBodies(bodyIds, count, activationMode)` — REL::ID(201384)
- `DestroyBodies(bodyIds, count, activationMode)` — REL::ID(1037327)
- `CommitAddBodies()` — REL::ID(121748)

**Body Properties:**
- `SetBodyTransform(bodyId, transform, activationBehavior)` — REL::ID(367136)
- `SetBodyShape(bodyId, shape)` — REL::ID(531033)
- `SetBodyMaterial(bodyId, materialId, rebuildMode)` — REL::ID(50770)
- `SetBodyMotionProperties(bodyId, motionPropertiesId)` — REL::ID(275629) — switch STATIC/DYNAMIC/KEYFRAMED
- `GetBodyAabb(bodyId, aabbOut)` — REL::ID(249572)

**Velocity & Impulse:**
- `SetBodyLinearVelocity(bodyId, velocity)` — REL::ID(1082668)
- `SetBodyAngularVelocity(bodyId, velocity)` — REL::ID(1068862)
- `SetBodyVelocity(bodyId, linVel, angVel)` — REL::ID(868461) — both at once
- `GetBodyAngularVelocity(bodyId, velocityOut)` — REL::ID(405369)
- `ApplyBodyLinearImpulse(bodyId, impulse)` — REL::ID(654985)
- `ApplyBodyAngularImpulse(bodyId, impulse)` — REL::ID(1345477)
- `ApplyBodyImpulseAt(bodyId, impulse, point)` — REL::ID(800106) — point impulse for throw
- `ComputeHardKeyFrame(bodyId, targetPos, targetRot, deltaTime, linVelOut, angVelOut)` — REL::ID(865502) — VR controller sync. Parameter is deltaTime (NOT invDt). The function internally computes 1/dt via RCPPS+Newton-Raphson. Ghidra-confirmed 2026-03-22.

**Activation:**
- `ActivateBody(bodyId)` — REL::ID(904571)

**Constraints:**
- `CreateConstraint(cinfo)` — REL::ID(441087)
- `DestroyConstraints(constraintIds, count)` — REL::ID(334340)
- `DisableConstraint(constraintId)` — REL::ID(320495)
- `EnableConstraint(constraintId, activationMode)` — REL::ID(399921)

**Events:**
- `GetEventSignal(eventType)` — REL::ID(1029853) — event type 3 = contact events
- `GetEventSignalForBody(eventType, bodyId)` — REL::ID(497909) — per-body events

**Simulation:**
- `StepCollide(input, taskQueue, solverData)` — REL::ID(687604)
- `ShiftBroadPhase(offset, offset2, bodyIdArray)` — REL::ID(218299)
- Constructor REL::ID(864328), Destructor REL::ID(748654), GetCinfo REL::ID(624763)

**Key structural offsets (documented in comments):**
- `+0x20` = body array (hknpBody*, stride 0x90)
- `+0xE0` = motion array (stride 0x80)
- `+0x128` = constraint array (stride 0x38)
- `+0x178` = collision query dispatcher
- `+0x5C8` = simulation island manager
- `+0x648` = event dispatcher (hknpEventDispatcher*)
- `+0x690` = read/write lock

### `RE/Havok/hknpBodyCinfo.h` (ENHANCED)
Body creation info struct (0x60 bytes), with comprehensive documentation.
Constructor at REL::ID(718403). All fields documented with usage notes.

```
+0x00 = const hknpShape* shape        — raw pointer (NOT ref-counted here)
+0x08 = hknpBodyId bodyId             — 0x7FFFFFFF = auto-assign
+0x0C = hknpMotionId motionId         — MUST set via AllocateMotion() for non-static
+0x10 = motionPropertiesId            — {0}=STATIC, {1}=DYNAMIC, {2}=KEYFRAMED
+0x12 = hknpMaterialId materialId
+0x14 = uint32_t collisionFilterInfo
+0x20 = const char* name              — debug name
+0x28 = uintptr_t userData            — SET TO 0 for custom bodies!
+0x30 = hkVector4f position           — Havok space
+0x40 = hkVector4f orientation        — quaternion {x,y,z,w}
```

### `RE/Havok/hknpCapsuleShape.h` (ENHANCED)
Capsule shape for hand colliders (0x70 bytes). Now with documentation of:
- Shape lifetime management (ref-counted but NOT auto-managed by bodies)
- Typical VR hand collider dimensions
- Radius location (inherited convexRadius at +0x14)

### `RE/Havok/hknpShape.h` (ENHANCED)
Shape base class with full shape type hierarchy documentation.
Now documents all shape types, creation patterns, and lifetime rules.

### `RE/Havok/hkVector4.h` (ENHANCED)
Added `Cross()` and `DistanceTo()` methods. Full coordinate space documentation.

### `RE/Havok/hknpCollisionQueryCollector.h` (ENHANCED)
Now with complete usage guide for choosing between collector types.

### `RE/Havok/hknpClosestHitCollector.h` (ENHANCED)
### `RE/Havok/hknpAllHitsCollector.h` (ENHANCED)
Both now have inline usage examples.

### `RE/Havok/hknpCollisionResult.h` (ENHANCED)
All fields documented with usage context.

## Extended Existing Headers

### `RE/Bethesda/BSHavok.h` — bhkWorld (ENHANCED)
Now with comprehensive documentation. 7 methods + full class-level docs:

- `PickObject(bhkPickData&)` — REL::ID(547249) — main raycast (handles locking!)
- `AddPhysicsStepListener(bhkIWorldStepListener&)` — REL::ID(137330)
- `SetDeltaTime(float, bool, bool)` — REL::ID(12890)
- `GetGravity()` — REL::ID(386279)
- `SetMotion(NiAVObject*, Preset, bool, bool, bool)` — REL::ID(357289)
- `EnableCollision(NiAVObject*, bool, bool, bool)` — REL::ID(50037)
- `SetOrigin(NiPoint3&)` — REL::ID(54519)

Internal layout documented: +0x18 = listener array, +0x60 = hknpWorld*.

### `RE/Bethesda/bhkPickData.h` (ENHANCED)
Now with complete usage documentation, member names improved from `field_*` to meaningful names.
10 methods with full parameter documentation.

### `RE/Bethesda/bhkCharacterController.h` — bhkNPCollisionObject
9 methods added (unchanged from previous):

- `SetLinearVelocity(hkVector4f&)` — REL::ID(301531)
- `SetAngularVelocity(hkVector4f&)` — REL::ID(287752)
- `SetVelocity(hkVector4f&, hkVector4f&)` — REL::ID(713376)
- `GetPreviousLinearVelocity(hkVector4f&)` — REL::ID(353575)
- `ApplyLinearImpulse(hkVector4f&)` — REL::ID(858124)
- `ApplyPointImpulseAt(hkVector4f&, hkVector4f&)` — REL::ID(844417)
- `SetMass(float)` — REL::ID(70998)
- `GetCollisionFilterInfo()` — REL::ID(229984)
- `GetCenterOfMassInWorld(hkVector4f&)` — REL::ID(122876)

## Registration

Headers registered in:
- `cmake/sourcelist.cmake` — all new headers added (hknpBody.h, hknpMotion.h, hknpContactEvent.h + originals)
- `include/RE/Fallout.h` — all new #include lines added

## Key Constants

- **Havok scale factor**: 69.99125 (~70.0). 1 Havok unit = ~70 Bethesda game units. Stored at DAT_143718110 (REL::ID 771023). Reciprocal ~0.01429.
- **Contact event type**: `hknpEventType::Enum` value 3 for contact/collision events.
- **Contact modifier flag**: 0x20000 for contact callbacks via `hknpModifierManager::addModifier` REL::ID(1455691).
- **Signal subscribe**: `hkSignal2::subscribe<GlobalSlot>` REL::ID(277607), `<MemberSlot>` REL::ID(427291).

## hknpEventType::Enum Values

| Value | Event |
|---|---|
| 3 | Contact / Collision (FOCollisionListener) |
| 0xB | Island activation |
| 0xC | Broadphase exit |
| 0x16 | Breakable constraint force exceeded |

## Additional Addresses Not Yet in Headers

These are mapped in `E:/fo4dev/.claude/skills/fo4vr-havok-physics/GHIDRA_CONFIRMED_ADDRESSES.md` but not yet wrapped in CommonLibF4VR headers:

**hknpBSWorld (Bethesda world wrapper):**
- `setBodyCollisionFilterInfo(bodyId, filterInfo, rebuildMode)` — REL::ID(101950)
- `setBodyKeyframed(bodyId, rebuildMode)` — REL::ID(527037) — critical for HIGGS hand colliders
- `setBodyStatic(bodyId, rebuildMode)` — REL::ID(1058828)

**bhkPhysicsSystem:**
- `CreateInstance(bhkWorld&, hkTransformf&)` — REL::ID(1245299)
- `HasInstanceInWorld(bhkWorld&)` — REL::ID(792872)
- `GetBodyId(uint)` — REL::ID(526734)
- `ContainsBodyId(hknpBodyId)` — REL::ID(260599)
- `AddToWorld()` — REL::ID(512878)

**Ragdoll (for PLANCK port):**
- `hknpRagdoll::hknpRagdoll(data, world, ...)` — REL::ID(921077)
- `hknpRagdollKeyFrameHierarchyController::ctor` — REL::ID(210136)
- `hkbRagdollDriver::addRagdollToWorld` — REL::ID(150630)

**Motion:**
- `hknpMotion::setFromMassProperties` — REL::ID(335193)
- `hknpMotion::setPointVelocity` — REL::ID(79963)
- `hknpMotion::buildEffMassMatrixAt` — REL::ID(24447)

## Heisenberg Compatibility Notes

Heisenberg (closed-source FO4VR mod) already implements:
- Pre/PostPhysicsCallback — working, exposed via API
- 6-DOF constraint grab — HIGGS-style (object DYNAMIC, hand KEYFRAMED)
- CollisionCallback with mass + separating velocity
- Contact detection via native CONTACT_STARTED events (event type 3)

Heisenberg does NOT have:
- linearCast / capsule shapes / collision layer setup
- Working physics hand bodies (coded but disabled, "may cause crashes")

## Lessons Learned (Pitfalls to Avoid)

These are hard-won discoveries from the HIGGS-FO4VR implementation:

1. **body.userData is NOT always a valid pointer** — FOIslandActivationListener dereferences it unconditionally. Set to 0 for custom bodies or validate (alignment, range, vtable) before dereferencing.

2. **Body position is NOT at body+0x00** — that's AABB data. Real position lives in the motion struct (world+0xE0 + body.motionIndex * 0x80).

3. **motionIndex can be garbage** — bounds-check (> 4096 = skip). motionIndex == 0 means shared static motion.

4. **SetBodyTransform fights the solver** — causes visible stutter for grabbed objects. Use velocity-driven movement instead: ComputeHardKeyFrame + SetBodyVelocity.

5. **Zero angular velocity prevents gyroscope spinning** — when holding objects via velocity, set angular velocity to {0,0,0,0} to prevent spinning.

6. **AllocateMotion() is required before CreateBody** — bodies without a motion slot have undefined behavior.

7. **Compound objects share motionId** — bodies 1593-1597 might all use motionId=165. Don't assume 1:1 mapping.

8. **PickObject needs world lock** — bhkWorld::PickObject handles this internally. Direct hknpWorld::CastRay without lock = crash.

9. **motionPropertiesId changes at runtime** — values like 259, 515, 770 appear. Use (id & 0xFF) to get the base preset.

## How to Use

### Complete VR Hand Collider Example

```cpp
#include "RE/Havok/hknpWorld.h"
#include "RE/Havok/hknpBody.h"
#include "RE/Havok/hknpMotion.h"
#include "RE/Havok/hknpBodyCinfo.h"
#include "RE/Havok/hknpCapsuleShape.h"
#include "RE/Havok/hknpContactEvent.h"
#include "RE/Bethesda/BSHavok.h"
#include "RE/Bethesda/bhkPickData.h"

// === Raycast from hand ===
RE::bhkPickData pick;
pick.SetStartEnd(handPos, handPos + handForward * maxDist);
pick.collisionFilter.filter = 0x02420028; // ItemPicker layer
if (bhkWorld->PickObject(pick)) {
    RE::hknpBody* body = pick.GetBody();
    RE::NiAVObject* obj = pick.GetNiAVObject();
    float dist = pick.GetHitFraction() * maxDist;
}

// === Create capsule hand collider ===
hkVector4f start = { 0, 0, -0.02f, 0 };
hkVector4f end   = { 0, 0,  0.02f, 0 };
auto* capsule = RE::hknpCapsuleShape::CreateCapsuleShape(start, end, 0.015f);

RE::hknpBodyCinfo cinfo;
cinfo.shape = capsule;
cinfo.motionId = { world->AllocateMotion() };      // MUST allocate motion first!
cinfo.motionPropertiesId = { 2 };                    // KEYFRAMED
cinfo.collisionFilterInfo = myCollisionLayer;
cinfo.userData = 0;                                  // MUST be 0 for custom bodies!
RE::hknpBodyId handBodyId = world->CreateBody(cinfo);

// === Sync hand collider to VR controller each frame ===
float targetPos[4] = { handX / 70.0f, handY / 70.0f, handZ / 70.0f, 0 };
float targetRot[4] = { qx, qy, qz, qw };
float linVel[4], angVel[4];
world->ComputeHardKeyFrame(handBodyId, targetPos, targetRot, dt, linVel, angVel);
hkVector4f lv(linVel[0], linVel[1], linVel[2]);
hkVector4f av(angVel[0], angVel[1], angVel[2]);
world->SetBodyVelocity(handBodyId, lv, av);

// === Read body position (type-safe) ===
hkVector4f bodyPos = world->GetBodyPosition(handBodyId);
NiPoint3 gamePos = NiPoint3(bodyPos.x * 70.0f, bodyPos.y * 70.0f, bodyPos.z * 70.0f);

// === Check if nearby body is dynamic ===
RE::hknpBody& nearBody = world->GetBody(someBodyId);
if (RE::IsBodyDynamic(nearBody)) {
    // Can be grabbed/pushed
}

// === Apply throw impulse on grip release ===
hkVector4f throwImpulse = handVelocity * 5.0f;  // 5x multiplier feels good
if (throwImpulse.Length() > 20.0f) {
    throwImpulse = throwImpulse.GetNormalized() * 20.0f;  // clamp
}
world->SetBodyMotionProperties(bodyId, { 1 });  // switch back to DYNAMIC
world->ApplyBodyImpulseAt(bodyId, throwImpulse, grabPoint);

// === Grab: switch to KEYFRAMED ===
world->SetBodyMotionProperties(bodyId, { 2 });  // KEYFRAMED — we control position

// === Register contact listener ===
void* signal = world->GetEventSignal(RE::hknpEventType::kContact);
// subscribe callback to signal via hkSignal2::subscribe REL::ID(277607)

// === Destroy hand collider (on cell transition or shutdown) ===
world->DestroyBodies(&handBodyId, 1);
```

### Proximity Detection via Body Scan

```cpp
// Scan all bodies near a point (used for HIGGS contact detection)
constexpr float kDetectionRadius = 30.0f;  // game units
constexpr float kHavokScale = 1.0f / 70.0f;
float detectionRadiusHavok = kDetectionRadius * kHavokScale;

hkVector4f handPosHavok = HavokUtils::NiPointToHkVector(handPos);
RE::hknpBody* bodyArray = world->GetBodyArray();
RE::hknpMotion* motionArray = world->GetMotionArray();

for (uint32_t i = 1; i < 2048; i++) {  // skip body 0 (world body)
    RE::hknpBody& body = bodyArray[i];
    if (body.motionIndex == 0) continue;           // static
    if (body.motionIndex > 4096) continue;         // invalid
    if (!RE::IsBodyDynamic(body)) continue;        // not pushable

    RE::hknpMotion& motion = motionArray[body.motionIndex];
    float dist = handPosHavok.DistanceTo(motion.position);
    if (dist < detectionRadiusHavok) {
        // Body is within detection radius — push it, trigger haptics, etc.
    }
}
```

## Full Address Reference

See `E:/fo4dev/.claude/skills/fo4vr-havok-physics/GHIDRA_CONFIRMED_ADDRESSES.md` for 150+ confirmed function addresses with REL::IDs, organized by category.
