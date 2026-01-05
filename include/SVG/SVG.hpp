//
//  SVG.hpp
//
//  Created by Roald Christesen on from 27.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

//  See: https://www.w3.org/TR/SVG2/
//  TODO: Support z-index.
//  TODO: Transformation class or struct!

#ifndef GrainSVG_hpp
#define GrainSVG_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Type/List.hpp"
#include "Math/Mat3.hpp"
#include "String/String.hpp"
#include "2d/Polygon.hpp"
#include "2d/GraphicCompoundPath.hpp"
#include "CSS/CSS.hpp"

#include "../Extern/tinyxml2.h"


namespace Grain {

    class SVG;
    class SVGRootElement;
    class SVGPaintServer;
    class SVGGradient;
    class Image;

    enum class SVGTransformType {
        Undefined,
        Matrix = 0,
        Translate,
        Scale,
        Rotate,
        SkewX,
        SkewY,
        Perspective
    };

    /**
     *  @brief Enum to define the type of gradient.
     */
    enum class SVGGradientType {
        Linear = 0, ///< Linear gradient
        Radial,     ///< Radial gradient
        First = Linear, Last = Radial, Count = Last + 1
    };

    /**
     *  @brief Enum to define the color interpolation mode.
     */
    enum class SVGGradientInterpolationMode {
        sRGB = 0,
        LinearRGB,
        First = sRGB, Last = LinearRGB, Count = Last + 1
    };

    enum class SVGGradientUnits {
        ObjectBoundingBox = 0,
        UserSpaceOnUse,
        First = ObjectBoundingBox, Last = UserSpaceOnUse, Count = Last + 1
    };

    enum class SVGSpreadMethod {
        Pad = 0,
        Reflect,
        Repeat,
        First = Pad, Last = Repeat, Count = Last + 1
    };


    /**
     *  @class SVGValuesParser
     *  @brief A utility class to parse numeric values from C-string data.
     *
     *  The `SVGValuesParser` class is designed to extract numeric values from a given C-string,
     *  following the specific rules and constraints of SVG syntax. It supports simple numeric
     *  parsing without scientific notation and ensures proper handling of invalid characters
     *  or formatting errors.
     *
     *  ### Features:
     *  - Parses numeric values starting with `-`, `+`, or digits `0-9`.
     *  - Ensures invalid characters are reported through error codes.
     *  - Tracks the number of successfully parsed values.
     *  - Supports re-initialization with new data for repeated parsing.
     *
     *  ### Usage Example:
     *  ```cpp
     *  SVGValuesParser parser(svg);
     *  parser.setup("10, 20, -5, +3");
     *
     *  double value;
     *  while (parser.next(value) == SVGValuesParser::kNextResult_Continue) {
     *      // Process each parsed value.
     *  }
     *  ```
     *
     *  ### Parsing Rules:
     *  - Values must start with a `-`, `+`, or a digit (`0-9`).
     *  - Scientific notation (e.g., `1e3`) is not supported.
     *  - Parsing stops upon encountering an invalid character or reaching the end of the data.
     *
     *  @note This class is designed to be used in conjunction with the `SVG` class.
     */
    class SVGValuesParser {

        friend class SVG;

    public:
        enum {
            kNextResult_NoMoreData = 0,
            kNextResult_UnknownCharacter = -1,
            kNextResult_Continue = 1
        };

    protected:
        int32_t m_value_count = 0;
        char m_buffer[256];
        int32_t m_buffer_pos = 0;
        const char* m_read_ptr = nullptr;
        bool m_run = false;


    public:public:
        SVGValuesParser() = default;
        SVGValuesParser(const char* str) { setup(str); }
        ~SVGValuesParser() = default;

        ErrorCode setup(const char* str) noexcept;
        int32_t next(double& out_next) noexcept;

        int32_t valueCount() const noexcept { return m_value_count; }
    };


    class SVGFunctionValuesParser {

        friend class SVG;

    public:
        enum {
            kNextFunctionResult_NoMoreData = 0,
            kNextFunctionResult_InvalidFunctionName = -1,
            kNextFunctionResult_InvalidFunctionNameLength = -2,
            kNextFunctionResult_EndOfDataBlockMissing = -3,
            kNextFunctionResult_InvalidDataLength = -4,
            kNextFunctionResult_Continue = 1,

