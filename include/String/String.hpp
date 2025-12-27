//
//  String.hpp
//
//  Created by Roald Christesen on 13.02.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

// TODO: Implement `toupper` and `tolower`
// TODO: See TODOs in mm file!

#ifndef GrainString_hpp
#define GrainString_hpp


#include "Grain.hpp"
#include "Type/Fix.hpp"
#include "Type/Range.hpp"
#include "Type/Object.hpp"
#include "Time/Timestamp.hpp"

#include <cuchar>

#if defined(__APPLE__) && defined(__MACH__)
    #include <CoreFoundation/CoreFoundation.h>
#endif

namespace Grain {
    /**
    *  @brief Function definition for checking if a UTF8 code is a delimiter.
    *
    *  Useful for layouting or splitting text.
    */
    typedef uint8_t (*Utf8DelimiterTestFunc)(const char*, int32_t);


    class Utf8SingleByteDelimiterStates {
    public:
        void setByChar(char c);
        void setByCharsInStr(const char* str);
        uint8_t check(char c);

    public:
        enum {
            kFlagsNone = 0x0,
            kFlagDelimiter = 0x1,
            kFlagWhiteSpace = 0x10,
            kMaskDelimiterAndWhiteSpace = kFlagDelimiter | kFlagDelimiter
        };

        uint8_t flags_[128]{};
    };


    class StringList;
    class Data;


    /**
     *  @class String
     *  @brief String representation in UTF-8 format, with dynamic memory handling.
     *
     *  A character refers to one variable length encoded UTF-8 character.
     *
     *  A character index refers to a position within the string in terms of characters, not memory addresses.
     *  This is important because, in UTF-8 encoding, not all characters occupy the same number of bytes,
     *  so character indices are used to locate specific characters regardless of their byte length.
     *
     *  A symbol index refers to a position within the string in terms of memory addresses, specifically
     *  pointing to the starting byte of a symbol. This is important for tasks that involve memory
     *  manipulation or encoding/decoding of the string.
     */
    class String : public Object {
    public:
        enum {
            // A UTF-8 symbol (code point) can be represented using one to four bytes, depending on the range of the code point
            kMaxUtf8SeqLength = 4,
            kUtf8SeqBufferSize = 5, ///< UTF-8 sequence plut EOS
            kDefaultByteCapacity = 32,
            kMaxCharIndex = std::numeric_limits<int64_t>::max() / 5
        };

        enum {
            // Return codes for find methods.
            kFindResult_MemError = -1,
            kFindResult_StrError = -2,
            kFindResult_CharacterIndexOutOfRange = -3,
            kFindResult_ByteIndexOutOfRange = -4,
            kFindResult_ConversionIndexFailed = -5,
            kFindResult_NothingFound = -6,
        };

        enum class TrimMode {
            None,
            All,
            Head,
            Tail
        };

        static constexpr char EOS = '\0';

    protected:
        char* data_ = nullptr;          ///< UTF-8 encoded string data
        int64_t character_len_ = 0;     ///< Number of Unicode characters in `data_`
        int64_t byte_len_ = 0;          ///< Number of bytes in `data_`
        int64_t byte_capacity_ = 32;    ///< Number of possible bytes with the currently allocated memory
        int64_t extra_grow_bytes_ = 32; ///< When buffer must grow, than grow with this amount of extra bytes
        int32_t grow_count_ = 0;        ///< How often the memory buffer has grown

        static const char* g_empty_data;    ///< Empty string with zero length
        static const String g_empty_string;
        static const char g_hex_chars[16];  ///< Possible chars of hex formatted values
        static const char* g_ascii_8859_1[128];
        static const char* g_ascii_windows1252[128];

    public:
        String() noexcept;
        explicit String(int64_t capacity) noexcept;
        String(const char* str) noexcept;
        explicit String(const char* str, int64_t max_byte_length) noexcept;
        String(const String& string) noexcept;
        explicit String(const String* string) noexcept;
        explicit String(const String& string, int64_t character_index, int64_t character_length) noexcept;

