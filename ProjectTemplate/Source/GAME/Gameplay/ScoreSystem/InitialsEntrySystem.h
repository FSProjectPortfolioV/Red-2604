#pragma once
#include <string>
#include <array>

class InitialsEntrySystem
{
public:
	InitialsEntrySystem();
	void Reset();

	// Letter select
	void MoveLeft();
	void MoveRight();
	void MoveUp();
	void MoveDown();

	int GetSelectedIdx() const;
	char GetCharAt(int index) const;
	std::string GetInitials() const;
	bool HaveOneLetter() const;

private:
	std::array<int, 3> charIdxs;
	int selectedIdx;

	static const std::string allowedChar;
};