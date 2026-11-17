#include "client.h"

#include <windows.h>
#include <winhttp.h>
#include <bcrypt.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

// JSON mínimo, escrito neste projeto (sem dependências nem licenças externas).
// Cobre apenas o subconjunto usado pelo cliente: objetos com valores de texto,
// número, booleano e objetos aninhados. O servidor não devolve arrays.
class Json {
public:
    enum class Kind { Null, Boolean, Number, String, Object };

    static Json Fields(std::initializer_list<std::pair<const char*, std::string>> fields) {
        Json json;
        json.kind_ = Kind::Object;
        for (const auto& field : fields) {
            Json member;
            member.kind_ = Kind::String;
            member.name_ = field.first;
            member.text_ = field.second;
            json.members_.push_back(std::move(member));
        }
        return json;
    }

    bool IsObject() const { return kind_ == Kind::Object; }

    std::string Dump() const {
        switch (kind_) {
            case Kind::Boolean: return boolean_ ? "true" : "false";
            case Kind::Number: return std::to_string(number_);
            case Kind::String: return Quote(text_);
            case Kind::Object: {
                std::string out = "{";
                for (std::size_t index = 0; index < members_.size(); ++index) {
                    if (index) out += ',';
                    out += Quote(members_[index].name_) + ":" + members_[index].Dump();
                }
                return out + "}";
            }
            default: return "null";
        }
    }

    const Json* Child(const std::string& name) const {
        for (const Json& member : members_) {
            if (member.name_ == name) return &member;
        }
        return nullptr;
    }

    std::string Text(const std::string& name, const std::string& fallback) const {
        const Json* member = Child(name);
        return member && member->kind_ == Kind::String ? member->text_ : fallback;
    }

    bool Boolean(const std::string& name, bool fallback) const {
        const Json* member = Child(name);
        return member && member->kind_ == Kind::Boolean ? member->boolean_ : fallback;
    }

    long long Number(const std::string& name, long long fallback) const {
        const Json* member = Child(name);
        return member && member->kind_ == Kind::Number
            ? static_cast<long long>(member->number_) : fallback;
    }

    // Devolve o valor analisado e define "valid" como falso se o texto não for
    // um documento JSON completo e bem formado.
    static Json Parse(const std::string& input, bool& valid) {
        std::size_t position = 0;
        bool failed = false;
        Json result = Read(input, position, failed);
        SkipSpace(input, position);
        valid = !failed && position == input.size();
        return valid ? result : Json();
    }

private:
    Kind kind_ = Kind::Null;
    bool boolean_ = false;
    double number_ = 0;
    std::string name_;
    std::string text_;
    std::vector<Json> members_;

