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
float flashEnd = 1.1f;
float timer = 0.0f;
bool flashOn = true;
int leftStart;
int rightStart;
int rightScroll;
int leftScroll;
bool charTyped = false;
bool fullyTyped = false;
float keyTimer = 0.0f;
float keyPress = 0.08f;
int keyCount = 0;
float screenTimer = 0.0f;
float screenTimerStart = 5.0f;
bool timeReached = false;
int finalStats = 3;
int StatIdx = 0;
bool resetTyping = false;

std::string TypewriterEffect(entt::registry& registry,std::string& dest, std::string& text);
std::string countLives(entt::registry& registry);

void InitializeUIOverlays(entt::registry& registry, entt::entity entity) {
	std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
	int windowWidth = (*config).at("Window").at("width").as<int>();
	int windowHeight = (*config).at("Window").at("height").as<int>();
	rightScroll = (windowWidth / 2) + 45;
	leftScroll = (windowWidth / 3) + 55;
	leftStart = 0;
	rightStart = windowWidth;
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
	font.DrawTextImmediate(W - 75, H - 20, "RRR", 3);
	std::string score = std::to_string(Scorring.GetScore());
	font.DrawTextImmediate((W / 2), 60, score.c_str(), score.length());
	font.DrawTextImmediate((W / 8) + 15, 60, score.c_str(), score.length());
	font.DrawTextImmediate(5, H - 20, countLives(registry).c_str(), countLives(registry).length());
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
	if (rightStart > rightScroll) {
		rightStart -= deltaTime;
	}
	if (leftStart < leftScroll) {
		leftStart += deltaTime * 300;
	}
	font.DrawTextImmediate(leftStart, H - 70, "CRIMSON ", 8);
	font.DrawTextImmediate(rightStart, H - 70, "MILLENIA", 8);
	font.DrawTextImmediate((W / 2) - 35, (H / 2), "2851", 4);
	if (timer <= flashEnd && flashOn) {
		timer += deltaTime;
		font.DrawTextImmediate((W / 2) - 100, (H / 2) + 100, "PRESS START", 11);
	}
	else {
		flashOn = false;
	}
	if (timer >= 0.0f && !flashOn){
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
	font.DrawTextImmediate(W - 75, H - 20, "RRR", 3);
	std::string score = std::to_string(Scorring.GetScore());
	font.DrawTextImmediate((W / 2), 60, score.c_str(), score.length());
	font.DrawTextImmediate(5, H - 20, countLives(registry).c_str(), countLives(registry).length());
	std::string statHeader;
	std::string proficiency = "TERMINATING CRAFTS 00%";
	TypewriterEffect(registry, statHeader, proficiency);
	if (statHeader == proficiency) {
		font.DrawTextImmediate((W / 2) - 50, (H / 2) + 50, "BONUS", 5);
		font.DrawTextImmediate((W / 2) - 75, (H / 2) + 100, "0000 PTS", 8);
		font.DrawTextImmediate((W / 3) + 25, (H / 2) + 150, "R X 1000 = 3000 PTS", 19);
	}
	font.DrawTextImmediate((W / 4) + 25, (H / 2) - 50, statHeader.c_str(), statHeader.length());
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void StartOfLevel(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	bltr.ClearColor(0x00000000);
	font.DrawTextImmediate(W / 8, 25, "1UP", 3);
	font.DrawTextImmediate(W - (W / 6), 25, "2UP", 3);
	font.DrawTextImmediate((W / 2) - 100, 25, "HIGH SCORE", 10);
	std::string score = std::to_string(Scorring.GetScore());
	font.DrawTextImmediate((W / 2), 60, score.c_str(), score.length());
	font.DrawTextImmediate((W / 8) + 15, 60, score.c_str(), score.length());
	font.DrawTextImmediate((W / 2) - 35, (H / 2) + 50, "Rank", 4);
	font.DrawTextImmediate((W / 2) - 125, (H / 2) + 100, "LAST ## STAGE", 13);
	font.DrawTextImmediate((W / 2) - 50, (H / 2) + 150, "READY", 5);
	font.DrawTextImmediate((W / 2) - 75, (H / 2) + 200, "PLAYER 1", 8);
	font.DrawTextImmediate(W - 75, H - 20, "RRR", 3);
	font.DrawTextImmediate(5, H - 20, countLives(registry).c_str(), countLives(registry).length());
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void WinScreen(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	bltr.ClearColor(0x00000000);
	font.DrawTextImmediate(W / 8, 25, "1UP", 3);
	font.DrawTextImmediate(W - (W / 6), 25, "2UP", 3);
	font.DrawTextImmediate((W / 2) - 100, 25, "HIGH SCORE", 10);
	font.DrawTextImmediate(W - 75, H - 20, "RRR", 3);
	font.DrawTextImmediate(5, H - 20, countLives(registry).c_str(), countLives(registry).length());
	std::string score = std::to_string(Scorring.GetScore());
	font.DrawTextImmediate((W / 2), 60, score.c_str(), score.length());
	font.DrawTextImmediate((W / 8) + 15, 60, score.c_str(), score.length());
	std::string statHeader;
	std::string proficiency = "TERMINATING CRAFTS    0000";
	std::string statHeader2;
	std::string proficiency2 = "PERCENTAGE      00%";
	std::string statHeader3;
	std::string proficiency3 = "TODAY'S TOP      00%";
	
	if (StatIdx != finalStats) {
		if (screenTimer < screenTimerStart && !timeReached) {
			screenTimer += deltaTime;
		}
		else if(!timeReached){
			timeReached = true;
		}
		if (timeReached && screenTimer > 0.0f) {
			screenTimer = 0.0f;
			StatIdx++;
		}
		else {
			timeReached = false;
		}
	}
	if (StatIdx >= 1) {
		TypewriterEffect(registry, statHeader, proficiency);
		font.DrawTextImmediate((W / 4), (H / 2) - 200, statHeader.c_str(), statHeader.length());
	}
	if (StatIdx >= 2) {
		TypewriterEffect(registry, statHeader2, proficiency2);
		font.DrawTextImmediate((W / 4), (H / 2) - 150, statHeader2.c_str(), statHeader2.length());
	}
	if (StatIdx >= 3) {
		TypewriterEffect(registry, statHeader3, proficiency3);
		font.DrawTextImmediate((W / 4), (H / 2) - 100, statHeader3.c_str(), statHeader3.length());
	}
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
		StartOfLevel(registry, ovl, bltr, font, W, H);
	break;
	case 2:
		GameplayUI(registry, ovl, bltr, font, W, H);
	break;
	case 3:
		EndOfLevel(registry, ovl, bltr, font, W, H);
	break;
	case 4:
		WinScreen(registry, ovl, bltr, font, W, H);
	break;
	case 5:

	break;
	case 6:

	break;
	case 7:

	break;
	}
}

std::string TypewriterEffect(entt::registry& registry, std::string& dest, std::string& text) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	if (!resetTyping) {
		if (keyCount != text.length() && !fullyTyped) {
			if (keyTimer < keyPress && !charTyped) {
				keyTimer += deltaTime;
			}
			else if (!charTyped) {
				charTyped = true;
				keyCount++;
			}
			if (charTyped && keyTimer > 0.0f) {
				keyTimer = 0.0f;
			}
			else {
				charTyped = false;
			}
			for (int i = 0; i < keyCount; i++) {
				dest += text[i];
			}
		}
		else {
			dest = text;
			fullyTyped = true;
			resetTyping = true;
		}
		return dest;
	}
	else {
		keyCount = 0;
		return dest;
	}
}

std::string countLives(entt::registry& registry) {
	std::string hits;
	auto player = registry.view<GAME::Player>();
	for (auto entity : player) {
		auto hitpoints = registry.get<GAME::Health>(entity).HP;
		for (int i = 0; i < hitpoints; i++) {
			hits[i] = '^';
		}
	}
	return hits;
}