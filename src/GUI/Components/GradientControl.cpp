//
//  GradientControl.cpp
//
//  Created by Roald Christesen on 18.01.2013
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#include "GUI/Components/GradientControl.hpp"
#include "Color/Gradient.hpp"


namespace Grain {

    GradientControl::GradientControl(const Rectd& rect) noexcept : Component(rect) {

        _initNSView(rect);

        type_ = ComponentType::GradientControl;

        m_padding.set(0, spot_icon_width_ / 2 + 1, spot_icon_height_ + 1, spot_icon_width_ / 2 + 1);
        m_gradient = new(std::nothrow) Gradient();
        m_radius = 6.0f;
        m_font = App::uiSmallFont();

        m_context_menu = _buildContextMenu();

        geometryChanged();
    }


    GradientControl::~GradientControl() noexcept {

        GRAIN_RELEASE(m_gradient);
    }


    GradientControl* GradientControl::add(View* view, const Rectd& rect) {

        return (GradientControl*)Component::addComponentToView((Component*)new(std::nothrow) GradientControl(rect), view);
    }


    bool GradientControl::setEnabled(bool enabled) noexcept {

        bool result = Component::setEnabled(enabled);

        if (result) {
            Component::setEnabled(m_color_wheel, enabled);
            Component::setEnabled(m_step_slider, enabled);
            Component::setEnabled(m_step_textfield, enabled);
        }

        if (m_label != nullptr) {
            m_label->setEnabled(enabled);
        }

        return result;
    }


    void GradientControl::geometryChanged() noexcept {

        gradient_rect_ = boundsRect();
        gradient_rect_.inset(m_padding);
        m_spots_rect = boundsRect();
        m_spots_rect.insetTop(gradient_rect_.height());
        m_gradient_width = gradient_rect_.width();
    }


    void GradientControl::drawStop(GraphicContext& gc, const GradientStop* stop) noexcept {

        if (stop == nullptr) {
            return;
        }

        Rectd rect = stopRect(stop);

        RGB bg_color = uiViewColor();
        RGB fg_color = uiFGColor();

        gc.save();
        gc.translate(rect.centerX(), rect.y());
        double scale = stop->isSelected() ? 20 : 14;
        gc.scale(scale);

        if (stop->isTwoColored()) {
            gc.addLeftHalfDropPath();
            gc.setFillColor(stop->leftColor());
            gc.fillPath();
            gc.addRightHalfDropPath();
            gc.setFillColor(stop->rightColor());
            gc.fillPath();
            gc.setStrokeColor(bg_color);
            gc.setStrokeWidth(1.0 / scale);
            gc.strokeVerticalLine(0, 0, scale);
        }
        else {
            gc.addDropPath();
            gc.setFillColor(stop->leftColor());
            gc.fillPath();
        }

        float d = bg_color.perceptualDistance(stop->leftColor());
        if (d < 0.1) {
            gc.addDropPath();
            gc.setStrokeColor(fg_color, Math::remap(0, 0.1, 0.5, 0, d));
            gc.setStrokeWidth(0.6f / scale);
            gc.strokePath();
        }

        gc.restore();
    }


    void GradientControl::draw(const Rectd& dirty_rect) noexcept {

        GraphicContext gc(this);

        bool disabled = !m_enabled;

        Rectd gradient_rect = contentRect();

        RGB view_color = uiViewColor();
        RGB bg_color = uiBGColor();
        RGB fg_color = uiFGColor();

        if (disabled == true) {
            m_look->disableColorMixed(fg_color, bg_color, view_color);
            m_look->disableColor(bg_color, view_color);
        }

        // Draw the gradient
        gc.save();
        gc.beginPath();
        gc.addRoundRectPath(gradient_rect, m_radius);
        gc.clipPath();

        int32_t stop_count = m_gradient->stopCount();

        if (stop_count > 1) {
            m_gradient->drawInRect(gc, gradient_rect, Direction::LeftToRight, true, true);

            if (disabled == true) {
                gc.setFillColor(bg_color, 1.0f - App::uiDisabledAlpha());
                gc.fillRect(gradient_rect);
            }
        }
        else {
            gc.setFillColor(bg_color);
            gc.fillRect(gradient_rect);
        }

        gc.restore();

        // Draw the stop handles
        if (m_enabled == true) {
            RGB handle_color = uiColor(UIColor::Handle);
            gc.setFillColor(handle_color);

            for (int32_t i = 0; i < stop_count; i++) {
                const GradientStop* stop = m_gradient->mutableStopPtrAtIndex(i);
                drawStop(gc, stop);
            }
        }

        if (stop_count < 1) {
            drawCenteredMessage(gc, "Empty Gradient", nullptr, fg_color);
        }
        else if (stop_count == 1) {
            drawCenteredMessage(gc, "Needs an extra Color Stop", nullptr, fg_color);
        }
    }


