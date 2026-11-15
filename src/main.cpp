#include "client.h"

#include <windows.h>
#include <conio.h>

#include <iostream>
#include <string>

#ifndef SECAUTH_SERVER
#define SECAUTH_SERVER L""
#endif
#ifndef SECAUTH_APP_ID
#define SECAUTH_APP_ID ""
#endif
#ifndef SECAUTH_CREDENTIAL
#define SECAUTH_CREDENTIAL ""
#endif

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

std::string Narrow(const std::wstring& value) {
    if (value.empty()) return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                                         nullptr, 0, nullptr, nullptr);
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                        &result[0], size, nullptr, nullptr);
    return result;
}

std::string Trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string Ask(const char* label, bool hidden = false) {
    std::cout << "  " << kDim << label << kReset << std::flush;

    if (!hidden) {
        std::string value;
        std::getline(std::cin, value);
        return Trim(value);
    }

    // Palavra-passe: lê tecla a tecla e mostra um asterisco por cada caractere.
    std::string value;
    for (;;) {
        const int key = _getch();
        if (key == -1) { // sem consola interativa: lê o resto da linha normalmente
            std::getline(std::cin, value);
            break;
        }
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
    std::cout << "\n"
              << "  " << kAccent << "[1]" << kReset << " Registar licença\n"
              << "  " << kAccent << "[2]" << kReset << " Iniciar sessão\n"
              << "  " << kAccent << "[3]" << kReset << " Estado da sessão\n"
              << "  " << kAccent << "[4]" << kReset << " Terminar sessão\n"
              << "  " << kAccent << "[0]" << kReset << " Sair\n";
}

void Run(SecAuth::Client& client) {
    bool loggedIn = false;
    for (;;) {
        Menu();
        const std::string option = Ask("Opção: ");
        if (!std::cin) return;
        std::cout << "\n";

        try {
            if (option == "0") {
                client.Logout();
                return;
            }

            if (option == "1") {
                const std::string username = Ask("Usuário      : ");
                const std::string email = Ask("E-mail       : ");
                const std::string password = Ask("Palavra-passe: ", true);
                const std::string license = Ask("Licença      : ");
                if (username.empty() || email.empty() || password.empty() || license.empty()) {
                    std::cout << kBad << "\n  Preencha todos os campos.\n" << kReset;
                } else {
                    client.Register(username, email, password, license);
                    std::cout << kGood << "\n  Conta criada. Use a opção 2 para iniciar sessão.\n" << kReset;
                }
            } else if (option == "2") {
                const std::string username = Ask("Usuário      : ");
                const std::string password = Ask("Palavra-passe: ", true);
                if (username.empty() || password.empty()) {
                    std::cout << kBad << "\n  Preencha o usuário e a palavra-passe.\n" << kReset;
                } else {
                    loggedIn = true;
                    Show(client.Login(username, password));
                }
            } else if (option == "3") {
                if (!loggedIn) {
                    std::cout << kBad << "\n  Inicie sessão primeiro.\n" << kReset;
                } else {
                    Show(client.Refresh());
                }
            } else if (option == "4") {
                client.Logout();
                loggedIn = false;
                std::cout << kGood << "\n  Sessão terminada.\n" << kReset;
            } else {
                std::cout << kBad << "\n  Opção inválida.\n" << kReset;
            }
        } catch (const std::exception& error) {
            loggedIn = false;
            std::cout << kBad << "\n  " << error.what() << "\n" << kReset;
        }

        if (!std::cin) return;
        Pause();
        if (!std::cin) return;
    }
}

}

int main() {
    PrepareConsole();

    std::cout << "\n" << kAccent << kBold
              << "  SECAUTH" << kReset << kDim
              << "   Cliente de exemplo (C++)\n" << kReset
              << kDim << "  " << Narrow(SECAUTH_SERVER) << kReset << "\n";

    try {
        SecAuth::Client client(SECAUTH_SERVER, SECAUTH_APP_ID, SECAUTH_CREDENTIAL);
        Run(client);
    } catch (const std::exception& error) {
        std::cout << kBad << "\n  " << error.what() << "\n" << kReset;
        return 1;
    }

    std::cout << "\n" << kDim << "  Até breve.\n" << kReset << "\n";
    return 0;
}
