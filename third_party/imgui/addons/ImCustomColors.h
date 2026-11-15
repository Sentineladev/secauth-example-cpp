#pragma once
#include <imgui.h>
#include "ImAdd.h"

namespace ImCustomColors {
    // ============================================
    // IDENTIDADE SECAUTH - AZUL MARINHO
    // ============================================
    // Paleta derivada do site (front/src/index.css):
    // fundo hsl(220,20%,2%), card hsl(225,20%,5%), navy hsl(222,47%,11%)
    // e acento azul hsl(217,91%,60%).
    namespace RGB {
        constexpr int NAVY_DEEP     = 0x070F24; // Fundo mais escuro (base do site)
        constexpr int NAVY_DARK     = 0x0D1B3E; // Azul-marinho profundo
        constexpr int NAVY          = 0x1A3673; // Azul-marinho (rodapé de botão)
        constexpr int BLUE_PRIMARY  = 0x2B52A1; // Azul institucional
        constexpr int BLUE_BRIGHT   = 0x3B82F6; // Azul de destaque hsl(217,91%,60%)
        constexpr int BLUE_GLOW     = 0x60A5FA; // Azul claro (brilho/hover)
        constexpr int BLUE_PALE     = 0xBFDBFE; // Azul quase branco
    }

    // ============================================
    // CORES DE GRADIENTE SECAUTH
    // ============================================
    namespace Gradient {
        inline ImVec4 GetPrimary() {
            return ImAdd::Hex2RGBA(RGB::BLUE_BRIGHT, 1.0f); // Azul de destaque
        }

        inline ImVec4 GetSecondary() {
            return ImAdd::Hex2RGBA(RGB::BLUE_GLOW, 1.0f); // Azul claro (hover)
        }

        inline ImVec4 GetDeep() {
            return ImAdd::Hex2RGBA(RGB::NAVY, 1.0f); // Azul-marinho (pressionado)
        }
    }

    // ============================================
    // CORES DE UI - TEMA AZUL MARINHO
    // ============================================
    namespace UI {
        // Fundos (azul-marinho quase preto, como o site)
        inline ImVec4 WindowBg() { return ImVec4(0.0f, 0.0f, 0.0f, 0.0f); }
        inline ImVec4 ChildBg() { return ImVec4(0.0f, 0.0f, 0.0f, 0.0f); }
        inline ImVec4 PopupBg() { return ImVec4(0.040f, 0.045f, 0.060f, 0.98f); }
        inline ImVec4 MenuBarBg() { return ImVec4(0.027f, 0.059f, 0.141f, 0.98f); }
        inline ImVec4 WindowBgSolid() { return ImVec4(0.016f, 0.019f, 0.024f, 1.00f); }
        inline ImVec4 ChildBgSolid() { return ImVec4(0.040f, 0.045f, 0.060f, 1.00f); }

        // Cards de destaque (fundo azul-marinho + borda clara discreta)
        inline ImVec4 CardBg() { return ImVec4(0.040f, 0.045f, 0.060f, 1.00f); }
        inline ImVec4 CardBorder() { return ImVec4(1.00f, 1.00f, 1.00f, 0.10f); }

        // Cores de texto
        inline ImVec4 Text() { return ImVec4(1.00f, 1.00f, 1.00f, 1.00f); }
        inline ImVec4 TextDisabled() { return ImVec4(0.380f, 0.400f, 0.450f, 1.00f); }
        inline ImVec4 TextSecondary() { return ImVec4(0.505f, 0.535f, 0.595f, 1.0f); }
        inline ImVec4 TextSeparator() { return ImVec4(0.505f, 0.535f, 0.595f, 1.00f); }

        // Cores de botões (neutros escuros; o azul entra no botão principal)
        inline ImVec4 Button() { return ImVec4(0.055f, 0.065f, 0.090f, 1.00f); }
        inline ImVec4 ButtonHovered() { return ImVec4(0.085f, 0.100f, 0.140f, 1.00f); }
        inline ImVec4 ButtonActive() { return ImVec4(0.040f, 0.050f, 0.070f, 1.00f); }
        inline ImVec4 ButtonTransparent() { return ImVec4(0.0f, 0.0f, 0.0f, 0.0f); }

