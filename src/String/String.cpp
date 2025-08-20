//
//  String.cpp
//
//  Created by Roald Christesen on 13.02.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 20.08.2025
//

#include "String/String.hpp"
#include "Math/Random.hpp"
#include "Time/Timestamp.hpp"

#include <libgen.h>
#include <uuid/uuid.h>
#include <cstdarg>


#if defined(__APPLE__) && defined(__MACH__)
    #include <CoreFoundation/CoreFoundation.h>
#endif


namespace Grain {

    #if defined(__APPLE__) && defined(__MACH__)
        void _macosApp_copyToPasteboard(String* string, int64_t character_index, int64_t character_length);
        int32_t _macosApp_pasteFromPasteboard(String* string, int64_t character_index);
    #endif

    const char* String::g_empty_data = "";
    const String String::g_empty_string;
    const char String::g_hex_chars[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    const char* String::g_ascii_8859_1[128] = {  // ASCII 8859-1 Latin-1
        " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ",
        " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ",
        " ", "¡", "¢", "£", "¤", "¥", "¦", "§", "¨", "©", "ª", "«", "¬", "­", "®", "¯",
        "°", "±", "²", "³", "´", "µ", "¶", "·", "¸", "¹", "º", "»", "¼", "½", "¾", "¿",
        "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "Ç", "È", "É", "Ê", "Ë", "Ì", "Í", "Î", "Ï",
        "Ð", "Ñ", "Ò", "Ó", "Ô", "Õ", "Ö", "×", "Ø", "Ù", "Ú", "Û", "Ü", "Ý", "Þ", "ß",
        "à", "á", "â", "ã", "ä", "å", "æ", "ç", "è", "é", "ê", "ë", "ì", "í", "î", "ï",
        "ð", "ñ", "ò", "ó", "ô", "õ", "ö", "÷", "ø", "ù", "ú", "û", "ü", "ý", "þ", "ÿ"
    };

    const char* String::g_ascii_windows1252[128] = {
        "€",  "", "‚", "ƒ", "„", "…", "†", "‡", "ˆ", "‰", "Š", "‹", "Œ",  "", "Ž",  "",
        "", "‘", "’", "“", "”", "•", "–", "—", "˜", "™", "š", "›", "œ",  "", "ž", "Ÿ",
        " ", "¡", "¢", "£", "¤", "¥", "¦", "§", "¨", "©", "ª", "«", "¬",  "­", "®", "¯",
        "°", "±", "²", "³", "´", "µ", "¶", "·", "¸", "¹", "º", "»", "¼", "½", "¾", "¿",
        "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "Ç", "È", "É", "Ê", "Ë", "Ì", "Í", "Î", "Ï",
        "Ð", "Ñ", "Ò", "Ó", "Ô", "Õ", "Ö", "×", "Ø", "Ù", "Ú", "Û", "Ü", "Ý", "Þ", "ß",
        "à", "á", "â", "ã", "ä", "å", "æ", "ç", "è", "é", "ê", "ë", "ì", "í", "î", "ï",
        "ð", "ñ", "ò", "ó", "ô", "õ", "ö", "÷", "ø", "ù", "ú", "û", "ü", "ý", "þ", "ÿ"
    };


    void Utf8SingleByteDelimiterStates::setByChar(char c) {

        if ((c & 0x80) == 0x0) {
            // Only if c is a valid 7 bit ASCII character
            m_flags[c] |= kFlagDelimiter;
            if (String::charIsWhiteSpace(c)) {
                m_flags[c] |= kFlagWhiteSpace;
            }
        }
    }


    void Utf8SingleByteDelimiterStates::setByCharsInStr(const char* str) {

        if (str) {
            const char* c = str;
            while (*c != String::EOS) {
                setByChar(*c);
                c++;
            }
        }
    }


    uint8_t Utf8SingleByteDelimiterStates::check(char c) {

        if ((c & 0x80) == 0x0) {
            // Only if c is a valid 7 bit ASCII character
            return m_flags[static_cast<uint8_t>(c)];
        }
        else {
            return kFlagsNone;
        }
    }


    /**
     *  @brief Constructs an empty String with a capacity of `kDefaultByteCapacity` bytes.
     */
    String::String() noexcept {

        checkCapacity(kDefaultByteCapacity);
    }


    /**
     *  @brief Constructs an empty String with a specific capacity.
     */
    String::String(int64_t capacity) noexcept {

        capacity = std::max(capacity, int64_t(16));
        checkCapacity(capacity);
    }


    /**
     *  @brief Constructs a String with a copy of a given C-string.
     *
     *  @param str A pointer to a C-string. A nullptr results in an empty string with the default capacity.
     */
    String::String(const char* str) noexcept {

        if (!str) {
            checkCapacity(kDefaultByteCapacity);
        }
        else {
            auto capacity = static_cast<int64_t>(strlen(str));
            checkCapacity(capacity, kDefaultByteCapacity);
            append(str);
        }
    }


    /**
     *  @brief Constructs a string with a copy of a given C-string, limited to a maximum byte length.
     *
     *  @param str A pointer to a C-string.
     *  @param max_byte_length Limits the length of the resulting string. If the length
     *  of the input string exceeds max_length, it will be truncated.
     */
    String::String(const char* str, int64_t max_byte_length) noexcept {

        if (!str) {
            checkCapacity(kDefaultByteCapacity);
        }
        else {
            checkCapacity(max_byte_length + 1, kDefaultByteCapacity);
            setByStr(str, max_byte_length);
        }
    }


    /**
     *  @brief Constructs a String as a copy of another String.
     *
     *  @param string A reference to the string which should be copied. The original string is not modified.
     */
    String::String(const String& string) noexcept {

        checkCapacity(string.length(), 32);
        append(string.utf8());
    }


    /**
     *  @brief Constructs a String as a copy of another String.
     *
     *  @param string A pointer to the string which should be copied. The original string is not modified.
     */
    String::String(const String* string) noexcept {

        if (string) {
            checkCapacity(string->length(), 32);
            append(string->utf8());
        }
    }


    /**
     *  @brief Constructs a String with a substring of another String.
     *
     *  @param string The source String from which to extract the substring.
     *  @param character_index The character index of the substring.
     *  @param character_length The character length of the substring.
     */
    String::String(const String& string, int64_t character_index, int64_t character_length) noexcept {

        string.subString(character_index, character_index + character_length - 1, *this);
    }


    String::~String() noexcept {

        std::free(m_data);
    }


    const char* String::className() const noexcept {

        return "String";
    }


    std::ostream& operator << (std::ostream& os, const String* o) {

        o == nullptr ? os << "String nullptr" : os << *o;
        return os;
    }


    std::ostream& operator << (std::ostream& os, const String& o) {

        os << o.utf8();
        return os;
    }


    const String& String::emptyString() noexcept {

        return g_empty_string;
    }


    void String::_init() noexcept {

        if (m_data) {
            free(m_data);
        }

        m_data = nullptr;
        m_byte_length = 0;
        m_character_length = 0;
        m_byte_capacity = 0;
        m_extra_grow_bytes = 32;
        m_grow_count = 0;
    }


    String& String::operator = (const String& other) { this->set(other); return *this; }
    String& String::operator = (const String* other) { this->set(other); return *this; }
    String& String::operator = (const char* str) { this->set(str); return *this; }
    String& String::operator = (char c) { this->setChar(c); return *this; }

    #if defined(__APPLE__) && defined(__MACH__)
        String& String::operator = (const CFStringRef cf_string) { this->set(cf_string); return *this; }
    #endif

    String& String::operator += (char c) { this->appendChar(c); return *this; }
    String& String::operator += (const char* str) { this->append(str); return *this; }
    String& String::operator += (const String& other) { this->append(other); return *this; }
    String& String::operator += (int8_t value) { this->appendInt64(value); return *this; }
    String& String::operator += (int16_t value) { this->appendInt64(value); return *this; }
    String& String::operator += (int32_t value) { this->appendInt64(value); return *this; }
    String& String::operator += (int64_t value) { this->appendInt64(value); return *this; }
    String& String::operator += (uint8_t value) { this->appendUInt64(value); return *this; }
    String& String::operator += (uint16_t value) { this->appendUInt64(value); return *this; }
    String& String::operator += (uint32_t value) { this->appendUInt64(value); return *this; }
    String& String::operator += (uint64_t value) { this->appendUInt64(value); return *this; }
    String& String::operator += (double value) { this->appendDouble(value); return *this; }
    String& String::operator += (Fix value) { this->appendFix(value); return *this; }

    String String::operator + (char c) const { String result(*this); result.appendChar(c); return result; }
    String String::operator + (const char* str) const { String result(*this); result.append(str); return result; }
    String String::operator + (const String& other) const { String result(*this); result.append(other); return result; }
    String String::operator + (const String* other) const { String result(*this); result.append(*other); return result; }
    String String::operator + (int8_t value) const { String result(*this); result.appendInt64(value); return result; }
    String String::operator + (int16_t value) const { String result(*this); result.appendInt64(value); return result; }
    String String::operator + (int32_t value) const { String result(*this); result.appendInt64(value); return result; }
    String String::operator + (int64_t value) const { String result(*this); result.appendInt64(value); return result; }
    String String::operator + (uint8_t value) const { String result(*this); result.appendUInt64(value); return result; }
    String String::operator + (uint16_t value) const { String result(*this); result.appendUInt64(value); return result; }
    String String::operator + (uint32_t value) const { String result(*this); result.appendUInt64(value); return result; }
    String String::operator + (uint64_t value) const { String result(*this); result.appendUInt64(value); return result; }
    String String::operator + (double value) const { String result(*this); result.appendDouble(value); return result; }
    String String::operator + (Fix value) const { String result(*this); result.appendFix(value); return result; }

    bool String::operator == (const String& other) const { return this->compare(other) == 0; }
    bool String::operator == (const char* str) const { return this->compare(str) == 0; }


    int64_t String::length() const noexcept {

        return m_character_length;
    }


    int64_t String::byteLength() const noexcept {

        return m_byte_length;
    }


    char* String::mutDataPtr() noexcept {

        return m_data;
    }


    const char* String::utf8() const noexcept {

        return m_data ? m_data : String::g_empty_data;
    }


    /**
     *  @brief Check if the string contains valid UTF-8 encoding.
     *
     *  @param[out] out_byte_index If the string contains invalid UTF-8, this variable
     *              will be set to the index of the byte where the encoding error occurred.
     *
     *  @return `true` if the string contains valid UTF-8 encoding, `false` otherwise.
     */
    bool String::isValidUtf8(int64_t* out_byte_index) const noexcept {

        if (!m_data) {
            return false;
        }

        int64_t byte_index = 0;

        while (byte_index < m_byte_length) {
            int32_t seq_length = String::utf8SeqLength((uint8_t*)&m_data[byte_index]);

            if (seq_length < 1) {
                if (out_byte_index) {
                    *out_byte_index = byte_index;
                }
                return false;
            }

            byte_index += seq_length;
        }

        if (out_byte_index) {
            *out_byte_index = byte_index;
        }

        return true;
    }


    bool String::isEmpty() const noexcept {

        return m_character_length < 1;
    }


    bool String::isNotEmpty() const noexcept {

        return m_character_length > 0;
    }


    /**
     *  @brief Check if all chars are pure 7 bit ASCII.
     *
     *  @return `true` if all chars are 7 bit ASCII.
     */
    bool String::isAscii() const noexcept {

        if (m_data && m_byte_length > 0) {
            int64_t n = m_byte_length;
            auto p = (uint8_t*)m_data;

            while (n--) {
                if (*p & 0x80) {
                    return false;
                }
                p++;
            }

            return true;
        }

        return false;
    }


    /**
     *  @brief Check if the string consists only of alphanumeric characters.
     *
     *  This function checks if the string contains only letters (A-Z, a-z)
     *  and digits (0-9). If the string is empty or contains any special
     *  characters, it returns false.
     *
     *  @return `true` if the string is alphanumeric, false otherwise.
     */
    bool String::isAlphaNumeric() const noexcept {

        if (!m_data || m_data[0] == String::EOS) {
            // Empty string is not considered alphanumeric
            return false;
        }

        int64_t i = 0;
        int64_t length = byteLength();

        while (i < length) {
            if (!isAlpha(m_data[i]) && !isDigit(m_data[i])) {
                return false;  // Found a non-alphanumeric character
            }
            i++;
        }

        return true;  // All characters are either letters or digits
    }


    /**
     *  @brief Check if string represents a valid natural or real number.
     *
     *  @return `true` if string represents a valid natural or real number.
     */
    bool String::isValidNumber() const noexcept {

        if (!m_data || m_data[0] == String::EOS) {
            // Empty string is not a valid number
            return false;
        }

        int64_t i = 0;
        int64_t length = byteLength();

        // Check for optional sign
        if (isSignChar(m_data[i])) {
            i++;
        }

        // Check for digits before the decimal point
        while (i < length && isDigit(m_data[i])) {
            i++;
        }

        // Check for optional decimal point
        if (i < length && m_data[i] == '.') {
            i++;
        }

        // Check for digits after the decimal point
        while (i < length && isDigit(m_data[i])) {
            i++;
        }

        // Check for optional exponent part
        if (i < length && isExponentChar(m_data[i])) {
            i++;

            // Check for optional sign after the exponent character
            if (i < length && isSignChar(m_data[i])) {
                i++;
            }

            // Check for digits in the exponent part
            while (i < length && isDigit(m_data[i])) {
                i++;
            }
        }

        // The string is valid if we have consumed the entire input
        return i == length;
    }


    /**
     *  @brief Check if string starts and ends with double quotes.
     *
     *  @return `true`, if string starts and ends with double quotes.
     */
    bool String::isQuoted() const noexcept {

        return m_data != nullptr && m_byte_length > 1 && m_data[0] == '"' && m_data[m_byte_length - 1] == '"';
    }


    /**
     *  @brief Check if a character index is in a valid range.
     *
     *  @param char_index The index to check.
     *
     *  @return `true` if index is within a valid range.
     */
    bool String::isCharacterIndexInRange(int64_t char_index) const noexcept {

        return m_data != nullptr && char_index >= 0 && char_index < m_character_length;
    }


    /**
     *  @brief Check if a byte index is in a valid range.
     *
     *  @param byte_index The byte index to check.
     *
     *  @return `true` if `byte_index` is within a valid range.
     */
    bool String::isByteIndexInRange(int64_t byte_index) const noexcept {

        return m_data != nullptr && byte_index >= 0 && byte_index < m_byte_length;
    }


    /**
     *  @brief Convert a character index to a byte index.
     *
     *  @param character_index The character index to be converted.
     *
     *  @return The corresponding byte index for the given `character_index` or -1,
     *          if the provided `character_index` is out of range.
     */
    int64_t String::byteIndexFromCharacterIndex(int64_t character_index) const noexcept {

        if (!isCharacterIndexInRange(character_index)) {
            return -1;
        }

        int64_t byte_index = 0;

        for (int64_t i = 0; i < character_index; i++) {
            int64_t seq_length = utf8SeqLengthAtByteIndex(byte_index);

            if (seq_length < 1) {
                return -2;
            }

            byte_index += seq_length;
        }

        return byte_index;
    }


    /**
     *  @brief Convert a byte index to a character index.
     *
     *  @param byte_index The byte index to be converted.
     *
     *  @return The corresponding character index for the given `byte_index` or -1
     *          if the provided `byte_index` is out of range.
     */
    int64_t String::characterIndexFromByteIndex(int64_t byte_index) const noexcept {

        if (!m_data || byte_index >= m_byte_length) {
            return -1;
        }

        int64_t index = 0;

        for (int64_t i = 0; i < byte_index; ) {
            int64_t seq_length = utf8SeqLengthAtByteIndex(i);

            if (seq_length < 1) {
                return -2;
            }

            i += seq_length;
            index++;
        }

        return index;
    }


    /**
     *  @brief Get the length of a UTF-8 coded character sequence.
     *
     *  @param character_index The UTF-8 character index where the UTF-8 sequence starts.
     *
     *  @return The length of the sequence in bytes or 0 if the sequence is invalid or
     *         `character_index` is out of range.
     */
    int32_t String::utf8SeqLengthAtCharacterIndex(int64_t character_index) const noexcept {

        auto byte_index = byteIndexFromCharacterIndex(character_index);
        return utf8SeqLengthAtByteIndex(byte_index);
    }


    /**
     *  @brief Get the length of a UTF-8 coded character sequence.
     *
     *  @param byte_index The byte index (in memory) where the UTF-8 sequence starts.
     *
     *  @return The length of the sequence in bytes or 0 if the sequence is invalid or
     *         `byte_index` is out of range.
     */
    int32_t String::utf8SeqLengthAtByteIndex(int64_t byte_index) const noexcept {

        return isByteIndexInRange(byte_index) ? utf8SeqLengthByStartByte(m_data[byte_index]) : 0;
    }


    /**
     *  @brief Get the number of bytes in a UTF-8 sequence.
     *
     *  @param start_byte The starting byte of the UTF-8 code sequence.
     *
     *  @return The length in bytes or 0 if `start_byte` doesn´t represents a UTF-8 start byte.
     */
    int32_t String::utf8SeqLengthByStartByte(uint8_t start_byte) noexcept {

        if (start_byte < 0x80) {
            return 1;  // ASCII character (single byte)
        }
        else if ((start_byte & 0xE0) == 0xC0) {
            return 2;  // 2-byte sequence
        }
        else if ((start_byte & 0xF0) == 0xE0) {
            return 3;  // 3-byte sequence
        }
        else if ((start_byte & 0xF8) == 0xF0) {
            return 4;  // 4-byte sequence
        }
        else {
            return 0;  // Invalid UTF-8 sequence
        }
    }


    /**
     *  @brief Get the number of Unicode characters in a given UTF-8 encoded C-string.
     *
     *  This function counts the number of Unicode characters in a UTF-8 encoded C-string,
     *  where a single character can be represented by one or more bytes.
     *
     *  @param str Pointer to a UTF-8 encoded C-string.
     *
     *  @return The number of Unicode characters in `str` or a negative error code
     *          if `str` is a null pointer or contains invalid UTF-8 encoding.
     */
    int64_t String::utf8Length(const char* str) noexcept {

        if (!str) {
            return -1;
        }
        else {
            auto n = static_cast<int64_t>(strlen(str));
            int64_t length = 0;

            for (int64_t byte_index = 0; byte_index < n; ) {
                int64_t seq_length = utf8SeqLengthByStartByte(str[byte_index]);

                if (seq_length < 1) {
                    return -1;  // UTF-8 sequence error
                }

                byte_index += seq_length;
                length++;
            }

            return length;
        }
    }


    /**
     *  @brief Get the Unicode value for a UTF-8 encoded character.
     *
     *  @param str Pointer to a UTF-8 encoded C-string.
     *  @return The Unicode value.
     */
    uint32_t String::unicodeFromUtf8(const char* str) noexcept {

        uint32_t unicode = 0;

        if (str != nullptr && str[0] != String::EOS) {
            uint8_t byte = str[0];

            if (byte < 0x80) {
                unicode = byte;
            }
            else if ((byte & 0xE0) == 0xC0) {
                unicode = (byte & 0x1F) << 6;
                unicode |= (str[1] & 0x3F);
            }
            else if ((byte & 0xF0) == 0xE0) {
                unicode = (byte & 0x0F) << 12;
                unicode |= (str[1] & 0x3F) << 6;
                unicode |= (str[2] & 0x3F);
            }
            else if ((byte & 0xF8) == 0xF0) {
                unicode = (byte & 0x07) << 18;
                unicode |= (str[1] & 0x3F) << 12;
                unicode |= (str[2] & 0x3F) << 6;
                unicode |= (str[3] & 0x3F);
            }
        }

        return unicode;
    }


    /**
     *  @brief Checks if a Unicode character is considered a word character.
     *
     *  This function determines if a given Unicode character is a word character.
     *  Word characters typically include letters (both uppercase and lowercase),
     *  digits, and the underscore character ('_').
     *
     *  @param unicode The Unicode code point to check.
     *  @return `true` if the character is a word character, `false` otherwise.
     *
     *  @note The function currently supports basic Latin letters, digits,
     *  underscore, and Latin-1 Supplement and Latin Extended-A/B ranges.
     */
    bool String::unicodeIsWordCharacter(uint32_t unicode) noexcept {

        // Check if the character is a letter or digit
        if ((unicode >= 0x0041 && unicode <= 0x005A) || // A-Z
            (unicode >= 0x0061 && unicode <= 0x007A) || // a-z
            (unicode >= 0x0030 && unicode <= 0x0039) || // 0-9
            (unicode == 0x005F)) { // underscore (_)
            return true;
        }

        // Check other Unicode ranges for letters and numbers
        if ((unicode >= 0x00C0 && unicode <= 0x00D6) || // À-Ö
            (unicode >= 0x00D8 && unicode <= 0x00F6) || // Ø-ö
            (unicode >= 0x00F8 && unicode <= 0x00FF) || // ø-ÿ
            (unicode >= 0x0100 && unicode <= 0x017F) || // Latin Extended-A
            (unicode >= 0x0180 && unicode <= 0x024F) || // Latin Extended-B
            (unicode >= 0x0250 && unicode <= 0x02AF) || // IPA Extensions
            (unicode >= 0x02B0 && unicode <= 0x02FF) || // Spacing Modifier Letters
            (unicode >= 0x0300 && unicode <= 0x036F) || // Combining Diacritical Marks
            (unicode >= 0x1E00 && unicode <= 0x1EFF) || // Latin Extended Additional
            (unicode >= 0x1F00 && unicode <= 0x1FFF)) { // Greek Extended
            return true;
        }

        return false;
    }


    /**
     *  @brief Checks if a Unicode character is considered a delimiter.
     *
     *  This function determines if a given Unicode character is a delimiter.
     *  Delimiters typically include spaces, punctuation marks, and various symbols.
     *
     *  @param unicode The Unicode code point to check.
     *  @return `true` if the character is a delimiter, `false` otherwise.
     *
     *  @note The function currently supports basic ASCII punctuation and space,
     *        as well as General Punctuation and CJK Symbols and Punctuation ranges.
     */
    bool String::isUnicodeDelimiter(uint32_t unicode) noexcept {

        // Check if the character is a space or punctuation
        if (unicode == 0x0020 ||  // Space
            unicode == 0x0009 ||  // Horizontal Tab
            unicode == 0x000A ||  // Line Feed
            unicode == 0x000D ||  // Carriage Return
            unicode == 0x00A0 ||  // Non-breaking space
            (unicode >= 0x0021 && unicode <= 0x002F) ||  // Punctuation !"#$%&'()*+,-./
            (unicode >= 0x003A && unicode <= 0x0040) ||  // Punctuation :;<=>?@
            (unicode >= 0x005B && unicode <= 0x0060) ||  // Punctuation [\]^_`
            (unicode >= 0x007B && unicode <= 0x007E)) {  // Punctuation {|}~
            return true;
        }

        // Check other Unicode ranges for punctuation and symbols
        if ((unicode >= 0x2000 && unicode <= 0x206F) ||  // General Punctuation
            (unicode >= 0x3000 && unicode <= 0x303F) ||  // CJK Symbols and Punctuation
            (unicode >= 0x2010 && unicode <= 0x2015) ||  // Dashes ‐–—―
            (unicode >= 0x2018 && unicode <= 0x201F) ||  // Various quotes ‘ ’ ‚ ‛ “ ” „ ‟
            (unicode >= 0x2039 && unicode <= 0x203A) ||  // Single guillemets ‹ ›
            (unicode >= 0x00AB && unicode <= 0x00BB) ||  // Double guillemets « »
            unicode == 0x0024 ||  // Dollar Sign
            unicode == 0x20AC ||  // Euro Sign
            unicode == 0x00A3 ||  // Pound Sign
            unicode == 0x00A5) {  // Yen Sign
            return true;
        }

        return false;
    }


    uint32_t String::unicodeAtIndex(int64_t index) const noexcept {

        return String::unicodeFromUtf8(utf8AtIndex(index));
    }


    /**
     *  @brief Check if a given character at a specific index represents a
     *         whitespace character.
     *
     *  @param index The index of the character to test.
     *  @return `true` if character represents a white space character.
     */
    bool String::isSpaceAtIndex(int64_t index) const noexcept {

        return utf8IsWhiteSpace(utf8AtIndex(index));
    }


    bool String::isWordCharacterAtIndex(int64_t index) const noexcept {

        return unicodeIsWordCharacter(unicodeAtIndex(index));
    }


    bool String::isDelimiterAtIndex(int64_t index) const noexcept {

        return unicodeIsWordCharacter(unicodeAtIndex(index));
    }


    /**
     *  @brief Clamp given character index and character length values for accessin
     *         parts of the string.
     *
     *  @param [in,out] character_index - A given index. The resulting index will be
     *                                    updated.
     *  @param [in,out] character_length - A given length. The resulting length will
     *                                     be updated.
     *
     *  @return `true`, if the provided character index and character length values
     *          could be clamped to represent a valid part of the string.
     */
    bool String::clampCharacterRange(int64_t& character_index, int64_t& character_length) const noexcept {

        if (m_data != nullptr) {
            if (character_index < m_character_length && character_length > 0) {
                if (character_index < 0) {
                    character_length += character_index;
                    character_index = 0;
                }

                int64_t max_length = m_character_length - character_index;
                if (character_length > max_length) {
                    character_length = max_length;
                }

                return character_length > 0;
            }
        }

        character_length = 0;

        return false;
    }


    /**
     *  @brief Converts a range from character space to byte space.
     *
     *  @param character_index An index in character space.
     *  @param character_length A length in character space.
     *  @param[out] out_byte_index The resulting index in byte space.
     *  @param[out] out_byte_length The resulting length in byte space.
     *
     *  @return `true` on success.
     */
    bool String::byteRangeFromCharacterRange(int64_t character_index, int64_t character_length, int64_t& out_byte_index, int64_t& out_byte_length) const noexcept {

        if (clampCharacterRange(character_index, character_length)) {
       out_byte_index = byteIndexFromCharacterIndex(character_index);

            int64_t last_index = character_index + character_length - 1;
            int64_t last_byte_index = byteIndexFromCharacterIndex(last_index);
            int64_t last_seq_length = utf8SeqLengthAtByteIndex(last_byte_index);

            if (isByteIndexInRange(out_byte_index)) {
                out_byte_length = (last_byte_index + last_seq_length) - out_byte_index;
                return out_byte_length > 0;
            }
        }

        return false;
    }


    /**
     *  @brief Count whitespace characters at the head.
     *
     *  @return Number of whitespaces or a negative error code.
     */
    int64_t String::whiteSpaceHead() const noexcept {

        if (!m_data) {
            return -1;  // Return error code
        }

        int64_t byte_index = 0;
        int64_t n = 0;  // Whitespaces counter

        char* p = m_data;

        while (*p != 0) {
            int64_t seq_length = utf8SeqLengthAtByteIndex(byte_index);

            if (seq_length < 1) {
                return -1;  // Error code
            }

            if (!utf8IsWhiteSpace(p)) {
                return n;
            }

            n++;

            // Check if there is an EOS in the data!
            // Should never occure in valid strings
            for (int64_t i = 1; i < seq_length; i++) {
                p++;
                if (*p == 0) {
                    return -1;  // Return error code
                }
            }

            p++;
            byte_index += seq_length;
        }

        return n;
    }


    /**
     *  @brief Count whitespace characters at the tail.
     *
     *  @return Number of whitespaces or a negative error code.
     */
    int64_t String::whiteSpaceTail() const noexcept {

// TODO: Should be optimized by looking for white spaces from the tail side.

        if (!m_data) {
            return -1;  // Return error code
        }

        int64_t byte_index = 0;
        int64_t n = 0;  // Whitespaces counter

        char* p = m_data;

        while (*p != 0) {
            int64_t seq_length = utf8SeqLengthAtByteIndex(byte_index);

            if (seq_length < 1) {
                return -1;  // Error code
            }

            if (utf8IsWhiteSpace(p)) {
                n++;
            }
            else {
                n = 0;  // Restart counting
            }

            // Check if there is an EOS in the data!
            // Should not occure in valid strings
            for (int64_t i = 1; i < seq_length; i++) {
                p++;
                if (*p == 0) {
                    return -1;  // Return error code
                }
            }

            p++;
            byte_index += seq_length;
        }

        return n;
    }


    /**
     *  @brief Clear (empty) the string.
     *
     *  @note The memory will notbe changed.
     */
    void String::clear() noexcept {

        m_byte_length = m_character_length = 0;
        if (m_data != nullptr) {
            m_data[0] = 0;
        }
    }


    /**
     *  @brief Removes leading and trailing whitespace characters from a string.
     *
     *  This methos removes any leading and trailing whitespace characters, including
     *  spaces, tabs, and newline characters, from the string.
     *
     *  @return `true` if whitespace characters were removed, `false` otherwise.
     */
    bool String::trim(TrimMode trim_mode) noexcept {

        if (!m_data) {
            return false;
        }

        int64_t head = 0;
        int64_t tail = 0;
        int64_t new_length = length();
        int64_t byte_index = 0;
        int64_t byte_length = 0;

        if (trim_mode == TrimMode::Head || trim_mode == TrimMode::All) {
            head = whiteSpaceHead();
        }

        if (trim_mode == TrimMode::Tail || trim_mode == TrimMode::All) {
            tail = whiteSpaceTail();
        }

        if (head > 0) {
            byteRangeFromCharacterRange(0, head, byte_index, byte_length);
            memmove(m_data, &m_data[byte_length], m_byte_length - byte_length + 1);
            new_length -= head;
        }

        if (tail > 0) {
            byteRangeFromCharacterRange(new_length - tail, 1, byte_index, byte_length);
            if (byte_index >= 0 && byte_index < m_byte_length) {
                m_data[byte_index] = 0;
            }
        }

        _updateInternalLengthInfo();

        return (head + tail) > 0;
    }


    /**
     *  @brief Set the content of the string using a char.
     *
     *  @param c A pointer to a C-string.
     *  @return `true` if the method succeeded.
     */
    bool String::setChar(char c) noexcept {

        clear();
        return appendChar(c);
    }


    /**
     *  @brief Set the content of the string using a C-string.
     *
     *  @param str A pointer to a C-string.
     *  @return `true` if the method succeeded.
     *
     *  @note If str is a nullptr, the string will be empty.
     */
    bool String::set(const char* str) noexcept {

        clear();
        return append(str);
    }


    /**
     *  @brief Set the content of the string using a CFStringRef.
     *
     *  @param cf_string A Core Foundation String Reference.
     *  @return `true` if the method succeeded.
     *
     *  @note If `cf_string` is a nullptr, the string will be empty.
     */
    #if defined(__APPLE__) && defined(__MACH__)
        bool String::set(CFStringRef cf_string) noexcept {

            bool result = false;

            if (cf_string != nullptr) {
                CFIndex length = CFStringGetLength(cf_string);
                CFIndex max_size = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;

                checkCapacity(max_size);

                if (CFStringGetCString(cf_string, m_data, max_size, kCFStringEncodingUTF8)) {
                    result = true;
                }
            }

            return result;
        }
    #endif


    /**
     *  @brief Set the string to be a substring of another string.
     *
     *  @param string The string, from which the substring is be taken.
     *  @param character_index The UTF-8 character index of the first character of the substring.
     *  @param character_length The UTF-8 character length of the substring.
     *  @return `true` if the method succeeded.
     *
     *  @note Parameters `character_index` and `character_length` will be internally clamped to a useful range.
     */
    bool String::set(const String& string, int64_t character_index, int64_t character_length) noexcept {

        if (&string != this) {
            if (string.subString(character_index, character_index + character_length - 1, *this) == character_length) {
                return true;
            }
        }

        return false;
    }


    /**
     *  @brief Set the content of the string to match another string.
     *
     *  @param string The source string.
     *  @return `true` if the method succeeded.
     */
    bool String::set(const String& string) noexcept {

        clear();
        return append(string);
    }


    /**
     *  @brief Set the content of the string to match another string.
     *
     *  @param string A pointer to the source string.
     *  @return `true` if the method succeeded.
     */
    bool String::set(const String* string) noexcept {

        if (string != nullptr) {
            clear();
            return true;
        }
        else {
            clear();
            return append(string);
        }
    }


    /**
     *  @brief Set the content of the string by a C-string.
     *
     *  @param str A pointer to the C-string.
     *  @param start Byte index to the first byte to use.
     *  @param end Byte index to the last byte to use.
     *  @return `true` if the method succeeded.
     */
    bool String::setByStr(const char* str, int64_t start, int64_t end) noexcept {

        if (start < 0 || end < start) {
            clear();
            return false;
        }
        else {
            return setByStr(&str[start], end - start + 1);
        }
    }


    /**
     *  @brief Set the content of the string by a C-string.
     *
     *  @param str A pointer to the C-string.
     *  @return `true` if the method succeeded.
     */
    bool String::setByStr(const char* str) noexcept {

        if (str != nullptr) {
            return setByStr(str, strlen(str));
        }
        else {
            return false;
        }
    }


    /**
     *  @brief Set the content of the string by a C-string.
     *
     *  @param str A pointer to the C-string.
     *  @param length Maximum number of bytes to be copied.
     *  @return `true` if the method succeeded.
     */
    bool String::setByStr(const char* str, int64_t length) noexcept {

        if (!str || length < 1) {
            clear();
            return true;
        }
        else {
            int64_t l = 0;
            const char* p = str;
            while (*p++ != String::EOS) {
                l++;
                if (l >= length) {
                    break;
                }
            }
            length = l;

            if (checkCapacity(length + 10)) {
                memcpy(m_data, str, length);
                m_data[length] = 0;
                m_byte_length = static_cast<int64_t>(strlen(m_data));
                m_character_length = utf8Length(m_data);
                return true;
            }
            else {
                return false;
            }
        }
    }


    /**
     *  @brief Extracts the content enclosed by specified opening and closing
     *         characters in a string.
     *
     *  This function searches for the first occurrence of the `open_c` character
     *  and the last occurrence of the `close_c` character in the provided string
     *  `str`. It extracts the substring enclosed between these characters and sets
     *  the string object with the content.
     *
     *  @param str The C-string containing the framed content.
     *  @param open_c The opening character to search for (e.g., '(').
     *  @param close_c The closing character to search for (e.g., ')').
     *
     *  @return The byte length of the extracted substring on success, otherwise
     *          a negative error code:
     *          - `-1`: Unsupported surrounding characters (either `open_c` or
     *                 `close_c` is null).
     *          - `-2`: Missing `open_c` in the string.
     *          - `-3`: Missing `close_c` in the string.
     *          - `-4`: `open_c` occurs after `close_c` (mismatched framing).
     */
    int64_t String::setByFramedContent(const char* str, char open_c, char close_c) noexcept {

        if (open_c == String::EOS || close_c == String::EOS) {
            return -1;  // Unsupported surrounding character(s)
        }

        const char* start_ptr = strchr(str, open_c);
        if (!start_ptr) {
            return -2;  // Missing `open_c`
        }

        const char* end_ptr = strrchr(str, close_c);
        if (!end_ptr) {
            return -3;  // Missing `close_c`
        }

        if (end_ptr < start_ptr) {
            return -4;  // Mismatched framing
        }

        setByStr(str, start_ptr - str + 1, end_ptr - str - 1);

        return byteLength();
    }


    /**
     *  @brief Set the content of the string by data.
     *
     *  @param data A pointer to the data containing the byte data.
     *  @param length Number of bytes to be copied.
     *  @return `true` if the method succeeded.
     */
    bool String::setByData(const Data* data, int32_t length) noexcept {
        /* TODO: !!!!!
        if (data != nullptr) {
            return setByStr((char*)data->data(), length);
        }
        else {
            return false;
        }
         */
        return false;
    }


    /**
     *  @brief Set a string with a text decribing a duration.
     *
     *  @param t Duration in timestamp ticks.
     *
     *  @note The string will automatically be formatted with information,
     *        that results in a human readable text.
     */
    void String::setElapsedTimeText(timestamp_t t) noexcept {

        int64_t milliseconds = t % 1000; t /= 1000;
        int64_t seconds = t % 60; t /= 60;
        int64_t minutes = t % 60; t /= 60;
        int64_t hours = t % 24; t /= 24;
        int64_t days = t;

        clear();

        if (days > 0) {
            appendFormatted(100, "%lld d ", days);
        }
        if (hours > 0) {
            appendFormatted(100, "%lld h ", hours);
        }
        if (minutes > 0) {
            appendFormatted(100, "%lld m ", minutes);
        }
        if (seconds > 0) {
            appendFormatted(100, "%lld s ", seconds);
        }
        if (milliseconds > 0 || (days == 0 && hours == 0 && minutes == 0 && seconds == 0)) {
            appendFormatted(100, "%lld ms", milliseconds);
        }

        trim();
    }


    /**
     *  @brief Sets the content using a format string, similar to the C `printf` function.
     *
     *  @param max_byte_length The maximum byte length of the resulting string.
     *  @return `ErrorCode::None` on success, or an appropriate `ErrorCode` on failure.
     *
     *  @note If `max_byte_length` is less than 2056, the method uses stack memory internally;
     *        otherwise, dynamic memory allocation is used.
     */
    ErrorCode String::setFormatted(int64_t max_byte_length, const char* format, ...) noexcept {
        clear();

        va_list args;
        va_start(args, format);
        ErrorCode result = appendFormatted(max_byte_length, format, args);
        va_end(args);

        return result;
    }


    /**
     *  @brief Appends to the content using a format similar to the C-function printf.
     *
     *  @param max_byte_length The maximum byte length of the resulting string.
     *  @return `true` if the method succeeded.
     *
     *  @note If `max_byte_length` is less than 2056, the method uses stack memory internally;
     *        otherwise, dynamic memory allocation is used.
     */
    ErrorCode String::appendFormatted(int64_t max_byte_length, const char* format, ...) noexcept {
        auto result = ErrorCode::None;
        constexpr int64_t kMaxStackBufferSize = 2056;
        bool allocate_flag = max_byte_length > kMaxStackBufferSize;

        if (max_byte_length < 32) {
            max_byte_length = 32;
        }

        char buffer[kMaxStackBufferSize];
        char* buffer_ptr = buffer;
        if (allocate_flag) {
            buffer_ptr = new (std::nothrow) char[max_byte_length];
            if (!buffer_ptr) {
                return ErrorCode::MemCantAllocate;
            }
        }

        // Initialize the variadic argument list
        va_list args;
        va_start(args, format);

        // Format the string
        int64_t byte_length = std::vsnprintf(buffer_ptr, max_byte_length, format, args);

        // Cleanup the argument list
        va_end(args);

        if (byte_length >= 0) {
            if (!append(buffer_ptr)) {
                result = ErrorCode::MemCantAllocate;
            }
        }

        if (allocate_flag) {
            delete[] buffer_ptr;
        }

        return result;
    }


    /**
     *  @brief Appends '1' if true, '0' if false.
     *
     *  @param v Boolean value to append.
     *  @return `true on success.
     */
    bool String::appendBool(bool v) noexcept {
        return v ? appendChar('1') : appendChar('0');
    }


    /**
     *  @brief Appends "true" or "false".
     *
     *  @param v Boolean value to append.
     *  @return `true` on success.
     */
    bool String::appendBoolTrueFalse(bool v) noexcept {

        return v ? append("true") : append("false");
    }


    /**
     *  @brief Appends "yes" or "no".
     *
     *  @param v Boolean value to append.
     *  @return `true` on success.
     */
    bool String::appendBoolYesNo(bool v) noexcept {

        return v ? append("yes") : append("no");
    }


    /**
     *  @brief Append a character at the end of the string.
     *
     *  @param c The character code to append.
     *  @return `true` if the method succeeded.
     *
     *  @note Only 7-bit ASCII characters can be appended.
     */
    bool String::appendChar(char c) noexcept {

        if (c <= 0) {
            // Not a 7 bit Ascii code
            return false;
        }
        else if (_checkExtraCapacity(1)) {
            m_data[m_byte_length] = c;
            m_data[m_byte_length + 1] = 0;
            m_byte_length++;
            m_character_length++;
            return true;
        }
        else {
            return false;
        }
    }


    /**
     *  @brief Append a character repeatedly at the end of the string.
     *
     *  @param c The character code to append.
     *  @param n Number of repetitions.
     *  @return `true` if the method succeeded, `false` otherwise.
     *
     *  @note Only 7-bit ASCII characters can be appended.
     */
    bool String::appendChars(char c, int64_t n) noexcept {

        if (c <= 0) {
            // Not a 7 bit Ascii code
            return false;
        }
        else if (n < 1) {
            // Nothing to append
            return false;
        }
        else if (_checkExtraCapacity(n)) {
            for (int64_t i = 0; i < n; i++) {
                m_data[m_byte_length + i] = c;
            }

            m_data[m_byte_length + n] = 0;
            m_byte_length += n;
            m_character_length += n;

            return true;
        }
        else {
            return false;
        }
    }


    /**
     *  @brief Append a C-string at the end of the string.
     *
     *  @param str The C-string to append.
     *  @return `true` if the method succeeded, `false` otherwise.
     */
    bool String::append(const char* str) noexcept {

        if (str != nullptr) {
            auto byte_length = static_cast<int64_t>(strlen(str));
            auto character_length = utf8Length(str);

            if (byte_length > 0 && character_length > 0) {
                if (_checkExtraCapacity(byte_length + 1)) {
                    memcpy(&m_data[m_byte_length], str, byte_length);
                    m_data[m_byte_length + byte_length] = String::EOS;
                    m_byte_length += byte_length;
                    m_character_length += character_length;
                    return true;
                }
            }
        }

        return false;
    }


    /**
     *  @brief Append a C-string at the end of the string.
     *
     *  @param str The C-string to append.
     *  @param max_byte_length Maximum number of bytes to append.
     *
     *  @return `true` if the method succeeded, `false` otherwise.
     */
    bool String::append(const char* str, int64_t max_byte_length) noexcept {

        if (str != nullptr) {
            int64_t byte_length = strlen(str);
            if (byte_length > max_byte_length) {
                byte_length = max_byte_length;
            }

            if (byte_length > 0) {
                if (_checkExtraCapacity(byte_length + 1)) {
                    memcpy(&m_data[m_byte_length], str, byte_length);
                    m_data[m_byte_length + byte_length] = String::EOS;
                    m_byte_length += byte_length;
                    m_character_length = utf8Length(m_data);
                    return true;
                }
            }
        }

        return false;
    }


    /**
     *  @brief Append a string at the end of the string.
     *
     *  @param string The string to append.
     *  @return `true` if the method succeeded, `false` otherwise.
     */
    bool String::append(const String& string) noexcept {

        return append(string.utf8());
    }


    /**
     *  @brief Append a string at the end of the string.
     *
     *  @param string The string to append.
     *  @return `true` if the method succeeded, `false` otherwise.
     */
    bool String::append(const String* string) noexcept {

        if (string != nullptr) {
            return append(string->utf8());
        }
        else {
            return false;
        }
    }


    /**
     *  @brief Append a substring from another string.
     *
     *  This function appends a substring from the specified string to the current
     *  string.
     *
     *  @param string The string from which a substring will be taken.
     *  @param character_start The index of the first character of the substring to
     *                         append.
     *  @param character_end The index of the last character of the substring to
     *                       append.
     *
     *  @return `true` if at least one character could be successfully appended,
     *          `false` otherwise.
     *
     *  @note The substring to be appended includes the character at the
     *        `character_start` index and continues up to the character at the
     *        `character_end` index.
     *
     *  @note If `character_start` or `character_end` is out of bounds for the
     *        source string, the function behavior is undefined. Ensure that the
     *        indices are valid before calling this function.
     */
    bool String::append(const String& string, int64_t character_start, int64_t character_end) noexcept {

        int64_t s = string.byteIndexFromCharacterIndex(character_start);
        int64_t e = string.byteIndexFromCharacterIndex(character_end);

        if (s < 0 || e < 0) {
            return false;
        }
        else {
            return append(&string.m_data[s], e - s + 1);
        }
    }


    /**
     *  @brief Append a single UTF-8 character from another string.
     *
     *  @param string The string to get the character from.
     *  @param character_index The character index.
     *  @return The length of the UTF-8 sequence or 0, if nothing could be appended.
     */
    int32_t String::appendCharacter(const String& string, int64_t character_index) noexcept {

        int32_t seq_length = 0;

        int64_t byte_index = string.byteIndexFromCharacterIndex(character_index);
        if (byte_index >= 0) {
            seq_length = string.utf8SeqLengthAtByteIndex(byte_index);

            if (seq_length > 0) {
                append(&string.m_data[character_index], seq_length);
            }
        }

        return seq_length;
    }


    /**
     *  @brief Append a value.
     *
     *  @param value The 32 bit integer value to append.
     *  @return `true` if the value was successfully appended, `false` otherwise.
     */
    bool String::appendInt32(int32_t value) noexcept {
        return appendFormatted(12, "%d", value) == ErrorCode::None;
    }


    /**
     *  @brief Append a value.
     *
     *  @param value The 32 bit unsigned integer value to append.
     *  @return `true` if the value was successfully appended, `false` otherwise.
     */
    bool String::appendUInt32(uint32_t value) noexcept {
        return appendFormatted(12, "%d", value) == ErrorCode::None;
    }


    /**
     *  @brief Append a value.
     *
     *  @param value The 64 bit integer value to append.
     *  @return `true` if the value was successfully appended, `false` otherwise.
     */
    bool String::appendInt64(int64_t value) noexcept {
        return appendFormatted(22, "%lld", value) == ErrorCode::None;
    }


    /**
     *  @brief Append a value.
     *
     *  @param value The 64 bit unsigned integer value to append.
     *  @return `true` if the value was successfully appended, `false` otherwise.
     */
    bool String::appendUInt64(uint64_t value) noexcept {
        return appendFormatted(22, "%llu", value) == ErrorCode::None;
    }


    /**
     *  @brief Append a double value with specified precision.
     *
     *  @param value The double value to append.
     *  @param precision Number of decimal places to include (0 to 9). Values
     *                   outside this range will be clamped.
     *  @return `true` if the value was successfully appended, `false` otherwise.
     */
    bool String::appendDouble(double value, int32_t precision) noexcept {

        static const char* format[] = {
                "%.0f", "%.1f", "%.2f", "%.3f", "%.4f", "%.5f", "%.6f", "%.7f", "%.8f", "%.9f",
        };

        if (precision < 0) {
            precision = 0;
        }
        else if (precision > 9) {
            precision = 9;
        }

        return appendFormatted(128, format[precision], value) == ErrorCode::None;
    }


    /**
     *  @brief Append a Fix value with specified precision.
     *
     *  @param value The Fix value to append.
     *  @param precision Number of decimal places to include (0 to 9). Values
     *                   outside this range will be clamped.
     *  @return `true` if the value was successfully appended, `false` otherwise.
     */
    bool String::appendFix(const Fix& value, int32_t precision) noexcept {

        char buffer[Fix::kMaxStrLength];
        value.toStr(buffer, Fix::kMaxStrLength, precision);
        return append(buffer);
    }


    /**
     *  @brief Insert a C-string.
     *
     *  This function inserts a UTF-8 encoded C-string into the current string at
     *  the given Unicode character index.
     *
     *  @param str Pointer to the UTF-8 encoded C-string to be inserted.
     *  @param character_index Position of the Unicode character where `str`
     *                         should be inserted. The index is zero-based.
     *  @return `true` if the insertion was successful, `false` otherwise.
     */
    bool String::insertAtCharacterIndex(const char* str, int64_t character_index) noexcept {

        if (!String::isValidUtf8() || !str) {
            return false;
        }

        auto byte_length = static_cast<int64_t>(strlen(str));
        if (byte_length < 1) {
            return false;
        }

        if (character_index == m_character_length) {
            append(str);
            return true;
        }

        if (!isCharacterIndexInRange(character_index)) {
            return false;
        }

        int64_t character_length = String::utf8Length(str);
        if (character_length < 1) {
            return false;
        }

        if (!_checkExtraCapacity(byte_length)) {
            return false;
        }

        int64_t byte_index = byteIndexFromCharacterIndex(character_index);
        if (!isByteIndexInRange(byte_index)) {
            return false;
        }

        int64_t n = m_byte_length - byte_index + 1;
        if (n < 1) {
            return false;
        }

        char* d = &m_data[m_byte_length + byte_length];
        const char* s = &m_data[m_byte_length];
        while (n--) {
            *d-- = *s--;
        }

        d = &m_data[byte_index];
        s = str;
        n = byte_length;
        while (n--) {
            *d++ = *s++;
        }

        m_character_length += character_length;
        m_byte_length += byte_length;
        m_data[m_byte_length] = 0;

        return true;
    }


    /**
     *  @brief Insert a string.
     *
     *  This function inserts a string into the current string at the given Unicode
     *  character index.
     *
     *  @param string The string to be inserted.
     *  @param character_index Position of the Unicode character where `string`
     *                         should be inserted. The index is zero-based.
     *  @return `true` if the insertion was successful, `false` otherwise.
     */
    bool String::insert(const String& string, int64_t character_index) noexcept {

        if (&string != this) {
            return insertAtCharacterIndex(string.utf8(), character_index);
        }
        else {
            return false;
        }
    }


    /**
     *  @brief Remove characters from the string.
     *
     *  Removes a substring from the string starting at the specified `character_index`
     *  and extending for the specified `character_length`.
     *
     *  @param character_index Starting character index of the substring to remove.
     *  @param character_length Number of characters to remove. If
     *                          `character_length` is greater than the number of
     *                          characters from `character_index` to the end of the
     *                          string, all characters from `character_index` to the
     *                          end are removed.
     *  @return `true` if characters were successfully removed, `false` otherwise
     *          (e.g., if `index` is out of range).
     */
    bool String::remove(int64_t character_index, int64_t character_length) noexcept {

        if (clampCharacterRange(character_index, character_length)) {
            int64_t byte_index, byte_length;

            if (byteRangeFromCharacterRange(character_index, character_length, byte_index, byte_length)) {
                _removeData(byte_index, byte_length, character_length);
                return true;
            }
        }

        return false;
    }


    /**
     *  @brief Truncate the string at the specified character index.
     *
     *  This method truncates the string, removing all characters starting from
     *  the specified `character_index` to the end of the string.
     *
     *  @param character_index The character index at which to start truncation.
     *                         If `character_index` is greater than the length of
     *                         the string, no truncation occurs.
     *  @return `true` if the string was successfully truncated, `false` otherwise.
     *
     *  @note The string is modified in place. The indices are zero-based.
     */
    bool String::truncate(int64_t character_index) noexcept {

        return remove(character_index, length() - character_index);
    }


    /**
     *  @brief Truncate the string by removing the first `length` characters.
     *
     *  This method truncates the string, removing the first `length` characters.
     *
     *  @param length The number of characters to remove from the start of the
     *                string. If `length` is greater than or equal to the length of
     *                the string, the string becomes empty.
     *  @return `true` if the string was successfully truncated, `false` otherwise.
     *
     *  @note The string is modified in place. The indices are zero-based.
     */
    bool String::truncateStart(int64_t length) noexcept {

        return remove(0, length);
    }


    /**
     *  @brief Replace a single character in the string.
     *
     *  @param character_index The character index where the replacement should take
     *                         place.
     *  @param c A pointer to a UTF-8 encoded character.
     *
     *  @return `true` if the character was successfully replaced, `false` otherwise.
     */
    bool String::replaceChar(int64_t character_index, const char* c) noexcept {

        if (isCharacterIndexInRange(character_index) && c != nullptr) {
            if (String::utf8Length(c) != 1) {
                return false;
            }

            int64_t byte_index = byteIndexFromCharacterIndex(character_index);
            if (!isByteIndexInRange(byte_index)) {
                return false;
            }

            int32_t seq_length = utf8SeqLengthAtByteIndex(byte_index);
            if (seq_length < 1) {
                return false;
            }

            auto new_seq_length = static_cast<int64_t>(strlen(c));
            if (new_seq_length < 1) {
                return false;
            }

            if (new_seq_length < seq_length) {
                _removeData(byte_index, seq_length - new_seq_length, 0);
            }

            for (int64_t i = 0; i < new_seq_length; i++) {
                m_data[byte_index + i] = c[i];
            }
        }

        return false;
    }


    /**
     *  @brief Replace a single character in the string.
     *
     *  @param character_index The character index in the target string where the
     *                         replacement should take place.
     *  @param src_string The source string from which the character is taken.
     *  @param src_character_index The character index in `src_string` where the
     *                             character should be read from.
     *  @return `true` if the character was successfully replaced, `false` otherwise.
     */
    bool String::replaceChar(int64_t character_index, const String& src_string, int64_t src_character_index) noexcept {

        if (src_string.isCharacterIndexInRange(src_character_index)) {
            auto src_byte_index = src_string.byteIndexFromCharacterIndex(src_character_index);
            int64_t seq_length = src_string.utf8SeqLengthAtByteIndex(src_byte_index);

            if (seq_length > 0) {
                char buffer[kUtf8SeqBufferSize];
                strncpy(buffer, src_string.utf8AtIndex(src_character_index), seq_length);
                buffer[seq_length] = 0;

                return replaceChar(character_index, buffer);
            }
        }

        return false;
    }


    /**
     *  @brief Replace occurrences of a string with another string.
     *
     *  @param search_string The string to be replaced.
     *  @param replacement_string The new string that replaces occurrences of
     *                            `search_string`.
     *  @return The number of successful replacements.
     */
    int64_t String::replace(const String& search_string, const String& replacement_string) noexcept {

        return replace(search_string.utf8(), replacement_string.utf8());
    }


    /**
     *  @brief Replace occurrences of a string with another string.
     *
     *  @param search_str The C-string to be replaced.
     *  @param replacement_str The new C-string that replaces occurrences of
     *                         `search_str`.
     *  @return The number of successful replacements.
     */
    int64_t String::replace(const char* search_str, const char* replacement_str) noexcept {

        int64_t n = 0;

        if (search_str != nullptr && replacement_str != nullptr) {
            int64_t search_str_length = utf8Length(search_str);
            int64_t replacement_str_length = utf8Length(replacement_str);

            if (search_str_length > 0) {
                int64_t index = 0;
                int64_t new_index;

                while ((new_index = find(search_str, index)) >= 0) {
                    remove(new_index, search_str_length);

                    if (replacement_str_length > 0) {
                        insertAtCharacterIndex(replacement_str, new_index);
                        index = new_index + replacement_str_length;
                    }
                    else
                        index = new_index;

                    n++;
                }
            }
        }

        return n;
    }


    /**
     *  @brief Convert a string representing a floating point number from scientific
     *         notation to standard decimal notation.
     *
     *  This function takes a string representation of a floating point number that
     *  may be in scientific notation (e.g., "1.23e+10") and converts it to a
     *  standard decimal notation (e.g., "12300000000").
     */
    ErrorCode String::removeScientificNotation() noexcept {

        if (!isValidNumber()) {
            return ErrorCode::InvalidNumber;
        }
        else {
            const char* str = utf8();
            int64_t exp_index = -1;
            while (*str != String::EOS) {
                if (*str == 'e' || *str == 'E') {
                    exp_index = str - utf8();
                    break;
                }
            }

            if (exp_index < 0) {
                return ErrorCode::None;
            }

            String exp_string;
            subString(exp_index + 1, exp_string);

            int32_t exp = exp_string.asInt32();
            int64_t dot_index = find(".");

            char sign = 0;
            int64_t start = 0;
            if (isSignChar(m_data[0])) {
                sign = m_data[0];
                start = 1;
            }

            String integer_part;
            String float_part;
            if (dot_index < 0) {
                subString(start, exp_index - 1, integer_part);
            }
            else {
                subString(start, dot_index - 1, integer_part);
                subString(dot_index + 1, exp_index - 1, float_part);
            }


            // Remove leading zeros from integer part
            int64_t index = 0;
            while (integer_part.m_data[index] == '0') {
                index++;
            }
            integer_part.remove(0, index);

            // Remove trailing zeros from floating point part
            index = float_part.byteLength() - 1;
            while (index >= 0 && float_part.m_data[index] == '0') {
                index--;
            }
            float_part.truncate(index + 1);


            int64_t integer_part_length = integer_part.length();
            int64_t float_part_length = float_part.length();

            String new_string;
            new_string.appendChar(sign);

            if (exp == 0) {
                new_string.append(integer_part);
                new_string.appendChar('.');
                new_string.append(float_part);
            }
            else if (exp < 1) {
                int32_t shift = -exp;
                if (shift == integer_part_length) {
                    new_string.append("0.");
                    new_string.append(integer_part);
                    new_string.append(float_part);
                }
                else {
                    if (shift > integer_part_length) {
                        new_string.append("0.");
                        new_string.appendChars('0', shift - integer_part_length);
                        new_string.append(integer_part);
                    }
                    else {
                        new_string.append(integer_part, 0, integer_part_length - shift - 1);
                        new_string.appendChar('.');
                        new_string.append(integer_part, integer_part_length - shift, integer_part_length - 1);
                    }
                    new_string.append(float_part);
                }
            }
            else {
                int32_t shift = exp;
                int64_t zeroes = 0;
                new_string.append(integer_part);
                if (float_part_length <= shift) {
                    new_string.append(float_part);
                    zeroes = shift - float_part_length;
                }
                else {
                    new_string.append(float_part, 0, shift - 1);
                    new_string.appendChar('.');
                    new_string.append(float_part, shift, float_part_length - 1);
                    zeroes = shift - float_part_length;
                }

                if (zeroes > 0) {
                    new_string.appendChars('0', zeroes);
                }
            }

            if (set(new_string)) {
                return ErrorCode::None;
            }
            else {
                return ErrorCode::MemCantGrow;
            }
        }
    }


    /**
     *  @brief Removes surrounding double quotes from the string if present.
     */
    void String::removeStringDoubleQuotes() noexcept {

        if (isQuoted()) {
            remove(m_byte_length - 1, 1);
            remove(0, 1);
        }
    }


    /**
     *  @brief Generates a random name of the given length.
     *
     *  @param length Desired length of the random name.
     *  @return ErrorCode indicating success or failure.
     */
    ErrorCode String::randomName(int64_t length) noexcept {

        if (length < 1) {
            return ErrorCode::BadArgs;
        }
        else if (!checkCapacity(length)) {
            return ErrorCode::MemCantGrow;
        }
        else {
            randomName(length, m_data);
            return ErrorCode::None;
        }
    }


    /**
     *  @brief Generates a random name based on a mask and optional path.
     *  @param mask Pattern to guide random name generation.
     *  @param path Optional path context.
     *  @return ErrorCode indicating success or failure.
     */
    ErrorCode String::randomName(const char* mask, const char* path) noexcept {

        int64_t length = randomNameLength(mask, path);

        if (length < 1) {
            return ErrorCode::BadArgs;
        }
        else if (!checkCapacity(length)) {
            return ErrorCode::MemCantGrow;
        }
        else {
            return randomName(mask, path, length + 1, m_data);
        }
    }


    /**
     *  @brief Generates a UUID.
     *
     *  This function generates a new UUID using `uuid_generate` and converts it
     *  to a human-readable string using `uuid_unparse`.
     *
     *  @return `true` on successful execution.
     */
    bool String::uuid() noexcept {

        if (checkCapacity(36)) {  // UUIDs are 36 characters
            uuid_t uuid;
            uuid_generate(uuid);
            uuid_unparse(uuid, m_data);
            return true;
        }
        else {
            return false;
        }
    }


    /**
     *  @brief Read data from a file.
     *
     *  @param file_path Location of the file to load the string from.
     *  @return `true` if data could be read; otherwise, `false`.
     */
    ErrorCode String::readFromFile(const String& file_path) noexcept {

        auto result = ErrorCode::None;

        /* TODO: !!!!!
        try {
            File file(file_path);
            file.startReadAscii();
            file.readToString(*this);
        }
        catch (ErrorCode err) {
            result = err;
        }
         */

        return result;
    }


    /**
     *  @brief Find an ASCII character from a given index.
     *
     *  @param c The ASCII character to find.
     *  @param index An index where to begin finding `c`.
     *  @return The position where the first occurrence `c` could be found or a
     *          negative error code.
     */
    int64_t String::findAsciiChar(char c, int64_t index) const noexcept {

        // TODO: Test!!!

        if (index < 0 || index >= m_character_length) {
            return kFindResult_CharacterIndexOutOfRange;
        }

        int64_t byte_index = byteIndexFromCharacterIndex(index);
        if (byte_index < 0 || byte_index >= m_byte_length) {
            return kFindResult_ByteIndexOutOfRange;
        }

        char* ptr = &m_data[byte_index];
        for (int64_t i = index; i < m_byte_length; i++) {
            if (*ptr == String::EOS) {
                break;
            }

            if (*ptr == c) {
                int64_t character_index = characterIndexFromByteIndex(static_cast<int64_t>(ptr - m_data));
                if (character_index < 0) {
                    return kFindResult_ConversionIndexFailed;
                }
                else {
                    return character_index;
                }
            }

            ptr++;
        }

        return kFindResult_NothingFound;
    }


    /**
     *  @brief Find a string from a given index.
     *
     *  @param str The C-string to find.
     *  @param index An index where to begin finding str.
     *  @return The position where the first occurrence of str could be found or a
     *          negative error code:
     *          -1: A memory error occured.
     *          -2: str is nullptr.
     *          -3: index is out of range.
     *          -4: byte index is out of range.
     *          -5: nothing found.
     */
    int64_t String::find(const char* str, int64_t index) const noexcept {

        if (!m_data) {
            return kFindResult_MemError;
        }

        if (!str) {
            return kFindResult_StrError;
        }

        if (index < 0 || index >= m_character_length) {
            return kFindResult_CharacterIndexOutOfRange;
        }

        int64_t byte_index = byteIndexFromCharacterIndex(index);
        if (byte_index < 0 || byte_index >= m_byte_length) {
            return kFindResult_ByteIndexOutOfRange;
        }

        char* ptr = strstr(&m_data[byte_index], str);
        if (!ptr) {
            return kFindResult_NothingFound;
        }

        int64_t character_index = characterIndexFromByteIndex(static_cast<int64_t>(ptr - m_data));
        if (character_index < 0) {
            return kFindResult_ConversionIndexFailed;
        }

        return character_index;
    }


    /**
     *  @brief Finds the first occurrence of a substring within this string.
     *
     *  @param string The substring to search for.
     *  @param index The position to start searching from. Must be >= 0.
     *  @return int64_t The byte index of the first occurrence of `string`,
     *                  or -1 if not found or if inputs are invalid.
     *
     *  @details
     *  - If `string` is empty, returns `index` (if within bounds).
     *  - If `string` is not valid or `index` is out of bounds, returns -1.
     */
    int64_t String::find(const String& string, int64_t index) const noexcept {

        if (&string != this) {
            return find(string.utf8(), index);
        }
        else {
            return 0;
        }
    }


    /**
     *  @brief Finds the first occurrence of a substring within this string,
     *         ignoring case.
     *
     *  @param string The substring to search for (case-insensitive).
     *  @param index The position to start searching from. Must be >= 0.
     *  @return The byte index of the first occurrence of `string` (ignoring case),
     *          or -1 if not found or if inputs are invalid.
     *
     *  @details
     *  - If `string` is empty, returns `index` (if within bounds).
     *  - If `string` is not valid or `index` is out of bounds, returns -1.
     */
    int64_t String::findIgnoreCase(const String& string, int64_t index) const noexcept {

        if (!string.isValidUtf8()) {
            return -1;
        }

        if (&string == this) {
            return 0;
        }

        const char* haystack = utf8();
        const int64_t haystack_n = byteLength();
        const char* needle = string.utf8();
        const int64_t needle_n = string.byteLength();

        if (needle_n == 0) {
            return (index >= 0 && index <= haystack_n) ? index : -1;
        }

        if (index < 0 || index > haystack_n - needle_n) {
            return -1;
        }

        for (int64_t i = index; i <= haystack_n - needle_n; ++i) {
            if (strncasecmp(&haystack[i], needle, needle_n) == 0) {
                return i;
            }
        }

        return -1;
    }

    int64_t String::findOneCharOf(const char* char_set, int64_t index) const noexcept {

        String string(char_set);
        return findOneCharOf(string, index);
    }

    /**
     *  @brief Finds the first occurrence of any character from the given string.
     *
     *  This function searches for the first occurrence of any character in the
     *  provided `char_set` within the current string, starting from the specified
     *  `index`.
     *
     *  @param char_set A reference to a `String` containing the characters to
     *                  search for.
     *  @param index The position in the string from where the search begins.
     *  @return int64_t The index of the first matching character found, or
     *         `        kFindResult_NothingFound` if no match is found.
     *
     *  @note The search is performed using UTF-8 encoded characters.
     */
    int64_t String::findOneCharOf(const String& char_set, int64_t index) const noexcept {

        char buffer[kUtf8SeqBufferSize];

        for (int64_t i = 0; i < length(); i++) {
            char_set.utf8CodeAtByteIndex(i, buffer);
            int64_t result = find(buffer, index);
            if (result >= 0) {
                return result;
            }
        }

        return kFindResult_NothingFound;
    }


    /**
     *  @brief Count how often a C-string is contained.
     *
     *  @param str The C-string to look for.
     *  @param index An optional index where to start searching.
     *  @return Number of occurrences.
     */
    int64_t String::count(const char* str, int64_t index) const noexcept {

        int64_t n = 0;

        while (true) {
            int64_t new_index = find(str, index);
            if (new_index > 0) {
                index = new_index + 1;
                n++;
            }
            else {
                break;
            }
        }

        return n;
    }


    /**
     *  @brief Count how often a string is contained.
     *
     *  @param string The string to be found.
     *  @param index An optional index where to start searching.
     *  @return Number of occurrences.
     */
    int64_t String::count(const String& string, int64_t index) const noexcept {

        if (&string != this) {
            return count(string.utf8(), index);
        }
        else {
            return 0;
        }
    }


    /**
     *  @brief Compares a character at a specific index with the given character.
     *
     *  @param c Character to compare.
     *  @param index Index in the string to check.
     *  @return `true` if characters match; `false` otherwise.
     */
    bool String::compareAsciiAtIndex(char c, int64_t index) const noexcept {

        char c_in_string;
        if (isAsciiAtIndex(index, c_in_string)) {
            return c_in_string == c;
        }
        else {
            return false;
        }
    }


    /**
     *  @brief Compares this string to a C-string.
     *
     *  @param str Null-terminated C-string to compare.
     *  @return 0 if equal, negative if less, positive if greater; std::numeric_limits<int32_t>::min() on
     *          null input.
     */
    int32_t String::compare(const char* str) const noexcept {

        if (str != nullptr && m_data != nullptr) {
            return strcmp(m_data, str);
        }
        else {
            return std::numeric_limits<int32_t>::min();
        }
    }


    /**
     *  @brief Compares this string to another String object.
     *
     *  @param string Another String to compare.
     *  @return 0 if equal, negative if less, positive if greater; may delegate
     *          to C-string compare.
     */
    int32_t String::compare(const String& string) const noexcept {

        return compare(string.utf8());
    }


    /**
     *  @brief Compare two strings with variable offsets and a limited length of
     *         comparison.
     *
     *  This function compares a portion of the current string with a portion of
     *  another string, starting from specified offsets and up to a specified
     *  length. The comparison is performed lexicographically.
     *
     *  @param string The string to compare with.
     *  @param offs The starting offset in the current string.
     *  @param offs_other The starting offset in the provided string.
     *  @param length The maximum number of characters to compare.
     *
     *  @return
     *    - A negative value if the portion of the current string is
     *      lexicographically less than the portion of the provided string.
     *    - Zero if the two portions are equal.
     *    - A positive value if the portion of the current string is
     *      lexicographically greater than the portion of the provided string.
     *
     *  @note
     *    - Non-ASCII characters may result in undefined behavior due to encoding
     *      complexities.
     *    - If `offs` or `offs_other` exceed the bounds of their respective strings,
     *      the behavior is undefined.
     *    - If `length` is greater than the remaining characters starting from the
     *      offsets, the comparison is truncated to the shorter length.
     *    - The comparison is case-sensitive.
     */
    int32_t String::compareAscii(const String& string, uint32_t offs, uint32_t offs_other, int32_t length) const noexcept {

        if (m_data != nullptr &&
            string.m_data != nullptr &&
            offs <= byteLength() &&
            offs_other <= string.byteLength()) {

            const char* ptr = &m_data[offs];
            const char* ptr_other = &string.m_data[offs_other];

            return strcmp(ptr, ptr_other);
        }
        else {
            return std::numeric_limits<int32_t>::min();
        }
    }


    /**
     *  @brief Compares this string to a C-string, ignoring case.
     *  @param str Null-terminated C-string to compare.
     *  @return 0 if equal (case-insensitive), negative if less, positive if
     *          greater; -1 on null input.
     */
    int32_t String::compareIgnoreCase(const char* str) const noexcept {

        if (str != nullptr && m_data != nullptr) {
            return strcasecmp(m_data, str);
        }
        else {
            return -1;
        }
    }


    /**
     *  @brief Compares this string to another String object, ignoring case.
     *  @param string Another String to compare.
     *  @return 0 if equal (case-insensitive), negative if less, positive if greater.
     */
    int32_t String::compareIgnoreCase(const String& string) const noexcept {

        return compareIgnoreCase(string.utf8());
    }


    /**
     *  @brief Split a string into values.
     *
     *  This function splits a string into values using the specified delimiter and
     *  quote characters.
     *
     *  @param delimiter A delimiter character that must be present between the
     *                     values.
     *  @param quote A quote character, which parenthesizes the value. If it 0, it
     *               will be ignored.
     *  @param trim_mode How values should be trimmed before adding to outList.
     *  @param out_list [out] The array that holds the extracted values.
     *
     *  @return Number of parts that could be extracted.
     */
    int64_t String::csvSplit(char delimiter, char quote, String::TrimMode trim_mode, StringList& out_list) const noexcept {

        int64_t result_n = 0;

        if (delimiter == 0) {
            return -1;
        }

        /* TODO: !!!!!
        CSVLineParser csv_line_parser(utf8());
        csv_line_parser.setDelimiter(delimiter);
        csv_line_parser.setQuote(quote);

        while (csv_line_parser.next()) {
            String string = csv_line_parser.currFieldStrPtr();
            string.trim(trim_mode);
            out_list.pushString(string);
            result_n++;
        }
         */

        return result_n;
    }


    /**
     *  @brief Extracts a substring starting at a given character index.
     *
     *  @param start Start index (character-based).
     *  @param out_string Output string to store the substring.
     *  @return Length of the substring, or -1 on failure.
     */
    int64_t String::subString(int64_t start, String& out_string) const noexcept {

        return subString(start, m_character_length - 1, out_string);
    }


    /**
     *  @brief Extracts a substring between two character indices.
     *
     *  @param start Start index (character-based).
     *  @param end End index (character-based).
     *  @param out_string Output string to store the substring.
     *  @return Length of the substring, or -1 on failure.
     */
    int64_t String::subString(int64_t start, int64_t end, String& out_string) const noexcept {

        if (start < 0 || start >= m_character_length || end < start) {
            return -1;
        }

        if (end >= m_character_length) {
            end = m_character_length - 1;
        }

        if (out_string.setByStr(utf8(), byteIndexFromCharacterIndex(start), byteIndexFromCharacterIndex(end))) {
            return out_string.length();
        }

        return -1;
    }


    /**
     *  @brief Extracts and trims a substring between two character indices.
     *
     *  @param start Start index.
     *  @param end End index.
     *  @param out_string Output string to store the trimmed substring.
     *  @return Length of the trimmed substring, or -1 on failure.
     */
    int64_t String::trimmedSubString(int64_t start, int64_t end, String& out_string) const noexcept {

        int64_t result = subString(start, end, out_string);

        if (result > 1) {
            out_string.trim();
        }

        return result;
    }


    /**
     *  @brief Extracts and trims a substring starting at the given index.
     *
     *  @param start Start index.
     *  @param out_string Output string to store the trimmed substring.
     *  @return Length of the trimmed substring, or -1 on failure.
     */
    int64_t String::trimmedSubString(int64_t start, String& out_string) const noexcept {

        int64_t result = subString(start, out_string);

        if (result > 1) {
            out_string.trim();
        }

        return result;
    }


    /**
     *  @brief Gets the ASCII character at a specific character index.
     *
     *  @param index Character index to look up.
     *  @param out_char Output character if valid and ASCII.
     *  @return `true` if successful and the character is ASCII; `false` otherwise.
     */
    bool String::isAsciiAtIndex(int64_t index, char& out_char) const noexcept {

        if (isCharacterIndexInRange(index)) {
            int64_t symbol_index = byteIndexFromCharacterIndex(index);

            if (isByteIndexInRange(symbol_index)) {
                if (utf8SeqLengthAtByteIndex(symbol_index) == 1) {
                    out_char = m_data[symbol_index];
                    return true;
                }
            }
        }

        return false;
    }


    /**
     *  @brief Returns the ASCII character at a specific index.
     *
     *  @param index Character index to look up.
     *  @return ASCII character at index, or 0 if invalid or non-ASCII.
     */
    char String::asciiAtIndex(int64_t index) const noexcept {

        char c;
        if (isAsciiAtIndex(index, c)) {
            return c;
        }
        else {
            return 0;
        }
    }


    /**
     * @brief Returns the first ASCII character in the string.
     *
     * @return First character, or 0 if the string is empty or null.
     */
    char String::firstAsciiChar() const noexcept {

        return m_data != nullptr && m_byte_length > 0 ? m_data[0] : 0;
    }


    /**
     *  @brief Get a pointer to UTF-8 encoded data starting at a specific character.
     *
     *  @param index The index of the character in the string from which the
     *               resulting pointer should start.
     *  @return A pointer to the UTF-8 encoded data, or a pointer to an empty
     *          C-string if charIndex is out of range.
     */
    const char* String::utf8AtIndex(int64_t index) const noexcept {

        if (m_data != nullptr) {
            int64_t symbol_index = byteIndexFromCharacterIndex(index);

            if (symbol_index < 0) {
                return String::g_empty_data;
            }

            return &m_data[symbol_index];
        }

        return String::g_empty_data;
    }


    /**
     *  @brief Extracts a UTF-8 substring into a provided buffer.
     *
     *  Copies a UTF-8 substring starting at a character index with the given
     *  character length, ensuring it fits within the specified byte capacity of
     *  the output buffer.
     *
     *  @param index Starting character index.
     *  @param length Number of characters to extract.
     *  @param max_byte_capacity Maximum size of the output buffer (including null
     *                           terminator).
     *  @param out_buffer Buffer to store the UTF-8 substring (null-terminated).
     *  @return `true` if successful; `false` if indices are invalid or capacity
     *          is insufficient.
     */
    bool String::utf8SubStr(int64_t index, int64_t length, int64_t max_byte_capacity, char* out_buffer) const noexcept {

        if (m_data != nullptr && out_buffer != nullptr) {
            int64_t symbol_index, symbol_length;

            if (byteRangeFromCharacterRange(index, length, symbol_index, symbol_length)) {
                if (symbol_length < max_byte_capacity) {
                    strncpy(out_buffer, &m_data[symbol_index], symbol_length);
                    out_buffer[symbol_length] = 0;

                    return true;
                }
            }
        }

        return false;
    }


    /**
     *  @brief Get a single UTF-8 encoded character from the string.
     *
     *  @param byte_index The byte index where the UTF-8 character begins.
     *  @param[out] out_buffer A pointer to a buffer where the UTF-8 byte sequence
     *              will be stored.
     *  @return The length of the UTF-8 sequence in bytes.
     */
    int32_t String::utf8CodeAtByteIndex(int64_t byte_index, char* out_buffer) const noexcept {

        int32_t seq_length = 0;

        if (m_data != nullptr && out_buffer != nullptr) {
            if (isByteIndexInRange(byte_index)) {
                seq_length = utf8SeqLengthAtByteIndex(byte_index);

                if (seq_length > 0) {
                    for (int32_t i = 0; i < seq_length; i++) {
                        out_buffer[i] = m_data[byte_index + i];
                    }
                    out_buffer[seq_length] = 0;
                }
            }
        }

        return seq_length;
    }


    /**
     *  @brief Check if UTF-8 data represents a soft line break character.
     *
     *  @param utf8_data Pointer so UTF-8 encoded data.
     *
     *  @return `true`, if data represents a soft line break character.
     */
    bool String::utf8IsSoftLineBreak(const char* utf8_data) noexcept {

        return (utf8_data != nullptr &&
                static_cast<uint8_t>(utf8_data[0]) == 0xe2 &&
                static_cast<uint8_t>(utf8_data[1]) == 0x80 &&
                static_cast<uint8_t>(utf8_data[2]) == 0xa8);
    }


    /**
     *  @brief Check if UTF-8 data represents a whitespace character.
     *
     *  @param utf8_data Pointer so UTF-8 encoded data.
     *
     *  @return true, if data represents a white space character.
     */
    bool String::utf8IsWhiteSpace(const char* utf8_data) noexcept {

        if (!utf8_data) {
            return false;
        }

        auto c = static_cast<uint8_t>(utf8_data[0]);
        int32_t seq_length = utf8SeqLengthByStartByte(c);

        if (seq_length == 1 && charIsWhiteSpace(c)) {
            return true;
        }

        if (c == 0xc2) {
            auto c2 = static_cast<uint8_t>(utf8_data[1]);
            if (c2 == 0xa0) {
                // No-Break, "\u00A0"
                return true;
            }
        }

        if (c == 0xe2) {
            auto c2 = static_cast<uint8_t>(utf8_data[1]);
            if (c2 == 0x80) {
                auto c3 = static_cast<uint8_t>(utf8_data[2]);
                if (c3 == 0x82 ||  // En Space, \u2002
                    c3 == 0x83 ||  // Em Space, \u2003
                    c3 == 0x84 ||  // Three-Per-Em Space, \u2004
                    c3 == 0x85 ||  // Four-Per-Em Space, \u2005
                    c3 == 0x86 ||  // Six-per-em space, \u2006
                    c3 == 0x87 ||  // Figure Space, \u2007
                    c3 == 0x88 ||  // Punctuation Space, \u2008
                    c3 == 0x89 ||  // Thin Space, \u2009
                    c3 == 0x8a ||  // Hair Space, \u200A
                    c3 == 0x8b ||  // Zero Width Space, \u200B
                    c3 == 0x8c ||  // Zero Width Non-Joiner, \u200C
                    c3 == 0x8d ||  // Zero Width Joiner, \u200D
                    c3 == 0xa8 ||  // Line Separator, \u2028  // TODO: Necessary?
                    c3 == 0xa9 )   // Paragraph Separator, \u2029
                {
                    return true;
                }
            }
            else if (c2 == 0xA0) {
                uint8_t c3 = static_cast<uint8_t>(utf8_data[2]);
                if (c3 == 0x80)  // Braille Pattern Blank, \u2800
                {
                    return true;
                }
            }
        }

        return false;
    }


    /**
     *  @brief Check if a 8 bit char represents a whitespace character.
     *
     *  Recognized as white spaces are:
     *  - Horizontal tab (0x09, '\t')
     *  - Line feed (0x0a, '\n')
     *  - Vertical tab (0x0b, '\v')
     *  - Form feed (0x0c, '\f')
     *  - Carriage return (0x0d, '\r')
     *  - Space (0x20, ' ')
     *
     *  @param c The char to test.
     *  @return `true`, if char represents a white space character.
     */
    bool String::charIsWhiteSpace(char c) noexcept {

        return (c == 0x09 || c == 0x0a || c == 0x0b || c == 0x0c || c == 0x0d || c == 0x20);
    }


    /**
     *  @brief Check if an 8-bit char represents a hexadecimal digit (0-9, a-f, A-F).
     *
     *  @param c The char to test.
     *  @return true if the char represents a hex digit; false otherwise.
     */
    bool String::charIsHexLetter(char c) noexcept {

        if ((c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F')) {
            return true;
        }
        else {
            return false;
        }
    }


    /**
     *  @brief Converts a single hexadecimal character to its corresponding decimal
     *         value.
     *
     *  This function takes a single character representing a hexadecimal digit
     *  (e.g., '0'-'9', 'a'-'f', 'A'-'F') and returns its integer equivalent.
     *  If the input character is not a valid hexadecimal digit, the function
     *  returns -1.
     *
     *  @param c The character to convert. It should be a valid hexadecimal character.
     *  @return The decimal value of the hexadecimal digit (0-15), or -1 if the
     *          input is invalid.
     *
     *  @note The function is case-insensitive and handles both uppercase and
     *        lowercase hexadecimal letters ('a'-'f' and 'A'-'F').
     */
int8_t String::valueForHexChar(char c) noexcept {

        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        else if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }
        else if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        else {
            return -1;
        }
    }


    /**
     *  @brief Checks whether a given C-string is a valid hexadecimal string.
     *
     *  This method verifies that the input string consists only of valid
     *  hexadecimal characters (0–9, a–f, A–F), optionally prefixed with "0x" or "0X".
     *
     *  @param str Pointer to a null-terminated C-string to validate.
     *  @return `true` if the string is a valid hex representation; `false` otherwise.
     *
     *  @note
     *  - If the input pointer is `nullptr` or points to an empty string, the
     *    function returns `false`.
     *  - If the string starts with "0x" or "0X", the prefix is correctly ignored.
     *  - Strings like "0x" without any digits after the prefix are considered
     *    invalid.
     *
     *  @example
     *  @code
     *  bool result1 = isValidHexString("0x1A3F"); // true
     *  bool result2 = isValidHexString("1A3F");   // true
     *  bool result3 = isValidHexString("xyz");    // false
     *  bool result4 = isValidHexString(nullptr);  // false
     *  bool result5 = isValidHexString("0x");     // false
     *  @endcode
     */
    bool String::isValidHexString(const char* str) noexcept {

        if (!str || *str == String::EOS) {
            return false;
        }

        // Optional: Skip "0x" or "0X" prefix
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            str += 2;
        }

        if (*str == String::EOS) {
            return false;  // only "0x" without digits
        }

        while (*str != String::EOS) {
            char c = *str;
            if (!( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') )) {
                return false;
            }
            str++;
        }

        return true;
    }


    /**
     *  @brief Returns a pointer to the first non-whitespace character in a C-string.
     *
     *  Skips leading whitespace characters and returns a pointer to the first
     *  non-whitespace character.
     *
     *  @param str Null-terminated C-string to scan.
     *  @return Pointer to the first non-whitespace character, or nullptr if input
     *          is null.
     */
    const char* String::firstNonWhiteSpaceCharPtr(const char* str) noexcept {

        if (!str) {
            return nullptr;
        }
        else {
            while (String::charIsWhiteSpace(*str)) {
                str++;
            }
            return str;
        }
    }


    /**
     *  @brief Function to check if a character is Base64 valid.
     */
    bool String::isBase64(char c) noexcept {

        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               (c == '+') || (c == '/');
    }


    /**
     *  @brief Get the length of a UTF-8 encoded character.
     *
     *  @param c Pointer to the UTF-8 encoded character.
     *  @return Length of the UTF-8 sequence in bytes, or a value less than 1 if
     *          not valid.
     *
     *  @note Only one character is recognized.
     */
    int32_t String::utf8SeqLength(const uint8_t* c) noexcept {

        if (!c) {
            return -2;
        }

        auto s = c;
        if (*s <= 0x7F) {
            // 1-byte character (ASCII, valid)
            return 1;
        }
        else if ((*s >= 0xC2 && *s <= 0xDF) && (s[1] >= 0x80 && s[1] <= 0xBF)) {
            // 2-byte character: starts with 110xxxxx and second byte is 10xxxxxx
            return 2;
        }
        else if ((*s >= 0xE0 && *s <= 0xEF) && (s[1] >= 0x80 && s[1] <= 0xBF) && (s[2] >= 0x80 && s[2] <= 0xBF)) {
            // 3-byte character: starts with 1110xxxx and subsequent bytes are 10xxxxxx
            return 3;
        }
        else if ((*s >= 0xF0 && *s <= 0xF4) && (s[1] >= 0x80 && s[1] <= 0xBF) && (s[2] >= 0x80 && s[2] <= 0xBF) && (s[3] >= 0x80 && s[3] <= 0xBF)) {
            // 4-byte character: starts with 11110xxx and subsequent bytes are 10xxxxxx
            return 4;
        }
        else {
            // Invalid byte sequence for UTF-8
            return -1;
        }
    }


    /**
     *  @brief Validates whether the provided byte sequence is a well-formed UTF-8
     *         encoded string.
     *
     *  This function checks if the given `str` contains a valid UTF-8 sequence by
     *  examining each byte in the string and verifying if it follows the rules of
     *  UTF-8 encoding (such as the correct number of continuation bytes for
     *  multi-byte characters). The function returns `false` if an invalid UTF-8
     *  sequence is encountered or if the string is `nullptr`. Otherwise, it returns
     *  `true` indicating that the string is valid UTF-8.
     *
     *  @param str A pointer to the input byte sequence (UTF-8 encoded string).
     *  @return `true` if the string is a valid UTF-8 sequence, `false` otherwise.
     */
    bool String::isValidUtf8(const uint8_t* str) noexcept {

        if (!str) {
            return false;
        }

        auto s = str;
        while (*s != String::EOS) {
            int32_t utf8_seq_length = utf8SeqLength(s);
            if (utf8_seq_length < 1) {
                return false;
            }
            s += utf8_seq_length;
        }

        return true;
    }


    /**
     *  @brief Copies the string's bytes into a buffer and zero-fills any remaining
     *         space.
     *
     *  Copies up to `length` bytes from the string into `out_buffer`. If the string
     *  is shorter than `length`, the remaining buffer is filled with null bytes.
     *
     *  @param length Number of bytes to fill in the buffer.
     *  @param out_buffer Destination buffer to receive the string content and
     *                    zero-padding.
     */
    void String::fillBuffer(int64_t length, char* out_buffer) const noexcept {

        if (out_buffer != nullptr) {
            int64_t n = std::min(m_byte_length, length);

            for (int64_t i = 0; i < n; i++) {
                out_buffer[i] = m_data[i];
            }

            for (int64_t i = n; i < length; i++) {
                out_buffer[i] = 0;
            }
        }
    }

    #if defined(__APPLE__) && defined(__MACH__)
        CFStringRef String::createCFStringRef() const noexcept {

            return CFStringCreateWithCString(kCFAllocatorDefault, utf8(), kCFStringEncodingUTF8);
        }
    #endif


    #if defined(__APPLE__) && defined(__MACH__)
        CFURLRef String::createCFURLRef() const noexcept {

            return createCFURLRef(utf8());
        }
    #endif


    #if defined(__APPLE__) && defined(__MACH__)
        CFURLRef String::createCFURLRef(const char* path) noexcept {

            if (path != nullptr) {
                CFStringRef cf_path = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
                if (cf_path != nullptr) {
                    CFURLRef cf_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cf_path, kCFURLPOSIXPathStyle, false);
                    CFRelease(cf_path);
                    return cf_url;
                }
            }

            return nullptr;
        }
    #endif


