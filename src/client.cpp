#include "client.h"
#include "json.hpp"
#include <windows.h>
#include <winhttp.h>
#include <bcrypt.h>
#include <stdexcept>

namespace {
using Json = nlohmann::json;

struct InternetHandle {
    HINTERNET value;
    explicit InternetHandle(HINTERNET handle) : value(handle) {
        if (!value) throw std::runtime_error("Falha ao iniciar a conexão.");
    }
    InternetHandle(const InternetHandle&) = delete;
    InternetHandle& operator=(const InternetHandle&) = delete;
    ~InternetHandle() { WinHttpCloseHandle(value); }
};

std::wstring Wide(const std::string& input) {
    if (input.empty()) return {};
    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(),
                                        static_cast<int>(input.size()), nullptr, 0);
    if (!size) throw std::runtime_error("Configuração inválida.");
    std::wstring output(size, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(),
                        static_cast<int>(input.size()), &output[0], size);
    return output;
}

std::string DeviceId(const std::string& appId) {
    wchar_t guid[64] = {};
    DWORD size = sizeof(guid);
    const auto status = RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography",
        L"MachineGuid", RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY, nullptr, guid, &size);
    if (status != ERROR_SUCCESS || size <= sizeof(wchar_t)) {
        throw std::runtime_error("Não foi possível identificar este dispositivo.");
    }

    std::string input = appId;
    input.append(reinterpret_cast<const char*>(guid), size - sizeof(wchar_t));
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0) {
        throw std::runtime_error("SHA-256 indisponível.");
    }
    unsigned char digest[32] = {};
    const auto result = BCryptHash(algorithm, nullptr, 0,
        reinterpret_cast<PUCHAR>(&input[0]), static_cast<ULONG>(input.size()), digest, sizeof(digest));
    BCryptCloseAlgorithmProvider(algorithm, 0);
    if (result < 0) throw std::runtime_error("Falha ao identificar o dispositivo.");
    const char hex[] = "0123456789abcdef";
    std::string id;
    for (const auto byte : digest) {
        id += hex[byte >> 4];
        id += hex[byte & 15];
    }
    return id;
}
}

namespace SecAuth {
Client::Client(const std::wstring& url, const std::string& appId, const std::string& desktopKey)
    : appId_(appId), desktopKey_(desktopKey) {
    if (desktopKey_.size() != 64 || desktopKey_.find_first_not_of("0123456789abcdef") != std::string::npos) {
        throw std::runtime_error("Chave do cliente C++ inválida.");
    }
    if (appId_.size() != 24 || appId_.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) {
        throw std::runtime_error("App ID inválido.");
    }

    URL_COMPONENTS parts = {};
    parts.dwStructSize = sizeof(parts);
    parts.dwHostNameLength = parts.dwUrlPathLength = parts.dwExtraInfoLength =
        parts.dwUserNameLength = parts.dwPasswordLength = static_cast<DWORD>(-1);
    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &parts) || parts.dwExtraInfoLength ||
        parts.dwUserNameLength || parts.dwPasswordLength || !parts.dwHostNameLength) {
        throw std::runtime_error("URL do servidor inválida.");
    }
    host_.assign(parts.lpszHostName, parts.dwHostNameLength);
    if (parts.dwUrlPathLength) basePath_.assign(parts.lpszUrlPath, parts.dwUrlPathLength);
    while (!basePath_.empty() && basePath_.back() == L'/') basePath_.pop_back();
    port_ = parts.nPort;
    tls_ = parts.nScheme == INTERNET_SCHEME_HTTPS;
    if (!tls_ && !(parts.nScheme == INTERNET_SCHEME_HTTP && host_ == L"127.0.0.1")) {
        throw std::runtime_error("Use HTTPS para acessar um servidor remoto.");
    }
    device_ = DeviceId(appId_);
}