        // Cores de frames (inputs/sliders/combos)
        inline ImVec4 FrameBg() { return ImVec4(0.070f, 0.080f, 0.105f, 1.00f); }
        inline ImVec4 FrameBgHovered() { return ImVec4(0.095f, 0.110f, 0.145f, 1.00f); }
        inline ImVec4 FrameBgActive() { return ImVec4(0.060f, 0.070f, 0.095f, 1.00f); }

        // Cores de header
        inline ImVec4 Header() { return ImVec4(0.070f, 0.080f, 0.105f, 0.90f); }
        inline ImVec4 HeaderHovered() { return ImVec4(0.100f, 0.115f, 0.150f, 0.95f); }
        inline ImVec4 HeaderActive() { return ImVec4(0.085f, 0.100f, 0.135f, 1.00f); }

        // Cores de separadores
        inline ImVec4 Separator() { return ImVec4(1.00f, 1.00f, 1.00f, 0.08f); }
        inline ImVec4 SeparatorHovered() { return ImVec4(1.00f, 1.00f, 1.00f, 0.16f); }
        inline ImVec4 SeparatorActive() { return ImVec4(0.169f, 0.322f, 0.631f, 1.00f); }

        // Cores de bordas (branco translúcido, como --border do site)
        inline ImVec4 Border() { return ImVec4(1.00f, 1.00f, 1.00f, 0.08f); }
        inline ImVec4 BorderShadow() { return ImVec4(0.0f, 0.0f, 0.0f, 0.0f); }
        inline ImVec4 BorderNormal() { return ImVec4(1.00f, 1.00f, 1.00f, 0.14f); }

        // Cores de scrollbar (azul discreto)
        inline ImVec4 ScrollbarBg() { return ImVec4(0.027f, 0.035f, 0.055f, 0.80f); }
        inline ImVec4 ScrollbarGrab() { return ImVec4(0.100f, 0.150f, 0.280f, 0.90f); }
        inline ImVec4 ScrollbarGrabHovered() { return ImVec4(0.150f, 0.240f, 0.450f, 0.95f); }
        inline ImVec4 ScrollbarGrabActive() { return ImVec4(0.231f, 0.510f, 0.965f, 1.00f); }

        // Cores de slider
        inline ImVec4 SliderGrab() { return ImVec4(0.231f, 0.510f, 0.965f, 1.0f); }
        inline ImVec4 SliderGrabActive() { return ImVec4(0.376f, 0.647f, 0.980f, 1.0f); }
        inline ImVec4 SliderTrack() { return ImVec4(0.070f, 0.080f, 0.105f, 1.0f); }

        // Cores de tabs
        inline ImVec4 Tab() { return ImVec4(0.055f, 0.065f, 0.090f, 0.90f); }
        inline ImVec4 TabHovered() { return ImVec4(0.085f, 0.100f, 0.140f, 0.95f); }
        inline ImVec4 TabActive() { return ImVec4(0.070f, 0.080f, 0.110f, 1.00f); }
        inline ImVec4 TabUnfocused() { return ImVec4(0.040f, 0.045f, 0.060f, 0.80f); }
        inline ImVec4 TabUnfocusedActive() { return ImVec4(0.055f, 0.065f, 0.090f, 0.90f); }

        // Cores de plot
        inline ImVec4 PlotLines() { return ImVec4(0.376f, 0.647f, 0.980f, 0.90f); }
        inline ImVec4 PlotLinesHovered() { return ImVec4(0.600f, 0.760f, 1.000f, 1.00f); }

        // Cores de dimming
        inline ImVec4 NavWindowingDimBg() { return ImVec4(0.016f, 0.019f, 0.024f, 0.70f); }
        inline ImVec4 ModalWindowDimBg() { return ImVec4(0.016f, 0.019f, 0.024f, 0.70f); }
    }

    // ============================================
    // CORES DE DESTAQUE AZUL SECAUTH
    // ============================================
    namespace Accent {
        inline ImVec4 Blue() { return ImVec4(0.231f, 0.510f, 0.965f, 1.0f); }
        inline ImVec4 BlueAlpha(float alpha) { return ImVec4(0.231f, 0.510f, 0.965f, alpha); }

