#pragma once

// hknpClosestHitCollector — keeps only the single closest collision hit
//
// The most commonly used collector. Automatically discards farther hits,
// so after a query you either have zero or one result.
//
// Usage:
//   RE::hknpClosestHitCollector collector;
//   world->CastRay(query, &collector);
//   if (collector.HasHit()) {
//       float dist = collector.result.fraction;
//       hknpBodyId hitBody = collector.result.hitBodyInfo.m_bodyId;
//       hkVector4f hitPos = collector.result.position;
//       hkVector4f hitNorm = collector.result.normal;
//   }

#include "RE/Havok/hkArray.h"
#include "RE/Havok/hknpCollisionQueryCollector.h"
#include "RE/Havok/hknpCollisionResult.h"

namespace RE
{
	/// hknpClosestHitCollector — stores only the closest hit from a collision query.
	///
	/// Memory layout: 0x90 bytes. Stack-allocatable.
	/// Constructor at REL::ID(951692)+0x10.
	class __declspec(novtable) hknpClosestHitCollector :
		public hknpCollisionQueryCollector  // 000
	{
	public:
		static constexpr auto RTTI{ RTTI::hknpClosestHitCollector };
		static constexpr auto VTABLE{ VTABLE::hknpClosestHitCollector };

		hknpClosestHitCollector()
		{
			typedef hknpClosestHitCollector* func_t(hknpClosestHitCollector*);
			static REL::Relocation<func_t>   func{ REL::ID(951692), 0x10 };
			func(this);
		}

		// override (hknpCollisionQueryCollector)
		void                       Reset() override;                             // 01
		void                       AddHit(const hknpCollisionResult&) override;  // 02
		bool                       HasHit() const override;                      // 03
		std::int32_t               GetNumHits() const override;                  // 04
		const hknpCollisionResult* GetHits() const override;                     // 05

		// members
		hknpCollisionResult result;  // 20 — the closest hit result (valid if hasHit == true)
		bool                hasHit;  // 80 — true if a hit was recorded
	};
	static_assert(sizeof(hknpClosestHitCollector) == 0x90);
}