    void GradientControl::handleMouseDown(const Event& event) noexcept {

        Vec2d mouse_down_pos = event.mouseDownPos();
        m_mouse_mode = kMouseMode_Undefined;
        m_drag_started = false;

        if (m_gradient == nullptr) {
            event.mousePressedFinished();
        }

        if (m_spots_rect.contains(mouse_down_pos)) {
            int32_t stop_index = stopIndexAtPos(mouse_down_pos);

            if (stop_index >= 0) {
                GradientStop* stop = m_gradient->mutableStopPtrAtIndex(stop_index);

                if (stop != nullptr) {
                    m_mouse_mode = kMouseMode_Stop;

                    if (event.isShiftPressedOnly()) {
                        if (stop->isSelected() == false) {
                            App::beep();
                            updateStopParameters(stop);
                        }
                        stop->toggleSelection();
                        needsDisplay();
                        return;
                    }

                    if (event.isShiftPressedOnly() == false && stop->isSelected() == false) {
                        m_gradient->deselectAllStops();
                    }

                    if (stop->isSelected() == false) {
                        stop->select();
                        needsDisplay();
                    }

                    updateStopParameters(stop);

                    m_gradient->rememberSelectedStops();
                }

                return;
            }


            // Insert a new stop

            if (event.isShiftPressedOnly()) {
                // No inserts when shift is pressed
                return;
            }

            RGBA new_color;

            double pos = viewXToPos(mouse_down_pos.m_x);
            m_gradient->lookupColor(pos, new_color);

            m_gradient->addStop(pos, new_color);
            GradientStop* new_stop = m_gradient->mutableStopPtrAtIndex(m_gradient->lastStopIndex());

            m_gradient->deselectAllStops();
            new_stop->select();
            new_stop->remember();

            if (m_color_wheel != nullptr) {
                m_color_wheel->setReceiverComponent(this);
                m_color_wheel->setColor(new_color);
            }

            event.mousePressedFinished();
            needsDisplay();
        }
        else {
            m_gradient->deselectAllStops();
            needsDisplay();
        }
    }


    void GradientControl::handleRightMouseDown(const Event& event) noexcept {

        Vec2d mouse_down_pos = event.mouseDownPos();

        if (m_gradient == nullptr) {
            event.rightMousePressedFinished();
            return;
        }

        _updateContextMenu();

        if (m_context_menu != nullptr) {
            m_context_menu->popUp(this, mouse_down_pos + Vec2d(2, 8));
        }

        event.rightMousePressedFinished();
    }


    void GradientControl::handleMouseUp(const Event& event) noexcept {

        /*
        if (m_mouse_mode == kMouseMode_Stop && event.mouseY() > height() + 20) {
            m_gradient->removeSelectedStops();
            fireAction();
        }
         */

        m_mouse_mode = kMouseMode_Undefined;
        needsDisplay();
    }


    void GradientControl::handleMouseDrag(const Event& event) noexcept {

        if (m_mouse_mode == kMouseMode_Stop) {
            if (m_drag_started == false) {
                if (std::fabs(event.mouseDragDeltaX()) > 2.0f) {
                    m_drag_started = true;
                }
            }

            if (m_drag_started == true) {
                if (m_gradient->moveSelectedStops(event.mouseDragDeltaX() / gradient_rect_.width())) {
                    fireActionAndDisplay(Component::ActionType::None, nullptr);
                }
            }
        }
    }


    void GradientControl::setByComponent(Component* component) noexcept {

        bool changed = false;

        if (component != nullptr) {
            if (component->isColorWheel()) {
                auto color_wheel = (ColorWheel*)component;
                RGB rgb = color_wheel->color();
                changed = m_gradient->setColorOfSelectedStops(rgb);
            }
            else if (component->componentType() == ComponentType::Slider) {
                auto slider = (Slider*)component;
                Fix value = slider->value();

                if (slider == m_step_slider) {
                    changed = m_gradient->setStepCountOfSelectedStops(value.asInt32());
                    if (m_step_textfield != nullptr) {
                        m_step_textfield->setValue(value);
                    }
                }
            }
            else if (component->isTextField()) {
                auto textfield = (TextField*)component;
                Fix value = textfield->value();

                if (textfield == m_step_textfield) {
                    changed = m_gradient->setStepCountOfSelectedStops(value.asInt32());
                    if (m_step_slider != nullptr) {
                        m_step_slider->setValue(value);
                    }
                }
            }
        }

        if (changed == true) {
            fireActionAndDisplay(Component::ActionType::None, component);
        }
    }


