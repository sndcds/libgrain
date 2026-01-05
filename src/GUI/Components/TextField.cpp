//
//  TextField.hpp
//
//  Created by Roald Christesen on from 02.05.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "GUI/Components/TextField.hpp"
#include "GUI/Views/View.hpp"
#include "GUI/Event.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Graphic/Font.hpp"
#include "2d/Rect.hpp"
#include "App/App.hpp"

#if defined(__APPLE__) && defined(__MACH__)
#include "Graphic/AppleCGContext.hpp"
#endif


namespace Grain {

    TextField::TextField(const Rectd& rect) noexcept : Component(rect, 0) {
        type_ = ComponentType::TextField;
        can_get_focus_ = true;
        text_alignment_ = Alignment::Left;
        is_editable_ = true;
        is_number_mode_ = false;

        setText("0");
        // setMouseCursor(App::MouseCursor::IBeam); TODO: !!!!
    }


    TextField::~TextField() noexcept {
        std::free(info_text_);
    }


    TextField* TextField::add(View* view, const Rectd& rect, const char* text, int32_t tag) {
        auto textfield = (TextField*)Component::addComponentToView((Component*)new (std::nothrow) TextField(rect), view, AddFlags::kNone);

        if (textfield) {
            textfield->setText(text);
            textfield->setTag(tag);
        }

        return textfield;
    }


    TextField* TextField::add(View* view, const Rectd& rect, Component* receiver, ComponentAction action, int32_t tag, void* action_ref) {
        TextField* textfield = nullptr;

        if (view) {
            textfield = new (std::nothrow) TextField(rect);

            if (textfield) {
                textfield->setTag(tag);

                if (receiver) {
                    receiver->setTextField(textfield);
                }
                textfield->setAction(action, action_ref);
                view->addComponent(textfield, AddFlags::kNone);
            }
        }

        return textfield;
    }


#if defined(__APPLE__) && defined(__MACH__)
    void TextField::draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
        auto agc = dynamic_cast<AppleCGContext*>(gc);

        gc->disableFontSubpixelQuantization();

        auto style= guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        CGContextRef cg_context = agc->cgContext();

        _checkSelectionAndCursor();

        content_rect_ = contentRect();
        double padding_left = style->paddingLeft();
        // double padding_top = style->paddingTop();
        double padding_right = style->paddingRight();
        // double padding_bottom = style->paddingBottom();

        auto view_color = style->viewColor();
        auto text_color = style->textColor();
        auto background_color = style->textBackgroundColor();
        auto selection_color = style->textSelectionColor();
        auto selection_background_color = style->textSelectionBackgroundColor();

        auto disabled_alpha = style->disabledAlpha();

        bool focused = isKeyComponent();

        if (!is_enabled_) {
            text_color.setBlend(background_color, 0.5f);
        }

        gc->setFillRGBA(background_color);
        gc->fillRect(boundsRect());

        int32_t text_length = textLength();

        if (isPasswordMode()) {
            // TODO: Implement passwort rendering with circles
        }

        if (text_length > 0) {
            gc->setTextMatrix(1, 0, 0, -1, 0, 0);
            float content_width = content_rect_.width();
            // RGBA fg_selected_color = style->textSelectionColor();

            CGColorRef cg_text_color1 = text_color.createCGColor();
            CGColorRef cg_text_color2 = selection_color.createCGColor();
            CFStringRef cf_str = CFStringCreateWithCString(NULL, text_->utf8(), kCFStringEncodingUTF8);

            CFMutableAttributedStringRef cf_attr_str = CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);
            CFAttributedStringReplaceString(cf_attr_str, CFRangeMake(0, 0), cf_str);
            CFRange range = CFRangeMake(0, text_length);

            if (range.length > 0) {
                CFAttributedStringSetAttribute(cf_attr_str, range, kCTFontAttributeName, style->font()->ctFont());
                CFAttributedStringSetAttribute(cf_attr_str, range, kCTForegroundColorAttributeName, cg_text_color1);
            }

            if (focused && selectionLength() > 1) {
                CFRange range = CFRangeMake(selection_begin_, selectionLength() - 1);

                if (range.length > 0) {
                    CFAttributedStringSetAttribute(cf_attr_str, range, kCTForegroundColorAttributeName, cg_text_color2);
                }
            }

            CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);


            // Alignment
            CGFloat ascent, descent, leading;
            double text_width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);

            text_min_x_offset_ = content_width - text_width;
            if (text_alignment_ != Alignment::Right) {
                if (text_min_x_offset_ > 0) {
                    text_min_x_offset_ = 0;
                }
                text_max_x_offset_ = 0;
            }
            else {
                text_max_x_offset_ = std::clamp<double>(content_width - text_width, 0, content_width);
            }

            double text_x = content_rect_.x_ + text_x_offset_;
            double text_y = centerY() + (ascent - descent) / 2;

            // Cursor
            double cursor_offset = cursor_index_ < 1 ? 0.0 : CTLineGetOffsetForStringIndex(line, cursor_index_, NULL);
            double cursor_x = text_x + cursor_offset;

            if (cursor_must_be_visible_) {
                double new_cursor_x = cursor_x;
                if (cursor_x < padding_left) {
                    new_cursor_x = padding_left;
                }
                else if (cursor_x > rect_.width_ - padding_right) {
                    new_cursor_x = rect_.width_ - padding_right;
                }

                if (new_cursor_x != cursor_x) {
                    text_x_offset_ += (new_cursor_x - cursor_x);
                    text_x += (new_cursor_x - cursor_x);
                    cursor_x = new_cursor_x;
                }
            }


            // Read just horizontal position of text and cursor
            text_x_offset_ = std::clamp<float>(text_x_offset_, text_min_x_offset_, text_max_x_offset_);
            text_x = content_rect_.x_ + text_x_offset_;
            cursor_x = text_x + cursor_offset;

            // Draw selection background
            if (selection_begin_ >= 0 && selection_end_ > selection_begin_) {
                double x1 = text_x + CTLineGetOffsetForStringIndex(line, selection_begin_, NULL);
                double x2 = text_x + CTLineGetOffsetForStringIndex(line, selection_end_, NULL);
                if (x1 < content_rect_.x_) {
                    x1 = content_rect_.x_;
                }
                if (x2 > content_rect_.x2()) {
                    x2 = content_rect_.x2();
                }

                Rectd rect = content_rect_;
                rect.x_ = x1;
                rect.width_ = x2 - x1;

                RGBA color = { 0, 0.5f, 1, 1};
                if (!isEnabled()) {
                }

                gc->setFillRGBA(selection_background_color);
                gc->fillRect(rect);
            }

            // Draw cursor
            if (is_enabled_ && focused) {
                drawCursor(gc, cursor_x);
            }

            // Draw text
            CGContextSaveGState(cg_context);
            CGContextClipToRect(cg_context, content_rect_.cgRect());

            CGContextSetTextPosition(cg_context, text_x, text_y);
            CTLineDraw(line, cg_context);
            CGContextRestoreGState(cg_context);

            // Cleanup
            CFRelease(cf_str);
            CFRelease(cf_attr_str);
            CFRelease(line);
            CGColorRelease(cg_text_color1);
            CGColorRelease(cg_text_color2);
        }
        else {
            // Draw the cursor
            if (is_enabled_ && focused) {
                float x = text_alignment_ == Alignment::Right ? width() - padding_right : padding_left;
                drawCursor(gc, x);
            }
        }

        if (text_length < 1 && info_text_) {
            Alignment alignment = text_alignment_ == Alignment::Right ? Alignment::Right : Alignment::Left;
            float alpha = is_enabled_ ? 0.5f : disabled_alpha;
            gc->drawTextInRect(info_text_, contentRect(), alignment, style->font(), style->textInfoColor(), alpha);
        }

        if (focused) {
            gc->setFillColor(1, 0, 0, 1);
            gc->fillFrame(boundsRect(), 2);
        }
    }
#else
    void TextField::draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept {
        #warning "TextField::draw() is not implemented for Linux"
    }
