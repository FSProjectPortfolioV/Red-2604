#ifndef DRAW_COMPONENTS_H
#define DRAW_COMPONENTS_H

#include "./Utility/load_data_oriented.h"
#include "../UTIL/Utilities.h"

namespace DRAW
{
	//*** TAGS ***//
	struct DoNotRender {};

	//*** COMPONENTS ***//

	struct MeshCollection
	{
		std::vector<entt::entity> meshEntities;
		entt::entity parent = entt::null;
		GW::MATH::GOBBF collider;
	};

	struct ModelManager
	{
		std::unordered_map<std::string, MeshCollection> collections;
	};

	struct VulkanRendererInitialization
	{
		std::string vertexShaderName;
		std::string fragmentShaderName;
		std::string starsVertexShaderName;
		std::string starsFragmentShaderName;
		VkClearColorValue clearColor;
		VkClearDepthStencilValue depthStencil;
		float fovDegrees;
		float nearPlane;
		float farPlane;
	};

	struct VulkanRenderer
	{
		GW::GRAPHICS::GVulkanSurface vlkSurface;
		VkDevice device = nullptr;
		VkPhysicalDevice physicalDevice = nullptr;
		VkRenderPass renderPass;
		VkShaderModule vertexShader = nullptr;
		VkShaderModule starsVertexShader = nullptr;
		VkShaderModule fragmentShader = nullptr;
		VkShaderModule starsFragmentShader = nullptr;
		VkPipeline pipeline = nullptr;
		VkPipeline starPipeline = nullptr;
		VkPipelineLayout pipelineLayout = nullptr;
		VkPipelineLayout starPipelineLayout = nullptr;
		GW::MATH::GMATRIXF projMatrix;
		VkDescriptorSetLayout descriptorLayout = nullptr;
		VkDescriptorPool descriptorPool = nullptr;
		std::vector<VkDescriptorSet> descriptorSets;
		VkClearValue clrAndDepth[2];
	};

	struct VulkanVertexBuffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
	};

	struct VulkanIndexBuffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
	};

	struct GeometryData
	{
		unsigned int indexStart, indexCount, vertexStart;
		inline bool operator < (const GeometryData a) const {
			return indexStart < a.indexStart;
		}
	};

	struct GPUInstance
	{
		GW::MATH::GMATRIXF	transform;
		H2B::ATTRIBUTES		matData;
	};

	struct VulkanGPUInstanceBuffer
	{
		unsigned long long element_count = 1;
		std::vector<VkBuffer> buffer;
		std::vector<VkDeviceMemory> memory;
	};

	struct SceneData
	{
		GW::MATH::GVECTORF sunDirection, sunColor, sunAmbient, camPos;
		GW::MATH::GMATRIXF viewMatrix, projectionMatrix;
	};

	struct VulkanUniformBuffer
	{
		std::vector<VkBuffer> buffer;
		std::vector<VkDeviceMemory> memory;
	};


	struct Camera
	{
		GW::MATH::GMATRIXF camMatrix;
	};

	struct CPULevel
	{
		std::string levelPath;
		std::string modelFolder;
		Level_Data lvlData;
	};

	struct GPULevel
	{

	};

	struct Star
	{
		GW::MATH::GVECTORF position;
		float speed;
		float brightness;
	};

	struct StarVertex
	{
		GW::MATH::GVECTORF pos;
		float brightness;
	};

	struct Starfield
	{
		std::vector<Star> stars;
		float width = 20.0f;   // X range
		float height = 20.0f;  // Y range
		float depth = 100.0f;  // Z range
	};

	struct StarfieldGPU
	{
		size_t starCount = 0;
	};

} // namespace DRAW

static void Construct_Starfield(entt::registry& registry, entt::entity e)
{
	auto& sf = registry.emplace<DRAW::Starfield>(e);

	const int STAR_COUNT = 2000;

	sf.stars.reserve(STAR_COUNT);

	sf.width = 1.0f;  // NDC X range [-1, 1]
	sf.height = 1.0f;  // NDC Y range [-1, 1]
	sf.depth = 1.0f;  // NDC Z range [0, 1] after mapping

	for (int i = 0; i < STAR_COUNT; i++)
	{
		DRAW::Star s;
		s.position = {
			UTIL::RandomFloat(-sf.width, sf.width),   // x in [-1, 1]
			UTIL::RandomFloat(-sf.height, sf.height), // y in [-1, 1]
			UTIL::RandomFloat(0.0f, sf.depth),        // z in [0, 1]
			1.0f
		};

		s.speed = UTIL::RandomFloat(1.0f, 5.0f);
		s.brightness = UTIL::RandomFloat(0.5f, 1.0f);

		sf.stars.push_back(s);
	}

}
#endif // !DRAW_COMPONENTS_H