    bool String::asBool() const noexcept {

        return m_data != nullptr && static_cast<bool>(0 != atoi(m_data));
    }


    int32_t String::asInt32() const noexcept {

        return m_data ? static_cast<int32_t>(atoi(m_data)) : 0;
    }


    int64_t String::asInt64() const noexcept {

        return m_data ? static_cast<int64_t>(atol(m_data)) : 0L;
    }


    float String::asFloat() const noexcept {

        return m_data ? static_cast<float>(parseDoubleWithDotOrComma(m_data)) : 0.0f;
    }


    double String::asDouble() const noexcept {

        return m_data ? parseDoubleWithDotOrComma(m_data) : 0.0;
    }


    Fix String::asFix() const noexcept {

        return Fix(m_data);
    }


    void String::toFix(Fix& out_fix) const noexcept {

        out_fix.setStr(m_data);
    }


    /**
     *  @brief Split a string into up to `max_parts` substrings using a
     *         single-character delimiter.
     *
     *  Each extracted part is copied into a contiguous memory block (`out_parts`)
     *  treated as a 2D array with dimensions [max_parts][part_len].
     *
     *  @param delimiter The character that separates values in the string.
     *  @param max_parts Maximum number of parts to extract.
     *  @param part_len Maximum length of each part (including null-terminator).
     *  @param out_parts Flat buffer for storing all parts, sized at
     *                   [max_parts * part_len]. Each part will be written at:
     *                   out_parts + i * part_len.
     *
     *  @return Number of parts successfully extracted, or -1 on buffer overflow or
     *          bad input.
     */
    int32_t String::splitFast(char delimiter, int32_t max_parts, int32_t part_len, char* out_parts) const noexcept {

        if (max_parts < 1 || part_len < 2 || !out_parts) {
            return -1;
        }

        int32_t i = 0;
        int32_t count = 0;
        char* des = out_parts;

        const char* p = m_data;
        while (*p != String::EOS && count < max_parts) {
            if (*p == delimiter) {
                if (i > 0) {
                    des[i] = String::EOS;
                    count++;
                    if (count >= max_parts) {
                        return count;
                    }
                    des = &out_parts[count * part_len];
                    i = 0;
                }
            }
            else {
                if (i >= part_len - 1) {
                    return -1;  // Buffer overflow
                }
                des[i++] = *p;
            }
            ++p;
        }

        if (i > 0 && count < max_parts) {
            des[i] = String::EOS;
            count++;
        }

        return count;
    }


