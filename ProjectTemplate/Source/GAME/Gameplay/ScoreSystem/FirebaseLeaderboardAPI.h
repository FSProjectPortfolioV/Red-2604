#pragma once
#include <string>
#include <vector>
#include "LeaderboardEntry.h"

class FirebaseLeaderboardAPI
{
public:
	FirebaseLeaderboardAPI(const std::string& host, const std::string& path)
	{
		database = host;
		leaderboard = path;
	}

	bool GetScores(std::vector<LeaderboardEntry>& scoreList);
	bool SaveScores(const std::vector<LeaderboardEntry>& scoreList);

private:
	std::string database;
	std::string leaderboard;

	bool HttpGet(const std::string& host, const std::string& path, std::string& response);
	bool HttpPut(const std::string& host, const std::string& path, const std::string& jsonBody);
};