#pragma once
#include <vector>
#include <algorithm>
#include "LeaderboardEntry.h"

class LeaderboardSystem
{
public:
	const std::vector<LeaderboardEntry>& GetEntries() const
	{
		return entries;
	}

	void SetEntries(const std::vector<LeaderboardEntry>& newEntries)
	{
		entries = newEntries;
		SortDescending();
		Top5();
	}

	bool IsHighscore(int score) const
	{
		if (entries.size() < 5)
		{
			return true;
		}

		return score > entries.back().score;
	}

	void InsertScore(const std::string& initials, int score)
	{
		entries.push_back({ initials, score });
		SortDescending();
		Top5();
	}

private:
	std::vector<LeaderboardEntry> entries;

	void SortDescending()
	{
		std::sort(entries.begin(), entries.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b)
		{
			return a.score > b.score;
		});
	}

	void Top5()
	{
		if (entries.size() > 5)
		{
			entries.resize(5);
		}
	}
};