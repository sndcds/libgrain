//
//  GUIStyle.hpp
//
//  Created by Roald Christesen on from 28.07.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 28.07.2025
//

#ifndef GrainGUIStyle_hpp
#define GrainGUIStyle_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Geometry.hpp"
#include "Color/RGBA.hpp"
#include "2d/RectEdges.hpp"


namespace Grain {

    class Font;

    class GUIStyle : public Object {
    public:
        enum State {
            kNormal = 0,
            kHighlighted = 1,
            kActive = 2
        };
        static constexpr int32_t kStateCount = 3;

        enum class CornerRadiusMode {
            No = 0,
            Same = 1,
            Different = 2,
        };

    public:
        GUIStyle();

        [[nodiscard]] float opacity() const { return m_opacity; }
        [[nodiscard]] float disabledAlpha() const { return 0.5f; }

        [[nodiscard]] const RGBA& viewColor() const { return m_view_color; }

        [[nodiscard]] const RGBA& backgroundColor(State state = kNormal) const {
            return m_background_color[_sanitizedState(state)];
        }
        [[nodiscard]] const RGBA& foregroundColor(State state = kNormal) const {
            return m_foreground_color[_sanitizedState(state)];
        }
        [[nodiscard]] const RGBA& borderColor(State state = kNormal) const {
            return m_border_color[_sanitizedState(state)];
        }

        [[nodiscard]] const RGBA& labelColor() const { return m_label_color; }

        [[nodiscard]] float padding(int32_t edge_index) const {
            if (edge_index < 0 || edge_index > 3) {
                edge_index = 0;
            }
            return m_padding[edge_index];
        }
        [[nodiscard]] float paddingTop() const { return m_padding[0]; }
        [[nodiscard]] float paddingRight() const { return m_padding[1]; }
        [[nodiscard]] float paddingBottom() const { return m_padding[2]; }
        [[nodiscard]] float paddingLeft() const { return m_padding[3]; }
        [[nodiscard]] CornerRadiusMode cornerRadiusMode() const { return _m_corner_radius_mode; }
        [[nodiscard]] float cornerRadius(int32_t corner_index) const {
            return corner_index >= 0 && corner_index < 4 ? m_corner_radius[corner_index] : 0.0f;
        }
        [[nodiscard]] float checkboxRadius() const { return m_checkbox_radius; }
        [[nodiscard]] float checkboxSize() const { return m_checkbox_size; }
        [[nodiscard]] Alignment textAlignment() const { return m_text_alignment; }
        [[nodiscard]] float textYOffset() const { return m_text_y_offset; }
        [[nodiscard]] Font* font() const { return m_font; }

        [[nodiscard]] RGBA textColor() const noexcept { return m_text_color; }
        [[nodiscard]] RGBA textBackgroundColor() const noexcept { return m_text_background_color; }
        [[nodiscard]] RGBA textSelectionColor() const noexcept { return m_text_selection_color; }
        [[nodiscard]] RGBA textSelectionBackgroundColor() const noexcept { return m_text_selection_background_color; }
        [[nodiscard]] RGBA textInfoColor() const noexcept { return m_text_info_color; }
        [[nodiscard]] RGBA textCursorColor() const noexcept { return m_text_cursor_color; }
        [[nodiscard]] RGBA scrollBarColor() const { return { 1, 0, 0, 1 }; }

        [[nodiscard]] float labelGap() const { return m_label_gap; }
        [[nodiscard]] float scrollBarPadding() const { return 3.0f; }


        void setViewColor(const RGBA& color) { m_view_color = color; }
        void setBackgroundColor(State state, const RGBA& color) { m_background_color[_sanitizedState(state)] = color; }
        void setForegroundColor(State state, const RGBA& color) { m_foreground_color[_sanitizedState(state)] = color; }
        void setBorderColor(State state, const RGBA& color) { m_border_color[_sanitizedState(state)] = color; }
        void setLabelColor(const RGBA& color) { m_label_color = color; }

        void setPadding(float padding) {
            m_padding[0] = m_padding[1] = m_padding[2] = m_padding[3] = padding;
        }

        void setPadding(float horizontal, float vertical) {
            m_padding[0] = m_padding[2] = vertical;
            m_padding[1] = m_padding[3] = horizontal;
        }

