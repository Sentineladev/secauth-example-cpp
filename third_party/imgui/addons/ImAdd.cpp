#include "ImAdd.h"
#include "ImCustomColors.h"
#include <cmath>

ImVec4 ImAdd::Hex2RGBA(int hexValue, float alpha)
{
    float r = (hexValue >> 16) & 0xFF;
    float g = (hexValue >> 8) & 0xFF;
    float b = hexValue & 0xFF;

    return ImVec4(r / 255, g / 255, b / 255, alpha);
}

bool ImAdd::Button(const char* label, const ImVec2& size_arg, ImDrawFlags drawflag, bool enabled)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered = false, held = false;
    bool pressed = false;
    if (enabled) {
        pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
    }
    // Desabilitado: item registrado mas sem interação (sem hover/pressed).

    // Animate Colors
    static std::map<ImGuiID, Button_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, Button_State() });
        it_anim = anim.find(id);
    }

    it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, (held && hovered) ? ImCustomColors::UI::ButtonActive() : hovered ? ImCustomColors::UI::ButtonHovered() : ImCustomColors::UI::Button(), g.IO.DeltaTime * fAnimationSpeed);

    // Render com gradiente de 2 cores
    ImGui::RenderNavHighlight(bb, id);

    // Estados (tema vermelho): hover clareia, pressed escurece, disabled é cinza chapado
    auto applyMul = [](const ImVec4& c, float m) {
        return ImVec4(ImMin(1.0f, c.x * m), ImMin(1.0f, c.y * m), ImMin(1.0f, c.z * m), c.w);
    };
    float stateMul = 1.0f;
    if (enabled) {
        if (held && hovered) stateMul = 0.70f;      // pressed: resposta imediata escurecida
        else if (hovered) stateMul = 1.20f;         // hover: clareia perceptível em toda a área
    }

    if (!enabled) {
        // Desabilitado/loading: cinza chapado, sem gradiente nem hover
        window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(ImVec4(0.13f, 0.13f, 0.13f, 1.0f)), style.FrameRounding, drawflag);
    } else {
    // Gradiente: cor1 (esquerda) -> cor2 (direita)
    ImVec4 gradientColor1 = applyMul(ImCustomColors::Gradient::GetPrimary(), stateMul);
    ImVec4 gradientColor2 = applyMul(ImCustomColors::Gradient::GetSecondary(), stateMul);

    // Desenhar gradiente horizontal
    ImU32 col1 = ImGui::GetColorU32(gradientColor1);
    ImU32 col2 = ImGui::GetColorU32(gradientColor2);

    // Desenhar gradiente horizontal com o MESMO rounding do botão:
    // só o primeiro/último segmento arredondam (meio sempre reto).
    // Segmentos vizinhos se sobrepõem 1px: sem isso, coordenadas
    // fracionárias deixam filetes sem pintar entre as fatias.
    if (style.FrameRounding > 0.0f) {
        // Criar gradiente usando múltiplos retângulos pequenos
        int segments = ImCustomStyle::Gradient::Segments; // Mais segmentos = gradiente mais suave
        for (int i = 0; i < segments; i++) {
            float t1 = (float)i / segments;
            float t2 = (float)(i + 1) / segments;
            ImVec2 p1 = ImVec2(bb.Min.x + (bb.Max.x - bb.Min.x) * t1, bb.Min.y);
            ImVec2 p2 = ImVec2(bb.Min.x + (bb.Max.x - bb.Min.x) * t2, bb.Max.y);
            if (p2.x <= p1.x) continue;
            if (i > 0) p1.x -= 1.0f;
            if (i < segments - 1) p2.x += 1.0f;
            // Interpolar cores: converter ImU32 para ImVec4, interpolar, converter de volta
            ImVec4 col1_vec = ImGui::ColorConvertU32ToFloat4(col1);
            ImVec4 col2_vec = ImGui::ColorConvertU32ToFloat4(col2);
            float t = (t1 + t2) * 0.5f;
            ImVec4 col_vec = ImVec4(
                col1_vec.x + (col2_vec.x - col1_vec.x) * t,
                col1_vec.y + (col2_vec.y - col1_vec.y) * t,
                col1_vec.z + (col2_vec.z - col1_vec.z) * t,
                col1_vec.w + (col2_vec.w - col1_vec.w) * t
            );
            ImU32 col = ImGui::ColorConvertFloat4ToU32(col_vec);
            float segRounding = 0.0f;
            ImDrawFlags segFlags = ImDrawFlags_RoundCornersNone;
            if (segments == 1) {
                segRounding = style.FrameRounding;
                segFlags = drawflag;
            } else if (i == 0) {
                segRounding = style.FrameRounding;
                segFlags = ImDrawFlags_RoundCornersLeft;
            } else if (i == segments - 1) {
                segRounding = style.FrameRounding;
                segFlags = ImDrawFlags_RoundCornersRight;
            }
            window->DrawList->AddRectFilled(p1, p2, col, segRounding, segFlags);
        }
    } else {
        // Sem rounding, usar AddRectFilledMultiColor diretamente
        ImVec4 c1m = applyMul(ImGui::ColorConvertU32ToFloat4(col1), stateMul);
        ImVec4 c2m = applyMul(ImGui::ColorConvertU32ToFloat4(col2), stateMul);
        window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max,
            ImGui::ColorConvertFloat4ToU32(c1m), ImGui::ColorConvertFloat4ToU32(c2m),
            ImGui::ColorConvertFloat4ToU32(c2m), ImGui::ColorConvertFloat4ToU32(c1m));
    }
    } // fim do fill com gradiente (enabled)
    if (style.FrameBorderSize > 0.0f)
    {
        window->DrawList->AddRect(bb.Min + ImVec2(1, 1), bb.Max + ImVec2(1, 1), ImGui::GetColorU32(ImCustomColors::UI::Border()), style.FrameRounding, drawflag, style.FrameBorderSize);
        window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImCustomColors::UI::Border()), style.FrameRounding, drawflag, style.FrameBorderSize);
    }

    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("[", "]");
    ImVec4 btnTextCol = !enabled ? ImCustomColors::UI::TextDisabled()
        : (held && hovered) ? ImVec4(0.82f, 0.82f, 0.82f, 1.0f)
        : hovered ? ImCustomColors::Basic::White()
        : ImVec4(0.96f, 0.96f, 0.96f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, btnTextCol);
    ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    ImGui::PopStyleColor();

    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool ImAdd::Selectable(const char* label, bool selected, const ImVec2& size_arg, ImDrawFlags draw_flags)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
    // Verificação adicional de hover para garantir que seja detectado corretamente
    if (!hovered) {
        hovered = ImGui::ItemHoverable(bb, id, 0);
    }

    // Animate Colors
    static std::map<ImGuiID, Selectable_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, Selectable_State() });
        it_anim = anim.find(id);
    }

    // Cor do fundo: priorizar hover mesmo quando selected, para efeito hover visível
    ImVec4 target_frame_col;
    if (hovered) {
        if (selected) {
            // Quando hover E selected: usar cor de selecionado com brilho aumentado
            ImVec4 selectedCol = ImCustomColors::UI::HeaderActive();
            // Aumentar brilho quando hover sobre selected
            selectedCol.x = ImMin(1.0f, selectedCol.x * 1.2f);
            selectedCol.y = ImMin(1.0f, selectedCol.y * 1.2f);
            selectedCol.z = ImMin(1.0f, selectedCol.z * 1.2f);
            target_frame_col = selectedCol;
        } else if (held) {
            target_frame_col = ImCustomColors::UI::ButtonActive();
        } else {
            // Quando hover mas NÃO selected: usar cor similar à selected mas bem visível
            // Isso garante que seja bem visível (igual ao efeito de seleção)
            ImVec4 selectedCol = ImCustomColors::UI::HeaderActive();
            // Usar cor de selecionado com brilho reduzido mas ainda bem visível
            // Se a cor de selected for muito escura, aumentar o brilho
            float brightness = (selectedCol.x + selectedCol.y + selectedCol.z) / 3.0f;
            if (brightness < 0.3f) {
                // Se muito escura, aumentar significativamente
                target_frame_col = ImVec4(
                    ImMin(1.0f, selectedCol.x * 1.5f),
                    ImMin(1.0f, selectedCol.y * 1.5f),
                    ImMin(1.0f, selectedCol.z * 1.5f),
                    ImMax(0.95f, selectedCol.w)  // Alta opacidade
                );
            } else {
                // Se já tem brilho, usar 90% para distinguir do selected
                target_frame_col = ImVec4(
                    ImMin(1.0f, selectedCol.x * 0.9f),
                    ImMin(1.0f, selectedCol.y * 0.9f),
                    ImMin(1.0f, selectedCol.z * 0.9f),
                    ImMax(0.95f, selectedCol.w)  // Alta opacidade
                );
            }
        }
    } else if (selected) {
        target_frame_col = ImCustomColors::UI::HeaderActive();
    } else {
        // Quando não hover e não selected: cor padrão (transparente ou muito escura)
        target_frame_col = ImCustomColors::UI::Button();
    }
    it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, target_frame_col, g.IO.DeltaTime * fAnimationSpeed);
    
    // Cor do texto: hover sempre tem prioridade para destacar
    ImVec4 target_label_col;
    if (hovered) {
        target_label_col = ImCustomColors::UI::Text();
    } else if (selected) {
        target_label_col = ImCustomColors::UI::Text();
    } else {
        target_label_col = ImCustomColors::UI::TextDisabled();
    }
    it_anim->second.label_col = ImLerp(it_anim->second.label_col, target_label_col, g.IO.DeltaTime * fAnimationSpeed);

    // Render
    const ImU32 frame_col = ImGui::GetColorU32(it_anim->second.frame_col);
    const ImU32 label_col = ImGui::GetColorU32(it_anim->second.label_col);
    ImGui::RenderNavHighlight(bb, id);

    // Desenhar fundo quando selecionado OU quando hover (efeito hover visível)
    // IMPORTANTE: Sempre desenhar quando hover para garantir visibilidade
    if (selected || hovered) {
        window->DrawList->AddRectFilled(bb.Min, bb.Max, frame_col, style.FrameRounding, draw_flags);
    }

    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("[", "]");

    // Renderizar texto com clipping para evitar corte de ícones
    ImVec2 text_pos = pos + (size / 2) - (label_size / 2);
    
    // Padding para evitar corte de ícones
    float padding_x = ImCustomStyle::Padding::GetIconPadding(style);
    float available_width = size.x - (padding_x * 2.0f);
    
    // Ajustar posição se o texto for muito largo
    if (label_size.x > available_width) {
        // Se o texto não cabe, alinhar à esquerda com padding
        text_pos.x = pos.x + padding_x;
    } else {
        // Centralizar normalmente, mas garantir que não saia dos limites
        if (text_pos.x < pos.x + padding_x) {
            text_pos.x = pos.x + padding_x;
        }
        if (text_pos.x + label_size.x > pos.x + size.x - padding_x) {
            text_pos.x = pos.x + size.x - padding_x - label_size.x;
        }
    }
    
    // Aplicar clipping para garantir que o texto não ultrapasse os limites
    ImRect clip_rect(
        pos.x + padding_x,
        pos.y,
        pos.x + size.x - padding_x,
        pos.y + size.y
    );
    
    window->DrawList->PushClipRect(clip_rect.Min, clip_rect.Max, true);
    window->DrawList->AddText(text_pos, ImGui::GetColorU32(it_anim->second.label_col), label);
    window->DrawList->PopClipRect();

    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool ImAdd::RadioButton(const char* label, int current_id, int* id_parent, const ImVec2& size_arg, bool has_separator)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

    if (pressed)
    {
        *id_parent = current_id;
    }

    // Animate Colors
    static std::map<ImGuiID, RadioButton_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, RadioButton_State() });
        it_anim = anim.find(id);
    }

    it_anim->second.label_col = ImLerp(it_anim->second.label_col, hovered ? ImCustomColors::UI::Text() : *id_parent == current_id ? ImCustomColors::Gradient::GetPrimary() : ImCustomColors::UI::TextDisabled(), g.IO.DeltaTime * fAnimationSpeed);

    ImGui::RenderNavHighlight(bb, id);

    if (has_separator)
        window->DrawList->AddLine(pos + ImVec2(size.x - 1, style.WindowPadding.y + style.FramePadding.y), pos + ImVec2(size.x - 1, size.y - style.WindowPadding.y - style.FramePadding.y), ImGui::GetColorU32(ImCustomColors::UI::TextDisabled()), 1.0f);

    window->DrawList->AddText(pos + size / 2 - label_size / 2, ImGui::GetColorU32(it_anim->second.label_col), label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool ImAdd::RadioButtonIcon(const char* icon, const char* label, int current_id, int* id_parent, const ImVec2& size_arg, ImGuiCol active_col, bool borders)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 icon_size = ImGui::CalcTextSize(icon, NULL, true);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, icon_size.x + label_size.x + style.FramePadding.x * 3.0f, ImGui::GetFontSize() + style.FramePadding.y * 2.0f);

    const float line_width = 2.f;

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held, active;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
    active = *id_parent == current_id;

    if (pressed)
    {
        *id_parent = current_id;
    }

    // Animate Colors
    static std::map<ImGuiID, RadioButtonIcon_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, RadioButtonIcon_State() });
        it_anim = anim.find(id);
    }

    ImVec4 frame_bg = ImCustomColors::UI::HeaderActive(); // Usar HeaderActive como padrão para active_col
    ImVec4 frame_col = active ? frame_bg : ImVec4(frame_bg.x, frame_bg.y, frame_bg.z, 0.f);
    it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, frame_col, g.IO.DeltaTime * fAnimationSpeed);

    ImVec4 label_col = active ? ImCustomColors::UI::Text() : hovered ? ImCustomColors::UI::Text() : ImCustomColors::UI::TextDisabled();
    it_anim->second.label_col = ImLerp(it_anim->second.label_col, label_col, g.IO.DeltaTime * fAnimationSpeed);

    ImVec4 icon_col = active ? ImCustomColors::Gradient::GetPrimary() : hovered ? ImCustomColors::UI::Text() : ImCustomColors::UI::TextDisabled();
    it_anim->second.icon_col = ImLerp(it_anim->second.icon_col, icon_col, g.IO.DeltaTime * fAnimationSpeed);

    ImVec4 line_rgb = ImCustomColors::Gradient::GetPrimary();
    ImVec4 line_col = active ? line_rgb : ImVec4(line_rgb.x, line_rgb.y, line_rgb.z, 0.f);
    it_anim->second.line_col = ImLerp(it_anim->second.line_col, line_col, g.IO.DeltaTime * fAnimationSpeed);

    ImVec4 border_rgb = ImCustomColors::UI::Border();
    ImVec4 border_col = active ? border_rgb : ImVec4(border_rgb.x, border_rgb.y, border_rgb.z, 0.f);
    it_anim->second.border_col = ImLerp(it_anim->second.border_col, border_col, g.IO.DeltaTime * fAnimationSpeed);

    // Render
    ImGui::RenderNavHighlight(bb, id);

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(it_anim->second.frame_col), style.FrameRounding, 0);
    if (active && borders && style.FrameBorderSize > 0.0f)
    {
        //window->DrawList->AddRect(bb.Min + ImVec2(1, 1), bb.Max + ImVec2(1, 1), ImGui::GetColorU32(it_anim->second.border_col), style.FrameRounding, 0, style.FrameBorderSize);
        window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(it_anim->second.border_col), style.FrameRounding, 0, style.FrameBorderSize);
    }

    window->DrawList->AddText(pos + ImVec2((size.y - icon_size.y) / 2, (size.y / 2) - (icon_size.y / 2) + 0.7f), ImGui::GetColorU32(it_anim->second.icon_col), icon);

    window->DrawList->AddText(pos + ImVec2(size.y, (size.y / 2) - (label_size.y / 2)), ImGui::GetColorU32(it_anim->second.label_col), label);


    auto s_pos = pos - ImVec2(style.WindowPadding.x, 0);
    int LineSize = 2;

    ImGui::GetForegroundDrawList()->AddRectFilled(s_pos + ImVec2(0, LineSize * 2), s_pos + ImVec2(LineSize, size.y - LineSize * 2), ImGui::GetColorU32(it_anim->second.line_col), LineSize / 2, ImDrawFlags_RoundCornersRight);

    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool ImAdd::Checkbox(const char* label, bool* v, bool padding)
{
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const float width = ImGui::CalcItemSize(ImVec2(-0.1, 0), 0, 0).x;

    ImVec2 frame_padding = padding ? style.FramePadding : ImVec2(0, 0);
    float height = padding ? GetFrameHeight() : GetFontSize();

    const float square_sz = GetFontSize() + frame_padding.y * 2;
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(width, label_size.y + frame_padding.y * 2.0f));
    ItemSize(total_bb, frame_padding.y);
    if (!ItemAdd(total_bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    // Animate Colors
    static std::map<ImGuiID, CheckBox_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, CheckBox_State() });
        it_anim = anim.find(id);
    }

    ImColor frame_col = *v ? (held && hovered) ? ImCustomColors::Gradient::GetSecondary() : ImCustomColors::Gradient::GetPrimary() : (held && hovered) ? ImCustomColors::UI::FrameBgActive() : hovered ? ImCustomColors::UI::FrameBgHovered() : ImCustomColors::UI::FrameBg();
    it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, frame_col, g.IO.DeltaTime * fAnimationSpeed);

    ImVec4 border_rgb = ImCustomColors::UI::Border();
    ImVec4 border_col = !*v ? border_rgb : ImVec4(border_rgb.x, border_rgb.y, border_rgb.z, 0.0f);
    it_anim->second.border_col = ImLerp(it_anim->second.border_col, border_col, g.IO.DeltaTime * fAnimationSpeed);

    // Checkmark: usar branco quando marcado para contrastar com o fundo laranja
    ImVec4 mark_col = *v ? ImCustomColors::Checkbox::CheckMark() : ImCustomColors::Checkbox::CheckMarkTransparent();
    it_anim->second.mark_col = ImLerp(it_anim->second.mark_col, mark_col, g.IO.DeltaTime * fAnimationSpeed);

    // Renders
    const ImRect check_bb(pos + ImVec2(width - height, 0), pos + ImVec2(square_sz, square_sz) + ImVec2(width - height, 0));
    RenderNavHighlight(total_bb, id);

    window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32(it_anim->second.frame_col), style.FrameRounding);
    const float border_size = g.Style.FrameBorderSize;
    if (border_size > 0.0f)
    {
        window->DrawList->AddRect(check_bb.Min, check_bb.Max, GetColorU32(it_anim->second.border_col), style.FrameRounding, 0, border_size);
    }

    ImU32 check_col = GetColorU32(it_anim->second.mark_col);
    bool mixed_value = (g.LastItemData.ItemFlags & ImGuiItemFlags_MixedValue) != 0;
    if (mixed_value)
    {
        // Undocumented tristate/mixed/indeterminate checkbox (#2644)
        // This may seem awkwardly designed because the aim is to make ImGuiItemFlags_MixedValue supported by all widgets (not just checkbox)
        ImVec2 pad(ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)), ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)));
        window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad, check_col, style.FrameRounding);
    }
    // Always render the mark
    {
        const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 3.7f));
        RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
    }

    ImVec2 label_pos = ImVec2(pos.x, check_bb.Min.y + frame_padding.y);
    if (g.LogEnabled)
        LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");

    if (label_size.x > 0.0f)
        RenderText(label_pos, label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}

