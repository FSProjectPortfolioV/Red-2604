#include "C:/FSU/dev4/dev4-starting-materials-v1-1-JOFullSail/ProjectTemplate/build/CMakeFiles/PPIV.dir/Release/cmake_pch.hxx"
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
