#pragma once
#include <iomanip>
#include <sstream>

#include "../GAME/GameComponents.h"
#include "./Utility/FileIntoString.h"
#include "shaderc/shaderc.h"
#include "./Overlay.h"
#include "./Font.h"
#include "./BLIT_Font.h"
#include "../UTIL/Utilities.h"
#include "../GAME/Gameplay/ScoreSystem/ScoreSystem.h"
#include "../GAME/Gameplay/ScoreSystem/InitialsEntrySystem.h"
#include "../GAME/GamePlay/ScoreSystem/LeaderboardSystem.h"
#include "../GAME/GamePlay/ScoreSystem/HighscoreScreenController.h"
#include "../GAME/Gameplay/ScoreSystem/LocalHighscoreSystem.h"
#include "../GAME/GameManager.h"
#include "../GAME/LevelLoader.h"
#include "../DRAW/Utility/TextureUTTL.h"
#include "../GAME/Gameplay/PowerUps/PowerUps.h"
#include "../APP/Window.hpp"


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
float screenTimerStart = 2.0f;
float screenTransition = 4 * screenTimerStart;
float screenTransitionStart = 0.0f;
int LineSpace = 100;
int FinaleIdx = 1;
int FinaleIdx2 = 1;
int FinaleIdx3 = 1;
bool settingsOpen = false;
bool levelStart = false;
int OverlayIndex = 0;
int PrevOverlayIndex = 0;
int finalScreenCounter = 1;
float sfxVol = 0.07f;
float musicVol = 0.05f;
float masterVol = 0.05f;
float volChange = 0.01f;
int volIndex = 0;
int levelIndex = 0;
int totalKilled = 0;
int totalSpawned = 0;
bool gameWon = false;
bool namedScore = false;
bool justLooking = false;

std::vector<std::string> FinalStats{
	"TERMINATING CRAFTS",
	"PERCENTAGE",
	"TODAY'S TOP"
};

std::vector<std::string> LevelStats{
	"TERMINATING CRAFTS",
	"BONUS",
	"R X 1000   ="
};

std::vector<std::string> LevelBegin{
	"Cadet",
	"Master Chief",
	"Admiral",
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
	"PRESS SPACEBAR OR A"
};

std::vector<std::string> EndGame{
	"ALL ENEMIES DESTROYED !!",
	"SPECIAL BONUS",
	"10, 000, 000 PTS",
	"BY CRIMSON MILLENIA",
	"PS. IF YA'LL WANT. . .",
	//Always last
	"GAME OVER"
};

std::vector<std::string> MenuOptions{
	"RESET [Y]",
	"SETTINGS [O]",
	"CREDITS [K]",
	"MASTER VOLUME",
	"MUSIC VOLUME",
	"SFX VOLUME",
	"LEADERBOARD [L]",
	//Always last
	"PAUSED"
};

static std::vector<float> ScreenTimers((EndGame.size()) + FinalStats.size());
static std::vector<int> KeyCounters(EndGame.size() + FinalStats.size());
static std::vector<std::string> forTyping{};
static std::vector<int> ElimPercentages;

void FlashingEffect(entt::registry& registry, BLIT_Font& font, int W, int H, std::string text);
std::string TypewriterEffect(entt::registry& registry,std::string& dest, std::string& text, float& timer, int& keyIndex);
void TypeLines(entt::registry& registry, BLIT_Font& font, int W, int H, std::vector<std::string>& texts, std::vector<float>& timers, std::vector<int>& keyIndices, int lineCount);
void TypeVictoryLines(entt::registry& registry, BLIT_Font& font, int W, int H, std::vector<std::string>& texts, std::vector<float>& timers, std::vector<int>& keyIndices, int lineCount);
void countLives(entt::registry& registry, BLIT_Font& font, int W, int H);
void SetRegularUI(entt::registry& registry, BLIT_Font& font, int W, int H);
void RenderOnScreen(BLIT_Font& font, int W, int H, std::string text);
void RegularOptions(BLIT_Font& font, int W, int H);
void FlashingUnderLine(entt::registry& registry, BLIT_Font& font, int W, int H, std::string text);
std::string ShowVolume(float volume);
std::string BuildRollCharges(int charges);
int LevelProficiency(float spawned, float killed);
std::string CalculateTodaysTop();

// Show PowerUps Timer
void QueuePowerUpText(entt::registry& registry, BLIT_Font& font, int W, int H);
void DrawPowerUpImages(entt::registry& registry, int W, int H, unsigned int* screenPixels);

