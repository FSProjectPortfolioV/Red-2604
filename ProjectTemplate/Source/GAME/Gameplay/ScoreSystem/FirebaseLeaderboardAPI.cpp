#include "FirebaseLeaderboardAPI.h"
#include <iostream>
#include "../../../../json-develop/single_include/nlohmann/json.hpp"
#include <Windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

// Helper due to WinHttp needing wstring
static std::wstring StringToWide(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}

// Get request
bool FirebaseLeaderboardAPI::HttpGet(const std::string& host, const std::string& path, std::string& response)
{
    response.clear();

    HINTERNET session = WinHttpOpen(L"LeaderboardClient/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session)
    {
        return false;
    }

    HINTERNET connection = WinHttpConnect(session, StringToWide(host).c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connection)
    {
        WinHttpCloseHandle(session);
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(connection, L"GET", StringToWide(path).c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!request)
    {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    bool result = WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (result)
    {
        result = WinHttpReceiveResponse(request, NULL);

        DWORD size = 0;
        
        do
        {
            DWORD downloaded = 0;

            if (!WinHttpQueryDataAvailable(request, &size))
            {
                break;
            }

            if (size == 0)
            {
                break;
            }

            std::string buffer(size, '\0');

            if (!WinHttpReadData(request, buffer.data(), size, &downloaded))
            {
                break;
            }
            response.append(buffer.c_str(), downloaded);
        } while (size > 0);
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);
    
    return result;
}

// Put request
bool FirebaseLeaderboardAPI::HttpPut(const std::string& host, const std::string& path, const std::string& jsonBody)
{
    HINTERNET session = WinHttpOpen(L"LeaderboardClient/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session)
    {
        return false;
    }

    HINTERNET connection = WinHttpConnect(session, StringToWide(host).c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connection)
    {
        WinHttpCloseHandle(session);
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(connection, L"PUT", StringToWide(path).c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!request)
    {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    bool result = WinHttpSendRequest(request, L"Content-Type: application/json\r\n", -1L, (LPVOID)jsonBody.c_str(), (DWORD)jsonBody.size(), (DWORD)jsonBody.size(), 0);
    if (result)
    {
        result = WinHttpReceiveResponse(request, NULL);
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    return result;
}

// Get highscores
bool FirebaseLeaderboardAPI::GetScores(std::vector<LeaderboardEntry>& scoreList)
{
    scoreList.clear();

    std::string response;

    if (!HttpGet(database, leaderboard, response))
    {
        std::cout << "Error getting scores\n";
        return false;
    }

    if (response.empty() || response == "null")
    {
        return true;
    }

    json result = json::parse(response);

    for (auto& item : result.items())
    {
        LeaderboardEntry entry;
        entry.initials = item.value().value("initials", "");
        entry.score = item.value().value("score", 0);

        scoreList.push_back(entry);
    }

    return true;
}

// Save highscores
bool FirebaseLeaderboardAPI::SaveScores(const std::vector<LeaderboardEntry>& scoreList)
{
    json result = json::object();

    for (size_t i = 0; i < scoreList.size(); i++)
    {
        result[std::to_string(i)] = 
        {
            { "initials", scoreList[i].initials },
            { "score", scoreList[i].score }
        };
    }

    std::string jsonBody = result.dump();

    if (!HttpPut(database, leaderboard, jsonBody))
    {
        std::cout << "Error saving scores\n";
        return false;
    }

    return true;
}