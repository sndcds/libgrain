cmake_minimum_required(VERSION 3.18)
project(libgrain LANGUAGES C CXX OBJCXX)

# -------------------------------------------------------
# C++ standard (target-based, not global)
# -------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
endif()

# -------------------------
# Dependencies (vcpkg ONLY)
# -------------------------
# -------------------------------------------------------
# Dependencies
# -------------------------------------------------------
find_package(PkgConfig REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

pkg_check_modules(LUA REQUIRED IMPORTED_TARGET lua)
pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo)
pkg_check_modules(PANGO REQUIRED IMPORTED_TARGET pango pangocairo)
pkg_check_modules(PROJ REQUIRED IMPORTED_TARGET proj)
pkg_check_modules(LIBPQ REQUIRED IMPORTED_TARGET libpq)
pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)
pkg_check_modules(LIBJPEG REQUIRED IMPORTED_TARGET libjpeg)
pkg_check_modules(LIBPNG REQUIRED IMPORTED_TARGET libpng)
pkg_check_modules(LIBTIFF REQUIRED IMPORTED_TARGET libtiff-4)
pkg_check_modules(WEBP REQUIRED IMPORTED_TARGET libwebp)
pkg_check_modules(LIBRAW REQUIRED IMPORTED_TARGET libraw)

# -------------------------
# Library target
# -------------------------
add_library(libgrain STATIC)
add_library(libgrain::grain ALIAS libgrain)

target_include_directories(libgrain PUBLIC
        ${CAIRO_INCLUDE_DIRS}
)

target_sources(libgrain PRIVATE
        src/Grain.cpp
        src/Geometry.cpp
        src/App/App.cpp

        src/Audio/AudioInterface.cpp
        src/Audio/AudioEngine.cpp
        src/Audio/Mac/AudioCoreAudio.cpp
        src/Audio/Mac/AudioStreamCoreAudio.cpp

        src/2d/Bounds2.cpp
        src/2d/PosRect.cpp
        src/2d/Polygon.cpp
        src/2d/Arc.cpp
        src/2d/GraphicPathPoint.cpp
        src/2d/GraphicPath.cpp
        src/2d/GraphicCompoundPath.cpp
        src/2d/CatmullRomCurve.cpp
        src/2d/Superellipse.cpp
        src/2d/PoissonDisc.cpp
        src/2d/Delaunay.cpp

        src/2d/Data/CVF2.cpp
        src/2d/Data/CVF2File.cpp
        src/2d/Data/CVF2TileManager.cpp
        src/2d/Data/ValueGrid.cpp

        src/3d/Cube.cpp
        src/3d/Bounds3.cpp

        src/Bezier/Bezier.cpp
        src/Bezier/BezierValueCurve.cpp

        src/Color/CDL.cpp
        src/Color/CIExyY.cpp
        src/Color/CIEXYZ.cpp
        src/Color/Color.cpp
        src/Color/Gradient.cpp
        src/Color/HSL.cpp
        src/Color/HSV.cpp
        src/Color/LMS.cpp
        src/Color/mixbox.cpp
        src/Color/NamedColor.cpp
        src/Color/OKColor.cpp
        src/Color/RGB.cpp
        src/Color/RGBA.cpp
        src/Color/RGBLUT1.cpp
        src/Color/RGBRamp.cpp
        src/Color/YUV.cpp

        src/Core/Hardware.cpp
        src/Core/Log.cpp
        src/Core/ThreadPool.cpp

        src/CSS/CSS.cpp
        src/CSS/CSSColor.cpp

        src/Data/DataComposer.cpp
        src/Data/JSONWalker.cpp

        src/Database/DBaseFile.cpp
        src/Database/PostgreSQL.cpp
        src/Database/PSQL.cpp

        src/DSP/DSP.cpp
        src/DSP/EnvelopeFollower.cpp
        src/DSP/FFT.cpp
        src/DSP/Freq.cpp
        src/DSP/LevelCurve.cpp
        src/DSP/LUT1.cpp
        src/DSP/Partials.cpp
        src/DSP/RingBuffer.cpp
        src/DSP/WeightedSamples.cpp

        src/File/File.cpp
        src/File/PolygonsFile.cpp
        src/File/TiffFile.cpp
        src/File/XYZFile.cpp

        src/Geo/Geo.cpp
        src/Geo/GeoMetaTile.cpp
        src/Geo/GeoProj.cpp
        src/Geo/GeoShape.cpp
        src/Geo/GeoShapeFile.cpp
        src/Geo/GeoTileRenderer.cpp
        src/Geo/WKBParser.cpp

        src/GUI/Components/AngleDial.cpp
        src/GUI/Components/Button.cpp
        src/GUI/Components/Checkbox.cpp
        src/GUI/Components/ColorWell.cpp
        src/GUI/Components/Component.cpp
        src/GUI/Components/ScrollBar.cpp
        src/GUI/Components/Slider.cpp
        src/GUI/Components/TextField.cpp
        src/GUI/Components/Toggle.cpp
        src/GUI/Components/ValueComponent.cpp

        src/GUI/Views/ScrollView.cpp
        src/GUI/Views/SplitView.cpp
        src/GUI/Views/View.cpp
        src/GUI/Views/Viewport.cpp

        src/Graphic/AnimationFrameDriver.cpp
        src/Graphic/AppleCGContext.cpp
        src/Graphic/CairoContext.cpp
        src/Graphic/Font.cpp
        src/Graphic/GraphicContext.cpp
        src/Graphic/StrokeStyle.cpp

        src/GUI/Event.cpp
        src/GUI/GUIStyle.cpp
        src/GUI/Screen.cpp
        src/GUI/Window.cpp

        src/Image/Image.cpp

        src/Math/Mat3.cpp
        src/Math/Mat4.cpp
        src/Math/Math.cpp
        src/Math/NumberSeries.cpp
        src/Math/Random.cpp
        src/Math/Vec2.cpp
        src/Math/Vec2Fix.cpp
        src/Math/Vec3.cpp
        src/Math/Vec3Fix.cpp

        src/Midi/Midi.cpp

        src/Movie/MovieWriter.cpp

        src/Scripting/Lua.cpp
        src/Scripting/Toml.cpp

        src/Signal/Audio.cpp
        src/Signal/Signal.cpp
        src/Signal/SignalAllPassFilter.cpp
        src/Signal/SignalBandPassFilter.cpp
        src/Signal/SignalButterworthFilter.cpp
        src/Signal/SignalFilter.cpp
        src/Signal/SignalFormantFilter.cpp
        src/Signal/SignalIR.cpp
        src/Signal/SignalLadderFilter.cpp
        src/Signal/SignalLowPassFilter.cpp
        src/Signal/SignalWave.cpp

        src/String/CSVData.cpp
        src/String/CSVString.cpp
        src/String/String.cpp
        src/String/StringList.cpp

        src/SVG/SVG.cpp
        src/SVG/SVGCircleElement.cpp
        src/SVG/SVGDefsElement.cpp
        src/SVG/SVGElement.cpp
        src/SVG/SVGEllipseElement.cpp
        src/SVG/SVGGradient.cpp
        src/SVG/SVGGroupElement.cpp
        src/SVG/SVGLineElement.cpp
        src/SVG/SVGPaintElement.cpp
        src/SVG/SVGPaintServer.cpp
        src/SVG/SVGPaintStyle.cpp
        src/SVG/SVGPathElement.cpp
        src/SVG/SVGPolygonElement.cpp
        src/SVG/SVGRectElement.cpp
        src/SVG/SVGRootElement.cpp

        src/Time/DateTime.cpp
        src/Time/Timestamp.cpp

        src/Type/Data.cpp
        src/Type/Fix.cpp
        src/Type/FixProperty.cpp
        src/Type/Flags.cpp
        src/Type/HiResValue.cpp
        src/Type/Object.cpp
        src/Type/Range.cpp
        src/Type/Type.cpp

        src/extern/tinyxml2.cpp
)