    /**
     *  @brief Computes the Shannon entropy of the string.
     *
     *  This function calculates the Shannon entropy, which is a measure of the
     *  unpredictability or randomness of the characters in the string. It can
     *  operate in two modes: bit mode (where each character is treated as a
     *  separate bit) or character mode (where each unique character is treated as
     *  a symbol). The entropy is calculated based on the frequency of occurrence of
     *  each character or code point in the string.
     *
     *  @param bits_mode A boolean flag indicating the mode of calculation:
     *                   - `true` for bit mode (each character is treated as one bit).
     *                   - `false` for character mode (each unique character is
     *                     treated as a symbol).
     *
     *  @return The Shannon entropy of the string as a double. Returns 0.0 in case
     *          of errors.
     */
    double String::shannonEntropy(bool bits_mode) const noexcept {

        double entropy = 0.0;

        struct CodePoint {
            uint32_t unicode;
            uint32_t n = 0;
        };

        List<CodePoint> code_points;

        try {
            int64_t cn = length();
            for (int64_t ci = 0; ci < cn; ci++) {
                auto unicode = unicodeAtIndex(ci);
                bool exists = false;
                for (auto& cp : code_points) {
                    if (cp.unicode == unicode) {
                        cp.n++;
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    CodePoint cp = { unicode, 1 };
                    code_points.push(cp);
                }
            }

            // Compute entropy
            entropy = 0.0;
            double total = 0.0;
            if (bits_mode) {
                total = length();
            }
            else {
                total = code_points.size();
            }

            for (auto& cp : code_points) {
                double p = cp.n / total;
                entropy -= p * log2(p);
            }
        }
        catch (ErrorCode err) {
            entropy = 0.0;
        }

        return entropy;
    }


    /**
     *  @brief Integer to argument.
     */
    int64_t String::itoa(int64_t value, char* buffer, int32_t radix) noexcept {

        if (!buffer) {
            return 0;
        }

        buffer[0] = 0;

        if (value == 0) {
            buffer[0] = '0';
            buffer[1] = 0;
            return 1;
        }

        if (radix < 2 || radix > 16) {
            return 0;
        }

        bool sign = false;
        if (value < 0) {
            value = -value;
            sign = true;
        }

        char* p = buffer;
        int64_t quotient = value;
        do {
            *p++ = "0123456789abcdef"[std::llabs(quotient % radix)];
            quotient /= radix;
        } while (quotient);

        if (sign) {
            *p++ = '-';
        }

        *p = String::EOS;
        int64_t len = static_cast<int64_t>(p - buffer);
        for (int64_t i = 0; i < len / 2; i++) {
            char temp = buffer[i];
            buffer[i] = buffer[len - i - 1];
            buffer[len - i - 1] = temp;
        }

        return len;
    }


    inline bool String::isAlpha(char c) {

        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }


    inline bool String::isDigit(char c) {

        return (c >= '0' && c <= '9');
    }


    inline bool String::isExponentChar(char c) {

        return (c == 'e' || c == 'E');
    }


    inline bool String::isSignChar(char c) {

        return (c == '+' || c == '-');
    }


    /**
     *  @brief Select a word in a UTF-8 encoded string, traversing left and right
     *         from `cursor_index`.
     *
     *  This function identifies the boundaries of a word within a UTF-8 encoded
     *  string, starting from the given `cursor_index`. It traverses to the left and
     *  right to find the start and end of the word.
     *
     *  @param cursor_index Index of the UTF-8 character from which to start
     *                      traversing left and right. It is expected to be a valid
     *                      byte index in the string.
     *  @param word_characters Optional list of characters that can be part of a
     *                         word. If not provided, alphanumeric characters and
     *                         underscores (`_`) are considered part of a word.
     *  @param custom_delimiters Optional list of characters that delimit words. If
     *                           not provided, spaces and punctuation are considered
     *                           delimiters.
     *  @param[out] out_range A pair of indices representing the start and end of
     *                        the selected word. The start index is inclusive, and
     *                        the end index is exclusive.
     *  @return `true` if a word is found, otherwise `false`.
     */
    bool String::selectWord(int32_t cursor_index, const StringList* word_characters, const StringList* custom_delimiters, Rangei &out_range) noexcept {

        int32_t start_index = cursor_index;
        while (start_index >= 0) {
            auto unicode = unicodeAtIndex(start_index);

            // TODO: ????
            // String s;
            // subString(start_index, start_index, s);
            // auto unicode = unicodeFromUtf8(s.utf8());

            if (isUnicodeDelimiter(unicode)) {
                break;
            }

            /* TODO: ????
            if (s.utf8IsWhiteSpace(s.utf8())) {
                break;
            }
            if (custom_delimiters != nullptr) {
                if (custom_delimiters->contains(s)) {
                    break;
                }
            }
            */

            start_index--;
        }

        int32_t end_index = cursor_index;
        while (end_index < length()) {
            auto unicode = unicodeAtIndex(end_index);

            // TODO: ????
            // String s;
            // subString(end_index, end_index, s);
            //auto unicode = unicodeFromUtf8(s.utf8());

            if (isUnicodeDelimiter(unicode)) {
                break;
            }

            /* TODO: ????
            if (s.utf8IsWhiteSpace(s.utf8())) {
                break;
            }
            if (custom_delimiters != nullptr) {
                if (custom_delimiters->contains(s)) {
                    break;
                }
            }
             */

            end_index++;
        }

        out_range.m_min = start_index + 1;
        out_range.m_max = end_index;

        // TODO: !!!!! Implement, use `word_characters` and `custom_delimiters`.

        return true;
    }


    /**
     *  @brief Copy a substring to the pasteboard.
     *
     *  This method copies a specified range of characters from the string to the
     *  system pasteboard.
     *
     *  @param character_index The starting index of the substring to copy.
     *  @param character_length The number of characters to copy. If
     *                          `character_length` extends beyond the end of the
     *                          string, the copy will include characters up to the
     *                          end of the string.
     *
     *  @note Indices are zero-based. The function does nothing if `character_index`
     *        is out of range.
     */
    #if defined(__APPLE__) && defined(__MACH__)
        void String::copyToPasteboard(int64_t character_index, int64_t character_length) noexcept {
            if (m_data) {
                _macosApp_copyToPasteboard(this, character_index, character_length);
            }
        }
    #else
        void String::copyToPasteboard(int64_t character_index, int64_t character_length) noexcept {
            #warning "String::copyToPasteboard() must be implemented for Linux"
        }
    #endif


    /**
     *  @brief Paste the contents of the pasteboard into the string.
     *
     *  This method inserts the contents of the system pasteboard into the string starting
     *  at the specified `character_index`.
     *
     *  @param character_index The character position in the string where the pasteboard contents
     *                         should be inserted.
     *
     *  @return The number of characters inserted from the pasteboard.
     */
    #if defined(__APPLE__) && defined(__MACH__)
        int64_t String::pasteFromPasteboard(int64_t character_index) noexcept {
            if (m_data != nullptr) {
                return _macosApp_pasteFromPasteboard(this, character_index);
            }
            return 0;
        }
    #else
        int64_t String::pasteFromPasteboard(int64_t character_index) noexcept {
            #warning "String::pasteFromPasteboard() must be implemented for Linux"
            // TODO: Implement
            return 0;
        }
    #endif


    /**
     *  @brief Check memory capacity.
     */
    bool String::checkCapacity(int64_t needed) noexcept {

        if (!m_data || m_byte_capacity < 0) {
            int64_t new_capacity = needed + m_extra_grow_bytes;
            m_data = (char*)calloc(new_capacity + 1, 1);

            if (!m_data) {
                return false;
            }

            m_byte_capacity = new_capacity;
        }
        else if (needed >= m_byte_capacity) {  // Array must grow
            int64_t new_capacity = needed + m_extra_grow_bytes;
            char* new_data = (char*)std::realloc(m_data, new_capacity + 1);

            if (!new_data) {
                return false;
            }

            m_grow_count++;
            m_data = new_data;
            m_byte_capacity = new_capacity;
        }

        return true;
    }


    bool String::checkCapacity(int64_t needed, int64_t min) noexcept {

        int64_t capacity = needed;

        if (capacity < min) {
            capacity = min;
        }

        return checkCapacity(capacity);
    }


    void String::_updateInternalLengthInfo() noexcept {

        if (m_data != nullptr) {
            m_byte_length = static_cast<int64_t>(strlen(m_data));
            m_character_length = utf8Length(m_data);
        }
    }


    /**
     *  @brief Check memory capacity.
     */
    bool String::_checkExtraCapacity(int64_t needed) noexcept {

        return checkCapacity(m_byte_length + needed);
    }


    /**
     *  @brief Remove from data.
     */
    void String::_removeData(int64_t byte_index, int64_t byte_length, int64_t character_length) noexcept {

        if (byte_length > 0 && character_length >= 0) {
            char* d = &m_data[byte_index];
            char* s = &m_data[byte_index + byte_length];
            int64_t n = m_byte_length - byte_length - byte_index;

            if (n > 0) {
                while (n--) {
                    *d++ = *s++;
                }
            }

            m_character_length -= character_length;
            m_byte_length -= byte_length;
            m_data[m_byte_length] = 0;
        }
    }


    /**
     *  @brief Get the file extension from a given file path.
     *
     *  This function extracts and returns the file extension from the provided file
     *  path. If no file extension is found, an empty String is returned.
     *
     *  @return The extracted file extension as a String or an empty String if no
     *          extension is found.
     */
    String String::fileExtension() const noexcept {

        const char* ext = strrchr(utf8(), '.');

        if (ext != nullptr) {
            return String(&ext[1]);
        }
        else {
            return String::emptyString();
        }
    }


    /**
     *  @brief Extract the base name with its extension.
     *
     *  @return The file base name with its extension.
     */
    String String::fileBaseName() const noexcept {

        char buffer[1024];
        buffer[0] = String::EOS;
        std::strncpy(buffer, utf8(), 1024 - 1);

        char* base_name = basename(buffer);

        if (base_name != nullptr) {
            return String(base_name);
        }
        else {
            return String::emptyString();
        }
    }


    /**
     *  @brief Extract the base name without its extension.
     *
     *  @return The file base name without its extension.
     */
    String String::fileBaseNameWithoutExtension() const noexcept {

        char buffer[1024];
        std::strncpy(buffer, utf8(), 1024 - 1);

        char* base_name = basename(buffer);

        if (base_name != nullptr) {

            // Find the last dot (.) in the base name
            char* dot = strrchr(base_name, '.');
            if (dot != nullptr) {
                *dot = 0;
            }

            return String(base_name);
        }
        else {
            return String::emptyString();
        }
    }


/**
 *  @brief Get the path to the parent directory from a given file path.
 *
 *  This function extracts and returns the path to the parent directory from the
 *  provided file path.
 *
 *  @return The extracted directory path as a String.
 */
String String::fileDirPath() const noexcept {

        char buffer[2048];
        std::strncpy(buffer, utf8(), 2048 - 1);
        char* dir_name = dirname(buffer);

        if (dir_name != nullptr) {
            return String(dir_name);
        }
        else {
            return String::emptyString();
        }
    }


    /**
     *  @brief Get a file path with a new extension.
     *
     *  @return The file path with the new extension.
     */
    String String::filePathWithChangedExtension(const String& extension) const noexcept {

        return fileDirPath() + "/" + fileBaseNameWithoutExtension() + "." + extension;
    }


    void String::buildFilePathAtDirWithRandomName(const String& file_path, int32_t file_name_length) noexcept {

        String name;
        name.randomName(file_name_length);
        *this = file_path.fileDirPath() + "/" + name + "." + file_path.fileExtension();
    }


    /**
     *  @brief Load text into string from a file.
     */
    ErrorCode String::loadText(const String& file_path) noexcept {

        auto result = ErrorCode::None;

        /* TODO: !!!!!
        try {
            File file(file_path);
            file.startReadAscii();
            file.checkBeforeReading();
            file.readToString(*this);
            file.close();
        }
        catch (ErrorCode err) {
            result = err;
            clear();
        }
         */

        return result;
    }


    /**
     *  @brief Save string content to a file.
     */
    ErrorCode String::saveText(const String& file_path) const noexcept {

        auto result = ErrorCode::None;

        /* TODO: !!!!!
        try {
            File file(file_path);
            file.startWriteAscii();
            file.checkBeforeWriting();
            file.writeString(*this);
            file.close();
        }
        catch (ErrorCode err) {
            result = err;
        }
        */

        return result;
    }


    /**
     *  @brief Compares two C-style strings for equality.
     *
     *  This function performs a safe comparison of two null-terminated C strings.
     *  It handles `nullptr` inputs gracefully:
     *  - If both `str_a` and `str_b` are `nullptr`, they are considered equal.
     *  - If one is `nullptr` and the other is not, they are considered unequal.
     *  - Otherwise, it compares the contents of the strings using `strcmp`.
     *
     *  @param str_a Pointer to the first C-string (may be `nullptr`).
     *  @param str_b Pointer to the second C-string (may be `nullptr`).
     *  @return `true` if the strings are equal or both `nullptr`, `false` otherwise.
     */
    bool String::strSame(const char* str_a, const char* str_b) noexcept {

        if (str_a == nullptr && str_b == nullptr) {
            return true;
        }
        else if (!str_a || !str_b) {
            return false;
        }
        else {
            return strcmp(str_a, str_b) == 0;
        }
    }


    /**
     *  @brief Check if a 16 bit unicode value represents a decimal digit.
     */
    bool String::unicharIsNumeric(uint16_t c) noexcept {

        static const uint16_t table[12] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '.'
        };

        for (int32_t i = 0; i < 12; i++) {
            if (c == table[i]) {
                return true;
            }
        }

        return false;
    }


