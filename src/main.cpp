#include "client.h"
#include "skStr.h"
#include <windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <cmath>
#include <future>
#include <memory>
#include <chrono>
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include "addons/ImCustomColors.h"

#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace {
LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam)) return 1;
    if (message == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcW(window, message, wParam, lParam);
}

std::unique_ptr<SecAuth::Client> CreateClient() {
    auto server = skCrypt(SECAUTH_SERVER);
    auto appId = skCrypt(SECAUTH_APP_ID);
    auto key = skCrypt(SECAUTH_CREDENTIAL);
    auto client = std::unique_ptr<SecAuth::Client>(
        new SecAuth::Client(server.decrypt(), appId.decrypt(), key.decrypt()));
    server.clear();
    appId.clear();
    key.clear();
    return client;
}

using Clock = std::chrono::steady_clock;
enum class Screen { Login, Register, Main };

// ============================================================
// TEMA — mesma identidade visual do site SecAuth
// ============================================================
constexpr float kContentWidth = 380.0f;

void ApplyTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(24, 20);
    style.FramePadding = ImVec2(12, 9);
    style.ItemSpacing = ImVec2(10, 10);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.ScrollbarSize = 10.0f;
    style.GrabMinSize = 10.0f;
    style.WindowRounding = 0.0f;
    style.ChildRounding = 14.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImCustomColors::UI::Text();
    colors[ImGuiCol_TextDisabled] = ImCustomColors::UI::TextDisabled();
    colors[ImGuiCol_WindowBg] = ImCustomColors::UI::WindowBgSolid();
    colors[ImGuiCol_ChildBg] = ImCustomColors::UI::ChildBg();
    colors[ImGuiCol_PopupBg] = ImCustomColors::UI::PopupBg();
    colors[ImGuiCol_Border] = ImCustomColors::UI::Border();
    colors[ImGuiCol_BorderShadow] = ImCustomColors::UI::BorderShadow();
    colors[ImGuiCol_FrameBg] = ImCustomColors::UI::FrameBg();
    colors[ImGuiCol_FrameBgHovered] = ImCustomColors::UI::FrameBgHovered();
    colors[ImGuiCol_FrameBgActive] = ImCustomColors::UI::FrameBgActive();
    colors[ImGuiCol_TitleBg] = ImCustomColors::UI::MenuBarBg();
    colors[ImGuiCol_TitleBgActive] = ImCustomColors::UI::MenuBarBg();
    colors[ImGuiCol_TitleBgCollapsed] = ImCustomColors::UI::MenuBarBg();
    colors[ImGuiCol_MenuBarBg] = ImCustomColors::UI::MenuBarBg();
    colors[ImGuiCol_ScrollbarBg] = ImCustomColors::UI::ScrollbarBg();
    colors[ImGuiCol_ScrollbarGrab] = ImCustomColors::UI::ScrollbarGrab();
    colors[ImGuiCol_ScrollbarGrabHovered] = ImCustomColors::UI::ScrollbarGrabHovered();
    colors[ImGuiCol_ScrollbarGrabActive] = ImCustomColors::UI::ScrollbarGrabActive();
    colors[ImGuiCol_CheckMark] = ImCustomColors::Checkbox::CheckMark();
    colors[ImGuiCol_SliderGrab] = ImCustomColors::UI::SliderGrab();
    colors[ImGuiCol_SliderGrabActive] = ImCustomColors::UI::SliderGrabActive();
    colors[ImGuiCol_Button] = ImCustomColors::Gradient::GetPrimary();
    colors[ImGuiCol_ButtonHovered] = ImCustomColors::Gradient::GetSecondary();
    colors[ImGuiCol_ButtonActive] = ImCustomColors::Gradient::GetDeep();
    colors[ImGuiCol_Header] = ImCustomColors::UI::Header();
    colors[ImGuiCol_HeaderHovered] = ImCustomColors::UI::HeaderHovered();
    colors[ImGuiCol_HeaderActive] = ImCustomColors::UI::HeaderActive();
    colors[ImGuiCol_Separator] = ImCustomColors::UI::Separator();
    colors[ImGuiCol_SeparatorHovered] = ImCustomColors::UI::SeparatorHovered();
    colors[ImGuiCol_SeparatorActive] = ImCustomColors::UI::SeparatorActive();
    colors[ImGuiCol_ResizeGrip] = ImCustomColors::UI::Button();
    colors[ImGuiCol_ResizeGripHovered] = ImCustomColors::UI::ButtonHovered();
    colors[ImGuiCol_ResizeGripActive] = ImCustomColors::UI::ButtonActive();
    colors[ImGuiCol_Tab] = ImCustomColors::UI::Tab();
    colors[ImGuiCol_TabHovered] = ImCustomColors::UI::TabHovered();
    colors[ImGuiCol_TabSelected] = ImCustomColors::UI::TabActive();
    colors[ImGuiCol_TabSelectedOverline] = ImCustomColors::Gradient::GetPrimary();
    colors[ImGuiCol_TabDimmed] = ImCustomColors::UI::TabUnfocused();
    colors[ImGuiCol_TabDimmedSelected] = ImCustomColors::UI::TabUnfocusedActive();
    colors[ImGuiCol_TextSelectedBg] = ImCustomColors::Accent::TextSelectedBg();
    colors[ImGuiCol_NavCursor] = ImCustomColors::Accent::NavHighlight();
    colors[ImGuiCol_NavWindowingDimBg] = ImCustomColors::UI::NavWindowingDimBg();
    colors[ImGuiCol_ModalWindowDimBg] = ImCustomColors::UI::ModalWindowDimBg();
}

