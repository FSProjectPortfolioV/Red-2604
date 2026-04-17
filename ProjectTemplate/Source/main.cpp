// main entry point for the application
// enables components to define their behaviors locally in an .hpp file
#include "CCL.h"
#include "UTIL/Utilities.h"
// include all components, tags, and systems used by this program
#include "DRAW/DrawComponents.h"
#include "GAME/GameComponents.h"
#include "APP/Window.hpp"
#include "DRAW/CloneEntity.h"
#include "GAME/Gameplay/ScoreSystem/ScoreSystem.h"



// Local routines for specific application behavior
void GraphicsBehavior(entt::registry& registry);
void GameplayBehavior(entt::registry& registry);
void MainLoopBehavior(entt::registry& registry);

// Architecture is based on components/entities pushing updates to other components/entities (via "patch" function)
int main()
{

	// All components, tags, and systems are stored in a single registry
	entt::registry registry;	

	registry.ctx().emplace<ScoreSystem>();

	// initialize the ECS Component Logic
	CCL::InitializeComponentLogic(registry);

	// Seed the rand
	unsigned int time = std::chrono::steady_clock::now().time_since_epoch().count();
	srand(time);

	registry.ctx().emplace<UTIL::Config>();

	GraphicsBehavior(registry); // create windows, surfaces, and renderers

	GameplayBehavior(registry); // create entities and components for gameplay
	
	MainLoopBehavior(registry); // update windows and input

	// clear all entities and components from the registry
	// invokes on_destroy() for all components that have it
	// registry will still be intact while this is happening
	registry.clear(); 

	return 0; // now destructors will be called for all components
}

