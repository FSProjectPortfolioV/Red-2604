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

	struct TextureData {
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkDescriptorSet descriptorSet;
	};

	struct ModelManager
	{
		std::unordered_map<std::string, MeshCollection> collections;
		std::unordered_map<std::string, TextureData> textures;
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

		VkDescriptorSetLayout textureLayout;
		std::vector<VkDescriptorSet> descriptorSets;
		VkSampler textureSampler = VK_NULL_HANDLE;
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
		VkDescriptorSet textureDescriptor = VK_NULL_HANDLE;

		inline bool operator < (const GeometryData& a) const {
			if (indexStart != a.indexStart)
				return indexStart < a.indexStart;

			return textureDescriptor < a.textureDescriptor;
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
		int layer; // for parallax effect, 0 = farthest, higher = closer
	};

	struct StarVertex
	{
		GW::MATH::GVECTORF pos;
		float brightness;
		int layer;
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

		// Random layer distribution
		float r = UTIL::RandomFloat(0.0f, 1.0f);
		if (r < 0.8f)      s.layer = 0; // background
		else if (r < 0.9f) s.layer = 1; // mid
		else               s.layer = 2; // foreground

		// Clip-space spawn
		s.position = {
			UTIL::RandomFloat(-1.0f, 1.0f),
			UTIL::RandomFloat(-1.0f, 1.0f),
			UTIL::RandomFloat(0.0f, 1.0f),
			1.0f
		};

		// Layer-based speed
		if (s.layer == 0) s.speed = UTIL::RandomFloat(0.1f, 0.3f);
		if (s.layer == 1) s.speed = UTIL::RandomFloat(0.3f, 0.6f);
		if (s.layer == 2) s.speed = UTIL::RandomFloat(0.6f, 1.0f);

		// Layer-based brightness
		if (s.layer == 0) s.brightness = UTIL::RandomFloat(0.2f, 0.4f);
		if (s.layer == 1) s.brightness = UTIL::RandomFloat(0.4f, 0.7f);
		if (s.layer == 2) s.brightness = UTIL::RandomFloat(0.7f, 1.0f);

		sf.stars.push_back(s);
	}


}
#endif // !DRAW_COMPONENTS_H
