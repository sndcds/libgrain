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


namespace Grain {

    TextField::TextField(const Rectd& rect) noexcept : Component(rect, 0) {
        m_type = ComponentType::TextField;
        m_can_get_focus = true;
        m_text_alignment = Alignment::Left;
        m_is_editable = true;
        m_is_number_mode = false;

        setText("0");
        // setMouseCursor(App::MouseCursor::IBeam); TODO: !!!!
    }


    TextField::~TextField() noexcept {
        std::free(m_info_text);
    }


    TextField* TextField::add(View* view, const Rectd& rect, const char* text, int32_t tag) {
        auto textfield = (TextField*)Component::addComponentToView((Component*)new(std::nothrow) TextField(rect), view, AddFlags::kNone);

        if (textfield) {
            textfield->setText(text);
            textfield->setTag(tag);
        }

        return textfield;
    }


    TextField* TextField::add(View* view, const Rectd& rect, Component* receiver, ComponentAction action, int32_t tag, void* action_ref) {
        TextField* textfield = nullptr;

        if (view) {
            textfield = new(std::nothrow) TextField(rect);

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
    void TextField::draw(const Rectd& dirty_rect) noexcept {
        GraphicContext gc(this);
        gc.disableFontSubpixelQuantization();

        auto style= guiStyle();
        if (!style) {
            drawDummy(gc);
            return;
        }

        CGContextRef cg_context = gc.macos_cgContext();

        _checkSelectionAndCursor();

        m_content_rect = contentRect();
        double padding_left = style->paddingLeft();
        double padding_top = style->paddingTop();
        double padding_right = style->paddingRight();
        double padding_bottom = style->paddingBottom();

        auto view_color = style->viewColor();
        auto text_color = style->textColor();
        auto background_color = style->textBackgroundColor();
        auto selection_color = style->textSelectionColor();
        auto selection_background_color = style->textSelectionBackgroundColor();

        auto disabled_alpha = style->disabledAlpha();

        bool focused = isKeyComponent();

        if (!m_is_enabled) {
            text_color.setBlend(background_color, 0.5f);
        }

        gc.setFillRGBA(background_color);
        gc.fillRect(boundsRect());

        int32_t text_length = textLength();

        if (text_length > 0) {
            gc.setTextMatrix(1, 0, 0, -1, 0, 0);
            float content_width = m_content_rect.width();
            RGBA fg_selected_color = style->textSelectionColor();

            CGColorRef cg_text_color1 = text_color.createCGColor();
            CGColorRef cg_text_color2 = selection_color.createCGColor();
            CFStringRef cf_str = nullptr;

            if (isPasswordMode()) {
                char buffer[text_length * 3 + 1]; // TODO: !!!!!
                for (int32_t i = 0; i < text_length; i++) {
                    buffer[i * 3] = 0xE2;
                    buffer[i * 3 + 1] = 0x97;
                    buffer[i * 3 + 2] = 0x8F;
                }

                buffer[text_length * 3] = 0;
                cf_str = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingUTF8);
            }
            else {
                cf_str = CFStringCreateWithCString(NULL, m_text->utf8(), kCFStringEncodingUTF8);
            }

            CFMutableAttributedStringRef cf_attr_str = CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);
            CFAttributedStringReplaceString(cf_attr_str, CFRangeMake(0, 0), cf_str);
            CFRange range = CFRangeMake(0, text_length);

            if (range.length > 0) {
                CFAttributedStringSetAttribute(cf_attr_str, range, kCTFontAttributeName, style->font()->ctFont());
                CFAttributedStringSetAttribute(cf_attr_str, range, kCTForegroundColorAttributeName, cg_text_color1);
            }

            if (focused && selectionLength() > 1) {
                CFRange range = CFRangeMake(m_selection_begin, selectionLength() - 1);

                if (range.length > 0) {
                    CFAttributedStringSetAttribute(cf_attr_str, range, kCTForegroundColorAttributeName, cg_text_color2);
                }
            }

            CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);


            // Alignment
            CGFloat ascent, descent, leading;
            double text_width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);

            m_text_min_x_offset = content_width - text_width;
            if (m_text_alignment != Alignment::Right) {
                if (m_text_min_x_offset > 0) {
                    m_text_min_x_offset = 0;
                }
                m_text_max_x_offset = 0;
            }
            else {
                m_text_max_x_offset = std::clamp<double>(content_width - text_width, 0, content_width);
            }

