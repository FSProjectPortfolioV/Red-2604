#pragma once
#include "../GAME/GameComponents.h"
#include "./Utility/FileIntoString.h"
#include "shaderc/shaderc.h"
#include "./Overlay.h"
#include "./Font.h"
#include "./BLIT_Font.h"
#include "../UTIL/Utilities.h"
#include "../GAME/Gameplay/ScoreSystem/ScoreSystem.h"
#include "../GAME/GamePlay/ScoreSystem/LeaderboardSystem.h"
#include "../GAME/GamePlay/ScoreSystem/HighscoreScreenController.h"
#include "../GAME/Gameplay/ScoreSystem/LocalHighscoreSystem.h"

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
int LineSpace = -100;
int FinaleIdx = 1;
int FinaleIdx2 = 1;
int FinaleIdx3 = 1;
bool settingsOpen = false;
bool levelStart = false;
int OverlayIndex = 0;
int PrevOverlayIndex = 0;
int finalScreenCounter = 1;
float sfxVol = 0.005f;
float musicVol = 0.05f;
float masterVol = 0.05f;
float volChange = 0.05f;
float sfxVolChange = 0.001f;
int volIndex = 0;

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
	"ALL ENEMIES DESTROYED !!",
	"SPECIAL BONUS",
	"00, 000, 000 PTS",
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
	gMusic2.Create(psMusic, gAudio, musicVol);
	gMusic2.Play(true);
	gMusic2.Pause();

	GSound& gSound = registry.ctx().emplace<GSound>();
	registry.emplace<GSound>(entity);
	registry.ctx().emplace<GAME::SoundCue>();
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
	FlashingEffect(registry, font, (W / 2) - 120, (H / 2) + 200, GameStart[4]);
	unsigned int* pixels;
	ovl.LockForUpdate(W * H, &pixels);
	bltr.ExportResult(false, W, H, 0, 0, pixels, nullptr, nullptr); 
	ovl.Unlock();
	ovl.TransferOverlay();
}

static void EndOfLevel(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	auto gameManager = registry.view<GAME::GameManager>();
	for (auto ent : gameManager) {
		registry.emplace_or_replace<GAME::Paused>(ent);
	}
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
	bltr.ClearColor(0x00000000);
	SetRegularUI(registry, font, W, H);
	RenderOnScreen(font, (W / 2) - 35, (H / 2) - 200, LevelBegin[0]);
	RenderOnScreen(font, (W / 2) - 125, (H / 2) - 125, LevelBegin[1]);
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
			}
			TypeLines(registry, font, (W / 3) - 125, (H / 2) - 200, FinalStats, ScreenTimers, KeyCounters, FinaleIdx);
			if (FinaleIdx == FinalStats.size()) {
				screenTimer = 0.0f;
			}
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

