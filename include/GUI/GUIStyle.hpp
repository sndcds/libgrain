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

        [[nodiscard]] float opacity() const { return opacity_; }
        [[nodiscard]] float disabledAlpha() const { return 0.5f; }

        [[nodiscard]] const RGBA& viewColor() const { return view_color_; }

        [[nodiscard]] const RGBA& backgroundColor(State state = kNormal) const {
            return bg_color_[_sanitizedState(state)];
        }
        [[nodiscard]] const RGBA& foregroundColor(State state = kNormal) const {
            return fg_color_[_sanitizedState(state)];
        }
        [[nodiscard]] const RGBA& borderColor(State state = kNormal) const {
            return border_color_[_sanitizedState(state)];
        }

        [[nodiscard]] const RGBA& labelColor() const { return label_color_; }

        [[nodiscard]] float padding(int32_t edge_index) const {
            if (edge_index < 0 || edge_index > 3) {
                edge_index = 0;
            }
            return padding_[edge_index];
        }
        [[nodiscard]] float paddingTop() const { return padding_[0]; }
        [[nodiscard]] float paddingRight() const { return padding_[1]; }
        [[nodiscard]] float paddingBottom() const { return padding_[2]; }
        [[nodiscard]] float paddingLeft() const { return padding_[3]; }
        [[nodiscard]] CornerRadiusMode cornerRadiusMode() const { return corner_radius_mode_; }
        [[nodiscard]] float cornerRadius(int32_t corner_index) const {
            return corner_index >= 0 && corner_index < 4 ? corner_radius_[corner_index] : 0.0f;
        }
        [[nodiscard]] float checkboxRadius() const { return checkbox_radius_; }
        [[nodiscard]] float checkboxSize() const { return checkbox_size_; }
        [[nodiscard]] Alignment textAlignment() const { return text_alignment_; }
        [[nodiscard]] float textYOffset() const { return text_y_offset_; }
        [[nodiscard]] Font* font() const { return font_; }

        [[nodiscard]] RGBA textColor() const noexcept { return text_color_; }
        [[nodiscard]] RGBA textBackgroundColor() const noexcept { return text_bg_color_; }
        [[nodiscard]] RGBA textSelectionColor() const noexcept { return text_selection_color_; }
        [[nodiscard]] RGBA textSelectionBackgroundColor() const noexcept { return text_selection_bg_color_; }
        [[nodiscard]] RGBA textInfoColor() const noexcept { return text_info_color_; }
        [[nodiscard]] RGBA textCursorColor() const noexcept { return text_cursor_color_; }
        [[nodiscard]] RGBA scrollBarHandleColor() const { return scrollbar_handle_color_; }
        [[nodiscard]] RGBA controllerTrackColor() const { return controller_track_color_; }
        [[nodiscard]] RGBA controllerHandleColor() const { return controller_handle_color_; }

        [[nodiscard]] float labelGap() const { return label_gap_; }
        [[nodiscard]] float scrollBarPadding() const { return 3.0f; }


        void setViewColor(const RGBA& color) { view_color_ = color; }
        void setBackgroundColor(State state, const RGBA& color) { bg_color_[_sanitizedState(state)] = color; }
        void setForegroundColor(State state, const RGBA& color) { fg_color_[_sanitizedState(state)] = color; }
        void setBorderColor(State state, const RGBA& color) { border_color_[_sanitizedState(state)] = color; }
        void setLabelColor(const RGBA& color) { label_color_ = color; }

        void setPadding(float padding) {
            padding_[0] = padding_[1] = padding_[2] = padding_[3] = padding;
        }

        void setPadding(float horizontal, float vertical) {
            padding_[0] = padding_[2] = vertical;
            padding_[1] = padding_[3] = horizontal;
        }

        void setPadding(float top, float right, float bottom, float left) {
            padding_[0] = left;
            padding_[1] = top;
            padding_[2] = right;
            padding_[3] = bottom;
        }

        void setCornerRadius(float r) {
            corner_radius_[0] = corner_radius_[1] = corner_radius_[2] = corner_radius_[3] = r;
        }

        void setCornerRadius(float top_right, float bottom_right, float bottom_left, float top_left) {
            corner_radius_[0] = top_right;
            corner_radius_[1] = bottom_right;
            corner_radius_[2] = bottom_left;
            corner_radius_[3] = top_left;
            if (top_left < 0.0f && top_right <= 0.0f && bottom_right <= 0.0f && bottom_left <= 0.0f) {
                corner_radius_mode_ = CornerRadiusMode::No;
            }
            else if (top_left == top_right && top_left == bottom_right && top_left == bottom_left) {
                corner_radius_mode_ = CornerRadiusMode::Same;
            }
            else {
                corner_radius_mode_ = CornerRadiusMode::Different;
            }
        }

        void setCheckboxSize(float size) { checkbox_size_ = size; }
        void setCheckboxRadius(float r) { checkbox_radius_ = r; }


        void setTextAlignment(Alignment alignment) noexcept { text_alignment_ = alignment; }
        void textYOffset(float text_y_offset) noexcept { text_y_offset_ = text_y_offset; }
        void setFont(Font* font) noexcept { font_ = font; }
        void setTextColor(const RGBA& color) noexcept { text_color_ = color; }
        void setTextBackgroundColor(const RGBA& color) noexcept { text_bg_color_ = color; }
        void setTextSelectionColor(const RGBA& color) noexcept { text_selection_color_ = color; }
        void setTextSelectionBackgroundColor(const RGBA& color) noexcept { text_selection_bg_color_ = color; }
        void setTextInfoColor(const RGBA& color) noexcept { text_info_color_ = color; }
        void setTextCursorColor(const RGBA& color) noexcept { text_cursor_color_ = color; }


        void setLabelGap(float label_gap) { label_gap_ = label_gap; }


        [[nodiscard]] static inline int32_t _sanitizedState(State state) {
            return static_cast<int32_t>(state);
        }

    protected:
        float opacity_ = 1.0f;

        RGBA view_color_{};

        RGBA bg_color_[kStateCount]{};
        RGBA fg_color_[kStateCount]{};

        RGBA border_color_[kStateCount]{};
        float border_width_[kStateCount] = { 1.0f, 1.0f, 1.0f };

        float corner_radius_[4] = { 5.0f, 5.0f, 5.0f, 5.0f };
        CornerRadiusMode corner_radius_mode_ = CornerRadiusMode::No; ///< Not to be set by user

        float padding_[4] = { 4.0f, 12.0f, 4.0f, 12.0f };

        Alignment text_alignment_ = Alignment::Center; ///< Text alignment inside the component
        float text_y_offset_ = 0.0f;                   ///< Vertical text offset
        Font* font_ = nullptr;                         ///< Fallback font
        RGBA text_color_ = { 1, 0, 0, 1 };
        RGBA text_bg_color_ = { 1, 0.6f, 0, 1 };
        RGBA text_selection_color_ = { 0.6f, 1, 0, 1 };
        RGBA text_selection_bg_color_ = { 0, 1, 0.6f, 1 };
        RGBA text_cursor_color_ = { 0.6f, 0, 1, 1 };
        RGBA scrollbar_handle_color_ = { 0.6, 0.6, 0.6, 1 };
        RGBA text_info_color_;

        RGBA controller_track_color_ = { 0.6, 0.6, 0.6, 1 };
        RGBA controller_handle_color_ = { 0.3, 0.3, 0.3, 1 };

        RGBA label_color_{};
        float label_gap_ = 12.0f; ///< Gap between element and label

        float checkbox_radius_ = 3.0f;
        float checkbox_size_ = 18.0f;
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
            style_list_.push(style);
            return style_list_.lastIndex();
        }

        [[nodiscard]] GUIStyle* styleAtIndex(int32_t index) {
            return style_list_.elementAtIndex(index);
        }

    protected:
        ObjectList<GUIStyle*>style_list_;
    };


} // End of namespace Grain

#endif // GrainGUIStyle_hpp