    void GradientControl::updateRepresentations(const Component* excluded_component) noexcept {

        /* TODO: Implement!
         if (_combineModePopup != nil)
            [_combineModePopup selectItemWithTag:Gr::colorCombinationModeIndex(combineMode)];

         if (_amountSlider != nil)
            [_amountSlider setFloatValue:amount];

         if (_amountTextField != nil)
            [_amountTextField setFloatValue:amount];
         */
    }


    int32_t GradientControl::stopIndexAtPos(Vec2d pos) const noexcept {

        if (m_gradient != nullptr) {
            int32_t index = 0;
            for (auto& stop : m_gradient->m_stops) {
                if (stopRect(&stop).contains(pos)) {
                    return index;
                }
                index++;
            }
        }

        return -1;
    }


    Rectd GradientControl::stopRect(const GradientStop* stop) const noexcept {

        if (stop != nullptr) {
            return { viewPosToX(stop) - spot_icon_width_ / 2, m_spots_rect.y(), spot_icon_width_, spot_icon_height_ };
        }
        else {
            return Rectd();
        }
    }


    double GradientControl::viewPosToX(const GradientStop* stop) const noexcept {

        return stop != nullptr ? m_padding.m_left + stop->pos() * m_gradient_width : 0.0f;
    }


    double GradientControl::viewXToPos(double x) const noexcept {

        return m_gradient_width < 1.0f ? 0.0f : Grain::Type::clampedDouble((x - m_padding.m_left) / m_gradient_width);
    }


    void GradientControl::setColorWheel(ColorWheel* color_wheel) noexcept {

        if (m_color_wheel != nullptr) {
            m_color_wheel->setReceiverComponent(nullptr);
        }

        m_color_wheel = color_wheel;
        if (m_color_wheel != nullptr) {
            m_color_wheel->setReceiverComponent(this);
            m_color_wheel->setEnabled(m_enabled);
        }
    }


    void GradientControl::setStepSlider(Slider* slider) noexcept {

        m_step_slider = slider;

        if (m_step_slider != nullptr) {
            m_step_slider->setReceiverComponent(this);
            m_step_slider->setEnabled(m_enabled);
            m_step_slider->setup(0, 32, 0, 0, 1, 1);
        }
    }


    void GradientControl::setStepTextField(TextField* textfield) noexcept {

        m_step_textfield = textfield;

        if (m_step_textfield != nullptr) {
            m_step_textfield->setReceiverComponent(this);
            m_step_textfield->setEnabled(m_enabled);
            m_step_textfield->setNumberMode(true);
            m_step_textfield->setValueRangeInt32(0, 32);
        }
    }


    void GradientControl::setGradientColor(int32_t index, const RGB& rgb) noexcept {

        if (index >= 0 && index < m_gradient->stopCount()) {
            /*
            if (m_gradient->setColorAtIndex(index, rgb)) {
                fireAction();
            }
             */
            // TODO: Gradient!!!!
        }
    }


    void GradientControl::setGradientColorHSV(int32_t index, const HSV& hsv) noexcept {

        if (index >= 0 && index < m_gradient->stopCount()) {
            /*
            if (m_gradient->setColorAtIndex(index, hsv)) {
                fireAction();
            }
             */
            // TODO: Gradient!!!!
        }
    }


    void GradientControl::updateStopParameters(const GradientStop* stop) noexcept {

        if (stop != nullptr) {
            if (m_color_wheel != nullptr) {
                RGBA color = stop->color(0);
                m_color_wheel->setColor(color);
                m_color_wheel->setReceiverComponent(this);
            }

            if (m_step_slider != nullptr) {
                m_step_slider->setValue(stop->stepCount());
            }

            if (m_step_textfield != nullptr) {
                m_step_textfield->setValue(stop->stepCount());
            }
        }
    }