            kMaxFunctionNameLength = 255,
            kMaxFunctionDataLength = 1023,
        };

    protected:
        int32_t m_function_count = 0;
        bool m_run = false;
        char m_function_name[kMaxFunctionNameLength + 1];
        char m_function_data[kMaxFunctionDataLength + 1];

        const char* m_read_ptr = nullptr;
        const char* m_function_name_ptr = nullptr;
        int32_t m_function_name_length = 0;
        const char* m_data_ptr = nullptr;
        int32_t m_data_length = 0;
        int32_t m_err_n = 0;


    public:public:
        SVGFunctionValuesParser() = default;
        SVGFunctionValuesParser(const char* str) { setup(str); }
        ~SVGFunctionValuesParser() {}

        ErrorCode setup(const char* str) noexcept;
        int32_t nextFunction() noexcept;
        const char* functionName() noexcept { return m_function_name; }
        const char* functionData() noexcept { return m_function_data; }
        int32_t functionCount() const noexcept { return m_function_count; }
        int32_t extractCSSValues(int32_t max, CSSValue* out_values) noexcept;
    };

    /**
     *  @class SVGPathParser
     *  @brief A parser class for SVG path data.
     *
     *  The `SVGPathParser` class is responsible for parsing and interpreting SVG path data (such as `M`, `L`, `C`, etc.)
     *  according to the SVG Path specification. It handles the parsing of individual path commands and their respective
     *  coordinate values (both relative and absolute) while storing the parsed path information into a `GraphicCompoundPath`
     *  object.
     *
     *  ### Usage Example:
     *  ```cpp
     *  SVGPathParser parser(svg, compoundPath);
     *  parser.parsePathData("M10 20 L30 40 C50 60 70 80 90 100");
     *  ```
     *
     *  ### Parsing Rules:
     *  - The class can handle both relative and absolute path commands.
     *  - Commands such as `M`, `L`, `C`, `Z`, etc., are parsed and the respective coordinates are recorded.
     *  - The parser processes commands and stores the path data (points, curves, etc.) in a `GraphicCompoundPath`
     *    object.
     *
     *  @note This class is designed to be used in conjunction with the `SVG` class.
     */
    class SVGPathParser {

        friend class SVG;

    public:
        enum {
            kErrValueOverflow = 1
        };

        enum {
            kMaxValueStrLength = 256
        };

    protected:
        SVG* m_svg = nullptr;           ///< Pointer to the SVG object hosting the parser
        const char* m_data = nullptr;   ///< Pointer to path data
        char m_curr_command;            ///< Current path command. Used in parsing path data
        char m_next_command;            ///< The next path command. Used in parsing path data
        bool m_relative_state;          ///< Flag signaling, if the current path command is reative or absolute
        bool m_value_state;             ///< Flag signaling, if parser is collecting a value
        char m_value_str[kMaxValueStrLength]{};
        int m_value_str_index;
        List<double>m_values;

        GraphicCompoundPath* m_compound_path = nullptr;
        GraphicPath* m_current_path = nullptr;

        Vec2d m_curr_pos = { 0.0, 0.0 };
        int32_t _m_unhandled_command_count = 0;

    public:
        explicit SVGPathParser(SVG* svg, GraphicCompoundPath* out_compound_path) {
            m_svg = svg;
            m_curr_command = 0;
            m_next_command = 0;
            m_relative_state = false;
            m_value_state = false;
            m_value_str_index = 0;
            m_compound_path = out_compound_path;
        }

        ~SVGPathParser() {
        }

        void setValuesCapacity() {
            m_values.reserve(64);
        }

        bool booleanAtIndex(int32_t index);
        double valueAtIndex(int32_t index);
        double xAtIndex(int32_t index);
        double yAtIndex(int32_t index);
        Vec2d posAtValueIndex(int32_t index);

        ErrorCode parsePathData(const char* str) noexcept;
        void _addValue();
        void _addSegment(const char command);
    };