#endif

    void TextField::drawCursor(GraphicContext* gc, float x) const noexcept {
        if (auto style = guiStyle()) {
            Rectd rect = contentRect();
            rect.x_ = x - 1;
            rect.width_ = 2;

            RGB color = style->textCursorColor();
            if (!isEnabled()) {
            }

            gc->setFillRGB(color);
            gc->fillRect(rect);
        }
    }


    bool TextField::setEnabled(bool enabled) noexcept {
        bool result = Component::setEnabled(enabled);

        if (result && !enabled) {
            cursor_index_ = 0;
            needsDisplay();
        }

        return result;
    }


    void TextField::setNumberMode(bool mode) noexcept {
        if (mode != is_number_mode_) {
            is_number_mode_ = mode;
            if (is_number_mode_) {
                value_ = 0;
                text_alignment_ = Alignment::Right;
                needsDisplay();
            }
        }
    }


    void TextField::stepNumber(bool use_big_step, bool negative) noexcept {
        Fix step = use_big_step ? big_step_ : step_;

        if (negative) {
            step =- step;
        }

        bool value_changed = false;

        char buffer[100];
        value_.toStr(buffer, 100, Fix::kDecPrecision);

        Fix new_value = value_;
        new_value += step;

        step.toStr(buffer, 100, Fix::kDecPrecision);

        if (step_flip_mode_) {
            new_value.flip(min_, max_);
        }
        else {
            new_value.clamp(min_, max_);
        }

        new_value.toStr(buffer, 100, Fix::kDecPrecision);

        value_changed = new_value != value_;
        setValue(new_value);
        setCursorToEnd();

        // Update receiver (if one is connected) and fire action
        if (value_changed) {
            fireAction(ActionType::None, nullptr);
        }
    }


    bool TextField::setSelection(int32_t begin, int32_t end) noexcept {
        bool changed = false;
        int32_t max = textLength();

        if (end < begin) {
            std::swap(begin, end);
        }

        begin = std::clamp<int32_t>(begin, 0, max);
        end = std::clamp<int32_t>(end, 0, max);

        if (begin != selection_begin_ || end != selection_end_) {
            selection_begin_ = begin;
            selection_end_ = end;
            changed = true;
        }

        if (cursor_index_ != end) {
            cursor_index_ = end;
            changed = true;
        }

        if (changed) {
            _checkSelectionAndCursor();
            needsDisplay();
        }

        return changed;
    }


    bool TextField::setCursor(int32_t cursor_index, bool selection_mode) noexcept {
        if (selection_mode) {
            setSelection(selection_drag_start_, cursor_index);
        }
        else {
            removeSelection();
        }

        cursor_index = std::clamp<int32_t>(cursor_index, 0, textLength());
        if (cursor_index != cursor_index_) {
            cursor_index_ = cursor_index;
            needsDisplay();
            return true;
        }

        return false;
    }


    bool TextField::moveCursor(int32_t offset, bool shift_pressed) noexcept {
        int32_t new_cursor_index = std::clamp<int32_t>(cursor_index_ + offset, 0, textLength());

        if (new_cursor_index != cursor_index_) {
            if (shift_pressed) {
                if (selection_drag_start_ < 0) {
                    selection_drag_start_ = cursor_index_;
                }
            }
            setCursor(new_cursor_index, shift_pressed);
        }
        else if (!shift_pressed) {
            removeSelection();
        }

        return false;
    }


    bool TextField::selectWordAtCursor() noexcept {
        bool result = false;

        if (text_) {
            int32_t n = textLength();
            int32_t begin = 0;
            int32_t end = n;
            char c;

            for (int32_t i = cursor_index_ - 1; i >= 0; i--) {
                if (text_->isAsciiAtIndex(i, c)) {
                    if (c == ' ' || c == '\t') {
                        begin = i + 1;
                        break;
                    }
                }
            }

            for (int32_t i = cursor_index_; i < n; i++) {
                if (text_->isAsciiAtIndex(i, c)) {
                    if (c == ' ' || c == '\t') {
                        end = i;
                        break;
                    }
                }
            }

            if (begin != selection_begin_ || end != selection_end_) {
                selection_begin_ = begin;
                selection_end_ = end;
                cursor_index_ = end;
                needsDisplay();
                return true;
            }
        }

        return result;
    }


    bool TextField::removeSelection() noexcept {
        if (selection_begin_ >= 0) {
            selection_begin_ = -1;
            selection_end_ = -1;
            selection_drag_start_ = -1;
            needsDisplay();
            return true;
        }
        else {
            return false;
        }
    }


    bool TextField::removeCharAheadOfCursor() noexcept {
        if (text_ && is_editable_ && cursor_index_ > 0) {
            text_->remove(cursor_index_ - 1, 1);
            cursor_index_--;

            needsDisplay();
            textChangedAction();

            return true;
        }
        else {
            return false;
        }
    }


    bool TextField::removeSelectedText() noexcept {
        if (text_ && is_editable_ && hasSelection()) {
            text_->remove(selection_begin_, selection_end_ - selection_begin_);
            setCursor(selection_begin_);
            selection_begin_ = -1;
            selection_drag_start_ = -1;

            needsDisplay();
            textChangedAction();

            return true;
        }
        else {
            return false;
        }
    }


    void TextField::insertText(const char* text) noexcept {
        if (text_ && is_editable_ && text && strlen(text) > 0) {
            removeSelectedText();

            int32_t lengthBeforeInsert = textLength();
            text_->insertAtCharacterIndex(text, cursor_index_);
            int32_t lengthAfterInsert = textLength();
            cursor_index_ += (lengthAfterInsert - lengthBeforeInsert);

            needsDisplay();
            textChangedAction();
        }
    }


    void TextField::updateEdit() noexcept {
        bool value_changed = false;

        if (is_number_mode_ && text_) {
            Fix new_value(text_->utf8());
            new_value.setPrecision(fractional_digits_);
            value_changed = new_value != value_;
            value_.set(new_value, min_, max_, fractional_digits_);

            if (value_changed) {
                transmit();
                fireAction(ActionType::None, nullptr);
            }
        }
        else {
            transmit();
            fireAction(ActionType::None, nullptr);
        }
    }


    void TextField::handleMouseDown(const Event& event) noexcept {
        cursor_must_be_visible_ = true;

        if (event.isMouseDoubleClicked()) {
            selectWordAtCursor();
        }
        else {
            int32_t new_cursor_index = cursorIndexAtPos(event.mouseDownPos());

            if (event.isShiftPressedOnly()) {
                if (hasSelection()) {
                    if (new_cursor_index < selection_begin_) {
                        selection_drag_start_ = selection_end_;
                    }
                    else if (new_cursor_index > selection_end_) {
                        selection_drag_start_ = selection_begin_;
                    }
                    else if (std::abs(new_cursor_index - selection_begin_) < std::abs(new_cursor_index - selection_end_)) {
                        selection_drag_start_ = selection_end_;
                    }
                    else {
                        selection_drag_start_ = selection_begin_;
                    }
                }
                else {
                    selection_drag_start_ = cursor_index_;
                }
            }
            else {
                selection_drag_start_ = new_cursor_index;
            }

            setCursor(new_cursor_index, selection_drag_start_);
        }
    }


    void TextField::handleMouseDrag(const Event& event) noexcept {
        cursor_must_be_visible_ = true;
        int32_t new_cursor_index = cursorIndexAtPos(event.mousePos());

        setCursor(new_cursor_index, true);
    }


    void TextField::handleMouseUp(const Event& event) noexcept {
        cursor_must_be_visible_ = true;

        if (isDelayed()) {
            fireAction(ActionType::None, nullptr);
        }

        if (hit(event) && isToggleMode()) {
            toggleSelection();
        }

        deHighlight();
    }


    void TextField::handleScrollWheel(const Event& event) noexcept {
        cursor_must_be_visible_ = false;
        text_x_offset_ += event.deltaX() * App::scrollWheelSpeed();

        needsDisplay();
    }


    void TextField::handleKeyDown(const Event& event) noexcept {
        cursor_must_be_visible_ = true;
        int32_t new_cursor_index = -1;
        uint32_t key_char = event.keyChar();

        if (event.keyCharCount() == 1) {
            if (event.isCommandPressedOnly()) {
                switch (key_char) {
                    case 'a':
                        selectAll();
                        break;
                    case 'c':
                        copyToPasteboard();
                        break;
                    case 'v':
                        pasteFromPasteboard();
                        break;
                    case 'x':
                        if (copyToPasteboard() > 0) {
                            removeSelectedText();
                        }
                        break;
                }
            }
            else {
                switch (key_char) {
                    case KeyCode::Enter:
                    case KeyCode::CarriageReturn:
                        updateEdit();
                        selectAll();
                        break;

                    case KeyCode::Backspace:
                    case KeyCode::Delete:
                        if (is_editable_) {
                            if (hasSelection()) {
                                removeSelectedText();
                                if (continuous_update_flag_) {
                                    updateEdit();
                                }
                            }
                            else {
                                removeCharAheadOfCursor();
                                if (continuous_update_flag_) {
                                    updateEdit();
                                }
                            }
                        }
                        break;

                    case KeyCode::FunctionUpArrow:
                        if (isNumberMode()) {
                            stepNumber(event.isShiftPressedOnly(), false);
                        }
                        else {
                            new_cursor_index = 0;
                        }
                        break;

                    case KeyCode::FunctionDownArrow:
                        if (isNumberMode()) {
                            stepNumber(event.isShiftPressedOnly(), true);
                        }
                        else {
                            new_cursor_index = textLength();
                        }
                        break;

                    case KeyCode::FunctionLeftArrow:
                        moveCursor(-1, event.isShiftPressed());
                        break;

                    case KeyCode::FunctionRightArrow:
                        moveCursor(1, event.isShiftPressed());
                        break;

                    default:
                        if (is_editable_) {
                            if (!isNumberMode() || String::unicharIsNumeric(key_char)) {
                                _interpretKeyEvents(event);
                                if (continuous_update_flag_) {
                                    updateEdit();
                                }
                            }
                        }
                        break;
                }
            }
        }

        if (new_cursor_index >= 0) {
            if (event.isShiftPressed()) {
                if (selectionDragStart() < 0) {
                    setSelectionDragStart(clampedCursorIndex());
                }
            }
            else {
                setSelectionDragStart(-1);
            }

            setCursor(new_cursor_index, event.isShiftPressed());
        }
    }


    void TextField::becomeFirstResponder() noexcept {
        selectAll();
        needsDisplay();
    }


    void TextField::resignFirstResponder() noexcept {
        updateEdit();
        cursor_index_ = 0;
        selection_begin_ = selection_end_ = -1;

        needsDisplay();
    }