            double text_x = m_content_rect.m_x + m_text_x_offset;
            double text_y = centerY() + (ascent - descent) / 2;

            // Cursor
            double cursor_offset = m_cursor_index < 1 ? 0.0 : CTLineGetOffsetForStringIndex(line, m_cursor_index, NULL);
            double cursor_x = text_x + cursor_offset;

            if (m_cursor_must_be_visible) {
                double new_cursor_x = cursor_x;
                if (cursor_x < padding_left) {
                    new_cursor_x = padding_left;
                }
                else if (cursor_x > m_rect.m_width - padding_right) {
                    new_cursor_x = m_rect.m_width - padding_right;
                }

                if (new_cursor_x != cursor_x) {
                    m_text_x_offset += (new_cursor_x - cursor_x);
                    text_x += (new_cursor_x - cursor_x);
                    cursor_x = new_cursor_x;
                }
            }


            // Read just horizontal position of text and cursor
            m_text_x_offset = std::clamp<float>(m_text_x_offset, m_text_min_x_offset, m_text_max_x_offset);
            text_x = m_content_rect.m_x + m_text_x_offset;
            cursor_x = text_x + cursor_offset;

            // Draw selection background
            if (m_selection_begin >= 0 && m_selection_end > m_selection_begin) {
                double x1 = text_x + CTLineGetOffsetForStringIndex(line, m_selection_begin, NULL);
                double x2 = text_x + CTLineGetOffsetForStringIndex(line, m_selection_end, NULL);
                if (x1 < m_content_rect.m_x) {
                    x1 = m_content_rect.m_x;
                }
                if (x2 > m_content_rect.x2()) {
                    x2 = m_content_rect.x2();
                }

                Rectd rect = m_content_rect;
                rect.m_x = x1;
                rect.m_width = x2 - x1;

                RGBA color = { 0, 0.5f, 1, 1};
                if (!isEnabled()) {
                }

                gc.setFillRGBA(selection_background_color);
                gc.fillRect(rect);
            }

            // Draw cursor
            if (m_is_enabled && focused) {
                drawCursor(gc, cursor_x);
            }

            // Draw text
            CGContextSaveGState(cg_context);
            CGContextClipToRect(cg_context, m_content_rect.cgRect());

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
            if (m_is_enabled && focused) {
                float x = m_text_alignment == Alignment::Right ? width() - padding_right : padding_left;
                drawCursor(gc, x);
            }
        }

        if (text_length < 1 && m_info_text) {
            Alignment alignment = m_text_alignment == Alignment::Right ? Alignment::Right : Alignment::Left;
            float alpha = m_is_enabled ? 0.5f : disabled_alpha;
            gc.drawTextInRect(m_info_text, contentRect(), alignment, style->font(), style->textInfoColor(), alpha);
        }
    }
#else
    void TextField::draw(const Rectd& dirty_rect) noexcept {
        #warning "TextField::draw() is not implemented for Linux"
    }
