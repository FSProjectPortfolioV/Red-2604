#pragma once
#include "../GAME/GameComponents.h"
#include "../GAME/GamePlay/ScoreSystem/LeaderboardSystem.h"
#include "../GAME/GamePlay/ScoreSystem/HighscoreScreenController.h"
#include "./Utility/FileIntoString.h"
#include "shaderc/shaderc.h"
#include "./Overlay.h"
#include "./Font.h"
#include "./BLIT_Font.h"
#include "../UTIL/Utilities.h"

float flashEnd = 1.1f;
float flashTimer = 0.0f;
bool flashOn = true;
int leftStart;
int rightStart;
int rightScroll;
int leftScroll;
float keyTimer = 0.0f;
float keyPress = 0.08f;
float screenTimer = 0.0f;
float screenTimerStart = 2.35f;
float screenTransition = 4 * screenTimerStart;
float screenTransitionStart = 0.0f;
int LineSpace = -100;
int FinaleIdx = 1;
int FinaleIdx2 = 1;
int FinaleIdx3 = 1;
bool settingsOpen = false;
bool levelStart = false;
int OverlayIndex = 0;
int PrevOverlayIndex = 0;
HighscoreScreenController ScoreControl;
LeaderboardSystem LeaderboardControl;

std::vector<std::string> FinalStats{
	"TERMINATING CRAFTS",
	"PERCENTAGE",
	"TODAY'S TOP"
};

std::vector<std::string> LevelStats{
	"TERMINATING CRAFTS",
	"BONUS",
	"0000 PTS",
	"R X 1000 = 3000 PTS"
};

std::vector<std::string> LevelBegin{
	"Rank",
	"LAST ## STAGE",
	"READY",
	"PLAYER 1"
};

std::vector<std::string> UI{
	"1UP",
	"2UP",
	"HIGH SCORE",
};

std::vector<std::string> GameStart{
	"2026 CRIMSON MILLENIA",
	"CRIMSON ",
	"MILLENIA",
	"2851",
	"PRESS SPACEBAR"
};

std::vector<std::string> EndGame{
	"ENEMIES DESTROYED",
	"SPECIAL BONUS",
	"00, 000, 000 PTS",
	"BY CRIMSON MILLENIA",
	"PS. IF YA'LL WANT. . .",
	//Always last
	"GAME OVER"
};

std::vector<std::string> MenuOptions{
	"RESET",
	"SETTINGS [O]",
	"CREDITS",
	"MASTER VOLUME",
	"MUSIC VOLUME",
	"SFX VOLUME",
	"LEADERBOARD",
	//Always last
	"OPTIONS"
};

static std::vector<float> ScreenTimers((EndGame.size()) + FinalStats.size());
static std::vector<int> KeyCounters(EndGame.size() + FinalStats.size());
static std::vector<std::string> forTyping{};

