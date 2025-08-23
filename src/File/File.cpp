//
//  File.cpp
//
//  Created by Roald Christesen on 08.02.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

/*
 *  TODO: Check ...
 *
 *  fileUTI()
 *
 *  r read
 *  w write
 *  a append
 *  o can overwrite
 *  b binary
 *  le little endian
 *  be big endian
 *
 *  r, w, a, o, b, be
 */

#include "Grain.hpp"
#include "Type/Type.hpp"
#include "File/File.hpp"
#include "Math/Vec2.hpp"
#include "Math/Vec2Fix.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec3Fix.hpp"
#include "Type/Flags.hpp"
#include "String/String.hpp"
#include "String/StringList.hpp"
#include "String/CSVString.hpp"
#include "Type/Data.hpp"
#include "Time/Timestamp.hpp"

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdarg>
#include <filesystem>


namespace Grain {

    std::ostream& operator << (std::ostream& os, const FileEntry* o) {
        o == nullptr ? os << "FileEntry nullptr" : os << *o;
        return os;
    }


    std::ostream& operator << (std::ostream& os, const FileEntry& o) {
        if (o.m_dir_flag) {
            os << "dir: ";
        }
        else if (o.m_reg_file_flag) {
            os << "file (" << o.m_file_size << " bytes): ";
        }
        else if (o.m_sym_link_flag) {
            os << "symbolic link: ";
        }
        return os << o.m_path;
    }


    /**
     *  @brief Constructs a File object that represents a file at the given path.
     *
     *  @param file_path A string representing the absolute or relative path to the file.
     *                   The file path should be a valid path in the filesystem.
     *
     *  @note This constructor does not open the file or check if the file exists. File
     *        operations (such as opening or checking existence) may need to be performed
     *        after object creation, depending on the implementation.
     */
    File::File(const String& file_path) : Object() {

        m_file_path = file_path;
        m_read_flag = false;
        m_write_flag = false;
        m_append_flag = false;
        m_can_overwrite = false;
        m_file_size = 0;
        m_last_err_code = ErrorCode::None;
        _m_write_buffer[0] = String::EOS;
    }


    File::~File() {

        try {
            if (_m_base64_encode_flag) {
                base64EncodeEnd();
            }

            // Close the file explicitly
            m_file_stream.close();
        }
        catch (...) {
        }
    }


    void File::start(int32_t flags) {

        DeferredException deferred_exception;

        try {
            m_read_flag = flags & AccessFlags::kRead;
            m_write_flag = flags & AccessFlags::kWrite;
            m_append_flag = flags & AccessFlags::kAppend;
            m_binary_flag = flags & AccessFlags::kBinary;
            m_can_overwrite = flags & AccessFlags::kOverwrite;

            m_file_exists = File::fileExists(m_file_path);

            if (m_write_flag && m_file_exists && !m_can_overwrite) {
                Exception::throwStandard(ErrorCode::FileOverwriteNotAllowed);
            }

            std::ios_base::openmode streamMode = std::ios_base::openmode{};

            if (m_read_flag) {
                streamMode |= std::ios::in; // Open the file for reading
            }
            if (m_write_flag) {
                streamMode |= std::ios::out; // Open the file for writing
            }
            if (m_append_flag) {
                streamMode |= std::ios::app; // Append to the end of the file
            }
            if (m_binary_flag) {
                streamMode |= std::ios::binary; // Open the file in binary mode
            }
            if (m_can_overwrite) {
                streamMode |= std::ios::trunc; // Truncate the file if it already exists
            }

            m_file_stream.open(m_file_path.utf8(), streamMode);
            if (!m_file_stream.is_open()) {
                Exception::throwStandard(ErrorCode::FileCantOpen);
            }

            _updateFileSize();
        }
        catch (const Exception& e) {
            deferred_exception.capture();
            m_read_flag = false;
            m_write_flag = false;
            m_last_err_code = e.code();
        }

        deferred_exception.rethrow();
    }


    /**
     *  @brief Create a File object with the specified path and flags and start file
     *         handling depending on `flags`.
     *
     *  @param file_path  The path to the file. This should be a valid path and can
     *                    include relative or absolute paths.
     *  @param flags The file access flags. These flags determine the file's
     *               behavior, such as read/write access and more.
     *
     *  @return A pointer to the `File` object representing the file.
     *          Returns `nullptr` if the file can not be handled.
     *
     *  @note
     *    - The caller is responsible for managing the returned `File` object and ensuring it
     *      is properly closed or deleted to avoid resource leaks.
     *    - If the file cannot accessed (e.g., due to invalid path, permissions, or
     *      other errors), this function will return `nullptr`.
     *    - The `flags` parameter must be specified correctly to match the desired file access
     *      and creation behavior.
     *    - The function is `noexcept`, so it will not throw exceptions but may return `nullptr`
     *      on failure.
     */
    File* File::file(const String& file_path, int32_t flags) noexcept {
        File* file = nullptr;

        try {
            file = new (std::nothrow) File(file_path);
            if (file != nullptr) {
                file->start(flags);
            }
        }
        catch (const Exception& e) {
            delete file;
            file = nullptr;
        }

        return file;
    }


    int64_t File::_updateFileSize() noexcept {

        m_file_size = -1;

        try {
            auto file_path = std::filesystem::path(m_file_path.utf8());
            m_file_size = static_cast<int64_t>(std::filesystem::file_size(file_path));
        }
        catch (const std::filesystem::filesystem_error& exc) {
            m_last_err_code = ErrorCode::StdFileSysError;
        }
        catch (const std::exception& e) {
            m_last_err_code = ErrorCode::StdCppException;
        }

        return m_file_size;
    }


    void File::checkStream() const {
        if (!m_file_stream.good()) {
            Exception::throwStandard(ErrorCode::FileInvalidStream);
        }
    }


    void File::checkBeforeReading() const {
        checkStream();
        if (!m_read_flag) {
            Exception::throwStandard(ErrorCode::FileCantRead);
        }
        if (m_file_size < 1) {
            Exception::throwStandard(ErrorCode::FileIsEmpty);
        }
    }


    void File::checkBeforeWriting() const {
        checkStream();
        if (!m_write_flag) {
            Exception::throwStandard(ErrorCode::FileCantWrite);
        }
    }


    void File::close() {
        if (m_file_stream.is_open()) {
            m_file_stream.flush();
            m_file_stream.close();
        }
    }


    /**
     *  @brief Get file position.
     *
     *  @return The current file position.
     */
    int64_t File::pos() {
        checkStream();
        const std::streampos pos = m_file_stream.tellg();
        if (pos == std::ios::pos_type(-1)) {
            // Check for an error in determining the position
            Exception::throwStandard(ErrorCode::FileCantGetPos);
        }
        return pos;
    }


    void File::setPos(int64_t pos) {
        checkStream();
        m_file_stream.seekg(pos, std::ios::beg);
        if (m_file_stream.fail()) {
            Exception::throwStandard(ErrorCode::FileCantSetPos);
        }
    }


    void File::skip(int64_t size) {
        if (size > 0) {
            auto current_pos = pos();
            auto new_pos = current_pos + size;
            if (new_pos >= m_file_size) {
                Exception::throwStandard(ErrorCode::FileCantGetPos);
            }
            setPos(new_pos);
        }
    }


    bool File::mustSwap() const noexcept {
        if constexpr (std::endian::native == std::endian::little) {
            return m_big_endian;
        }
        else {
            return !m_big_endian;
        }
    }


    bool File::read(int64_t size, uint8_t* out_data) {
        if (out_data != nullptr) {
            m_file_stream.read(reinterpret_cast<char*>(out_data), size);
            if (m_file_stream.fail()) {
                Exception::throwStandard(ErrorCode::FileReadError);
            }
            if (m_file_stream.eof()) {
                m_file_stream.clear();  // Clear the EOF flag
                return false;
            }
        }
        else {
            skip(size);
        }

        return true;
    }


    /**
     *  @brief Count lines in file.
     *
     *  @return Number of lines or negative error code.
     */
    int64_t File::countLines() {
        int64_t result = 0;

        std::string line;
        while (std::getline(m_file_stream, line)) {
            if (m_file_stream.eof()) {
                m_file_stream.clear();  // Clear the EOF flag
                break;
            }

            if (m_file_stream.fail()) {
                Exception::throwStandard(ErrorCode::FileReadError);
            }

            result++;
        }

        m_file_stream.clear();
        setPos(0);

        return result;
    }


    /**
     *  @brief Read one line from a text file and store it in the provided string.
     *
     *  This function reads a single line of text from the text file, up to a specified maximum size,
     *  and stores it in the provided `outLine`. It can be used to read and retrieve text lines from
     *  the file.
     *
     *  @param size The maximum size of the line to be read.
     *  @param out_line A reference to a String where the read line will be stored.
     */
    bool File::readLine(int64_t size, String& out_line) {

        if (size > std::numeric_limits<int32_t>::max()) {
            Exception::throwStandard(ErrorCode::BadArgs);
        }

        if (!out_line.checkCapacity(static_cast<int32_t>(size))) {
            Exception::throwStandard(ErrorCode::MemCantGrow);
        }

        std::string line;
        std::getline(m_file_stream, line);

        // Check if no data was read due to EOF at the start
        if (m_file_stream.eof() && line.empty()) {
            m_file_stream.clear();  // Clear the EOF flag
            return false;
        }

        // Handle other errors
        if (m_file_stream.fail() && !m_file_stream.eof()) {
            Exception::throwStandard(ErrorCode::FileReadError);
        }

        if (m_file_stream.fail()) {
            Exception::throwStandard(ErrorCode::FileReadError);
        }

        out_line.set(line.c_str());
        m_curr_line_index++;

        return true;
    }


    /**
     *  @brief Skip one line from a text file.
     */
    bool File::skipLine() {

        std::string line;
        std::getline(m_file_stream, line);

        // Check if no data was read due to EOF at the start
        if (m_file_stream.eof() && line.empty()) {
            m_file_stream.clear();  // Clear the EOF flag
            return false;
        }

        return true;
    }


