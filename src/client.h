#pragma once
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

namespace SecAuth {
struct Profile {
    std::string username;
    std::string activatedAt;
    std::string expiresAt;
    long long remainingSeconds = 0;
};

class Client {
public:
    Client(const std::wstring& server, const std::string& appId, const std::string& desktopKey, const std::string& version);
    ~Client();
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    void Register(const std::string& username, const std::string& email,
                  const std::string& password, const std::string& license);
    Profile Login(const std::string& username, const std::string& password);
    Profile Refresh();
    bool Logout();
    bool IsLoggedIn() const;

private:
    Profile Authorize(const std::wstring& endpoint, const std::string& body);
    std::string Request(const std::wstring& endpoint, const std::string& body) const;
    void EraseToken();
    void StartAutoRefresh();
    void StopAutoRefresh();

    std::wstring host_;
    std::wstring basePath_;
    unsigned short port_ = 0;
    bool tls_ = true;
    std::string appId_;
    std::string desktopKey_;
    std::string version_;
    std::string device_;
    std::string token_;
    std::chrono::steady_clock::time_point verifiedAt_{};
    mutable std::mutex stateMutex_;
    std::mutex workerMutex_;
    std::condition_variable workerCv_;
    std::thread autoRefreshThread_;
    bool workerStop_ = false;
};
}
