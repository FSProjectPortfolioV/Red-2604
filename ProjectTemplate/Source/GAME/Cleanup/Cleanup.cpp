#include "Cleanup.h"
#include "../GameComponents.h"

void Cleanup::Destroy(entt::registry& registry)
{
	auto toDestroy = registry.view<GAME::ToDestroy>();
	for (auto ent : toDestroy)
	{
		registry.destroy(ent);
	}
}
