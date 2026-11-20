#pragma once

#include <string>
#include "../include/skStr.h"

namespace ClientSettings {
inline std::string Server() {
    auto encrypted = skCrypt("https://api.getsecauth.com");
    std::string result(encrypted.decrypt());
    encrypted.clear();
    return result;
}

inline std::string AppId() {
    auto encrypted = skCrypt("SEU_APP_ID");
    std::string result(encrypted.decrypt());
    encrypted.clear();
    return result;
}

inline std::string AppName() {
    auto encrypted = skCrypt("MeuApp");
    std::string result(encrypted.decrypt());
    encrypted.clear();
    return result;
}

inline std::string Version() {
    auto encrypted = skCrypt("1.0.0");
    std::string result(encrypted.decrypt());
    encrypted.clear();
    return result;
}

inline std::string ClientKey() {
    auto encrypted = skCrypt("COLE_SUA_CLIENT_KEY");
    std::string result(encrypted.decrypt());
    encrypted.clear();
    return result;
}

}
