#pragma once

// hknpAllHitsCollector — collects ALL collision hits (up to 10, stack-allocated)
//
// Unlike hknpClosestHitCollector which keeps only the closest, this collector
// stores all hits. Uses an hkInplaceArray with capacity for 10 results (0x3F0 bytes).
// If more than 10 hits occur, the array will heap-allocate (but this is rare).
//
// Usage:
//   RE::hknpAllHitsCollector collector;
//   world->QueryAabb(&query, &collector);
//   for (int i = 0; i < collector.hits._size; i++) {
//       const auto& hit = collector.hits._data[i];
//       // hit.fraction, hit.hitBodyInfo.m_bodyId, hit.position, hit.normal
//   }
//
// IMPORTANT: Access the inline hkInplaceArray members directly (hits._size,
// hits._data) instead of using the virtual GetNumHits()/GetHits() methods.
// The virtual functions are declared as overrides in this header but have no
// compiled implementations in the game binary — calling them causes unresolved
// external linker errors. The declarations are kept here for vtable completeness.

#include "RE/Havok/hkArray.h"
#include "RE/Havok/hknpCollisionQueryCollector.h"
#include "RE/Havok/hknpCollisionResult.h"

namespace RE
{
	/// hknpAllHitsCollector — stores all hits from a collision query (up to 10 inline).
	///
	/// Memory layout: 0x3F0 bytes. Stack-allocatable but large.
	/// Uses hkInplaceArray<hknpCollisionResult, 10> for inline storage.
	class __declspec(novtable) hknpAllHitsCollector :
		public hknpCollisionQueryCollector  // 000
	{
	public:
		static constexpr auto RTTI{ RTTI::hknpAllHitsCollector };
		static constexpr auto VTABLE{ VTABLE::hknpAllHitsCollector };

		hknpAllHitsCollector()
		{
			stl::emplace_vtable<hknpAllHitsCollector>(this);
			hits._data = (hknpCollisionResult*)((uintptr_t)this + 0x30);
			hits._capacityAndFlags = 0x8000000A;  // 10 inline slots, local storage flag
		}

		// override (hknpCollisionQueryCollector)
		void                       Reset() override;                             // 01
		void                       AddHit(const hknpCollisionResult&) override;  // 02
		bool                       HasHit() const override;                      // 03
		std::int32_t               GetNumHits() const override;                  // 04
		const hknpCollisionResult* GetHits() const override;                     // 05

		// members
		hkInplaceArray<hknpCollisionResult, 10> hits;  // 020 — inline array of up to 10 results
	};
	static_assert(sizeof(hknpAllHitsCollector) == 0x3F0);
}