# -------------------------------------------------------
# Public include interface (ONLY your headers)
# -------------------------------------------------------
target_include_directories(libgrain
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/libgrain>
)


# -------------------------------------------------------
# Compile features (modern, target-scoped)
# -------------------------------------------------------
target_compile_features(libgrain PUBLIC cxx_std_20)

target_compile_options(libgrain PRIVATE
        $<$<CONFIG:Release>:-O3>
)

# -------------------------------------------------------
# Dependencies (STRICTLY PRIVATE unless needed in headers)
# -------------------------------------------------------
target_link_libraries(libgrain PRIVATE
        ${CAIRO_LIBRARIES}
        PkgConfig::LIBPNG
        PkgConfig::LIBJPEG
        PkgConfig::LIBTIFF
        PkgConfig::WEBP
        PkgConfig::LIBRAW
        PkgConfig::PANGO
        PkgConfig::LUA
        PkgConfig::CURL
        PkgConfig::PROJ
        PkgConfig::LIBPQ
        libpqxx::pqxx
        AVFORMAT
        AVCODEC
        AVUTIL
)

# -------------------------------------------------------
# Apple-specific sources + frameworks
# -------------------------------------------------------
if(APPLE)
    target_sources(libgrain PRIVATE
            src/Graphic/AppleCGContext.cpp
            src/Graphic/macos_image.mm
            src/App/macos_app.mm
            src/GUI/macos_ui_helper.mm
            src/GUI/macos_window.mm
            src/GUI/macos_view.mm
            src/GUI/macos_event.mm
            src/Image/macos_image_loader.mm
    )

    target_link_libraries(libgrain PRIVATE
            "-framework Cocoa"
            "-framework Foundation"
            "-framework AudioToolbox"
            "-framework Accelerate"
            "-framework CoreVideo"
            "-framework CoreAudio"
            "-framework CoreMIDI"
            "-framework CoreFoundation"
            "-framework CoreGraphics"
    )
endif()

# -------------------------------------------------------
# Output name
# -------------------------------------------------------
set_target_properties(libgrain PROPERTIES
        OUTPUT_NAME grain
)

# -------------------------------------------------------
# Install (modern package-friendly)
# -------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS libgrain
        EXPORT libgrainTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libgrain
)

install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libgrain
)

install(EXPORT libgrainTargets
        FILE libgrainTargets.cmake
        NAMESPACE libgrain::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libgrain
)

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/libgrainConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/libgrainConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libgrain
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/libgrainConfigVersion.cmake
        VERSION 1.0.0
        COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/libgrainConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/libgrainConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libgrain
)