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
float flashTimer = 0.0f;
int leftStart;
int rightStart;
int rightScroll;
int leftScroll;
float keyTimer = 0.0f;
float keyPress = 0.08f;
float screenTimer = 0.0f;
float screenTimerStart = 3.0f;
float screenTransition = 4 * screenTimerStart;
float screenTransitionStart = 0.0f;
int FinaleIdx = 1;
int FinaleIdx2 = 1;

std::string FinalStats[]{
	"TERMINATING CRAFTS",
	"PERCENTAGE",
	"TODAY'S TOP"
};

std::string LevelStats[]{
	"TERMINATING CRAFTS",
	"BONUS",
	"0000 PTS",
	"R X 1000 = 3000 PTS"
};

std::string LevelBegin[]{
	"Rank",
	"LAST ## STAGE",
	"READY",
	"PLAYER 1"
};

std::string UI[]{
	"1UP",
	"2UP",
	"HIGH SCORE",
};

std::string GameStart[]{
	"2026 CRIMSON MILLENIA",
	"CRIMSON ",
	"MILLENIA",
	"2851",
	"PRESS START"
};

std::string EndGame[]{
	"ENEMIES DESTROYED",
	"SPECIAL BONUS",
	"00, 000, 000 PTS",
	"BY CRIMSON MILLENIA",
	"PS. IF YA'LL WANT. . ."
};

float ScreenTimers[(sizeof(EndGame) / sizeof(std::string)) + (sizeof(FinalStats) / sizeof(std::string))]{};
int KeyCounters[(sizeof(EndGame) / sizeof(std::string)) + (sizeof(FinalStats) / sizeof(std::string))]{};