    /**
     *  @brief Get the length of a UTF-8 string in terms of the number of
     *         characters, not bytes.
     *
     *  @param str Pointer to a null terminated C-string.
     *  @return Count of UTF-8 characters in the string.
     */
    int64_t String::strUtf8Length(const char* str) noexcept {

        /*
         *  For a multi-byte UTF-8 character, the bytes that are part of the
         *  character (continuation bytes) have the highest two bits set to 10
         *  (i.e., they are in the range 0x80 to 0xBF).
         *  Therefore, it checks if the current byte does not have 0xC0 as the
         *  highest two bits. If this condition is true, it means the byte is the
         *  start of a new UTF-8 character (not a continuation byte).
         */

        int64_t n = 0;
        auto p = str;
        while (*p) {
            // Check if the byte is a leading byte (not a continuation byte)
            if ((*p & 0xC0) != 0x80) {
                n++;
            }
            p++;
        }

        return n;
    }


    /**
     *  @brief Checks if a C-string has a specific ending.
     */
    bool String::strEndsWith(const char* str, const char* ending, bool case_sensitive) noexcept {

        // Find the length of both strings
        size_t str_length = strlen(str);
        size_t end_length = strlen(str);

        // If needle is longer than haystack, haystack can't end with needle
        if (end_length > str_length) {
            return false;
        }

        if (case_sensitive) {
            return (strcmp(&str[str_length - end_length], ending)) == 0;
        }
        else {
            return (strcasecmp(&str[str_length - end_length], ending)) == 0;
        }
    }