bool ImAdd::ClassicCheckbox(const char* label, bool* v)
{
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 frame_padding = ImVec2(0, 0);

    const float square_sz = GetFontSize() + frame_padding.y;
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + frame_padding.y * 2.0f));
    ItemSize(total_bb, frame_padding.y);
    if (!ItemAdd(total_bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    // Animate Colors
    static std::map<ImGuiID, CheckBox_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, CheckBox_State() });
        it_anim = anim.find(id);
    }

    ImColor frame_col = *v ? (held && hovered) ? ImCustomColors::Gradient::GetSecondary() : ImCustomColors::Gradient::GetPrimary() : (held && hovered) ? ImCustomColors::UI::FrameBgActive() : hovered ? ImCustomColors::UI::FrameBgHovered() : ImCustomColors::UI::FrameBg();
    it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, frame_col, g.IO.DeltaTime * fAnimationSpeed);

    ImVec4 border_rgb = ImCustomColors::UI::Border();
    ImVec4 border_col = !*v ? border_rgb : ImVec4(border_rgb.x, border_rgb.y, border_rgb.z, 0.0f);
    it_anim->second.border_col = ImLerp(it_anim->second.border_col, border_col, g.IO.DeltaTime * fAnimationSpeed);

    // Checkmark: usar branco quando marcado para contrastar com o fundo laranja
    ImVec4 mark_col = *v ? ImCustomColors::Checkbox::CheckMark() : ImCustomColors::Checkbox::CheckMarkTransparent();
    it_anim->second.mark_col = ImLerp(it_anim->second.mark_col, mark_col, g.IO.DeltaTime * fAnimationSpeed);

    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
    RenderNavHighlight(total_bb, id);

    window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32(it_anim->second.frame_col), style.FrameRounding);
    const float border_size = g.Style.FrameBorderSize;
    if (border_size > 0.0f)
    {
        window->DrawList->AddRect(check_bb.Min, check_bb.Max, GetColorU32(it_anim->second.border_col), style.FrameRounding, 0, style.FrameBorderSize);
    }

    ImU32 check_col = GetColorU32(it_anim->second.mark_col);
    bool mixed_value = (g.LastItemData.ItemFlags & ImGuiItemFlags_MixedValue) != 0;
    if (mixed_value)
    {
        // Undocumented tristate/mixed/indeterminate checkbox (#2644)
        // This may seem awkwardly designed because the aim is to make ImGuiItemFlags_MixedValue supported by all widgets (not just checkbox)
        ImVec2 pad(ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)), ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)));
        window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad, check_col, style.FrameRounding);
    }
    // Always draw
    {
        const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 3.7f));
        RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
    }

    ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + frame_padding.y);
    if (g.LogEnabled)
        LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
    if (label_size.x > 0.0f)
        RenderText(label_pos, label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}