        #if defined(__APPLE__) && defined(__MACH__)
            explicit String(CFStringRef cf_string) { set(cf_string); }
        #endif

        ~String() noexcept override;

        [[nodiscard]] const char* className() const noexcept override { return "String"; }

        friend std::ostream& operator << (std::ostream& os, const String* o);
        friend std::ostream& operator << (std::ostream& os, const String& o);

        static const String& emptyString() noexcept;

        void _init() noexcept;


        // Operators
        String& operator = (char c);
        String& operator = (const char* str);
        String& operator = (const String& other);
        String& operator = (const String* other);
        String& operator = (int8_t value);
        String& operator = (int16_t value);
        String& operator = (int32_t value);
        String& operator = (int64_t value);
        String& operator = (uint8_t value);
        String& operator = (uint16_t value);
        String& operator = (uint32_t value);
        String& operator = (uint64_t value);
        String& operator = (double value);
        String& operator = (Fix value);

        #if defined(__APPLE__) && defined(__MACH__)
            String& operator = (CFStringRef cf_string);
        #endif


        String& operator += (char c);
        String& operator += (const char* str);
        String& operator += (const String& other);
        String& operator += (int8_t value);
        String& operator += (int16_t value);
        String& operator += (int32_t value);
        String& operator += (int64_t value);
        String& operator += (uint8_t value);
        String& operator += (uint16_t value);
        String& operator += (uint32_t value);
        String& operator += (uint64_t value);
        String& operator += (double value);
        String& operator += (Fix value);

        String operator + (char c) const;
        String operator + (const char* str) const;
        String operator + (const String& other) const;
        String operator + (const String* other) const;
        String operator + (int8_t value) const;
        String operator + (int16_t value) const;
        String operator + (int32_t value) const;
        String operator + (int64_t value) const;
        String operator + (uint8_t value) const;
        String operator + (uint16_t value) const;
        String operator + (uint32_t value) const;
        String operator + (uint64_t value) const;
        String operator + (double value) const;
        String operator + (Fix value) const;

        bool operator == (const String& other) const;
        bool operator == (const char* str) const;


        [[nodiscard]] int64_t length() const noexcept;
        [[nodiscard]] int64_t byteLength() const noexcept;
        [[nodiscard]] char* mutDataPtr() noexcept;
        [[nodiscard]] const char* utf8() const noexcept;


        [[nodiscard]] bool isValidUtf8(int64_t* out_byte_index = nullptr) const noexcept;
        [[nodiscard]] bool isEmpty() const noexcept;
        [[nodiscard]] bool isNotEmpty() const noexcept;
        [[nodiscard]] bool isAscii() const noexcept;

        [[nodiscard]] bool isAlphaNumeric() const noexcept;
        [[nodiscard]] bool isValidNumber() const noexcept;
        [[nodiscard]] bool isQuoted() const noexcept;

        [[nodiscard]] bool isCharacterIndexInRange(int64_t char_index) const noexcept;
        [[nodiscard]] bool isByteIndexInRange(int64_t byte_index) const noexcept;

        [[nodiscard]] int64_t byteIndexFromCharacterIndex(int64_t character_index) const noexcept;
        [[nodiscard]] int64_t characterIndexFromByteIndex(int64_t byte_index) const noexcept;
        [[nodiscard]] int32_t utf8SeqLengthAtCharacterIndex(int64_t character_index) const noexcept;
        [[nodiscard]] int32_t utf8SeqLengthAtByteIndex(int64_t byte_index) const noexcept;

        [[nodiscard]] static int32_t utf8SeqLengthByStartByte(uint8_t start_byte) noexcept;
        [[nodiscard]] static int64_t utf8Length(const char* str) noexcept;

        [[nodiscard]] static uint32_t unicodeFromUtf8(const char* str) noexcept;
        [[nodiscard]] static bool unicodeIsWordCharacter(uint32_t unicode) noexcept;
        [[nodiscard]] static bool isUnicodeDelimiter(uint32_t unicode) noexcept;