#if defined(__APPLE__) && defined(__MACH__)
    int32_t TextField::cursorIndexAtPos(Vec2d pos) noexcept {
        int32_t cursor_index = -1;
        auto style = App::guiStyleAtIndex(style_index_);

        // TODO: Eventually use font->indexForCharAtPos inside direct CoreText functions here.

        if (hasText() && style) {
            _checkSelectionAndCursor();

            Rectd content_rect = content_rect_;

            CFStringRef cf_str = CFStringCreateWithCString(NULL, text_->utf8(), kCFStringEncodingUTF8);
            CFMutableAttributedStringRef cf_attr_str = CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);
            CFAttributedStringReplaceString(cf_attr_str, CFRangeMake(0, 0), cf_str);

            CFRange range = CFRangeMake(0, text_->length());
            if (range.length > 0) {
                CFAttributedStringSetAttribute(cf_attr_str, range, kCTFontAttributeName, style->font()->ctFont());
            }

            CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);
            double text_x = content_rect.x_ + text_x_offset_;

            double min_delta = std::numeric_limits<float>::max();
            for (int32_t i = 0; i <= textLength(); i++) {
                double cx = text_x + CTLineGetOffsetForStringIndex(line, i, NULL);
                double delta = std::fabs(pos.x_ - cx);
                if (delta < min_delta) {
                    min_delta = delta;
                    cursor_index = i;
                }
            }

            CFRelease(cf_str);
            CFRelease(cf_attr_str);
            CFRelease(line);
        }

        return cursor_index;
    }