void DrawImageToOverlay(unsigned int* screenPixels, int screenW, int screenH,
	unsigned int* imgPixels, int imgW, int imgH,
	int startX, int startY);


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
	auto& font = registry.emplace<BLIT_Font>(entity, blitter, "../Fonts/font.tga", font_Arial);

	//Extra emplacing for switching components
	registry.ctx().emplace<Overlay>(windowWidth, windowHeight, win, surface, 512);
	auto& ctxBlitter = registry.ctx().emplace< GW::GRAPHICS::GBlitter>();
	ctxBlitter.Create(windowWidth, windowHeight);
	auto& ctxFont = registry.ctx().emplace<BLIT_Font>(ctxBlitter, "../Fonts/font.tga", font_Arial);

	using namespace GW::AUDIO;
	GAudio& gAudio = registry.ctx().emplace<GAudio>();
	gAudio.Create();
	gAudio.SetMasterVolume(masterVol);

	GMusic& gMusic = registry.ctx().emplace<GMusic>();
	const char* bgMusic = (*config).at("Sounds").at("gpmusic").as<const char*>();
	gMusic.Create(bgMusic, gAudio, musicVol);
	gMusic.Play(true);

	auto& gMusic2 = registry.emplace<GMusic>(entity);
	const char* psMusic = (*config).at("Sounds").at("psmusic").as<const char*>();
	gMusic2.Create(psMusic, gAudio);
	gMusic2.Play(true);
	gMusic2.Stop();

	auto& soundstorage = registry.ctx().emplace<GAME::SoundStorage>();
	soundstorage.soundCues.resize(6);
	soundstorage.sounds.resize(6);
	for (int i = 0; i < soundstorage.soundCues.size(); i++) {
		soundstorage.soundCues[i] = false;
	}
	const char* playerShoot = config->at("Sounds").at("pshoot").as<const char*>();
	soundstorage.sounds[0].Create(playerShoot, gAudio);

	const char* playerHit = config->at("Sounds").at("phit").as<const char*>();
	soundstorage.sounds[1].Create(playerHit, gAudio);

	const char* playerDead = config->at("Sounds").at("pdeath").as<const char*>();
	soundstorage.sounds[2].Create(playerDead, gAudio);

	const char* pauseSound = (*config).at("Sounds").at("menu").as<const char*>();
	soundstorage.sounds[3].Create(pauseSound, gAudio);

	const char* enemyHit = (*config).at("Sounds").at("ehit").as<const char*>();
	soundstorage.sounds[4].Create(enemyHit, gAudio);

	const char* enemyDead = (*config).at("Sounds").at("edeath").as<const char*>();
	soundstorage.sounds[5].Create(enemyDead, gAudio);

	//Power Ups
	std::string sfIcoPath = (*config).at("PowerUps").at("SideFighterIcon_Path").as<std::string>();
	std::string msIcoPath = (*config).at("PowerUps").at("MultiShotIcon_Path").as<std::string>();

	LoadUIIcon("SideFighter", sfIcoPath);
	LoadUIIcon("MultiShot", msIcoPath);
}

static void GameplayUI(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);

	QueuePowerUpText(registry, font, W, H);

	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);

	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);

	DrawPowerUpImages(registry, W, H, pixels);

	ovl.Unlock();
	ovl.TransferOverlay();
}

void Title(Overlay& ctxovl, GW::GRAPHICS::GBlitter& ctxbltr, BLIT_Font& font, int W, int H, std::string text) {
	ctxbltr.ClearColor(0x00000000);
	RenderOnScreen(font, (W / 2) - 30, (H / 2) + 10, text);
	unsigned int* titlePixels;
	ctxovl.LockForUpdate(W * H, &titlePixels);
	ctxbltr.ExportResult(false, W, H, 0, 0, titlePixels, nullptr, nullptr);
	ctxovl.Unlock();
	ctxovl.TransferOverlay();
}

void ClearLargerText(Overlay& ctxovl, GW::GRAPHICS::GBlitter& ctxbltr, BLIT_Font& font, int W, int H) {
	ctxbltr.ClearColor(0x00000000);
	unsigned int* titlePixels;
	ctxovl.LockForUpdate(W * H, &titlePixels);
	ctxbltr.ExportResult(false, W, H, 0, 0, titlePixels, nullptr, nullptr);
	ctxovl.Unlock();
	ctxovl.TransferOverlay();
}