// Carrega uma fonte do sistema parecida com a do site (Inter). Sem dependências
// externas: usa apenas fontes presentes no Windows.
void LoadFonts() {
    ImGuiIO& io = ImGui::GetIO();
    const char* candidates[] = {
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\tahoma.ttf",
        "C:\\Windows\\Fonts\\verdana.ttf",
    };
    for (const char* path : candidates) {
        if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) continue;
        if (io.Fonts->AddFontFromFileTTF(path, 16.0f) != nullptr) return;
    }
    io.Fonts->AddFontDefault();
}

// Glow azul no topo + grade, como o "ambient-glow" e o "grid-overlay" do site.
void DrawBackdrop(ImDrawList* draw, const ImVec2& size) {
    const ImU32 grid = IM_COL32(255, 255, 255, 10);
    for (float x = 0.0f; x < size.x; x += 50.0f) draw->AddLine(ImVec2(x, 0), ImVec2(x, size.y), grid);
    for (float y = 0.0f; y < size.y; y += 50.0f) draw->AddLine(ImVec2(0, y), ImVec2(size.x, y), grid);
    const float glow = size.y * 0.55f;
    draw->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(size.x, glow),
        IM_COL32(59, 130, 246, 48), IM_COL32(59, 130, 246, 48),
        IM_COL32(59, 130, 246, 0), IM_COL32(59, 130, 246, 0));
}

// Gradiente vertical com cantos arredondados (o ImGui 1.92 só tem a versão reta).
void AddRoundedGradient(ImDrawList* draw, const ImVec2& p0, const ImVec2& p1,
                        const ImVec4& top, const ImVec4& bottom, float rounding) {
    const float width = p1.x - p0.x;
    const float height = p1.y - p0.y;
    const float radius = ImMin(rounding, ImMin(width, height) * 0.5f);
    const int bands = ImMax(1, static_cast<int>(height * 0.5f));
    for (int i = 0; i < bands; ++i) {
        const float t0 = static_cast<float>(i) / bands;
        const float t1 = static_cast<float>(i + 1) / bands;
        const float y0 = p0.y + height * t0;
        const float y1 = p0.y + height * t1 + 0.5f;
        float inset = 0.0f;
        const float samples[2] = {y0, y1};
        for (float y : samples) {
            const float dy = y < p0.y + radius ? (p0.y + radius - y)
                           : y > p1.y - radius ? (y - (p1.y - radius)) : 0.0f;
            if (dy > 0.0f) inset = ImMax(inset, radius - sqrtf(ImMax(0.0f, radius * radius - dy * dy)));
        }
        draw->AddRectFilled(ImVec2(p0.x + inset, y0), ImVec2(p1.x - inset, y1),
            ImGui::GetColorU32(ImLerp(top, bottom, t0)));
    }
}

