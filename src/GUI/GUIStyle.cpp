//
//  GUIStyle.hpp
//
//  Created by Roald Christesen on from 28.07.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Grain.hpp"
#include "GUI/GUIStyle.hpp"
#include "Graphic/Font.hpp"
#include "App/App.hpp"


namespace Grain {


GUIStyle::GUIStyle() {
    setViewColor({ 0.95f, 0.95f, 0.95f, 1 });
    setForegroundColor(GUIStyle::State::kNormal, { 0, 0, 0, 1 });
    setBackgroundColor(GUIStyle::State::kNormal, { 0.85f, 0.85f, 0.85f, 1 });
    setLabelColor({ 0.3f, 0.3f, 0.3f, 1 });
    setTextBackgroundColor({ 1, 1, 1, 1 });
    setTextColor({ 0, 0, 0, 1 });
    setTextSelectionColor({ 1, 1, 1, 1 });
    setTextSelectionBackgroundColor({ 0.5f, 0.4f, 1, 1 });
    setTextInfoColor({ 0.6f, 0.6f, 0.6f, 1 });
    setTextCursorColor({ 0.06f, 0, 0.2f, 1 });
    setFont(App::uiFont());
}


} // End of namespace Grain
