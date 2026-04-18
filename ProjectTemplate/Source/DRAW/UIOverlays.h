#pragma once
#include "../GAME/GameComponents.h"
#include "../GAME/GamePlay/ScoreSystem/ScoreSystem.h"
#include "./Utility/FileIntoString.h"
#include "shaderc/shaderc.h"
#include "./Overlay.h"
#include "./Font.h"
#include "./BLIT_Font.h"
#include "../UTIL/Utilities.h"

void InitializeUIOverlays(entt::registry& registry, entt::entity entity) {
	std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
	int windowWidth = (*config).at("Window").at("width").as<int>();
	int windowHeight = (*config).at("Window").at("height").as<int>();
	auto& win = registry.get<GW::SYSTEM::GWindow>(entity);
	auto& surface = registry.get<DRAW::VulkanRenderer>(entity).vlkSurface;

	auto& overlay = registry.emplace<Overlay>(entity, windowWidth, windowHeight, win, surface, 0);
	auto& blitter = registry.emplace<GW::GRAPHICS::GBlitter>(entity);
	blitter.Create(windowWidth, windowHeight);
	auto& font = registry.emplace<BLIT_Font>(entity, blitter, "../Source/DRAW/font.tga", font_Arial);
}

static void GameplayUI(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	bltr.ClearColor(0x00000000);
	font.DrawTextImmediate(W / 8, 25, "1UP", 3);
	font.DrawTextImmediate(W - (W / 6), 25, "2UP", 3);
	font.DrawTextImmediate((W / 2) - 100, 25, "HIGH SCORE", 10);
	std::string hits;
	auto player = registry.view<GAME::Player>();
	for (auto entity : player) {
		auto hitpoints = registry.get<GAME::Health>(entity).HP;
		for (int i = 0; i < hitpoints; i++) {
			hits[i] = '^';
		}
	}
	font.DrawTextImmediate(5, H - 20, hits.c_str(), 7);
	font.DrawTextImmediate(W - 75, H - 20, "RRR", 3);
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

void UpdateUIOverlays(entt::registry& registry, int state, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	switch (state) {
	case 0:

	break;
	case 1:
		GameplayUI(registry, ovl, bltr, font, W, H);
	break;
	case 2:

	break;
	case 3:

	break;
	case 4:

	break;
	case 5:

	break;
	case 6:

	break;
	case 7:

	break;
	}
}