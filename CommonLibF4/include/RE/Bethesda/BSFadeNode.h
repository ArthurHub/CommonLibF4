#pragma once

#include "RE/Bethesda/BSTArray.h"
#include "RE/NetImmerse/NiNode.h"
#include "RE/NetImmerse/NiSmartPointer.h"

namespace RE
{
	class BSGeometry;
	class BSLight;

	class BSShaderPropertyLightData
	{
	public:
		// members
		std::uint32_t      lightListFence;    // 00
		std::uint32_t      shadowAccumFlags;  // 04
		std::uint32_t      lightListChanged;  // 08
		BSTArray<BSLight*> lightList;         // 10
	};
	static_assert(sizeof(BSShaderPropertyLightData) == 0x28);

	class __declspec(novtable) BSFadeNode :
		public NiNode  // 000
	{
	public:
		static constexpr auto RTTI{ RTTI::BSFadeNode };
		static constexpr auto VTABLE{ VTABLE::BSFadeNode };
		static constexpr auto Ni_RTTI{ Ni_RTTI::BSFadeNode };

		struct FlattenedGeometryData
		{
		public:
			// members
			NiBound               bound;     // 00
			NiPointer<BSGeometry> geometry;  // 10
			std::uint32_t         flags;     // 18
		};
		static_assert(sizeof(FlattenedGeometryData) == 0x20);

		struct RUNTIME_DATA
		{
#define RUNTIME_DATA_CONTENT                                                                         \
	BSShaderPropertyLightData                                        lightData;              /*140*/ \
	BSTArray<FlattenedGeometryData>                                  geomArray;              /*168*/ \
	BSTArray<NiBound, BSTAlignedHeapArrayAllocator<0x10>::Allocator> mergedGeomBounds;       /*180*/ \
	float                                                            nearDistSqr;            /*198*/ \
	float                                                            farDistSqr;             /*19C*/ \
	float                                                            currentFade;            /*1A0*/ \
	float                                                            currentDecalFade;       /*1A4*/ \
	float                                                            boundRadius;            /*1A8*/ \
	float                                                            timeSinceUpdate;        /*1AC*/ \
	std::int32_t                                                     frameCounter;           /*1B0*/ \
	float                                                            previousMaxA;           /*1B4*/ \
	float                                                            currentShaderLODLevel;  /*1B8*/ \
	float                                                            previousShaderLODLevel; /*1BC*/
            RUNTIME_DATA_CONTENT
		};
		static_assert(sizeof(RUNTIME_DATA) == 0x80);

		[[nodiscard]] inline RUNTIME_DATA& GetRuntimeData() noexcept
		{
			return REL::RelocateMember<RUNTIME_DATA>(this, 0x140, 0x180);
		}

		[[nodiscard]] inline const RUNTIME_DATA GetRuntimeData() const noexcept
		{
			return REL::RelocateMember<RUNTIME_DATA>(this, 0x140, 0x180);
		}

		// members
#if !defined(ENABLE_FALLOUT_VR) || ((!defined(ENABLE_FALLOUT_NG) && !defined(ENABLE_FALLOUT_F4)))
		RUNTIME_DATA_CONTENT
#endif
	};
#if !defined(ENABLE_FALLOUT_VR)
	static_assert(sizeof(BSFadeNode) == 0x1C0);
#elif (!defined(ENABLE_FALLOUT_NG) && !defined(ENABLE_FALLOUT_F4))
	static_assert(sizeof(BSFadeNode) == 0x200);
#endif
}
#undef RUNTIME_DATA_CONTENT