static void StartMenu(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	auto gameManager = registry.view<GAME::GameManager>();
	for (auto ent : gameManager) {
		registry.emplace_or_replace<GAME::Paused>(ent);
	}
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
	FlashingEffect(registry, font, (W / 2) - 150, (H / 2) + 200, GameStart[4]);
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
	if (forTyping.empty()) {
		forTyping.push_back("");
	}
	forTyping[0] = TypewriterEffect(registry, forTyping[0], LevelStats[0], keyTimer, KeyCounters[0]);
	RenderOnScreen(font, (W / 4) - 25, 150, forTyping[0]);
	if (forTyping[0] == LevelStats[0]) {
		int rollsLeft = 0;
		auto playerView = registry.view<GAME::Player, GAME::RollCharges>();
		for (auto entity : playerView)
		{
			auto& charges = registry.get<GAME::RollCharges>(entity).charges;
			RenderOnScreen(font, (W / 2) + 50, (H / 2), std::to_string(charges * 1000) + " PTS");
		}
		auto lmView = registry.view<GAME::LevelManager>();
		auto lmEntity = lmView.front();
		auto& lm = registry.get<GAME::LevelManager>(lmEntity);
		RenderOnScreen(font, (W / 2) + 150, 150, std::to_string(ElimPercentages[ElimPercentages.size() - 1]) + '%');
		RenderOnScreen(font, (W / 2) - 50, (H / 2) - 150, LevelStats[1]);
		RenderOnScreen(font, (W / 2) - 75, (H / 2) - 75, std::to_string(LevelProficiency(lm.enemyTotal, lm.enemyKilled)) + " PTS");
		RenderOnScreen(font, (W / 3) + 25, (H / 2), LevelStats[2]);

		if (lm.levelIndex == 0 && lm.loops > 0)
		{
			FlashingEffect(registry, font, (W / 2) - 190, (H / 2) + 100, "DIFFICULTY INCREASED!");
		}
		if (screenTimer < screenTransition) {
			screenTimer += deltaTime;
		}
		else {
			screenTimer = 0.0f;
			OverlayIndex = 1;
		}
	}
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void StartOfLevel(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	auto gameManager = registry.view<GAME::GameManager>();
	for (auto ent : gameManager) {
		registry.remove<GAME::Paused>(ent);
	}
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	if (screenTimer < screenTimerStart) {
		screenTimer += deltaTime;
	}
	else {
		screenTimer = 0.0f;
		levelStart = true;
	}
	auto lmView = registry.view<GAME::LevelManager>();
	auto lmEntity = lmView.front();
	auto& lm = registry.get<GAME::LevelManager>(lmEntity);
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	int rankX = 0;
	switch (lm.levelIndex) {
		case 0:
			rankX = (W / 2) - 35;
		break;
		case 1:
			rankX = (W / 2) - 95;
		break;
		case 2:
			rankX = (W / 2) - 50;
		break;
	}
	RenderOnScreen(font, rankX, (H / 2) - 200, LevelBegin[lm.levelIndex]);
	RenderOnScreen(font, (W / 2) - 63, (H / 2) - 125, "STAGE " + std::to_string(lm.levelIndex + 1));
	RenderOnScreen(font, (W / 2) - 50, (H / 2) - 50, LevelBegin[3]);
	RenderOnScreen(font, (W / 2) - 75, (H / 2) + 25, LevelBegin[4]);
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

void TypeFinalStats(entt::registry& registry, BLIT_Font& font, int W, int H, std::vector<std::string>&
	texts, std::vector<float>& timers, std::vector<int>& keyIndices, int lineCount) {

	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	TypeLines(registry, font, W, H, texts, timers, keyIndices, lineCount);
	if (forTyping[0] == FinalStats[0]) {
		RenderOnScreen(font, W + 400, H, std::to_string(totalKilled));
	}
	if (forTyping[1] == FinalStats[1]) {
		int totalPercentage;
		if (totalKilled == 0 || totalSpawned == 0) {
			totalPercentage = 0;
		}
		else {
			totalPercentage == (totalKilled / totalSpawned) * 100;
		} 
		RenderOnScreen(font, W + 400, H + LineSpace, std::to_string(totalPercentage) + '%');
	}
	if (forTyping[2] == FinalStats[2]) {
		RenderOnScreen(font, W + 400, H + (LineSpace * 2), CalculateTodaysTop());
	}
}

static void WinScreen(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	float deltaTime = registry.ctx().get<UTIL::DeltaTime>().dtSec;
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	if (screenTransitionStart < screenTransition) {
		screenTransitionStart += deltaTime;
	}
	else {
		screenTransitionStart = 0.0f;
		finalScreenCounter++;
	}
	switch (finalScreenCounter) {
		case 1:
			if (FinaleIdx != FinalStats.size()) {
				if (screenTimer < screenTimerStart) {
					screenTimer += deltaTime;
				}
				else {
					screenTimer = 0.0f;
					FinaleIdx++;
					ScreenTimers[FinaleIdx - 1] = 0.0f;
				}
				TypeFinalStats(registry, font, (W / 3) - 125, (H / 2) - 200, FinalStats, ScreenTimers, KeyCounters, FinaleIdx);
			}else
				screenTimer = 0.0f;
		break;
		case 2:
			if (FinaleIdx2 != EndGame.size()) {
				if (screenTimer < screenTimerStart) {
					screenTimer += deltaTime;
					ScreenTimers[FinaleIdx2 - 1] = 0.0f;
					KeyCounters[FinaleIdx2 - 1] = 0;
					forTyping[FinaleIdx2 - 1] = "";
				}
				else {
					screenTimer = 0.0f;
					FinaleIdx2++;
				}
			}
			TypeVictoryLines(registry, font, W, (H / 2) - 225, EndGame, ScreenTimers, KeyCounters, FinaleIdx2);
			if (FinaleIdx2 == EndGame.size()) {
				screenTimer = 0.0f;
			}
			else {
				screenTransitionStart -= deltaTime;
			}
		break;
		case 3:
			auto& Bonus = registry.ctx().get<ScoreSystem>();
			Bonus.AddPoints(stoi(EndGame[2]));
			OverlayIndex = 7;
		break;
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
	TypeFinalStats(registry, font, W/3 - 100, H/4, FinalStats, ScreenTimers, KeyCounters, FinaleIdx3);
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
	auto& input = registry.ctx().get<UTIL::Input>();
	float mouseX, mouseY;
	input.immediateInput.GetMousePosition(mouseX, mouseY);
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	FlashingEffect(registry, font, (W / 2) - 50, (H / 4) - 50, MenuOptions[MenuOptions.size() - 1]);
	RegularOptions(font, W, H);
	int widthOffset = 300;
	int heightOffset = 25;
	if (settingsOpen) {
		for (int i = 5; i > 1; i--) {
			RenderOnScreen(font, (W / 3) - 100, 80 + (i * 75), MenuOptions[i]);
			if (mouseX >= (W / 3) - 100 && mouseX <= ((W / 3) - 100 + widthOffset) &&
				mouseY >= 80 + (i * 75) && mouseY <= (80 + (i * 75) + heightOffset)) {
				FlashingUnderLine(registry, font, (W / 3) - 100, 80 + (i * 75), MenuOptions[i]);
				float leftMouse = 0.0f;
				input.immediateInput.GetState(G_BUTTON_LEFT, leftMouse);
				if (leftMouse > 0.0f && (i == 5 || i == 4 || i == 3)) {
					volIndex = i;
				}
			}
		}
		RenderOnScreen(font, (W / 3) + 280, 80 + (3 * 75), ShowVolume(masterVol));
		RenderOnScreen(font, (W / 3) + 280, 80 + (4 * 75), ShowVolume(musicVol));
		RenderOnScreen(font, (W / 3) + 280, 80 + (5 * 75), ShowVolume(sfxVol));
	}
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

//void NumberOnePlayer(Overlay& ctxovl, GW::GRAPHICS::GBlitter& ctxbltr, BLIT_Font& font, int W, int H, std::string text) {
//	ctxbltr.ClearColor(0x00000000);
//	RenderOnScreen(font, (W / 2) - 30, (H / 2) - 50, text);
//	unsigned int* titlePixels;
//	ctxovl.LockForUpdate(W * H, &titlePixels);
//	ctxbltr.ExportResult(false, W, H, 0, 0, titlePixels, nullptr, nullptr);
//	ctxovl.Unlock();
//	ctxovl.TransferOverlay();
//}

static void HighScoreMenu(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H,
	Overlay& ctxovl, GW::GRAPHICS::GBlitter& ctxbltr, BLIT_Font& ctxfont) {
	auto& pressEvents = registry.ctx().get<GW::CORE::GEventCache>();
	GW::GEvent event;
	auto& leaderboard = registry.ctx().get<HighscoreScreenController>();
	if (!leaderboard.IsLoaded())
		leaderboard.Begin(registry); // Making sure this gets called only once when entering highscore screen. Otherwise, it runs every frame which gets very expensive.
	auto score = registry.ctx().get<ScoreSystem>().GetScore();
	GW::INPUT::GBufferedInput::Events inputEvent;
	GW::INPUT::GBufferedInput::EVENT_DATA inputData;

	bltr.ClearColor(0x00000000);
	if (leaderboard.IsNewHighscore() && !namedScore && !justLooking) {
		FlashingEffect(registry, font, (W / 2) - 100, (H / 2) - 150, "New Highscore!");
		RenderOnScreen(font, (W / 4) - 45, (H / 2) - 75, "INPUT INITIALS WITH ARROWS (Ex. \"ABC\")");
		if (forTyping.empty()) {
			forTyping.push_back("");
		}
		RenderOnScreen(font, (W / 2) - 30, (H / 2) + 25, forTyping[forTyping.size() - 1]);
	}
	else {
		if (!leaderboard.GetEntries().empty())
		{
			//NumberOnePlayer(ctxovl, ctxbltr, ctxfont, W, H, leaderboard.GetEntries()[0].initials);
			//RenderOnScreen(font, (W / 2) + 100, (H / 2) - 50, std::to_string(leaderboard.GetEntries()[0].score));
			for (int i = 0; i < leaderboard.GetEntries().size(); i++) {
				RenderOnScreen(font, (W / 3) + 75, (H / 2) - 75 + (i * 50), leaderboard.GetEntries()[i].initials);
				RenderOnScreen(font, (W / 2) + 25, (H / 2) - 75 + (i * 50), std::to_string(leaderboard.GetEntries()[i].score));
			}
		}
		else
		{
			RenderOnScreen(font, (W / 2) - 100, (H / 2), "No scores yet!");
		}
	}
	SetRegularUI(registry, font, W, H);
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr);
	ovl.Unlock();
	ovl.TransferOverlay();
}

void UpdateUIOverlays(entt::registry& registry, entt::entity entity, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	auto& pressEvents = registry.ctx().get<GW::CORE::GEventCache>();
	GW::GEvent event;
	GW::INPUT::GBufferedInput::Events inputEvent;
	GW::INPUT::GBufferedInput::EVENT_DATA inputData;
	std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
	using namespace GW::AUDIO;
	auto& menuMusic = registry.ctx().get<GMusic>();
	auto& audio = registry.ctx().get<GAudio>();
	auto& gameMusic = registry.get<GMusic>(entity);

	auto& soundStorage = registry.ctx().get<GAME::SoundStorage>();
	for (int i = 0; i < soundStorage.sounds.size(); i++) {
		if (soundStorage.soundCues[i]) {
			soundStorage.sounds[i].Play();
			soundStorage.soundCues[i] = false;
		}
	}

	// Controller
	auto& input = registry.ctx().get<UTIL::Input>();
	static bool startHeld = false;
	float startButton = 0.0f;
	input.gamePads.GetState(0, G_START_BTN, startButton);
	bool controllerStartPressed = false;

	if (startButton > 0.0f && !startHeld)
	{
		controllerStartPressed = true;
	}

	startHeld = startButton > 0.0f;

	if (controllerStartPressed)
	{
		auto gameManager = registry.view<GAME::GameManager>();

		if (OverlayIndex == 3)
		{
			OverlayIndex = PrevOverlayIndex;

			for (auto entt : gameManager)
			{
				registry.remove<GAME::Paused>(entt);
			}
			gameMusic.Resume();
		}
		else
		{
			PrevOverlayIndex = OverlayIndex;
			OverlayIndex = 3;

			soundStorage.soundCues[3] = true;

			for (auto entt : gameManager)
			{
				registry.emplace_or_replace<GAME::Paused>(entt);
			}
			gameMusic.Pause();
		}
	}

	// Start the game with A on Controller
	static bool aHeld = false;
	float aButton = 0.0f;
	input.gamePads.GetState(0, G_SOUTH_BTN, aButton);
	bool controllerAPressed = false;

	if (aButton > 0.0f && !aHeld)
	{
		controllerAPressed = true;
	}

	aHeld = aButton > 0.0f;

	if (controllerAPressed && OverlayIndex == 0)
	{
		OverlayIndex = 1;
		menuMusic.Stop();
		gameMusic.Play(true);
	}

	while (+pressEvents.Pop(event))
	{
		auto gameManager = registry.view<GAME::GameManager>();

		if (+event.Read(inputEvent, inputData))
		{
			//Press P or Start to pause, press again to unpause
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_P
				&& (OverlayIndex == 3 || OverlayIndex == 5 || OverlayIndex == 7 || OverlayIndex == 2)) {

				if (OverlayIndex == 3) {
					if (PrevOverlayIndex == 2) {
						OverlayIndex = PrevOverlayIndex;
					}
					else {
						OverlayIndex = 2;
						justLooking = false;
					}
					for (auto ent : gameManager) {
						registry.remove<GAME::Paused>(ent);
					}
					gameMusic.Resume();
				}
				else {
					PrevOverlayIndex = OverlayIndex;
					OverlayIndex = 3;
					soundStorage.soundCues[3] = true;
					for (auto ent : gameManager) {
						registry.emplace_or_replace<GAME::Paused>(ent);
					}
					gameMusic.Pause();
				}
			}
			//When paused, press O to open settings, press again to close settings
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_O
				&& OverlayIndex == 3) {
				settingsOpen = !settingsOpen;
			}

			// Reset button. Completely reset game state.
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_Y
				&& (OverlayIndex == 3 || OverlayIndex == 5 || OverlayIndex == 7)) {

				// Reset level manager back to level 1
				auto lmView = registry.view<GAME::LevelManager>();
				for (auto entity : lmView)
				{
					auto& lm = registry.get<GAME::LevelManager>(entity);
					std::shared_ptr<const GameConfig> config = registry.ctx().get<UTIL::Config>().gameConfig;
					std::string waveFile = config->at("Level1").at("waveFile").as<std::string>();
					lm.level = GAME::LoadLevelData(waveFile);
					lm.time = 0.0f;
					lm.nextWaveIndex = 0;
					lm.levelComplete = false;
					lm.readyForNextLevel = false;
					lm.levelIndex = 0;
				}

				// Destroy all enemies, powerups, bullets and enemy bullets immediately
				auto enemyView = registry.view<GAME::Enemy>();
				for (auto enemy : enemyView)
					registry.destroy(enemy);

				auto bulletView = registry.view<GAME::Bullet>();
				for (auto bullet : bulletView)
					registry.destroy(bullet);

				auto enemyBulletView = registry.view<GAME::EnemyBullets>();
				for (auto bullet : enemyBulletView)
					registry.destroy(bullet);				

				// Reset player
				auto playerView = registry.view<GAME::Player, GAME::Health, GAME::Lives, GAME::Transform>();
				for (auto player : playerView)
				{
					auto& config = registry.ctx().get<UTIL::Config>().gameConfig;
					auto& health = registry.get<GAME::Health>(player);
					auto& lives = registry.get<GAME::Lives>(player);
					health.HP = config->at("Player").at("hitpoints").as<int>();
					lives.count = config->at("Player").at("lives").as<int>();

					// Reset player position to center
					auto& transform = registry.get<GAME::Transform>(player);
					GW::MATH::GMatrix::IdentityF(transform.matrix);

					// Restore player visibility
					if (registry.all_of<DRAW::MeshCollection>(player))
					{
						auto& meshes = registry.get<DRAW::MeshCollection>(player);
						for (auto mesh : meshes.meshEntities)
							registry.emplace_or_replace<GAME::Visible>(mesh).show = true;
					}

					// Remove any leftover state components
					if (registry.all_of<GAME::RespawnTimer>(player))
						registry.remove<GAME::RespawnTimer>(player);
					if (registry.all_of<GAME::Invuln>(player))
						registry.remove<GAME::Invuln>(player);
					if (registry.all_of<GAME::Roll>(player))
						registry.remove<GAME::Roll>(player);
					
					ClearPowerUPs(registry, player);

					// Reset roll charges
					if (registry.all_of<GAME::RollCharges>(player))
						registry.get<GAME::RollCharges>(player).charges = 3;

					//Check if player has collider
					if (registry.all_of<GAME::Collidable>(player))
					{
						std::cout << "Player has collider" << std::endl;
					}
					else
					{
						std::cout << "Player has NO collider" << std::endl;
						// If not, add it back
						registry.emplace_or_replace<GAME::Collidable>(player);
					}

				}

				// Reset score
				registry.ctx().get<ScoreSystem>().Reset();

				// Remove GameOver from game manager
				auto gmView = registry.view<GAME::GameManager>();
				for (auto gm : gmView)
				{
					if (registry.all_of<GAME::GameOver>(gm))
						registry.remove<GAME::GameOver>(gm);
					if (registry.all_of<GAME::Paused>(gm))
						registry.remove<GAME::Paused>(gm);
				}

				// Restore menu music state
				gameMusic.Stop();
				menuMusic.Play(true);

				OverlayIndex = 0;
				totalKilled = 0;
				totalSpawned = 0;
				namedScore = false;
			}
			
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_L
				&& (OverlayIndex == 3 || OverlayIndex == 5)) {
				justLooking = true;
				OverlayIndex = 7;
			}

			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_SPACE
				&& OverlayIndex == 0) {
				OverlayIndex = 1;
				menuMusic.Stop();
				gameMusic.Play(true);
			}
			if (levelStart) {
				OverlayIndex = 2;
				levelStart = false;
			}

			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && (inputData.data == G_KEY_NUMPAD_6 || inputData.data == G_KEY_RIGHT) && settingsOpen) {
				switch (volIndex) {
				case 3:
					masterVol += volChange;
					break;
				case 4:
					musicVol += volChange;
					break;
				case 5:
					sfxVol += volChange;
					break;
				}
			}

			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && (inputData.data == G_KEY_NUMPAD_4 || inputData.data == G_KEY_LEFT) && settingsOpen) 
			{
				switch (volIndex) {
				case 3:
					masterVol -= volChange;
					break;
				case 4:
					musicVol -= volChange;
					break;
				case 5:
					sfxVol -= volChange;
					break;
				}
			}
			audio.SetMasterVolume(masterVol);
			menuMusic.SetVolume(musicVol);
			gameMusic.SetVolume(musicVol);
			for (int i = 0; i < soundStorage.sounds.size(); i++) {
				soundStorage.sounds[i].SetVolume(sfxVol);
			}

			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_ESCAPE 
				&& !registry.ctx().contains<GAME::QuitRequested>() && (OverlayIndex == 3 || OverlayIndex == 5))
			{
				// Close the window
				registry.ctx().emplace<GAME::QuitRequested>();
			}


			auto& leaderboard = registry.ctx().get<HighscoreScreenController>();
			auto& Initials = registry.ctx().get<InitialsEntrySystem>();

			if (!namedScore && !justLooking && OverlayIndex == 7)
			{
				forTyping[forTyping.size() - 1][Initials.GetSelectedIdx()] = Initials.GetCharAt(Initials.GetSelectedIdx());
			}

			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && (inputData.data == G_KEY_NUMPAD_6 || inputData.data == G_KEY_RIGHT)) {
				Initials.MoveRight();
			}
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && (inputData.data == G_KEY_NUMPAD_4 || inputData.data == G_KEY_LEFT)) {
				Initials.MoveLeft();
			}
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && (inputData.data == G_KEY_NUMPAD_8 || inputData.data == G_KEY_UP)) {
				Initials.MoveUp();
			}
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && (inputData.data == G_KEY_NUMPAD_2 || inputData.data == G_KEY_DOWN)) {
				Initials.MoveDown();
			}
			else if (inputData.data == G_KEY_ENTER) {
				if (!Initials.HaveOneLetter()) {
					forTyping[forTyping.size() - 1] = "MUST HAVE AT LEAST 1 LETTER";
				}
				else {
					leaderboard.SubmitInitials(registry, forTyping[forTyping.size() - 1]);
					namedScore = true;
				}
			}
		}
	}
	auto& displayOvl = registry.ctx().get<Overlay>();
	auto& ctxBltr = registry.ctx().get<GW::GRAPHICS::GBlitter>();
	auto& ctxFont = registry.ctx().get<BLIT_Font>();

	auto lmView = registry.view<GAME::LevelManager>();
	auto lmEntity = lmView.front();
	auto& lm = registry.get<GAME::LevelManager>(lmEntity);
	if (lm.levelIndex > levelIndex || (lm.levelIndex == 0 && levelIndex == 2)) {
		levelIndex = lm.levelIndex;
		auto gameManager = registry.view<GAME::GameManager>();
		for (auto ent : gameManager) {
			registry.emplace<GAME::Paused>(ent);
		}
		OverlayIndex = 4;
		auto& Bonus = registry.ctx().get<ScoreSystem>();
		auto playerView = registry.view<GAME::Player, GAME::RollCharges>();
		for (auto entity : playerView)
		{
			auto& charges = registry.get<GAME::RollCharges>(entity).charges;
			for (int i = 0; i < charges; i++) {
				Bonus.AddPoints(1000);
			}
		}
		Bonus.AddPoints(LevelProficiency(lm.enemyTotal, lm.enemyKilled));
		totalKilled += lm.enemyKilled;
		totalSpawned += lm.enemyTotal;
	}

	if (gameWon) {
		OverlayIndex = 6;
	}

	switch (OverlayIndex) {
	case 0:
		Title(displayOvl, ctxBltr, ctxFont, W, H, GameStart[3]);
		StartMenu(registry, ovl, bltr, font, W, H);
		break;
	case 1:
		ClearLargerText(displayOvl, ctxBltr, ctxFont, W, H);
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
		HighScoreMenu(registry, ovl, bltr, font, W, H, displayOvl, ctxBltr, ctxFont);
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
	auto& leaderboard = registry.ctx().get<HighscoreScreenController>();
	auto player = registry.view<GAME::Player>();
	for (auto entity : player) {
		auto hitpoints = registry.get<GAME::Lives>(entity).count;
		for (int i = 0; i < hitpoints; i++) {
			hits += '^';
		}
		if(hitpoints == 0) {
			hits = "Vessel Destroyed";
			if (!leaderboard.IsNewHighscore() && !justLooking) {
				OverlayIndex = 5;
			}
		}
	}
	RenderOnScreen(font, 5, H - 20, hits);
}

