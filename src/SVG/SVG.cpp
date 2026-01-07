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
#include "Core/Log.hpp"


namespace Grain {

    SVG::SVG(const String& file_path) noexcept {
        file_path_ = file_path;
    }

    SVG::~SVG() noexcept {
        // TODO: Check all objects etc.
        delete svg_root_;
        delete xml_doc_;
    }

    void SVG::log(std::ostream& os, int32_t indent, const char* label) const {
        Log l(os);
        l.header(label);
        l << "m_file_path: " << file_path_ << Log::endl;
        if (xml_error_id_ != kXMLErr_None) {
            l << "m_xml_error_id: " << xml_error_id_ << Log::endl;
            ++l;
            l << "m_xml_error_line: " << xml_error_line_ << Log::endl;
            l << "m_xml_error_message: " << xml_error_message_ << Log::endl;
            --l;
        }
    }

    SVGGradient* SVG::addGradient(SVGGradientType type, int32_t capacity) noexcept {
        auto gradient = new(std::nothrow) SVGGradient(type, capacity);
        if (gradient) {
            paint_servers_.push((SVGPaintServer*)gradient);
        }
        return gradient;
    }

    SVGPaintServer* SVG::paintServerByID(const String& id) const noexcept {
        for (auto& paint_server : paint_servers_) {
            if (paint_server->id_.compareAscii(id, 0, 1) == 0) {
                return paint_server;
            }
        }
        return nullptr;
    }

    void SVG::clearError() noexcept {
       xml_error_id_ = kXMLErr_None;
        xml_error_message_.clear();
        xml_error_line_ = -1;
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
            xml_doc_ = new(std::nothrow) tinyxml2::XMLDocument();
            if (!xml_doc_) {
                Exception::throwSpecific(kErrXMLDocError);
            }

            // Load the SVG file
            if (xml_doc_->LoadFile(file_path_.utf8()) != tinyxml2::XML_SUCCESS) {
                // Get detailed error information
                xml_error_id_ = xml_doc_->ErrorID();
                xml_error_message_ = xml_doc_->ErrorStr();
                xml_error_line_ = xml_doc_->ErrorLineNum();
                Exception::throwSpecific(kErrXMLDocLoadFileError);
            }

            // The root group, which ist the files <svg> element
            svg_root_ = new(std::nothrow) SVGRootElement(nullptr);
            if (!svg_root_) {
                Exception::throwStandard(ErrorCode::MemCantAllocate);
            }

            // Get the root <svg> element
            auto* xml_element = xml_doc_->FirstChildElement("svg");
            if (!xml_element) {
                Exception::throwSpecific(kErrSVGTagNotFound);
            }

            svg_root_->parse(this, xml_element);
        }
        catch (const Exception& e) {
            std::cout << "SVG::parse() err: " << e.codeAsInt() << std::endl;
            result = e.code();
        }

        if (svg_root_ != nullptr) {
            // TODO:
        }

