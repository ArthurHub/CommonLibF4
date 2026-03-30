#pragma once

// bhkPickData — Bethesda's raycast query/result container
//
// Used with bhkWorld::PickObject() to perform raycasts against the physics world.
// Encapsulates the ray origin/destination, collision filter, and hit results.
//
// The struct supports both single-hit and multi-hit queries:
//   - Single hit: check HasHit(), then GetHitFraction() and GetNiAVObject()
//   - Multi-hit:  check GetAllCollectorRayHitSize(), iterate with GetAllCollectorRayHitAt()
//
// Thread safety:
//   bhkWorld::PickObject handles world locking internally. Do NOT call
//   hknpWorld::CastRay directly without acquiring the world read lock first
//   (it will crash — this is a confirmed FO4VR-specific issue).
//
// Collision filter values (common):
//   0x02420028 — ItemPicker layer (picks up interactive objects)
//   0x00000001 — Everything
//   See COL_LAYER enum for layer definitions.
//
// Usage:
//   RE::bhkPickData pick;
//   pick.SetStartEnd(handPos, handPos + handForward * maxDist);
//   pick.collisionFilter.filter = 0x02420028;
//
//   if (bhkWorld->PickObject(pick)) {
//       // Single closest hit
//       RE::NiAVObject* hitObj = pick.GetNiAVObject();
//       RE::hknpBody* hitBody = pick.GetBody();
//       float fraction = pick.GetHitFraction();  // 0..1 along ray
//       float hitDist = fraction * maxDist;
//
//       // Or iterate all hits
//       pick.SortAllCollectorHits();
//       for (int i = 0; i < pick.GetAllCollectorRayHitSize(); i++) {
//           RE::hknpCollisionResult res;
//           if (pick.GetAllCollectorRayHitAt(i, res)) {
//               // Process each hit...
//           }
//       }
//   }
//
//   // Reuse for another raycast
//   pick.Reset();
//   pick.SetStartEnd(newStart, newEnd);

#include "RE/Bethesda/MemoryManager.h"
#include "RE/Bethesda/bhkCharacterController.h"
#include "RE/Havok/hkVector4.h"
#include "RE/Havok/hknpCollisionResult.h"
#include "RE/NetImmerse/NiPoint.h"

namespace RE
{
	/// bhkPickData — raycast query parameters and results.
	///
	/// Memory layout: 0xE0 bytes (heap-allocated via F4_HEAP_REDEFINE_NEW).
	/// Construct with default constructor, then configure with SetStartEnd()
	/// and collisionFilter before passing to bhkWorld::PickObject().
	struct bhkPickData
	{
	public:
		/// Construct with default values (no hit, zero ray).
		bhkPickData()
		{
			typedef bhkPickData*           func_t(bhkPickData*);
			static REL::Relocation<func_t> func{ REL::ID(526783) };
			func(this);
		}

		/// Set the ray start and end points in game-space coordinates.
		/// The ray goes from start to end — fraction 0.0 is at start, 1.0 is at end.
		///
		/// @param start  Ray origin (game-space coordinates)
		/// @param end    Ray endpoint (game-space coordinates)
		void SetStartEnd(const NiPoint3& start, const NiPoint3& end)
		{
			using func_t = decltype(&bhkPickData::SetStartEnd);
			static REL::Relocation<func_t> func{ REL::ID(747470) };
			return func(this, start, end);
		}

		/// Reset all results for reuse (keeps the struct allocated).
		/// Call this before reusing a bhkPickData for another raycast.
		void Reset()
		{
			using func_t = decltype(&bhkPickData::Reset);
			static REL::Relocation<func_t> func{ REL::ID(438299) };
			return func(this);
		}

		/// Check if the raycast found any hit.
		/// @return True if at least one body was hit
		bool HasHit()
		{
			using func_t = decltype(&bhkPickData::HasHit);
			static REL::Relocation<func_t> func{ REL::ID(1181584) };
			return func(this);
		}

		/// Get the hit fraction (0.0 = ray start, 1.0 = ray end).
		/// Multiply by your max distance to get the actual hit distance.
		/// Only valid if HasHit() returns true.
		float GetHitFraction()
		{
			using func_t = decltype(&bhkPickData::GetHitFraction);
			static REL::Relocation<func_t> func{ REL::ID(476687) };
			return func(this);
		}

		/// Get the number of hits in the all-hits collector.
		/// Only populated if using the all-hits collector mode.
		int32_t GetAllCollectorRayHitSize()
		{
			using func_t = decltype(&bhkPickData::GetAllCollectorRayHitSize);
			static REL::Relocation<func_t> func{ REL::ID(1288513) };
			return func(this);
		}

		/// Get a specific hit result from the all-hits collector by index.
		///
		/// @param i    Hit index (0-based)
		/// @param res  [out] Collision result with hit position, normal, body info
		/// @return True if the hit at index i was valid
		bool GetAllCollectorRayHitAt(uint32_t i, hknpCollisionResult& res)
		{
			using func_t = decltype(&bhkPickData::GetAllCollectorRayHitAt);
			static REL::Relocation<func_t> func{ REL::ID(583997) };
			return func(this, i, res);
		}

		/// Sort all-hits collector results by hit fraction (closest first).
		/// Call this before iterating if you want hits in distance order.
		void SortAllCollectorHits()
		{
			using func_t = decltype(&bhkPickData::SortAllCollectorHits);
			static REL::Relocation<func_t> func{ REL::ID(1274842) };
			return func(this);
		}

		/// Resolve the closest hit to a NiAVObject (scene graph node).
		/// Returns nullptr if the hit body has no associated NiAVObject.
		/// This traverses from the physics body back to the render object.
		NiAVObject* GetNiAVObject()
		{
			using func_t = decltype(&bhkPickData::GetNiAVObject);
			static REL::Relocation<func_t> func{ REL::ID(863406) };
			return func(this);
		}

		/// Get the hknpBody that was hit (closest hit).
		/// Returns the raw Havok body pointer — use for physics operations.
		hknpBody* GetBody()
		{
			using func_t = decltype(&bhkPickData::GetBody);
			static REL::Relocation<func_t> func{ REL::ID(1223055) };
			return func(this);
		}

		// members
		std::uint64_t        raycastFlags;     // 00 — raycast configuration flags
		std::uint16_t        raycastMode;      // 08 — query mode (single/all hits)
		CFilter              collisionFilter;  // 0A — collision layer filter (set before PickObject)
		std::uint64_t        shapeCastInfo;    // 10 — shape cast extra info
		std::uint32_t        queryFlags;       // 18 — additional query flags
		hkVector4f           rayOrigin;        // 20 — ray start point (Havok space, set by SetStartEnd)
		hkVector4f           rayDest;          // 30 — ray end point (Havok space, set by SetStartEnd)
		char                 gap40[16];        // 40 — internal state
		int                  hitStatus;        // 50 — hit status code
		hknpCollisionResult  result;           // 58 — closest hit collision result
		hknpCollisionResult* allHitsResult;    // B8 — pointer to all-hits result array
		std::uint64_t        collisionLayer;   // C0 — resolved collision layer
		__int64              collector;        // C8 — internal collector pointer
		int                  collectorStatus;  // D0 — collector state
		__int16              field_D4;         // D4
		char                 field_D6;         // D6
		char                 field_D7;         // D7
		F4_HEAP_REDEFINE_NEW(bhkPickData);
	};
	static_assert(sizeof(bhkPickData) == 0xE0);
};