std::string BuildRollCharges(int charges)
{
	std::string result;
	for (int i = 0; i < charges; i++)
		result += "R ";
	return result;
}

void SetRegularUI(entt::registry& registry, BLIT_Font& font, int W, int H) {
	RenderOnScreen(font, W / 8, 25, UI[0]);
	RenderOnScreen(font, (W / 2) - 100, 25, UI[2]);
	RenderOnScreen(font, W - (W / 6), 25, UI[1]);
	auto& scoreSystem = registry.ctx().get<ScoreSystem>();
	auto& highscore = registry.ctx().get<LocalHighscoreSystem>();
	int liveScore = scoreSystem.GetScore();
	std::string score = std::to_string(liveScore);
	std::string localHighscore = std::to_string(highscore.GetHighscore());
	RenderOnScreen(font, (W / 2), 60, localHighscore);
	RenderOnScreen(font, (W / 8) + 15, 60, score);
	countLives(registry, font, W, H);

	// Roll charges display
	auto playerView = registry.view<GAME::Player, GAME::RollCharges>();
	for (auto entity : playerView)
	{
		auto& charges = registry.get<GAME::RollCharges>(entity);
		std::string rollStr = BuildRollCharges(charges.charges);
		int xPos = W - 15 - (charges.charges * 25); // Subtract 25 per charge
		RenderOnScreen(font, xPos, H - 20, rollStr);
	}
}

