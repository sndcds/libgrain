//
//  Log.hpp
//
//  Created by Roald Christesen on 15.01.2025
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Core/Log.hpp"
#include "Time/Timestamp.hpp"
#include "File/File.hpp"


namespace Grain {

    // Definition of static variables
    Log::EndManipulator Log::endl;


    Log::Log() noexcept : m_stream(std::cout), m_indent(0), m_indent_step(4), m_needs_indent(true) {
        m_does_output = m_stream.rdbuf() != nullptr;
    }


    /**
     *  @brief Constructor
     */
    Log::Log(std::ostream& stream, int32_t indent, int32_t indent_step) noexcept
            : m_stream(stream), m_indent(indent), m_indent_step(indent_step), m_needs_indent(true) {
        m_does_output = m_stream.rdbuf() != nullptr;
    }


    /**
     *  @brief Constructor
     */
    Log::Log(File& file, int32_t indent, int32_t indent_step) noexcept
            : m_stream(*file.stream()), m_indent(indent), m_indent_step(indent_step), m_needs_indent(true) {
        m_does_output = m_stream.rdbuf() != nullptr;
    }


    /**
     *  @brief Prefix increment: ++log.
     */
    Log& Log::operator++() { right(); return *this; }


    /**
     *  @brief Postfix increment: log++.
     */
    Log Log::operator++(int) { Log temp = *this; right(); return temp; }


    /**
     *  @brief Prefix decrement: --log.
     */
    Log& Log::operator--() { left(); return *this; }


    /**
     *  @brief Postfix decrement: log--.
     */
    Log Log::operator--(int) { Log temp = *this; left(); return temp; }


    /**
     *  @brief Indent method.
     *
     *  Set the flag to apply indentation.
     */
    Log& Log::indent() {
        m_needs_indent = true;  // Next output will have indentation
        return *this;  // Return self for chaining
    }


    /**
     *  @brief Label method for displaying a label.
     */
    void Log::label(const char* label) {
        if (m_does_output && label != nullptr) {
            _applyIndent();
            m_stream << label << ": ";
        }
    }


    /**
     *  @brief Header method for displaying a label.
     */
    void Log::header(const char* label) {
        if (m_does_output && label != nullptr) {
            _applyIndent();
            m_stream << label << std::endl;
            m_needs_indent = true;  // Start indentation after the header
            right();
        }
    }


    void Log::_dateTime() {
        if (m_does_output) {
            char buffer[30];
            Timestamp ts;
            ts.dateTimeUTCStr(30, buffer);
            m_stream << buffer;
        }
    }


    void Log::ubyteDecimal(uint8_t* data, int32_t n, char delimiter) {
        if (m_does_output) {
            if (data == nullptr) {
                m_stream << "nullptr";
            }
            else {
                for (int32_t i = 0; i < n; i++) {
                    if (i > 0 && delimiter != '\0') {
                        m_stream << delimiter;
                    }
                    m_stream << static_cast<int>(data[i]);
                }
            }
        }
    }


    /**
     *  @brief Adjust indentation level to rigt
     */
    void Log::right() { m_indent++; }


    /**
     *  @brief Adjust indentation level to left.
     */
    void Log::left() { if (m_indent > 0) { m_indent--; } }


    /**
     *  @brief Helper function to apply indentation.
     */
    void Log::_applyIndent() {
        if (m_does_output) {
            int32_t n = m_indent * m_indent_step;
            while (n-- > 0) {
                m_stream << ' ';  // Output spaces for indentation
            }
            m_needs_indent = false;  // Reset the indentation flag
        }
    }


} // End of namespace Grain