        [[nodiscard]] uint32_t unicodeAtIndex(int64_t index) const noexcept;
        [[nodiscard]] bool isSpaceAtIndex(int64_t index) const noexcept;
        [[nodiscard]] bool isWordCharacterAtIndex(int64_t index) const noexcept;
        [[nodiscard]] bool isDelimiterAtIndex(int64_t index) const noexcept;

        bool clampCharacterRange(int64_t& character_index, int64_t& character_length) const noexcept;
        bool byteRangeFromCharacterRange(int64_t character_index, int64_t character_length, int64_t& out_byte_index, int64_t& out_byte_length) const noexcept;

        [[nodiscard]] int64_t whiteSpaceHead() const noexcept;
        [[nodiscard]] int64_t whiteSpaceTail() const noexcept;

        void clear() noexcept;
        bool trim(TrimMode trim_mode = TrimMode::All) noexcept;

        bool setChar(char c) noexcept;
        bool set(const char* str) noexcept;

        #if defined(__APPLE__) && defined(__MACH__)
            bool set(CFStringRef cf_string) noexcept;
        #endif

        bool set(const String& string, int64_t character_index, int64_t character_length) noexcept;
        bool set(const String& string) noexcept;
        bool set(const String* string) noexcept;
        bool setByStr(const char* str, int64_t start, int64_t end) noexcept;
        bool setByStr(const char* str) noexcept;
        bool setByStr(const char* str, int64_t length) noexcept;
        int64_t setByFramedContent(const char* str, char open_c, char close_c) noexcept;
        bool setByData(const Data* data, int32_t length) noexcept;
        void setElapsedTimeText(timestamp_t t) noexcept;
        ErrorCode setFormatted(int64_t max_byte_length, const char* format, ...) noexcept;

        ErrorCode readFromFile(const String& file_path) noexcept;

        ErrorCode appendFormatted(int64_t max_byte_length, const char* format, ...) noexcept;
        ErrorCode appendFormattedV(int64_t max_byte_length, const char* format, va_list args) noexcept;
        bool appendBool(bool v) noexcept;
        bool appendBoolTrueFalse(bool v) noexcept;
        bool appendBoolYesNo(bool v) noexcept;
        bool appendChar(char c) noexcept;
        bool appendChars(char c, int64_t n) noexcept;
        bool append(const char* str) noexcept;
        bool append(const char* str, int64_t max_byte_length) noexcept;
        bool append(const String& string) noexcept;
        bool append(const String* string) noexcept;
        bool append(const String& string, int64_t character_start, int64_t character_end) noexcept;
        int32_t appendCharacter(const String& string, int64_t character_index) noexcept;

        bool appendInt32(int32_t value) noexcept;
        bool appendUInt32(uint32_t value) noexcept;
        bool appendInt64(int64_t value) noexcept;
        bool appendUInt64(uint64_t value) noexcept;
        bool appendDouble(double value, int32_t precision = 8) noexcept;
        bool appendFix(const Fix& value, int32_t precision = 8) noexcept;

        bool insertAtCharacterIndex(const char* str, int64_t character_index) noexcept;
        bool insert(const String& string, int64_t character_index) noexcept;
        bool remove(int64_t character_index, int64_t character_length) noexcept;
        bool removeByRange(int64_t start_index, int64_t end_index) noexcept {
            return remove(start_index, end_index - start_index + 1);
        }
        bool truncate(int64_t character_index) noexcept;
        bool truncateStart(int64_t character_index) noexcept;

        bool replaceChar(int64_t character_index, const char* c) noexcept;
        bool replaceChar(int64_t character_index, const String& src_string, int64_t src_character_index) noexcept;
        int64_t replace(const String& search_string, const String& replacement_string) noexcept;
        int64_t replace(const char* search_str, const char* replacement_str) noexcept;

        ErrorCode removeScientificNotation() noexcept;
        void removeStringDoubleQuotes() noexcept;