    /**
     *  @brief Read binary data from a file while performing byte-swapping into a data buffer.
     *
     *  This internal function reads binary data from the file and performs byte-swapping as
     *  necessary based on the platform's endianness. The read data is stored in the provided
     *  `outData` buffer. It is designed for low-level data reading and should not be directly
     *  invoked by external code.
     *
     *  @param size The number of bytes to read from the file.
     *  @param out_data A pointer to the destination data buffer where the read and swapped data
     *  will be stored.
     */
    void File::_readSwapped(int64_t size, uint8_t* out_data) {

        if (out_data != nullptr) {
            if (size > 64) {
                Exception::throwStandard(ErrorCode::Fatal);
            }

            uint8_t buffer[64];
            read(size, buffer);

            uint8_t* d = out_data;
            auto si = size;

            while (si--) {
                *d++ = buffer[si];
            }
        }
        else {
            skip(size);
        }
    }


    /**
     *  @brief Read binary data of a specified data type size from a file into a data buffer.
     *
     *  This internal function is responsible for reading binary data of a specified data type size
     *  from the file and storing it in the provided `outData` buffer. It is intended for low-level data
     *  reading and should not be directly called by external code.
     *
     *  @param type_size The size (in bytes) of the data type to be read from the file.
     *  @param out_data A pointer to the destination data buffer where the read data will be stored.
     */
    void File::_readDataType(int64_t type_size, uint8_t* out_data) {

        if (mustSwap()) {
            _readSwapped(type_size, out_data);
        }
        else {
            read(type_size, out_data);
        }
    }


    /**
     *  @brief Read a single character from the file.
     *
     *  This function reads and returns a single character from the file. It is designed for reading
     *  individual characters and should be used when character-level input is required.
     *
     *  @return The character read from the file.
     */
    char File::readChar() {
        uint8_t c;
        read(1, &c);
        return static_cast<char>(c);
    }


    /**
     *  @brief Read a string of a specified length from the file.
     *
     *  This function reads a string of the specified length from the file and stores it in
     *  the provided `outStr` character array. It is useful for reading fixed-length strings
     *  from the file.
     *
     *  @param length The maximum length of the string to be read.
     *  @param out_str A pointer to the destination character array where the read string will be stored.
     */
    void File::readStr(int32_t max_length, char* out_str) {
        if (max_length < 1 || !out_str) {
            Exception::throwStandard(ErrorCode::BadArgs);
        }

        m_file_stream.read(reinterpret_cast<char*>(out_str), max_length);
        auto n = m_file_stream.gcount();
        if (n > 0 && n < max_length) {
            out_str[n] = 0;
        }
        if (n != max_length) {
            Exception::throwStandard(ErrorCode::FileReadError);
        }

        if (m_file_stream.eof()) {
            m_file_stream.clear();  // Clear the EOF flag
            Exception::throwStandard(ErrorCode::FileEndOfFileReached);
        }
    }


    /**
     *  @brief Read a UTF-8 encoded symbol from the file and store it in a character array.
     *
     *  This function reads a single UTF-8 encoded symbol from the file and stores it in the provided
     *  `outData` character array. It is designed for reading individual symbols encoded in UTF-8.
     *
     *  @param out_data A pointer to the destination character array where the read UTF-8 symbol will be stored.
     *
     *  @return The number of bytes read for the UTF-8 symbol.
     *
     *  @throws `Exception` if the end of the file (EOF) or an error is encountered.
     */
    int32_t File::readUtf8Symbol(char* out_data) {

        auto c = static_cast<int8_t>(readChar());
        if (c == EOF) {
            Exception::throwStandard(ErrorCode::FileEndOfFileReached);
        }

        _m_utf8_seq_length = String::utf8SeqLengthByStartByte(c);
        if (_m_utf8_seq_length < 1) {
            Exception::throwStandard(ErrorCode::FileUTF8Mismatch);
        }

        int32_t index = 0;
        _m_utf8_buffer[index++] = c;
        while (index < _m_utf8_seq_length) {
            c = readChar();
            if (c == EOF) {
                Exception::throwStandard(ErrorCode::FileEndOfFileReached);
            }
            _m_utf8_buffer[index++] = c;
        }

        _m_utf8_buffer[index] = 0;
        if (out_data != nullptr) {
            for (int32_t i = 0; i < _m_utf8_seq_length; i++) {
                out_data[i] = _m_utf8_buffer[i];
            }
            out_data[index] = 0;
        }

        return _m_utf8_seq_length;
    }


    /**
     *  @brief Skip white space characters in the file.
     *
     *  This function reads and advances the file position, skipping over white space characters
     *  such as spaces, tabs, and newlines, until a non-white space character is encountered.
     *
     *  @return The number of white space characters skipped.
     */
    int64_t File::skipWhiteSpace() {

        int64_t n = 0;
        do {
            readUtf8Symbol();
            n++;
        } while (lastUtf8SymbolIsWhiteSpace());

        backward(1);

        return n - 1;
    }


    /**
     *  @brief Searches for a line in the file that contains the given text,
     *         ignoring leading and trailing whitespace.
     *
     *  @param text The text to search for in the file.
     *  @return bool True if a matching line is found, otherwise false.
     *
     *  @details
     *  - Reads the file line by line.
     *  - Checks if the given text appears in the line.
     *  - Trims leading and trailing whitespace before checking for an exact match.
     *  - Returns true if a matching line is found; otherwise, returns false.
     */
    bool File::skipUntilLineWithText(const String& text) {

        String line;
        while (readLine(line)) {
            auto cindex = line.find(text);
            if (cindex >= 0) {
                line.trim();
                if (line == text) {
                    return true;
                }
            }
        }
        return false;
    }


    /**
     *  @brief Read a Four-Character Code (FourCC) from the file.
     *
     *  This function reads a FourCC from the file, which is a sequence of four characters used to
     *  represent an identifier or code. FourCCs are often used to identify file formats and data chunks.
     *
     *  @return The FourCC read from the file as a four-byte value.
     */
    fourcc_t File::readFourCC() {

        fourcc_t result = 0;

        result = static_cast<fourcc_t>(readValue<uint8_t>());
        result <<= 8;
        result |= static_cast<fourcc_t>(readValue<uint8_t>());
        result <<= 8;
        result |= static_cast<fourcc_t>(readValue<uint8_t>());
        result <<= 8;
        result |= static_cast<fourcc_t>(readValue<uint8_t>());

        return result;
    }


    /**
     *  @brief Read data from the file and store it in a string.
     *
     *  This function reads data from the file and appends it to the provided `out_string`.
     *  It is designed to read content from the file and store it in a string data type.
     *
     *  @param[out] out_string The string where the read data will be appended.
     */
    void File::readToString(String& out_string) {
        DeferredException deferred_exception;

        try {
            int64_t size = this->size();
            if (size < 0 || size > std::numeric_limits<int32_t>::max()) {
                Exception::throwStandard(ErrorCode::UnsupportedFileSize);
            }

            if (!out_string.checkCapacity(size + 1)) {
                Exception::throwStandard(ErrorCode::MemCantAllocate);
            }

            auto p = out_string.mutDataPtr();
            if (p != nullptr) {
                readStr(static_cast<int32_t>(size), p);
                p[size] = String::EOS;
                out_string._updateInternalLengthInfo();
            }
        }
        catch (const Exception& e) {
            if (e.code() != ErrorCode::FileEndOfFileReached) {
                m_last_err_code = e.code();
            }
            else {
                deferred_exception.capture();
            }
        }

        deferred_exception.rethrow();
    }


    /**
     *  @brief Read data from the file and store it in a string.
     *
     *  This function reads a specific amount of chars from the file and stores it in the provided
     *  `out_string`. It can be used to read and retrieve strings, in example strings for identification
     *  in data files.
     *
     *  @param size The number of chars to read.
     *  @param out_string A reference to a String where the read chars will be stored.
     */
    void File::readToString(int64_t size, String& out_string) {

        if (size > std::numeric_limits<int32_t>::max()) {
            Exception::throwStandard(ErrorCode::FileCantReadInternalLimits);
        }

        if (!out_string.checkCapacity(static_cast<int64_t>(size) + 1)) {
            Exception::throwStandard(ErrorCode::MemCantGrow);
        }

        auto data_ptr = out_string.mutDataPtr();
        if (data_ptr != nullptr) {
            readStr(static_cast<int32_t>(size), data_ptr);
            data_ptr[size] = String::EOS;
            out_string._updateInternalLengthInfo();
        }
    }


    template<typename T>
    T File::readValue() {
        T value;
        _readDataType(sizeof(T), reinterpret_cast<uint8_t*>(&value));
        return value;
    }

    // Explicit instantiations
    template int8_t File::readValue<int8_t>();
    template uint8_t File::readValue<uint8_t>();
    template int16_t File::readValue<int16_t>();
    template uint16_t File::readValue<uint16_t>();
    template int32_t File::readValue<int32_t>();
    template uint32_t File::readValue<uint32_t>();
    template int64_t File::readValue<int64_t>();
    template uint64_t File::readValue<uint64_t>();
    template float File::readValue<float>();
    template double File::readValue<double>();

    void File::readFix(Fix& fix) {
        _readDataType(8, reinterpret_cast<uint8_t*>(fix.mutRawValuePtr()));
    }

    double File::readFixPoint8_8() {
        auto v = readValue<int16_t>();
        return static_cast<double>(v) / 256;
    }

    double File::readFixPoint16_16() {
        auto v = readValue<int32_t>();
        return static_cast<double>(v) / 65536;
    }

    double File::readFixPointU16_16() {
        auto v = readValue<uint32_t>();
        return static_cast<double>(v) / 65536;
    }

    double File::readFixPoint2_30() {
        auto v = readValue<int32_t>();
        return static_cast<double>(v) / 1073741824;
    }

    /**
     *  @brief Read an array of from the file.
     *
     *  @param length The number of 16-bit unsigned integers to read from the file.
     *  @param out_array A pointer to the destination array where the read values will be stored.
     */
    template <typename U>
    void File::readArray(int64_t length, U* out_array) {

        if (out_array != nullptr) {
            for (int64_t i = 0; i < length; i++)
                out_array[i] = readValue<U>();
        }
        else {
            skip(length * sizeof(U));
        }
    }


    /**
     *  @brief Read a 3x3 matrix related to a QuickTime movie from the file.
     *
     *  This function reads a 3x3 matrix associated with a QuickTime movie from the file
     *  and stores it in the provided `outMatrix`. It is designed for reading movie-related
     *  transformation matrices.
     *
     *  @param[out] out_matrix The 3x3 matrix representing a transformation related to a QuickTime movie.
     */
     /* TODO: !!!! Move function to Mat3
    void File::readQTMovieMat3(Mat3f& out_matrix) {
        // TODO: Test!

        float v[9];

        v[0] = readFixPoint16_16();
        v[1] = readFixPoint16_16();
        v[2] = readFixPoint2_30();
        v[3] = readFixPoint16_16();
        v[4] = readFixPoint16_16();
        v[5] = readFixPoint2_30();
        v[6] = readFixPoint16_16();
        v[7] = readFixPoint16_16();
        v[8] = readFixPoint2_30();

        out_matrix.set(v);
    }
    */