bool ImAdd::ToggleButton(const char* label, bool* v)
{
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float w = ImGui::CalcItemSize(ImVec2(-0.1f, 0.f), 0.f, 0.f).x;

    float height = ImGui::GetFontSize();
    const ImVec2 pos = window->DC.CursorPos;

    float width = height * 2.f;
    float radius = height * 0.50f;

    //const ImRect total_bb(pos, ImVec2(pos.x + width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), pos.y + label_size.y));
    const ImRect total_bb(pos, pos + ImVec2(w, label_size.y));

    ItemSize(total_bb);
    if (!ItemAdd(total_bb, id))
        return false;

    float last_active_id_timer = g.LastActiveIdTimer;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
        g.LastActiveIdTimer = 0.f;
    }

    if (g.LastActiveIdTimer == 0.f && g.LastActiveId == id && !pressed)
        g.LastActiveIdTimer = last_active_id_timer;

    // Animate Colors
    static std::map<ImGuiID, ToggleButton_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, ToggleButton_State() });
        it_anim = anim.find(id);
    }

    ImColor frame_col = *v ? (held && hovered) ? ImCustomColors::Gradient::GetSecondary() : ImCustomColors::Gradient::GetPrimary() : (held && hovered) ? ImCustomColors::UI::FrameBgActive() : hovered ? ImCustomColors::UI::FrameBgHovered() : ImCustomColors::UI::FrameBg();
    it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, frame_col, g.IO.DeltaTime * fAnimationSpeed);

    ImVec4 border_rgb = ImCustomColors::UI::Border();
    ImVec4 border_col = !*v ? border_rgb : ImVec4(border_rgb.x, border_rgb.y, border_rgb.z, 0.0f);
    it_anim->second.border_col = ImLerp(it_anim->second.border_col, border_col, g.IO.DeltaTime * fAnimationSpeed);

    ImVec4 dot_col = (*v || hovered) ? ImCustomColors::UI::Text() : ImCustomColors::UI::TextDisabled();
    it_anim->second.dot_col = ImLerp(it_anim->second.dot_col, dot_col, g.IO.DeltaTime * fAnimationSpeed);

    // Anime slide
    float t = *v ? 1.0f : 0.0f;

    if (g.LastActiveId == id)
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / 0.1f);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    const ImRect frame_bb(pos + ImVec2(w - width, 0), ImVec2(pos.x + width, pos.y + height) + ImVec2(w - width, 0));
    float edge_rounding = height * 0.5f;
    
    // Render: gradiente quando ligado, cor normal quando desligado
    if (*v) {
        // Quando ligado: gradiente de 2 cores (igual ao botão)
        ImVec4 gradientColor1 = ImCustomColors::Gradient::GetPrimary();
        ImVec4 gradientColor2 = ImCustomColors::Gradient::GetSecondary();
        ImU32 col1 = ImGui::GetColorU32(gradientColor1);
        ImU32 col2 = ImGui::GetColorU32(gradientColor2);
        
        // Desenhar gradiente horizontal com rounding correto
        // Primeiro desenhar um retângulo base com rounding para criar a máscara
        window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, col1, edge_rounding);
        
        // Depois desenhar o gradiente sobreposto, mas apenas nos segmentos internos
        // (evitar bordas para manter o rounding)
        if (edge_rounding > 0.0f) {
            // Criar gradiente usando múltiplos retângulos pequenos, mas sem rounding individual
            // para que o rounding geral seja respeitado
            int segments = 40;
            float padding = edge_rounding * 0.5f; // Pequeno padding para respeitar o rounding
            for (int i = 0; i < segments; i++) {
                float t1 = (float)i / segments;
                float t2 = (float)(i + 1) / segments;
                ImVec2 p1 = ImVec2(frame_bb.Min.x + (frame_bb.Max.x - frame_bb.Min.x) * t1 + padding, frame_bb.Min.y + padding);
                ImVec2 p2 = ImVec2(frame_bb.Min.x + (frame_bb.Max.x - frame_bb.Min.x) * t2 - padding, frame_bb.Max.y - padding);
                
                // Garantir que não ultrapasse os limites
                if (p2.x > p1.x && p2.y > p1.y) {
                    ImVec4 col1_vec = ImGui::ColorConvertU32ToFloat4(col1);
                    ImVec4 col2_vec = ImGui::ColorConvertU32ToFloat4(col2);
                    float t = (t1 + t2) * 0.5f;
                    ImVec4 col_vec = ImVec4(
                        col1_vec.x + (col2_vec.x - col1_vec.x) * t,
                        col1_vec.y + (col2_vec.y - col1_vec.y) * t,
                        col1_vec.z + (col2_vec.z - col1_vec.z) * t,
                        col1_vec.w + (col2_vec.w - col1_vec.w) * t
                    );
                    ImU32 col = ImGui::ColorConvertFloat4ToU32(col_vec);
                    window->DrawList->AddRectFilled(p1, p2, col, 0.0f, 0); // Sem rounding nos segmentos
                }
            }
        } else {
            // Sem rounding, usar AddRectFilledMultiColor diretamente
            window->DrawList->AddRectFilledMultiColor(frame_bb.Min, frame_bb.Max, col1, col2, col2, col1);
        }
    } else {
        // Quando desligado: cor normal (sem gradiente)
        window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, GetColorU32(it_anim->second.frame_col), edge_rounding);
    }
    
    if (!*v && style.FrameBorderSize > 0.0f)
    {
        window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, GetColorU32(GetColorU32(it_anim->second.border_col)), edge_rounding, 0, style.FrameBorderSize);
    }

    RenderNavHighlight(total_bb, id);

    ImVec2 label_pos = ImVec2(pos.x, frame_bb.Min.y);
    RenderText(label_pos, label);
    window->DrawList->AddCircleFilled(ImVec2(pos.x + radius + t * (width - radius * 2.0f), pos.y + radius) + ImVec2(w - width, 0), radius - 4, ImGui::GetColorU32(it_anim->second.dot_col), 36);

    return pressed;
}

