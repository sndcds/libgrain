//
//  TextField.hpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 18.08.2025
//

#ifndef GrainTextField_hpp
#define GrainTextField_hpp

#include "Grain.hpp"
#include "Component.hpp"
#include "Type/Type.hpp"
#include "Type/Fix.hpp"


namespace Grain {

    class TextField : public Component {

    protected:
        Fix m_value = 0;
        Fix m_min = -1000000;
        Fix m_max = 1000000;
        Fix m_step = 1;
        Fix m_big_step = 10;

        bool m_step_flip_mode = false;
        bool m_pass_word_mode = false;
        void* m_var_ptr = nullptr;

        int32_t m_fractional_digits = 2;
        bool m_value_changed = false;

        float m_beam_width = 2;

        int32_t m_cursor_index = 0;
        int32_t m_selection_begin = -1, m_selection_end = -1;
        int32_t m_selection_drag_start = -1;

        float m_text_x_offset = 0;
        float m_text_min_x_offset = 0;
        float m_text_max_x_offset = 0;
        Rectd m_content_rect;
        bool m_cursor_must_be_visible = true;

        char* m_info_text = nullptr;
        bool m_show_loupe = false;    // TODO: For search fields
        bool m_show_loupe_if_empty = false;

        Alignment m_text_alignment = Alignment::Left;
        float m_padding[4];

    public:
        TextField(const Rectd& rect) noexcept;
        virtual ~TextField() noexcept;

        const char* className() const noexcept override { return "TextField"; }


        static TextField* add(View* view, const Rectd& rect, const char* text = nullptr, int32_t tag = 0);
        static TextField* add(View* view, const Rectd& rect, Component* receiver, ComponentAction action, int32_t tag, void* action_ref);


        void draw(const Rectd& dirty_rect) noexcept override;
        void drawCursor(GraphicContext* gc, float x) const noexcept;

        virtual float beamWidth() const noexcept { return m_beam_width; }

        virtual int32_t cursorIndex() const noexcept { return m_cursor_index; }
        virtual int32_t clampedCursorIndex() const noexcept { return std::clamp<int32_t>(m_cursor_index, 0, textLength()); }
        virtual int32_t selectionBegin() const noexcept { return m_selection_begin; }
        virtual int32_t selectionEnd() const noexcept { return m_selection_end; }
        virtual int32_t selectionLength() const noexcept { return hasSelection() ? m_selection_end - m_selection_begin + 1 : 0; }
        virtual int32_t selectionDragStart() const noexcept{ return m_selection_drag_start; }

        virtual bool stepFlipMode() const noexcept { return m_step_flip_mode; }
        virtual bool isPasswordMode() const noexcept { return m_pass_word_mode; }


        bool setEnabled(bool enabled) noexcept override;
        void setNumberMode(bool mode) noexcept override;
        void stepNumber(bool use_big_step, bool negative) noexcept override;

        virtual void setBeamWidth(float beam_width) noexcept { m_beam_width = beam_width; needsDisplay(); }

        virtual void setCursorIndex(int32_t cursor_index) noexcept { m_cursor_index = cursor_index; needsDisplay(); }
        virtual void setSelectionBegin(int32_t selection_begin) noexcept { m_selection_begin = selection_begin; needsDisplay(); ; }
        virtual void setSelectionEnd(int32_t selection_end) noexcept { m_selection_end = selection_end; needsDisplay(); }
        virtual void setSelectionDragStart(int32_t selection_drag_start) noexcept { m_selection_drag_start = selection_drag_start; }

        virtual bool setSelection(int32_t begin, int32_t end) noexcept;
        virtual bool setCursor(int32_t cursor_index) noexcept { return setCursor(cursor_index, false); }
        virtual bool setCursor(int32_t cursor_index, bool selection_mode) noexcept;
        virtual bool moveCursor(int32_t offset, bool shift_pressed) noexcept;
        virtual bool setCursorToEnd() noexcept { return setCursor(textLength()); }
        virtual bool selectAll() noexcept { return setSelection(0, textLength()); }
        virtual bool selectWordAtCursor() noexcept;
        virtual bool removeSelection() noexcept;
        virtual bool hasSelection() const noexcept { return m_selection_begin >= 0 && m_selection_end > m_selection_begin; }

        virtual bool removeCharAheadOfCursor() noexcept;
        virtual bool removeSelectedText() noexcept;
        void insertText(const char* text) noexcept override;
        virtual void updateEdit() noexcept;


        void setReceiverComponent(Component* receiver) noexcept override {
            m_receiver_component = receiver;
            if (m_receiver_component) {
                m_receiver_component->setTextField(this);
            }
        }


        void setFractionalDigits(int32_t fractional_digits) noexcept {
            m_fractional_digits = std::clamp<int32_t>(fractional_digits, 0, Fix::kDecPrecision);
        }

        Fix value() const noexcept override { return m_value; }
        int32_t valueAsInt32() const noexcept override { return m_value.asInt32(); }
        double valueAsDouble() const noexcept override { return m_value.asDouble(); }

        bool setValue(const Fix& value) noexcept override {
            bool result = m_value.set(value, m_min, m_max, m_fractional_digits);
            char buffer[100];
            m_value.toStr(buffer, 100, Fix::kDecPrecision);
            setText(buffer);
            needsDisplay();
            return result;
        }

        bool setValueInt(int32_t value) noexcept override {
            return setValue(Fix(value));
        }

        void setValueRange(const Fix& min, const Fix& max) noexcept override {
            setNumberMode(true);
            m_min = min;
            m_max = max;
        }

        virtual void setValueRangeInt32(int32_t min, int32_t max) noexcept {
            setNumberMode(true);
            m_min.setInt32(min);
            m_max.setInt32(max);
        }

        virtual void setValueRangeReal64(double min, double max) noexcept {
            setNumberMode(true);
            m_min.setDouble(min);
            m_max.setDouble(max);
        }

        virtual void setStep(Fix step, Fix big_step) noexcept {
            setNumberMode(true);
            m_step = step;
            m_big_step = big_step;
        }

        virtual void setStepReal64(double step, double big_step) noexcept {
            setNumberMode(true);
            m_step.setDouble(step);
            m_big_step.setDouble(big_step);
        }

        void setStepFlipMode(bool step_flip_mode) noexcept { m_step_flip_mode = step_flip_mode; }
        void setPasswordMode(bool password_mode) noexcept { m_pass_word_mode = password_mode; needsDisplay(); }
        void enablePasswordMode() noexcept { setPasswordMode(true); }
        void disablePasswordMode() noexcept { setPasswordMode(false); }


        void setInfoText(const char* text) noexcept {
            std::free(m_info_text);
            m_info_text = text ? strdup(text) : nullptr;
            needsDisplay();
        }

        void handleMouseDown(const Event& event) noexcept override;
        void handleMouseDrag(const Event& event) noexcept override;
        void handleMouseUp(const Event& event) noexcept override;
        void handleScrollWheel(const Event& event) noexcept override;
        void handleKeyDown(const Event& event) noexcept override;

        void becomeFirstResponder() noexcept override;
        void resignFirstResponder() noexcept override;

        int32_t cursorIndexAtPos(Vec2d pos) noexcept;
        void _checkSelectionAndCursor() noexcept;

        int32_t copyToPasteboard() noexcept;
        int32_t pasteFromPasteboard() noexcept;
    };


} // End of namespace Grain

#endif // GrainTextField_hpp