static void HighScoreMenu(entt::registry& registry, Overlay& ovl, GW::GRAPHICS::GBlitter& bltr, BLIT_Font& font, int W, int H) {
	auto& pressEvents = registry.ctx().get<GW::CORE::GEventCache>();
	GW::GEvent event;
	auto& leaderboard = registry.ctx().get<HighscoreScreenController>();
	auto score = registry.ctx().get<ScoreSystem>().GetScore();
	GW::INPUT::GBufferedInput::Events inputEvent;
	GW::INPUT::GBufferedInput::EVENT_DATA inputData;

	if (leaderboard.Begin(registry)) {
		FlashingEffect(registry, font, (W / 2) - 100, (H / 2) - 50, "New Highscore!");
		RenderOnScreen(font, (W / 2) - 100, (H / 2) - 150, "Input Initials (Ex. \"ABC\")");
		if (forTyping.size() == 0) {
			forTyping.push_back("");
		}
		RenderOnScreen(font, (W / 2) - 100, (H / 2) - 150, forTyping[forTyping.size() - 1]);
		while (+pressEvents.Pop(event))
		{
			auto gameManager = registry.view<GAME::GameManager>();

			if (+event.Read(inputEvent, inputData))
			{
				if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data != G_KEY_ENTER) {
					forTyping[forTyping.size() - 1] += (char)inputData.data;
				}
			}
		}
	}
	bltr.ClearColor(0x00000000);
	RenderOnScreen(font, W / 8, 25, UI[0]);
	RenderOnScreen(font, (W / 2) - 100, 25, UI[2]);
	RenderOnScreen(font, W - (W / 6), 25, UI[1]);
	RenderOnScreen(font, (W / 2), 60, std::to_string(score));
	RenderOnScreen(font, (W / 8) + 15, 60, std::to_string(score));
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
	auto& gameMusic = registry.ctx().get<GMusic>();
	auto& audio = registry.ctx().get<GAudio>();
	auto& ctxSfx = registry.ctx().get<GSound>();
	auto& sfx = registry.get<GSound>(entity);
	auto& displayMusic = registry.get<GMusic>(entity);
	auto& soundCues = registry.ctx().get<GAME::SoundCue>();
	if (soundCues.sound1) {
		const char* hitSound = config->at("Sounds").at("phit").as<const char*>();
		ctxSfx.Create(hitSound, audio);
		ctxSfx.Play();
		soundCues.sound1 = false;
	}
	if (soundCues.sound2) {
		const char* shootSound = config->at("Sounds").at("pshoot").as<const char*>();
		sfx.Create(shootSound, audio);
		sfx.Play();
		soundCues.sound2 = false;
	}
	while (+pressEvents.Pop(event))
	{
		auto gameManager = registry.view<GAME::GameManager>();

		if (+event.Read(inputEvent, inputData))
		{
			//Press P to pause, press again to unpause
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_P) {

				if (OverlayIndex == 3) {
					OverlayIndex = PrevOverlayIndex;
					for (auto ent : gameManager) {
						registry.remove<GAME::Paused>(ent);
					}
					gameMusic.Resume();
					displayMusic.Pause();
				}
				else {
					PrevOverlayIndex = OverlayIndex;
					OverlayIndex = 3;
					const char* pauseSound = (*config).at("Sounds").at("menu").as<const char*>();
					sfx.Create(pauseSound, audio, sfxVol);
					sfx.Play();
					for (auto ent : gameManager) {
						registry.emplace_or_replace<GAME::Paused>(ent);
					}
					displayMusic.Resume();
					gameMusic.Pause();
				}
			}
			//When paused, press O to open settings, press again to close settings
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_O
				&& OverlayIndex == 3) {
				settingsOpen = !settingsOpen;
			}

			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_Y
				&& (OverlayIndex == 3 || OverlayIndex == 5)) {
				OverlayIndex = 0;
			}
			
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_L
				&& (OverlayIndex == 3 || OverlayIndex == 5)) {
				OverlayIndex = 7;
			}

			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && inputData.data == G_KEY_SPACE
				&& OverlayIndex == 0) {
				OverlayIndex = 1;
			}
			if (levelStart) {
				OverlayIndex = 2;
				levelStart = false;
			}
			int check = inputData.data;
			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && (inputData.data == G_KEY_NUMPAD_6 || inputData.data == G_KEY_RIGHT) && settingsOpen) {
				switch (volIndex) {
				case 3:
					masterVol += volChange;
					break;
				case 4:
					musicVol += volChange;
					break;
				case 5:
					sfxVol += sfxVolChange;
					break;
				}
			}

			if (inputEvent == GW::INPUT::GBufferedInput::Events::KEYPRESSED && (inputData.data == G_KEY_NUMPAD_4 || inputData.data == G_KEY_LEFT) && settingsOpen) {
				switch (volIndex) {
				case 3:
					masterVol -= volChange;
					break;
				case 4:
					musicVol -= volChange;
					break;
				case 5:
					sfxVol -= sfxVolChange;
					break;
				}
			}
			audio.SetMasterVolume(masterVol);
			gameMusic.SetVolume(musicVol);
			displayMusic.SetVolume(musicVol);
			ctxSfx.SetVolume(sfxVol);
			sfx.SetVolume(sfxVol);
		}
	}
	auto& displayOvl = registry.ctx().get<Overlay>();
	auto& ctxBltr = registry.ctx().get<GW::GRAPHICS::GBlitter>();
	auto& ctxFont = registry.ctx().get<BLIT_Font>();
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
	RenderOnScreen(font, 5, H - 20, hits);
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
		RenderOnScreen(font, W, H - (i * LineSpace), forTyping[i]);
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
	RenderOnScreen(font, (W / 3) - 100, H - 100, MenuOptions[0]);
	RenderOnScreen(font, (W / 2) + 100, H - 100, MenuOptions[1]);
	RenderOnScreen(font, (W / 2) - 150, H - 200, MenuOptions[MenuOptions.size() - 2]);
}

std::string ShowVolume(float volume) {
	int range;
	if(volume == sfxVol) {
		volume = sfxVol;
		range = static_cast<int>(volume / 0.001f);
	}
	else {
		 range = static_cast<int>(volume / 0.05f);
	}
	if (range <= 0) {
		volume = 0.00f;
		return "OFF";
	}
	if (range >= 6) {
		volume = 0.30f;
		return "MAX";
	}
	return std::to_string(range);
}

//std::string ProficiencyPercentage(entt::registry& registry) {
//	
//}

std::string CalculateTodaysTop(std::vector<int> percentages) {
	std::sort(percentages.begin(), percentages.end(), [](int a, int b) {
		return a > b;
	});
	return std::to_string(percentages[0]) + "%";
}

void FlashingUnderLine(entt::registry& registry, BLIT_Font& font, int W, int H, std::string text) {
	std::string underline;
	for (int i = 0; i < text.length(); i++) {
		underline += '_';
	}
	RenderOnScreen(font, W, H + 10, underline);
}