void FlashingEffect(entt::registry& registry, BLIT_Font& font, int W, int H, std::string text);
std::string TypewriterEffect(entt::registry& registry,std::string& dest, std::string& text, float& timer, int& keyIndex);
void TypeLines(entt::registry& registry, BLIT_Font& font, int W, int H, std::vector<std::string>& texts, std::vector<float>& timers, std::vector<int>& keyIndices, int lineCount);
void countLives(entt::registry& registry, BLIT_Font& font, int W, int H);
void SetRegularUI(entt::registry& registry, BLIT_Font& font, int W, int H);
void RenderOnScreen(BLIT_Font& font, int W, int H, std::string text);
void RegularOptions(BLIT_Font& font, int W, int H);

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
	SetRegularUI(registry, font, W, H);
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void StartMenu(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	if (rightStart > rightScroll) {
		rightStart -= deltaTime * 222;
	}
	if (leftStart < leftScroll) {
		leftStart += deltaTime * 300;
	}
	bltr.ClearColor(0x00000000);
	RenderOnScreen(font, W / 8, 25, UI[0]);
	RenderOnScreen(font, W - (W / 6), 25, UI[1]);
	RenderOnScreen(font, (W / 2) - 100, 25, UI[2]);
	RenderOnScreen(font, (W / 2) - 150, H - 25, GameStart[0]);
	RenderOnScreen(font, leftStart, H - 70, GameStart[1]);
	RenderOnScreen(font, rightStart, H - 70, GameStart[2]);
	RenderOnScreen(font, (W / 2), (H / 2), GameStart[3]);
	FlashingEffect(registry, font, (W / 2) - 120, (H / 2) + 100, GameStart[4]);
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr); 
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void EndOfLevel(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	if (screenTimer < screenTimerStart) {
		screenTimer += deltaTime;
	}
	else {
		screenTimer = 0.0f;
		levelStart = false;
	}
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	std::string statHeader;
	statHeader = TypewriterEffect(registry, statHeader, LevelStats[0], keyTimer, KeyCounters[0]);
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
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	if (screenTimer < screenTimerStart) {
		screenTimer += deltaTime;
	}
	else {
		levelStart = true;
	}
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	RenderOnScreen(font, (W / 2) - 35, (H / 2) - 200, LevelBegin[0]);
	RenderOnScreen(font, (W / 2) - 125, (H / 2) - 125, LevelBegin[1]);
	//RenderOnScreen(font, (W / 2) - 125, (H / 2) + 100, LevelBegin[1]);
	RenderOnScreen(font, (W / 2) - 50, (H / 2) - 50, LevelBegin[2]);
	RenderOnScreen(font, (W / 2) - 75, (H / 2) + 25, LevelBegin[3]);
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
		if (FinaleIdx != FinalStats.size()) {
			if (screenTimer < screenTimerStart) {
				screenTimer += deltaTime;
			}
			else {
				screenTimer = 0.0f;
				FinaleIdx++;
				ScreenTimers[FinaleIdx - 1] = 0.0f;
			}
		}
		TypeLines(registry, font, (W / 3), (H / 2) - 200, FinalStats, ScreenTimers, KeyCounters, FinaleIdx);
		if (FinaleIdx == FinalStats.size()) {
			screenTimer = 0.0f;
		}
	}
	else {
		if (FinaleIdx2 != EndGame.size()) {
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
		TypeLines(registry, font, (W / 3), (H / 2) - 225, EndGame, ScreenTimers, KeyCounters, FinaleIdx2);
	}
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void LoseMenu(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	if (FinaleIdx != FinalStats.size()) {
		if (screenTimer < screenTimerStart) {
			screenTimer += deltaTime;
		}
		else {
			screenTimer = 0.0f;
			FinaleIdx3++;
			ScreenTimers[FinaleIdx3 - 1] = 0.0f;
		}
	}
	TypeLines(registry, font, W/3 - 100, H/4, FinalStats, ScreenTimers, KeyCounters, FinaleIdx3);
	if(KeyCounters[2] >= FinalStats[2].length()) {
		FlashingEffect(registry, font, (W / 2) - 100, (H / 2) + 100, EndGame[5]);
		RegularOptions(font, W, H);
	}
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void PauseMenu(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	FlashingEffect(registry, font, (W / 2) - 100, (H / 4) - 50, MenuOptions[MenuOptions.size() - 1]);
	RegularOptions(font, W, H);
	if(settingsOpen)
	for (int i = 6; i > 1; i--){
		RenderOnScreen(font, (W / 3), 100 + (i * 75), MenuOptions[i]);
	}
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void HighScoreMenu(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	RenderOnScreen(font, (W / 2) - 50, (H / 2) + 50, "HIGH SCORE");
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

void UpdateUIOverlays(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	auto& pressEvents = registry.ctx().get<GW::CORE::GEventCache>();
	GW::GEvent event;
	while (+pressEvents.Pop(event))
	{
		GW::INPUT::GBufferedInput::Events inputEvent;
		GW::INPUT::GBufferedInput::EVENT_DATA inputData;

		if (+event.Read(inputEvent, inputData))
		{
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_P
				&& OverlayIndex != 0 && OverlayIndex != 1 && OverlayIndex != 4
				&& OverlayIndex != 5 && OverlayIndex != 6 && OverlayIndex != 7) {

				if (OverlayIndex == 3) {
					OverlayIndex = PrevOverlayIndex;
				}
				else {
					PrevOverlayIndex = OverlayIndex;
					OverlayIndex = 3;
				}
			}

			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_O
				&& OverlayIndex == 3) {
				settingsOpen = !settingsOpen;
			}

			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_SPACE
				&& OverlayIndex == 0) {
				OverlayIndex = 1;
			}
			if (levelStart) {
				OverlayIndex = 2;
				levelStart = false;
			}
		}
	}
	switch (OverlayIndex) {
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
		PauseMenu(registry, ovl, bltr, font, W, H);
	break;
	case 4:
		EndOfLevel(registry, ovl, bltr, font, W, H);
	break;
	case 5:
		LoseMenu(registry, ovl, bltr, font, W, H);
	break;
	case 6:
		WinScreen(registry, ovl, bltr, font, W, H);
	break;
	case 7:
		HighScoreMenu(registry, ovl, bltr, font, W, H);
	break;
	}
}

void FlashingEffect(entt::registry& registry, BLIT_Font& font, int W, int H, std::string text) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	if (flashTimer <= flashEnd && flashOn) {
		flashTimer += deltaTime;
		RenderOnScreen(font, W, H, text);
	}
	else {
		flashOn = false;
	}
	if (flashTimer >= 0.0f && !flashOn) {
		flashTimer -= deltaTime;
	}
	else {
		flashOn = true;
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
	else {
		return text;
	}
	dest = "";
	for (int i = 0; i < keyIndex; i++) {
		dest += text[i];
	}
	return dest;
}

void countLives(entt::registry& registry, BLIT_Font& font, int W, int H) {
	std::string hits;
	auto player = registry.view<GAME::Player>();
	for (auto entity : player) {
		auto hitpoints = registry.get<GAME::Lives>(entity).count;
		for (int i = 0; i < hitpoints; i++) {
			hits += '^';
		}
		if(hitpoints == 0) {
			hits = "Vessel Destroyed";
			OverlayIndex = 5;
		}
	}
	font.DrawTextImmediate(5, H - 20, hits.c_str(), hits.length());
}

void SetRegularUI(entt::registry& registry, BLIT_Font& font, int W, int H) {
	RenderOnScreen(font, W / 8, 25, UI[0]);
	RenderOnScreen(font, (W / 2) - 100, 25, UI[1]);
	RenderOnScreen(font, W - 75, H - 20, UI[2]);
	//font.DrawTextImmediate(W - 75, H - 20, "RRR", 3);
	std::string score = std::to_string(ScoreControl.GetPlayerScore());
	RenderOnScreen(font, (W / 2), 60, score);
	RenderOnScreen(font, (W / 8) + 15, 60, score);
	countLives(registry, font, W, H);
}

void RenderOnScreen(BLIT_Font& font, int W, int H, std::string text) {
	font.DrawTextImmediate(W, H, text.c_str(), text.length());
}

void TypeLines(entt::registry& registry, BLIT_Font& font, int W, int H, std::vector<std::string>& 
texts, std::vector<float>& timers, std::vector<int>& keyIndices, int lineCount) {
	forTyping.resize(texts.size());
	if (lineCount > texts.size()) {
		lineCount = texts.size();
	}
	for (int i = 0; i < lineCount; i++) {
		forTyping[i] = (TypewriterEffect(registry, forTyping[i], texts[i], timers[i], keyIndices[i]));
		RenderOnScreen(font, W, H - (i * LineSpace), forTyping[i]);
	}
}

void RegularOptions(BLIT_Font& font, int W, int H) {
	RenderOnScreen(font, (W / 3) - 100, H - 100, MenuOptions[0]);
	RenderOnScreen(font, (W / 2) + 100, H - 100, MenuOptions[1]);
}