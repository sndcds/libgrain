//
//  Grain.cpp
//
//  Created by Roald Christesen on 14.01.24.
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Grain.hpp"
#include "Core/Log.hpp"
#include "String/String.hpp"


namespace Grain {

    template<>
    bool Safe::canSafelyDivideBy<float>(float v) {
        return std::abs(v) > 1e-6f;
    }

    template<>
    bool Safe::canSafelyDivideBy<double>(double v) {
        return std::abs(v) > 1e-12;
    }

    Exception::Exception(ErrorCode code, const char* message)
            : m_code(code) {
        m_message = message;
    }

    void Exception::throwStandard(ErrorCode code) {
        if (code != ErrorCode::None) {
            throw Exception(code, "Standard Grain Exception");
        }
    }

    void Exception::throwSpecific(int32_t code, const char *message) {
        throw Exception(static_cast<ErrorCode>(code + static_cast<int32_t>(ErrorCode::Specific)), message);
    }

    void Exception::throwMessage(ErrorCode code, const char* message) {
        if (code != ErrorCode::None) {
            throw Exception(code, message);
        }
    }

    void Exception::throwFormattedMessage(ErrorCode code, const char* format, ...) {
        if (code != ErrorCode::None) {
            va_list args;
            va_start(args, format);
            std::string buffer(2048, '\0');
            std::vsnprintf(buffer.data(), buffer.size(), format, args);
            va_end(args);
            throw Exception(code, buffer.c_str());
        }
    }

    void Exception::throwSpecificFormattedMessage(uint32_t code, const char* format, ...) {
        if (code >= 0) {
            va_list args;
            va_start(args, format);
            std::string buffer(2048, '\0');
            std::vsnprintf(buffer.data(), buffer.size(), format, args);
            va_end(args);
            throw Exception(static_cast<ErrorCode>(code + static_cast<int32_t>(ErrorCode::Specific)), buffer.c_str());
        }
    }

    void Exception::log(Log& l) const {
        l << "Grain Exception " << static_cast<int32_t>(m_code);
        l << ": " << m_message;
        l << l.endl;
    }

    ErrorCode Exception::code() const noexcept {
        return m_code;
    }

    const char* Exception::message() const noexcept {
        return m_message.c_str();
    }
}