void RenderOnScreen(BLIT_Font& font, int W, int H, std::string text) {
	font.DrawTextImmediate(W, H, text.c_str(), text.length());
}

void TypeLines(entt::registry& registry, BLIT_Font& font, int W, int H, std::vector<std::string>& 
texts, std::vector<float>& timers, std::vector<int>& keyIndices, int lineCount) {
	forTyping.resize(texts.size() + 1);
	if (lineCount > texts.size()) {
		lineCount = texts.size();
	}
	for (int i = 0; i < lineCount; i++) {
		forTyping[i] = (TypewriterEffect(registry, forTyping[i], texts[i], timers[i], keyIndices[i]));
		RenderOnScreen(font, W, H + (i * LineSpace), forTyping[i]);
	}
}

void TypeVictoryLines(entt::registry& registry, BLIT_Font& font, int W, int H, std::vector<std::string>&
	texts, std::vector<float>& timers, std::vector<int>& keyIndices, int lineCount) {
	forTyping.resize(texts.size() + 1);
	if (lineCount > texts.size()) {
		lineCount = texts.size();
	}
	int fullWidth = W;
	for (int i = 0; i < lineCount - 1; i++) {
		if(lineCount >= i)
		forTyping[i] = (TypewriterEffect(registry, forTyping[i], texts[i], timers[i], keyIndices[i]));
		if (i == 0) 
		{
			W = (fullWidth / 3) - 50;
		}
		if (i == 1)
		{
			W = (fullWidth / 3) + 45;
		}
		if (i == 2)
		{
			W = (fullWidth / 3) + 50;
		}
		if (i == 3)
		{
			W = (fullWidth / 3);
		}
		if (i == 4)
		{
			W = (fullWidth / 3) + 25;
		}
		if (i == 5)
		{
			W = (fullWidth / 3);
		}
		RenderOnScreen(font, W, H - (i * LineSpace), forTyping[i]);
	}
}

