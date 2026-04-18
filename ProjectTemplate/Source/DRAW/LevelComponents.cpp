#include "DrawComponents.h"
#include "../GAME/GameComponents.h"
#include "../CCL.h"
#include "Utility/TextureUTTL.h"

namespace DRAW
{
	void Construct_CPULevel(entt::registry& registry, entt::entity entity)
	{
		auto& cpuLevel = registry.get<CPULevel>(entity);

		GW::SYSTEM::GLog log;
		log.Create("lvlLog");
		log.EnableConsoleLogging(true);

		cpuLevel.lvlData.LoadLevel(cpuLevel.levelPath.c_str(), cpuLevel.modelFolder.c_str(), log);
	}

	void Construct_GPULevel(entt::registry& registry, entt::entity entity)
	{
		auto& gpuLevel = registry.get<CPULevel>(entity);

		registry.emplace<VulkanVertexBuffer>(entity);
		registry.emplace<std::vector<H2B::VERTEX>>(entity, gpuLevel.lvlData.levelVertices);
		registry.patch<VulkanVertexBuffer>(entity);

		registry.emplace<VulkanIndexBuffer>(entity);
		registry.emplace<std::vector<unsigned>>(entity, gpuLevel.lvlData.levelIndices);
		registry.patch<VulkanIndexBuffer>(entity);

		if (!registry.ctx().contains<ModelManager>())
			registry.ctx().emplace<ModelManager>();

		auto& manager = registry.ctx().get<ModelManager>();
		auto& levelData = gpuLevel.lvlData;
		auto& vulkanRenderer = registry.ctx().get<VulkanRenderer>();

		for (auto& obj : levelData.blenderObjects)
		{
			MeshCollection collection;
			auto& model = levelData.levelModels[obj.modelIndex];
			collection.collider = levelData.levelColliders[model.colliderIndex];

			// If this is a static collidable object, create a collision-only entity
			if (model.isCollidable && !model.isDynamic)
			{
				entt::entity wall = registry.create();
				registry.emplace<GAME::Collidable>(wall);
				registry.emplace<GAME::Obstacle>(wall);

				// Copy collider
				auto& wallCollection = registry.emplace<MeshCollection>(wall);
				wallCollection.collider = collection.collider;
				wallCollection.meshEntities = collection.meshEntities;
				wallCollection.parent = wall;

				// Copy transform
				auto& wallTransform = registry.emplace<GAME::Transform>(wall);
				wallTransform.matrix = levelData.levelTransforms[obj.transformIndex];
			}

			for (int meshIdx = 0; meshIdx < model.meshCount; ++meshIdx)
			{
				auto& meshInfo = levelData.levelMeshes[model.meshStart + meshIdx];
				auto& material = levelData.levelMaterials[meshInfo.materialIndex];
				auto newMesh = registry.create();

				auto& gpuInstance = registry.emplace<GPUInstance>(newMesh);
				gpuInstance.transform = levelData.levelTransforms[obj.transformIndex];
				gpuInstance.matData = material.attrib;

				auto& geoData = registry.emplace<GeometryData>(newMesh);
				geoData.indexCount = meshInfo.drawInfo.indexCount;
				geoData.indexStart = model.indexStart + meshInfo.drawInfo.indexOffset;
				geoData.vertexStart = model.vertexStart;

				if (material.map_Kd[0] != '\0')
				{
					std::string texName = material.map_Kd;

					if (manager.textures.find(texName) == manager.textures.end())
					{
						TextureData newTex;
						std::string fullPath = gpuLevel.modelFolder + "/" + texName;
						UploadTextureToGPU(vulkanRenderer.vlkSurface, fullPath,
							newTex.memory, newTex.image, newTex.view, false);

						manager.textures[texName] = newTex;
					}

					geoData.textureView = manager.textures[texName].view;
				}

				if (model.isDynamic)
				{
					registry.emplace<DoNotRender>(newMesh);
					collection.meshEntities.push_back(newMesh);
				}
			}

			if (model.isDynamic)
			{
				// This is a dynamic object (player/enemy)
				// Create a parent entity for collision
				entt::entity dyn = registry.create();
				registry.emplace<GAME::Collidable>(dyn);

				auto& dynCollection = registry.emplace<DRAW::MeshCollection>(dyn);
				dynCollection.collider = levelData.levelColliders[model.colliderIndex];

				dynCollection.parent = dyn;

				auto& dynTransform = registry.emplace<GAME::Transform>(dyn);
				dynTransform.matrix = levelData.levelTransforms[obj.transformIndex];

				// Store mesh entities for rendering
				collection.parent = dyn;
				manager.collections[obj.blendername] = collection;
			}
		}
	}

	void OnDestroy_MeshCollection(entt::registry& registry, entt::entity entity)
	{
		auto& collection = registry.get<MeshCollection>(entity);
		for (auto mesh : collection.meshEntities)
		{
			if (registry.valid(mesh))
				registry.destroy(mesh);
		}
	}

	// Use this MACRO to connect the EnTT Component Logic
	CONNECT_COMPONENT_LOGIC() 
	{
		// register the Window component's logic
		registry.on_construct<CPULevel>().connect<Construct_CPULevel>();
		registry.on_construct<GPULevel>().connect<Construct_GPULevel>();
		registry.on_destroy<MeshCollection>().connect<OnDestroy_MeshCollection>();
	}

} // namespace DRAW