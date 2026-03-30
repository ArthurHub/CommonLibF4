#pragma once

// hknpCollisionResult — result data from a collision query
//
// Returned by all collision queries (raycast, shape cast, closest points, AABB).
// Contains the hit position, surface normal, hit fraction, and body identification
// for both the query body and the hit body.
//
// Key fields:
//   position  — world-space hit point (Havok coordinates)
//   normal    — surface normal at hit point
//   fraction  — parametric distance along ray (0.0 = start, 1.0 = end)
//   hitBodyInfo.m_bodyId — hknpBodyId of the body that was hit
//
// Usage (after a raycast):
//   if (collector.HasHit()) {
//       auto& r = collector.result;  // or collector.GetHits()[i]
//       NiPoint3 hitPos = NiPoint3(r.position.x, r.position.y, r.position.z) * 70.0f;
//       hknpBodyId bodyId = r.hitBodyInfo.m_bodyId;
//   }

#include "RE/Havok/hkBaseTypes.h"
#include "RE/Havok/hkVector4.h"
#include "RE/Havok/hknpBodyId.h"
#include "RE/Havok/hknpMaterialId.h"

namespace RE
{
	/// Collision query type (forward-declared enum).
	struct hknpCollisionQueryType
	{
	public:
		enum class Enum;
	};

	/// hknpCollisionResult — a single collision query hit result.
	///
	/// Memory layout: 0x60 bytes.
	/// Contains geometric data (position, normal, fraction) and identification
	/// for both the query and hit bodies.
	struct hknpCollisionResult
	{
	public:
		/// BodyInfo — identifies one side of a collision (query or hit body).
		struct BodyInfo
		{
		public:
			// members
			hknpBodyId              m_bodyId;                    // 00 — body ID in the world
			hknpMaterialId          m_shapeMaterialId;           // 04 — material of the hit shape
			hkPadSpu<std::uint32_t> m_shapeKey;                  // 08 — shape key (for composite shapes)
			hkPadSpu<std::uint32_t> m_shapeCollisionFilterInfo;  // 0C — collision filter of hit shape
			hkPadSpu<std::size_t>   m_shapeUserData;             // 10 — user data from the hit shape
		};
		static_assert(sizeof(BodyInfo) == 0x18);

		// members
		hkVector4f                                               position;       // 00 — world-space hit point (Havok coords)
		hkVector4f                                               normal;         // 10 — surface normal at hit point
		hkPadSpu<float>                                          fraction;       // 20 — parametric hit distance (0..1)
		BodyInfo                                                 queryBodyInfo;  // 28 — info about the query body
		BodyInfo                                                 hitBodyInfo;    // 40 — info about the hit body
		REX::EnumSet<hknpCollisionQueryType::Enum, std::int32_t> queryType;      // 58 — type of query that generated this
		hkPadSpu<std::uint32_t>                                  hitResult;      // 5C — hit result flags
	};
	static_assert(sizeof(hknpCollisionResult) == 0x60);
}