        void setPadding(float top, float right, float bottom, float left) {
            m_padding[0] = left;
            m_padding[1] = top;
            m_padding[2] = right;
            m_padding[3] = bottom;
        }

        void setCornerRadius(float r) {
            m_corner_radius[0] = m_corner_radius[1] = m_corner_radius[2] = m_corner_radius[3] = r;
        }

        void setCornerRadius(float top_right, float bottom_right, float bottom_left, float top_left) {
            m_corner_radius[0] = top_right;
            m_corner_radius[1] = bottom_right;
            m_corner_radius[2] = bottom_left;
            m_corner_radius[3] = top_left;
            if (top_left < 0.0f && top_right <= 0.0f && bottom_right <= 0.0f && bottom_left <= 0.0f) {
                _m_corner_radius_mode = CornerRadiusMode::No;
            }
            else if (top_left == top_right && top_left == bottom_right && top_left == bottom_left) {
                _m_corner_radius_mode = CornerRadiusMode::Same;
            }
            else {
                _m_corner_radius_mode = CornerRadiusMode::Different;
            }
        }

        void setCheckboxSize(float size) { m_checkbox_size = size; }
        void setCheckboxRadius(float r) { m_checkbox_radius = r; }


        void setTextAlignment(Alignment alignment) noexcept { m_text_alignment = alignment; }
        void textYOffset(float text_y_offset) noexcept { m_text_y_offset = text_y_offset; }
        void setFont(Font* font) noexcept { m_font = font; }
        void setTextColor(const RGBA& color) noexcept { m_text_color = color; }
        void setTextBackgroundColor(const RGBA& color) noexcept { m_text_background_color = color; }
        void setTextSelectionColor(const RGBA& color) noexcept { m_text_selection_color = color; }
        void setTextSelectionBackgroundColor(const RGBA& color) noexcept { m_text_selection_background_color = color; }
        void setTextInfoColor(const RGBA& color) noexcept { m_text_info_color = color; }
        void setTextCursorColor(const RGBA& color) noexcept { m_text_cursor_color = color; }


        void setLabelGap(float label_gap) { m_label_gap = label_gap; }


        [[nodiscard]] static inline int32_t _sanitizedState(State state) {
            return static_cast<int32_t>(state);
        }

    protected:
        float m_opacity = 1.0f;

        RGBA m_view_color{};

        RGBA m_background_color[kStateCount]{};
        RGBA m_foreground_color[kStateCount]{};

        RGBA m_border_color[kStateCount]{};
        float m_border_width[kStateCount] = { 1.0f, 1.0f, 1.0f };

        float m_corner_radius[4] = { 5.0f, 5.0f, 5.0f, 5.0f };
        CornerRadiusMode _m_corner_radius_mode = CornerRadiusMode::No; ///< Not to be set by user

        float m_padding[4] = { 4.0f, 12.0f, 4.0f, 12.0f };

        Alignment m_text_alignment = Alignment::Center; ///< Text alignment inside the component
        float m_text_y_offset = 0.0f;                   ///< Vertical text offset
        Font* m_font = nullptr;                         ///< Fallback font
        RGBA m_text_color = { 1, 0, 0, 1 };
        RGBA m_text_background_color = { 1, 0.6f, 0, 1 };
        RGBA m_text_selection_color = { 0.6f, 1, 0, 1 };;
        RGBA m_text_selection_background_color = { 0, 1, 0.6f, 1 };
        RGBA m_text_cursor_color = { 0.6f, 0, 1, 1 };
        RGBA m_text_info_color;

        RGBA m_label_color{};
        float m_label_gap = 12.0f; ///< Gap between element and label

        float m_checkbox_radius = 3.0f;
        float m_checkbox_size = 18.0f;
    };

    class GUIStyleSet {
    public:
        GUIStyleSet() = default;
        ~GUIStyleSet() = default;

        int32_t addStyle() {
            auto style = new (std::nothrow) GUIStyle();
            if (!style) {
                return -1;
            }
            m_style_list.push(style);
            return m_style_list.lastIndex();
        }

        [[nodiscard]] GUIStyle* styleAtIndex(int32_t index) {
            return m_style_list.elementAtIndex(index);
        }

    protected:
        ObjectList<GUIStyle*>m_style_list;
    };


} // End of namespace Grain

#endif // GrainGUIStyle_hpp