// This function will be called by the main loop to update the graphics
// It will be responsible for loading the Level, creating the VulkanRenderer, and all VulkanInstances
void GraphicsBehavior(entt::registry& registry)
{
	std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;

	// Setup scroll speed
	float scrollSpeed = (*config).at("Global").at("scrollSpeed").as<float>();
	registry.ctx().emplace<GAME::ScrollingBackground>(scrollSpeed); // Make scroll speed available to anything with registry access

	// Add Gateware Audio System, for music and sound effects

	using namespace GW::AUDIO;
	GAudio& gAudio = registry.ctx().emplace<GAudio>();
	gAudio.Create();
	gAudio.SetMasterVolume(0.1f);

	GMusic& gMusic = registry.ctx().emplace<GMusic>();
	const char* bgMusic = (*config).at("Sounds").at("gpmusic").as<const char*>();
	gMusic.Create(bgMusic, gAudio, 0.1f);
	gMusic.Play(true);

	// Add an entity to handle all the graphics data
	auto display = registry.create();

	// Emplace CPULevel. Placing here to reduce occurrence of a json race condition crash
	registry.emplace<DRAW::CPULevel>(display, DRAW::CPULevel{(*config).at("Level1").at("levelFile").as<std::string>(), (*config).at("Level1").at("modelPath").as<std::string>()});

	// Emplace and initialize Window component
	int windowWidth = (*config).at("Window").at("width").as<int>();
	int windowHeight = (*config).at("Window").at("height").as<int>();
	int startX = (*config).at("Window").at("xstart").as<int>();
	int startY = (*config).at("Window").at("ystart").as<int>();
	registry.emplace<APP::Window>(display,
		APP::Window{ startX, startY, windowWidth, windowHeight, GW::SYSTEM::GWindowStyle::WINDOWEDBORDERED, "2851 by Crimson Millenia"});


	// Create the input
	auto& input =  registry.ctx().emplace<UTIL::Input>();
	auto& window = registry.get<GW::SYSTEM::GWindow>(display);
	window.ChangeWindowStyle(GW::SYSTEM::GWindowStyle::WINDOWEDLOCKED);
	input.bufferedInput.Create(window);
	input.immediateInput.Create(window);
	input.gamePads.Create();
	auto& pressEvents = registry.ctx().emplace<GW::CORE::GEventCache>();
	pressEvents.Create(32);
	input.bufferedInput.Register(pressEvents);
	input.gamePads.Register(pressEvents);

	// Create a transient component to initialize the Renderer
	std::string vertShader = (*config).at("Shaders").at("vertex").as<std::string>();
	std::string pixelShader = (*config).at("Shaders").at("pixel").as<std::string>();
	std::string starsVertexShader = (*config).at("Shaders").at("starVertex").as<std::string>();
	std::string starsFragmentShader = (*config).at("Shaders").at("starPixel").as<std::string>();
	registry.emplace<DRAW::VulkanRendererInitialization>(display,
		DRAW::VulkanRendererInitialization{ 
			vertShader, pixelShader, starsVertexShader, starsFragmentShader,
			{ {0.0f, 0.0f, 0.0f, 1} } , { 1.0f, 0u }, 75.f, 0.1f, 100.0f });
	registry.emplace<DRAW::VulkanRenderer>(display);
	
	// Emplace GPULevel
	registry.emplace<DRAW::GPULevel>(display);

	// Create a starfield entity
	entt::entity starEnt = registry.create();
	Construct_Starfield(registry, starEnt);

	// Attach GPU components to starfield entity
	registry.emplace<DRAW::StarfieldGPU>(starEnt);
	registry.emplace<DRAW::VulkanVertexBuffer>(starEnt);

	// Register for Vulkan clean up
	GW::CORE::GEventResponder shutdown;
	shutdown.Create([&](const GW::GEvent& e) {
		GW::GRAPHICS::GVulkanSurface::Events event;
		GW::GRAPHICS::GVulkanSurface::EVENT_DATA data;
		if (+e.Read(event, data) && event == GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES) {

			registry.clear<DRAW::Starfield>();
			registry.clear<DRAW::StarfieldGPU>();

			registry.clear<DRAW::VulkanVertexBuffer>();
			registry.clear<DRAW::VulkanIndexBuffer>();
			registry.clear<DRAW::VulkanGPUInstanceBuffer>();
			registry.clear<DRAW::VulkanUniformBuffer>();
			registry.clear<DRAW::VulkanRenderer>();
		}
		});

	registry.get<DRAW::VulkanRenderer>(display).vlkSurface.Register(shutdown);
	registry.emplace<GW::CORE::GEventResponder>(display, shutdown.Relinquish());

	// Create a camera and emplace it
	GW::MATH::GMATRIXF initialCamera;
	GW::MATH::GVECTORF translate = { 0.0f,  45.0f, -5.0f };
	GW::MATH::GVECTORF lookat = { 0.0f, 0.0f, 0.0f };
	GW::MATH::GVECTORF up = { 0.0f, 1.0f, 0.0f };
	GW::MATH::GMatrix::TranslateGlobalF(initialCamera, translate, initialCamera);
	GW::MATH::GMatrix::LookAtLHF(translate, lookat, up, initialCamera);
	// Inverse to turn it into a camera matrix, not a view matrix. This will let us do
	// camera manipulation in the component easier
	GW::MATH::GMatrix::InverseF(initialCamera, initialCamera);
	registry.emplace<DRAW::Camera>(display,
		DRAW::Camera{ initialCamera });
}

// This function will be called by the main loop to update the gameplay
// It will be responsible for updating the VulkanInstances and any other gameplay components
void GameplayBehavior(entt::registry& registry)
{
	if (!registry.ctx().contains<DRAW::ModelManager>())
		registry.ctx().emplace<DRAW::ModelManager>();

	std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;

	// Create player
	entt::entity player = registry.create();
	registry.emplace<GAME::Player>(player);
	auto& playerCollection = registry.emplace<DRAW::MeshCollection>(player);
	auto& playerTransform = registry.emplace<GAME::Transform>(player);
	registry.emplace<GAME::Collidable>(player);
	auto& pHP = registry.emplace<GAME::Health>(player);
	pHP.HP = (*config).at("Player").at("hitpoints").as<int>();

	// Create game manager
	entt::entity gm = registry.create();
	registry.emplace<GAME::GameManager>(gm);

	// Get model manager
	auto& manager = registry.ctx().get<DRAW::ModelManager>();

	// Look up model names from config
	std::string playerModelName = config->at("Player").at("model").as<std::string>();
	std::string enemyModelName = config->at("Enemy1").at("model").as<std::string>();

	// Clone meshes
	CloneModelToEntity(
		registry,
		manager.collections[playerModelName],
		playerCollection,
		playerTransform
	);

}