        return result;
    }

    void SVG::draw(GraphicContext& gc) noexcept {
        if (svg_root_) {
            // gc.translate(700, 700);
            // gc.scale(10);
            gc.save();
            svg_root_->draw(this, gc);
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

            buffer_pos_ = 0;
            read_ptr_ = str;
            run_ = true;
        }
        catch (const Exception& e) {
            result = e.code();
        }

        return result;
    }

    int32_t SVGValuesParser::next(double& out_next) noexcept {
        if (!run_) {
            return kNextResult_NoMoreData;
        }

        while (true) {
            auto c = *read_ptr_++;

            if ((c >= '0' && c <= '9') || c == '-' | c == '.') {
                buffer_[buffer_pos_] = c;
                buffer_pos_++;
            }
            else if (c == ' ' || c == ',' || c == '\0') {
                if (buffer_pos_ > 0) {
                    buffer_[buffer_pos_] = '\0';
                    out_next = String::parseDoubleWithDotOrComma(buffer_);
                    buffer_pos_ = 0;
                    buffer_[0] = '\0';
                    value_count_++;

                    if (c == '\0') {
                        run_ = false;
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

            read_ptr_ = str;
            function_name_ptr_ = str;
            function_name_length_ = 0;
            function_count_ = 0;
            data_length_ = 0;
            data_ptr_ = nullptr;
            err_n_ = 0;
            run_ = true;
        }
        catch (const Exception& e) {
            result = e.code();
        }

        return result;
    }

    int32_t SVGFunctionValuesParser::nextFunction() noexcept {
        function_data_[0] = '\0';
        data_length_ = 0;

        if (!run_) {
            return kNextFunctionResult_NoMoreData;
        }

        // Skip spaces
        while (*read_ptr_ == ' ') {
            read_ptr_++;
        }

        function_name_ptr_ = read_ptr_;

        int32_t n = SVG::isValidFunctionName(read_ptr_);
        if (n <= 0) {
            run_ = false;
            err_n_++;
            return kNextFunctionResult_InvalidFunctionName;
        }

        if (n > kMaxFunctionNameLength) {
            err_n_++;
            return kNextFunctionResult_InvalidFunctionNameLength;
        }

        strncpy(function_name_, function_name_ptr_, n);
        function_name_[n] = '\0';
        data_ptr_ = function_name_ptr_ + n + 1;
        function_name_length_ = n;

        const char* end_ptr = strchr(data_ptr_, ')');
        if (end_ptr == nullptr) {
            err_n_++;
            return kNextFunctionResult_EndOfDataBlockMissing;
        }

        data_length_ = (int32_t)(end_ptr - data_ptr_);
        if (data_length_ > kMaxFunctionDataLength) {
            err_n_++;
            return kNextFunctionResult_InvalidDataLength;
        }

        strncpy(function_data_, data_ptr_, data_length_);
        function_data_[data_length_] = '\0';
        // Replace comma with spaces for easy splitting the values
        String::replaceChar(function_data_, ',', ' ');

        read_ptr_ = data_ptr_ + data_length_ + 1;
        function_count_++;

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
                err_n_++;
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
        if ((index < 0) || (index >= values_.size())) {
            Exception::throwSpecific(SVG::kErrValueIndexOutOfRange);
        }
        return std::fabs(values_[index]) > FLT_EPSILON;
    }

    double SVGPathParser::valueAtIndex(int32_t index) {
        if ((index < 0) || (index >= values_.size())) {
            Exception::throwSpecific(SVG::kErrValueIndexOutOfRange);
        }
        return values_[index];
    }

    double SVGPathParser::xAtIndex(int32_t index) {
        if ((index < 0) || (index >= values_.size())) {
            Exception::throwSpecific(SVG::kErrValueIndexOutOfRange);
        }
        if (relative_state_) {
            return values_[index] + curr_pos_.x_;
        }
        return values_[index];
    }

    double SVGPathParser::yAtIndex(int32_t index) {
        if ((index < 0) || (index >= values_.size())) {
            Exception::throwSpecific(SVG::kErrValueIndexOutOfRange);
        }
        if (relative_state_) {
            return values_[index] + curr_pos_.y_;
        }
        return values_[index];
    }

    Vec2d SVGPathParser::posAtValueIndex(int32_t index) {
        if ((index < 0) || (index >= values_.size())) {
            Exception::throwSpecific(SVG::kErrValueIndexOutOfRange);
        }

        Vec2d pos = { values_[index], values_[index + 1] };
        if (relative_state_) {
            pos += curr_pos_;
        }

        return pos;
    }

    ErrorCode SVGPathParser::parsePathData(const char* str) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!str) {
                Exception::throwStandard(ErrorCode::NullData);
            }

            if (!compound_path_) {
                Exception::throwSpecific(SVG::kErrCompoundPathIsNull);
            }

            data_ = str;
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
                        next_command_ = c;
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
                        value_state_ = true;
                        value_str_[value_str_index_++] = c;
                        value_str_[value_str_index_] = 0;
                        if (value_str_index_ >= kMaxValueStrLength - 1) {
                            Exception::throwSpecific(kErrValueOverflow);
                        }
                        break;

                    default:
                        break;
                }

                if (next_command_ != 0) {
                    _addValue();
                    relative_state_ = islower(next_command_);
                    curr_command_ = tolower(next_command_);
                    next_command_ = 0;
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
        if (value_state_) {
            values_.push(String::parseDoubleWithDotOrComma(value_str_));
            auto n = (int32_t)values_.size();

            // std::cout << "_addValue command: " << curr_command_ << ", n: " << n << std::endl;

            switch (curr_command_) {
                case 'm':
                case 'l':
                    if (n > 2) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 2) {
                        _addSegment(curr_command_);
                        curr_command_ = 'l';   // Repeated commands to 'm' are always 'l'.
                        values_.clear();
                    }
                    break;

                case 'h':
                case 'v':
                    if (n > 1) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 1) {
                        _addSegment(curr_command_);
                        values_.clear();
                    }
                    break;

                case 'q':
                    if (n > 4) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 4) {
                        _addSegment(curr_command_);
                        values_.clear();
                    }
                    break;

                case 'c':
                    if (n > 6) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 6) {
                        _addSegment(curr_command_);
                        values_.clear();
                    }
                    break;

                case 's':
                    if (n > 4) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 4) {
                        _addSegment(curr_command_);
                        values_.clear();
                    }
                    break;

                case 't':
                    if (n > 2) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 2) {
                        _addSegment(curr_command_);
                        values_.clear();
                    }
                    break;

                case 'a':
                    if (n > 7) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    if (n == 7) {
                        _addSegment(curr_command_);
                        values_.clear();
                    }
                    break;

                default:
                    unhandled_command_count_++;
                    // std::cout << "SVGPathParser::_addValue() unhandled command: " << curr_command_ << std::endl;
                    break;
            }

            // Prepare for parsing of next value
            value_str_[0] = 0;
            value_str_index_ = 0;
            value_state_ = false;
        }
    }

    /**
     *  @brief Add a segment to the current path.
     */
    void SVGPathParser::_addSegment(const char command) {
        auto n = (int32_t)values_.size();

        switch (command) {
            case 'm':
                compound_path_->addEmptyPath();
                current_path_ = compound_path_->lastPathPtr();
            case 'l':
                {
                    if (!current_path_) {
                        Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                    }
                    if (n != 2) {
                        Exception::throwSpecific(SVG::kErrValueMismatch);
                    }
                    curr_pos_ = posAtValueIndex(0);
                    current_path_->addPoint(curr_pos_);
                }
                break;

            case 'h':
                if (!current_path_) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 1) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                curr_pos_.x_ = xAtIndex(0);
                current_path_->addPoint(curr_pos_);
                break;

            case 'v':
                if (!current_path_) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 1) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                curr_pos_.y_ = yAtIndex(0);
                current_path_->addPoint(curr_pos_);
                break;

            case 'q':
                /*
                 *  Quadratic Bézier curve segment.
                 *  x1, y1: Control point for the curve.
                 *  x, y: Endpoint of the curve.
                 */
                if (!current_path_) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 4) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                {
                    Vec2d c_pos = posAtValueIndex(0);
                    Vec2d end_pos = posAtValueIndex(2);
                    current_path_->addQuadraticBezier(c_pos, end_pos);
                    curr_pos_ = end_pos;
                }
                break;

            case 'c':
                /*
                 *  Cubic Bézier curve segment.
                 *  x1, y1: First control point.
                 *  x2, y2: Second control point.
                 *  x, y: Endpoint of the curve.
                 */
                if (!current_path_) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 6) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                {
                    Vec2d c1_pos = posAtValueIndex(0);
                    Vec2d c2_pos = posAtValueIndex(2);
                    Vec2d end_pos = posAtValueIndex(4);
                    current_path_->addBezier(c1_pos, c2_pos, end_pos);
                    curr_pos_ = end_pos;
                }
                break;

            case 's':
                /*
                 *  Smooth Cubic Bézier Curve.
                 *  x2, y2: Second control point.
                 *  x, y: Endpoint of the curve.
                 */
                if (!current_path_) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 4) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                {
                    Vec2d c2_pos = posAtValueIndex(0);
                    Vec2d end_pos = posAtValueIndex(2);
                    current_path_->addSmoothBezier(c2_pos, end_pos);
                    curr_pos_ = end_pos;
                }
                break;

            case 't':
                /*
                 *  Smooth Cubic Bézier Curve.
                 *  x, y: Endpoint of the curve.
                 */
                if (!current_path_) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                if (n != 2) {
                    Exception::throwSpecific(SVG::kErrValueMismatch);
                }
                {
                    Vec2d end_pos = posAtValueIndex(0);
                    current_path_->addSmoothQuadraticBezier(end_pos);
                    curr_pos_ = end_pos;
                }
                break;

            case 'a':
                {
                    if (!current_path_) {
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
                    current_path_->addArcAsBezier(radii, x_axis_rotation, large_arc_flag, sweep_flag, end_pos, 5);
                    curr_pos_ = end_pos;
                }
                break;

            case 'z':
                if (!current_path_) {
                    Exception::throwSpecific(SVG::kErrCurrentPathIsNull);
                }
                current_path_->close();
                break;

            default:
                // std::cout << "SVGPathParser::_addSegment() unhandled command: " << curr_command_ << std::endl;
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
