#pragma once
#include <string>

namespace SecAuth {
struct Profile {
    std::string username;
    std::string activatedAt;
    std::string expiresAt;
    long long remainingSeconds = 0;
};

class Client {
public:
    Client(const std::wstring& server, const std::string& appId, const std::string& desktopKey);
    void Register(const std::string& username, const std::string& email,
                  const std::string& password, const std::string& license);
    Profile Login(const std::string& username, const std::string& password);
    Profile Refresh();
    void Logout();

private:
    Profile Authorize(const std::wstring& endpoint, const std::string& body);
    std::string Request(const std::wstring& endpoint, const std::string& body) const;
    std::wstring host_;
    std::wstring basePath_;
    unsigned short port_ = 0;
    bool tls_ = true;
    std::string appId_;
    std::string desktopKey_;
    std::string device_;
    std::string token_;
};
}
