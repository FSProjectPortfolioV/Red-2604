#include "HighscoreScreenController.h"
#include "FirebaseLeaderboardAPI.h"
#include "LeaderboardSystem.h"
#include "ScoreSystem.h"
#include <iostream>

bool HighscoreScreenController::IsValidInitials(const std::string& initials)
{
	if (initials.size() != 3)
	{
		return false;
	}

	bool hasLetter = false;

	for (char c : initials)
	{
		if (c == ' ')
		{
			continue;
		}

		if (c < 'A' || c > 'Z')
		{
			return false;
		}

		hasLetter = true;
	}

	return hasLetter;
}

void HighscoreScreenController::Reset()
{
	visibleEntries.clear();
	playerScore = 0;
	loaded = false;
	newHighscore = false;
	submitted = false;
}

bool HighscoreScreenController::Begin(entt::registry& registry)
{
	Reset();

	auto& firebase = registry.ctx().get<FirebaseLeaderboardAPI>();
	auto& leaderboard = registry.ctx().get<LeaderboardSystem>();
	auto& scoreSystem = registry.ctx().get<ScoreSystem>();

	std::vector<LeaderboardEntry> scores;

	if (!firebase.GetScores(scores))
	{
		std::cout << "Error getting scores\n";
		return false;
	}

	leaderboard.SetEntries(scores);

	playerScore = scoreSystem.GetScore();
	newHighscore = leaderboard.IsHighscore(playerScore);

	visibleEntries = leaderboard.GetEntries();
	loaded = true;

	return true;
}

bool HighscoreScreenController::NeedsInitialsEntry() const
{
	return loaded && newHighscore && !submitted;
}

bool HighscoreScreenController::SubmitInitials(entt::registry& registry, const std::string& initials)
{
	if (!NeedsInitialsEntry())
	{
		return false;
	}

	if (!IsValidInitials(initials))
	{
		return false;
	}

	auto& firebase = registry.ctx().get<FirebaseLeaderboardAPI>();
	auto& leaderboard = registry.ctx().get<LeaderboardSystem>();

	leaderboard.InsertScore(initials, playerScore);

	if (!firebase.SaveScores(leaderboard.GetEntries()))
	{
		std::cout << "Error saving scores\n";
		return false;
	}

	visibleEntries = leaderboard.GetEntries();
	submitted = true;

	return true;
}

const std::vector<LeaderboardEntry>& HighscoreScreenController::GetEntries() const
{
	return visibleEntries;
}

int HighscoreScreenController::GetPlayerScore() const
{
	return playerScore;
}

bool HighscoreScreenController::IsNewHighscore() const
{
	return newHighscore;
}

bool HighscoreScreenController::IsLoaded() const
{
	return loaded;
}