// Botão principal com o gradiente azul do site (btn-glow).
bool AccentButton(const char* label, const ImVec2& size, bool enabled = true) {
    const ImGuiStyle& style = ImGui::GetStyle();
    const float height = size.y > 0.0f ? size.y : ImGui::GetFrameHeight();
    const float width = size.x > 0.0f ? size.x : ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::BeginDisabled(!enabled);
    const bool clicked = ImGui::InvisibleButton(label, ImVec2(width, height));
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    ImGui::EndDisabled();

    const float lift = held ? 0.82f : hovered ? 1.10f : 1.0f;
    const float dim = enabled ? 1.0f : 0.45f;
    ImVec4 top = ImCustomColors::Gradient::GetPrimary();
    ImVec4 bottom = ImCustomColors::Gradient::GetDeep();
    top = ImVec4(ImMin(1.0f, top.x * lift), ImMin(1.0f, top.y * lift), ImMin(1.0f, top.z * lift), dim);
    bottom = ImVec4(ImMin(1.0f, bottom.x * lift), ImMin(1.0f, bottom.y * lift), ImMin(1.0f, bottom.z * lift), dim);

    const ImVec2 max(pos.x + width, pos.y + height);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(ImVec2(pos.x + 1, pos.y + 3), ImVec2(max.x - 1, max.y + 5),
        IM_COL32(59, 130, 246, 45), style.FrameRounding + 2.0f);
    AddRoundedGradient(draw, pos, max, top, bottom, style.FrameRounding);
    draw->AddRect(pos, max, ImGui::GetColorU32(ImVec4(1, 1, 1, (hovered ? 0.30f : 0.16f) * dim)), style.FrameRounding);
    const ImVec2 text = ImGui::CalcTextSize(label);
    draw->AddText(ImVec2(pos.x + (width - text.x) * 0.5f, pos.y + (height - text.y) * 0.5f),
        ImGui::GetColorU32(ImVec4(1, 1, 1, dim)), label);
    return clicked;
}

// Botão secundário discreto (borda clara + fundo translúcido no hover).
bool GhostButton(const char* label, const ImVec2& size, bool enabled = true) {
    const ImGuiStyle& style = ImGui::GetStyle();
    const float height = size.y > 0.0f ? size.y : ImGui::GetFrameHeight();
    const float width = size.x > 0.0f ? size.x : ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::BeginDisabled(!enabled);
    const bool clicked = ImGui::InvisibleButton(label, ImVec2(width, height));
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    ImGui::EndDisabled();

    const ImVec2 max(pos.x + width, pos.y + height);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const float alpha = enabled ? 1.0f : 0.45f;
    const float fill = (held ? 0.10f : hovered ? 0.06f : 0.02f) * alpha;
    draw->AddRectFilled(pos, max, ImGui::GetColorU32(ImVec4(1, 1, 1, fill)), style.FrameRounding);
    draw->AddRect(pos, max, ImGui::GetColorU32(ImVec4(1, 1, 1, (hovered ? 0.22f : 0.12f) * alpha)), style.FrameRounding);
    const ImVec2 text = ImGui::CalcTextSize(label);
    const ImVec4 textColor = hovered ? ImCustomColors::UI::Text() : ImCustomColors::UI::TextSecondary();
    draw->AddText(ImVec2(pos.x + (width - text.x) * 0.5f, pos.y + (height - text.y) * 0.5f),
        ImGui::GetColorU32(ImVec4(textColor.x, textColor.y, textColor.z, alpha)), label);
    return clicked;
}

// Campo de texto com o fundo/borda do site.
void Field(const char* id, const char* hint, char* value, size_t size, ImGuiInputTextFlags flags = 0) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImCustomColors::UI::Border());
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint(id, hint, value, size, flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

// Cabeçalho com a marca SECAUTH (SEC branco + AUTH azul), como no site.
void DrawHeader() {
    ImGui::PushFont(nullptr, 30.0f);
    ImGui::TextUnformatted("SEC");
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::TextColored(ImCustomColors::Gradient::GetPrimary(), "AUTH");
    ImGui::PopFont();
    ImGui::TextColored(ImCustomColors::UI::TextSecondary(), "Enterprise");
    ImGui::Dummy(ImVec2(0, 4));
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilledMultiColor(pos, ImVec2(pos.x + width, pos.y + 2.0f),
        IM_COL32(59, 130, 246, 200), IM_COL32(37, 99, 235, 60),
        IM_COL32(37, 99, 235, 60), IM_COL32(59, 130, 246, 200));
    ImGui::Dummy(ImVec2(0, 10));
}

void InfoRow(const char* label, const char* value) {
    ImGui::TextColored(ImCustomColors::UI::TextSecondary(), "%s", label);
    ImGui::SameLine();
    const float available = ImGui::GetContentRegionMax().x - ImGui::GetCursorPosX();
    const float textWidth = ImGui::CalcTextSize(value).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (available - textWidth));
    ImGui::TextColored(ImCustomColors::UI::Text(), "%s", value);
}

