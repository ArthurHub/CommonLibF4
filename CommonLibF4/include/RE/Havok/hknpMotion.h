#pragma once

// hknpMotion — Havok 2014 (New Physics) motion data
//
// Each motion lives in a flat array inside hknpWorld (world+0xE0, stride 0x80).
// Motions store the actual world-space position, orientation, and velocity of bodies.
// Multiple bodies can share one motion (compound objects).
//
// Key insight: body position is NOT at body+0x00 (that's AABB data).
// Real position lives here in the motion struct at offset +0x00.
//
// Motion index 0 is the "shared static motion" — all static bodies point here.
// Dynamic/keyframed bodies have their own motion slot (allocated via AllocateMotion()).
//
// Offsets RE'd via Ghidra on Fallout4VR.exe.unpacked.exe.
// Confirmed by reading actual motion data in HIGGS-FO4VR contact detection.
//
// Coordinate space: all positions/velocities are in Havok space (1 unit = 70 game units).
// Use HavokUtils::kHavokScale (1/70) to convert to game space.
//
// Usage:
//   // Get motion for a body
//   auto* motionArray = *reinterpret_cast<uint8_t**>(
//       reinterpret_cast<uintptr_t>(hknpWorld) + 0xE0);
//   auto* motion = reinterpret_cast<RE::hknpMotion*>(
//       motionArray + body.motionIndex * sizeof(RE::hknpMotion));
//
//   // Read world position (Havok space)
//   float worldX = motion->position.x;  // multiply by 70.0 for game units
//
//   // Read velocity
//   float speed = motion->linearVelocity.Length();
//
//   // Check if motion is active (not sleeping)
//   bool active = (motion->spaceSplitterWeight > 0.0f);

#include "RE/Havok/hkVector4.h"

namespace RE
{
	/// hknpMotion — per-motion data stored in the world's motion array.
	///
	/// Memory layout: 0x80 bytes per motion (stride confirmed via Ghidra).
	/// The motion array is at hknpWorld+0xE0.
	///
	/// Motion index 0 is reserved for the "shared static motion" (all static
	/// bodies reference this). Custom bodies need their own motion slot
	/// allocated with hknpWorld::AllocateMotion().
	/// Ghidra-verified layout (2026-03-22), confirmed against FRIK GrabConstraint.h
	/// (MOTION_PACKED_INERTIA_OFFSET = 0x20) and Hand.h (body+0x30 = translation).
	///
	/// IMPORTANT: Velocities are at +0x40/+0x50, NOT at +0x20/+0x30.
	/// +0x20 is packed inverse inertia (int16[4]), not velocity!
	///
	///   +0x00  hkVector4f  COM position (world-space, Havok coords)
	///   +0x10  hkVector4f  orientation quaternion (x, y, z, w)
	///   +0x20  int16[4]    packed inverse inertia [invInertiaX, Y, Z, invMass]
	///   +0x28  uint32      firstBodyId
	///   +0x2C  uint32      inverseMassOrState
	///   +0x30  uint64      internalData
	///   +0x38  uint16      motionPropertiesId (2=KEYFRAMED)
	///   +0x3A  uint16      maxLinearVelocity (half-float)
	///   +0x3C  uint16      maxAngularVelocity (half-float)
	///   +0x3E  uint8       deactivationState
	///   +0x3F  uint8       activationTickCounter
	///   +0x40  hkVector4f  LINEAR VELOCITY (world-space, Havok units/sec)
	///   +0x50  hkVector4f  ANGULAR VELOCITY (body-local, 2x scaled)
	///   +0x60  hkVector4f  integrationAccumulators
	///   +0x70  float       spaceSplitterWeight
	///   +0x74  float       maxLinearAccelerationDistancePerStep
	///   +0x78  float       maxRotationToPreventTunneling
	///   +0x7C  float       timeFactor
	struct hknpMotion
	{
	public:
		// +0x00: World-space position (Havok coordinates, divide by kHavokScale for game units).
		// This is the CENTER OF MASS position, not necessarily the visual origin.
		// w component may contain encoded data — treat as padding.
		hkVector4f position;          // 00

		// +0x10: Orientation quaternion (x, y, z, w).
		// Normalized quaternion representing the body's current rotation.
		hkVector4f orientation;       // 10

		// +0x20: Packed inverse inertia (NOT velocity — velocity is at +0x40!).
		// int16[4]: invInertiaX, invInertiaY, invInertiaZ, invMass.
		// FRIK GrabConstraint.h confirms: MOTION_PACKED_INERTIA_OFFSET = 0x20.
		std::int16_t packedInverseInertia[4];  // 20

		// +0x28: First body ID that references this motion.
		std::uint32_t firstBodyId;    // 28

		// +0x2C: Inverse mass or motion state flag.
		std::uint32_t inverseMassOrState;  // 2C

		// +0x30: Internal data used by the solver.
		std::uint64_t internalData;   // 30

		// +0x38: Motion properties ID — determines physics behavior preset.
		// 0=STATIC, 1=DYNAMIC, 2=KEYFRAMED, 3=FROZEN
		std::uint16_t motionPropertiesId;  // 38

		// +0x3A: Maximum linear velocity (half-float encoded).
		std::uint16_t maxLinearVelocity;   // 3A

		// +0x3C: Maximum angular velocity (half-float encoded).
		std::uint16_t maxAngularVelocity;  // 3C

		// +0x3E: Deactivation state (island sleeping).
		std::uint8_t deactivationState;    // 3E

		// +0x3F: Activation tick counter.
		std::uint8_t activationTickCounter;  // 3F

		// +0x40: LINEAR VELOCITY in Havok space (units/second).
		// For keyframed bodies, this is set via ComputeHardKeyFrame() each frame.
		// WARNING: This is at +0x40, NOT +0x20 (which is packed inertia).
		hkVector4f linearVelocity;    // 40

		// +0x50: ANGULAR VELOCITY (body-local frame, 2x scaled, radians/second).
		// For grabbed objects, set to zero to prevent gyroscope spinning.
		// WARNING: This is at +0x50, NOT +0x30.
		hkVector4f angularVelocity;   // 50

		// +0x60: Integration accumulators (solver internal).
		hkVector4f integrationAccumulators;  // 60

		// +0x70: Additional motion properties
		float spaceSplitterWeight;     // 70 — island/deactivation weight (0 = sleeping)
		float maxLinearAccelerationDistancePerStep;  // 74
		float maxRotationToPreventTunneling;         // 78
		float timeFactor;              // 7C — time scale (1.0 = normal, 0.0 = frozen)
	};
	static_assert(sizeof(hknpMotion) == 0x80);

	/// Helper: stride between motions in the world's motion array.
	/// Use this when doing pointer arithmetic on the raw motion array.
	inline constexpr std::size_t kHknpMotionStride = 0x80;

	/// Helper: stride between bodies in the world's body array.
	inline constexpr std::size_t kHknpBodyStride = 0x90;
}