    /**
     *  @brief Formats a double value to a C-string.
     */
    void String::strFromDouble(double value, int32_t fractional_digits, int32_t max_out_size, char* out_str) noexcept {

        static const char* format_str[] = {
            "%lld", "%.1f", "%.2f", "%.3f", "%.4f", "%.5f", "%.6f", "%.7f",
            "%.8f", "%.9f", "%.10f", "%.11f", "%.12f", "%.13f", "%.14f"
        };

        if (out_str != nullptr) {
            if (fractional_digits < 0) {
                fractional_digits = 0;
            }
            else if (fractional_digits > 14) {
                fractional_digits = 14;
            }

            if (fractional_digits == 0) {
                std::snprintf(out_str, max_out_size, format_str[fractional_digits], static_cast<int64_t>(std::round(value)));
            }
            else {
                std::snprintf(out_str, max_out_size, format_str[fractional_digits], value);
            }

            // Remove trailing zeros in fractional part
            for (int64_t i = static_cast<int64_t>(strlen(out_str)) - 1; i > 0; i--) {
                if (out_str[i] == '.') {
                    out_str[i] = 0;
                    break;
                }
                else if (out_str[i] == '0') {
                    out_str[i] = 0;
                }
                else {
                    break;
                }
            }
        }
    }


    /**
     *  @brief Get a string representing the hex values of a byte stream.
     *
     *  @param data Pointer to the stream of data.
     *  @param length Number of bytes in the stream.
     *  @param out_str A string representing the hex values of the byte stream.
     *
     *  @return true if the string could be generated, else false.
     */
    bool String::strHexFromData(const uint8_t* data, size_t length, char* out_str) noexcept {

        if (data == nullptr || length < 1 || out_str == nullptr) {
            return false;
        }
        else {
            const char* hex = "0123456789abcdef";
            const uint8_t* s = data;
            char* d = out_str;

            for (size_t i = 0; i < length; i++) {
                *d++ = hex[(*s >> 4) & 0xF];
                *d++ = hex[*s & 0xF];
                s++;
            }

            *d = String::EOS;

            return true;
        }
    }


