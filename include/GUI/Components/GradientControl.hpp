//
//  GradientControl.hpp
//
//  Created by Roald Christesen on 18.01.2013
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>
//

#ifndef GrainGradientControl_hpp
#define GrainGradientControl_hpp

#include "GUI/Components/Component.hpp"


namespace Grain {

    class ColorWheel;
    class PopUpButton;
    class Slider;
    class TextField;
    class GradientStop;
    class Menu;


    class GradientControl : public Component {
    public:
        enum {
            kMenu_Options = 0,
            kMenu_Reset,
            kMenu_SelectAll,
            kMenu_RevertSelection,
            kMenu_SelectEachSecond,
            kMenu_Distribute,
            kMenu_Stretch,
            kMenu_Flip,
            kMenu_DeleteSpots,
            kMenu_SpotsSetSingleColor,
            kMenu_SpotsSetTwoColors,
            kMenu_LoadFile,
            kMenu_SaveFile
        };

        enum {
            kMouseMode_Undefined,
            kMouseMode_Stop,

        };

    protected:
        double spot_icon_height_ = 20.0;
        double spot_icon_width_ = 20.0 * 0.7;
        Rectd gradient_rect_;      ///< The area where the gradient is beeing visualized
        Rectd spots_rect_;         ///< The area where the gradient spots are presented
        double gradient_width_;    ///< Shortcode for m_gradient_rect.width()

        int32_t mouse_mode_ = kMouseMode_Undefined;    ///< Current mouse mode
        bool drag_started_ = false;                    ///< Indicates, that dragging was started

        Menu* context_menu_ = nullptr;

        // Connected GUI elements
        ColorWheel* color_wheel_ = nullptr;
        Slider* step_slider_ = nullptr;
        TextField* step_textfield_ = nullptr;


    public:
        GradientControl(const Rectd& rect) noexcept;
        virtual ~GradientControl() noexcept;

        const char* className() const noexcept override { return "GradientControl"; }


        static GradientControl* add(View* view, const Rectd& rect);


        bool setEnabled(bool enabled) noexcept override;
        void geometryChanged() noexcept override;

        void drawStop(GraphicContext& gc, const GradientStop* stop) noexcept;
        void draw(GraphicContext* gc, const Rectd& dirty_rect) noexcept override;
        void handleMouseDown(const Event& event) noexcept override;
        void handleRightMouseDown(const Event& event) noexcept override;
        void handleMouseUp(const Event& event) noexcept override;
        void handleMouseDrag(const Event& event) noexcept override;

        void setByComponent(Component* component) noexcept override;
        void updateRepresentations(const Component* excluded_component) noexcept override;

        int32_t stopIndexAtPos(Vec2d pos) const noexcept;
        Rectd stopRect(const GradientStop* stop) const noexcept;

        double viewPosToX(const GradientStop* stop) const noexcept;
        double viewXToPos(double x) const noexcept;

        void setColorWheel(ColorWheel* color_wheel) noexcept;
        void setStepSlider(Slider* slider) noexcept;
        void setStepTextField(TextField* textfield) noexcept;
        void setGradientColor(int32_t index, const RGB& rgb) noexcept;
        void setGradientColorHSV(int32_t index, const HSV& hsv) noexcept;

        void updateStopParameters(const GradientStop* stop) noexcept;

        static void _menuAction(Menu* menu, int32_t tag, void* ref) noexcept;
        Menu* _buildContextMenu() noexcept;
        void _updateContextMenu() noexcept;

        void loadFromFile() noexcept;
        void saveToFileAs() noexcept;
    };

} // End of namespace

#endif // GrainGradientControl_hpp