#endif

    void TextField::drawCursor(GraphicContext& gc, float x) const noexcept {
        if (auto style = guiStyle()) {
            Rectd rect = contentRect();
            rect.m_x = x - 1;
            rect.m_width = 2;

            RGB color = style->textCursorColor();
            if (!isEnabled()) {
            }

            gc.setFillRGB(color);
            gc.fillRect(rect);
        }
    }


    bool TextField::setEnabled(bool enabled) noexcept {
        bool result = Component::setEnabled(enabled);

        if (result && !enabled) {
            m_cursor_index = 0;
            needsDisplay();
        }

        return result;
    }


    void TextField::setNumberMode(bool mode) noexcept {
        if (mode != m_is_number_mode) {
            m_is_number_mode = mode;
            if (m_is_number_mode) {
                m_value = 0;
                m_text_alignment = Alignment::Right;
                needsDisplay();
            }
        }
    }


    void TextField::stepNumber(bool use_big_step, bool negative) noexcept {
        Fix step = use_big_step ? m_big_step : m_step;

        if (negative) {
            step =- step;
        }

        bool value_changed = false;

        char buffer[100];
        m_value.toStr(buffer, 100, Fix::kDecPrecision);

        Fix new_value = m_value;
        new_value += step;

        step.toStr(buffer, 100, Fix::kDecPrecision);

        if (m_step_flip_mode) {
            new_value.flip(m_min, m_max);
        }
        else {
            new_value.clamp(m_min, m_max);
        }

        new_value.toStr(buffer, 100, Fix::kDecPrecision);

        value_changed = new_value != m_value;
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

        if (begin != m_selection_begin || end != m_selection_end) {
            m_selection_begin = begin;
            m_selection_end = end;
            changed = true;
        }

        if (m_cursor_index != end) {
            m_cursor_index = end;
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
            setSelection(m_selection_drag_start, cursor_index);
        }
        else {
            removeSelection();
        }

        cursor_index = std::clamp<int32_t>(cursor_index, 0, textLength());
        if (cursor_index != m_cursor_index) {
            m_cursor_index = cursor_index;
            needsDisplay();
            return true;
        }

        return false;
    }


    bool TextField::moveCursor(int32_t offset, bool shift_pressed) noexcept {
        int32_t new_cursor_index = std::clamp<int32_t>(m_cursor_index + offset, 0, textLength());

        if (new_cursor_index != m_cursor_index) {
            if (shift_pressed) {
                if (m_selection_drag_start < 0) {
                    m_selection_drag_start = m_cursor_index;
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

        if (m_text) {
            int32_t n = textLength();
            int32_t begin = 0;
            int32_t end = n;
            char c;

            for (int32_t i = m_cursor_index - 1; i >= 0; i--) {
                if (m_text->isAsciiAtIndex(i, c)) {
                    if (c == ' ' || c == '\t') {
                        begin = i + 1;
                        break;
                    }
                }
            }

            for (int32_t i = m_cursor_index; i < n; i++) {
                if (m_text->isAsciiAtIndex(i, c)) {
                    if (c == ' ' || c == '\t') {
                        end = i;
                        break;
                    }
                }
            }

            if (begin != m_selection_begin || end != m_selection_end) {
                m_selection_begin = begin;
                m_selection_end = end;
                m_cursor_index = end;
                needsDisplay();
                return true;
            }
        }

        return result;
    }


    bool TextField::removeSelection() noexcept {
        if (m_selection_begin >= 0) {
            m_selection_begin = -1;
            m_selection_end = -1;
            m_selection_drag_start = -1;
            needsDisplay();
            return true;
        }
        else {
            return false;
        }
    }


    bool TextField::removeCharAheadOfCursor() noexcept {
        if (m_text && m_is_editable && m_cursor_index > 0) {
            m_text->remove(m_cursor_index - 1, 1);
            m_cursor_index--;

            needsDisplay();
            textChangedAction();

            return true;
        }
        else {
            return false;
        }
    }


    bool TextField::removeSelectedText() noexcept {
        if (m_text && m_is_editable && hasSelection()) {
            m_text->remove(m_selection_begin, m_selection_end - m_selection_begin);
            setCursor(m_selection_begin);
            m_selection_begin = -1;
            m_selection_drag_start = -1;

            needsDisplay();
            textChangedAction();

            return true;
        }
        else {
            return false;
        }
    }


    void TextField::insertText(const char* text) noexcept {
        if (m_text && m_is_editable && text && strlen(text) > 0) {
            removeSelectedText();

            int32_t lengthBeforeInsert = textLength();
            m_text->insertAtCharacterIndex(text, m_cursor_index);
            int32_t lengthAfterInsert = textLength();
            m_cursor_index += (lengthAfterInsert - lengthBeforeInsert);

            needsDisplay();
            textChangedAction();
        }
    }


    void TextField::updateEdit() noexcept {
        bool value_changed = false;

        if (m_is_number_mode && m_text) {
            Fix new_value(m_text->utf8());
            new_value.setPrecision(m_fractional_digits);
            value_changed = new_value != m_value;
            m_value.set(new_value, m_min, m_max, m_fractional_digits);

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
        m_cursor_must_be_visible = true;

        if (event.isMouseDoubleClicked()) {
            selectWordAtCursor();
        }
        else {
            int32_t new_cursor_index = cursorIndexAtPos(event.mouseDownPos());

            if (event.isShiftPressedOnly()) {
                if (hasSelection()) {
                    if (new_cursor_index < m_selection_begin) {
                        m_selection_drag_start = m_selection_end;
                    }
                    else if (new_cursor_index > m_selection_end) {
                        m_selection_drag_start = m_selection_begin;
                    }
                    else if (std::abs(new_cursor_index - m_selection_begin) < std::abs(new_cursor_index - m_selection_end)) {
                        m_selection_drag_start = m_selection_end;
                    }
                    else {
                        m_selection_drag_start = m_selection_begin;
                    }
                }
                else {
                    m_selection_drag_start = m_cursor_index;
                }
            }
            else {
                m_selection_drag_start = new_cursor_index;
            }

            setCursor(new_cursor_index, m_selection_drag_start);
        }
    }


    void TextField::handleMouseDrag(const Event& event) noexcept {
        m_cursor_must_be_visible = true;
        int32_t new_cursor_index = cursorIndexAtPos(event.mousePos());

        setCursor(new_cursor_index, true);
    }


    void TextField::handleMouseUp(const Event& event) noexcept {
        m_cursor_must_be_visible = true;

        if (isDelayed()) {
            fireAction(ActionType::None, nullptr);
        }

        if (hit(event) && isToggleMode()) {
            toggleSelection();
        }

        deHighlight();
    }


    void TextField::handleScrollWheel(const Event& event) noexcept {
        m_cursor_must_be_visible = false;
        m_text_x_offset += event.deltaX() * App::scrollWheelSpeed();

        needsDisplay();
    }


    void TextField::handleKeyDown(const Event& event) noexcept {
        m_cursor_must_be_visible = true;
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
                        if (m_is_editable) {
                            if (hasSelection()) {
                                removeSelectedText();
                                if (m_continuous_update_flag) {
                                    updateEdit();
                                }
                            }
                            else {
                                removeCharAheadOfCursor();
                                if (m_continuous_update_flag) {
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
                        if (m_is_editable) {
                            if (!isNumberMode() || String::unicharIsNumeric(key_char)) {
                                _interpretKeyEvents(event);
                                if (m_continuous_update_flag) {
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
        m_cursor_index = 0;
        m_selection_begin = m_selection_end = -1;

        needsDisplay();
    }


#if defined(__APPLE__) && defined(__MACH__)
    int32_t TextField::cursorIndexAtPos(Vec2d pos) noexcept {
        int32_t cursor_index = -1;
        auto style = App::guiStyleAtIndex(m_style_index);

        // TODO: Eventually use font->indexForCharAtPos inside direct CoreText functions here.

        if (hasText() && style) {
            _checkSelectionAndCursor();

            Rectd content_rect = m_content_rect;

            CFStringRef cf_str = CFStringCreateWithCString(NULL, m_text->utf8(), kCFStringEncodingUTF8);
            CFMutableAttributedStringRef cf_attr_str = CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);
            CFAttributedStringReplaceString(cf_attr_str, CFRangeMake(0, 0), cf_str);

            CFRange range = CFRangeMake(0, m_text->length());
            if (range.length > 0) {
                CFAttributedStringSetAttribute(cf_attr_str, range, kCTFontAttributeName, style->font()->ctFont());
            }

            CTLineRef line = CTLineCreateWithAttributedString(cf_attr_str);
            double text_x = content_rect.m_x + m_text_x_offset;

            double min_delta = std::numeric_limits<float>::max();
            for (int32_t i = 0; i <= textLength(); i++) {
                double cx = text_x + CTLineGetOffsetForStringIndex(line, i, NULL);
                double delta = std::fabs(pos.m_x - cx);
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
        m_selection_begin = std::clamp<int32_t>(m_selection_begin, 0, textLength());
        m_selection_end = std::clamp<int32_t>(m_selection_end, m_selection_begin, textLength());

        if (selectionLength() > 0) {
            m_cursor_index = std::clamp<int32_t>(m_cursor_index, m_selection_begin, m_selection_end);
        }
    }


    int32_t TextField::copyToPasteboard() noexcept {
        int32_t result = 0;

        if (m_text && hasSelection()) {
            m_text->copyToPasteboard(m_selection_begin, m_selection_end - m_selection_begin);
            result = selectionLength();
        }

        return result;
    }


    int32_t TextField::pasteFromPasteboard() noexcept {
        int32_t result = 0;

        if (m_text) {
            if (hasSelection()) {
                m_cursor_index = m_selection_begin;
                m_text->remove(m_selection_begin, m_selection_end - m_selection_begin);
                removeSelection();
            }

            result = (int32_t)m_text->pasteFromPasteboard(m_cursor_index);
            m_cursor_index += result;

            needsDisplay();
            textChangedAction();
        }

        return result;
    }

} // End of namespace Grain
