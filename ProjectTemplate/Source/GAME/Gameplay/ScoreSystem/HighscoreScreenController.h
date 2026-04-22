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

	// Decide if initials input should appear on UI
	bool NeedsInitialsEntry() const;

	// Calls this after the player confirms initials
	bool SubmitInitials(entt::registry& registry, const std::string& initials);

	// UI should read leaderboard entries from here
	const std::vector<LeaderboardEntry>& GetEntries() const;

	// Show player score to UI
	int GetPlayerScore() const;

	// Check if player got a highscore in UI
	bool IsNewHighscore() const;

	// Check if leaderboard is loaded
	bool IsLoaded() const;

	void Reset();

private:
	std::vector<LeaderboardEntry> visibleEntries;
	int playerScore = 0;

	bool loaded = false;
	bool newHighscore = false;
	bool submitted = false;
	static bool IsValidInitials(const std::string& initials);
};