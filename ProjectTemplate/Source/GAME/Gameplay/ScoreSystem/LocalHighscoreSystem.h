#pragma once
#include <string>

class LocalHighscoreSystem
{
public:
	void Load();
	void Save();
	void Update(int liveScore);
	
	int GetHighscore() const;

private:
	int localHighscore = 0;
};