        ErrorCode randomName(int64_t length) noexcept;
        ErrorCode randomName(const char* mask, const char* path = nullptr) noexcept;
        bool uuid() noexcept;

        [[nodiscard]] int64_t findAsciiChar(char c, int64_t index = 0) const noexcept;
        [[nodiscard]] int64_t find(const char* str, int64_t index = 0) const noexcept;
        [[nodiscard]] int64_t find(const String& string, int64_t index = 0) const noexcept;
        [[nodiscard]] int64_t findIgnoreCase(const String& string, int64_t index = 0) const noexcept;
        [[nodiscard]] int64_t findOneCharOf(const char* str, int64_t index = 0) const noexcept;
        [[nodiscard]] int64_t findOneCharOf(const String& string, int64_t index = 0) const noexcept;

        [[nodiscard]] int64_t count(const char* str, int64_t index = 0) const noexcept;
        [[nodiscard]] int64_t count(const String& string, int64_t index = 0) const noexcept;

        [[nodiscard]] bool compareAsciiAtIndex(char c, int64_t index) const noexcept;
        [[nodiscard]] int32_t compare(const char* str) const noexcept;
        [[nodiscard]] int32_t compare(const String& string) const noexcept;
        [[nodiscard]] int32_t compareAscii(const String& string, uint32_t offs, uint32_t offs_other, int32_t length = std::numeric_limits<int32_t>::max()) const noexcept;
        [[nodiscard]] int32_t compareIgnoreCase(const char* str) const noexcept;
        [[nodiscard]] int32_t compareIgnoreCase(const String& string) const noexcept;
        [[nodiscard]] bool isRGB() const { return compareIgnoreCase("rgb") == 0; }
        [[nodiscard]] bool isHSV() const { return compareIgnoreCase("hsv") == 0; }

        int64_t csvSplit(char delimiter, char quote, TrimMode trim_mode, StringList& out_list) const noexcept;
        int64_t subString(int64_t start, int64_t end, String& out_string) const noexcept;
        int64_t subString(int64_t start, String& out_string) const noexcept;
        int64_t trimmedSubString(int64_t start, int64_t end, String& out_string) const noexcept;
        int64_t trimmedSubString(int64_t start, String& out_string) const noexcept;

        bool isAsciiAtIndex(int64_t index, char& out_char) const noexcept;
        [[nodiscard]] char asciiAtIndex(int64_t index) const noexcept;
        [[nodiscard]] char firstAsciiChar() const noexcept;

        [[nodiscard]] const char* utf8AtIndex(int64_t char_index) const noexcept;
        bool utf8SubStr(int64_t index, int64_t length, int64_t max_byte_capacity, char* out_buffer) const noexcept;

        int32_t utf8CodeAtByteIndex(int64_t byte_index, char* out_buffer) const noexcept;
        [[nodiscard]] static bool utf8IsSoftLineBreak(const char* utf8_data) noexcept;
        [[nodiscard]] static bool utf8IsWhiteSpace(const char* utf8_data) noexcept;

        [[nodiscard]] static bool charIsWhiteSpace(char c) noexcept;
        [[nodiscard]] static bool charIsHexLetter(char c) noexcept;

        static int8_t valueForHexChar(char c) noexcept;
        static bool isValidHexString(const char* str) noexcept;

        [[nodiscard]] static const char* firstNonWhiteSpaceCharPtr(const char* str) noexcept;

        [[nodiscard]] static bool isBase64(char c) noexcept;

        [[nodiscard]] static int32_t utf8SeqLength(const uint8_t* c) noexcept;
        [[nodiscard]] static bool isValidUtf8(const uint8_t* str) noexcept;

        void fillBuffer(int64_t length, char* out_buffer) const noexcept;

        #if defined(__APPLE__) && defined(__MACH__)
            [[nodiscard]] CFStringRef createCFStringRef() const noexcept;
            [[nodiscard]] CFURLRef createCFURLRef() const noexcept;
            [[nodiscard]] static CFURLRef createCFURLRef(const char* path) noexcept;
        #endif