    void GradientControl::_menuAction(Menu* menu, int32_t tag, void* ref) noexcept {

        if (ref == nullptr) {
            return;
        }

        auto control = (GradientControl*)ref;
        auto gradient = control->gradient();
        if (gradient == nullptr) {
            return;
        }

        bool changed = false;

        switch (tag) {
            case kMenu_Options:

            case kMenu_Reset:
                gradient->reset();
                changed = true;
                break;

            case kMenu_SelectAll:
                gradient->selectAllStops();
                changed = true;
                break;

            case kMenu_RevertSelection:
                gradient->revertStopSelection();
                changed = true;
                break;

            case kMenu_SelectEachSecond:
                gradient->selectStopEach(2);
                changed = true;
                break;

            case kMenu_Flip:
                gradient->flip();
                changed = true;
                break;

            case kMenu_Distribute:
                gradient->distribute();
                changed = true;
                break;

            case kMenu_Stretch:
                gradient->stretch();
                changed = true;
                break;

            case kMenu_DeleteSpots:
                changed = gradient->removeSelectedStops() > 0;
                break;

            case kMenu_SpotsSetSingleColor:
                changed = gradient->setColorModeOfSetectedStops(false);
                break;

            case kMenu_SpotsSetTwoColors:
                changed = gradient->setColorModeOfSetectedStops(true);
                break;

            case kMenu_LoadFile:
                control->loadFromFile();
                changed = true;
                break;

            case kMenu_SaveFile:
                control->saveToFileAs();
                break;
        }

        if (changed == true) {
            control->needsDisplay();
        }
    }


    Menu* GradientControl::_buildContextMenu() noexcept {

        Menu* menu = new(std::nothrow) Menu();

        if (menu != nullptr) {
            menu->addItem("Options ...", GradientControl::kMenu_Options);
            menu->addItem("Reset", GradientControl::kMenu_Reset);

            menu->addSeparator();
            menu->addItem("Select all", GradientControl::kMenu_SelectAll);
            menu->addItem("Revert selection", GradientControl::kMenu_RevertSelection);
            menu->addItem("Select each second", GradientControl::kMenu_SelectEachSecond);

            Menu* editMenu = menu->addSubMenu("Edit");
            // editMenu->addItem("Copy", GradientControl::kMenu_Copy);    TODO: !!
            // editMenu->addItem("Paste", GradientControl::kMenu_Paste);
            editMenu->addItem("Distribute", GradientControl::kMenu_Distribute);
            editMenu->addItem("Stretch", GradientControl::kMenu_Stretch);
            editMenu->addItem("Flip", GradientControl::kMenu_Flip);
            editMenu->setAction(_menuAction, this);

            menu->addSeparator();
            Menu* spotsMenu = menu->addSubMenu("Selected Spots");
            spotsMenu->addItem("Delete", GradientControl::kMenu_DeleteSpots);
            spotsMenu->addItem("Set Single Color", GradientControl::kMenu_SpotsSetSingleColor);
            spotsMenu->addItem("Set Two Color", GradientControl::kMenu_SpotsSetTwoColors);

            menu->addSeparator();
            Menu* fileMenu = menu->addSubMenu("File");
            fileMenu->addItem("Load ...", GradientControl::kMenu_LoadFile);
            fileMenu->addItem("Save ...", GradientControl::kMenu_SaveFile);
            fileMenu->setAction(_menuAction, this);

            menu->setAction(_menuAction, this);
        }

        return menu;
    }


    void GradientControl::_updateContextMenu() noexcept {

        int32_t stop_count = 0;
        int32_t selected_stop_count = 0;
        if (m_gradient != nullptr) {
            stop_count = m_gradient->stopCount();
            selected_stop_count = m_gradient->selectedStopCount();
        }

        bool usable = stop_count > 0;

        if (m_context_menu != nullptr) {
            m_context_menu->setItemEnabled(GradientControl::kMenu_Distribute, usable);
            m_context_menu->setItemEnabled(GradientControl::kMenu_Stretch, usable);
            m_context_menu->setItemEnabled(GradientControl::kMenu_Flip, usable);
            m_context_menu->setItemEnabled(GradientControl::kMenu_DeleteSpots, usable);
            m_context_menu->setItemEnabled(GradientControl::kMenu_SpotsSetSingleColor, usable);
            m_context_menu->setItemEnabled(GradientControl::kMenu_SpotsSetTwoColors, usable);
        }
    }


    void GradientControl::loadFromFile() noexcept {

        // TODO: Implement
        printf("loadFromFile\n");
        // gradient->setFromDataFile([[fileURL path] UTF8String]);
    }


    void GradientControl::saveToFileAs() noexcept {

        if (m_gradient == nullptr) {
            return;
        }

        // TODO: Implement!
        /*
        SavePanel* savePanel = new(std::nothrow) GrSavePanel();    // TODO: dirKey???
        if (savePanel) {

            savePanel->setFileTypes("data,DATA");
    //        savePanel->setNameFieldFromFilePath(mFilePath);    // TODO: ???

            if (savePanel->runModal()) {

                auto filePath = savePanel->getFilePath();
                m_gradient->saveDataFile(filePath);

    //            savePanel->dirPathToUserDefaults();    // TODO: ???
            }
        }
         */
    }

} // End of namespace
