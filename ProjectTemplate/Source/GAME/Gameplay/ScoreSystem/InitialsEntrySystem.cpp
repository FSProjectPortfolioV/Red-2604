#include "InitialsEntrySystem.h"

const std::string InitialsEntrySystem::allowedChar = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";

InitialsEntrySystem::InitialsEntrySystem()
{
	Reset();
}

void InitialsEntrySystem::Reset()
{
	charIdxs = { 0, 0, 0 };
	selectedIdx = 0;
}

void InitialsEntrySystem::MoveLeft()
{
	if (selectedIdx > 0)
	{
		selectedIdx--;
	}
}

void InitialsEntrySystem::MoveRight()
{
	if (selectedIdx < 2)
	{
		selectedIdx++;
	}
}

void InitialsEntrySystem::MoveUp()
{
	charIdxs[selectedIdx]++;

	if (charIdxs[selectedIdx] >= (int)allowedChar.size())
	{
		charIdxs[selectedIdx] = 0;
	}
}

void InitialsEntrySystem::MoveDown()
{
	charIdxs[selectedIdx]--;

	if (charIdxs[selectedIdx] < 0)
	{
		charIdxs[selectedIdx] = (int)allowedChar.size() - 1;
	}
}

int InitialsEntrySystem::GetSelectedIdx() const
{
	return selectedIdx;
}

char InitialsEntrySystem::GetCharAt(int index) const
{
	if (index < 0 || index > 2)
	{
		return ' ';
	}

	return allowedChar[charIdxs[index]];
}

std::string InitialsEntrySystem::GetInitials() const
{
	std::string result;
	result += allowedChar[charIdxs[0]];
	result += allowedChar[charIdxs[1]];
	result += allowedChar[charIdxs[2]];
	return result;
}

bool InitialsEntrySystem::HaveOneLetter() const
{
	return charIdxs[0] != 0 || charIdxs[1] != 0 || charIdxs[2] != 0;
}