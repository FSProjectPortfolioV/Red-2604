#pragma once
#include "../../entt-3.13.1/single_include/entt/entt.hpp"

struct DebugOverlay
{
	// Print FPS, enemy count, bullet count
	static void Draw(entt::registry& registry);
};

struct DebugDraw
{
	// WIP. Won't draw anything right now. Need to write Vulkan pipeline and shader code.
	static void Colliders(entt::registry& registry);
};

struct DebugLog
{
	static void Info(const std::string&);
	static void Warn(const std::string&);
	static void Error(const std::string&);
};

struct DebugInspector
{
	// Set this to whichever entity you want to inspect
	static entt::entity selected;

	// List the selected entity's components and their values
	static void Inspect(entt::registry& registry);
};

// Internal functions for DebugDraw. You won't call any of these yourself.
struct DebugRenderer
{
	static void DrawOBB(GW::MATH::GOBBF& obb);

	static std::vector<GW::MATH::GVECTORF> lineVertices;

	static void DrawLine(const GW::MATH::GVECTORF& a,
		const GW::MATH::GVECTORF& b);

	static void Flush();
};