    static std::string Quote(const std::string& value) {
        std::string out = "\"";
        for (const char raw : value) {
            const unsigned char character = static_cast<unsigned char>(raw);
            switch (character) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\b': out += "\\b"; break;
                case '\f': out += "\\f"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:
                    if (character < 0x20) {
                        char escape[8] = {};
                        std::snprintf(escape, sizeof(escape), "\\u%04x", character);
                        out += escape;
                    } else {
                        out += raw;
                    }
            }
        }
        return out + "\"";
    }

    static void SkipSpace(const std::string& text, std::size_t& position) {
        while (position < text.size() && (text[position] == ' ' || text[position] == '\t' ||
               text[position] == '\n' || text[position] == '\r')) {
            ++position;
        }
    }

    static bool Word(const std::string& text, std::size_t& position, const char* literal) {
        const std::size_t length = std::strlen(literal);
        if (text.compare(position, length, literal) != 0) return false;
        position += length;
        return true;
    }

    static void AppendUtf8(std::string& out, unsigned code) {
        if (code < 0x80) {
            out += static_cast<char>(code);
        } else if (code < 0x800) {
            out += static_cast<char>(0xC0 | (code >> 6));
            out += static_cast<char>(0x80 | (code & 0x3F));
        } else if (code < 0x10000) {
            out += static_cast<char>(0xE0 | (code >> 12));
            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (code & 0x3F));
        } else {
            out += static_cast<char>(0xF0 | (code >> 18));
            out += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (code & 0x3F));
        }
    }

    static unsigned Hex4(const std::string& text, std::size_t& position, bool& failed) {
        if (position + 4 > text.size()) { failed = true; return 0; }
        unsigned code = 0;
        for (int index = 0; index < 4; ++index) {
            const char digit = text[position++];
            code <<= 4;
            if (digit >= '0' && digit <= '9') code |= digit - '0';
            else if (digit >= 'a' && digit <= 'f') code |= digit - 'a' + 10;
            else if (digit >= 'A' && digit <= 'F') code |= digit - 'A' + 10;
            else { failed = true; return 0; }
        }
        return code;
    }

    static std::string ReadString(const std::string& text, std::size_t& position, bool& failed) {
        if (position >= text.size() || text[position] != '"') { failed = true; return {}; }
        ++position;
        std::string out;
        while (position < text.size()) {
            const char character = text[position++];
            if (character == '"') return out;
            if (character != '\\') { out += character; continue; }
            if (position >= text.size()) break;
            switch (text[position++]) {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;
                case 't': out += '\t'; break;
                case 'u': {
                    unsigned code = Hex4(text, position, failed);
                    if (failed) return {};
                    if (code >= 0xD800 && code <= 0xDBFF && position + 2 <= text.size() &&
                        text[position] == '\\' && text[position + 1] == 'u') {
                        position += 2;
                        const unsigned low = Hex4(text, position, failed);
                        if (failed) return {};
                        code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                    }
                    AppendUtf8(out, code);
                    break;
                }
                default: failed = true; return {};
            }
        }
        failed = true;
        return {};
    }

    static Json ReadNumber(const std::string& text, std::size_t& position, bool& failed) {
        const std::size_t start = position;
        if (position < text.size() && (text[position] == '-' || text[position] == '+')) ++position;
        while (position < text.size() && std::isdigit(static_cast<unsigned char>(text[position]))) ++position;
        if (position < text.size() && text[position] == '.') {
            ++position;
            while (position < text.size() && std::isdigit(static_cast<unsigned char>(text[position]))) ++position;
        }
        if (position < text.size() && (text[position] == 'e' || text[position] == 'E')) {
            ++position;
            if (position < text.size() && (text[position] == '-' || text[position] == '+')) ++position;
            while (position < text.size() && std::isdigit(static_cast<unsigned char>(text[position]))) ++position;
        }
        if (position == start) { failed = true; return Json(); }
        Json json;
        json.kind_ = Kind::Number;
        json.number_ = std::strtod(text.substr(start, position - start).c_str(), nullptr);
        return json;
    }

    static Json ReadObject(const std::string& text, std::size_t& position, bool& failed) {
        Json json;
        json.kind_ = Kind::Object;
        ++position; // consome '{'
        SkipSpace(text, position);
        if (position < text.size() && text[position] == '}') { ++position; return json; }
        for (;;) {
            SkipSpace(text, position);
            std::string name = ReadString(text, position, failed);
            if (failed) return Json();
            SkipSpace(text, position);
            if (position >= text.size() || text[position] != ':') { failed = true; return Json(); }
            ++position;
            Json value = Read(text, position, failed);
            if (failed) return Json();
            value.name_ = std::move(name);
            json.members_.push_back(std::move(value));
            SkipSpace(text, position);
            if (position >= text.size()) { failed = true; return Json(); }
            if (text[position] == ',') { ++position; continue; }
            if (text[position] == '}') { ++position; return json; }
            failed = true;
            return Json();
        }
    }

    static Json Read(const std::string& text, std::size_t& position, bool& failed) {
        if (failed) return Json();
        SkipSpace(text, position);
        if (position >= text.size()) { failed = true; return Json(); }
        switch (text[position]) {
            case '{': return ReadObject(text, position, failed);
            case '"':
            {
                Json json;
                json.kind_ = Kind::String;
                json.text_ = ReadString(text, position, failed);
                return json;
            }
            case 't':
                if (!Word(text, position, "true")) { failed = true; return Json(); }
                {
                    Json json;
                    json.kind_ = Kind::Boolean;
                    json.boolean_ = true;
                    return json;
                }
            case 'f':
                if (!Word(text, position, "false")) { failed = true; return Json(); }
                {
                    Json json;
                    json.kind_ = Kind::Boolean;
                    json.boolean_ = false;
                    return json;
                }
            case 'n':
                if (!Word(text, position, "null")) { failed = true; return Json(); }
                return Json();
            default: return ReadNumber(text, position, failed);
        }
    }
};

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
    bool valid = false;
    const Json result = Json::Parse(response, valid);
    if (!valid || !result.IsObject()) throw std::runtime_error("Resposta inválida do servidor.");
    if (status != 200 || !result.Boolean("success", false)) {
        throw std::runtime_error(result.Text("message", "Acesso recusado pelo servidor."));
    }
    return response;
}

void Client::Register(const std::string& username, const std::string& email,
                      const std::string& password, const std::string& license) {
    Request(L"register", Json::Fields({{"appId", appId_}, {"device", device_}, {"username", username},
        {"email", email}, {"password", password}, {"license", license}}).Dump());
}

Profile Client::Authorize(const std::wstring& endpoint, const std::string& body) {
    try {
        bool valid = false;
        const Json result = Json::Parse(Request(endpoint, body), valid);
        const Json* profile = valid ? result.Child("profile") : nullptr;
        if (!profile) throw std::runtime_error("Resposta inválida do servidor.");
        Profile parsed;
        parsed.username = profile->Text("username", "");
        parsed.activatedAt = profile->Text("activatedAt", "");
        parsed.expiresAt = profile->Text("expiresAt", "");
        parsed.remainingSeconds = profile->Number("remainingSeconds", 0);
        token_ = result.Text("token", "");
        return parsed;
    } catch (...) {
        Logout();
        throw;
    }
}

Profile Client::Login(const std::string& username, const std::string& password) {
    Logout();
    return Authorize(L"login", Json::Fields({{"appId", appId_}, {"device", device_},
        {"username", username}, {"password", password}}).Dump());
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
