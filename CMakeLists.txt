cmake_minimum_required(VERSION 3.15)
project(libgrain LANGUAGES CXX)

# Use modern C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


if (APPLE)
    include_directories(
            /opt/local/include
            /opt/local/include/postgresql15/
            /opt/homebrew/include
    )
    link_directories(
            /opt/local/lib
            /opt/homebrew/lib
    )
elseif (UNIX AND NOT APPLE)
    include_directories(
            /usr/include
            /usr/include/postgresql/
            /usr/local/include
    )
    link_directories(
            /usr/lib
            /usr/local/lib
    )
endif()


# Create static library
add_library(libgrain STATIC
        src/Grain.cpp

        src/Geometry.cpp

        src/App/App.cpp

        src/2d/Line.cpp
        src/2d/Rect.cpp
        src/2d/Circle.cpp
        src/2d/Triangle.cpp
        src/2d/Quadrilateral.cpp
        src/2d/RectEdges.cpp
        src/2d/Dimension.cpp
        src/2d/Polygon.cpp
        src/2d/GraphicPath.cpp
        src/2d/GraphicPathPoint.cpp
        src/2d/CatmullRomCurve.cpp
        src/2d/Superellipse.cpp

        src/2d/Data/CVF2.cpp
        src/2d/Data/CVF2File.cpp
        src/2d/Data/CVF2TileManager.cpp
        src/2d/Data/ValueGrid.cpp
        src/2d/RangeRect.cpp

        src/3d/Cube.cpp
        src/3d/RangeCube.cpp

        src/Bezier/Bezier.cpp
        src/Bezier/BezierValueCurve.cpp

        src/Color/Color.cpp
        src/Color/RGB.cpp
        src/Color/RGBA.cpp
        src/Color/HSV.cpp
        src/Color/HSL.cpp
        src/Color/OKColor.cpp
        src/Color/CIExyY.cpp
        src/Color/CIEXYZ.cpp
        src/Color/LMS.cpp
        src/Color/YUV.cpp
        src/Color/NamedColor.cpp
        src/Color/CDL.cpp
        src/Color/mixbox.cpp
        src/Color/Gradient.cpp
        src/Color/RGBLUT1.cpp
        src/Color/RGBRamp.cpp

        src/Core/Hardware.cpp
        src/Core/Log.cpp
        src/Core/ThreadPool.cpp

        src/CSS/CSSColor.cpp
        src/CSS/CSS.cpp

        src/Database/PostgreSQL.cpp

        src/DSP/RingBuffer.cpp
        src/DSP/WeightedSamples.cpp
        src/DSP/LevelCurve.cpp
        src/DSP/Partials.cpp
        src/DSP/LUT1.cpp
        src/DSP/DSP.cpp
        src/DSP/FFT.cpp
        src/DSP/Freq.cpp

        src/File/File.cpp
        src/File/TiffFile.cpp
        src/File/XYZFile.cpp

        src/Geo/Geo.cpp
        src/Geo/GeoMetaTile.cpp
        src/Geo/GeoProj.cpp

        src/GUI/Screen.cpp
        src/GUI/Window.cpp
        src/GUI/Components/Component.cpp
        src/GUI/Components/ValueComponent.cpp
        src/GUI/Components/ScrollBar.cpp
        src/GUI/Components/Button.cpp
        src/GUI/Components/Checkbox.cpp
        src/GUI/Components/TextField.cpp
        src/GUI/Views/View.cpp
        src/GUI/Views/ScrollView.cpp
        src/GUI/Views/SplitView.cpp
        src/GUI/Event.cpp
        src/GUI/GUIStyle.cpp

        src/Graphic/GraphicContext.cpp
        src/Graphic/CairoContext.cpp
        src/Graphic/Font.cpp

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

        src/String/String.cpp
        src/String/StringList.cpp
        src/String/CSVString.cpp

        src/Time/Timestamp.cpp
        src/Time/DateTime.cpp

        src/Type/Data.cpp
        src/Type/Type.cpp
        src/Type/Fix.cpp
        src/Type/Flags.cpp
        src/Type/HiResValue.cpp
        src/Type/Object.cpp
        src/Type/Range.cpp
        src/Type/FixProperty.cpp

        src/Signal/Signal.cpp
        src/Signal/Audio.cpp
        src/Signal/SignalFilter.cpp
        src/Signal/SignalButterworthFilter.cpp
        src/Signal/SignalLowPassFilter.cpp
        src/Signal/SignalBandPassFilter.cpp
        src/Signal/SignalAllPassFilter.cpp
        src/Signal/SignalLadderFilter.cpp
)

# Add the warning flag for GCC/Clang
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(libgrain PRIVATE -Wmultichar)
endif()


if (APPLE)
    target_sources(libgrain PRIVATE
            src/App/macos_app.mm
            src/GUI/macos_window.mm
            src/GUI/macos_view.mm
            src/GUI/macos_event.mm
            src/Image/macos_image_loader.mm
    )

    target_link_libraries(libgrain
            "-framework Cocoa"
            "-framework Foundation"
            "-framework Cocoa"
            "-framework AudioToolbox"
            "-framework Accelerate"
    )
endif()


# Force the output file to be exactly libgrain.a
set_target_properties(libgrain PROPERTIES OUTPUT_NAME grain)

target_include_directories(libgrain
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/libgrain>
)

# Install libgrain.a to /usr/local/lib, headers to /usr/local/include/libgrain
install(TARGETS libgrain
        EXPORT libgrainTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include/libgrain
)

# Install public headers under /usr/local/include/libgrain
install(DIRECTORY include/ DESTINATION include/libgrain)

# Export targets for find_package support
install(EXPORT libgrainTargets
        FILE libgrainTargets.cmake
        NAMESPACE libgrain::
        DESTINATION lib/cmake/libgrain
)

# Configure and install package config files
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/libgrainConfigVersion.cmake"
        VERSION 1.0.0
        COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libgrainConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/libgrainConfig.cmake"
        INSTALL_DESTINATION lib/cmake/libgrain
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libgrainConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/libgrainConfigVersion.cmake"
        DESTINATION lib/cmake/libgrain
)