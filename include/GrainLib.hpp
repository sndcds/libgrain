//
//  Grain.hpp
//
//  Created by Roald Christesen on 14.01.24.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 10.07.2025
//

#ifndef GrainLib_hpp
#define GrainLib_hpp

#include <Geometry.hpp>

#include "App/App.hpp"

#include "2d/Line.hpp"
#include "2d/Rect.hpp"
#include "2d/Circle.hpp"
#include "2d/Triangle.hpp"
#include "2d/Quadrilateral.hpp"
#include "2d/RectEdges.hpp"
#include "2d/Dimension.hpp"
#include "2d/Polygon.hpp"
#include "2d/GraphicPath.hpp"
#include "2d/GraphicPathPoint.hpp"
#include "2d/CatmullRomCurve.hpp"
#include "2d/RangeRect.hpp"
#include "2d/Superellipse.hpp"

#include "2d/Data/CVF2.hpp"
#include "2d/Data/CVF2File.hpp"
#include "2d/Data/CVF2TileManager.hpp"
#include "2d/Data/ValueGrid.hpp"
#include "File/XYZFile.hpp"


#include "3d/Cube.hpp"
#include "3d/RangeCube.hpp"

#include "Bezier/Bezier.hpp"
#include "Bezier/BezierValueCurve.hpp"

#include "Color/Color.hpp"
#include "Color/RGB.hpp"
#include "Color/HSV.hpp"
#include "Color/HSL.hpp"
#include "Color/OKColor.hpp"
#include "Color/CIExyY.hpp"
#include "Color/CIEXYZ.hpp"
#include "Color/LMS.hpp"
#include "Color/YUV.hpp"
#include "Color/NamedColor.hpp"
#include "Color/CDL.hpp"
#include "Color/Gradient.hpp"
#include "Color/RGBLUT1.hpp"
#include "Color/RGBRamp.hpp"

#include "Core/Hardware.hpp"
#include "Core/Log.hpp"
#include "Core/ThreadPool.hpp"

#include "CSS/CSSColor.hpp"
#include "CSS/CSS.hpp"

#include "DSP/DSP.hpp"
#include "DSP/FFT.hpp"
#include "DSP/Freq.hpp"
#include "DSP/LUT1.hpp"
#include "DSP/Partials.hpp"
#include "DSP/LevelCurve.hpp"
#include "DSP/WeightedSamples.hpp"
#include "DSP/RingBuffer.hpp"

#include "File/File.hpp"
#include "File/TiffFile.hpp"

#include "Geo/Geo.hpp"
#include "Geo/GeoMetaTile.hpp"
#include "Geo/GeoProj.hpp"

#include "Graphic/GraphicContext.hpp"
#include "Graphic/CairoContext.hpp"
#include "Graphic/Font.hpp"

#include "GUI/Screen.hpp"
#include "GUI/Window.hpp"
#include "GUI/Event.hpp"
#include "GUI/View.hpp"
#include "GUI/Component.hpp"
#include "GUI/ValueComponent.hpp"
#include "GUI/Component.hpp"
#include "GUI/Style.hpp"
#include "GUI/Components/Button.hpp"
#include "GUI/Components/Checkbox.hpp"

#include "Image/Image.hpp"

#include "Math/Random.hpp"
#include "Math/Mat3.hpp"
#include "Math/Mat4.hpp"
#include "Math/Math.hpp"
#include "Math/NumberSeries.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec2Fix.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec3Fix.hpp"

#include "Signal/Signal.hpp"
#include "Signal/Audio.hpp"
#include "Signal/SignalFilter.hpp"
#include "Signal/SignalLowPassFilter.hpp"
#include "Signal/SignalBandPassFilter.hpp"
#include "Signal/SignalAllPassFilter.hpp"
#include "Signal/SignalButterworthFilter.hpp"
#include "Signal/SignalLadderFilter.hpp"

#include "String/CSVString.hpp"
#include "String/String.hpp"
#include "String/StringList.hpp"

#include "Time/Timestamp.hpp"
#include "Time/DateTime.hpp"

#include "Type/Data.hpp"
#include "Type/HiResValue.hpp"
#include "Type/KeyValue.hpp"
#include "Type/Object.hpp"
#include "Type/Fix.hpp"
#include "Type/Flags.hpp"
#include "Type/Range.hpp"
#include "Type/Type.hpp"
#include "Type/FixProperty.hpp"


#endif