void RegularOptions(BLIT_Font& font, int W, int H) {
	RenderOnScreen(font, (W / 4) - 90, H - 75, MenuOptions[0]);
	RenderOnScreen(font, (W / 2) - 90, H - 75, "QUIT [ESC]");
	RenderOnScreen(font, (W / 4) * 3 - 90, H - 75, MenuOptions[1]);
	RenderOnScreen(font, (W / 2) - 130, H - 150, MenuOptions[MenuOptions.size() - 2]);
}

std::string ShowVolume(float volume) {
	int range;
	range = static_cast<int>(volume / 0.01f);
	if (range <= 0) {
		volume = 0.00f;
		return "OFF";
	}
	if (range >= 10) {
		volume = 0.1f;
		return "MAX";
	}
	return std::to_string(range);
}

int LevelProficiency (float spawned, float killed) {
	ElimPercentages.push_back((killed / spawned) * 100);
	int latest = ElimPercentages[ElimPercentages.size() - 1];
	if ( latest >= 100) {
		return 50000;
	}
	else if (latest > 95) {
		return 20000;
	}
	else if (latest > 90) {
		return 10000;
	}
	else if (latest > 85) {
		return 5000;
	}
	else if (latest > 80) {
		return 4000;
	}
	else if (latest > 70) {
		return 3000;
	}
	else if (latest > 60) {
		return 2000;
	}
	else if (latest > 50) {
		return 1000;
	}
	else if (latest < 50) {
		return 0;
	}
}