    /**
     *  @brief Get a string representing a dataType.
     *
     *  @param ptr Pointer to the type.
     *  @param byte_count Size of the type in bytes.
     *  @param prefixed If `true`, the resulting string will start with "0x" to
     *                  indicate a hex number.
     *  @param out_str A pointer to the buffer where the generated string should be stored.
     *
     *  @return length of the resulting string.
     */
    int64_t String::strHexFromType(void* ptr, int32_t byte_count, bool prefixed, char* out_str) noexcept {

        static const char* hex = "0123456789abcdef";
        int64_t length = 0;

        if (ptr != nullptr && byte_count > 0 && out_str != nullptr) {
            auto _ptr = (uint8_t*)ptr;
            char* d = out_str;

            if (prefixed) {
                *d++ = '0';
                *d++ = 'x';
            }

            int32_t start, end, step;

            if constexpr (std::endian::native == std::endian::little) {
                start = byte_count - 1;
                end = -1;
                step = -1;
            }
            else if constexpr (std::endian::native == std::endian::big) {
                start = 0;
                end = byte_count;
                step = 1;
            }

            for (int32_t i = start; i != end; i += step) {

                int32_t nibble = (static_cast<int32_t>(_ptr[i]) >> 4) & 0xF;
                if (nibble != 0 || length > 0) {
                    *d++ = hex[nibble];
                    length++;
                }
                nibble = static_cast<int32_t>(_ptr[i]) & 0xF;
                if (nibble != 0 || length > 0) {
                    *d++ = hex[nibble];
                    length++;
                }
            }

            if (length < 1) {  // If all nibbles where 0, then the value is 0
                *d++ = '0';
                length++;
            }

            *d = String::EOS;
        }

        return prefixed ? length + 2 : length;
    }


