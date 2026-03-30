#pragma once

// hknpCollisionQueryCollector — abstract base for collecting collision query results
//
// All collision queries (raycast, shape cast, closest points, AABB query) deliver
// their results through a collector. Choose the right collector for your use case:
//
//   hknpClosestHitCollector — keeps only the single closest hit (0x90 bytes)
//     Use for: simple raycasts, grab target detection
//     Usage:
//       RE::hknpClosestHitCollector collector;
//       world->CastRay(query, &collector);
//       if (collector.HasHit()) {
//           auto& hit = collector.result;
//           // hit.position, hit.normal, hit.fraction, hit.hitBodyInfo.m_bodyId
//       }
//
//   hknpAllHitsCollector — collects up to 10 hits (0x3F0 bytes, stack-allocated)
//     Use for: penetration detection, multi-target queries, QueryAabb
//     Usage (access inline array directly — virtual methods cause linker errors):
//       RE::hknpAllHitsCollector collector;
//       world->QueryAabb(&query, &collector);
//       for (int i = 0; i < collector.hits._size; i++) {
//           auto& hit = collector.hits._data[i];
//       }
//
// For gameplay raycasts, prefer bhkWorld::PickObject(bhkPickData&) which handles
// world locking and NiAVObject resolution automatically.

#include "RE/Havok/hkBaseObject.h"
#include "RE/Havok/hkSimdFloat.h"

namespace RE
{
	struct hknpCollisionResult;

	/// hknpCollisionQueryCollector — abstract interface for receiving collision query results.
	///
	/// Memory layout: 0x20 bytes.
	/// Subclasses implement the virtual methods to store/filter results.
	class __declspec(novtable) hknpCollisionQueryCollector :
		public hkBaseObject  // 00
	{
	public:
		static constexpr auto RTTI{ RTTI::hknpCollisionQueryCollector };
		static constexpr auto VTABLE{ VTABLE::hknpCollisionQueryCollector };

		// add
		virtual void                       Reset() = 0;                                   // 01 — clear all stored results
		virtual void                       AddHit(const hknpCollisionResult& a_hit) = 0;  // 02 — called by queries for each hit
		virtual bool                       HasHit() const = 0;                            // 03 — true if any hits were collected
		virtual std::int32_t               GetNumHits() const = 0;                        // 04 — number of collected hits
		virtual const hknpCollisionResult* GetHits() const = 0;                           // 05 — pointer to hit array

		// members
		std::int32_t  hints;              // 08 — query hints (collision filter cache)
		hkSimdFloat32 earlyOutThreshold;  // 10 — early-out distance (closest hit so far)
	};
	static_assert(sizeof(hknpCollisionQueryCollector) == 0x20);
}