std::string CalculateTodaysTop() {
	if (ElimPercentages.empty()) {
		return std::to_string(0) + '%';
	}
	std::sort(ElimPercentages.begin(), ElimPercentages.end(), [](int a, int b) {
		return a > b;
	});
	return std::to_string(ElimPercentages[0]) + '%';
}

void FlashingUnderLine(entt::registry& registry, BLIT_Font& font, int W, int H, std::string text) {
	std::string underline;
	for (int i = 0; i < text.length(); i++) {
		underline += '_';
	}
	RenderOnScreen(font, W, H + 10, underline);
}

// QUEUES THE TEXT
void QueuePowerUpText(entt::registry& registry, BLIT_Font& font, int W, int H)
{
	auto playerView = registry.view<GAME::Player>();
	if (playerView.empty()) return;
	auto player = playerView.front();

	int yOffset = H/2 + 10;

	// 1. Queue SideFighter
	if (registry.any_of<GAME::HasSideFighters>(player))
	{
		auto& sf = registry.get<GAME::HasSideFighters>(player);
		std::stringstream stream;
		stream << std::fixed << std::setprecision(1) << sf.timer;

		if (sf.timer > 3.0f || (static_cast<int>(sf.timer * 10) % 2 == 0))
		{
			int textX = 5; // Default starting position

			// Ask the dictionary for the width!
			if (activeUIIcons.count("SideFighter") > 0 && activeUIIcons["SideFighter"].pixels != nullptr) {
				textX = 5 + activeUIIcons["SideFighter"].width + 5;
			}

			RenderOnScreen(font, textX, yOffset, " " + stream.str() + "s");
		}
		yOffset -= 45;
	}

	// 2. Queue Multi-Shot
	if (registry.any_of<GAME::MultiShot>(player))
	{
		auto& ms = registry.get<GAME::MultiShot>(player);
		std::stringstream stream; stream << std::fixed << std::setprecision(1) << ms.timer;

		if (ms.timer > 3.0f || (static_cast<int>(ms.timer * 10) % 2 == 0))
		{
			int textX = 5; // Default starting position

			if (activeUIIcons.count("MultiShot") > 0 && activeUIIcons["MultiShot"].pixels != nullptr) {
				textX = 5 + activeUIIcons["MultiShot"].width + 5;
			}

			RenderOnScreen(font, textX, yOffset, " " + stream.str() + "s");
		}
		yOffset -= 45;
	}
}