class Panel {
public:
    Panel() {
        try { client_ = CreateClient(); }
        catch (const std::exception& error) { message_ = error.what(); }
    }

    void Draw() {
        CompleteRequest();
        if (screen_ == Screen::Main && !pending_.valid() && Clock::now() >= refreshAt_) {
            BeginRequest(Screen::Main, [this] { return client_->Refresh(); });
        }

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("SecAuth", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        DrawBackdrop(ImGui::GetWindowDrawList(), ImGui::GetIO().DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, fade_);
        fade_ = ImMin(1.0f, fade_ + ImGui::GetIO().DeltaTime * 7.0f);

        DrawHeader();

        const float width = ImGui::GetContentRegionAvail().x;
        const float card = ImMin(kContentWidth, width);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (width - card) * 0.5f);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImCustomColors::UI::CardBg());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(22, 20));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 14.0f);
        ImGui::BeginChild("##card", ImVec2(card, 0.0f),
            ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

        if (screen_ == Screen::Main) DrawSession();
        else DrawCredentials();

        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();

        if (pending_.valid() || !message_.empty()) {
            ImGui::Dummy(ImVec2(0, 12));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (width - card) * 0.5f);
            const float wrap = ImGui::GetCursorPosX() + card;
            ImGui::PushTextWrapPos(wrap);
            if (pending_.valid()) ImGui::TextColored(ImCustomColors::UI::TextSecondary(), "Conectando...");
            if (!message_.empty()) {
                const ImVec4 color = pending_.valid() ? ImCustomColors::UI::TextSecondary()
                                                      : ImCustomColors::Feedback::Error();
                ImGui::TextColored(color, "%s", message_.c_str());
            }
            ImGui::PopTextWrapPos();
        }

        ImGui::PopStyleVar();
        ImGui::End();
    }

