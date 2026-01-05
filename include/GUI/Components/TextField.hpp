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
        Fix value_ = 0;
        Fix min_ = -1000000;
        Fix max_ = 1000000;
        Fix step_ = 1;
        Fix big_step_ = 10;

        bool step_flip_mode_ = false;
        bool pass_word_mode_ = false;
        void* var_ptr_ = nullptr;

        int32_t fractional_digits_ = 2;
        bool value_changed_ = false;

        float beam_width_ = 2.0f;

        int32_t cursor_index_ = 0;
        int32_t selection_begin_ = -1, selection_end_ = -1;
        int32_t selection_drag_start_ = -1;

        float text_x_offset_ = 0;
        float text_min_x_offset_ = 0;
        float text_max_x_offset_ = 0;
        Rectd content_rect_;
        bool cursor_must_be_visible_ = true;

        char* info_text_ = nullptr;
        bool show_loupe_ = false;    // TODO: For search fields
        bool show_loupe_if_empty_ = false;

        Alignment text_alignment_ = Alignment::Left;
        float padding_[4]{};

    public:
        explicit TextField(const Rectd& rect) noexcept;
        ~TextField() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "TextField"; }


        static TextField* add(View* view, const Rectd& rect, const char* text = nullptr, int32_t tag = 0);
        static TextField* add(View* view, const Rectd& rect, Component* receiver, ComponentAction action, int32_t tag, void* action_ref);


        void draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept override;
        void drawCursor(GraphicContext* gc, float x) const noexcept;

        [[nodiscard]] virtual float beamWidth() const noexcept{ return beam_width_; }

        [[nodiscard]] virtual int32_t cursorIndex() const noexcept { return cursor_index_; }
        [[nodiscard]] virtual int32_t clampedCursorIndex() const noexcept { return std::clamp<int32_t>(cursor_index_, 0, textLength()); }
        [[nodiscard]] virtual int32_t selectionBegin() const noexcept { return selection_begin_; }
        [[nodiscard]] virtual int32_t selectionEnd() const noexcept { return selection_end_; }
        [[nodiscard]] virtual int32_t selectionLength() const noexcept { return hasSelection() ? selection_end_ - selection_begin_ + 1 : 0; }
        [[nodiscard]] virtual int32_t selectionDragStart() const noexcept{ return selection_drag_start_; }

        [[nodiscard]] virtual bool stepFlipMode() const noexcept { return step_flip_mode_; }
        [[nodiscard]] virtual bool isPasswordMode() const noexcept { return pass_word_mode_; }


        bool setEnabled(bool enabled) noexcept override;
        void setNumberMode(bool mode) noexcept override;
        void stepNumber(bool use_big_step, bool negative) noexcept override;

        virtual void setBeamWidth(float beam_width) noexcept { beam_width_ = beam_width; needsDisplay(); }

        virtual void setCursorIndex(int32_t cursor_index) noexcept { cursor_index_ = cursor_index; needsDisplay(); }
        virtual void setSelectionBegin(int32_t selection_begin) noexcept { selection_begin_ = selection_begin; needsDisplay(); ; }
        virtual void setSelectionEnd(int32_t selection_end) noexcept { selection_end_ = selection_end; needsDisplay(); }
        virtual void setSelectionDragStart(int32_t selection_drag_start) noexcept { selection_drag_start_ = selection_drag_start; }

        virtual bool setSelection(int32_t begin, int32_t end) noexcept;
        virtual bool setCursor(int32_t cursor_index) noexcept { return setCursor(cursor_index, false); }
        virtual bool setCursor(int32_t cursor_index, bool selection_mode) noexcept;
        virtual bool moveCursor(int32_t offset, bool shift_pressed) noexcept;
        virtual bool setCursorToEnd() noexcept { return setCursor(textLength()); }
        virtual bool selectAll() noexcept { return setSelection(0, textLength()); }
        virtual bool selectWordAtCursor() noexcept;
        virtual bool removeSelection() noexcept;
        [[nodiscard]] virtual bool hasSelection() const noexcept { return selection_begin_ >= 0 && selection_end_ > selection_begin_; }

        virtual bool removeCharAheadOfCursor() noexcept;
        virtual bool removeSelectedText() noexcept;
        void insertText(const char* text) noexcept override;
        virtual void updateEdit() noexcept;


        void setReceiverComponent(Component* receiver) noexcept override {
            receiver_component_ = receiver;
            if (receiver_component_) {
                receiver_component_->setTextField(this);
            }
        }


        void setFractionalDigits(int32_t fractional_digits) noexcept {
            fractional_digits_ = std::clamp<int32_t>(fractional_digits, 0, Fix::kDecPrecision);
        }

        Fix value() const noexcept override { return value_; }
        int32_t valueAsInt32() const noexcept override { return value_.asInt32(); }
        double valueAsDouble() const noexcept override { return value_.asDouble(); }

        bool setValue(const Fix& value) noexcept override {
            bool result = value_.set(value, min_, max_, fractional_digits_);
            char buffer[100];
            value_.toStr(buffer, 100, Fix::kDecPrecision);
            setText(buffer);
            needsDisplay();
            return result;
        }

        bool setValueInt(int32_t value) noexcept override {
            return setValue(Fix(value));
        }

        void setValueRange(const Fix& min, const Fix& max) noexcept override {
            setNumberMode(true);
            min_ = min;
            max_ = max;
        }

        virtual void setValueRangeInt32(int32_t min, int32_t max) noexcept {
            setNumberMode(true);
            min_.setInt32(min);
            max_.setInt32(max);
        }

        virtual void setValueRangeReal64(double min, double max) noexcept {
            setNumberMode(true);
            min_.setDouble(min);
            max_.setDouble(max);
        }

        virtual void setStep(Fix step, Fix big_step) noexcept {
            setNumberMode(true);
            step_ = step;
            big_step_ = big_step;
        }

        virtual void setStepReal64(double step, double big_step) noexcept {
            setNumberMode(true);
            step_.setDouble(step);
            big_step_.setDouble(big_step);
        }

        void setStepFlipMode(bool step_flip_mode) noexcept { step_flip_mode_ = step_flip_mode; }
        void setPasswordMode(bool password_mode) noexcept { pass_word_mode_ = password_mode; needsDisplay(); }
        void enablePasswordMode() noexcept { setPasswordMode(true); }
        void disablePasswordMode() noexcept { setPasswordMode(false); }


        void setInfoText(const char* text) noexcept {
            std::free(info_text_);
            info_text_ = text ? strdup(text) : nullptr;
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