        [[nodiscard]] bool asBool() const noexcept;
        [[nodiscard]] int32_t asInt32() const noexcept;
        [[nodiscard]] static int32_t asInt32(const char* str) noexcept;
        [[nodiscard]] int64_t asInt64() const noexcept;
        [[nodiscard]] static int64_t asInt64(const char* str) noexcept;
        [[nodiscard]] float asFloat() const noexcept;
        [[nodiscard]] double asDouble() const noexcept;
        [[nodiscard]] static double asDouble(const char* str) noexcept;
        [[nodiscard]] Fix asFix() const noexcept;
        void toFix(Fix& out_fix) const noexcept;

        int32_t splitFast(char delimiter, int32_t max_parts, int32_t part_len, char* out_parts) const noexcept;

        [[nodiscard]] double shannonEntropy(bool bits_mode = false) const noexcept;

        [[nodiscard]] static int64_t itoa(int64_t value, char* buffer, int32_t radix) noexcept;

        [[nodiscard]] static inline bool isAlpha(char c);
        [[nodiscard]] static inline bool isDigit(char c);
        [[nodiscard]] static inline bool isExponentChar(char c);
        [[nodiscard]] static inline bool isSignChar(char c);

        bool selectWord(int32_t cursor_index, const StringList* word_characters, const StringList* custom_delimiters, Rangei& out_range) noexcept;

        void copyToPasteboard(int64_t character_index, int64_t character_length) noexcept;
        int64_t pasteFromPasteboard(int64_t character_index) noexcept;

        bool checkCapacity(int64_t needed) noexcept;
        bool checkCapacity(int64_t needed, int64_t min) noexcept;
        void _updateInternalLengthInfo() noexcept;

        [[nodiscard]] String fileExtension() const noexcept;
        [[nodiscard]] String fileBaseName() const noexcept;
        [[nodiscard]] String fileBaseNameWithoutExtension() const noexcept;
        [[nodiscard]] String fileDirPath() const noexcept;
        [[nodiscard]] String filePathWithChangedExtension(const String& extension) const noexcept;
        void buildFilePathAtDirWithRandomName(const String& file_path, int32_t file_name_length = 16) noexcept;

        ErrorCode loadText(const String& file_path) noexcept;
        [[nodiscard]] ErrorCode saveText(const String& file_path) const noexcept;

        // Utitily methods for C-string

        [[nodiscard]] inline static bool strSame(const char* str_a, const char* str_b) noexcept;

        [[nodiscard]] static bool unicharIsNumeric(uint16_t c) noexcept;

        [[nodiscard]] static int64_t strUtf8Length(const char* str) noexcept;
        [[nodiscard]] static bool strEndsWith(const char* str, const char* ending, bool case_sensitive) noexcept;
        static void strFromDouble(double value, int32_t fractional_digits, int32_t max_out_size, char* out_str) noexcept;

        static bool strHexFromData(const uint8_t* data, size_t length, char* out_str) noexcept;
        static int64_t strHexFromType(void* ptr, int32_t byte_count, bool prefixed, char* out_str) noexcept;

        static void strHexFromFloat(float value, bool prefixed, char* out_str) noexcept;
        static void strHexFromDouble(double value, bool prefixed, char* out_str) noexcept;

        template <typename U>
        static bool strToVar(const char* str, U& out_value) noexcept {
            // Unsupported type
            return false;
        }

        // integral specialization
        template <typename U>
        requires std::is_integral_v<U>
        static bool strToVar(const char* str, U& out_value) noexcept {
            if (!str || !*str) return false;

            char* end = nullptr;

            if constexpr (std::is_signed_v<U>) {
                auto val = std::strtoll(str, &end, 10);
                if (end == str || val < std::numeric_limits<U>::min() || val > std::numeric_limits<U>::max()) return false;
                out_value = static_cast<U>(val);
                return true;
            }
            else {
                auto val = std::strtoull(str, &end, 10);
                if (end == str || val > std::numeric_limits<U>::max()) return false;
                out_value = static_cast<U>(val);
                return true;
            }
        }