std::string TypewriterEffect(entt::registry& registry,std::string& dest, std::string& text, float& timer, int& keyIndex);
std::string countLives(entt::registry& registry);
void SetRegularUI(entt::registry& registry, BLIT_Font& font, int W, int H);
void TypeOutVictory(entt::registry& registry, BLIT_Font& font, int W, int H, int statIndex, std::string* victoryText);
void RenderOnScreen(BLIT_Font& font, int W, int H, std::string text);

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
	RenderOnScreen(font, W / 8, 25, UI[0]);
	RenderOnScreen(font, W - (W / 6), 25, UI[1]);
	RenderOnScreen(font, (W / 2) - 100, 25, UI[2]);
	//font.DrawTextImmediate(W - 75, H - 20, UI[3].c_str(), 3);
	std::string score = std::to_string(Scorring.GetScore());
	RenderOnScreen(font, (W / 2), 60, score);
	RenderOnScreen(font, (W / 8) + 15, 60, score);
	RenderOnScreen(font, 5, H - 20, countLives(registry));
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void StartMenu(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	bltr.ClearColor(0x00000000);
	RenderOnScreen(font, W / 8, 25, UI[0]);
	RenderOnScreen(font, W - (W / 6), 25, UI[1]);
	RenderOnScreen(font, (W / 2) - 100, 25, UI[2]);
	RenderOnScreen(font, (W / 2) - 150, H - 25, GameStart[0]);
	if (rightStart > rightScroll) {
		rightStart -= deltaTime;
	}
	if (leftStart < leftScroll) {
		leftStart += deltaTime * 300;
	}
	RenderOnScreen(font, leftStart, H - 70, GameStart[1]);
	RenderOnScreen(font, rightStart, H - 70, GameStart[2]);
	RenderOnScreen(font, (W / 2) - 35, (H / 2), GameStart[3]);
	if (flashTimer < flashEnd) {
		flashTimer += deltaTime;
		RenderOnScreen(font, (W / 2) - 100, (H / 2) + 100, GameStart[4]);
	}
	if (flashTimer > flashEnd) {
		flashTimer = 0.00f;
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
	SetRegularUI(registry, font, W, H);
	std::string statHeader;
	TypewriterEffect(registry, statHeader, LevelStats[0], keyTimer, KeyCounters[0]);
	RenderOnScreen(font, (W / 4) + 25, (H / 2) - 50, statHeader);
	if (statHeader == LevelStats[0]) {
		RenderOnScreen(font, (W / 2) - 50, (H / 2) + 50, LevelStats[1]);
		RenderOnScreen(font, (W / 2) - 75, (H / 2) + 100, LevelStats[2]);
		RenderOnScreen(font, (W / 3) + 25, (H / 2) + 150, LevelStats[3]);
	}
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void StartOfLevel(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	RenderOnScreen(font, (W / 2) - 35, (H / 2) + 50, LevelBegin[0]);
	RenderOnScreen(font, (W / 2) - 125, (H / 2) + 100, LevelBegin[1]);
	//RenderOnScreen(font, (W / 2) - 125, (H / 2) + 100, LevelBegin[1]);
	RenderOnScreen(font, (W / 2) - 50, (H / 2) + 150, LevelBegin[2]);
	RenderOnScreen(font, (W / 2) - 75, (H / 2) + 200, LevelBegin[3]);
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void WinScreen(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	if (screenTransitionStart < screenTransition) {
		screenTransitionStart += deltaTime;
		std::string statHeader;
		std::string statHeader2;
		std::string statHeader3;
		if (FinaleIdx != FinalStats->length()) {
			if (screenTimer < screenTimerStart) {
				screenTimer += deltaTime;
			}
			else {
				screenTimer = 0.0f;
				FinaleIdx++;
				ScreenTimers[FinaleIdx - 1] = 0.0f;
			}
		}
		if (FinaleIdx >= 1) {
			TypewriterEffect(registry, statHeader, FinalStats[0], ScreenTimers[0], KeyCounters[0]);
			RenderOnScreen(font, (W / 4), (H / 2) - 200, statHeader);
		}
		if (FinaleIdx >= 2) {
			TypewriterEffect(registry, statHeader2, FinalStats[1], ScreenTimers[1], KeyCounters[1]);
			RenderOnScreen(font, (W / 4), (H / 2) - 150, statHeader2);
		}
		if (FinaleIdx >= 3) {
			TypewriterEffect(registry, statHeader3, FinalStats[2], ScreenTimers[2], KeyCounters[2]);
			RenderOnScreen(font, (W / 4), (H / 2) - 100, statHeader3);
		}
		if (FinaleIdx == FinalStats->length()) {
			screenTimer = 0.0f;
		}
	}
	else {
		std::string victoryText;
		std::string victoryText1;
		std::string victoryText2;
		std::string victoryText3;
		std::string victoryText4;
		if (FinaleIdx2 != EndGame->length()) {
			if (screenTimer < screenTimerStart) {
				screenTimer += deltaTime;
				ScreenTimers[FinaleIdx + FinaleIdx2 - 2] = 0.0f;
				KeyCounters[FinaleIdx + FinaleIdx2 - 2] = 0;
			}
			else {
				screenTimer = 0.0f;
				FinaleIdx2++;
			}
		}
		if (FinaleIdx2 >= 1) {
			TypewriterEffect(registry, victoryText, EndGame[0], ScreenTimers[3], KeyCounters[3]);
			RenderOnScreen(font, (W / 3), (H / 2) - 225, victoryText);
		}
		if (FinaleIdx2 >= 2) {
			TypewriterEffect(registry, victoryText1, EndGame[1], ScreenTimers[4], KeyCounters[4]);
			RenderOnScreen(font, (W / 3), (H / 2) - 125, victoryText1);
		}
		if (FinaleIdx2 >= 3) {
			TypewriterEffect(registry, victoryText2, EndGame[2], ScreenTimers[5], KeyCounters[5]);
			RenderOnScreen(font, (W / 3), (H / 2) - 25, victoryText2);
		}
		if (FinaleIdx2 >= 4) {
			TypewriterEffect(registry, victoryText3, EndGame[3], ScreenTimers[6], KeyCounters[6]);
			RenderOnScreen(font, (W / 3), (H / 2) + 75, victoryText3);
		}
		if (FinaleIdx2 >= 5) {
			TypewriterEffect(registry, victoryText4, EndGame[4], ScreenTimers[7], KeyCounters[7]);
			RenderOnScreen(font, (W / 3), (H / 2) + 175, victoryText4);
		}
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

std::string TypewriterEffect(entt::registry& registry, std::string& dest, std::string& text, float& timer, int& keyIndex) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	if (keyIndex < text.length()) {
		timer += deltaTime;
		if (timer >= keyPress) {
			timer = 0.0f;
			keyIndex++;
		}
	}
	dest = "";
	for (int i = 0; i < keyIndex; i++) {
		dest += text[i];
	}
	return dest;
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

void SetRegularUI(entt::registry& registry, BLIT_Font& font, int W, int H) {
	font.DrawTextImmediate(W / 8, 25, "1UP", 3);
	font.DrawTextImmediate(W - (W / 6), 25, "2UP", 3);
	font.DrawTextImmediate((W / 2) - 100, 25, "HIGH SCORE", 10);
	font.DrawTextImmediate(W - 75, H - 20, "RRR", 3);
	std::string score = std::to_string(Scorring.GetScore());
	font.DrawTextImmediate((W / 2), 60, score.c_str(), score.length());
	font.DrawTextImmediate((W / 8) + 15, 60, score.c_str(), score.length());
	font.DrawTextImmediate(5, H - 20, countLives(registry).c_str(), countLives(registry).length());
}

void RenderOnScreen(BLIT_Font& font, int W, int H, std::string text) {
	font.DrawTextImmediate(W, H, text.c_str(), text.length());
}