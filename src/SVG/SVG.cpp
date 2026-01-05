//
//  SVG.cpp
//
//  Created by Roald Christesen on from 27.12.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "SVG/SVG.hpp"
#include "SVG/SVGRootElement.hpp"
#include "SVG/SVGGradient.hpp"
#include "Image/Image.hpp"
#include "Graphic/GraphicContext.hpp"
#include "2d/GraphicPathPoint.hpp"
#include "Bezier/Bezier.hpp"
#include "Time/Timestamp.hpp"
#include "Core/Log.hpp"


namespace Grain {

    SVG::SVG(const String& file_path) noexcept {
        m_file_path = file_path;
    }

    SVG::~SVG() noexcept {
        // TODO: Check all objects etc.
        delete m_svg_root;
        delete m_xml_doc;
    }

    void SVG::log(std::ostream& os, int32_t indent, const char* label) const {
        Log l(os);
        l.header(label);
        l << "m_file_path: " << m_file_path << Log::endl;
        if (m_xml_error_id != kXMLErr_None) {
            l << "m_xml_error_id: " << m_xml_error_id << Log::endl;
            l++;
            l << "m_xml_error_line: " << m_xml_error_line << Log::endl;
            l << "m_xml_error_message: " << m_xml_error_message << Log::endl;
            l--;
        }
    }

    SVGGradient* SVG::addGradient(SVGGradientType type, int32_t capacity) noexcept {
        auto gradient = new(std::nothrow) SVGGradient(type, capacity);
        if (gradient) {
            m_paint_servers.push((SVGPaintServer*)gradient);
        }
        return gradient;
    }

    SVGPaintServer* SVG::paintServerByID(const String& id) const noexcept {
        for (auto& paint_server : m_paint_servers) {
            if (paint_server->m_id.compareAscii(id, 0, 1) == 0) {
                return paint_server;
            }
        }
        return nullptr;
    }

    void SVG::clearError() noexcept {
       m_xml_error_id = kXMLErr_None;
        m_xml_error_message.clear();
        m_xml_error_line = -1;
    }


    /**
     *  @brief Recursively parse an SVG element and all of its descendants.
     *
     *  This method starts at the specified root element and traverses
     *  through all of its child elements, recursively handling each child.
     *  Each element is processed according to the rules defined for its type
     *  (e.g., <rect>, <circle>, <g>, etc.). If an element has attributes
     *  or child elements, they are handled as well.
     */
    ErrorCode SVG::parse() noexcept {
        auto result = ErrorCode::None;

        try {
            clearError();

            // XML document
            m_xml_doc = new(std::nothrow) tinyxml2::XMLDocument();
            if (!m_xml_doc) { Exception::throwSpecific(kErrXMLDocError); }

            // Load the SVG file
            if (m_xml_doc->LoadFile(m_file_path.utf8()) != tinyxml2::XML_SUCCESS) {
                // Get detailed error information
                m_xml_error_id = m_xml_doc->ErrorID();
                m_xml_error_message = m_xml_doc->ErrorStr();
                m_xml_error_line = m_xml_doc->ErrorLineNum();
                Exception::throwSpecific(kErrXMLDocLoadFileError);
            }

            // The root group, which ist the files <svg> element
            m_svg_root = new(std::nothrow) SVGRootElement(nullptr);
            if (!m_svg_root) { Exception::throwStandard(ErrorCode::MemCantAllocate); }

            // Get the root <svg> element
            auto* xml_element = m_xml_doc->FirstChildElement("svg");
            if (!xml_element) { Exception::throwSpecific(kErrSVGTagNotFound); }

            m_svg_root->parse(this, xml_element);
        }
        catch (const Exception& e) {
            std::cout << "SVG::parse() err: " << e.codeAsInt() << std::endl;
            result = e.code();
        }

        if (m_svg_root != nullptr) {
            // TODO:
        }

        return result;
    }

    void SVG::draw(GraphicContext& gc) noexcept {
        if (m_svg_root) {
            // gc.translate(700, 700);
            // gc.scale(10);
            gc.save();
            m_svg_root->draw(this, gc);
            gc.restore();
        }
    }

    const char* SVG::gradientTypeName(SVGGradientType type) noexcept {
        static const char* _names[] = { "Linear", "Radial", "unknown" };
        if (type >= SVGGradientType::First && type <= SVGGradientType::Last) {
            return _names[(int32_t)type];
        }
        return _names[(int32_t)SVGGradientType::Count];
    }