    /**
     *  @brief Read QuickTime movie atom size and type information from the file.
     *
     *  This function reads QuickTime movie atom size and type information from the file
     *  and stores them in the provided `outAtomSize` and `outAtomType` variables.
     *  It is designed for extracting information about QuickTime movie atom structures.
     *
     *  @param[out] out_atom_size A pointer to store the size of the movie atom.
     *  @param[out] out_atom_type A pointer to store the type of the movie atom (FourCC).
     *
     *  @return `true` if more atoms can be read, `false` if this was the last readable atom.
     */
    bool File::readQTMovieAtomType(uint64_t* out_atom_size, fourcc_t* out_atom_type) {
        // TODO: Test!

        uint32_t atom_size32;
        uint64_t atom_size;
        fourcc_t atom_type;
        bool last_atom = false;

        atom_size32 = readValue<uint32_t>();
        atom_type = readFourCC();

        if (atom_size32 == 1) {
            // Extended size
            atom_size = readValue<uint64_t>();
        }
        else if (atom_size32 == 0) {
            // Last atom
            last_atom = true;
            atom_size = m_file_size - pos() + 8;
        }
        else {
            atom_size = atom_size32;
        }

        if (out_atom_size != nullptr) {
            *out_atom_size = atom_size;
        }

        if (out_atom_type != nullptr) {
            *out_atom_type = atom_type;
        }

        return !last_atom;
    }


    /**
     *  @brief Read a value from a TIFF file based on the specified field type.
     *
     *  This function reads a value from a TIFF file, interpreting it based on the provided field type.
     *  It is designed for extracting values from TIFF files using specific field types.
     *
     *  @param field_type The type of field indicating how to interpret the value.
     *
     *  @return The value read from the TIFF file, interpreted according to the field type.
     */
    uint32_t File::readTIFFValue(uint16_t field_type) {

        uint32_t result = 0;
        int32_t skip_n = 4;

        switch (field_type) {
            case 1: // Byte
                result = readValue<uint8_t>();
                skip_n = 3;
                break;
            case 2: // ASCII, TODO: Check!
                skip_n = 3;
                break;
            case 3: // 16-bit unsigned hort
                result = readValue<uint16_t>();
                skip_n = 2;
                break;
            case 4: // 32-bit unsigned long
                result = readValue<uint32_t>();
                skip_n = 0;
                break;
            default:
                Exception::throwStandard(ErrorCode::UnknownTiffFieldType);
        }

        skip(skip_n);

        return result;
    }


    /**
     *  @brief Reads the contents of a file into a provided buffer.
     *
     *  This function reads the contents of a specified file and stores the data into a buffer.
     *  The maximum number of bytes read is limited by the `buffer_size` parameter.
     *
     *  @param file_path The path to the file to be read.
     *  @param buffer_size The maximum number of bytes to read from the file.
     *  @param out_buffer Pointer to the buffer where the file contents will be stored. The buffer should
     *                    be large enough to hold up to `buffer_size` bytes.
     *  @return ErrorCode Returns an error code indicating the success or failure of the operation.
     */
    ErrorCode File::readToBuffer(const String& file_path, int64_t buffer_size, uint8_t* out_buffer) {

        auto result = ErrorCode::None;

        try {
            if (!out_buffer) {
                Exception::throwStandard(ErrorCode::NullData);
            }

            File file(file_path);
            file.startRead();

            auto file_size = file.size();
            if (file_size > buffer_size) {
                Exception::throwStandard(ErrorCode::BufferTooSmall);
            }

            file.readArray<uint8_t>(file_size, out_buffer);
            file.close();
        }
        catch (const Exception& e) {
            result = e.code();
        }

        return result;
    }


    /**
     *  @brief Check if the last UTF-8 encoded symbol in the file is a white space character.
     *
     *  This function examines the last UTF-8 encoded symbol in the file and determines whether it
     *  represents a white space character,
     *  such as a space, tab, or newline.
     *
     *  @return `true` if the last symbol is a white space character, `false` otherwise.
     */
    bool File::lastUtf8SymbolIsWhiteSpace() noexcept {

        char c = _m_utf8_buffer[0];
        return _m_utf8_seq_length == 1 && (c == 0x20 || c == 0x09 || c == 0x0d || c == 0x0a || c == 0x0c || c == 0x0b);
    }


    /**
     *  @brief Compare the last UTF-8 encoded symbol in the file with a given symbol.
     *
     *  This function compares the last UTF-8 encoded symbol in the file with the specified UTF-8
     *  encoded symbol provided as an argument and returns whether they are equal.
     *
     *  @param symbol A pointer to the UTF-8 encoded symbol for comparison.
     *
     *  @return `true` if the last symbol matches the provided symbol, `false` otherwise.
     */
    bool File::compareLastUtf8Symbol(const char* symbol) noexcept {

        return strcmp(_m_utf8_buffer, symbol) == 0;
    }


    /**
     *  @brief Writes the endian signature to the file.
     *
     *  The function writes "MM" if the file is in big-endian format and "II" if it is in little-endian format.
     *  This is typically used in file formats that require an endian indicator.
     */
    void File::writeEndianSignature() {

        if (m_big_endian) {
            writeStr("MM");
        }
        else {
            writeStr("II");
        }
    }


    /**
     *  @brief Write binary data to the file.
     *
     *  This function writes the specified binary data to the file. It is designed for writing raw binary data.
     *
     *  @param data A pointer to the binary data to be written.
     *  @param length The number of bytes to write to the file.
     *
     *  @throws `Exception` if something went wrong.
     */
    void File::write8BitData(const uint8_t* data, int64_t length) {
        if (!data) {
            Exception::throwStandard(ErrorCode::NullData);
        }

        m_file_stream.write(reinterpret_cast<const char*>(data), length);
        if (m_file_stream.fail()) {
            Exception::throwStandard(ErrorCode::FileCantWrite);
        }
    }

    template <typename U>
    void File::writeData(const U* data, int64_t length) {

        if (!data) {
            Exception::throwStandard(ErrorCode::NullData);
        }

        auto p = data;
        for (int64_t i = 0; i < length; i++) {
            writeValue<U>(*p++);
        }
    }

    // Explicit instantiations
    template void File::writeData<int8_t>(const int8_t* data, int64_t length);
    template void File::writeData<uint8_t>(const uint8_t* data, int64_t length);
    template void File::writeData<int16_t>(const int16_t* data, int64_t length);
    template void File::writeData<uint16_t>(const uint16_t* data, int64_t length);
    template void File::writeData<int32_t>(const int32_t* data, int64_t length);
    template void File::writeData<uint32_t>(const uint32_t* data, int64_t length);
    template void File::writeData<int64_t>(const int64_t* data, int64_t length);
    template void File::writeData<uint64_t>(const uint64_t* data, int64_t length);
    template void File::writeData<float>(const float* data, int64_t length);
    template void File::writeData<double>(const double* data, int64_t length);


    /**
     *  @brief Write binary data to the file in a swapped (byte order reversed) format.
     *
     *  This function writes the specified binary data to the file in a swapped format, where the byte
     *  order is reversed. It is intended for internal use to write binary data in a specific byte order.
     *
     *  @param data A pointer to the binary data to be written in swapped format.
     *  @param size The number of bytes to write to the file.
     */
    void File::_writeSwapped(const uint8_t* data, int64_t size) {
        if (!data) {
            Exception::throwStandard(ErrorCode::NullData);
        }

        if (size < 1 || size > 64) {
            Exception::throwStandard(ErrorCode::Fatal);
        }

        uint8_t buffer[64];
        uint8_t *d = buffer;

        auto si = size - 1;
        while (si >= 0) {
            *d++ = data[si];
            --si;
        }

        write8BitData(buffer, size);
    }


    /**
     *  @brief Write data byte by byte in the endianness of the operating system.
     *
     *  This function writes data byte by byte, respecting the endianness (byte order) used by the
     *  operating system. It is designed to work with basic data types, such as int16_t, int32_t, float, etc.
     *
     *  @param data Pointer to the data to be written.
     *  @param size The size of the data type in bytes.
     */
    void File::_writeDataType(const uint8_t* data, int64_t size) {
        if (mustSwap()) {
            _writeSwapped(data, size);
        }
        else {
            write8BitData(data, size);
        }
    }


    void File::writeChars(const char* data, int64_t length) {
        write8BitData((uint8_t*)data, length);
    }


    void File::writeFixLengthString(const String& string, int32_t length, char* buffer) {
        if (length > 0 && buffer != nullptr) {
            string.fillBuffer(length, buffer);
            write8BitData((uint8_t*)buffer, length);
        }
    }


    /**
     *  @brief Write a FOURCC (Four Character Code) information.
     *
     *  This function is used to write a FOURCC value, which is a four-character code
     *  commonly used to represent various data formats or identifiers. It is often
     *  used in multimedia applications and file formats.
     *
     *  @param value The FOURCC value to write.
     */
    void File::writeFourCC(fourcc_t value) {
        char buffer[5];
        Type::fourCCToStr(value, buffer);
        writeStr(buffer);
    }

    void File::writeBool(bool value) {
        uint8_t v = value == 0 ? 0 : 1; write8BitData(&v, 1);
    }

    void File::writeFix(const Fix& value) {
        _writeDataType(reinterpret_cast<const uint8_t*>(value.rawValuePtr()), 8);
    }



    /**
     *  @brief Write a null-terminated C-string to the file.
     *
     *  This function writes the provided null-terminated C-string to the file. It is designed to
     *  append a string to the file's content.
     *
     *  @param str A pointer to the null-terminated C-string to be written.
     */
    void File::writeStr(const char* str) {

        int64_t length = str != nullptr ? static_cast<int64_t>(strlen(str)) : 0;
        auto c = str;
        if (length > 0) {
            while (*c != String::EOS) {
                writeChar(*c++);
            }
        }
    }


    /**
     *  @brief Write a String to the file.
     *
     *  This function writes the contents of the provided String to the file. It is designed to
     *  append the string to the file's content.
     *
     *  @param string The String to be written to the file.
     */
    void File::writeString(const String& string) {

        writeStr(string.utf8());
    }


