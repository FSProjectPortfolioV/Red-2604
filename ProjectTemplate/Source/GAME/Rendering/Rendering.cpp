#include "Rendering.h"
#include "../GameComponents.h"
#include "../../DRAW/DrawComponents.h"

void Rendering::SyncTransforms(entt::registry& registry)
{
	auto view = registry.view<GAME::Transform, DRAW::MeshCollection>();

	for (auto entity : view)
	{
		auto& transform = view.get<GAME::Transform>(entity);
		auto& collection = view.get<DRAW::MeshCollection>(entity);

		for (auto mesh : collection.meshEntities)
		{
			if (!registry.valid(mesh)) continue;

			auto& inst = registry.get<DRAW::GPUInstance>(mesh);
			inst.transform = transform.matrix;
		}
	}
}