    const char* SVG::gradientInterpolationModeName(SVGGradientInterpolationMode mode) noexcept {
        static const char* _names[] = { "sRGB", "LinearRGB", "unknown" };
        if (mode >= SVGGradientInterpolationMode::First && mode <= SVGGradientInterpolationMode::Last) {
            return _names[(int32_t)mode];
        }
        return _names[(int32_t)SVGGradientType::Count];
    }

    const char* SVG::gradientUnitsName(SVGGradientUnits units) noexcept {
        static const char* _names[] = { "ObjectBoundingBox", "UserSpaceOnUse", "unknown" };
        if (units >= SVGGradientUnits::First && units <= SVGGradientUnits::Last) {
            return _names[(int32_t)units];
        }
        return _names[(int32_t)SVGGradientUnits::Count];
    }

    const char* SVG::spreadMethodName(SVGSpreadMethod method) noexcept {
        static const char* _names[] = { "Pad", "Reflect", "Repeat", "unknown" };
        if (method >= SVGSpreadMethod::First && method <= SVGSpreadMethod::Last) {
            return _names[(int32_t)method];
        }
        return _names[(int32_t)SVGSpreadMethod::Count];
    }


    /**
     *  @brief Converts a C-string to a `double` value.
     *
     *  This method attempts to convert the provided C-string `str` to a `double`
     *  value. If the string is null or empty, it returns the specified
     *  `default_value` instead. If the string contains a valid representation of a
     *  number, `String::parseDoubleWithDotOrComma()` function to perform the
     *  conversion.
     *
     *  @param str The C-string to convert to a `double`. This string should represent a numeric value.
     *  @param default_value The value to return if the provided string is null or empty.
     *
     *  @return The converted `double` value if the string is valid; otherwise, returns the `default_value`.
     *
     *  ### Example:
     *  ```cpp
     *  double result = SVG::doubleFromStr("3.14", 0.0);  // result will be 3.14
     *  double invalidResult = SVG::doubleFromStr("", 0.0);  // invalidResult will be 0.0
     *  ```
     */
    double SVG::doubleFromStr(const char* str, double default_value) noexcept {
        if (str == nullptr || str[0] == '\0') {
            return default_value;
        }
        return String::parseDoubleWithDotOrComma(str);
    }