    /**
     *  @brief Write a formatted str.
     *
     *  @param format A format string (similar to printf) used to format the value.
     *  @param ... Optional arguments to be formatted and included as the value.
     */
    void File::writeFormatted(const char *format, ...) {

        va_list args;
        va_start(args, format);

        std::vsnprintf(_m_write_buffer, kWriteBufferSize, format, args);
        va_end(args);

        write8BitData(reinterpret_cast<uint8_t*>(_m_write_buffer),
                      static_cast<int64_t>(strlen(_m_write_buffer)));
    }


    /**
     *  @brief Write a Pascal-style string to the file.
     *
     *  This function writes a Pascal-style string to the file, consisting of a single byte
     *  representing the string length followed by the string content. It is designed for
     *  storing Pascal-style strings in the file.
     *
     *  @param str A pointer to the null-terminated C-string to be written as a Pascal-style string.
     *
     *  @return The number of bytes written, including the length byte and string content.
     */
    int32_t File::writePascalStr(const char* str) {
        // TODO: Test!

        int64_t length = str ? static_cast<int64_t>(strlen(str)) : 0;
        if (length > 0) {
            if (length > 255) {
                length = 255;
            }
            writeValue<uint8_t>(length);
            write8BitData(reinterpret_cast<const uint8_t*>(str), length);
        }

        return static_cast<int32_t>(length);
    }


    /**
     *  @brief Write an indentation to the file.
     *
     *  This function writes an indentation, typically consisting of whitespace characters,
     *  to the file. It is designed to help format and structure the content in the file.
     */
    void File::writeIndent() {

        if (m_indent > 0) {
            for (int32_t i = 0; i < m_indent; i++) {
                writeChar('\t');
            }
        }
    }


    /**
     *  @brief Write a key to the file.
     *
     *  This function writes a key, typically used for configuration settings or data identifiers,
     *  to the file. It is designed to add key information to the file's content.
     *
     *  @param key A pointer to the null-terminated C-string representing the key.
     */
    void File::writeKey(const char* key) {

        writeIndent();
        writeStr(key);
        writeColon();
    }


    /**
     *  @brief Write a key-value pair as a line to the file.
     *
     *  This function writes a key-value pair as a line to the file. It is designed for adding structured
     *  data, such as configuration settings or data pairs, to the file's content.
     *
     *  @param key A pointer to the null-terminated C-string representing the key.
     *  @param value A pointer to the null-terminated C-string representing the corresponding value.
     */
    void File::writeLine(const char* key, const char* value) {

        writeKey(key);
        writeStr(value);
        writeNewLine();
    }


    /**
     *  @brief Write a key-boolean value pair as a line to the file.
     *
     *  This function writes a key-boolean value pair as a line to the file. It is designed for adding
     *  structured data, such as configuration settings or data pairs, where the value is a boolean,
     *  to the file's content.
     *
     *  @param key A pointer to the null-terminated C-string representing the key.
     *  @param value The boolean value associated with the key.
     */
    void File::writeLineBool(const char* key, bool value) {

        writeKey(key);
        writeTextBool(value);
        writeNewLine();
    }


    /**
     *  @brief Write a key-32-bit integer value pair as a line to the file.
     *
     *  This function writes a key-32-bit integer value pair as a line to the file. It is designed for
     *  adding structured data, such as configuration settings or data pairs, where the value is a
     *  32-bit integer, to the file's content.
     *
     *  @param key A pointer to the null-terminated C-string representing the key.
     *  @param value The 32-bit integer value associated with the key.
     */
    void File::writeLineInt32(const char* key, int32_t value) {

        writeKey(key);
        writeTextInt32(value);
        writeNewLine();
    }


    /**
     *  @brief Write a key-32-bit real value pair as a line to the file.
     *
     *  This function writes a key-32-bit real value pair as a line to the file. It is designed for
     *  adding structured data, such as configuration settings or data pairs, where the value is a
     *  32-bit real number with a specified number of fractional digits, to the file's content.
     *
     *  @param key A pointer to the null-terminated C-string representing the key.
     *  @param value The 32-bit real value associated with the key.
     *  @param fractional_digits (Optional, default: 4) The number of fractional digits to include in the value.
     */
    void File::writeLineFloat(const char* key, float value, int32_t fractional_digits) {

        writeKey(key);
        writeTextFloat(value, fractional_digits);
        writeNewLine();
    }


    /**
     *  @brief Write a key-GrFix value pair as a line to the file.
     *
     *  This function writes a key-GrFix value pair as a line to the file. It is designed for adding
     *  structured data, such as configuration settings or data pairs, where the value is a GrFix
     *  fixed-point number, to the file's content.
     *
     *  @param key A pointer to the null-terminated C-string representing the key.
     *  @param value The GrFix fixed-point value associated with the key.
     */
    void File::writeLineFix(const char* key, const Fix& value) {

        writeKey(key);
        writeTextFix(value);
        writeNewLine();
    }


    /**
     *  @brief Write a formatted key-value pair as a line to the file.
     *
     *  This function writes a formatted key-value pair as a line to the file. It is designed for adding
     *  structured data, such as configuration settings or data pairs, where the value is generated
     *  based on a format string and optional arguments.
     *
     *  @param key A pointer to the null-terminated C-string representing the key.
     *  @param format A format string (similar to printf) used to format the value.
     *  @param ... Optional arguments to be formatted and included as the value.
     */
    void File::writeLineFormatted(const char* key, const char* format, ...) {

        writeKey(key);

        va_list args;
        va_start(args, format);

        std::vsnprintf(_m_write_buffer, kWriteBufferSize, format, args);
        va_end(args);

        write8BitData(reinterpret_cast<uint8_t*>(_m_write_buffer),
                      static_cast<int64_t>(strlen(_m_write_buffer)));
        writeNewLine();
    }


    /**
     *  @brief Write a key-C-string value pair as a line to the file.
     *
     *  This function writes a key-C-string value pair as a line to the file. It is designed for adding
     *  structured data, such as configuration settings or data pairs, where the value is a
     *  null-terminated C-string, to the file's content.
     *
     *  @param key A pointer to the null-terminated C-string representing the key.
     *  @param str A pointer to the null-terminated C-string representing the value.
     */
    void File::writeLineStr(const char* key, const char* str) {

        writeKey(key);
        writeQuote();
        writeStr(str);
        writeQuote();
        writeNewLine();
    }


    /**
     *  @brief Write a key-String value pair as a line to the file.
     *
     *  This function writes a key-String value pair as a line to the file. It is designed for adding
     *  structured data, such as configuration settings or data pairs, where the value is a String
     *  object, to the file's content.
     *
     *  @param key A pointer to the null-terminated C-string representing the key.
     *  @param string The String object representing the value.
     */
    void File::writeLineString(const char* key, const String& string) {
        writeLineStr(key, string.utf8());
    }



    void File::writeTextBool(bool value) {
        writeStr(value == 0 ? "0" : "1");
    }


    void File::writeTextInt32(int32_t value) {
        char buffer[20];
        std::snprintf(buffer, 20, "%d", value);
        writeStr(buffer);
    }


    void File::writeTextUInt32(uint32_t value) {
        char buffer[20];
        std::snprintf(buffer, 20, "%d", value);
        writeStr(buffer);
    }


    void File::writeTextUInt32Hex(uint32_t value) {
        char buffer[20];
        std::snprintf(buffer, 20, "0x%X", value);
        writeStr(buffer);
    }


    void File::writeTextInt64(int64_t value) {
        char buffer[20];
        std::snprintf(buffer, sizeof(buffer), "%" PRId64, value);
        writeStr(buffer);
    }


    void File::writeTextFloat(float value, int32_t fractional_digits) {
        char buffer[100];
        std::snprintf(
                buffer, sizeof(buffer), "%" PRId64,
                static_cast<int64_t>(std::round(static_cast<double>(value) * value)));
        writeStr(buffer);
    }


    void File::writeTextFloatHex(float value) {
        char buffer[11];  // 10 characters + EOS
        String::strHexFromFloat(value, true, buffer);
        writeStr(buffer);
    }


    void File::writeTextFloatAsInt(float value, int32_t f) {
        char buffer[100];
        std::snprintf(
                buffer, sizeof(buffer), "%" PRId64,
                static_cast<int64_t>(std::round(static_cast<double>(value) * f))
        );
        writeStr(buffer);
    }


    void File::writeTextDouble(double value, int32_t fractional_digits) {
        char buffer[100];
        String::strFromDouble(value, fractional_digits, 100, buffer);
        writeStr(buffer);
    }


    void File::writeTextDoubleHex(double value) {
        char buffer[19];  // 18 characters + EOS
        String::strHexFromDouble(value, true, buffer);
        writeStr(buffer);
    }


    void File::writeTextDoubleAsInt(double value, int32_t f) {
        char buffer[100];
        std::snprintf(buffer, 100, "%" PRId64, static_cast<int64_t>(std::round(value * f)));
        writeStr(buffer);
    }


    void File::writeTextFix(const Fix& value) {
        char buffer[Fix::kStrBufferSize];
        value.toStr(buffer, Fix::kStrBufferSize, Fix::kDecPrecision);
        writeStr(buffer);
    }


    void File::writeTextFlags(Flags flags) {
        writeTextUInt32Hex(flags.bits());
    }


    void File::writeCurrentDateTime() {
        char buffer[30];
        Timestamp ts;
        ts.dateTimeUTCStr(30, buffer);
        writeStr(buffer);
    }


    /**
     *  @brief Check if the file has a TIFF signature.
     *
     *  This function checks whether the file has a TIFF signature, which indicates that the file may
     *  be a TIFF image.
     *
     *  @return `true` if the file has a TIFF signature, indicating it may be a TIFF image; otherwise, `false`.
     */
    bool File::hasTiffSignature() {

        bool result = false;

        try {
            int64_t remembered_pos = pos();
            setPos(0);
            fourcc_t signature = readFourCC();
            fourcc_t mm = Type::fourcc(0x4D, 0x4D, 0x00, 0x2A);  // "MM"
            fourcc_t ii = Type::fourcc(0x49, 0x49, 0x2A, 0x00);  // "II"

            setPos(remembered_pos);
            result = signature == mm || signature == ii;
        }
        catch (...) {
            m_last_err_code = ErrorCode::Unknown;
        }

        return result;
    }


