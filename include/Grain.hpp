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

#ifndef Grain_hpp
#define Grain_hpp

#include <ostream>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cmath>
#include <cfloat>
#include <concepts>
#include <stdexcept>
#include <string>
#include <numbers>
#include <type_traits>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <algorithm>
#include <cstdarg>

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>  // Required for TARGET_OS_* macros
#endif

#define GRAIN_RETAIN(o) Grain::Object::retain(o)
#define GRAIN_RELEASE(o) if (o != nullptr) { if (Grain::Object::release(o) == nullptr) { o = nullptr; } }


namespace Grain {

    using fourcc_t = uint32_t;

    // Scalar = anything that behaves like a number
    template <typename T>
    concept ScalarType = std::integral<T> || std::floating_point<T>;


    enum class GraphicContextType {
        Undefined = -1,
        Cairo = 0,
        AppleMac = 1,
        // AGG
    };


    /**
     *  @brief Enumeration representing various data types or value classes.
     *
     *  This enum is used to classify different primitive and custom data types
     *  for use in type handling, serialization, or introspection systems.
     */
    enum class DataType {
        Undefined = 0,
        Bool = 1,
        Char = 2,       ///< 8 bit char
        Int8 = 3,       ///< 8 bit integer
        Int16 = 4,      ///< 16 bit integer
        Int32 = 5,      ///< 32 bit integer
        Int64 = 6,      ///< 64 bit integer
        UInt8 = 7,      ///< 8 bit unsigned integer
        UInt16 = 8,     ///< 16 bit unsigned integer
        UInt32 = 9,     ///< 32 bit unsigned integer
        UInt64 = 10,    ///< 64 bit unsigned integer
        Float = 11,     ///< 32 bit floating point
        Double = 12,    ///< 64 bit floating point
        FourCC = 13,    ///< 32 bit, four chars code
        Fix = 14,       ///< 64 bit fix point
        Pointer = 15    ///< Pointer type, bit depth system specific
    };


    enum class Endianess {
        Little = 0,
        Big
    };

    enum class CanOverwrite {
        Yes = 1,
        No = 0
    };

    enum class CharSet {
        UTF8 = 0,
        ASCII = 1,
        ASCII_8859_1_Latin1 = 2,
        ASCII_Windows1252 = 3,

        Count,
        First = 0,
        Last = Count - 1,
    };

    enum class ErrorCode {
        None = 0,                 ///< No error

        NullPointer,              ///< A nullptr where a valid pointer is expected
        NullData,                 ///< A pointer representing data is expected
        BadArgs,                  ///< A method or function was called with bad arguments
        NoData,
        NoMatch,                  ///< Nothing found where a match was expected
        UnexpectedData,           ///< In data something unexpected is contained
        UnexpectedBehaviour,
        ComputationFailed,
        SortFailed,               ///< Sorting failed
        FormatMismatch,
        FileSystemErr,
        InvalidNumber,
        Base64NoBase64Code,

        UnknownParameter,
        UnknownValue,

        StdFileSysError = 100,
        StdCppException,

        ClassInstantiationFailed = 200,
        ObjectParamSetFailed,
        ObjectMessageFailed,

        MemPointsToItself = 300,
        BufferTooSmall,             ///< A buffer is to small
        StrBufferTooSmall,
        BuffersMustBeDifferent,
        IndexOutOfRange,            ///< An index is println of range
        RegionOutOfRange,
        OffsOutOfRange,
        LenOutOfRange,
        DestinationOutOfBounds,
        SourceOutOfBounds,
        UnsupportedStepSize,
        UnsupportedDataType,
        UnsupportedSettings,
        LimitExceeded,

        MemCantAllocate = 400,
        MemCantGrow,
        MemExternalMemCantGrow,

        DataComposerUnknownPropertyType = 460,
        DataComposerPropertyTypeMismatch,
        DataComposerObjectCreationModelIsNull,
        DataComposerObjectCreationFailed,
        DataComposerNoModelForName,
        DataComposerModelCreationNoBaseModel,

        FileOverwriteNotAllowed = 500,
        FileNoHandle,
        FileCantCreate,
        FileCantOpen,
        FileInvalidStream,
        FileCantRead,
        FileCantReadInternalLimits,
        FileCantWrite,
        FileIsEmpty,
        FileCantGetPos,
        FileCantSetPos,
        FileReadError,
        FileEndOfFileReached,
        FileUTF8Mismatch,
        FileBase64EncodeError,
        FileInstantiationFailed,
        FileNotFound,
        FileCantBeRemoved,
        FileDirNotCreated,
        FileDirNotFound,
        FileFlushFailed,
        FileCloseFailed,
        FileFSyncFailed,
        DirAllCantBeRemoved,
        UnsupportedFileSize,
        UnsupportedEndianess,
        UnsupportedFileFormat,
        UnknownTiffFieldType,

        UnsupportedDimension = 600,
        UnsupportedResolution,
        UnsupportedColorModel,
        UnsupportedChannelCount,
        InvalidChannel,
        UnsupportedSampleRate,
        SampleRateMustBeDifferent,
        SampleRateMustBeEqual,

