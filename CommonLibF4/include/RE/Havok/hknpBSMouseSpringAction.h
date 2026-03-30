#pragma once

// hknpBSMouseSpringAction — Bethesda-custom Havok action for spring-based grab.
//
// This is NOT a Havok spring constraint. It is a Havok Action that runs every
// physics step and directly manipulates body velocity + applies correction impulses
// to pull the grabbed body toward a target position/orientation.
//
// Used by:
//   - Normal Z-key grab (hold object in front of player)
//   - VR wand grab (hold object in VR hand)
//   - Telekinesis magic effect
//
// The game allocates 0xE0 bytes for this object via the Havok TLS allocator.
// It is registered with hknpActionManager::AddAction and removed with RemoveAction.
//
// Spring physics algorithm (applyAction, called every physics step):
//   1. Transform local grab point to world space, compute error vs target
//   2. Call hknpWorld::computeHardKeyFrame for desired velocity
//   3. Set body velocity directly
//   4. Compute correction impulse: impulse = elasticity * error + damping * velError
//   5. Apply impulse via hknpWorld::applyBodyLinearImpulse
//   6. Re-enable keep-awake flag (0x8000000)
//
// RTTI: ".?AVhknpBSMouseSpringAction@@" at VR 0x1438b1988
// Vtable: VR 0x142e88f28
//
// See also: E:\fo4dev\plans\stalone\ghidra_mapping\ghidra_findings_native_interaction.md

#include "RE/Havok/hkVector4.h"
#include "RE/Havok/hknpBodyId.h"
#include "RE/Havok/hkReferencedObject.h"

namespace RE
{
	class hknpWorld;

	/// hknpBSMouseSpringAction — velocity-based spring action for grab/hold mechanics.
	///
	/// Each physics step, the action:
	///   1. Computes velocity to reach the target via computeHardKeyFrame
	///   2. Sets body velocity directly
	///   3. Applies correction impulse with elasticity/damping parameters
	///
	/// The grabbed object stays fully collidable with the world — the spring
	/// just applies forces each step to pull it toward the target position.
	///
	/// PlayerCharacter grab data offsets:
	///   +0x9E0  Primary hand grab data (formId + rotation + type + distance)
	///   +0xA30  Secondary hand grab data
	///   +0xA80  Primary hand spring array (BSTArray<hkRefPtr<hknpBSMouseSpringAction>>)
	///   +0xA98  Secondary hand spring array
	class hknpBSMouseSpringAction
	{
	public:
		// Construction — use the game's allocator + constructor, not new
		// Constructor: VR 0x141e4a850 (takes Cinfo struct)
		// Destructor:  VR 0x141e4afc0

		/// Set the target world position the spring pulls toward.
		/// Call every frame with the VR hand/wand position.
		void SetWorldPosition(hkVector4f& a_position)
		{
			using func_t = void(*)(hknpBSMouseSpringAction*, hkVector4f&);
			static REL::Relocation<func_t> func{ REL::Offset(0x1E4A960) };
			func(this, a_position);
		}

		/// Set the target orientation for the spring.
		/// @param a_rotation   Goal rotation matrix (3x4)
		/// @param a_transform  Current body transform for reference
		void SetGoalRotation(void* a_rotation, void* a_transform)
		{
			using func_t = void(*)(hknpBSMouseSpringAction*, void*, void*);
			static REL::Relocation<func_t> func{ REL::Offset(0x1E4A9B0) };
			func(this, a_rotation, a_transform);
		}

		/// Get the current target world position.
		hkVector4f* GetWorldPosition()
		{
			using func_t = hkVector4f*(*)(hknpBSMouseSpringAction*);
			static REL::Relocation<func_t> func{ REL::Offset(0x1E4AA10) };
			return func(this);
		}

		/// Get the local grab point on the body.
		hkVector4f* GetLocalPosition()
		{
			using func_t = hkVector4f*(*)(hknpBSMouseSpringAction*);
			static REL::Relocation<func_t> func{ REL::Offset(0x1E4AA20) };
			return func(this);
		}

		/// Manually trigger one step of the spring physics.
		/// Normally called automatically by the action manager each physics step.
		/// Also called during paused physics (flag at 0x1465a3d8c == 0).
		void ApplyAction(hknpWorld* a_world, float a_deltaTime, bool a_param3 = false)
		{
			using func_t = void(*)(hknpBSMouseSpringAction*, hknpWorld*, float, bool);
			static REL::Relocation<func_t> func{ REL::Offset(0x1E4AA30) };
			func(this, a_world, a_deltaTime, a_param3);
		}

		// members — 0xE0 bytes total
		void*         vtable;           // 00 — vtable (VR 0x142e88f28)
		std::uint32_t refcountFlags;    // 08 — 0xFFFF0001 initial (refcount=1, memSizeAndFlags=0xFFFF)
		std::uint32_t pad0C;            // 0C
		std::uint64_t reserved10;       // 10
		hknpBodyId    bodyId;           // 18 — the grabbed body
		std::uint8_t  pad1C[0x04];      // 1C
		std::uint8_t  initialTransform[0x30]; // 20 — initial body transform (3x4 matrix, from Cinfo)
		std::uint8_t  pad50[0x10];      // 50
		std::uint8_t  goalRotation[0x30]; // 60 — goal rotation matrix (set by SetGoalRotation)
		hkVector4f    targetPosition;   // 90 — target world position (set by SetWorldPosition)
		hkVector4f    localGrabPoint;   // A0 — local grab point on body (from Cinfo)
		hkVector4f    previousDelta;    // B0 — previous frame position delta (convergence check)
		float         maxAngularForce;  // C0 — angular force limit
		float         elasticity;       // C4 — spring stiffness (how fast body tracks target)
		float         damping;          // C8 — oscillation reduction
		float         objectDamping;    // CC — additional mass-based damping
		std::uint32_t bodyFlagsToEnable; // D0 — typically 0x8000000 (keep-awake)
		std::uint32_t padD4;            // D4
		std::uint64_t padD8;            // D8
	};
	static_assert(sizeof(hknpBSMouseSpringAction) == 0xE0);
}