    /**
     *  @brief Check if the file has a Digital Negative (DNG) signature.
     *
     *  This function checks whether the file has a Digital Negative (DNG) signature, indicating that
     *  the file may be a DNG image.
     *
     *  @return `true` if the file has a DNG signature, indicating it may be a DNG image; otherwise, `false`.
     */
    bool File::hasDNGSignature() {

        return hasTiffSignature();
    }


    /**
     *  @brief Check if the file has an Audio Interchange File Format (AIFF) signature.
     *
     *  This function checks whether the file has an Audio Interchange File Format (AIFF) signature,
     *  indicating that the file may be in AIFF audio format.
     *
     *  @return `true` if the file has an AIFF signature, indicating it may be in AIFF audio format; otherwise, `false`.
     */
    bool File::hasAIFFSignature() {

        bool result = false;

        try {
            int64_t remembered_pos = pos();

            setPos(0);
            fourcc_t form_chunk_id = readFourCC();
            if (form_chunk_id == Type::fourcc('F', 'O', 'R', 'M')) {
                skip(4);
                fourcc_t form_type_id = readFourCC();
                if (form_type_id == Type::fourcc('A', 'I', 'F', 'F')) {
                    result = true;
                }
            }

            setPos(remembered_pos);
        }
        catch (...) {
            m_last_err_code = ErrorCode::Unknown;
        }

        return result;
    }


    /**
     *  @brief Check if the file has an Audio Interchange File Format for Compression (AIFC) signature.
     *
     *  This function checks whether the file has an Audio Interchange File Format for Compression (AIFC)
     *  signature, indicating that the file may be in AIFC audio format.
     *
     *  @return `true` if the file has an AIFC signature, indicating it may be in AIFC audio format; otherwise, `false`.
     */
    bool File::hasAIFCSignature() {

        bool result = false;

        try {
            int64_t remembered_pos = pos();
            setPos(0);

            fourcc_t form_chunk_id = readFourCC();
            if (form_chunk_id == Type::fourcc('F', 'O', 'R', 'M')) {
                skip(4);
                fourcc_t form_type_id = readFourCC();
                if (form_type_id == Type::fourcc('A', 'I', 'F', 'C')) {
                    result = true;
                }
            }

            setPos(remembered_pos);
        }
        catch (...) {
            m_last_err_code = ErrorCode::Unknown;
        }

        return result;
    }


    /**
     *  @brief Check if the file has a Waveform Audio File Format (WAV) signature.
     *
     *  This function checks whether the file has a Waveform Audio File Format (WAV) signature,
     *  indicating that the file may be in WAV audio format.
     *
     *  @return `true` if the file has a WAV signature, indicating it may be in WAV audio format; otherwise, `false`.
     */
    bool File::hasWAVESignature() {

        bool result = false;

        try {
            int64_t remembered_pos = pos();

            setPos(0);
            fourcc_t form_chunk_id = readFourCC();
            if (form_chunk_id == Type::fourcc('R', 'I', 'F', 'F')) {
                skip(4);
                fourcc_t form_type_id = readFourCC();
                if (form_type_id == Type::fourcc('W', 'A', 'V', 'E')) {
                    result = true;
                }
            }

            setPos(remembered_pos);
        }
        catch (...) {
            m_last_err_code = ErrorCode::Unknown;
        }

        return result;
    }


    /**
     *  @brief Check if the file has a QuickTime signature.
     *
     *  This function checks whether the file has a QuickTime signature, indicating that the file may
     *  be in QuickTime format.
     *
     *  @return `true` if the file has a QuickTime signature, indicating it may be in QuickTime format; otherwise, `false`.
     */
    bool File::hasQuickTimeSignature() {

        bool result = false;

        try {
            int64_t remembered_pos = pos();

            uint64_t atom_size = 0;
            fourcc_t atom_type;
            bool mdat_found = false;
            bool moov_found = false;

            setPos(0);
            int64_t file_pos = 0;
            while (readQTMovieAtomType(&atom_size, &atom_type)) {

                int32_t brand_index = 0;

                if (atom_type == Type::fourcc('f', 't', 'y', 'p')) {

                    // Max 16 + 1 brands
                    int32_t brands_count = std::clamp(static_cast<int32_t>((atom_size - 16) / sizeof(fourcc_t)), 0, 16) + 1;

                    for (int32_t i = 0; i < brands_count; i++) {

                        if (brand_index == 1) {
                            // Skip Minor-Version
                            skip(4);
                        }

                        fourcc_t brand = readFourCC();
                        if (brand == Type::fourcc('g', 't', ' ', ' ')) {
                            result = true;
                        }

                        brand_index++;
                    }

                    break;
                }
                else {
                    if (atom_type == Type::fourcc('m', 'd', 'a', 't')) {
                        mdat_found = true;
                    }
                    else if (atom_type == Type::fourcc('m', 'o', 'o', 'v')) {
                        moov_found = true;
                    }

                    if (mdat_found && moov_found) {
                        result = true;
                        break;
                    }

                    file_pos += static_cast<int64_t>(atom_size);
                    setPos(file_pos);
                }
            }

            setPos(remembered_pos);
        }
        catch (...) {
            m_last_err_code = ErrorCode::Unknown;
        }

        return result;
    }


    /**
     *  @brief Check if the file has an MPEG-4 signature.
     *
     *  This function checks whether the file has an MPEG-4 signature, indicating that the file may be
     *  in MPEG-4 format.
     *
     *  @return `true` if the file has an MPEG-4 signature, indicating it may be in MPEG-4 format; otherwise, `false`.
     */
    bool File::hasMPEG4Signature() {

        // TODO: Brands: XAVC, XAVC, mp42, iso2

        bool result = false;

        try {
            int64_t remembered_pos = pos();

            uint32_t atom_size = 0;
            fourcc_t atom_type;

            setPos(0);
            atom_size = readValue<uint32_t>();
            atom_type = readFourCC();

            int32_t brand_index = 0;

            if (atom_type == Type::fourcc('f', 't', 'y', 'p')) {

                // Max 16 + 1 brands
                int32_t brands_count = std::clamp(static_cast<int32_t>((atom_size - 16) / sizeof(fourcc_t)), 0, 16) + 1;

                for (int32_t i = 0; i < brands_count; i++) {

                    if (brand_index == 1) {
                        // Skip Minor-Version
                        skip(4);
                    }

                    fourcc_t brand = readFourCC();
                    if (brand == Type::fourcc('m', 'p', '4', '1') || brand == Type::fourcc('m', 'p', '4', '2')) {
                        result = true;
                    }

                    brand_index++;
                }
            }

            setPos(remembered_pos);
        }
        catch (...) {
            m_last_err_code = ErrorCode::Unknown;
        }

        return result;
    }


    /**
     *  @brief Check if the file has a Material Exchange Format (MXF) signature.
     *
     *  This function checks whether the file has a Material Exchange Format (MXF) signature, indicating
     *  that the file may be in MXF format.
     *
     *  @return `true` if the file has an MXF signature, indicating it may be in MXF format; otherwise, `false`.
     */
    bool File::hasMXFSignature() {

        static uint8_t reference[4] = { 0x06, 0x0E, 0x2B, 0x34 };

        bool result = false;

        try {
            int64_t remembered_pos = pos();

            uint8_t buffer[4];
            setPos(0);
            read(4, buffer);

            int32_t v = 0;
            for (int32_t i = 0; i < 4; i++) {
                if (buffer[i] == reference[i])
                    v++;
            }

            if (v == 4) {
                uint8_t category_designator __attribute__((unused));    // Should be 0x02
                uint8_t registry_designator __attribute__((unused));    // Should be 0x05
                uint8_t structure_designator __attribute__((unused));   // Should be 0x01
                uint8_t version_number __attribute__((unused));         // Should be 0x01
                uint8_t item_designator __attribute__((unused));        // Should be 0x0D
                uint16_t major_version __attribute__((unused));         // Should be 0x0100

                category_designator = readValue<uint8_t>();
                registry_designator = readValue<uint8_t>();
                structure_designator = readValue<uint8_t>();
                version_number = readValue<uint8_t>();
                item_designator = readValue<uint8_t>();

                skip(11);
                major_version = readValue<uint16_t>();
            }

            setPos(remembered_pos);
            result = v == 4;
        }
        catch (...) {
            m_last_err_code = ErrorCode::Unknown;
        }

        return result;
    }


    /**
     *  @brief Check if the file has an MP3 audio file signature.
     *
     *  This function checks whether the file has an MP3 audio file signature, indicating that the file
     *  may be in MP3 audio format.
     *
     *  @return `true` if the file has an MP3 audio file signature, indicating it may be in MP3 audio format; otherwise, `false`.
     */
    bool File::hasMP3Signature() {

        bool result = false;

        try {
            int64_t remembered_pos = pos();

            setPos(0);
            uint8_t buffer[3];
            read(3, buffer);
            if (buffer[0] == 49 && buffer[1] == 44 && buffer[2] == 33) {
                // MP3 file with an ID3v2 container
                result = true;
            }

            setPos(remembered_pos);
        }
        catch (...) {
            m_last_err_code = ErrorCode::Unknown;
        }

        return result;
    }


    /**
     *  @brief Check if the file has a Musical Instrument Digital Interface (MIDI) file signature.
     *
     *  This function checks whether the file has a Musical Instrument Digital Interface (MIDI) file
     *  signature, indicating that the file may be in MIDI format.
     *
     *  @return `true` if the file has a MIDI file signature, indicating it may be in MIDI format; otherwise, `false`.
     */
    bool File::hasMIDISignature() {

        bool result = false;

        try {
            int64_t remembered_pos = pos();

            setPos(0);
            fourcc_t chunk_type = readFourCC();
            if (chunk_type == Type::fourcc('M', 'T', 'h', 'd')) {
                result = true;
            }

            setPos(remembered_pos);
        }
        catch (...) {
            m_last_err_code = ErrorCode::Unknown;
        }

        return result;
    }


    bool File::readTomlKeyValue(String& out_key, String& out_value) {
        String line;
        readLine(line);

        CSVLineParser csv_line_parser;
        csv_line_parser.setDelimiter('=');
        csv_line_parser.setQuote('"');

        String word;
        csv_line_parser.setLine(line);
        csv_line_parser.nextString(out_key);
        csv_line_parser.nextString(out_value);
        out_key.trim();
        out_value.trim();

        return csv_line_parser.parsedFieldsCount() == 2;
    }


