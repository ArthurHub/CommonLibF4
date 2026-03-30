#pragma once

// hknpContactEvent — Havok contact/collision event data
//
// There are TWO different contact event structures in FO4VR:
//
// 1. hknpContactSignalData — the RAW data pointer passed to callbacks subscribed
//    via hknpWorld::GetEventSignal(3). This is what you receive in your callback.
//    Confirmed via Ghidra decompilation of FUN_14061b5c0 (FOCollisionListener).
//
// 2. hknpContactEvent — a processed/wrapper event used internally by the engine.
//    The original struct in this header had offsets that DON'T match the raw callback.
//    Kept for reference but should NOT be used for signal callback parsing.
//
// CALLBACK SIGNATURE (confirmed via Ghidra 2026-03-16):
//   void Callback(void* userData, void** worldPtrHolder, void* contactSignalData)
//
//   - userData: whatever you passed to subscribe_ext
//   - *worldPtrHolder: dereference to get hknpWorld*
//   - contactSignalData: pointer to hknpContactSignalData below
//
// SUBSCRIBE PATTERN:
//   void* signal = hknpWorld->GetEventSignal(3);  // REL::ID(1029853)
//   // Use extended subscribe at VR 0x1403b9e50 (no REL::ID):
//   struct { void* fn; uint64_t ctx; } info = { &Callback, 0 };
//   subscribeExt(signal, userData, &info);
//
// No explicit unsubscribe needed — hknpWorld destructor cleans up all slots.

#include "RE/Havok/hkVector4.h"
#include "RE/Havok/hknpBodyId.h"

namespace RE
{
	/// hknpContactSignalData — RAW contact event data from getEventSignal(3) callback.
	///
	/// This is the pointer you receive as the 3rd argument in your contact callback.
	/// Offsets confirmed via Ghidra decompilation of FOCollisionListener (VR 0x14061b5c0).
	///
	/// WARNING: This struct layout is for the RAW signal callback data only.
	/// Do NOT confuse with the old hknpContactEvent struct below.
	///
	/// Usage:
	///   static void OnContact(void* userData, void** worldPtrHolder, void* eventData) {
	///       auto* data = static_cast<const RE::hknpContactSignalData*>(eventData);
	///       uint32_t bodyA = data->bodyIdA;
	///       uint32_t bodyB = data->bodyIdB;
	///       float sepVel = data->separatingVelocities[0];
	///   }
	struct hknpContactSignalData
	{
	public:
		// +0x00: Internal event header / vtable data (do not access)
		std::uint64_t header;  // 00

		// +0x08: ID of the first body involved in the contact.
		std::uint32_t bodyIdA;  // 08

		// +0x0C: ID of the second body involved in the contact.
		std::uint32_t bodyIdB;  // 0C

		// +0x10: Pointer to contact manifold data (internal Havok structure).
		// Contains per-contact-point positions, normals, and shape keys.
		void* manifoldPtr;  // 10

		// +0x18: Index into the manifold's contact point array.
		std::uint8_t contactIndex;  // 18

		// +0x19: Manifold type flag. 0 = single contact point, 1 = multi-point manifold.
		// When 1, use contactIndex to index into the manifold's per-point data.
		std::uint8_t manifoldType;  // 19

		// +0x1A: Padding
		std::uint8_t pad1A[0x16];  // 1A

		// +0x30: Array of separating velocities (one float per contact point).
		// Positive = bodies separating. Negative = bodies approaching.
		// For simple contacts, just read separatingVelocities[0].
		// The number of valid entries depends on the manifold's contact count.
		float separatingVelocities[4];  // 30 (4 entries covers typical max contacts)
	};
	static_assert(sizeof(hknpContactSignalData) == 0x40);

	/// hknpContactEvent — LEGACY/INTERNAL processed contact event.
	///
	/// WARNING: This struct does NOT represent the raw data received in
	/// getEventSignal(3) callbacks. Use hknpContactSignalData for callbacks.
	///
	/// This may represent a processed/wrapper event used elsewhere in the engine.
	/// Kept for reference — offsets not fully verified for any specific use case.
	struct hknpContactEvent
	{
	public:
		hkVector4f    contactPoint;        // 00 — world-space contact point (Havok coords)
		hkVector4f    normal;              // 10 — contact normal (A toward B)
		hknpBodyId    bodyIdA;             // 20
		float         separatingVelocity;  // 24
		hknpBodyId    bodyIdB;             // 28
		std::uint32_t additionalInfo;      // 2C
		std::uint8_t  flags;               // 30
		std::uint8_t  flags2;              // 31
		std::uint8_t  pad32[0x0E];         // 32
	};
	static_assert(sizeof(hknpContactEvent) == 0x40);
}
