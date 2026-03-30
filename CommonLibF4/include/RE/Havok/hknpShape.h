#pragma once

// hknpShape — Havok 2014 (New Physics) collision shape base class
//
// All collision shapes inherit from hknpShape. The shape defines the geometric
// volume used for collision detection. Shapes are reference-counted via
// hkReferencedObject and can be shared between multiple bodies.
//
// Shape hierarchy in FO4:
//   hknpShape (abstract base)
//   ├── hknpConvexShape (convex primitives)
//   │   ├── hknpCapsuleShape    — pill shape (hand colliders, character controllers)
//   │   ├── hknpSphereShape     — sphere (not RE'd yet)
//   │   └── hknpConvexPolytopeShape — arbitrary convex hull
//   ├── hknpCompressedMeshShape — triangle mesh (most static world geometry)
//   ├── hknpStaticCompoundShape — compound of multiple child shapes
//   └── hknpHeightFieldShape    — terrain
//
// Shape types (hknpShapeType::Enum):
//   kConvex = 0           — generic convex
//   kCapsule = 3          — capsule (used for VR hand colliders)
//   kCompressedMesh = 5   — static world geometry
//   kStaticCompound = 7   — compound containers
//
// Creating shapes:
//   // Capsule (for hand colliders):
//   auto* capsule = RE::hknpCapsuleShape::CreateCapsuleShape(start, end, radius);
//
//   // Bethesda's cached capsule factory (auto-scales, 10-entry cache):
//   // REL::ID(1526865) — bhkCharacterControllerShapeManager::GetCapsuleShape
//
// Shape lifetime:
//   Shapes are ref-counted. When assigned to a body via hknpBodyCinfo::shape,
//   the body does NOT take ownership. The caller must keep the shape alive
//   for the body's lifetime, or use hkRefPtr<hknpShape> for automatic management.
//
// userData field (offset +0x18):
//   Per-shape user data, separate from body userData. Often unused (0).
//   Can store a pointer to application-specific shape metadata.

#include "RE/Havok/hkBaseTypes.h"
#include "RE/Havok/hkBlockStream.h"
#include "RE/Havok/hkReferencedObject.h"

namespace RE
{
	namespace hkcdGsk
	{
		struct Cache;
	}

	class hkAabb;
	class hkcdVertex;
	class hknpShapeSignals;
	class hkRefCountedProperties;
	class hkTransformf;

	template <class, class>
	class hkArray;

	struct hkDiagonalizedMassProperties;
	struct hkGeometry;
	struct hknpAabbQuery;  // Full definition in RE/Havok/hknpAabbQuery.h
	struct hknpCdBody;
	struct hknpCollisionQueryContext;
	struct hknpPointQuery;
	struct hknpQueryFilterData;
	struct hknpRayCastQuery;
	struct hknpShapeCollector;
	struct hknpShapeKeyMask;
	struct hknpShapeQueryInfo;
	struct hknpSolverInfo;

	/// Collision dispatch type — determines which collision algorithm is used
	/// for narrowphase collision detection between two shapes.
	struct hknpCollisionDispatchType
	{
	public:
		enum Enum
		{
			kNone,       ///< No collision
			kConvex,     ///< Convex-convex (GJK/EPA)
			kComposite,  ///< Composite shape (iterate children)
			kDistanceField,  ///< SDF-based collision
			kUser        ///< Custom collision algorithm
		};
	};
	static_assert(std::is_empty_v<hknpCollisionDispatchType>);

	/// Shape type enum — identifies the concrete shape subclass.
	/// Use GetType() to query at runtime.
	struct hknpShapeType
	{
	public:
		enum class Enum
		{
			kConvex,                ///< Generic convex shape
			kConvexPolytope,        ///< Convex hull with explicit vertices/faces
			kSphere,                ///< Sphere (radius only)
			kCapsule,               ///< Capsule / pill shape (line segment + radius)
			kTriangle,              ///< Single triangle
			kCompressedMesh,        ///< Compressed triangle mesh (most world geometry)
			kExternMesh,            ///< External mesh reference
			kStaticCompound,        ///< Static compound of child shapes
			kDynamicCompound,       ///< Dynamic compound (children can move)
			kHeightField,           ///< Terrain height field
			kCompressedHeightField, ///< Compressed terrain
			kScaledConvex,          ///< Uniformly scaled convex shape
			kMasked,                ///< Shape with disabled regions
			kMaskedCompound,        ///< Compound with masked children
			kLOD,                   ///< Level-of-detail shape
			kDummy,                 ///< Placeholder shape
			kUser0,                 ///< User-defined type 0
			kUser1,                 ///< User-defined type 1
			kUser2,                 ///< User-defined type 2
			kUser3,                 ///< User-defined type 3