bool ImAdd::ClassicToggleButton(const char* label, bool* v)
{
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    float height = ImGui::GetFontSize();
    const ImVec2 pos = window->DC.CursorPos;

    float width = height * 2.f;
    float radius = height * 0.50f;

    const ImRect total_bb(pos, ImVec2(pos.x + width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), pos.y + label_size.y));

    ItemSize(total_bb);
    if (!ItemAdd(total_bb, id))
        return false;

    float last_active_id_timer = g.LastActiveIdTimer;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
        g.LastActiveIdTimer = 0.f;
    }

    if (g.LastActiveIdTimer == 0.f && g.LastActiveId == id && !pressed)
        g.LastActiveIdTimer = last_active_id_timer;

    // Animate Colors
    static std::map<ImGuiID, ToggleButton_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, ToggleButton_State() });
        it_anim = anim.find(id);
    }

    ImColor frame_col = *v ? (held && hovered) ? ImCustomColors::Gradient::GetSecondary() : ImCustomColors::Gradient::GetPrimary() : (held && hovered) ? ImCustomColors::UI::FrameBgActive() : hovered ? ImCustomColors::UI::FrameBgHovered() : ImCustomColors::UI::FrameBg();
    it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, frame_col, g.IO.DeltaTime * fAnimationSpeed);

    ImVec4 border_rgb = ImCustomColors::UI::Border();
    ImVec4 border_col = !*v ? border_rgb : ImVec4(border_rgb.x, border_rgb.y, border_rgb.z, 0.0f);
    it_anim->second.border_col = ImLerp(it_anim->second.border_col, border_col, g.IO.DeltaTime * fAnimationSpeed);

    ImVec4 dot_col = (*v || hovered) ? ImCustomColors::UI::Text() : ImCustomColors::UI::TextDisabled();
    it_anim->second.dot_col = ImLerp(it_anim->second.dot_col, dot_col, g.IO.DeltaTime * fAnimationSpeed);

    // Anime slide
    float t = *v ? 1.0f : 0.0f;

    if (g.LastActiveId == id)
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / 0.1f);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    const ImRect frame_bb(pos, ImVec2(pos.x + width, pos.y + height));
    float edge_rounding = height * 0.5f;
    window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, GetColorU32(it_anim->second.frame_col), edge_rounding);
    if (!*v && style.FrameBorderSize > 0.0f)
    {
        window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, GetColorU32(GetColorU32(it_anim->second.border_col)), edge_rounding, 0, style.FrameBorderSize);
    }

    RenderNavHighlight(total_bb, id);

    ImVec2 label_pos = ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y);
    RenderText(label_pos, label);
    window->DrawList->AddCircleFilled(ImVec2(pos.x + radius + t * (width - radius * 2.0f), pos.y + radius), radius - 4, ImGui::GetColorU32(it_anim->second.dot_col), 36);

    return pressed;
}

bool ImAdd::CustomToggleButton(const char* label, bool* v)
{
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const float square_sz = 18.0f;
    const float y_size = 18.0f;
    const ImVec2 pos = window->DC.CursorPos;
    const float w = GetContentRegionMax().x - style.WindowPadding.x;

    const ImRect rect(pos + ImVec2(0, 0), pos + ImVec2(w, y_size));
    const ImRect total_bb(pos, pos + ImVec2(CalcTextSize(label).x + 110, y_size));

    ItemSize(total_bb, 0.f);
    if (!ItemAdd(rect, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(rect, id, &hovered, &held);

    // Animate state
    static std::map<ImGuiID, CustomToggleButton_State> anim;
    auto it_anim = anim.find(id);
    if (it_anim == anim.end())
    {
        anim.insert({ id, CustomToggleButton_State() });
        it_anim = anim.find(id);
        it_anim->second.background_opticaly = *v ? 0.2f : 1.0f;
        it_anim->second.circle_offset = *v ? 0.0f : -square_sz * 2.0f;
    }

    // Cores do projeto (usando as mesmas cores dos outros componentes)
    ImVec4 accentColor = ImCustomColors::Gradient::GetPrimary(); // Cor principal (laranja)
    ImVec4 fillingColor = ImCustomColors::UI::FrameBg(); // Cor de fundo
    ImVec4 circleMarkColor = ImCustomColors::UI::FrameBgHovered(); // Cor do círculo quando desligado
    ImVec4 textActiveColor = ImCustomColors::UI::Text(); // Texto quando ativo
    ImVec4 textHovColor = ImCustomColors::UI::Text(); // Texto quando hover
    ImVec4 textColor = ImCustomColors::UI::TextDisabled(); // Texto normal

    // Animar valores
    it_anim->second.background_opticaly = ImLerp(it_anim->second.background_opticaly, *v ? 0.2f : 1.0f, g.IO.DeltaTime * 12.0f);
    it_anim->second.circle_offset = ImLerp(it_anim->second.circle_offset, *v ? 0.0f : -square_sz * 2.0f, g.IO.DeltaTime * 12.0f);
    it_anim->second.background = ImLerp(it_anim->second.background, *v ? accentColor : fillingColor, g.IO.DeltaTime * 12.0f);
    it_anim->second.circle = ImLerp(it_anim->second.circle, *v ? accentColor : circleMarkColor, g.IO.DeltaTime * 12.0f);
    it_anim->second.text = ImLerp(it_anim->second.text, *v ? textActiveColor : hovered ? textHovColor : textColor, g.IO.DeltaTime * 12.0f);

    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    const ImRect check_bb(pos + ImVec2(w - (square_sz * 2), (y_size - square_sz) / 2), pos + ImVec2(w, (y_size + square_sz) / 2));
    const float rounding = 30.0f;

    // Desenhar fundo do toggle com gradiente quando ligado (com rounding)
    ImVec4 bgColor = it_anim->second.background;
    bgColor.w *= it_anim->second.background_opticaly;
    
    if (*v) {
        // Quando ligado: gradiente com rounding SEMPRE aplicado
        ImVec4 gradientColor1 = ImCustomColors::Gradient::GetPrimary();
        ImVec4 gradientColor2 = ImCustomColors::Gradient::GetSecondary();
        
        // Desenhar retângulo base com rounding completo primeiro (garante que o rounding seja visível)
        ImU32 baseCol = GetColorU32(ImVec4(gradientColor1.x, gradientColor1.y, gradientColor1.z, bgColor.w));
        window->DrawList->AddRectFilled(check_bb.Min + ImVec2(0, 3), check_bb.Max - ImVec2(0, 3), baseCol, rounding);
        
        // Depois desenhar o gradiente sobreposto usando múltiplos segmentos, mas mantendo o rounding nas extremidades
        int segments = 40;
        for (int i = 0; i < segments; i++) {
            float t1 = (float)i / segments;
            float t2 = (float)(i + 1) / segments;
            ImVec2 p1 = ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) * t1, check_bb.Min.y + 3);
            ImVec2 p2 = ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) * t2, check_bb.Max.y - 3);
            
            // Garantir que o primeiro segmento começa exatamente no início para ter rounding
            if (i == 0) {
                p1.x = check_bb.Min.x;
            }
            
            // Garantir que não ultrapasse os limites
            if (p2.x <= p1.x) continue;
            
            ImVec4 col1_vec = gradientColor1;
            ImVec4 col2_vec = gradientColor2;
            float t_lerp = (t1 + t2) * 0.5f;
            ImVec4 col_vec = ImVec4(
                col1_vec.x + (col2_vec.x - col1_vec.x) * t_lerp,
                col1_vec.y + (col2_vec.y - col1_vec.y) * t_lerp,
                col1_vec.z + (col2_vec.z - col1_vec.z) * t_lerp,
                bgColor.w
            );
            ImU32 interpolated_col = GetColorU32(col_vec);
            
            // Aplicar rounding SEMPRE nas extremidades para manter o rounding completo
            float segRounding = 0.0f;
            ImDrawFlags drawFlags = 0;
            if (i == 0) {
                segRounding = rounding; // Rounding na esquerda - SEMPRE
                drawFlags = ImDrawFlags_RoundCornersLeft;
            } else if (i == segments - 1) {
                segRounding = rounding; // Rounding na direita - SEMPRE
                drawFlags = ImDrawFlags_RoundCornersRight;
            }
            
            window->DrawList->AddRectFilled(p1, p2, interpolated_col, segRounding, drawFlags);
        }
    } else {
        // Quando desligado: cor sólida com rounding completo (já estava correto)
        window->DrawList->AddRectFilled(check_bb.Min + ImVec2(0, 3), check_bb.Max - ImVec2(0, 3), GetColorU32(bgColor), rounding);
    }

    // Desenhar círculo com efeito luminoso quando ligado
    ImVec2 circleCenter = check_bb.Max - ImVec2(y_size - it_anim->second.circle_offset, y_size) / 2.0f;
    
    if (*v) {
        // Efeito luminoso: círculos concêntricos com cores mais claras
        ImVec4 circleColor = it_anim->second.circle;
        
        // Círculo externo mais brilhante (glow)
        ImVec4 glowColor = circleColor;
        glowColor.x = ImMin(1.0f, glowColor.x * 1.5f);
        glowColor.y = ImMin(1.0f, glowColor.y * 1.5f);
        glowColor.z = ImMin(1.0f, glowColor.z * 1.5f);
        glowColor.w *= 0.4f;
        window->DrawList->AddCircleFilled(circleCenter, 11.0f, GetColorU32(glowColor), 30);
        
        // Círculo médio (brilho intermediário)
        ImVec4 midGlowColor = circleColor;
        midGlowColor.x = ImMin(1.0f, midGlowColor.x * 1.3f);
        midGlowColor.y = ImMin(1.0f, midGlowColor.y * 1.3f);
        midGlowColor.z = ImMin(1.0f, midGlowColor.z * 1.3f);
        midGlowColor.w *= 0.6f;
        window->DrawList->AddCircleFilled(circleCenter, 10.0f, GetColorU32(midGlowColor), 30);
        
        // Círculo principal (muito brilhante)
        ImVec4 brightColor = circleColor;
        brightColor.x = ImMin(1.0f, brightColor.x * 1.2f);
        brightColor.y = ImMin(1.0f, brightColor.y * 1.2f);
        brightColor.z = ImMin(1.0f, brightColor.z * 1.2f);
        window->DrawList->AddCircleFilled(circleCenter, 9.0f, GetColorU32(brightColor), 30);
        
        // Ponto central ainda mais brilhante
        ImVec4 centerColor = circleColor;
        centerColor.x = ImMin(1.0f, centerColor.x * 1.4f);
        centerColor.y = ImMin(1.0f, centerColor.y * 1.4f);
        centerColor.z = ImMin(1.0f, centerColor.z * 1.4f);
        window->DrawList->AddCircleFilled(circleCenter, 6.0f, GetColorU32(centerColor), 30);
    } else {
        // Quando desligado: círculo normal
        window->DrawList->AddCircleFilled(circleCenter, 9.0f, GetColorU32(it_anim->second.circle), 30);
    }

    // Desenhar texto
    PushStyleColor(ImGuiCol_Text, it_anim->second.text);
    RenderText(rect.Min, label);
    PopStyleColor();

    return pressed;
}

