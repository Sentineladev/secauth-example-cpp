#pragma once

#if __has_include("config.local.h")
#include "config.local.h"
#else
namespace ClientSettings {
constexpr char Server[] = "https://api.getsecauth.com";
constexpr char AppId[] = "SEU_APP_ID";
constexpr char SecretKey[] = "SUA_SECRET_KEY";
}
#endif
