//
//  GrainLib.hpp
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

#include "Grain.hpp"

#include <Geometry.hpp>

#include "App/App.hpp"

#include "Audio/AudioInterface.hpp"

#include "2d/Line.hpp"
#include "2d/Rect.hpp"
#include "2d/PosRect.hpp"
#include "2d/Circle.hpp"
#include "2d/Triangle.hpp"
#include "2d/Quadrilateral.hpp"
#include "2d/Polygon.hpp"
#include "2d/Border.hpp"
#include "2d/Dimension.hpp"
#include "2d/GraphicPathPoint.hpp"
#include "2d/GraphicPath.hpp"
#include "2d/GraphicCompoundPath.hpp"
#include "2d/CatmullRomCurve.hpp"
#include "2d/RangeRect.hpp"
#include "2d/Superellipse.hpp"
#include "2d/Arc.hpp"
#include "2d/PoissonDisc.hpp"
#include "2d/Delaunay.hpp"

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

#include "Data/DataComposer.hpp"

#include "Database/PostgreSQL.hpp"
#include "Database/DBaseFile.hpp"

#include "DSP/DSP.hpp"
#include "DSP/FFT.hpp"
#include "DSP/Freq.hpp"
#include "DSP/LinearScaler.hpp"
#include "DSP/LUT1.hpp"
#include "DSP/Partials.hpp"
#include "DSP/LevelCurve.hpp"
#include "DSP/WeightedSamples.hpp"
#include "DSP/RingBuffer.hpp"

#include "File/File.hpp"
#include "File/TiffFile.hpp"
#include "File/XYZFile.hpp"
#include "File/PolygonsFile.hpp"

#include "Geo/Geo.hpp"
#include "Geo/GeoMetaTile.hpp"
#include "Geo/GeoProj.hpp"
#include "Geo/GeoShape.hpp"
#include "Geo/GeoShapeFile.hpp"
#include "Geo/GeoTileRenderer.hpp"
#include "Geo/WKBParser.hpp"

#include "Graphic/GraphicContext.hpp"
#include "Graphic/AppleCGContext.hpp"
#include "Graphic/CairoContext.hpp"
#include "Graphic/Font.hpp"
#include "Graphic/StrokeStyle.hpp"
#include "Graphic/AnimationFrameDriver.hpp"

#include "GUI/Screen.hpp"
#include "GUI/Window.hpp"
#include "GUI/Event.hpp"
#include "GUI/Components/Component.hpp"
#include "GUI/Components/ValueComponent.hpp"
#include "GUI/Components/Component.hpp"
#include "GUI/Components/ScrollBar.hpp"
#include "GUI/Components/Button.hpp"
#include "GUI/Components/Checkbox.hpp"
#include "GUI/Components/Toggle.hpp"
#include "GUI/Components/TextField.hpp"
#include "GUI/Components/Slider.hpp"
#include "GUI/Views/View.hpp"
#include "GUI/Views/Viewport.hpp"
#include "GUI/Views/ScrollView.hpp"
#include "GUI/Views/SplitView.hpp"
#include "GUI/GUIStyle.hpp"

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

#include "Midi/Midi.hpp"

#include "Scripting/Lua.hpp"
#include "Scripting/Toml.hpp"

#include "Signal/Signal.hpp"
#include "Signal/Audio.hpp"
#include "Signal/SignalFilter.hpp"
#include "Signal/SignalLowPassFilter.hpp"
#include "Signal/SignalBandPassFilter.hpp"
#include "Signal/SignalAllPassFilter.hpp"
#include "Signal/SignalButterworthFilter.hpp"
#include "Signal/SignalLadderFilter.hpp"
#include "Signal/SignalFormantFilter.hpp"
#include "Signal/SignalIR.hpp"
#include "Signal/SignalWave.hpp"

#include "String/String.hpp"
#include "String/StringList.hpp"
#include "String/CSVString.hpp"
#include "String/CSVData.hpp"

#include "SVG/SVG.hpp"
#include "SVG/SVGElement.hpp"
#include "SVG/SVGDefsElement.hpp"
#include "SVG/SVGRectElement.hpp"
#include "SVG/SVGCircleElement.hpp"
#include "SVG/SVGEllipseElement.hpp"
#include "SVG/SVGLineElement.hpp"
#include "SVG/SVGPolygonElement.hpp"
#include "SVG/SVGPathElement.hpp"
#include "SVG/SVGGradient.hpp"
#include "SVG/SVGGroupElement.hpp"
#include "SVG/SVGPaintElement.hpp"
#include "SVG/SVGPaintServer.hpp"
#include "SVG/SVGPaintStyle.hpp"
#include "SVG/SVGRootElement.hpp"


#include "Time/TimeMeasure.hpp"
#include "Time/Timestamp.hpp"
#include "Time/DateTime.hpp"

#include "Type/Data.hpp"
#include "Type/HiResValue.hpp"
#include "Type/KeyValue.hpp"
#include "Type/ByteOrder.hpp"
#include "Type/Object.hpp"
#include "Type/Fix.hpp"
#include "Type/Flags.hpp"
#include "Type/Range.hpp"
#include "Type/Type.hpp"
#include "Type/List.hpp"
#include "Type/FixProperty.hpp"


#endif