    ErrorCode SVGValuesParser::setup(const char* str) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!str) {
                Exception::throwStandard(ErrorCode::BadArgs);
            }

            m_buffer_pos = 0;
            m_read_ptr = str;
            m_run = true;
        }
        catch (const Exception& e) {
            result = e.code();
        }

        return result;
    }

    int32_t SVGValuesParser::next(double& out_next) noexcept {
        if (m_run == false) {
            return kNextResult_NoMoreData;
        }

        while (true) {
            auto c = *m_read_ptr++;

            if ((c >= '0' && c <= '9') || c == '-' | c == '.') {
                m_buffer[m_buffer_pos] = c;
                m_buffer_pos++;
            }
            else if (c == ' ' || c == ',' || c == '\0') {
                if (m_buffer_pos > 0) {
                    m_buffer[m_buffer_pos] = '\0';
                    out_next = String::parseDoubleWithDotOrComma(m_buffer);
                    m_buffer_pos = 0;
                    m_buffer[0] = '\0';
                    m_value_count++;

                    if (c == '\0') {
                        m_run = false;
                    }
                    return kNextResult_Continue;
                }
            }
            else {
                return kNextResult_UnknownCharacter;
            }
        }
    }

    ErrorCode SVGFunctionValuesParser::setup(const char* str) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!str) {
                Exception::throwStandard(ErrorCode::BadArgs);
            }

            m_read_ptr = str;
            m_function_name_ptr = str;
            m_function_name_length = 0;
            m_function_count = 0;
            m_data_length = 0;
            m_data_ptr = nullptr;
            m_err_n = 0;
            m_run = true;
        }
        catch (const Exception& e) {
            result = e.code();
        }

        return result;
    }

    int32_t SVGFunctionValuesParser::nextFunction() noexcept {
        m_function_data[0] = '\0';
        m_data_length = 0;

        if (m_run == false) {
            return kNextFunctionResult_NoMoreData;
        }

        // Skip spaces
        while (*m_read_ptr == ' ') {
            m_read_ptr++;
        }

        m_function_name_ptr = m_read_ptr;

        int32_t n = SVG::isValidFunctionName(m_read_ptr);
        if (n <= 0) {
            m_run = false;
            m_err_n++;
            return kNextFunctionResult_InvalidFunctionName;
        }

        if (n > kMaxFunctionNameLength) {
            m_err_n++;
            return kNextFunctionResult_InvalidFunctionNameLength;
        }

        strncpy(m_function_name, m_function_name_ptr, n);
        m_function_name[n] = '\0';
        m_data_ptr = m_function_name_ptr + n + 1;
        m_function_name_length = n;

        const char* end_ptr = strchr(m_data_ptr, ')');
        if (end_ptr == nullptr) {
            m_err_n++;
            return kNextFunctionResult_EndOfDataBlockMissing;
        }

        m_data_length = (int32_t)(end_ptr - m_data_ptr);
        if (m_data_length > kMaxFunctionDataLength) {
            m_err_n++;
            return kNextFunctionResult_InvalidDataLength;
        }

        strncpy(m_function_data, m_data_ptr, m_data_length);
        m_function_data[m_data_length] = '\0';
        // Replace comma with spaces for easy splitting the values
        String::replaceChar(m_function_data, ',', ' ');

        m_read_ptr = m_data_ptr + m_data_length + 1;
        m_function_count++;

        return kNextFunctionResult_Continue;
    }

    int32_t SVGFunctionValuesParser::extractCSSValues(int32_t max, CSSValue* out_values) noexcept {
        if (out_values == nullptr) {
            return -1; // Error code
        }

        auto ptr = (char*)functionData();
        int32_t index = 0;

        while (ptr != nullptr) {
            auto err = CSS::extractCSSValueFromStr(ptr, out_values[index], &ptr);
            if (Error::isError(err)) {
                m_err_n++;
                return -2; // Error code
            }

            index++;
            if (index >= max) {
                return index;
            }
        }

        return index;
    }

    bool SVGPathParser::booleanAtIndex(int32_t index) {
        if ((index < 0) || (index >= m_values.size())) {
            Exception::throwSpecific(SVG::kErrValueIndexOutOfRange);
        }
        return std::fabs(m_values[index]) > FLT_EPSILON;
    }

    double SVGPathParser::valueAtIndex(int32_t index) {
        if ((index < 0) || (index >= m_values.size())) {
            Exception::throwSpecific(SVG::kErrValueIndexOutOfRange);
        }
        return m_values[index];
    }

    double SVGPathParser::xAtIndex(int32_t index) {
        if ((index < 0) || (index >= m_values.size())) {
            Exception::throwSpecific(SVG::kErrValueIndexOutOfRange);
        }
        if (m_relative_state) {
            return m_values[index] + m_curr_pos.x_;
        }
        return m_values[index];
    }

    double SVGPathParser::yAtIndex(int32_t index) {
        if ((index < 0) || (index >= m_values.size())) {
            Exception::throwSpecific(SVG::kErrValueIndexOutOfRange);
        }
        if (m_relative_state) {
            return m_values[index] + m_curr_pos.y_;
        }
        return m_values[index];
    }

    Vec2d SVGPathParser::posAtValueIndex(int32_t index) {
        if ((index < 0) || (index >= m_values.size())) {
            Exception::throwSpecific(SVG::kErrValueIndexOutOfRange);
        }

        Vec2d pos = { m_values[index], m_values[index + 1] };
        if (m_relative_state) {
            pos += m_curr_pos;
        }

        return pos;
    }

    ErrorCode SVGPathParser::parsePathData(const char* str) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!str) {
                Exception::throwStandard(ErrorCode::NullData);
            }

            if (!m_compound_path) {
                Exception::throwSpecific(SVG::kErrCompoundPathIsNull);
            }

            m_data = str;
            const char* p = str;

            while (*p != '\0') {   // Loop until char is EOS
                auto c = *p;

                switch (c) {
                    case ',':
                    case ' ':
                        _addValue();
                        break;

                    case 'M':   // moveto
                    case 'm':
                    case 'L':   // lineto
                    case 'l':
                    case 'H':   // horizontal lineto
                    case 'h':
                    case 'V':   // vertical lineto
                    case 'v':
                    case 'A':   // elliptical arc
                    case 'a':
                    case 'Q':   // quadratic curveto, Quadratic Bezier curve
                    case 'q':
                    case 'T':   // smooth quadratic curveto, Quadratic Bezier curve
                    case 't':
                    case 'C':   // curveto, Cubic Bezier curve
                    case 'c':
                    case 'S':   // smooth curveto, Cubic Bezier curve
                    case 's':
                    case 'R':   // Catmull-Rom
                    case 'r':
                    case 'B':   // bearing
                    case 'b':
                    case 'Z':   // closepath
                    case 'z':
                        m_next_command = c;
                        break;

                    case '-':
                        _addValue();
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.':
                        m_value_state = true;
                        m_value_str[m_value_str_index++] = c;
                        m_value_str[m_value_str_index] = 0;
                        if (m_value_str_index >= kMaxValueStrLength - 1) {
                            Exception::throwSpecific(kErrValueOverflow);
                        }
                        break;

                    default:
                        break;
                }

                if (m_next_command != 0) {
                    _addValue();
                    m_relative_state = islower(m_next_command);
                    m_curr_command = tolower(m_next_command);
                    m_next_command = 0;
                }

                p++;
            }

            _addValue();
            // _finalizePath();
        }
        catch (const Exception& e) {
            result = e.code();
        }
        catch (const std::exception& e) {
            result = ErrorCode::StdCppException;
        }

        return result;
    }

    void SVGPathParser::_addValue() {
        if (m_value_state) {
            m_values.push(String::parseDoubleWithDotOrComma(m_value_str));
            auto n = (int32_t)m_values.size();

            // std::cout << "_addValue command: " << m_curr_command << ", n: " << n << std::endl;

            switch (m_curr_command) {
                case 'm':
                case 'l':
                    if (n > 2) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 2) {
                        _addSegment(m_curr_command);
                        m_curr_command = 'l';   // Repeated commands to 'm' are always 'l'.
                        m_values.clear();
                    }
                    break;

                case 'h':
                case 'v':
                    if (n > 1) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 1) {
                        _addSegment(m_curr_command);
                        m_values.clear();
                    }
                    break;

                case 'q':
                    if (n > 4) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 4) {
                        _addSegment(m_curr_command);
                        m_values.clear();
                    }
                    break;

                case 'c':
                    if (n > 6) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 6) {
                        _addSegment(m_curr_command);
                        m_values.clear();
                    }
                    break;

                case 's':
                    if (n > 4) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 4) {
                        _addSegment(m_curr_command);
                        m_values.clear();
                    }
                    break;

                case 't':
                    if (n > 2) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 2) {
                        _addSegment(m_curr_command);
                        m_values.clear();
                    }
                    break;

                case 'a':
                    if (n > 7) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 7) {
                        _addSegment(m_curr_command);
                        m_values.clear();
                    }
                    break;

                default:
                    _m_unhandled_command_count++;
                    // std::cout << "SVGPathParser::_addValue() unhandled command: " << m_curr_command << std::endl;
                    break;
            }

            // Prepare for parsing of next value
            m_value_str[0] = 0;
            m_value_str_index = 0;
            m_value_state = false;
        }
    }

    /**
     *  @brief Add a segment to the current path.
     */
    void SVGPathParser::_addSegment(const char command) {
        auto n = (int32_t)m_values.size();

        switch (command) {
            case 'm':
                m_compound_path->addEmptyPath();
                m_current_path = m_compound_path->lastPathPtr();
            case 'l':
                {
                    if (!m_current_path) {
                        Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                    }
                    if (n != 2) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    m_curr_pos = posAtValueIndex(0);
                    m_current_path->addPoint(m_curr_pos);
                }
                break;

            case 'h':
                if (!m_current_path) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 1) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                m_curr_pos.x_ = xAtIndex(0);
                m_current_path->addPoint(m_curr_pos);
                break;

            case 'v':
                if (!m_current_path) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 1) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                m_curr_pos.y_ = yAtIndex(0);
                m_current_path->addPoint(m_curr_pos);
                break;

            case 'q':
                /*
                 *  Quadratic Bézier curve segment.
                 *  x1, y1: Control point for the curve.
                 *  x, y: Endpoint of the curve.
                 */
                if (!m_current_path) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 4) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                {
                    Vec2d c_pos = posAtValueIndex(0);
                    Vec2d end_pos = posAtValueIndex(2);
                    m_current_path->addQuadraticBezier(c_pos, end_pos);
                    m_curr_pos = end_pos;
                }
                break;

            case 'c':
                /*
                 *  Cubic Bézier curve segment.
                 *  x1, y1: First control point.
                 *  x2, y2: Second control point.
                 *  x, y: Endpoint of the curve.
                 */
                if (!m_current_path) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 6) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                {
                    Vec2d c1_pos = posAtValueIndex(0);
                    Vec2d c2_pos = posAtValueIndex(2);
                    Vec2d end_pos = posAtValueIndex(4);
                    m_current_path->addBezier(c1_pos, c2_pos, end_pos);
                    m_curr_pos = end_pos;
                }
                break;

            case 's':
                /*
                 *  Smooth Cubic Bézier Curve.
                 *  x2, y2: Second control point.
                 *  x, y: Endpoint of the curve.
                 */
                if (!m_current_path) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 4) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                {
                    Vec2d c2_pos = posAtValueIndex(0);
                    Vec2d end_pos = posAtValueIndex(2);
                    m_current_path->addSmoothBezier(c2_pos, end_pos);
                    m_curr_pos = end_pos;
                }
                break;

            case 't':
                /*
                 *  Smooth Cubic Bézier Curve.
                 *  x, y: Endpoint of the curve.
                 */
                if (!m_current_path) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 2) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                {
                    Vec2d end_pos = posAtValueIndex(0);
                    m_current_path->addSmoothQuadraticBezier(end_pos);
                    m_curr_pos = end_pos;
                }
                break;

            case 'a':
                {
                    if (!m_current_path) {
                        Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                    }
                    if (n != 7) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    Vec2d radii = { valueAtIndex(0), valueAtIndex(1) };
                    double x_axis_rotation = valueAtIndex(2);
                    bool large_arc_flag = booleanAtIndex(3);
                    bool sweep_flag = booleanAtIndex(4);
                    Vec2d end_pos = posAtValueIndex(5);
                    m_current_path->addArcAsBezier(radii, x_axis_rotation, large_arc_flag, sweep_flag, end_pos, 5);
                    m_curr_pos = end_pos;
                }
                break;

            case 'z':
                if (!m_current_path) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                m_current_path->close();
                break;

            default:
                // std::cout << "SVGPathParser::_addSegment() unhandled command: " << m_curr_command << std::endl;
                break;
        }
    }


    /**
     *  @brief Checks if the provided C-string is a valid function name in SVG context.
     *
     *  This function verifies whether a given string represents a valid function name
     *  in an SVG context. A valid function name must start with an alphabetic character
     *  and can contain alphanumeric characters or underscores (_), followed by an open
     *  parenthesis `(`, signaling the start of the function arguments.
     *
     *  @param str The C-string representing the function name to be validated.
     *
     *  @return
     *    - A positive integer representing the length of the valid function name if
     *      the string is a valid function name.
     *    - 0 if the string is not a valid function name.
     *
     *  @note The function name is validated based on the following rules:
     *    - It must start with an alphabetic character (a-z, A-Z).
     *    - It can contain alphabetic characters, digits (0-9), or underscores (_).
     *    - It must end when encountering an opening parenthesis `(`.
     *    - The function name must not contain spaces or any other special characters.
     */
    int32_t SVG::isValidFunctionName(const char* str) noexcept {
        if (str == nullptr || !std::isalpha(*str)) {
            return -1; // Must start with an alphabetic character
        }

        auto ptr = str + 1;
        while (*ptr != '\0') {
            char c = *ptr;
            if (c == '(') {
                return (int32_t)(ptr - str);
            }
            if ((std::isalpha(c) || (c >= '0' && c <= '9') || c == '_') == false) {
                return -2; // Unsupported character
            }
            ptr++;
        }

        return -3;  // Missing '('
    }


    /**
     *  @brief Utility function to print out all attribues of a XML element.
     */
    void SVG::logXMLElementAttributes(std::ostream &os, const tinyxml2::XMLElement* xml_element) noexcept {
        if (!xml_element) {
            os << "XML element is null.\n";
            return;
        }

        // Print the element's name
        os << "XML element name: " << xml_element->Name() << '\n';

        // Iterate over all attributes of the element
        const tinyxml2::XMLAttribute* attribute = xml_element->FirstAttribute();
        if (attribute == nullptr) {
            os << "No attributes found.\n";
        }
        else {
            while (attribute != nullptr) {
                os << "    attribute: " << attribute->Name() << ", value: " << attribute->Value() << '\n';
                attribute = attribute->Next();
            }
        }
    }

} // End of namespace Grain
