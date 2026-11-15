#pragma once
#include <imgui.h>
#include <cmath>

namespace ImCustomStyle {
    // ============================================
    // CONFIGURAÇÕES DE PADDING E ESPAÇAMENTO
    // ============================================
    namespace Padding {
        // Padding para ícones (evitar corte)
        constexpr float IconMultiplier = 1.5f;      // Multiplicador base para padding de ícones
        constexpr float IconMultiplierTab = 2.0f;   // Multiplicador para tabs (mais espaço)
        
        // Padding padrão baseado no FramePadding do ImGui
        inline float GetIconPadding(const ImGuiStyle& style) {
            return style.FramePadding.x * IconMultiplier;
        }
        
        inline float GetIconPaddingTab(const ImGuiStyle& style) {
            return style.FramePadding.x * IconMultiplierTab;
        }
    }
    
    // ============================================
    // CONFIGURAÇÕES DE OPACIDADE
    // ============================================
    namespace Opacity {
        // Hover states
        constexpr float HoverTab = 0.12f;           // Hover em tabs (suave)
        constexpr float HoverSidebar = 0.4f;        // Hover na sidebar (mais visível)
        constexpr float HoverCombo = 0.25f;         // Hover em itens do combo
        
        // Selected states
        constexpr float SelectedTab = 1.0f;         // Tab selecionada (opaco)
        constexpr float SelectedCombo = 0.4f;      // Item do combo selecionado
        
        // Borders
        constexpr float BorderHoverTab = 0.4f;      // Borda hover em tabs
        constexpr float BorderHoverCombo = 0.6f;   // Borda hover em combo
        constexpr float BorderSelectedCombo = 1.0f; // Borda item selecionado no combo
        
        // Pulse/Animation
        constexpr float PulseMin = 0.3f;            // Opacidade mínima do pulso
        constexpr float PulseMax = 0.5f;            // Opacidade máxima do pulso
        constexpr float PulseSelectedMin = 0.8f;    // Opacidade mínima do pulso quando selecionado
        constexpr float PulseSelectedMax = 1.0f;    // Opacidade máxima do pulso quando selecionado
    }
    
    // ============================================
    // CONFIGURAÇÕES DE BORDAS
    // ============================================
    namespace Border {
        // Espessura das bordas
        constexpr float ThicknessHover = 1.0f;       // Borda hover padrão
        constexpr float ThicknessSelected = 1.5f;   // Borda item selecionado no combo
        constexpr float ThicknessPulse = 2.0f;      // Borda pulsante (tabs/sidebar)
        constexpr float ThicknessFrame = 2.0f;      // Borda da moldura principal

        // Moldura WG
        constexpr float FrameBorderThickness = 2.0f;  // Espessura da borda da moldura
        constexpr float FrameGlowThickness = 6.0f;    // Espessura do glow externo
        constexpr int FrameGlowAlpha = 45;            // Alpha do glow externo
    }
    
    // ============================================
    // CONFIGURAÇÕES DE ANIMAÇÃO
    // ============================================
    namespace Animation {
        // Velocidades
        constexpr float PulseSpeed = 2.0f;         // Velocidade do pulso (tabs)
        constexpr float GlowSpeed = 2.0f;           // Velocidade do glow interno
        constexpr float HoverFactorSpeed = 8.0f;    // Velocidade do fator de hover
        
        // Intensidades
        constexpr float GlowIntensityMin = 0.7f;    // Intensidade mínima do glow
        constexpr float GlowIntensityMax = 1.0f;    // Intensidade máxima do glow
        constexpr float GlowIntensityRange = 0.3f;  // Range do glow (0.3 = 0.7 a 1.0)
        constexpr int GlowAlpha = 20;               // Alpha do glow branco (0-255)
        
        // Pulse ranges
        constexpr float PulseRangeMin = 0.2f;       // Range mínimo do pulso
        constexpr float PulseRangeMax = 0.8f;       // Range máximo do pulso
    }
    
    // ============================================
    // CONFIGURAÇÕES DE GLOW/BRILHO
    // ============================================
    namespace Glow {
        // Glow interno (pulsante)
        constexpr float Offset = 2.0f;             // Offset do glow (pixels)
        constexpr float RoundingOffset = 2.0f;      // Offset do rounding do glow
        
        // Glow hover (sidebar)
        constexpr float HoverGlowFactor = 0.5f;     // Fator do glow no hover
        constexpr int HoverGlowAlpha = 30;          // Alpha do glow branco no hover (0-255)
        constexpr float HoverGlowThickness = 1.5f;  // Espessura do glow no hover
    }
    
    // ============================================
    // CONFIGURAÇÕES DE GRADIENTE
    // ============================================
    namespace Gradient {
        // Segmentos do gradiente (mais segmentos = mais suave)
        constexpr int Segments = 64;                // Número de segmentos para criar gradiente suave
    }

    // ============================================
    // CONFIGURAÇÕES DE CARDS WG
    // ============================================
    namespace Card {
        constexpr float Rounding = 10.0f;           // Arredondamento dos cards de setting
        constexpr float PaddingX = 16.0f;           // Padding horizontal interno
        constexpr float PaddingY = 12.0f;           // Padding vertical interno
        constexpr float Spacing = 10.0f;            // Espaço entre cards
        constexpr float SidebarButtonH = 44.0f;     // Altura dos botões da sidebar
        constexpr float LogoHeaderH = 92.0f;        // Altura do header da logo
    }
    
    // ============================================
    // CONFIGURAÇÕES DE CLIPPING
    // ============================================
    namespace Clipping {
        // Configurações de clipping para texto/ícones
        constexpr bool EnableClipping = true;       // Habilitar clipping para evitar corte
    }
    
    // ============================================
    // FUNÇÕES AUXILIARES DE ANIMAÇÃO
    // ============================================
    namespace Helpers {
        // Calcular pulso baseado no tempo
        inline float CalculatePulse(float time, float speed = Animation::PulseSpeed) {
            return (sinf(time * speed) + 1.0f) * 0.5f; // 0.0 a 1.0
        }
        
        // Calcular opacidade do pulso
        inline float CalculatePulseAlpha(float pulse, float min = Opacity::PulseMin, float max = Opacity::PulseMax) {
            return min + (pulse * (max - min));
        }
        
        // Calcular intensidade do glow
        inline float CalculateGlowIntensity(float time, float speed = Animation::GlowSpeed) {
            return (sinf(time * speed) + 1.0f) * Animation::GlowIntensityRange + Animation::GlowIntensityMin;
        }
        
        // Calcular opacidade do pulso quando selecionado
        inline float CalculatePulseSelectedAlpha(float pulse) {
            return Opacity::PulseSelectedMin + (pulse * (Opacity::PulseSelectedMax - Opacity::PulseSelectedMin));
        }
    }
}