			kTotal,

			kInvalid
		};
	};
	static_assert(std::is_empty_v<hknpShapeType>);

	/// hknpShape — abstract base class for all Havok 2014 collision shapes.
	///
	/// Memory layout: 0x30 bytes (16-byte aligned).
	/// Inherits hkReferencedObject (ref count at +0x08, size at +0x0C).
	class __declspec(novtable) alignas(0x10) hknpShape :
		public hkReferencedObject  // 00
	{
	public:
		static constexpr auto RTTI{ RTTI::hknpShape };
		static constexpr auto VTABLE{ VTABLE::hknpShape };

		/// Shape capability flags — indicate what operations a shape supports.
		enum class FlagsEnum
		{
			kIsConvexShape = 1u << 0,        ///< Shape is a convex primitive
			kIsConvexPolytopeShape = 1u << 1, ///< Shape has explicit vertex/face data
			kIsCompositeShape = 1u << 2,      ///< Shape contains child shapes
			kIsHeightFieldShape = 1u << 3,    ///< Shape is a height field
			kUseSinglePointManifold = 1u << 4, ///< Use single-point contact manifold
			kIsInteriorTriangle = 1u << 5,
			kSupportsCollisionsWithInteriorTriangles = 1u << 6,
			kUseNormalToFindSupportPlane = 1u << 7,
			kUseSmallFaceIndices = 1u << 8,
			kNoGetShapeKeysOnSPU = 1u << 9,
			kShapeNotSupportedOnSPU = 1u << 10,
			kIsTriangleOrQuadShape = 1u << 11,
			kIsQuadShape = 1u << 12
		};

		struct BuildSurfaceGeometryConfig;
		struct GetShapeKeysConfig;
		struct MassConfig;
		struct SdfContactPoint;
		struct SdfQuery;

		// Virtual methods (vtable indices relative to hkBaseObject)
		virtual hknpShapeType::Enum GetType() const;                                                                                                                                                                                                                                                                                                          // 04
		virtual std::int32_t        CalcSize() const;                                                                                                                                                                                                                                                                                                         // 05
		virtual void                CalcAabb(const hkTransformf& a_transform, hkAabb& a_aabbOut) const;                                                                                                                                                                                                                                                       // 06
		virtual hknpShapeSignals*   GetMutationSignals();                                                                                                                                                                                                                                                                                                     // 07
		virtual std::int32_t        GetNumberOfSupportVertices() const;                                                                                                                                                                                                                                                                                       // 08
		virtual const hkcdVertex*   GetSupportVertices(hkcdVertex* a_vertexBuffer, std::int32_t a_bufferSize) const;                                                                                                                                                                                                                                          // 09
		virtual void                GetSupportingVertex(const hkVector4f& a_direction, hkcdVertex* a_vertexBufferOut) const;                                                                                                                                                                                                                                  // 0A
		virtual void                ConvertVertexIdsToVertices(const std::uint8_t* a_ids, std::int32_t a_numIDs, hkcdVertex* a_verticesOut) const;                                                                                                                                                                                                            // 0B
		virtual std::int32_t        GetNumberOfFaces() const;                                                                                                                                                                                                                                                                                                 // 0C
		virtual void                GetFaceInfo(std::int32_t a_faceID, hkVector4f& a_planeOut, std::int32_t& a_minAngleOut) const;                                                                                                                                                                                                                            // 0D
		virtual std::int32_t        GetFaceVertices(std::int32_t a_faceID, hkVector4f& a_planeOut, hkcdVertex* a_vertexBufferOut) const;                                                                                                                                                                                                                      // 0E
		virtual std::int32_t        GetSupportingFace(hkVector4f& a_direction, hkVector4f& a_surfacePoint, const hkcdGsk::Cache* a_gskCache, bool a_useB, std::uint32_t a_prevFaceID, hkVector4f& a_planeOut, std::int32_t& a_minAngleOut) const;                                                                                                             // 0F
		virtual float               CalcMinAngleBetweenFaces() const;                                                                                                                                                                                                                                                                                         // 10
		virtual void                GetSignedDistances(const SdfQuery& a_query, SdfContactPoint* a_contactsOut) const;                                                                                                                                                                                                                                        // 11
		virtual std::int32_t        GetSignedDistanceContacts(const hknpCdBody& a_queryBody, float a_maxDistance, std::int32_t a_vertexIdOffset, hkBlockStream<SdfContactPoint>::Writer& a_contactPointsOut) const;                                                                                                                                           // 12
		virtual hknpShapeKeyMask*   CreateShapeKeyMask() const;                                                                                                                                                                                                                                                                                               // 13
		virtual std::int32_t        GetShapeKeys(std::uint32_t* a_shapeKeysOut, std::int32_t a_capacity, std::uint32_t a_previousKey, const GetShapeKeysConfig& a_config) const;                                                                                                                                                                              // 14
		virtual std::int32_t        GetLeafShapes(const std::uint32_t* a_keys, std::int32_t a_numKeys, hknpShapeCollector* a_collector) const;                                                                                                                                                                                                                // 15
		virtual void                CastRayImpl(hknpCollisionQueryContext* a_queryContext, const hknpRayCastQuery& a_query, const hknpQueryFilterData& a_targetShapeFilterData, const hknpShapeQueryInfo& a_targetShapeInfo, hknpCollisionQueryCollector* a_collector) const;                                                                                 // 16
		virtual void                QueryPointImpl(hknpCollisionQueryContext* a_queryContext, const hknpPointQuery& a_query, const hknpQueryFilterData& a_targetShapeFilterData, const hknpShapeQueryInfo& a_targetShapeInfo, hknpCollisionQueryCollector* a_collector) const;                                                                                // 17
		virtual void                QueryAabbImpl(hknpCollisionQueryContext* a_queryContext, const hknpAabbQuery& a_query, const hknpShapeQueryInfo& a_queryShapeInfo, const hknpQueryFilterData& a_targetShapeFilterData, const hknpShapeQueryInfo& a_targetShapeInfo, hkArray<std::uint32_t, hkContainerHeapAllocator>* a_hits, hkAabb* a_nmpInOut) const;  // 19
		virtual void                QueryAabbImpl(hknpCollisionQueryContext* a_queryContext, const hknpAabbQuery& a_query, const hknpShapeQueryInfo& a_queryShapeInfo, const hknpQueryFilterData& a_targetShapeFilterData, const hknpShapeQueryInfo& a_targetShapeInfo, hknpCollisionQueryCollector* a_collector, hkAabb* a_nmpInOut) const;                  // 18
		virtual std::int32_t        SelectLevelOfDetail(const hknpSolverInfo& a_si, const hknpCdBody& a_partner, std::int32_t a_previousLevelOfDetail) const;                                                                                                                                                                                                 // 1A
		virtual void                BuildMassProperties(const MassConfig& a_massConfig, hkDiagonalizedMassProperties& a_massPropertiesOut) const;                                                                                                                                                                                                             // 1B
		virtual hkResult            BuildSurfaceGeometry(const BuildSurfaceGeometryConfig& a_config, hkGeometry* a_geometryOut) const;                                                                                                                                                                                                                        // 1C
		virtual void                CheckConsistency() const;                                                                                                                                                                                                                                                                                                 // 1D

		// members
		hkFlags<FlagsEnum, std::uint16_t>                     flags;            // 10 — shape capability flags
		std::uint8_t                                          numShapeKeyBits;  // 12 — bits used for shape key encoding
		hkEnum<hknpCollisionDispatchType::Enum, std::uint8_t> dispatchType;     // 13 — collision algorithm selection
		float                                                 convexRadius;     // 14 — convex radius (margin for GJK)
		std::uintptr_t                                        userData;         // 18 — per-shape user data (often 0)
		hkRefCountedProperties*                               properties;       // 20 — optional ref-counted properties
	};
	static_assert(sizeof(hknpShape) == 0x30);
}