// DRAWS THE IMAGE
void DrawPowerUpImages(entt::registry& registry, int W, int H, unsigned int* screenPixels)
{
	auto playerView = registry.view<GAME::Player>();
	if (playerView.empty()) return;
	auto player = playerView.front();

	int yOffset = H/2;

	// 1. Draw SideFighter Image
	if (registry.any_of<GAME::HasSideFighters>(player))
	{
		auto& sf = registry.get<GAME::HasSideFighters>(player);
		if (sf.timer > 3.0f || (static_cast<int>(sf.timer * 10) % 2 == 0))
		{
			// Grab the icon bundle from the dictionary
			UIIcon& icon = activeUIIcons["SideFighter"];
			if (icon.pixels != nullptr) {
				DrawImageToOverlay(screenPixels, W, H, icon.pixels, icon.width, icon.height, 5, yOffset - 15);
			}
		}
		yOffset -= 45;
	}

	// 2. Draw Multi-Shot Image
	if (registry.any_of<GAME::MultiShot>(player))
	{
		auto& ms = registry.get<GAME::MultiShot>(player);
		if (ms.timer > 3.0f || (static_cast<int>(ms.timer * 10) % 2 == 0))
		{
			UIIcon& icon = activeUIIcons["MultiShot"];
			if (icon.pixels != nullptr) {
				DrawImageToOverlay(screenPixels, W, H, icon.pixels, icon.width, icon.height, 5, yOffset - 15);
			}
		}
		yOffset -= 45;
	}
}

void DrawImageToOverlay(unsigned int* screenPixels, int screenW, int screenH,
	unsigned int* imgPixels, int imgW, int imgH,
	int startX, int startY)
{
	if (!imgPixels || !screenPixels) return;

	for (int y = 0; y < imgH; ++y)
	{
		for (int x = 0; x < imgW; ++x)
		{
			int screenX = startX + x;
			int screenY = startY + y;

			if (screenX >= 0 && screenX < screenW && screenY >= 0 && screenY < screenH)
			{
				unsigned int color = imgPixels[y * imgW + x];

				// stb_image loads as RGBA. On Windows, this reads backwards as ABGR.
				// We need to extract the channels and re-pack them as ARGB!
				unsigned char r = (color & 0xFF);
				unsigned char g = ((color >> 8) & 0xFF);
				unsigned char b = ((color >> 16) & 0xFF);
				unsigned char a = ((color >> 24) & 0xFF);

				// Only draw if it's mostly solid (Alpha > 10)
				if (a > 10)
				{
					// Re-pack the bytes in the correct Gateware order (AARRGGBB)
					unsigned int fixedColor = (a << 24) | (r << 16) | (g << 8) | b;
					screenPixels[screenY * screenW + screenX] = fixedColor;
				}
			}
		}
	}
}