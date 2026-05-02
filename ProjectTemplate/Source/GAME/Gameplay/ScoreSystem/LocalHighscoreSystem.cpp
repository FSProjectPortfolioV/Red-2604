#include "LocalHighscoreSystem.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <fstream>
#include <filesystem>
#include <cstdlib>

#endif // __EMSCRIPTEN__

// EXE save path
#ifndef __EMSCRIPTEN__
static std::string GetSaveFilePath()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);

	std::filesystem::path exePath = std::filesystem::path(buffer).parent_path();
	std::filesystem::path savePath = exePath / "local_highscore.txt";

	return savePath.string();
}
#endif // __EMSCRIPTEN__

void LocalHighscoreSystem::Load()
{
#ifdef __EMSCRIPTEN__
	// HTML
	int savedScore = EM_ASM_INT(
		{
			let score = localStorage.getItem("local_highscore");
			return score ? parseInt(score) : 0;
		});

	localHighscore = savedScore;

#else
	// EXE
	std::ifstream file(GetSaveFilePath());

	if (file.is_open())
	{
		file >> localHighscore;
        file.close();
	}
#endif // __EMSCRIPTEN__
}

void LocalHighscoreSystem::Save()
{
#ifdef __EMSCRIPTEN__
	// HTML
	EM_ASM(
		{
			localStorage.setItem("local_highscore", $0.toString());
		}, localHighscore);

#else
	// EXE
	std::ofstream file(GetSaveFilePath());

	if (file.is_open())
	{
		file << localHighscore;
		file.close();
	}

#endif // __EMSCRIPTEN__
}

void LocalHighscoreSystem::Update(int liveScore)
{
	if (liveScore > localHighscore)
	{
		localHighscore = liveScore;
		Save();
	}
}

int LocalHighscoreSystem::GetHighscore() const
{
	return localHighscore;
}