    void String::strHexFromFloat(float value, bool prefixed, char* out_str) noexcept {

        strHexFromType((char*)&value, 4, prefixed, out_str);
    }


    void String::strHexFromDouble(double value, bool prefixed, char* out_str) noexcept {

        strHexFromType((char*)&value, 8, prefixed, out_str);
    }

    /**
     *  @brief Build float from a C-string.
     */
    bool String::strToFloat(const char* str, float& out_value) noexcept {

        bool result = false;

        if (str != nullptr) {

            if (str[0] == '0' &&  str[1] == 'x') {  // Hexadicimal
                int64_t length = static_cast<int64_t>(strlen(str)) - 2;
                uint32_t* p = reinterpret_cast<uint32_t*>(&out_value);
                *p = 0x0;

                int32_t shift = 0;
                for (int64_t j = length - 1; j >= 0; j--) {
                    uint32_t bits = 0x0;
                    switch (str[j + 2]) {
                        case '1': bits = 0x1; break;
                        case '2': bits = 0x2; break;
                        case '3': bits = 0x3; break;
                        case '4': bits = 0x4; break;
                        case '5': bits = 0x5; break;
                        case '6': bits = 0x6; break;
                        case '7': bits = 0x7; break;
                        case '8': bits = 0x8; break;
                        case '9': bits = 0x9; break;
                        case 'A': case 'a': bits = 0xA; break;
                        case 'B': case 'b': bits = 0xB; break;
                        case 'C': case 'c': bits = 0xC; break;
                        case 'D': case 'd': bits = 0xD; break;
                        case 'E': case 'e': bits = 0xE; break;
                        case 'F': case 'f': bits = 0xF; break;
                    }

                    if constexpr (std::endian::native == std::endian::little) {
                        *p |= bits << shift;
                    }
                    else if constexpr (std::endian::native == std::endian::big) {
                        *p |= bits << ((32 - 4) - shift);
                    }

                    shift += 4;
                }

                result = true;
            }
            else {
                out_value = static_cast<float>(parseDoubleWithDotOrComma(str));
                result = true;
            }
        }

        return result;
    }