bool ImAdd::ColorEdit4(const char* label, float col[4])
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float width = ImGui::CalcItemSize(ImVec2(-0.1f, 0.f), 0.f, 0.f).x;

    auto pos = ImGui::GetCursorPos();

    std::string str1 = "##";
    std::string str2 = label;

    ImGui::Text(label);

    ImGui::SameLine(pos.x + width - ImGui::GetFrameHeight());

    if (ImGui::ColorButton((str1 + str2).c_str(), ImVec4(col[0], col[1], col[2], col[3]), ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoDragDrop, { ImGui::GetFrameHeight(), ImGui::GetFontSize() }))
    {
        ImGui::OpenPopup(("COLORPICKER##" + str2).c_str());
    }
    if (ImGui::BeginPopup(("COLORPICKER##" + str2).c_str()))
    {
        {
            ImGui::ColorPicker4(label, col, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview);
        }
        ImGui::EndPopup();
    }
    
    return false; // Retorna false pois não há mudança direta de valor
}

bool ImAdd::SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    //const float w = ImGui::CalcItemWidth();
    const float w = ImGui::CalcItemSize(ImVec2(-0.1, 0), 0, 0).x;

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos + ImVec2(0, ImGui::GetFontSize()), window->DC.CursorPos + ImVec2(0, ImGui::GetFontSize() - 4) + ImVec2(w, label_size.y));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
    const ImRect size_bb(frame_bb.Min, frame_bb.Max + ImVec2(0, ImGui::GetFontSize()));

    const bool temp_input_allowed = false;
    ImGui::ItemSize(size_bb, 0);
    if (!ImGui::ItemAdd(size_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
        return false;

    // Default format string when passing NULL
    if (format == NULL)
        format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;

    const bool hovered = ImGui::ItemHoverable(frame_bb, id, g.LastItemData.ItemFlags);
    const bool clicked = hovered && ImGui::IsMouseClicked(0, ImGuiInputFlags_None, id);
    const bool make_active = (clicked || g.NavActivateId == id);
    if (make_active && clicked)
        ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);

    if (make_active)
    {
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    // Animate Colors
    static std::map<ImGuiID, SliderScalar_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, SliderScalar_State() });
        it_anim = anim.find(id);
    }

    // Trilha estilo referência: cinza escuro liso, sem borda
    it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, ImCustomColors::UI::SliderTrack(), g.IO.DeltaTime * fAnimationSpeed);
    it_anim->second.line_col = ImLerp(it_anim->second.line_col, g.ActiveId == id ? ImCustomColors::Gradient::GetSecondary() : ImCustomColors::Gradient::GetPrimary(), g.IO.DeltaTime * fAnimationSpeed);
    it_anim->second.grab_col = ImLerp(it_anim->second.grab_col, ImCustomColors::Basic::White(), g.IO.DeltaTime * fAnimationSpeed);

    // Draw frame: trilha cheia arredondada, sem borda
    ImGui::RenderNavHighlight(frame_bb, id);
    ImRect frame_rect = ImRect(frame_bb.Min + ImVec2(2, 2), frame_bb.Max - ImVec2(2, 2));
    float frameRounding = (frame_rect.Max.y - frame_rect.Min.y) * 0.5f; // Pill completa
    window->DrawList->AddRectFilled(frame_rect.Min, frame_rect.Max, ImGui::GetColorU32(it_anim->second.frame_col), frameRounding);

    // Slider behavior
    ImRect grab_bb;
    const bool value_changed = ImGui::SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, 0, &grab_bb);
    if (value_changed)
        ImGui::MarkItemEdited(id);

    // Fill estilo referência: vermelho chapado, pill completa (o knob cobre a ponta)
    if (grab_bb.Max.x > grab_bb.Min.x) {
        ImVec2 lineMin = ImVec2(frame_rect.Min.x, frame_rect.Min.y);
        ImVec2 lineMax = ImVec2(grab_bb.Max.x - (grab_bb.Max.x - grab_bb.Min.x) * 0.5f, frame_rect.Max.y);
        if (lineMax.x > lineMin.x + 1.0f) {
            window->DrawList->AddRectFilled(lineMin, lineMax,
                ImGui::GetColorU32(ImCustomColors::Gradient::GetSecondary()), frameRounding);
        }
    }

    // Render grab circle (knob branco com anel vermelho WG)
    if (grab_bb.Max.x > grab_bb.Min.x)
    {
        ImVec2 knobCenter = grab_bb.Max - ImVec2((grab_bb.Max - grab_bb.Min).y / 2, (grab_bb.Max - grab_bb.Min).y / 2);
        float knobRadius = (frame_bb.Max - frame_bb.Min).y / 2;
        // Sombra sutil para destacar do fundo
        window->DrawList->AddCircleFilled(knobCenter + ImVec2(0, 1.0f), knobRadius, IM_COL32(0, 0, 0, 90), 32);
        // Corpo branco
        window->DrawList->AddCircleFilled(knobCenter, knobRadius, ImGui::GetColorU32(ImCustomColors::Basic::White()), 32);
        // Anel vermelho externo
        window->DrawList->AddCircle(knobCenter, knobRadius, ImGui::GetColorU32(ImCustomColors::Gradient::GetSecondary()), 32, 2.0f);
    }

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    char value_buf[64];
    const char* value_buf_end = value_buf + ImGui::DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

    ImVec2 value_label_pos = frame_bb.Max - ImVec2(ImGui::CalcTextSize(value_buf).x, ImGui::GetFontSize() * 2);
    window->DrawList->AddText(g.Font, g.FontSize, value_label_pos, ImGui::GetColorU32(ImCustomColors::UI::TextDisabled()), value_buf, value_buf_end);

    if (label_size.x > 0.0f)
        ImGui::RenderText(ImVec2(frame_bb.Min.x, frame_bb.Min.y - label_size.y - 4.0f), label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (temp_input_allowed ? ImGuiItemStatusFlags_Inputable : 0));
    return value_changed;
}

bool ImAdd::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format)
{
    return ImAdd::SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format);
}

bool ImAdd::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format)
{
    return ImAdd::SliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format);
}