        DatabaseNotConnected = 800,
        DatabaseNoResult = 801,
        DatabaseSetTimeoutFailed,

        CSVIndexOutOfRange,
        CSVValueOutOfRange,
        CSVTypeError,

        CSSInternalError = 1100,
        CSSInternalMemoryError,
        CSSClosingBracketMissing,
        CSSContentMissing,
        CSSContentToBig,
        CSSValueStorageOverflow,
        CSSInvalidFormat,
        CSSNumberParseError,
        CSSUnknownUnit,
        CSSNoneHexLetter,
        CSSToManyDigitsInHexCode,
        CSSWrongDigitsInHexCode,
        CSSColorFunctionDoesntSupportModernSyntax,
        CSSWrongNumberOfValues,
        CSSWrongCommaDelimiterSequence,
        CSSWrongUnit,
        CSSValueOverflow,

        LuaInstantiationFailed = 1500,
        LuaCodeError = 1501,

        TomlParseError = 1700,
        TomlNoName,
        TomlExpectedTableItem,
        TomlExpectedTable,
        TomlExpectedNode,
        TomlWrongType,

        InvalidProjection,

        UnexpectedRuntimeError = 999997,
        Unknown = 999998,
        Fatal = 999999,
        Specific = 1000000
    };

    class Error {
    public:
        static inline bool isError(ErrorCode code) {
            return code != ErrorCode::None;
        }

        // TODO: Check, is this needed anymore?
        static void throwSpecific(int32_t custom_code) {
            throw static_cast<ErrorCode>(custom_code + 1000000);
        }

        static ErrorCode specific(int32_t custom_code) {
            return static_cast<ErrorCode>(custom_code + 1000000);
        }
    };


    class Log;

    class Exception : public std::exception {
    private:
        ErrorCode m_code{};
        std::string m_message;
    public:
        explicit Exception(ErrorCode code, const char* message);

        [[nodiscard]] ErrorCode code() const noexcept;
        [[nodiscard]] const char* message() const noexcept;

        void log(Log& l) const;

        static void throwStandard(ErrorCode code);
        static void throwSpecific(int32_t specific_code, const char* message = nullptr);
        static void throwMessage(ErrorCode code, const char* message = nullptr);
        static void throwFormattedMessage(ErrorCode code, const char* format, ...);
        static void throwSpecificFormattedMessage(uint32_t code, const char* format, ...);
    };

    class DeferredException {
        std::exception_ptr m_ptr;

    public:
        DeferredException() noexcept = default;

        explicit DeferredException(std::exception_ptr ptr) noexcept : m_ptr(ptr) {}

        // Capture current exception
        void capture() noexcept {
            m_ptr = std::current_exception();
        }

        // Check if exception is stored
        [[nodiscard]] bool hasException() const noexcept {
            return m_ptr != nullptr;
        }

        // Rethrow stored exception if any
        void rethrow() const {
            if (m_ptr) {
                std::rethrow_exception(m_ptr);
            }
        }

        // Clear stored exception
        void reset() noexcept {
            m_ptr = nullptr;
        }

        /**
         *  @brief Create and store a custom deferred Exception without throwing.
         *
         *  Constructs an Exception with the given message and code, and stores it
         *  in m_ptr as a std::exception_ptr for later re-throwing.
         *
         *  @param message The error message.
         *  @param code    Optional specific error code (default: UnexpectedRuntimeError).
         */
        void createAndCaptureUnexpected(
                const char* message,
                ErrorCode code = ErrorCode::UnexpectedRuntimeError) noexcept
        {
            try {
                // Create the Exception object
                Exception e(code, message);

                // Store it as a std::exception_ptr
                m_ptr = std::make_exception_ptr(e);
            }
            catch (...) {
                // This should never throw, but in case of memory allocation failure
                m_ptr = std::current_exception();
            }
        }

        void createAndCaptureUnexpected(const Exception& e) {
            createAndCaptureUnexpected(e.message(), e.code());
        }
    };


    class Safe {
    public:
        template<typename T>
        [[nodiscard]] static bool canSafelyDivideBy(T v) {
            if constexpr (std::is_integral_v<T>) {
                // For integers, check not zero and avoid INT_MIN / -1 overflow
                if (v == 0) return false;
                if constexpr (std::is_signed_v<T>) {
                    return v != -1 || std::numeric_limits<T>::min() != v;
                }
                return true;
            }
            else {
                // For unsupported types -> compile error
                static_assert(std::is_arithmetic_v<T>, "canSafelyDivideBy only supports arithmetic types");
            }
        }

    };

    // Declare specializations
    template<>
    [[nodiscard]] bool Safe::canSafelyDivideBy<float>(float v);

    template<>
    [[nodiscard]] bool Safe::canSafelyDivideBy<double>(double v);


    typedef void (*SimpleFunc)(void* ref);
    typedef int (*SortCompareFunc)(const void* a, const void* b);


} // End of namespace Grain

#endif // Grain_hpp