        static bool strToFloat(const char* str, float& out_value) noexcept;
        static bool strToDouble(const char* str, double& out_value) noexcept;

        static double parseDoubleWithDotOrComma(const char* str) {
            char buffer[128];  // Adjust size as needed
            size_t i = 0;

            while (*str && i < sizeof(buffer) - 1) {
                buffer[i++] = (*str == ',') ? '.' : *str;
                ++str;
            }
            buffer[i] = String::EOS;
            return atof(buffer);
        }

        static int32_t strHexToUInt8Array(const char* str, int32_t max_length, uint8_t* out_array) noexcept;

        static int32_t utf8CodeToStr(uint32_t code, char out_str[5]) noexcept;

        [[nodiscard]] static int32_t indexForStrInArray(const char* str, const char** str_array) noexcept;

        static char* timeStrFromSeconds(int64_t seconds, int32_t max_out_size, char* out_str) noexcept;
        static void fpsStr(double fps, int32_t max_out_size, char* out_str) noexcept;

        [[nodiscard]] static char randomNameChar() noexcept;
        static ErrorCode randomName(int64_t length, char* out_str) noexcept;
        [[nodiscard]] static int64_t randomNameLength(const char* mask, const char* path) noexcept;
        static ErrorCode randomName(const char* mask, const char* path, int64_t max_outSize, char* out_str) noexcept;

        static int32_t replaceChar(char* str, char search_c, char replacement_c) noexcept;

        [[nodiscard]] static const char* charSetName(CharSet char_set) noexcept;
        [[nodiscard]] static const char** extendedAsciiTable(CharSet char_set) noexcept;
        [[nodiscard]] static int32_t extendedAsciiToUTF8(uint8_t ascii_code, CharSet char_set, char* out_utf8_code) noexcept;

        [[nodiscard]] static uint64_t fnv1a_hash(const char* str) {
            uint64_t hash = 14695981039346656037ULL;  // FNV offset basis
            while (*str) {
                hash ^= static_cast<unsigned char>(*str++);
                hash *= 1099511628211ULL;  // FNV prime
            }
            return hash;
        }

    private:
        bool _checkExtraCapacity(int64_t needed) noexcept;
        void _removeData(int64_t byte_index, int64_t byty_length, int64_t character_length) noexcept;
    };

    class StringRing : Object {
    protected:
        uint32_t size_ = 0;
        int32_t pos_ = -1;
        int32_t index_ = -1;
        String* *strings_ = nullptr;

    public:
        explicit StringRing(uint32_t size) : Object() {
            size_ = size < 8 ? 8 : size;
            strings_ = static_cast<String**>(std::malloc(sizeof(String*) * size));
            if (strings_) {
                for (uint32_t i = 0; i < size; i++) {
                    strings_[i] = nullptr;
                }
            }
        }

        ~StringRing() override {
            if (strings_) {
                for (uint32_t i = 0; i < size_; i++) {
                    delete strings_[i];
                }
                free(strings_);
            }
        }

        friend std::ostream& operator << (std::ostream& os, const StringRing& o) {
            for (int32_t i = 0; i < static_cast<int32_t>(o.size_); i++) {
                std::cout << i << ": " << o.read(i) << std::endl;
            }
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const StringRing* o) {
            o == nullptr ? os << "StringRing nullptr" : os << *o;
            return os;
        }

        [[nodiscard]] uint32_t size() const noexcept { return size_; }

        void write(const String& string) noexcept;
        void write(const char* str) noexcept;
        void writeFormatted(const char* format, ...) noexcept;
        void writeError(const char* format, ...) noexcept;
        [[nodiscard]] const char* read(int32_t index = 0) const noexcept;
    };


} // End of namespace Grain

#endif // GrainString_hpp
