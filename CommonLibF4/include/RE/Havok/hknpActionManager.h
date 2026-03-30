#pragma once

// hknpActionManager — Manages Havok actions (force appliers) in the physics world.
//
// Actions are custom force appliers that run every physics step. They are
// distinct from constraints — actions directly manipulate body velocities,
// while constraints are solved by the Havok constraint solver.
//
// The primary action used in FO4VR is hknpBSMouseSpringAction (the grab spring).
//
// The action manager is accessed via hknpWorld at an internal offset.
// Actions are registered/unregistered during grab/release.
//
// Key addresses (FO4VR):
//   AddAction:    VR 0x14155b6d0 (REL::ID 411727)
//   RemoveAction: VR 0x14155ba60 (REL::ID 104709)

namespace RE
{
	class hknpBSMouseSpringAction;

	/// hknpActionManager — registers and runs Havok actions each physics step.
	///
	/// Usage:
	///   // Get the action manager from the world (internal offset)
	///   // Register a spring action for grab:
	///   actionManager->AddAction(springAction);
	///   // Remove on release:
	///   actionManager->RemoveAction(springAction);
	class hknpActionManager
	{
	public:
		/// Register an action to run every physics step.
		/// The action's applyAction() method will be called each step until removed.
		///
		/// @param a_action  The action to register (e.g., hknpBSMouseSpringAction*)
		void AddAction(void* a_action)
		{
			using func_t = void(*)(hknpActionManager*, void*);
			static REL::Relocation<func_t> func{ REL::ID(411727) };
			func(this, a_action);
		}

		/// Remove a previously registered action.
		/// The action will no longer run during physics steps.
		///
		/// @param a_action  The action to remove
		void RemoveAction(void* a_action)
		{
			using func_t = void(*)(hknpActionManager*, void*);
			static REL::Relocation<func_t> func{ REL::ID(104709) };
			func(this, a_action);
		}
	};
}