        // Compatibilidade: aliases herdados do tema anterior
        inline ImVec4 Red() { return Blue(); }
        inline ImVec4 RedAlpha(float alpha) { return BlueAlpha(alpha); }

        // Cores de navegação e drag-drop
        inline ImVec4 TextSelectedBg() { return ImVec4(0.231f, 0.510f, 0.965f, 0.40f); }
        inline ImVec4 DragDropTarget() { return ImVec4(0.231f, 0.510f, 0.965f, 0.45f); }
        inline ImVec4 NavHighlight() { return ImVec4(0.231f, 0.510f, 0.965f, 0.45f); }
        inline ImVec4 NavWindowingHighlight() { return ImVec4(0.376f, 0.647f, 0.980f, 0.65f); }

        // Cores de botão destacado
        inline ImVec4 ButtonHighlight() { return ImVec4(0.231f, 0.510f, 0.965f, 1.0f); }
        inline ImVec4 ButtonHighlightHovered() { return ImVec4(0.376f, 0.647f, 0.980f, 1.0f); }
        inline ImVec4 ButtonHighlightActive() { return ImVec4(0.102f, 0.212f, 0.451f, 1.0f); }

        // Cores de título/seção
        inline ImVec4 Title() { return ImVec4(0.376f, 0.647f, 0.980f, 1.0f); }
    }

    // ============================================
    // CORES DE FEEDBACK
    // ============================================
    namespace Feedback {
        inline ImVec4 Success() { return ImVec4(0.133f, 0.773f, 0.369f, 1.0f); }
        inline ImVec4 SuccessLight() { return ImVec4(0.290f, 0.870f, 0.502f, 1.0f); }

        // Erro
        inline ImVec4 Error() { return ImVec4(0.937f, 0.267f, 0.267f, 1.0f); }

        // Aviso
        inline ImVec4 Warning() { return ImVec4(0.961f, 0.620f, 0.043f, 1.0f); }
    }

    // ============================================
    // CORES DE CHECKBOX/CHECKMARK
    // ============================================
    namespace Checkbox {
        // Checkmark branco para contrastar com o fundo azul
        inline ImVec4 CheckMark() { return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); }
        inline ImVec4 CheckMarkTransparent() { return ImVec4(0.0f, 0.0f, 0.0f, 0.0f); }
    }

    // ============================================
    // CORES DE ANIMAÇÃO/TEXTO
    // ============================================
    namespace Animation {
        // Cores de texto animado
        inline ImVec4 TextNormal() { return ImVec4(0.95f, 0.95f, 0.95f, 1.0f); }
        inline ImVec4 TextHover() { return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); }

        // Cores de pulso
        inline ImVec4 TextPulse(float pulse) {
            return ImVec4(0.98f * pulse, 0.98f * pulse, 0.98f * pulse, 1.0f);
        }
    }

    // ============================================
    // CORES BÁSICAS/PADRÃO
    // ============================================
    namespace Basic {
        // Branco puro
        inline ImVec4 White() { return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); }
        inline ImVec4 WhiteAlpha(float alpha) { return ImVec4(1.0f, 1.0f, 1.0f, alpha); }

        // Preto puro
        inline ImVec4 Black() { return ImVec4(0.0f, 0.0f, 0.0f, 1.0f); }
        inline ImVec4 BlackAlpha(float alpha) { return ImVec4(0.0f, 0.0f, 0.0f, alpha); }

        // Transparente
        inline ImVec4 Transparent() { return ImVec4(0.0f, 0.0f, 0.0f, 0.0f); }

        // Glow azul (identidade SecAuth)
        inline ImU32 GlowWhite(int alpha = 255) { return IM_COL32(96, 165, 250, alpha); }
        inline ImU32 GlowBlue(int alpha = 80) { return IM_COL32(59, 130, 246, alpha); }
        // Compatibilidade com o nome antigo
        inline ImU32 GlowRed(int alpha = 80) { return GlowBlue(alpha); }
    }
}