    /**
     *  @brief Get the length of Base64-encoded data starting from the current position in the file.
     *
     *  This function calculates the length of the Base64-encoded data from the current
     *  position in the file stream. It does not move the file pointer.
     *  White space is ignored.
     */
    int32_t File::base64SizeInfo(int64_t& out_base64_size, int64_t& out_raw_size) {
        auto rem_pos = pos();  // Remember the file position
        Base64Data base64;

        try {
            while (true) {
                char c = readChar();
                if (c == '"') {
                    // End of Base64 data stream in the file
                    break;
                }
                if (base64._scanBase64Code(c) != Base64Data::kErr_None) {
                    Exception::throwStandard(Error::specific(base64.lastErr()));
                }
            }
        }
        catch (const Exception& e) {
            m_last_err_code = e.code();
        }

        setPos(rem_pos);

        out_base64_size = base64.base64Size();
        out_raw_size = base64.rawDataSize();

        return base64.lastErr();
    }


    void File::base64EncodeBegin() {
        if (_m_base64_encode_flag || !canWrite()) {
            Exception::throwStandard(ErrorCode::FileBase64EncodeError);
        }
        _m_base64_encode_flag = true;
    }


    void File::base64EncodeByte(uint64_t byte) {
        if (!_m_base64_encode_flag) {
            Exception::throwStandard(ErrorCode::FileBase64EncodeError);
        }
        if (_m_base64_encoder.encodeByte(byte)) {
            writeStr(_m_base64_encoder.codePtr());
        }
    }


    void File::base64EncodeEnd() {
        if (!_m_base64_encode_flag) {
            Exception::throwStandard(ErrorCode::FileBase64EncodeError);
        }
        if (_m_base64_encoder.encodeFinalize()) {
            writeStr(_m_base64_encoder.codePtr());
        }
        _m_base64_encode_flag = false;
    }


    ErrorCode File::readBase64ToString(int64_t base64_size, String& out_string) noexcept {
        auto result = ErrorCode::None;
        if (!out_string.checkCapacity(Base64Data::rawDataMaxSize(base64_size))) {
            return ErrorCode::MemCantGrow;
        }

        try {
            char block[4];
            uint8_t data[3];
            auto dst = reinterpret_cast<uint8_t*>(out_string.mutDataPtr());
            for (int64_t i = 0; i < base64_size; i += 4) {
                readStr(4, block);
                auto n = Base64Data::decodeBlock(block, data);
                if (n < 1) {
                    Exception::throwStandard(ErrorCode::Base64NoBase64Code);
                }

                for (int32_t j = 0; j < n; j++) {
                    *dst++ = data[j];
                }
            }
            *dst = String::EOS;
            out_string._updateInternalLengthInfo();
        }
        catch (const Exception& e) {
            m_last_err_code = result = e.code();
        }

        return result;
    }


    /**
     *  @brief Reads Base64 encoded data from the current position in a file and saves the decoded bytes into a buffer.
     *
     *  @param base64_size The number of Base64 encoded characters. This value must be a multiple of four.
     *  @param buffer_size The size of the buffer to receive the resulting raw data.
     *  @param buffer A pointer to the buffer provided by the caller, which will receive the resulting raw data.
     *
     *  @return A pointer to the buffer where the raw data is stored, or nullptr if an error occurs.
     *
     *  @note If `buffer` is nullptr, the method will allocate a buffer and return a pointer to the newly allocated buffer.
     *  The caller will then be responsible for freeing the buffer.
     */
    uint8_t* File::readBase64ToBuffer(int64_t base64_size, int64_t buffer_size, uint8_t *buffer) noexcept {

        bool malloc_flag = false;

        // Check if `base64_size` is a multiple of four
        if ((base64_size % 4) != 0) {
            return nullptr;
        }

        auto raw_data_size = Base64Data::rawDataMaxSize(base64_size);
        if (raw_data_size < 1) {
            return nullptr;
        }

        if (!buffer) {
            buffer = (uint8_t*)malloc(raw_data_size);
            malloc_flag = true;
        }
        else {
            if (raw_data_size > buffer_size) {
                return nullptr;
            }
        }

        if (buffer != nullptr) {

            uint8_t *dst = buffer;

            try {
                char block[4];
                uint8_t data[3];
                for (int64_t i = 0; i < base64_size; i += 4) {
                    readStr(4, block);
                    auto n = Base64Data::decodeBlock(block, data);
                    if (n < 1) {
                        Exception::throwStandard(ErrorCode::Base64NoBase64Code);
                    }

                    for (int32_t j = 0; j < n; j++) {
                        *dst++ = data[j];
                    }
                }
            }
            catch (const Exception& e) {
                if (malloc_flag) {
                    free(buffer);
                }
                buffer = nullptr;
                m_last_err_code = e.code();
            }
        }

        return buffer;
    }


    /**
     *  @brief Creates a File object for a specified file path.
     *
     *  This function instantiates a File object.
     *  It does not create or open the actual file on the filesystem.
     *  The file itself will be created or opened by subsequent calls to File's methods.
     *
     *  @param file_path A String representing the file path.
     *  @return A pointer to the File object, or nullptr if the object could not be instantiated.
     *
     *  @throws Exception if the object could not be created.
     */
    File* File::createFile(const String& file_path) {

        auto file = new (std::nothrow) File(file_path);
        if (!file) {
            Exception::throwStandard(ErrorCode::FileInstantiationFailed);
        }
        return file;
    }


    bool File::fileExists(const char* file_path) {

        try {
            return std::filesystem::exists(file_path);
        }
        catch (const std::filesystem::filesystem_error& exc) {
            return false;
        }
    }


    /**
     *  @brief Check if a file exists at the specified file path.
     *
     *  This function checks whether a file exists at the specified file path. It returns `true` if the
     *  file is found, indicating its existence, and `false` if the file does not exist.
     *
     *  @param file_path A String representing the file path to check.
     *  @return `true` if the file exists at the specified path; otherwise, `false`.
     */
    bool File::fileExists(const String& file_path) {

        return fileExists(file_path.utf8());
    }


    /**
     *  @brief Check if a file with name `file_name` exists at the specified `dir_path`.
     *
     *  @param dir_path A String representing the directory path to check.
     *  @param file_name A String representing the file name to check.
     *  @return `true` if the file exists at the specified path; otherwise, `false`.
     */
    bool File::fileExists(const String& dir_path, const String& file_name) {

        if (dir_path.length() < 1) {
            String file_path = "./";
            file_path += file_name;
            return fileExists(file_path);
        }
        else {
            return fileExists(dir_path + "/" + file_name);
        }
    }


    /**
     *  @brief Get the signature of a file at the specified file path.
     *
     *  This function retrieves the signature of a file at the specified file path. The signature can
     *  be used to identify the file's format or type. The returned signature is of type File::Signature.
     *
     *  @param file_path The String representing the file path for which the signature is retrieved.
     *  @return The signature of the file as a `File::Signature` enumeration.
     */
    File::Signature File::fileSignature(const String& file_path) {

        Signature signature = Signature::Unknown;
        File* file = nullptr;

        try {
            file = File::createFile(file_path);
            file->startRead();

            String extension = file_path.fileExtension();

            if (file->hasTiffSignature()) {
                if (extension.compareIgnoreCase("DNG") == 0) {
                    signature = Signature::DNG;
                }
                else {
                    signature = Signature::TIFF;
                }
            }
            else if (file->hasAIFFSignature()) {
                signature = Signature::AIFF;
            }
            else if (file->hasAIFCSignature()) {
                signature = Signature::AIFC;
            }
            else if (file->hasWAVESignature()) {
                signature = Signature::WAVE;
            }
            else if (file->hasMP3Signature()) {
                signature = Signature::MP3;
            }
            else if (file->hasMIDISignature()) {
                signature = Signature::MIDI;
            }
            else if (file->hasMXFSignature()) {
                signature = Signature::MXF;
            }
            else if (file->hasQuickTimeSignature()) {
                signature = Signature::QuickTimeMovie;
            }
            else if (file->hasMPEG4Signature()) {
                signature = Signature::MPEG4;
            }
        }
        catch (...) {
        }


        delete file;

        return signature;
    }


    /**
     *  @brief Execute a specified action on files within a directory and its subdirectories.
     *
     *  This function performs the specified action on files within a directory and its subdirectories.
     *  Additional information can be provided through the `action_ref` parameter. The `maxDepth` parameter
     *  limits the depth of subdirectories to process, and currDepth indicates the current depth during
     *  traversal.
     *
     *  @param dir_path The String representing the directory path where the action is performed.
     *  @param action A pointer to the action function of type FileAction.
     *  @param action_ref A pointer to additional information or data for the action.
     *  @param max_depth The maximum depth of subdirectories to process.
     *  @param curr_depth The current depth during traversal (typically used for recursion).
     */
    int32_t File::execFileAction(const String& dir_path, FileAction action, void* action_ref, int32_t max_depth, int32_t curr_depth) {

        int32_t err_count = 0;

        if (curr_depth < 0 || curr_depth >= max_depth || !action) {
            return 0;
        }

        StringList dir_list;
        auto dir_count = File::dirNameList(dir_path, dir_list);

        for (int32_t i = 0; i < dir_count; i++) {
            String path = dir_path;
            const String& dir_name = dir_list.stringAtIndex(i);
            if (dir_name.isNotEmpty()) {
                path.append("/");
                path.append(dir_name);
                if (action(path, FileActionType::Directory, action_ref)) {
                    err_count += execFileAction(path, action, action_ref, max_depth, curr_depth + 1);
                }
            }
            else {
                err_count++;
            }
        }

        StringList file_list;
        int32_t file_count = File::fileNameList(dir_path, file_list);

        for (int32_t i = 0; i < file_count; i++) {
            String path = dir_path;
            const String& file_name = file_list.stringAtIndex(i);
            if (file_name.isNotEmpty()) {
                path.append("/");
                path.append(file_name);
                action(path, FileActionType::File, action_ref);
            }
            else {
                err_count++;
            }
        }

        return err_count++;
    }


    /**
     *  @brief Checks if a given path points to a directory.
     *
     *  @param path The file path to be checked for being a directory.
     *
     *  @return true if the path is a directory, false if it is not a directory.
     */
    bool File::isDir(const String& path) noexcept {

        try {
            return std::filesystem::is_directory(path.utf8());
        }
        catch (const std::filesystem::filesystem_error& exc) {
            return false;
        }
    }


