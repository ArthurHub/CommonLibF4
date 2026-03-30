#pragma once

// BSHavok.h — Bethesda's wrapper around the Havok 2014 physics world
//
// bhkWorld is the main Bethesda physics world object. It wraps hknpWorld and adds
// game-specific functionality (raycasting with NiAVObject resolution, collision
// layer management, physics step listeners, etc.).
//
// Getting a bhkWorld:
//   auto* cell = RE::PlayerCharacter::GetSingleton()->GetParentCell();
//   RE::bhkWorld* bhkWorld = cell->GetbhkWorld();
//
// Getting the underlying hknpWorld:
//   // The hknpWorld pointer is at bhkWorld+0x60
//   auto* hknpWorld = *reinterpret_cast<RE::hknpWorld**>(
//       reinterpret_cast<uintptr_t>(bhkWorld) + 0x60);
//
// Thread safety:
//   bhkWorld methods handle their own locking internally. When calling
//   bhkWorld::PickObject, the world read lock is acquired automatically.
//   For direct hknpWorld access, you may need to manage locking yourself.
//
// Key internal layout:
//   +0x18 = physics step listener array
//   +0x60 = hknpWorld* (the underlying Havok world)
//   +0x68..0x17F = remaining Bethesda-specific state (TODO: map fully)

#include "RE/NetImmerse/NiObject.h"
#include "RE/NetImmerse/NiPoint.h"

namespace RE
{
	struct bhkPickData;
	struct hknpWorldCinfo;
	class bhkIWorldStepListener;
	class hknpWorld;