bool ImAdd::Hotkey(const char* label, int* k, const ImVec2& size_arg)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    char buf_display[32] = "NONE";
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 buf_display_size = ImGui::CalcTextSize(buf_display, NULL, true);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 size = ImGui::CalcItemSize(size_arg, buf_display_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    size.x *= 5.f;

    ImRect frame_bb(pos, pos + size);
    ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(style.ItemInnerSpacing.x + buf_display_size.x, 0));

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    const bool hovered = ImGui::ItemHoverable(frame_bb, id, 0);

    if (hovered)
    {
        ImGui::SetHoveredID(id);
        g.MouseCursor = ImGuiMouseCursor_Hand;
    }

    const bool user_clicked = hovered && io.MouseClicked[0];

    if (user_clicked)
    {
        if (g.ActiveId != id)
        {
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            //memset(io.KeysDown, 0, sizeof(io.KeysDown));
            *k = 0;
        }
        ImGui::SetActiveID(id, window);
        ImGui::FocusWindow(window);
    } else if (io.MouseClicked[0])
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    bool value_changed = false;
    int key = *k;

    static bool is_ready_to_listen = false;

    if (g.ActiveId == id)
    {
        // --- Phase 1: Waiting for Activation Release ---
        // When the widget first becomes active, we are NOT ready to listen.
        // We wait for the user to release the left mouse button.
        // We use IsMouseReleased because it's a clean, single-frame event.
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            is_ready_to_listen = true;
        }

        // --- Phase 2: Listening for the New Key ---
        // Only if we have been explicitly told to listen, do we check for keys.
        if (is_ready_to_listen)
        {
            const int mouseButtons[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
            for (const auto& button : mouseButtons)
            {
                if (GetAsyncKeyState(button) & 0x8000)
                {
                    key = button;
                    value_changed = true;
                    break; // Found a key, stop searching
                }
            }

            if (!value_changed)
            {
                for (auto i = 0x08; i <= 0xA5; i++)
                {
                    if (GetAsyncKeyState(i) & 0x8000)
                    {
                        key = i;
                        value_changed = true;
                        break; // Found a key, stop searching
                    }
                }
            }
        }

        // --- Phase 3: Handling the Result ---
        if (value_changed)
        {
            *k = key;
            ImGui::ClearActiveID();
        } else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            *k = 0;
            ImGui::ClearActiveID();
        }
    } else
    {
        // If our widget is no longer the active one for any reason
        // (e.g., user clicked elsewhere), we must reset our state.
        is_ready_to_listen = false;
    }

    window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImCustomColors::UI::Button()), style.FrameRounding);

    const float border_size = g.Style.FrameBorderSize;
    if (border_size > 0.0f)
    {
        window->DrawList->AddRect(frame_bb.Min + ImVec2(1, 1), frame_bb.Max + ImVec2(1, 1), ImGui::GetColorU32(ImCustomColors::UI::Border()), style.FrameBorderSize, 0, border_size);
        window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImCustomColors::UI::Border()), style.FrameBorderSize, 0, border_size);
    }

    if (*k != 0 && g.ActiveId != id)
        strcpy(buf_display, szKeyNames[*k]);
    else if (g.ActiveId == id)
        strcpy(buf_display, "...");

    const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y);
    ImGui::RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL, NULL, style.ButtonTextAlign, &clip_rect);

    ImVec2 label_pos = frame_bb.Max - ImVec2(-style.ItemInnerSpacing.x, label_size.y + style.FramePadding.y);
    if (label_size.x > 0.0f)
        ImGui::RenderText(label_pos, label);

    return value_changed;
}

void ImAdd::SeparatorText(const char* text)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    auto pos = ImGui::GetCursorScreenPos();
    ImGui::PushStyleColor(ImGuiCol_Text, ImCustomColors::UI::TextSeparator());
    ImGui::Text(text);
    ImGui::PopStyleColor();

    float yOffset = static_cast<int>(ImGui::CalcTextSize(text).y / 2);
    bool bAjustThinkness = (ImGui::CalcTextSize(text).y / 2) - yOffset >= 0.5f ? true : false;

    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(pos.x + ImGui::CalcTextSize(text).x + style.WindowPadding.x,
            pos.y + static_cast<int>(ImGui::CalcTextSize(text).y / 2)),
        ImVec2(pos.x + ImGui::GetWindowSize().x - style.WindowPadding.x * 2,
            pos.y + static_cast<int>(ImGui::CalcTextSize(text).y / 2)),
        ImGui::GetColorU32(ImCustomColors::UI::Separator()),
        bAjustThinkness ? 2.f : 1.f);

}

void ImAdd::BeginChildGroup(const char* label, const ImVec2& size_arg)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.ItemInnerSpacing.y, style.ItemInnerSpacing.y));
    ImGui::BeginChild(label, size_arg, true, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) ImGui::TextDisabled(label), ImGui::EndMenuBar();
    ImGui::PopStyleVar();
}

bool ImAdd::BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags)
{
    using namespace ImGui;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.HasFlags;
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

    ImVec2 pos = window->DC.CursorPos;
    const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const float width = ImGui::CalcItemSize(ImVec2(-0.1f, 0.f), 0.f, 0.f).x;
    ImVec2 r_pos = ImVec2(0, label_size.y + style.ItemInnerSpacing.y);

    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(width, label_size.y + style.FramePadding.y * 2.0f));

    const ImRect total_bb(bb.Min, bb.Max + ImVec2(width, label_size.y + style.ItemInnerSpacing.y));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &bb))
        return false;

    // Open on click
    bool hovered, held;
    bool pressed = ButtonBehavior(ImRect(total_bb.Min + r_pos, total_bb.Max), id, &hovered, &held);
    const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
    bool popup_open = IsPopupOpen(popup_id, ImGuiPopupFlags_None);
    if (pressed && !popup_open)
    {
        OpenPopupEx(popup_id, ImGuiPopupFlags_None);
        popup_open = true;
    }

    // Animate Colors
    static std::map<ImGuiID, Combo_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, Combo_State() });
        it_anim = anim.find(id);
    }

    // Efeito hover suave - aumentar brilho quando hover
    ImVec4 frame_col_base = ImCustomColors::UI::FrameBg();
    ImVec4 frame_col_hover = ImCustomColors::UI::FrameBgHovered();
    
    if (hovered) {
        // Quando hover: aumentar brilho suavemente
        frame_col_hover.x = ImMin(1.0f, frame_col_hover.x * 1.15f);
        frame_col_hover.y = ImMin(1.0f, frame_col_hover.y * 1.15f);
        frame_col_hover.z = ImMin(1.0f, frame_col_hover.z * 1.15f);
    }
    
    ImVec4 frame_col = hovered ? frame_col_hover : frame_col_base;
    it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, frame_col, g.IO.DeltaTime * fAnimationSpeed);

    // Render shape
    const float value_x2 = ImMax(bb.Min.x, bb.Max.x - arrow_size);
    RenderNavHighlight(total_bb, id);

    if (!(flags & ImGuiComboFlags_NoPreview))
        window->DrawList->AddRectFilled(r_pos + bb.Min, r_pos + ImVec2(value_x2, bb.Max.y), GetColorU32(it_anim->second.frame_col), style.FrameRounding, (flags & ImGuiComboFlags_NoArrowButton) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersLeft);

    if (!(flags & ImGuiComboFlags_NoArrowButton))
    {
        ImVec4 bg_col = (popup_open || hovered) ? ImCustomColors::UI::ButtonHovered() : ImCustomColors::UI::Button();
        it_anim->second.bg_col = ImLerp(it_anim->second.bg_col, bg_col, g.IO.DeltaTime * fAnimationSpeed);

        ImVec4 text_col = ImCustomColors::UI::Text();
        it_anim->second.text_col = ImLerp(it_anim->second.text_col, text_col, g.IO.DeltaTime * fAnimationSpeed);

        window->DrawList->AddRectFilled(r_pos + ImVec2(value_x2, bb.Min.y), r_pos + bb.Max, GetColorU32(it_anim->second.bg_col), style.FrameRounding, (width <= arrow_size) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersRight);

        if (value_x2 + arrow_size - style.FramePadding.x <= bb.Max.x)
            RenderArrow(window->DrawList, r_pos + ImVec2(value_x2 + style.FramePadding.y, bb.Min.y + style.FramePadding.y), GetColorU32(it_anim->second.text_col), ImGuiDir_Down, 1.0f);
    }

    // Desenhar borda laranja quando popup está aberto ou hover
    if (popup_open || hovered) {
        ImVec4 borderColor = ImCustomColors::Gradient::GetPrimary();
        borderColor.w = 1.0f;
        float borderThickness = 2.0f;
        
        // Desenhar borda laranja ao redor do combo
        ImRect comboRect = ImRect(r_pos + bb.Min, r_pos + bb.Max);
        window->DrawList->AddRect(comboRect.Min, comboRect.Max, GetColorU32(borderColor), style.FrameRounding, ImDrawFlags_RoundCornersAll, borderThickness);
    } else {
        // Borda normal quando não está aberto
        RenderFrameBorder(r_pos + bb.Min, r_pos + bb.Max, style.FrameRounding);
    }

    // Custom preview
    if (flags & ImGuiComboFlags_CustomPreview)
    {
        g.ComboPreviewData.PreviewRect = ImRect(bb.Min.x, bb.Min.y, value_x2, bb.Max.y);
        IM_ASSERT(preview_value == NULL || preview_value[0] == 0);
        preview_value = NULL;
    }

    // Render preview and label
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
    {
        if (g.LogEnabled)
            LogSetNextTextDecoration("{", "}");
        RenderTextClipped(r_pos + bb.Min + style.FramePadding, r_pos + ImVec2(value_x2, bb.Max.y), preview_value, NULL, NULL);
    }
    if (label_size.x > 0)
        RenderText(ImVec2(pos.x, bb.Min.y), label);

    if (!popup_open)
        return false;

    g.NextWindowData.HasFlags = backup_next_window_data_flags;
    return ImAdd::BeginComboPopup(popup_id, ImRect(bb.Min + r_pos, bb.Max + r_pos), flags);
}