    /**
     *  @brief Build double from a C-string.
     */
    bool String::strToDouble(const char* str, double& out_value) noexcept {

        bool result = false;

        if (str != nullptr) {
            if (str[0] == '0' &&  str[1] == 'x') {  // Hexadicimal
                int64_t length = static_cast<int64_t>(strlen(str)) - 2;
                auto p =  reinterpret_cast<uint64_t*>(&out_value);
                *p = 0x0L;

                int32_t shift = 0;
                for (int64_t j = length - 1; j >= 0; j--) {
                    uint64_t bits = 0x0L;

                    switch (str[j + 2]) {
                        case '1': bits = 0x1L; break;
                        case '2': bits = 0x2L; break;
                        case '3': bits = 0x3L; break;
                        case '4': bits = 0x4L; break;
                        case '5': bits = 0x5L; break;
                        case '6': bits = 0x6L; break;
                        case '7': bits = 0x7L; break;
                        case '8': bits = 0x8L; break;
                        case '9': bits = 0x9L; break;
                        case 'A': case 'a': bits = 0xAL; break;
                        case 'B': case 'b': bits = 0xBL; break;
                        case 'C': case 'c': bits = 0xCL; break;
                        case 'D': case 'd': bits = 0xDL; break;
                        case 'E': case 'e': bits = 0xEL; break;
                        case 'F': case 'f': bits = 0xFL; break;
                    }

                    if constexpr (std::endian::native == std::endian::little) {
                        *p |= bits << shift;
                    }
                    else if constexpr (std::endian::native == std::endian::big) {
                        *p |= bits << ((64 - 4) - shift);
                    }

                    shift += 4;
                }

                result = true;
            }
            else {
                out_value = parseDoubleWithDotOrComma(str);
                result = true;
            }
        }

        return result;
    }


    // float specialization
    template <>
    bool String::strToVar<float>(const char* str, float& out_value) noexcept {
        return strToFloat(str, out_value);
    }

    // double specialization
    template <>
    bool String::strToVar<double>(const char* str, double& out_value) noexcept {
        return strToDouble(str, out_value);
    }


    /**
     *  @brief Get bytes from HEX content in a C-string.
     *
     *  HEX content consists of pairs of HEX characters (0-9, A-F, a-f), where each
     *  pair represents a byte.
     *
     *  @param str C-string with HEX content.
     *  @param max_length Max number of bytes to get into `out_array`.
     *  @param out_array The resulting bytes are stored in this array.
     *
     *  @return Number of bytes or a negative error code.
     */
    int32_t String::strHexToUInt8Array(const char* str, int32_t max_length, uint8_t* out_array) noexcept {

        if (str == nullptr || out_array == nullptr) {
            return 0;
        }

        int32_t value_index = 0;
        int32_t nibble_index = 0;
        int32_t v = 0;

        for (int32_t char_index = 0; value_index < max_length; char_index++) {
            char c = str[char_index];

            if (c == String::EOS) {
                if (nibble_index != 1) {
                    return -2;
                }
                else {
                    return value_index;
                }
            }

            if (nibble_index == 0) {
                v = 0;
            }

            c = std::tolower(c);

            bool found = false;
            for (int i = 0; i < 16 && !found; i++) {
                if (c == g_hex_chars[i]) {
                    if (nibble_index == 0) {
                        v = i << 4;
                        nibble_index++;
                    }
                    else {
                        out_array[value_index] = v | i;
                        value_index++;
                        if (value_index >= max_length) {
                            return value_index;
                        }
                        nibble_index = 0;
                    }

                    found = true;
                }
            }

            if (!found) {
                return -1;
            }
        }

        return -3;
    }


    /**
     *  @brief Find a C-string in an array.
     *
     *  @return The index of the first occurence or -1, if nothing has been found.
     */
    int32_t String::indexForStrInArray(const char* str, const char** str_array) noexcept {

        if (str != nullptr && str_array != nullptr) {
            int32_t index = 0;

            while (str_array[index] != nullptr) {
                if (strcmp(str, str_array[index]) == 0) {
                    return index;
                }
                index++;
            }
        }

        return -1;
    }


    /**
     *  @brief Format a C-string representing time.
     */
    char* String::timeStrFromSeconds(int64_t seconds, int32_t max_out_size, char* out_str) noexcept {

        if (out_str != nullptr) {
            auto minutes = static_cast<int64_t>(seconds / 60);
            seconds = seconds - minutes * 60;
            auto hours = static_cast<int64_t>(minutes / 60);
            minutes = minutes - hours * 60;

            std::snprintf(
                    out_str, max_out_size,
                    "%02" PRId64 ":%02" PRId64 ":%02" PRId64,
                    hours, minutes, seconds);
        }

        return out_str;
    }


    /**
     *  @brief Format a C-string representing FPS (frames per second).
     */
    void String::fpsStr(double_t fps, int32_t max_out_size, char* out_str) noexcept {

        if (out_str != nullptr) {
            std::snprintf(out_str, max_out_size, "%.3f", fps);

            if (strstr(out_str, ".")) {
                for (auto i = static_cast<int64_t>(strlen(out_str)) - 1; i > 0; i--) {
                    if (out_str[i] == '.') {
                        out_str[i] = 0;
                        break;
                    }
                    else if (out_str[i] == '0') {
                        out_str[i] = 0;
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }


    /**
     *  @brief Get a random character for building up random names.
     */
    char String::randomNameChar() noexcept {

        const int32_t n = 10 + 26 + 26;
        static const char char_array[n] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    };

        return char_array[Random::nextInt(n - 1)];
    }


    /**
     *  @brief Build a random name.
     */
    ErrorCode String::randomName(int64_t length, char* out_str) noexcept {

        if (out_str == nullptr) {
            return ErrorCode::NullData;
        }

        if (length < 1) {
            return ErrorCode::BadArgs;
        }

        for (int64_t i = 0; i < length; i++) {
            out_str[i] = randomNameChar();
        }

        out_str[length] = String::EOS;

        return ErrorCode::None;
    }


    /**
     *  @brief Get the length of a of C-string build by a mask and a path.
     *
     *  @param mask A mask for the name format.
     *  @param path A string for a path with which the generated name should start.
     *              If nullptr, the generated name will not have a specific starting
     *              path.
     *
     *  @return The size in bytes needed to store the C-string including the
     *          terminating '\0' character.
     */
    int64_t String::randomNameLength(const char* mask, const char* path) noexcept {

        if (mask == nullptr) {
            return 0;
        }

        auto n = static_cast<int64_t>(strlen(mask));
        if (n < 1) {
            return 0;
        }

        if (path != nullptr) {
            n += strlen(path);
            n += 1;  // One slash
        }

        return n;
    }


    /**
     *  @brief Generates a random name with the format of a given mask.
     *
     *  @param mask A mask containing placeholder characters '#' where a random name
     *              should be inserted.
     *  @param path A path with which the generated string starts. If nullptr, the
     *              generated name will not have a specific starting path.
     *  @param max_out_size The maximum capacity of `out_str`.
     *  @param out_str A pointer to a buffer for the result.
     *
     *  @return `ErrorCode::None` if the name could be generated, else an error
     *          code.
     */
    ErrorCode String::randomName(const char* mask, const char* path, int64_t max_out_size, char* out_str) noexcept {

        if (mask == nullptr || out_str == nullptr) {
            return ErrorCode::NullData;
        }

        if (max_out_size < 1) {
            return ErrorCode::BadArgs;
        }

        size_t mask_length = strlen(mask);
        if (mask_length < 1) {
            return ErrorCode::BadArgs;
        }

        int64_t length = randomNameLength(mask, path);
        if ((length + 1) > max_out_size) {  // Incl. space for EOS
            return ErrorCode::BufferTooSmall;
        }

        char* d = out_str;
        if (path != nullptr) {
            strcpy(d, path);
            d += strlen(path);
            *d++ = '/';
        }

        for (size_t i = 0; i < mask_length; i++) {
            *d++ = mask[i] == '#' ? randomNameChar() : mask[i];
        }

        *d = String::EOS;

        return ErrorCode::None;
    }


    int32_t String::replaceChar(char* str, char search_c, char replacement_c) noexcept {

        int32_t n = 0;

        if (str != nullptr) {
            while (*str != String::EOS) {
                if (*str == search_c) {
                    *str = replacement_c;
                    n++;
                }
                ++str;
            }
        }

        return n;
    }


    const char* String::charSetName(CharSet char_set) noexcept {

        static const char* _names[] = {
            "UTF8",
            "ASCII 7 Bit",
            "ASCII ISO-8859-1/Latin 1",
            "ASCII Windows1252",
            "Unknown"
        };

        if (char_set >= CharSet::First && char_set <= CharSet::Last) {
            return _names[static_cast<int32_t>(char_set)];
        }
        else {
            return _names[static_cast<int32_t>(CharSet::Count)];
        }
    }


    const char** String::extendedAsciiTable(CharSet char_set) noexcept {

        switch (char_set) {
            case CharSet::ASCII_8859_1_Latin1: return g_ascii_8859_1;
            case CharSet::ASCII_Windows1252: return g_ascii_windows1252;

            default:
                return nullptr;
        }

    }


    int32_t String::extendedAsciiToUTF8(uint8_t ascii_code, CharSet char_set, char* out_utf8_code) noexcept {

        if (ascii_code < 128) {
            out_utf8_code[0] = ascii_code;
            out_utf8_code[1] = String::EOS;
            return 1;
        }

        const char** table = extendedAsciiTable(char_set);
        if (out_utf8_code == nullptr || table == nullptr || char_set < CharSet::First || char_set > CharSet::Last) {
            return 0;
        }

        char* ptr = (char*)table[ascii_code - 128];
        int32_t index = 0;
        while (*ptr) {
            out_utf8_code[index++] = *ptr++;
        }

        out_utf8_code[index] = String::EOS;

        return index;
    }


    void StringRing::write(const String& string) noexcept {

        write(string.utf8());
    }


    void StringRing::write(const char* str) noexcept {

        m_pos++;
        m_index++;

        if (m_pos >= m_size) {
            m_pos = 0;
        }

        if (m_strings[m_pos] == nullptr) {
            m_strings[m_pos] = new(std::nothrow) String(str);
        }

        else if (m_strings[m_pos] != nullptr) {
            *m_strings[m_pos] = str;
        }
    }


    void StringRing::writeFormatted(const char* format, ...) noexcept {

        constexpr std::size_t max_buffer_size = 2056;

        char buffer[max_buffer_size];
        va_list args;
        va_start(args, format);
        const int written = std::vsnprintf(buffer, max_buffer_size, format, args);
        va_end(args);

        if (written > 0 && static_cast<std::size_t>(written) < max_buffer_size) {
            write(buffer);
        }
    }


    void StringRing::writeError(const char* format, ...) noexcept {

        constexpr std::size_t max_buffer_size = 2056;
        char buffer[max_buffer_size];

        Timestamp ts;
        strcpy(buffer, "Grain Error [");
        ts.dateTimeStr(max_buffer_size - 13, &buffer[13]);
        buffer[36] = ']';
        buffer[37] = ':';
        buffer[38] = ' ';

        va_list args;
        va_start(args, format);
        const int written = std::vsnprintf(&buffer[39], max_buffer_size - 39, format, args);
        va_end(args);

        if (written > 0 && static_cast<std::size_t>(written) < max_buffer_size - 39) {
            write(buffer);
        }
    }


    const char* StringRing::read(int32_t index) const noexcept {

        static const char* _default = "!?";

        if (index >= m_size || index < 0) {
            return _default;
        }

        auto read_index = m_pos - index;

        if (read_index < 0) {
            read_index += static_cast<int32_t>(m_size);
        }

        if (m_strings[read_index] != nullptr) {
            return m_strings[read_index]->utf8();
        }
        else {
            return _default;
        }
    }


}  // End of namespace Grain