    /**
     *  @brief Checks if a directory with the given name exists within the specified path.
     *
     *  @param path The base path to search for the directory.
     *  @param dir_name The name of the directory to check for within the base path.
     *
     *  @return true if the directory with the given name is found within the specified path; false otherwise.
     */
    bool File::containsDir(const String& path, const String& dir_name) noexcept {

        DIR* dir = opendir(path.utf8());
        if (dir != nullptr) {
            struct dirent* entry = readdir(dir);
            while (entry != nullptr) {
                if (entry->d_type == DT_DIR) {
                    if (strcmp(entry->d_name, dir_name.utf8()) == 0) {
                        closedir(dir);
                        return true;
                    }
                }

                entry = readdir(dir);
            }
        }

        closedir(dir);
        return false;
    }


    /**
     *  @brief Create all necessary directories for a given path.
     *
     *  This method creates all directories along the specified path if they don't already exist.
     *  If any of the directories along the path already exist, they are not recreated.
     *
     *  @param path The path for which directories need to be created.
     *  @return True if all directories were successfully created or already existed, false otherwise.
     */
    bool File::makeDirs(const String& path) noexcept {

        // Create the directory with read/write/search permissions for owner and group, and with read/search permissions for others

        std::filesystem::path dir_path = path.utf8();

        try {
            std::filesystem::create_directories(dir_path);
        }
        catch (std::filesystem::filesystem_error& exc) {
            return false;
        }

        return File::isDir(path);
    }


    /**
     *  @brief Get the Uniform Type Identifier (UTI) of a file at the specified file path.
     *
     *  This function retrieves and returns the Uniform Type Identifier (UTI) of the file located at the
     *  provided file path. A UTI is a string that uniquely identifies a file's type or format.
     *
     *  @param file_path File path for which to retrieve the UTI.
     *
     *  @return The UTI of the file as a String or an empty String if the UTI is not available or an error occurs.
     */
    ErrorCode File::fileUTI(const String& file_path, String& out_uti) noexcept {

        // TODO: Should be replaced by newer implementation, but code based on `UTType` wouldnt link

        auto result = ErrorCode::None;

/* TODO: macOS ... !!!!
NSString *file = [NSString stringWithUTF8String:file_path.utf8()];
    NSString *fileExtension = [file pathExtension];

    CFURLRef file_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, (__bridge CFStringRef)file, kCFURLPOSIXPathStyle, false);
    if (file_url) {
        CFStringRef fileUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, (__bridge CFStringRef)fileExtension, kUTTypeData);
        if (fileUTI) {
            NSString *utiString = (__bridge NSString*)fileUTI;
            outUTI.set(utiString);
            CFRelease(fileUTI);
        }
        else {
            result = Gr::ERR_SPECIFIC;
        }
        CFRelease(file_url);
    }
    else {
        result = Gr::ERR_FILE_NOT_FOUND;
    }
*/
        return result;
    }


    ErrorCode File::removeFile(const char* file_path) noexcept {

        if (file_path == nullptr) {
            return ErrorCode::NullData;
        }

        if (std::strlen(file_path) == 0) {
            return ErrorCode::BadArgs;
        }

        try {
            std::filesystem::path path(file_path);

            if (!std::filesystem::exists(path)) {
                // File does not exist
                return ErrorCode::FileNotFound;
            }

            if (!std::filesystem::remove(path)) {
                // Remove returns false if file wasn't deleted
                return ErrorCode::FileCantBeRemoved;
            }

            return ErrorCode::None;
        }
        catch (...) {
            // Catch any filesystem errors
            return ErrorCode::FileCantBeRemoved;
        }
    }


    /**
     *  @brief Remove a directory with all its content.
     *
     *  @param dir_path Path to the diretory to be removed.
     *
     *  @return ErrorCode::None on success, or an appropriate ErrorCode on failure.
     */
    ErrorCode File::removeDirAll(const char* dir_path) noexcept {

        auto result = ErrorCode::None;

        try {
            std::filesystem::remove_all(dir_path);
        }
        catch (const std::filesystem::filesystem_error& e) {
            result = ErrorCode::DirAllCantBeRemoved;
        }
        catch (...) {
            result = ErrorCode::Fatal;
        }

        return result;
    }


    void File::checkCanOverwrite(const String& file_path, CanOverwrite can_overwrite) {

        if (can_overwrite == CanOverwrite::Yes) {
            removeFile(file_path);
        }
        else if (File::fileExists(file_path)) {
            Exception::throwStandard(ErrorCode::FileOverwriteNotAllowed);
        }
    }