std::string Client::Request(const std::wstring& endpoint, const std::string& body) const {
    InternetHandle session(WinHttpOpen(L"SecAuth/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!WinHttpSetTimeouts(session.value, 4000, 4000, 5000, 5000)) {
        throw std::runtime_error("Falha ao configurar os tempos de conexão.");
    }
    if (tls_) {
        DWORD protocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        if (!WinHttpSetOption(session.value, WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof(protocols))) {
            throw std::runtime_error("TLS 1.2 indisponível.");
        }
    }
    InternetHandle connection(WinHttpConnect(session.value, host_.c_str(), port_, 0));
    const auto path = basePath_ + L"/api/desktop/" + endpoint;
    InternetHandle request(WinHttpOpenRequest(connection.value, L"POST", path.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, tls_ ? WINHTTP_FLAG_SECURE : 0));

    DWORD redirect = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
    if (!WinHttpSetOption(request.value, WINHTTP_OPTION_REDIRECT_POLICY, &redirect, sizeof(redirect))) {
        throw std::runtime_error("Falha ao configurar a conexão.");
    }
    std::wstring headers = L"Content-Type: application/json\r\n";
    headers += L"X-Desktop-Key: " + Wide(desktopKey_) + L"\r\n";
    if (endpoint == L"session") headers += L"Authorization: Bearer " + Wide(token_) + L"\r\n";
    const DWORD length = static_cast<DWORD>(body.size());
    if (!WinHttpSendRequest(request.value, headers.c_str(), static_cast<DWORD>(headers.size()),
                            const_cast<char*>(body.data()), length, length, 0) ||
        !WinHttpReceiveResponse(request.value, nullptr)) {
        throw std::runtime_error("Falha na conexão com o servidor.");
    }

    DWORD status = 0, statusSize = sizeof(status);
    if (!WinHttpQueryHeaders(request.value, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX)) {
        throw std::runtime_error("Resposta HTTP inválida.");
    }
    std::string response;
    char buffer[4096];
    for (;;) {
        DWORD received = 0;
        if (!WinHttpReadData(request.value, buffer, sizeof(buffer), &received)) {
            throw std::runtime_error("Falha ao ler a resposta do servidor.");
        }
        if (!received) break;
        if (response.size() + received > 65536) throw std::runtime_error("Resposta muito grande.");
        response.append(buffer, received);
    }
    const Json result = Json::parse(response, nullptr, false);
    if (!result.is_object()) throw std::runtime_error("Resposta inválida do servidor.");
    if (status != 200 || !result.value("success", false)) {
        throw std::runtime_error(result.value("message", std::string("Acesso recusado pelo servidor.")));
    }
    return response;
}

void Client::Register(const std::string& username, const std::string& email,
                      const std::string& password, const std::string& license) {
    Request(L"register", Json{{"appId", appId_}, {"device", device_}, {"username", username},
        {"email", email}, {"password", password}, {"license", license}}.dump());
}

Profile Client::Authorize(const std::wstring& endpoint, const std::string& body) {
    try {
        const Json result = Json::parse(Request(endpoint, body));
        const auto& profile = result.at("profile");
        Profile parsed;
        parsed.username = profile.at("username").get<std::string>();
        parsed.activatedAt = profile.at("activatedAt").get<std::string>();
        parsed.expiresAt = profile.at("expiresAt").get<std::string>();
        parsed.remainingSeconds = profile.at("remainingSeconds").get<long long>();
        token_ = result.at("token").get<std::string>();
        return parsed;
    } catch (...) {
        Logout();
        throw;
    }
}

Profile Client::Login(const std::string& username, const std::string& password) {
    Logout();
    return Authorize(L"login", Json{{"appId", appId_}, {"device", device_},
        {"username", username}, {"password", password}}.dump());
}

Profile Client::Refresh() {
    if (token_.empty()) throw std::runtime_error("Faça login para continuar.");
    return Authorize(L"session", "{}");
}

void Client::Logout() {
    if (!token_.empty()) SecureZeroMemory(&token_[0], token_.size());
    token_.clear();
}
}
