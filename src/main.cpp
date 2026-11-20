#include "client.h"
#include "../config/config.h"
#include "../include/skStr.h"

#include <windows.h>
#include <conio.h>

#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr const char* kReset = "\x1b[0m";
constexpr const char* kBold = "\x1b[1m";
constexpr const char* kDim = "\x1b[90m";
constexpr const char* kAccent = "\x1b[38;5;75m";
constexpr const char* kGood = "\x1b[38;5;42m";
constexpr const char* kBad = "\x1b[38;5;203m";

void PrepareConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    const HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(output, &mode)) {
        SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

std::wstring Wide(const std::string& value) {
    if (value.empty()) return {};
    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                                         nullptr, 0);
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), &result[0], size);
    return result;
}

std::string Lower(std::string value) {
    for (char& character : value) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return value;
}

std::string Trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string Ask(const char* label, bool hidden = false) {
    std::cout << "  " << kDim << label << kReset << std::flush;

    // Sem consola interativa (entrada redirecionada): lê a linha normalmente.
    DWORD mode = 0;
    if (!hidden || !GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode)) {
        std::string value;
        std::getline(std::cin, value);
        return Trim(value);
    }

    // Palavra-passe: lê tecla a tecla e mostra um asterisco por cada caractere.
    std::string value;
    for (;;) {
        const int key = _getch();
        if (key == -1) break;
        if (key == '\r' || key == '\n') break;
        if (key == '\b') {
            if (!value.empty()) {
                value.pop_back();
                std::cout << "\b \b" << std::flush;
            }
        } else if (key == 0 || key == 0xE0) {
            _getch(); // descarta o segundo byte de teclas especiais (setas, F1-F12)
        } else {
            value.push_back(static_cast<char>(key));
            std::cout << '*' << std::flush;
        }
    }
    std::cout << "\n";
    return Trim(value);
}

void Clear() {
    std::cout << "\x1b[2J\x1b[H" << std::flush;
}

// Limpa o ecrã antes de mostrar um erro, para não acumular o histórico no terminal.
void Fail(const std::string& message) {
    Clear();
    std::cout << kBad << "\n  " << message << "\n" << kReset;
}

void Pause() {
    std::cout << kDim << "\n  Pressione Enter para voltar ao menu..." << kReset << std::flush;
    std::cin.get();
}

void Show(const SecAuth::Profile& profile) {
    const long long hours = profile.remainingSeconds / 3600;
    const long long minutes = (profile.remainingSeconds % 3600) / 60;
    std::cout << "\n" << kGood << "  Sessão ativa.\n" << kReset
              << "  " << kDim << "Usuário   " << kReset << "  " << profile.username << "\n"
              << "  " << kDim << "Ativada em" << kReset << "  " << profile.activatedAt << "\n"
              << "  " << kDim << "Expira em " << kReset << "  " << profile.expiresAt
              << kDim << "  (" << hours << "h " << minutes << "m)" << kReset << "\n";
}

void Menu() {
    std::cout << "\n" << kAccent << kBold << "  SECAUTH" << kReset << kDim
              << "   Cliente de exemplo (C++)\n" << kReset << "\n"
              << "  " << kAccent << "[1]" << kReset << " Registar licença\n"
              << "  " << kAccent << "[2]" << kReset << " Iniciar sessão\n"
              << "  " << kAccent << "[3]" << kReset << " Estado da sessão\n"
              << "  " << kAccent << "[4]" << kReset << " Terminar sessão\n"
              << "  " << kAccent << "[0]" << kReset << " Sair\n";
}

void Run(SecAuth::Client& client) {
    for (;;) {
        Clear();
        Menu();
        const std::string option = Ask("Opção: ");
        if (!std::cin) return;
        Clear();

        try {
            if (option == "0") {
                client.Logout();
                return;
            }

            if (option == "1") {
                std::cout << "\n  Registar licença\n\n";
                const std::string username = Ask("Usuário      : ");
                const std::string email = Ask("E-mail       : ");
                const std::string password = Ask("Palavra-passe: ", true);
                const std::string license = Ask("Licença      : ");
                if (username.empty() || email.empty() || password.empty() || license.empty()) {
                    Fail("Preencha todos os campos.");
                } else {
                    Clear();
                    client.Register(username, email, password, license);
                    std::cout << kGood << "\n  Conta criada. Use a opção 2 para iniciar sessão.\n" << kReset;
                }
            } else if (option == "2") {
                std::cout << "\n  Iniciar sessão\n\n";
                const std::string username = Ask("Usuário      : ");
                const std::string password = Ask("Palavra-passe: ", true);
                if (username.empty() || password.empty()) {
                    Fail("Preencha o usuário e a palavra-passe.");
                } else {
                    Clear();
                    const SecAuth::Profile profile = client.Login(username, password);
                    Show(profile);
                }
            } else if (option == "3") {
                if (!client.IsLoggedIn()) {
                    Fail("Inicie sessão primeiro.");
                } else {
                    Show(client.Refresh());
                }
            } else if (option == "4") {
                const bool serverConfirmed = client.Logout();
                if (serverConfirmed) {
                    std::cout << kGood << "\n  Sessão revogada no servidor.\n" << kReset;
                } else {
                    std::cout << kBad << "\n  Sessão local removida; servidor não confirmou o logout.\n" << kReset;
                }
            } else {
                Fail("Opção inválida.");
            }
        } catch (const std::exception& error) {
            Fail(error.what());
        }

        if (!std::cin) return;
        Pause();
        if (!std::cin) return;
    }
}

}

int main() {
    PrepareConsole();

    try {
        const std::string appId = Lower(ClientSettings::AppId);
        auto obfuscated = skCrypt(ClientSettings::DesktopKey);
        std::string desktopKey = obfuscated.decrypt();
        SecAuth::Client client(Wide(ClientSettings::Server), appId, desktopKey);
        SecureZeroMemory(&desktopKey[0], desktopKey.size());
        obfuscated.clear();
        Run(client);
    } catch (const std::exception& error) {
        Fail(error.what());
        return 1;
    }

    std::cout << "\n" << kDim << "  Até breve.\n" << kReset << "\n";
    return 0;
}