private:
    void SetScreen(Screen screen) {
        screen_ = screen;
        fade_ = 0.25f;
        message_.clear();
        SecureZeroMemory(password_, sizeof(password_));
        SecureZeroMemory(license_, sizeof(license_));
    }

    template <typename Operation>
    void BeginRequest(Screen destination, Operation operation) {
        destination_ = destination;
        message_.clear();
        pending_ = std::async(std::launch::async, operation);
    }

    void CompleteRequest() {
        if (!pending_.valid() || pending_.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
        try {
            profile_ = pending_.get();
            const bool registered = screen_ == Screen::Register;
            if (screen_ != destination_) SetScreen(destination_);
            if (registered) message_ = "Cadastro concluído. Faça login.";
            profileAt_ = Clock::now();
            refreshAt_ = profileAt_ + std::chrono::seconds((std::min)(60LL, profile_.remainingSeconds));
        } catch (const std::exception& error) {
            if (screen_ == Screen::Main) {
                profile_ = {};
                SetScreen(Screen::Login);
            }
            message_ = error.what();
        }
    }

    void DrawSession() {
        ImGui::TextColored(ImCustomColors::Feedback::Success(), "Logado com sucesso");
        ImGui::Dummy(ImVec2(0, 6));
        InfoRow("Usuário", profile_.username.c_str());
        InfoRow("Ativação (UTC)", profile_.activatedAt.c_str());
        const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - profileAt_).count();
        const auto remaining = (std::max)(0LL, profile_.remainingSeconds - elapsed);
        char validity[64];
        ImFormatString(validity, sizeof(validity), "%lld dias, %02lld:%02lld:%02lld",
            remaining / 86400, remaining / 3600 % 24, remaining / 60 % 60, remaining % 60);
        InfoRow("Validade restante", validity);
        ImGui::Dummy(ImVec2(0, 12));
        if (GhostButton("Sair", ImVec2(-1, 38), !pending_.valid())) {
            client_->Logout();
            profile_ = {};
            SetScreen(Screen::Login);
        }
    }

    void DrawCredentials() {
        const bool registering = screen_ == Screen::Register;
        ImGui::TextColored(ImCustomColors::UI::Text(), registering ? "Criar conta" : "Entrar");
        ImGui::TextColored(ImCustomColors::UI::TextSecondary(),
            registering ? "Preencha os dados e a chave de licença." : "Informe suas credenciais de acesso.");
        ImGui::Dummy(ImVec2(0, 8));

        const bool enabled = !pending_.valid() && client_ != nullptr;
        ImGui::BeginDisabled(!enabled);
        Field("##username", "Nome de usuário", username_, sizeof(username_));
        if (registering) Field("##email", "E-mail", email_, sizeof(email_));
        Field("##password", "Senha", password_, sizeof(password_), ImGuiInputTextFlags_Password);
        if (registering) Field("##license", "Chave de licença", license_, sizeof(license_));
        ImGui::Dummy(ImVec2(0, 10));

        if (AccentButton(registering ? "Cadastrar" : "Entrar", ImVec2(-1, 40), enabled)) {
            const std::string username(username_), email(email_), password(password_), license(license_);
            SecureZeroMemory(password_, sizeof(password_));
            if (registering) {
                SecureZeroMemory(license_, sizeof(license_));
                BeginRequest(Screen::Login, [this, username, email, password, license] {
                    client_->Register(username, email, password, license);
                    return SecAuth::Profile{};
                });
            } else {
                BeginRequest(Screen::Main, [this, username, password] { return client_->Login(username, password); });
            }
        }
        ImGui::Dummy(ImVec2(0, 4));
        if (GhostButton(registering ? "Voltar para o login" : "Criar conta", ImVec2(-1, 34), enabled)) {
            SetScreen(registering ? Screen::Login : Screen::Register);
        }
        ImGui::EndDisabled();
    }

    std::unique_ptr<SecAuth::Client> client_;
    std::future<SecAuth::Profile> pending_;
    SecAuth::Profile profile_;
    Screen screen_ = Screen::Login;
    Screen destination_ = Screen::Login;
    Clock::time_point refreshAt_, profileAt_;
    std::string message_;
    float fade_ = 1.0f;
    char username_[65] = {}, email_[255] = {}, password_[73] = {}, license_[129] = {};
};
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    WNDCLASSW wc = {};
    wc.hInstance = instance;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = L"SecAuthExample";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(4, 5, 6));
    if (!RegisterClassW(&wc)) return 1;
    HWND window = CreateWindowW(wc.lpszClassName, L"SecAuth Exemplo",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 480, 520, nullptr, nullptr, instance, nullptr);
    if (!window) return 1;

    // Barra de título escura, para combinar com o tema do site.
    BOOL darkTitle = TRUE;
    if (FAILED(DwmSetWindowAttribute(window, 20, &darkTitle, sizeof(darkTitle))))
        DwmSetWindowAttribute(window, 19, &darkTitle, sizeof(darkTitle));

    DXGI_SWAP_CHAIN_DESC swap = {};
    swap.BufferCount = 2;
    swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap.OutputWindow = window;
    swap.SampleDesc.Count = 1;
    swap.Windowed = TRUE;
    IDXGISwapChain* chain = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &swap, &chain, &device, nullptr, &context);
    if (FAILED(result)) result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &swap, &chain, &device, nullptr, &context);
    if (FAILED(result)) {
        MessageBoxW(window, L"Não foi possível iniciar o Direct3D 11.", L"SecAuth", MB_ICONERROR);
        return 1;
    }
    ID3D11Texture2D* buffer = nullptr;
    ID3D11RenderTargetView* target = nullptr;
    result = chain->GetBuffer(0, IID_PPV_ARGS(&buffer));
    if (SUCCEEDED(result)) {
        result = device->CreateRenderTargetView(buffer, nullptr, &target);
        buffer->Release();
    }
    if (FAILED(result)) {
        context->Release(); device->Release(); chain->Release();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    LoadFonts();
    ApplyTheme();
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, context);
    ShowWindow(window, show);

    Panel panel;
    bool quit = false;
    while (!quit) {
        MSG message;
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
            if (message.message == WM_QUIT) quit = true;
        }
        if (quit) break;
        if (IsIconic(window)) { Sleep(50); continue; }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        panel.Draw();
        ImGui::Render();
        const float background[4] = {0.016f, 0.019f, 0.024f, 1.0f};
        context->OMSetRenderTargets(1, &target, nullptr);
        context->ClearRenderTargetView(target, background);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        chain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    target->Release(); context->Release(); device->Release(); chain->Release();
    return 0;
}
