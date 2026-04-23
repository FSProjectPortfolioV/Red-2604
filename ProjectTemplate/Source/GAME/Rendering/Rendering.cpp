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

			auto& instance = registry.get<DRAW::GPUInstance>(mesh);

			instance.transform = transform.matrix;

			if (registry.all_of<GAME::Visible>(mesh))
			{
				if (!registry.get<GAME::Visible>(mesh).show)
				{
					GW::MATH::GVECTORF hideOffset = { 10000.0f, 10000.0f, 10000.0f, 0.0f };
					GW::MATH::GMatrix::TranslateGlobalF(instance.transform, hideOffset, instance.transform);
				}
			}
		}
	}
}