	/// bhkWorld — Bethesda's game-level physics world wrapper.
	///
	/// One bhkWorld exists per loaded cell. It owns the hknpWorld and manages
	/// the lifecycle of all physics bodies in that cell.
	///
	/// When the player transitions between cells, the old bhkWorld is destroyed
	/// and a new one is created. Any custom physics bodies (e.g., hand colliders)
	/// must be destroyed before the old world goes away, and recreated in the new one.
	class bhkWorld :
		public NiObject  // 00
	{
	public:
		static constexpr auto RTTI{ RTTI::bhkWorld };
		static constexpr auto VTABLE{ VTABLE::bhkWorld };
		static constexpr auto Ni_RTTI{ Ni_RTTI::bhkWorld };

		// add
		virtual bool Update(std::uint32_t a_updateFlags);  // 28
		virtual void Init(const hknpWorldCinfo& a_info);   // 29

		/// Remove all physics objects attached to a NiAVObject (and optionally its children).
		///
		/// @param a_object   The scene graph node to clean up
		/// @param a_recurse  If true, also remove from all children recursively
		/// @param a_force    If true, force removal even if the object is in use
		/// @return True if any objects were removed
		static bool RemoveObjects(NiAVObject* a_object, bool a_recurse, bool a_force)
		{
			using func_t = decltype(&RemoveObjects);
			static REL::Relocation<func_t> func{ REL::RelocationID(1514984, 2277721) };
			return func(a_object, a_recurse, a_force);
		}

		/// Perform a raycast against the physics world.
		///
		/// This is the preferred raycast method — it handles world locking internally
		/// and resolves hit results to NiAVObject/hknpBody references.
		///
		/// CRASH WARNING: Calling hknpWorld::CastRay directly without holding the
		/// world read lock will crash. Always use this method instead.
		///
		/// @param a_pickData  Raycast parameters and results (see bhkPickData)
		/// @return True if any hit was found
		///
		/// Example:
		///   RE::bhkPickData pick;
		///   pick.SetStartEnd(eyePos, eyePos + lookDir * 1000.0f);
		///   pick.collisionFilter.filter = 0x02420028;  // ItemPicker layer
		///   if (bhkWorld->PickObject(pick)) {
		///       RE::NiAVObject* hitObj = pick.GetNiAVObject();
		///       float hitDist = pick.GetHitFraction() * 1000.0f;
		///   }
		bool PickObject(bhkPickData& a_pickData)
		{
			using func_t = decltype(&bhkWorld::PickObject);
			static REL::Relocation<func_t> func{ REL::ID(547249) };
			return func(this, a_pickData);
		}

		/// Register a listener that gets called every physics step.
		/// The listener array is stored at bhkWorld+0x18.
		///
		/// @param a_listener  Listener to add (must outlive the bhkWorld)
		void AddPhysicsStepListener(bhkIWorldStepListener& a_listener)
		{
			using func_t = decltype(&bhkWorld::AddPhysicsStepListener);
			static REL::Relocation<func_t> func{ REL::ID(137330) };
			return func(this, a_listener);
		}

		/// Set the physics simulation timestep.
		///
		/// @param a_deltaTime  Time step in seconds (typically 1/60 or 1/90 for VR)
		/// @param a_param2     Purpose not fully RE'd (typically false)
		/// @param a_param3     Purpose not fully RE'd (typically false)
		void SetDeltaTime(float a_deltaTime, bool a_param2, bool a_param3)
		{
			using func_t = decltype(&bhkWorld::SetDeltaTime);
			static REL::Relocation<func_t> func{ REL::ID(12890) };
			return func(this, a_deltaTime, a_param2, a_param3);
		}

		/// Get the world's gravity vector.
		/// Default is approximately {0, 0, -9.81} in Havok space.
		void GetGravity()
		{
			using func_t = decltype(&bhkWorld::GetGravity);
			static REL::Relocation<func_t> func{ REL::ID(386279) };
			return func(this);
		}

		/// Change the motion type of a NiAVObject's physics body.
		/// This is the Bethesda-level way to switch between STATIC/DYNAMIC/KEYFRAMED.
		///
		/// @param a_object  Scene graph object with attached physics
		/// @param a_type    Target motion type (STATIC=0, DYNAMIC=1, KEYFRAMED=2)
		/// @param a_param3  Purpose not fully RE'd
		/// @param a_param4  Purpose not fully RE'd
		/// @param a_param5  Purpose not fully RE'd
		void SetMotion(NiAVObject* a_object, hknpMotionPropertiesId::Preset a_type, bool a_param3, bool a_param4, bool a_param5)
		{
			using func_t = decltype(&bhkWorld::SetMotion);
			static REL::Relocation<func_t> func{ REL::ID(357289) };
			return func(this, a_object, a_type, a_param3, a_param4, a_param5);
		}

		/// Enable or disable collision for a NiAVObject's physics body.
		///
		/// @param a_object  Scene graph object with attached physics
		/// @param a_enable  True to enable collisions, false to disable
		/// @param a_param3  Purpose not fully RE'd
		/// @param a_param4  Purpose not fully RE'd
		void EnableCollision(NiAVObject* a_object, bool a_enable, bool a_param3, bool a_param4)
		{
			using func_t = decltype(&bhkWorld::EnableCollision);
			static REL::Relocation<func_t> func{ REL::ID(50037) };
			return func(this, a_object, a_enable, a_param3, a_param4);
		}

		/// Set the physics world origin offset.
		/// Used during cell transitions to re-center the physics world and prevent
		/// floating-point precision issues far from the origin.
		void SetOrigin(NiPoint3& a_origin)
		{
			using func_t = decltype(&bhkWorld::SetOrigin);
			static REL::Relocation<func_t> func{ REL::ID(54519) };
			return func(this, a_origin);
		}

		// members
		// +0x10: Start of Bethesda-specific members
		// +0x18: Physics step listener array
		// +0x60: hknpWorld* — the underlying Havok 2014 physics world
		// Remaining layout TODO — 0x170 bytes of unmapped state
		std::byte pad[0x180 - 0x10];  // 10
	};
	static_assert(sizeof(bhkWorld) == 0x180);
}
