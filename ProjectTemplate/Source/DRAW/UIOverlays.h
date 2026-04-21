#pragma once
#include "../GAME/GameComponents.h"
#include "../GAME/GamePlay/ScoreSystem/ScoreSystem.h"
#include "./Utility/FileIntoString.h"
#include "shaderc/shaderc.h"
#include "./Overlay.h"
#include "./Font.h"
#include "./BLIT_Font.h"
#include "../UTIL/Utilities.h"

ScoreSystem Scorring;
float flashStart = 0.0f;
float flashEnd = 1.1f;
float timer = 0.0f;
bool flashOn = true;
int leftStart;
float rightStart;
int rightScroll;
int leftScroll;
bool typed = false;
float keyTimer = 0.0f;
float keyPress = 2.0f;
int keyCount = 0;

void InitializeUIOverlays(entt::registry& registry, entt::entity entity) {
	std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
	int windowWidth = (*config).at("Window").at("width").as<int>();
	int windowHeight = (*config).at("Window").at("height").as<int>();
	rightScroll = (windowWidth / 2) - 100;
	leftScroll = (windowWidth / 2) + 25;
	leftStart = windowWidth;
	rightStart = 0;
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
	std::string score = std::to_string(Scorring.GetScore());
	font.DrawTextImmediate((W / 2), 60, score.c_str(), score.length());
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

static void StartMenu(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	bltr.ClearColor(0x00000000);
	font.DrawTextImmediate(W / 8, 25, "1UP", 3);
	font.DrawTextImmediate(W - (W / 6), 25, "2UP", 3);
	font.DrawTextImmediate((W / 2) - 100, 25, "HIGH SCORE", 10);
	font.DrawTextImmediate((W / 2) - 150, H - 25, "2026 CRIMSON MILLENIA", 21);
	if (rightStart < rightScroll) {
		rightStart += deltaTime * 190;
	}
	if (leftStart != leftScroll) {
		leftStart -= deltaTime * 10;
	}
	font.DrawTextImmediate(rightStart, H - 70, "CRIMSON ", 8);
	font.DrawTextImmediate(leftStart, H - 70, "MILLENIA", 8);
	font.DrawTextImmediate((W / 2) - 35, (H / 2), "2851", 4);
	//font.DrawTextImmediate((W / 3), (H / 3),		"_____________      ===    =====  ==", 29);
	//font.DrawTextImmediate((W / 3), (H / 3) - 20,  "/             |=====   ==", 28);
	//font.DrawTextImmediate((W / 3), (H / 3) - 20, "/      ___     |=====   ==", 28);
	//font.DrawTextImmediate((W / 3), (H / 3) - 20, "|      |  |    | =====   ==", 28);
	//font.DrawTextImmediate((W / 3), (H / 3) - 20, "____      ===     =====   ==", 28);
	//font.DrawTextImmediate((W / 3), (H / 3) - 20,	"===      ===     =====   ==", 28);
	//font.DrawTextImmediate((W / 3), (H / 3) - 20,	"===      ===     =====   ==", 28);
	//font.DrawTextImmediate((W / 3), (H / 3) - 20,	"===      ===     =====   ==", 28);
	if (timer <= flashEnd && flashOn) {
		timer += deltaTime;
		font.DrawTextImmediate((W / 2) - 100, (H / 2) + 100, "PRESS START", 11);
	}
	else {
		flashOn = false;
	}
	if (timer >= flashStart && !flashOn){
		timer -= deltaTime;
	}
	else {
		flashOn = true;
	}
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr); 
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void EndOfLevel(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	bltr.ClearColor(0x00000000);
	font.DrawTextImmediate(W / 8, 25, "1UP", 3);
	font.DrawTextImmediate(W - (W / 6), 25, "2UP", 3);
	font.DrawTextImmediate((W / 2) - 100, 25, "HIGH SCORE", 10);
	std::string score = std::to_string(Scorring.GetScore());
	font.DrawTextImmediate((W / 2), 60, score.c_str(), score.length());
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
	std::string statHeader;
	std::string proficiency = "TERMINATING CRAFTS 00%";
	if (keyCount != proficiency.length()) {
		if (keyTimer < keyPress && !typed) {
			keyTimer += deltaTime;
			for (int i = 0; i < keyCount; i++) {
				statHeader += proficiency[i];
			}
			keyCount++;
		}
		else {
			typed = true;
			keyTimer -= deltaTime;
			if (keyTimer <= 0.04f) {
				keyTimer = 0.0f;
				typed = false;
			}
		}
	}
	else {
		statHeader = proficiency;
	}
	font.DrawTextImmediate((W / 4) + 25, (H / 2) - 50, statHeader.c_str(), statHeader.length());
	font.DrawTextImmediate((W / 2) - 50, (H / 2) + 50, "BONUS", 5);
	font.DrawTextImmediate((W / 2) - 75, (H / 2) + 100, "0000 PTS", 8);
	font.DrawTextImmediate((W / 3) + 25, (H / 2) + 150, "R X 1000 = 3000 PTS", 19);
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

void UpdateUIOverlays(entt::registry& registry, int state, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	switch (state) {
	case 0:
		StartMenu(registry, ovl, bltr, font, W, H);
	break;
	case 1:
		GameplayUI(registry, ovl, bltr, font, W, H);
	break;
	case 2:
		EndOfLevel(registry, ovl, bltr, font, W, H);
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