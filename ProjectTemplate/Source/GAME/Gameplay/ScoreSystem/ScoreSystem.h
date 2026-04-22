#pragma once

class ScoreSystem
{
public:
	void Reset()
	{
		totalScore = 0;
	}

	void AddPoints(int points)
	{
		totalScore += points;

		if (totalScore <= 0)
		{
			totalScore = 0;
		}
	}

	int GetScore() const
	{
		return totalScore;
	}

	void SetScore(int score)
	{
		totalScore = score;

		if (totalScore < 0)
		{
			totalScore = 0;
		}
	}

private:
	int totalScore = 0;
};