// This function will be called by the main loop to update the main loop
// It will be responsible for updating any created windows and handling any input
void MainLoopBehavior(entt::registry& registry)
{	
	// main loop
	int closedCount; // count of closed windows
	auto winView = registry.view<APP::Window>(); // for updating all windows
	auto& deltaTime = registry.ctx().emplace<UTIL::DeltaTime>().dtSec;
	// for updating all windows
	do {
		// Set the delta time
		static auto start = std::chrono::steady_clock::now();
		double elapsed = std::chrono::duration<double>(
			std::chrono::steady_clock::now() - start).count();
		start = std::chrono::steady_clock::now();
		// Cap delta time to min 30 fps. This will prevent too much time from simulating when dragging the window
		if(elapsed > 1.0 / 30.0)
		{
			elapsed = 1.0 / 30.0;
		}
		deltaTime = elapsed;

		//Update Game
		auto gmView = registry.view<GAME::GameManager>();
		for (auto gm : gmView)
			registry.patch<GAME::GameManager>(gm);

		//Update SideFighters
		auto sfView = registry.view<GAME::SideFighter>();
		for (auto entity : sfView)
			registry.patch<GAME::SideFighter>(entity);

		// Update Starfield
		auto starView = registry.view<DRAW::Starfield>();
		for (auto entity : starView)
		{
			auto& sf = registry.get<DRAW::Starfield>(entity);

			float dt = registry.ctx().get<UTIL::DeltaTime>().dtSec;

			for (auto& s : sf.stars)
			{
				// Move star down based on its speed and layer (parallax effect)
				float layerSpeed = s.speed;
				if (s.layer == 0) layerSpeed *= 0.5f; // background
				if (s.layer == 1) layerSpeed *= 1.0f; // mid
				if (s.layer == 2) layerSpeed *= 1.8f; // foreground
				
				float baseScroll = registry.ctx().get<GAME::ScrollingBackground>().scrollSpeed;
				s.position.y += baseScroll * s.speed * dt;

				// Horizontal sway using sine wave based on vertical position
				s.position.x += sinf(s.position.y * 5.0f) * 0.000225f;

				// Wrap when star goes below the screen
				if (s.position.y > 1.0f)
				{
					s.position.y = -1.0f;
					s.position.x = UTIL::RandomFloat(-1.0f, 1.0f);
					s.position.z = UTIL::RandomFloat(0.0f, 1.0f);
				}



				// Wrap horizontally as well, because why not?
				if (s.position.x < -1) s.position.x = 1;
				if (s.position.x > 1) s.position.x = -1;
				if (s.position.z < 0)  s.position.z = 1;
				if (s.position.z > 1)  s.position.z = 0;
			}

			std::vector<DRAW::StarVertex> gpuVerts;
			gpuVerts.reserve(sf.stars.size());

			for (auto& s : sf.stars)
				gpuVerts.push_back({ s.position, s.brightness, s.layer });
			registry.emplace_or_replace<std::vector<DRAW::StarVertex>>(entity, gpuVerts);
			registry.patch<DRAW::VulkanVertexBuffer>(entity);

			registry.get<DRAW::StarfieldGPU>(entity).starCount = gpuVerts.size();
		}



		closedCount = 0;
		// find all Windows that are not closed and call "patch" to update them
		for (auto entity : winView) {
			if (registry.any_of<APP::WindowClosed>(entity))
				++closedCount;
			else
				registry.patch<APP::Window>(entity); // calls on_update()
		}
	} while (winView.size() != closedCount); // exit when all windows are closed
}