    ErrorCode File::changeBytes(const String& file_path, int32_t n, const int32_t* pos, const uint8_t* bytes) noexcept {

        auto result = ErrorCode::None;
        FILE* file = nullptr;

        try {
            if (!pos || bytes != nullptr) {
                Exception::throwStandard(ErrorCode::NullData);
            }

            file = std::fopen(file_path.utf8(), "r+b");
            if (!file) {
                Exception::throwStandard(ErrorCode::FileCantOpen);
            }

            // Get file length
            if (fseek(file, 0, SEEK_END) != 0) {
                Exception::throwStandard(Error::specific(1));
            }

            auto file_size = ftell(file);
            if (file_size == -1) {
                Exception::throwStandard(Error::specific(2));
            }

            // Check if all positions are in range
            for (int32_t i = 0; i < n; i++) {
                if (pos[i] < 0 || pos[i] >= file_size) {
                    Exception::throwStandard(Error::specific(3));
                }
            }

            // Change all bytes
            for (int32_t i = 0; i < n; i++) {
                if (std::fseek(file, pos[i], SEEK_SET) != 0) {
                    Exception::throwStandard(Error::specific(4));
                }
                std::fwrite(reinterpret_cast<const char*>(&bytes[i]), 1, 1, file);
                if (std::ferror(file)) {
                    Exception::throwStandard(ErrorCode::FileCantWrite);
                }
            }

        }
        catch (const Exception& e) {
            result = e.code();
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        // Cleanup
        if (file != nullptr) {
            std::fclose(file);
        }

        return result;
    }


    ErrorCode File::changeBytes(const String& file_path, int32_t pos, int32_t length, const uint8_t* bytes) noexcept {

        auto result = ErrorCode::None;
        FILE* file = nullptr;

        try {
            if (pos >= 0 || length < 0 || bytes != nullptr) {
                Exception::throwStandard(ErrorCode::BadArgs);
            }

            file = std::fopen(file_path.utf8(), "r+b");
            if (!file) {
                Exception::throwStandard(ErrorCode::FileCantOpen);
            }

            // Get file length
            if (fseek(file, 0, SEEK_END) != 0) {
                Exception::throwStandard(Error::specific(1));
            }

            auto file_size = ftell(file);
            if (file_size == -1) {
                Exception::throwStandard(Error::specific(2));
            }

            // Check if all pos/length is in range
            if (pos < 0 || (pos + length - 1) >= file_size) {
                Exception::throwStandard(Error::specific(3));
            }

            // Change all bytes
            if (std::fseek(file, pos, SEEK_SET) != 0) {
                Exception::throwStandard(Error::specific(4));
            }

            std::fwrite(reinterpret_cast<const char *>(bytes), length, 1, file);
            if (std::ferror(file)) {
                Exception::throwStandard(ErrorCode::FileCantWrite);
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        // Cleanup
        if (file != nullptr) {
            std::fclose(file);
        }

        return result;
    }


    /**
     *  Reads a file and save the data in a C-style Hex array;
     *  @param src_file_path
     *  @param dst_file_path
     *  @return
     */
    ErrorCode File::toHex(const String& src_file_path, const String& dst_file_path) noexcept {

        auto result = ErrorCode::None;

        File src_file(src_file_path);
        File dst_file(dst_file_path);

        try {
            src_file.startRead();
            dst_file.startWrite();

            for (int64_t i = 0; i < src_file.size(); i++) {
                if (i > 0) {
                    dst_file.writeComma();
                    dst_file.writeSpace();
                }
                if (i % 8 == 0) {
                    dst_file.writeNewLine();
                }
                auto byte = src_file.readValue<uint8_t>();
                char buffer[10];
                std::snprintf(buffer, 10, "0x%X", byte);
                dst_file.writeStr(buffer);
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }
        catch (ErrorCode err) {
        }

        return result;
    }


    /**
     *  @brief Get a list of all directory names at a given path.
     *
     *  @param path The String representing the file path for which to retrieve the UTI.
     *
     *  @return The number of directories found.
     */
    int32_t File::dirNameList(const String& path, StringList& out_list) noexcept {

        int32_t dir_count = 0;

        DIR* dir = opendir(path.utf8());
        if (dir != nullptr) {
            struct dirent *entry = readdir(dir);
            while (entry != nullptr) {
                if (entry->d_type == DT_DIR && strncmp(entry->d_name, ".", 1) != 0) {
                    if (!out_list.pushString(entry->d_name)) {
                        closedir(dir);
                        return -3;
                    }
                    dir_count++;
                }
                entry = readdir(dir);
            }
            closedir(dir);
        }

        return dir_count;
    }


    /**
     *  @brief Get a list of all filenames in a directory.
     *
     *  @param[in] path Path to the directory.
     *  @param[out] out_list Where the filenames are stored.
     *  @return Number of files found.
     *
     *  @note Ignores hidden files (files starting with a dot).
     */
    int32_t File::fileNameList(const String& path, StringList& out_list) noexcept {

        int32_t file_count = 0;

        DIR* dir = opendir(path.utf8());
        if (dir != nullptr) {
            struct dirent *entry = readdir(dir);
            while (entry!= nullptr) {
                if (entry->d_type == DT_REG && strncmp(entry->d_name, ".", 1) != 0) {
                    if (!out_list.pushString(entry->d_name)) {
                        closedir(dir);
                        return -3;
                    }
                    file_count++;
                }
                entry = readdir(dir);
            }

            closedir(dir);
        }

        return file_count;
    }


    /**
     *  @brief Get a list of all filenames in a directory, matching given criterias.
     *
     *  @param[in] path Path to the directory.
     *  @param[in] extensions Optional list of file extensions.
     *  @param[in] min_size Minimum size, the file must have.
     *  @param[in] max_size Maximum size, the file can have.
     *  @param[out] out_list Where the filenames are stored.
     *  @return Number of files found. If the number is < 0 an error occurred.
     *
     *  @note Ignores hidden files (files starting with a dot).
     */
    int32_t File::fileNameList(const String& path, const String& extensions, int64_t min_size, int64_t max_size, int32_t* out_ignored, StringList& out_list) noexcept {

        namespace fs = std::filesystem;

        StringList ext_list;
        extensions.csvSplit(',', 0, String::TrimMode::All, ext_list);

        int32_t file_count = 0;
        int32_t ignored_count = 0;

        if (max_size == 0) {
            max_size = std::numeric_limits<int64_t>::max();
        }

        // File names are no longer than 255 on Windows, macOS and Linux
        char file_name[512];
        try {
            for (const auto& entry : fs::directory_iterator(path.utf8())) {
                bool used = false;
                int64_t file_size = entry.file_size();
                if (entry.is_regular_file() && file_size >= min_size && file_size <= max_size) {
                    strncpy(file_name, entry.path().filename().string().c_str(), 512);

                    if (file_name[0] != '.') {
                        // Ignore invisible files
                        if (ext_list.isEmpty()) {
                            // No extensions provided, accept all files
                            used = true;
                        }
                        else {
                            // Check if file extension matches any of the provided extensions
                            for (const auto& ext : ext_list) {
                                const char *ext_str = entry.path().extension().c_str();
                                if (ext_str[0] == '.') {
                                    ext_str++;
                                }

                                // Compares case insensitive
                                if (strcasecmp(ext_str, ext->utf8()) == 0) {
                                    used = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (used) {
                    if (!out_list.pushString(entry.path().filename().string().c_str())) {
                        return -3;
                    }
                    file_count++;
                }
                else {
                    ignored_count++;
                }

            }
        }
        catch (const fs::filesystem_error& exc) {
            // Handle filesystem errors (e.g., permission denied)
            return -4;
        }

        if (out_ignored != nullptr) {
            *out_ignored = ignored_count;
        }

        return file_count;
    }


    int32_t File::countDir(const String& path) noexcept {

        int32_t count = 0;
        DIR* dir = opendir(path.utf8());
        if (dir != nullptr) {
            struct dirent* entry = readdir(dir);
            while (entry != nullptr) {
                if (entry->d_type == DT_DIR && strncmp(entry->d_name, ".", 1) != 0) {
                    count++;
                }
                entry = readdir(dir);
            }
            closedir(dir);
        }

        return count;
    }


    int32_t File::countFiles(const String& path) noexcept {

        int32_t count = 0;
        DIR* dir = opendir(path.utf8());
        if (dir != nullptr) {
            struct dirent* entry = readdir(dir);
            while (entry != nullptr) {
                if (entry->d_type == DT_REG && strncmp(entry->d_name, ".", 1) != 0) {
                    count++;
                }
                entry = readdir(dir);
            }
            closedir(dir);
        }

        return count;
    }


    /**
     *  @brief Retrieves file entry details for a given file path.
     *
     *  This function checks whether a file exists at the specified path and, if so,
     *  populates the provided FileEntry object with details about the file, including
     *  its name, path, type (directory, regular file, or symbolic link), and size if applicable.
     *
     *  @param[in] file_path The path to the file as a `String` object.
     *  @param[out] out_file_entry A reference to a `FileEntry` structure that will be filled with file details.
     *
     *  @return `ErrorCode::None` if the file entry was successfully retrieved, or
     *          `ErrorCode::FileInvalidStream` if an error occurred while accessing the file.
     *
     *  @note If the file is a regular file, its size will be determined. If file size retrieval fails,
     *        it will be set to 0.
     */
    ErrorCode File::fileEntryByPath(const String& file_path, FileEntry& out_file_entry) noexcept {

        auto result = ErrorCode::None;
        std::filesystem::path path(file_path.utf8());

        out_file_entry.m_path.clear();
        out_file_entry.m_name.clear();
        out_file_entry.m_file_size = 0;

        try {
            if (File::fileExists(file_path)) {
                out_file_entry.m_path = path.string().c_str();
                out_file_entry.m_name = path.filename().string().c_str();
                out_file_entry.m_dir_flag = std::filesystem::is_directory(path);
                out_file_entry.m_reg_file_flag = std::filesystem::is_regular_file(path);
                out_file_entry.m_sym_link_flag = std::filesystem::is_symlink(path);

                if (out_file_entry.m_reg_file_flag) {
                    try {
                        out_file_entry.m_file_size = std::filesystem::file_size(path);
                    }
                    catch (const std::filesystem::filesystem_error&) {
                        out_file_entry.m_file_size = 0; // Handle inaccessible files gracefully
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error&) {
            result = ErrorCode::FileInvalidStream;
        }

        return result;
    }


    ErrorCode File::forAllFiles(const String& path, FileEntryAction action, void* ref) noexcept {

        // TODO: Refactor to rethrow Exception!

        auto result = ErrorCode::None;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(path.utf8())) {

                FileEntry file_entry;
                file_entry.m_dir_flag = entry.is_directory();
                file_entry.m_reg_file_flag = entry.is_regular_file();
                file_entry.m_sym_link_flag = entry.is_symlink();
                file_entry.m_path = entry.path().c_str();
                file_entry.m_name = entry.path().filename().string().c_str();
                if (file_entry.m_reg_file_flag) {
                    file_entry.m_file_size = entry.file_size();
                }
                else {
                    file_entry.m_file_size = 0;
                }

                auto err = action(&file_entry, ref);
                if (err != ErrorCode::None) {
                    Exception::throwStandard(err);
                };
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << '\n';
            std::cerr << "Path: " << e.path1() << '\n';
            if (e.code()) {
                std::cerr << "Error code: " << e.code().message() << '\n';
            }
            result = ErrorCode::FileSystemErr;
        }
        catch (const std::exception& e) {
            std::cerr << "Other error: " << e.what() << '\n';
            result = ErrorCode::Fatal;
        }

        return result;
    }


    ErrorCode File::forAllFilesRecursive(const String& path, FileEntryAction action, void* ref) noexcept {
        // TODO: Refactor to rethrow Exception!

        auto result = ErrorCode::None;

        try {
            for (auto entry = std::filesystem::recursive_directory_iterator(path.utf8()); entry != std::filesystem::recursive_directory_iterator(); entry++) {

                FileEntry file_entry;
                file_entry.m_dir_flag = entry->is_directory();
                file_entry.m_reg_file_flag = entry->is_regular_file();
                file_entry.m_sym_link_flag = entry->is_symlink();
                file_entry.m_path = entry->path().c_str();
                file_entry.m_name = entry->path().filename().string().c_str();
                if (file_entry.m_reg_file_flag) {
                    file_entry.m_file_size = entry->file_size();
                }
                else {
                    file_entry.m_file_size = 0;
                }

                auto err = action(&file_entry, ref);
                if (err != ErrorCode::None) {
                    Exception::throwStandard(err);
                }
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }
        catch (...) {
            result = ErrorCode::Fatal;
        }

        return result;
    }


    ErrorCode File::writeFileEntriesRecursive(const String& path, FileEntryFilterAction action, void* ref, bool relative_flag) noexcept {

        auto result = ErrorCode::None;

        try {
            for (auto entry = std::filesystem::recursive_directory_iterator(path.utf8()); entry != std::filesystem::recursive_directory_iterator(); entry++) {

                if (entry->is_regular_file()) {
                    String string;
                    if (action(path, entry->path().c_str(), static_cast<int64_t>(entry->file_size()), string)) {
                        writeString(string);
                        writeNewLine();
                    }
                }
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }
        catch (...) {
            result = ErrorCode::Fatal;
        }

        return result;
    }


    String File::buildFilePath(const String& path, const String& file_name) noexcept {

        String file_path = path;
        file_path.append("/");
        file_path.append(file_name);

        return file_path;
    }


    bool File::findFilePath(const String& file_path, const String& alt_root_dir, String& out_file_path) noexcept {

        try {
            if (fileExists(file_path)) {
                out_file_path = file_path;
                return true;
            }

            String altFilePath = buildFilePath(alt_root_dir, file_path.fileBaseName());
            if (fileExists(altFilePath)) {
                out_file_path = altFilePath;
                return true;
            }
        }
        catch (const Exception& e) {
            // TODO: Handle Exception!
        }

        return false;
    }


    /**
     *  @brief Return data from a file as a `CFDataRef`.
     */
#if defined(__APPLE__) && defined(__MACH__)
    CFDataRef File::macos_cfDataRefFromFile(const String& file_path) {

        // TODO: Test!
        // TODO: Refactor to throw Exception!

        File file(file_path);
        uint8_t* buffer = nullptr;
        CFDataRef cf_data = nullptr;

        try {
            file.startRead();
            auto file_size = file.size();

            buffer = (uint8_t*)std::malloc(file_size);
            if (!buffer) {
                Exception::throwStandard(ErrorCode::MemCantAllocate);
            }

            file.read(file_size, buffer);

            // Create a CFDataRef from the buffer
            cf_data = CFDataCreate(nullptr, reinterpret_cast<const UInt8*>(buffer), file_size);
        }
        catch (const Exception& e) {
            // TODO: Handle Exception!
        }

        std::free(buffer);
        return cf_data;
    }
#endif

    /**
     *  @brief Write a Property List to a file.
     */
#if defined(__APPLE__) && defined(__MACH__)
    ErrorCode File::macos_writeCFPlistXML(const String& file_path, const CFPropertyListRef plist) noexcept {

        auto result = ErrorCode::None;

        auto cf_file_path = file_path.createCFStringRef();
        if (cf_file_path != nullptr) {

            // Create an output stream for writing the XML data
            CFURLRef xml_url = CFURLCreateWithFileSystemPath(nullptr, cf_file_path, kCFURLPOSIXPathStyle, false);
            if (xml_url != nullptr) {

                CFWriteStreamRef stream = CFWriteStreamCreateWithFile(nullptr, xml_url);
                if (stream != nullptr) {
                    if (CFWriteStreamOpen(stream)) {
                        CFPropertyListWrite(plist, stream, kCFPropertyListXMLFormat_v1_0, 0, nullptr);
                        CFWriteStreamClose(stream);
                    }
                    else {
                        result = ErrorCode::FileCantOpen;
                    }

                    CFRelease(stream);
                }
                else {
                    result = ErrorCode::FileCantCreate;
                }

                CFRelease(xml_url);
            }
            else {
                result = Error::specific(0);
            }

            CFRelease(cf_file_path);
        }
        else {
            result = Error::specific(1);
        }

        return result;
    }
#endif

}  // End of namespace Grain