#else
    int32_t TextField::cursorIndexAtPos(Vec2d pos) noexcept {
        #warning "TextField::cursorIndexAtPos() is not implemented for Linux"
        // TODO: Implement!
        return 0;
    }
#endif


    void TextField::_checkSelectionAndCursor() noexcept {
        selection_begin_ = std::clamp<int32_t>(selection_begin_, 0, textLength());
        selection_end_ = std::clamp<int32_t>(selection_end_, selection_begin_, textLength());

        if (selectionLength() > 0) {
            cursor_index_ = std::clamp<int32_t>(cursor_index_, selection_begin_, selection_end_);
        }
    }


    int32_t TextField::copyToPasteboard() noexcept {
        int32_t result = 0;

        if (text_ && hasSelection()) {
            text_->copyToPasteboard(selection_begin_, selection_end_ - selection_begin_);
            result = selectionLength();
        }

        return result;
    }


    int32_t TextField::pasteFromPasteboard() noexcept {
        int32_t result = 0;

        if (text_) {
            if (hasSelection()) {
                cursor_index_ = selection_begin_;
                text_->remove(selection_begin_, selection_end_ - selection_begin_);
                removeSelection();
            }

            result = static_cast<int32_t>(text_->pasteFromPasteboard(cursor_index_));
            cursor_index_ += result;

            needsDisplay();
            textChangedAction();
        }

        return result;
    }

} // End of namespace Grain