bool ImAdd::BeginComboPopup(ImGuiID popup_id, const ImRect& bb, ImGuiComboFlags flags)
{
    using namespace ImGui;

    ImGuiContext& g = *GImGui;
    if (!IsPopupOpen(popup_id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags();
        return false;
    }

    // Set popup size
    float w = bb.GetWidth();
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSizeConstraint)
    {
        g.NextWindowData.SizeConstraintRect.Min.x = ImMax(g.NextWindowData.SizeConstraintRect.Min.x, w);
    } else
    {
        if ((flags & ImGuiComboFlags_HeightMask_) == 0)
            flags |= ImGuiComboFlags_HeightRegular;
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_)); // Only one
        int popup_max_height_in_items = -1;
        if (flags & ImGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
        else if (flags & ImGuiComboFlags_HeightSmall)  popup_max_height_in_items = 4;
        else if (flags & ImGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
        ImVec2 constraint_min(0.0f, 0.0f), constraint_max(FLT_MAX, FLT_MAX);
        if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize) == 0 || g.NextWindowData.SizeVal.x <= 0.0f) // Don't apply constraints if user specified a size
            constraint_min.x = w;
        if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize) == 0 || g.NextWindowData.SizeVal.y <= 0.0f)
            constraint_max.y = CalcMaxPopupHeightFromItemCount(popup_max_height_in_items);
        SetNextWindowSizeConstraints(constraint_min, constraint_max);
    }

    // This is essentially a specialized version of BeginPopupEx()
    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

    // Set position given a custom constraint (peak into expected window size so we can position it)
    // FIXME: This might be easier to express with an hypothetical SetNextWindowPosConstraints() function?
    // FIXME: This might be moved to Begin() or at least around the same spot where Tooltips and other Popups are calling FindBestWindowPosForPopupEx()?
    if (ImGuiWindow* popup_window = FindWindowByName(name))
        if (popup_window->WasActive)
        {
            // Always override 'AutoPosLastDirection' to not leave a chance for a past value to affect us.
            ImVec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
            popup_window->AutoPosLastDirection = (flags & ImGuiComboFlags_PopupAlignLeft) ? ImGuiDir_Left : ImGuiDir_Down; // Left = "Below, Toward Left", Down = "Below, Toward Right (default)"
            ImRect r_outer = GetPopupAllowedExtentRect(popup_window);
            ImVec2 pos = FindBestWindowPosForPopupEx(bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, bb, ImGuiPopupPositionPolicy_ComboBox);
            SetNextWindowPos(pos);
        }

    // We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
    PushStyleVar(ImGuiStyleVar_WindowPadding, g.Style.FramePadding); // Horizontally align ourselves with the framed text
    bool ret = Begin(name, NULL, window_flags);
    PopStyleVar();
    if (!ret)
    {
        EndPopup();
        IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
        return false;
    }
    return true;
}

bool ImAdd::Combo(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int popup_max_height_in_items)
{
    using namespace ImGui;

    ImGuiContext& g = *GImGui;

    // Call the getter to obtain the preview string which is a parameter to BeginCombo()
    const char* preview_value = NULL;
    if (*current_item >= 0 && *current_item < items_count)
        items_getter(data, *current_item, &preview_value);

    // The old Combo() API exposed "popup_max_height_in_items". The new more general BeginCombo() API doesn't have/need it, but we emulate it here.
    if (popup_max_height_in_items != -1 && !(g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSizeConstraint))
        SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

    if (!ImAdd::BeginCombo(label, preview_value, ImGuiComboFlags_None))
        return false;

    // Display items
    // FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
    bool value_changed = false;
    for (int i = 0; i < items_count; i++)
    {
        PushID(i);
        const bool item_selected = (i == *current_item);
        const char* item_text;
        if (!items_getter(data, i, &item_text))
            item_text = "*Unknown item*";

        PushStyleVar(ImGuiStyleVar_ItemSpacing, g.Style.FramePadding);
        
        // Renderizar o Selectable
        bool item_pressed = ImAdd::Selectable(item_text, item_selected, ImVec2(-0.1, 0));
        
        // Verificar hover após o Selectable ser renderizado
        bool item_hovered = ImGui::IsItemHovered();
        
        // Efeito hover nos itens do combo: vinho profundo (sem rosar)
        if (item_hovered && !item_selected) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImRect item_bb = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

            // Fundo vinho escuro e saturado
            ImVec4 hoverColor = ImCustomColors::Gradient::GetDeep();
            hoverColor.w = 0.60f;
            draw_list->AddRectFilled(item_bb.Min, item_bb.Max, ImGui::GetColorU32(hoverColor), g.Style.FrameRounding);

            // Borda vermelha viva no hover
            ImVec4 hoverBorderColor = ImCustomColors::Gradient::GetSecondary();
            hoverBorderColor.w = 0.85f;
            draw_list->AddRect(item_bb.Min, item_bb.Max, ImGui::GetColorU32(hoverBorderColor), g.Style.FrameRounding, 0, ImCustomStyle::Border::ThicknessHover);
        }
        
        // Item selecionado: destacar com cor laranja mais forte
        if (item_selected) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImRect item_bb = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            
            // Fundo laranja quando selecionado
            ImVec4 selectedColor = ImCustomColors::Gradient::GetPrimary();
            selectedColor.w = ImCustomStyle::Opacity::SelectedCombo;
            draw_list->AddRectFilled(item_bb.Min, item_bb.Max, ImGui::GetColorU32(selectedColor), g.Style.FrameRounding);
            
            // Borda laranja quando selecionado
            ImVec4 selectedBorderColor = ImCustomColors::Gradient::GetPrimary();
            selectedBorderColor.w = ImCustomStyle::Opacity::BorderSelectedCombo;
            draw_list->AddRect(item_bb.Min, item_bb.Max, ImGui::GetColorU32(selectedBorderColor), g.Style.FrameRounding, 0, ImCustomStyle::Border::ThicknessSelected);
        }
        
        if (item_pressed && *current_item != i)
        {
            value_changed = true;
            *current_item = i;
        }
        PopStyleVar();

        if (item_selected)
            SetItemDefaultFocus();
        PopID();
    }

    EndCombo();

    if (value_changed)
        MarkItemEdited(g.LastItemData.ID);

    return value_changed;
}

// Combo box helper allowing to pass an array of strings.
bool ImAdd::Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
{
    using namespace ImGui;

    const bool value_changed = Combo(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_in_items);
    return value_changed;
}

// Combo box helper allowing to pass all items in a single string literal holding multiple zero-terminated items "item1\0item2\0"
bool ImAdd::Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int height_in_items)
{
    using namespace ImGui;

    int items_count = 0;
    const char* p = items_separated_by_zeros;       // FIXME-OPT: Avoid computing this, or at least only when combo is open
    while (*p)
    {
        p += strlen(p) + 1;
        items_count++;
    }
    bool value_changed = Combo(label, current_item, Items_SingleStringGetter, (void*)items_separated_by_zeros, items_count, height_in_items);
    return value_changed;
}

bool ImAdd::InputText(const char* label, const char* text, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImVec2 pos = window->DC.CursorPos;

    char* inv_label;
    inv_label = (char*)malloc(strlen(label) + 3);
    if (inv_label != NULL) {
        strcpy(inv_label, "##");
        strcat(inv_label, label);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(g.Style.ItemSpacing.x, g.Style.ItemInnerSpacing.y));
    ImGui::Text(label);
    IM_ASSERT(!(flags & ImGuiInputTextFlags_Multiline)); // call InputTextMultiline()
    ImGui::PushItemWidth(ImGui::CalcItemSize(ImVec2(-0.1, 0), 0, 0).x);
    auto result = ImGui::InputTextEx(inv_label, NULL, buf, (int)buf_size, ImVec2(0, 0), flags, callback, user_data);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();

    if (!ImGui::IsItemActive() && !strlen(buf))
    {
        window->DrawList->AddText(pos + ImVec2(g.Style.FramePadding.x, ImGui::GetFontSize() + g.Style.ItemInnerSpacing.y + g.Style.FramePadding.y), ImGui::GetColorU32(ImCustomColors::UI::TextDisabled()), text);
    }

    free(inv_label);
    return result;
}