    /**
     *  @class SVG
     *  @brief A class that represents an SVG document and handles its parsing, processing, and rendering.
     *
     *  The `SVG` class provides functionality to parse an SVG document, access its elements, and perform operations on them.
     *  It can load an SVG file, parse its XML data, manage the document's structure (such as the root group and other elements),
     *  and facilitate interaction with the contents of the SVG file.
     *
     *  ### Key Features:
     *  - Load and parse an SVG file.
     *  - Manage and manipulate the SVG document's structure (root group and its elements).
     *  - Provides error handling and status codes for SVG operations.
     *  - Functionality for drawing the SVG image onto a specified image buffer.
     *  - Utility to convert strings to double values.
     *
     *  ### Usage Example:
     *  ```cpp
     *  SVG svg("path_to_svg_file.svg");
     *  svg.parse();
     *  svg.drawImage(image);
     *  ```
     */
    class SVG : public Object {

        friend class SVGPathParser;

    public:
        enum {
            kErrSVGTagNotFound = 1,
            kErrXMLDocError,
            kErrXMLDocLoadFileError,
            kErrValueMismatch,          ///< When there are to few or to many values for a specific command
            kErrValueIndexOutOfRange,   ///< When there are to few or to many values for a specific command
            kErrCompoundPathIsNull,
            kErrCurrentPathIsNull,
            kErrAddGradientFailed,
        };

        enum {
            kXMLErr_None = 0,
            kXMLErr_NoAttribute,
            kXMLErr_WrongAttributeType,
            kXMLErr_FileNotFound,
            kXMLErr_FileCouldNotBeOpened,
            kXMLErr_FileReadError,
            kXMLErr_ParsingElement,
            kXMLErr_ParsingAttribute,
            kXMLErr_ParsingText,
            kXMLErr_ParsingCData,
            kXMLErr_ParsingComment,
            kXMLErr_ParsingDeclaration,
            kXMLErr_ParsingUnknown,
            kXMLErr_EmptyDocument,
            kXMLErr_MismatchedElement,
            kXMLErr_ErrorParsing,
            kXMLErr_CanNotConvertText,
            kXMLErr_NoTextNode,
            kXMLErr_ElementDepthExceeded,

            kXMLErr_Count
        };

    protected:
        String m_file_path;
        tinyxml2::XMLDocument* m_xml_doc = nullptr;

        SVGRootElement* m_svg_root = nullptr;   ///< The SVG root group
        int32_t m_group_iteration_depth = 0;

        ObjectList<SVGPaintServer*> m_paint_servers;  ///< List of paint servers

        // Error handling
        int32_t m_xml_error_id{};       ///< An error id
        String m_xml_error_message;     ///< An error message as text
        int32_t m_xml_error_line{};     ///< Line number in file, where the error occured

    public:
        SVG(const String& file_path) noexcept;
        ~SVG() noexcept;

        virtual const char* className() const noexcept { return "SVG"; }

        friend std::ostream& operator << (std::ostream& os, const SVG* o) {
            o == nullptr ? os << "SVG nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const SVG& o) {
            o.log(os);
            return os;
        }

        void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const;


        SVGGradient* addGradient(SVGGradientType type, int32_t capacity = 16) noexcept;

        SVGPaintServer* paintServerByID(const String& id) const noexcept;


        void clearError() noexcept;
        ErrorCode parse() noexcept;

        int32_t groupIterationDepth() const noexcept { return m_group_iteration_depth; }
        void incGroupIterationDepth() noexcept { m_group_iteration_depth++; }
        void decGroupIterationDepth() noexcept { m_group_iteration_depth--; }

        void draw(GraphicContext& gc) noexcept;

        static const char* gradientTypeName(SVGGradientType type) noexcept;
        static const char* gradientInterpolationModeName(SVGGradientInterpolationMode mode) noexcept;
        static const char* gradientUnitsName(SVGGradientUnits units) noexcept;
        static const char* spreadMethodName(SVGSpreadMethod method) noexcept;


        static double doubleFromStr(const char* str, double default_value) noexcept;
        static int32_t isValidFunctionName(const char* str) noexcept;

        static inline bool isTag(const char* str, const char* tag_name) noexcept {
            return strcasecmp(str, tag_name) == 0;
        }

        static void logXMLElementAttributes(std::ostream& os, const tinyxml2::XMLElement* xml_element) noexcept;
    };


} // End of namespace Grain


#endif // GrainSVG_hpp
