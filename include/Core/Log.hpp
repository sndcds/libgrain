//
//  Log.hpp
//
//  Created by Roald Christesen on 15.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 19.07.2025
//

#ifndef GrainLog_hpp
#define GrainLog_hpp

#include "Grain.hpp"


namespace Grain {

    class File;

    class Log {
    public:
        Log() noexcept;
        explicit Log(std::ostream& stream, int32_t indent = 0, int32_t indent_step = 4) noexcept;
        explicit Log(File& file, int32_t indent = 0, int32_t indent_step = 4) noexcept;

        [[nodiscard]] bool doesOutput() const noexcept { return m_does_output; }

        // Overloaded << operator for `FixedManipulator`
        struct FixedManipulator {
            int32_t precision;
            explicit FixedManipulator(int32_t p) : precision(p) {}
        };
        [[nodiscard]] FixedManipulator fixed(int precision) {
            return FixedManipulator(precision);
        }
        Log& operator << (const FixedManipulator& f) {
            m_stream << std::fixed << std::setprecision(f.precision);
            return *this;
        }

        // Overloaded << operator for `DefaultFloatManipulator`
        struct DefaultFloatManipulator {};
        static constexpr DefaultFloatManipulator defaultfloat{};
        Log& operator << (const DefaultFloatManipulator&) {
            m_stream << std::defaultfloat;
            return *this;
        }

        // Overloaded << operator for `EndManipulator`
        struct EndManipulator {};       ///< End manipulator used to handle `log.endl`
        static EndManipulator endl;     ///< Static member to provide `log.endl`
        Log& operator << (const EndManipulator&) {
            m_stream << std::endl;
            m_needs_indent = true;
            return *this;
        }

        /**
         *  @brief Overload the << operator for general types.
         */
        template <typename T>
        Log& operator << (const T& value) {
            if (m_needs_indent) {
                _applyIndent();
            }
            m_stream << value; // Output the value
            return *this; // Return *this for chaining
        }

        /**
         *  @brief Overload the << operator for const char* to safely handle nullptr.
         */
        Log& operator<<(const char* str) {
            if (m_needs_indent) {
                _applyIndent();
            }
            if (str == nullptr) {
                m_stream << "(NULL)";
            }
            else {
                m_stream << str;
            }
            return *this;
        }

        /**
         *  @brief Overload the << operator for manipulators (like std::endl).
         */
        Log& operator << (std::ostream&(*manip)(std::ostream&)) {
            if (manip == static_cast<std::ostream&(*)(std::ostream&)>(std::endl)) {
                m_stream << manip;  // Output the newline
                m_needs_indent = true;  // Set the flag for next line
            }
            else {
                m_stream << manip;  // Pass through other manipulators
            }
            return *this;
        }

        /**
         *  @brief Overload the << operator for manipulators.
         */
        using LogManipulator = Log&(*)(Log&);
        Log& operator << (LogManipulator manip) {
            return manip(*this);  // Call the manipulator function
        }

        /**
         *  @brief Helper class for representing boolean values.
         */
        struct BoolWrapper {
            bool m_value;
        };

        /**
         *  @brief Method to create a BoolWrapper.
         */
        [[nodiscard]] BoolWrapper boolValue(bool value) {
            return BoolWrapper{value};
        }

        /**
         *  @brief Overload << operator for BoolWrapper.
         */
        Log& operator << (const BoolWrapper& bool_wrapper) {
            m_stream << (bool_wrapper.m_value ? "true" : "false");
            return *this;
        }

        /**
         *  @brief Helper class for representing `fourcc_t` values.
         */
        struct FourCCWrapper {
            uint32_t m_value;
        };

        /**
         *  @brief Method to create a FourCCWrapper.
         */
        [[nodiscard]] FourCCWrapper fourCCValue(uint32_t value) {
            return FourCCWrapper{value};
        }

        /**
         *  @brief Overload << operator for FourCCWrapper.
         */
        Log& operator << (const FourCCWrapper& fourcc_wrapper) {
            char c1 = static_cast<char>((fourcc_wrapper.m_value >> 24) & 0xFF);
            char c2 = static_cast<char>((fourcc_wrapper.m_value >> 16) & 0xFF);
            char c3 = static_cast<char>((fourcc_wrapper.m_value >> 8) & 0xFF);
            char c4 = static_cast<char>((fourcc_wrapper.m_value) & 0xFF);
            m_stream << "'" << c1 << c2 << c3 << c4 << "'";
            return *this;
        }

        /**
         *  @brief Helper class for representing DateTime values.
         */
        struct DateTimeWrapper {};

        /**
         *  @brief Method to create a DateTimeWrapper.
         */
        [[nodiscard]] DateTimeWrapper dateTime() { return DateTimeWrapper{}; }

        /**
         *  @brief Overload << operator for DateTimeWrapper.
         */
        Log& operator << (const DateTimeWrapper&) {
            _dateTime();
            return *this;
        }

        Log& operator++();    // Prefix increment: ++log
        Log operator++(int);  // Postfix increment: log++
        Log& operator--();    // Prefix decrement: --log
        Log operator--(int);  // Postfix decrement: log--

        Log& indent();
        void label(const char* label);
        void header(const char* header);
        void _dateTime();
        void ubyteDecimal(uint8_t* data, int32_t n, char delimiter);

        void right();
        void left();

        [[nodiscard]] std::ostream& stream() { return m_stream; }

        [[nodiscard]] int64_t index() const noexcept { return m_index; }
        void incIndex() noexcept { m_index++; }

        void out(const char* str) {
            m_stream << str << std::endl;
        }

    private:
        void _applyIndent();

    private:
        std::ostream& m_stream;     ///< Reference to the output stream
        int32_t m_indent;           ///< Number of spaces for one level of indentation
        int32_t m_indent_step;      ///< Multiplier for increasing indentation levels
        bool m_needs_indent;        ///< Flag to track if we need indentation
        bool m_does_output = false; ///< Flag indicating wether log has to be done or not
        int64_t m_index = 0;        ///< Index used to track the order of log items (incremented over time)
    };


} // End of namespace Grain

#endif // GrainLog_hpp
