#pragma once
#include <vector>
#include <string>
#include "../../../../entt-3.13.1/single_include/entt/entt.hpp"
#include "LeaderboardEntry.h"

class HighscoreScreenController
{
public:
	// Call this when player loses
	bool Begin(entt::registry& registry);

	bool NeedsInitialsEntry() const;

	bool SubmitInitials(entt::registry& registry, const std::string& initials);

	bool IsNewHighscore() const;

	// Check if leaderboard is loaded
	bool IsLoaded() const;

	// Read leaderboard entries from here
	const std::vector<LeaderboardEntry>& GetEntries() const;

	int GetFinalScore() const;

	int GetLocalBestScore() const;

	void Reset();

private:
	std::vector<LeaderboardEntry> visibleEntries;
	int finalScore = 0;
	int localBestScore = 0;
	bool loaded = false;
	bool newHighscore = false;
	bool submitted = false;
	static bool IsValidInitials(const std::string& initials);
};