void ImAdd::RenderFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding, ImDrawFlags draw_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding, draw_flags);
    const float border_size = g.Style.FrameBorderSize;
    if (border && border_size > 0.0f)
    {
        window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), ImGui::GetColorU32(ImCustomColors::UI::Border()), rounding, draw_flags, border_size);
        window->DrawList->AddRect(p_min, p_max, ImGui::GetColorU32(ImCustomColors::UI::Border()), rounding, draw_flags, border_size);
    }
}

void ImAdd::TabBox(const char* str_id, int* current_id, std::vector<const char*> list, ImVec2 size_arg)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImCustomColors::UI::Border());
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, style.FrameBorderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, style.FrameRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.FrameRounding);
    ImGui::BeginChild(str_id, size_arg, false);
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        float width = window_size.x / list.size();

        // Desenhar borda externa do container
        draw_list->AddRect(window_pos, window_pos + window_size, ImGui::GetColorU32(ImCustomColors::UI::Border()), style.ChildRounding, 0, style.FrameBorderSize);

        // Efeito pulsante para tabs selecionadas
        static float pulseTime = 0.0f;
        pulseTime += g.IO.DeltaTime * 2.0f; // Velocidade do pulso
        if (pulseTime > 6.28318f) pulseTime = 0.0f; // Reset após 2*PI (ciclo completo)
        float pulse = (sinf(pulseTime) + 1.0f) * 0.5f; // 0.0 a 1.0
        float pulseAlpha = 0.3f + (pulse * 0.2f); // Varia entre 0.3 e 0.5

        for (int i = 0; i < list.size(); ++i)
        {
            ImDrawFlags draw_flags = i == 0 ? ImDrawFlags_RoundCornersLeft : i == list.size() - 1 ? ImDrawFlags_RoundCornersRight : ImDrawFlags_RoundCornersNone;
            bool is_selected = *current_id == i;
            
            // Calcular posição da tab
            ImVec2 tab_pos = window_pos + ImVec2(width * i, 0);
            ImVec2 tab_size = ImVec2(width, size_arg.y);
            ImRect tab_bb(tab_pos, tab_pos + tab_size);

            // Renderizar o Selectable primeiro
            if (ImAdd::Selectable(list[i], is_selected, ImVec2(width, size_arg.y), draw_flags))
            {
                *current_id = i;
            }
            
            // Verificar hover após o Selectable ser renderizado
            bool is_hovered = ImGui::IsItemHovered();
            
            // Quando SELECIONADA: sobrescrever com fundo laranja sólido + efeito pulsante + borda (igual aos botões da sidebar)
            if (is_selected) {
                ImRect item_bb = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
                
                // Fundo laranja sólido (sobrescreve o fundo cinza do Selectable)
                ImVec4 bgColor = ImCustomColors::Gradient::GetPrimary();
                bgColor.w = 1.0f; // Opaco
                draw_list->AddRectFilled(item_bb.Min, item_bb.Max, ImGui::GetColorU32(bgColor), style.FrameRounding, draw_flags);
                
                // Efeito pulsante interno (igual aos botões da sidebar)
                float glowIntensity = ImCustomStyle::Helpers::CalculateGlowIntensity(ImGui::GetTime());
                draw_list->AddRectFilled(
                    ImVec2(item_bb.Min.x - ImCustomStyle::Glow::Offset, item_bb.Min.y - ImCustomStyle::Glow::Offset),
                    ImVec2(item_bb.Max.x + ImCustomStyle::Glow::Offset, item_bb.Max.y + ImCustomStyle::Glow::Offset),
                    ImCustomColors::Basic::GlowWhite((int)(ImCustomStyle::Animation::GlowAlpha * glowIntensity)),
                    style.FrameRounding + ImCustomStyle::Glow::RoundingOffset,
                    draw_flags
                );
                
                // Borda laranja pulsante (igual aos botões da sidebar)
                float pulse = ImCustomStyle::Helpers::CalculatePulse(ImGui::GetTime());
                float pulseAlpha = ImCustomStyle::Helpers::CalculatePulseSelectedAlpha(pulse);
                ImVec4 borderColor = ImCustomColors::Gradient::GetPrimary();
                borderColor.w = pulseAlpha;
                draw_list->AddRect(item_bb.Min, item_bb.Max, ImGui::GetColorU32(borderColor), style.FrameRounding, draw_flags, ImCustomStyle::Border::ThicknessPulse);
                
                // Redesenhar o texto por cima do fundo laranja para garantir visibilidade
                // Calcular tamanho do texto e garantir que não seja cortado
                ImVec2 label_size = ImGui::CalcTextSize(list[i], NULL, true);
                
                // Padding horizontal generoso para evitar corte do ícone
                float padding_x = ImCustomStyle::Padding::GetIconPaddingTab(style);
                float available_width = item_bb.GetWidth() - (padding_x * 2.0f);
                
                // Calcular posição centralizada do texto
                ImVec2 text_pos = ImVec2(
                    item_bb.GetCenter().x - (label_size.x * 0.5f),
                    item_bb.GetCenter().y - (label_size.y * 0.5f)
                );
                
                // Garantir que o texto não saia dos limites (com padding)
                if (text_pos.x < item_bb.Min.x + padding_x) {
                    text_pos.x = item_bb.Min.x + padding_x;
                }
                if (text_pos.x + label_size.x > item_bb.Max.x - padding_x) {
                    text_pos.x = item_bb.Max.x - padding_x - label_size.x;
                }
                
                // Aplicar clipping para garantir que o texto não ultrapasse os limites
                ImRect clip_rect(
                    item_bb.Min.x + padding_x,
                    item_bb.Min.y,
                    item_bb.Max.x - padding_x,
                    item_bb.Max.y
                );
                
                draw_list->PushClipRect(clip_rect.Min, clip_rect.Max, true);
                draw_list->AddText(text_pos, ImGui::GetColorU32(ImCustomColors::UI::Text()), list[i]);
                draw_list->PopClipRect();
            }
            // Quando HOVER (mas não selecionada): vinho profundo + texto branco
            else if (is_hovered) {
                ImRect item_bb = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

                // Fundo vinho escuro e saturado (sem rosar)
                ImVec4 hoverColor = ImCustomColors::Gradient::GetDeep();
                hoverColor.w = 0.65f;
                draw_list->AddRectFilled(item_bb.Min, item_bb.Max, ImGui::GetColorU32(hoverColor), style.FrameRounding, draw_flags);

                // Borda vermelha viva
                ImVec4 hoverBorderColor = ImCustomColors::Gradient::GetSecondary();
                hoverBorderColor.w = 0.85f;
                draw_list->AddRect(item_bb.Min, item_bb.Max, ImGui::GetColorU32(hoverBorderColor), style.FrameRounding, draw_flags, ImCustomStyle::Border::ThicknessHover);

                // Texto branco por cima (o Selectable desenharia apagado)
                ImVec2 label_size = ImGui::CalcTextSize(list[i], NULL, true);
                float padding_x = ImCustomStyle::Padding::GetIconPaddingTab(style);
                ImVec2 text_pos = ImVec2(
                    item_bb.GetCenter().x - (label_size.x * 0.5f),
                    item_bb.GetCenter().y - (label_size.y * 0.5f)
                );
                if (text_pos.x < item_bb.Min.x + padding_x) {
                    text_pos.x = item_bb.Min.x + padding_x;
                }
                if (text_pos.x + label_size.x > item_bb.Max.x - padding_x) {
                    text_pos.x = item_bb.Max.x - padding_x - label_size.x;
                }
                ImRect clip_rect(
                    item_bb.Min.x + padding_x,
                    item_bb.Min.y,
                    item_bb.Max.x - padding_x,
                    item_bb.Max.y
                );
                draw_list->PushClipRect(clip_rect.Min, clip_rect.Max, true);
                draw_list->AddText(text_pos, ImGui::GetColorU32(ImCustomColors::UI::Text()), list[i]);
                draw_list->PopClipRect();
            }
            // Aba normal (não selecionada, sem hover): texto cinza claro
            // (o Selectable desenha apagado demais e parece véu por cima)
            else {
                ImRect item_bb = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
                ImVec2 label_size = ImGui::CalcTextSize(list[i], NULL, true);
                float padding_x = ImCustomStyle::Padding::GetIconPaddingTab(style);
                ImVec2 text_pos = ImVec2(
                    item_bb.GetCenter().x - (label_size.x * 0.5f),
                    item_bb.GetCenter().y - (label_size.y * 0.5f)
                );
                if (text_pos.x < item_bb.Min.x + padding_x) {
                    text_pos.x = item_bb.Min.x + padding_x;
                }
                if (text_pos.x + label_size.x > item_bb.Max.x - padding_x) {
                    text_pos.x = item_bb.Max.x - padding_x - label_size.x;
                }
                ImRect clip_rect(
                    item_bb.Min.x + padding_x,
                    item_bb.Min.y,
                    item_bb.Max.x - padding_x,
                    item_bb.Max.y
                );
                draw_list->PushClipRect(clip_rect.Min, clip_rect.Max, true);
                draw_list->AddText(text_pos, ImGui::GetColorU32(ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), list[i]);
                draw_list->PopClipRect();
            }
            
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(5);
    ImGui::PopStyleColor();
    ImGui::Spacing();
}