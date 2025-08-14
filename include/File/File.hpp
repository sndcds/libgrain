//
//  File.hpp
//
//  Created by Roald Christesen on 08.02.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainFile_hpp
#define GrainFile_hpp

#include "Grain.hpp"
#include "Type/Fix.hpp"
#include "Type/Object.hpp"
#include "Type/List.hpp"
#include "String/String.hpp"
#include "Type/Data.hpp"

#include <fstream>


namespace Grain {


    enum class FileActionType {
        Directory = 1,
        File = 2
    };


    typedef bool (*FileAction)(String& path, FileActionType type, void* action_ref);


    class File;
    class Flags;
    class StringList;
    class Vec2Fix;
    class Vec3Fix;


    /**
     *  @struct FileEntry
     *  @brief Represents a file or directory entry with metadata.
     */
    struct FileEntry : public Object {
        String m_path;              ///< Absolute full path to the file or directory
        String m_name;              ///< Name of the file or directory
        uint64_t m_file_size;       ///< Size of the file in bytes (0 for directories)
        bool m_dir_flag;            ///< True if the entry is a directory
        bool m_reg_file_flag;       ///< True if the entry is a regular file
        bool m_sym_link_flag;       ///< True if the entry is a symbolic link
    };

    std::ostream& operator << (std::ostream& os, const FileEntry* o);
    std::ostream& operator << (std::ostream& os, const FileEntry& o);


    /**
     *  @typedef FileEntryAction
     *  @brief Function pointer type for performing an action on a file entry.
     *  @param entry Pointer to the FileEntry structure.
     *  @param ref User-defined reference data passed to the function.
     *  @return ErrorCode indicating the success or failure of the action.
     */
    typedef ErrorCode (*FileEntryAction)(const FileEntry* entry, void* ref);

    /**
     *  @typedef FileEntryFilterAction
     *  @brief Function pointer type for filtering file entries based on criteria.
     *  @param base_path Base directory path.
     *  @param file_path Relative or absolute path of the file.
     *  @param file_size Size of the file in bytes.
     *  @param out_string Output parameter for storing filtered file information.
     *  @return True if the file matches the filter criteria, false otherwise.
     */
    typedef bool (*FileEntryFilterAction)(const String& base_path, const char* file_path, int64_t file_size, String& out_string);


    /**
     *  @brief File base class.
     *
     *  `File` is a utility class that represents a file capable of read and write operations.
     *  It supports reading and writing basic data types such as integers and floating-point numbers, taking care of their endianness. Additionally, it provides convenient utility methods for various file-related operations.
     *
     *  Supported operations include opening and closing files, seeking to specific positions, and managing file pointers. The class abstracts the complexities of working with file I/O, allowing developers to focus on their application logic.
     *
     *  Working with files should always be done within a try-catch block, as most functions throw exceptions of type ErrorCode when something goes wrong.
     *
     *  @note The term "endianness" refers to the byte order of multi-byte data types, ensuring compatibility between different architectures.
     */
    class File : public Object {

    public:
        enum {
            kWriteBufferSize = 10000,  // TODO: Check this. Should it be implemented dynamic?
            kFileActionMaxRecursionDeoth = std::numeric_limits<int32_t>::max()
        };

        enum AccessFlags : int32_t {
            kRead = 0x1,
            kWrite = 0x1 << 1,
            kAppend = 0x1 << 2,
            kBinary = 0x1 << 3,
            kOverwrite = 0x1 << 4
        };

        enum class Signature {
            Unknown = -1,
            TIFF,
            DNG,
            AIFF,
            AIFC,
            WAVE,
            QuickTimeMovie,
            MPEG4,
            MXF,
            MP3,
            MIDI
        };

    public:
        explicit File(const String& file_path);
        ~File() override;

        const char* className() const noexcept override { return "File"; }

        friend std::ostream& operator << (std::ostream& os, const File* o) {
            o == nullptr ? os << "File nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const File& o) {
            return os << "file" << std::endl; // TODO: !!!!!
        }

        /* TODO: !!!!!
        void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {
            Log log(os, indent);
            log.header(label);
            log << "m_file_path : " << m_file_path << log.endl;
            log << "m_file_size: " << m_file_size << log.endl;
            log << "m_big_endian: " << m_big_endian << log.endl;
            log << "m_read_flag: " << log.boolValue(m_read_flag) << log.endl;
            log << "m_write_flag: " << log.boolValue(m_write_flag) << log.endl;
            log << "m_append_flag: " << log.boolValue(m_append_flag) << log.endl;
            log << "m_binary_flag: " << log.boolValue(m_binary_flag) << log.endl;
            log << "m_file_exists: " << log.boolValue(m_file_exists) << log.endl;
            log << "m_can_overwrite: " << log.boolValue(m_can_overwrite) << log.endl;
            log << "m_last_err_code: " << m_last_err_code << log.endl;
            log << "m_last_err_message: " << m_last_err_message << log.endl;
        }
         */

        virtual void start(int32_t flags);
        virtual void startRead() { return start(kRead | kBinary); }
        virtual void startReadAscii() { return start(kRead); }
        virtual void startWrite() { return start(kWrite | kBinary); }
        virtual void startWriteOverwrite() { return start(kWrite | kBinary | kOverwrite); }
        virtual void startWriteAscii() { return start(kWrite); }
        virtual void startWriteAsciiOverwrite() { return start(kWrite | kOverwrite); }
        virtual void startWriteAppend() { return start(kAppend | kBinary); }
        virtual void startWriteAsciiAppend() { return start(kAppend); }
        virtual void startReadWrite() { return start(kRead | kWrite | kBinary); }
        virtual void startReadWriteOverwrite() { return start(kRead | kWrite | kBinary | kOverwrite); }

        static File* file(const String& file_path, int32_t flags) noexcept;

        int64_t _updateFileSize() noexcept;

        int32_t currLineIndex() const noexcept { return m_curr_line_index; }

        bool isBigEndian() const noexcept { return m_big_endian; }
        bool isLittleEndian() const noexcept { return !m_big_endian; }

        void setLittleEndian() noexcept { m_big_endian = false; }
        void setBigEndian(bool bigEndian = true) noexcept { m_big_endian = bigEndian; }

        /**
         * @brief Determines if a given 2-character signature indicates big-endian byte order.
         * @param buffer
         * @return
         */
        bool isBigEndianSignature(const char* buffer) const {
            if (buffer == nullptr) {
                throw ErrorCode::NullData;
            }
            if (buffer[0] == 'I' && buffer[1] == 'I') {
                return false;
            }
            else if (buffer[0] == 'M' && buffer[1] == 'M') {
                return true;
            }
            else {
                throw ErrorCode::UnsupportedEndianess;
            }
        }

        void setEndianBySignature(const char* buffer) {
            if (buffer == nullptr) {
                throw ErrorCode::NullData;
            }
            if (buffer[0] == 'I' && buffer[1] == 'I') {
                m_big_endian = false;
            }
            else if (buffer[0] == 'M' && buffer[1] == 'M') {
                m_big_endian = true;
            }
            else {
                throw ErrorCode::UnsupportedEndianess;
            }
        }

        void checkSignature(const char* buffer, int32_t length, const char* signature) {
            if (std::strncmp(buffer, signature, length) != 0) {
                throw ErrorCode::UnsupportedFileFormat;
            }
        }


        bool canRead() const noexcept {
            return m_read_flag && m_file_stream && m_file_size > 0;
        }

        bool canWrite() const noexcept {
            if (m_write_flag && m_file_stream) {
                return !m_file_exists || m_can_overwrite;
            }
            else {
                return false;
            }
        }

        static void checkBeforeReading(File* file) {
            if (file == nullptr) {
                throw ErrorCode::NullData;
            }
            file->checkBeforeReading();
        }

        static void checkBeforeWriting(File* file) {
            if (file == nullptr) {
                throw ErrorCode::NullData;
            }
            file->checkBeforeWriting();
        }

        void checkStream() const;
        void checkBeforeReading() const;
        void checkBeforeWriting() const;

        String filePath() { return m_file_path; }
        String dirPath() { return m_file_path.fileDirPath(); }
        int64_t size() const { return m_file_size; }
        bool isPosAtEnd() { return pos() >= m_file_size; }
        int64_t bytesLeft() { return m_file_size - pos() - 1; }

        void flush() { m_file_stream.flush(); }

        virtual void close();

        void savePos() {
            _m_pos_stack.push(pos());
        }

        void restorePos() {
            int64_t pos;
            if (_m_pos_stack.pop(&pos)) {
                setPos(pos);
            }
        }

        std::fstream* stream() { return &m_file_stream; }

        int64_t pos();

        void rewind() {
            m_file_stream.clear();
            setPos(0);
            m_curr_line_index = -1;
        }

        void setPos(int64_t pos);
        void skip(int64_t size);
        void backward(int64_t size) { setPos(pos() - size); }

        int64_t skipWhiteSpace();

        bool skipUntilLineWithText(const String& text);


        bool mustSwap() const noexcept;
        bool lastUtf8SymbolIsWhiteSpace() noexcept;
        bool compareLastUtf8Symbol(const char* symbol) noexcept;

        int32_t indent() const noexcept { return m_indent; }
        void setIndent(int32_t value) noexcept {
            m_indent += value;
            if (m_indent < 0)
                m_indent = 0;
        }
        void moveIndentRight() noexcept { setIndent(1); }
        void moveIndentLeft() noexcept { setIndent(-1); }


        bool read(int64_t size, uint8_t* out_data);

        int64_t countLines();
        bool readLine(int64_t size, String& out_line);
        bool readLine(String& out_line) {
            return readLine(std::numeric_limits<int32_t>::max(), out_line);  // Maximum circa 2 GB
        }
        bool readTrimmedLine(String& out_line) {
            bool result = readLine(1000000, out_line);
            out_line.trim();
            return result;
        }

        bool skipLine();

        void _readSwapped(int64_t size, uint8_t* out_data);
        void _readDataType(int64_t type_size, uint8_t* out_data);
        char readChar();
        void readStr(int32_t max_length, char* out_str);
        int32_t readUtf8Symbol(char* out_data = nullptr);
        fourcc_t readFourCC();

        void readToString(String& out_string);
        void readToString(int64_t size, String& out_string);

        template <typename U>
        U readValue();


        void readFix(Fix& fix);

        double readFixPoint8_8();       // Used in QuickTime files
        double readFixPoint16_16();
        double readFixPointU16_16();
        double readFixPoint2_30() ;


        template <typename U>
        void readArray(int64_t length, U* out_array);

        // void readQTMovieMat3(Mat3& out_matrix); TODO: !!!!
        bool readQTMovieAtomType(uint64_t* out_atom_size, fourcc_t* out_atom_type);

        uint32_t readTIFFValue(uint16_t field_type);

        static ErrorCode readToBuffer(const String& file_path, int64_t buffer_size, uint8_t* out_buffer);


        void writeEndianSignature();
        void write8BitData(const uint8_t* data, int64_t length);

        template <typename U>
        void writeData(const U* data, int64_t length);

        void _writeSwapped(const uint8_t* data, int64_t size);
        void _writeDataType(const uint8_t* data, int64_t size);

        void writeChar(char c) {
            m_file_stream.write(&c, 1);
            if (m_file_stream.fail()) {
                throw ErrorCode::FileCantWrite;
            }
        }

        void writeChars(const char* data, int64_t length);
        void writeFixLengthString(const String& string, int32_t length, char* buffer);

        template <typename U>
        void writeValue(const U& value) {
            auto ptr = reinterpret_cast<const uint8_t*>(&value);
            _writeDataType(ptr, sizeof(U));
        }

        void writeFourCC(fourcc_t value);
        void writeBool(bool value);

        void writeFix(const Fix& value);
        void writeStr(const char* str);
        void writeString(const String& string);
        void writeFormatted(const char* format, ...);

        void writeQuotedStr(const char* str) {
            writeQuote();
            writeStr(str);
            writeQuote();
        }

        void writeQuotedString(const String* string) {
            writeQuote();
            if (string != nullptr) {
                writeString(*string);
            }
            writeQuote();
        }

        void writeQuotedString(const String& string) {
            writeQuote();
            writeString(string);
            writeQuote();
        }

        void writeSingleQuotedStr(const char* str) {
            writeSingleQuote();
            writeStr(str);
            writeSingleQuote();
        }

        void writeSingleQuotedString(const String* string) {
            writeSingleQuote();
            if (string != nullptr) {
                writeString(*string);
            }
            writeSingleQuote();
        }

        void writeSingleQuotedString(const String& string) {
            writeSingleQuote();
            writeString(string);
            writeSingleQuote();
        }

        void writeSQLString(const String& string) {
            writeSingleQuote();
            String local_string = string;
            local_string.replace("'", "''");  // Escape single qoutes for SQL
            writeString(local_string);
            writeSingleQuote();
        }

        int32_t writePascalStr(const char* str);

        virtual void writeNewLine() { writeChar('\n'); }

        void writeSpace() { writeChar(' '); }
        void writeTab() { writeChar('\t'); }
        void writeComma() { writeChar(','); }
        void writeColon() { writeChar(':'); }
        void writeQuote() { writeChar('"'); }
        void writeSingleQuote() { writeChar('\''); }
        void writeIndent();
        void writeKey(const char* key);
        void writeLine(const char* key, const char* value);
        void writeLineBool(const char* key, bool value);
        void writeLineInt32(const char* key, int32_t value);
        void writeLineFloat(const char* key, float value, int32_t fractional_digits = 4);
        void writeLineFix(const char* key, const Fix& value);
        void writeLineFormatted(const char* key, const char* format, ...);
        void writeLineStr(const char* key, const char* str);
        void writeLineString(const char* key, const String& string);


        // Write without automatic indentation for hierarchical data structures like classes and arrays

        void writeQoutedText(const String& string);
        void writeTextBool(bool value);
        void writeTextInt32(int32_t value);
        void writeTextUInt32(uint32_t value);
        void writeTextUInt32Hex(uint32_t value);
        void writeTextInt64(int64_t value);
        void writeTextFloat(float value, int32_t fractional_digits = 4);
        void writeTextFloatHex(float value);
        void writeTextFloatAsInt(float value, int32_t f = 1000);
        void writeTextDouble(double value, int32_t fractional_digits = 8);
        void writeTextDoubleHex(double value);
        void writeTextDoubleAsInt(double value, int32_t f = 1000);
        void writeTextFix(const Fix& value);
        void writeTextFlags(Flags flags);

        void writeCurrentDateTime();


        bool hasTiffSignature();
        bool hasDNGSignature();
        bool hasAIFFSignature();
        bool hasAIFCSignature();
        bool hasWAVESignature();
        bool hasQuickTimeSignature();
        bool hasMPEG4Signature();
        bool hasMXFSignature();
        bool hasMP3Signature();
        bool hasMIDISignature();

        bool readTomlKeyValue(String& out_key, String& out_value);

        int32_t base64SizeInfo(int64_t& out_base64_size, int64_t& out_raw_size);

        void base64EncodeBegin();
        void base64EncodeByte(uint64_t byte);
        void base64EncodeEnd();

        ErrorCode readBase64ToString(int64_t base64_size, String& out_string) noexcept;
        uint8_t* readBase64ToBuffer(int64_t base64_size, int64_t buffer_size = 0, uint8_t* buffer = nullptr) noexcept;


        ErrorCode readFile(BaseObject* receiver) noexcept;

        static File* createFile(const String& file_path);

        static bool fileExists(const char* file_path);
        static bool fileExists(const String& file_path);
        static bool fileExists(const String& dir_path, const String& file_name);
        static File::Signature fileSignature(const String& file_path);

        static bool isDir(const String& path) noexcept;
        static bool containsDir(const String& path, const String& dir_name) noexcept;
        static bool makeDirs(const String& path) noexcept;

        static int32_t dirNameList(const String& path, StringList& out_list) noexcept;
        static int32_t fileNameList(const String& path, StringList& out_list) noexcept;
        static int32_t fileNameList(const String& path, const String& extensions, int64_t min_size, int64_t max_size, int32_t* out_ignored, StringList& out_list) noexcept;
        static int32_t countDir(const String& path) noexcept;
        static int32_t countFiles(const String& path) noexcept;


        static ErrorCode fileEntryByPath(const String& file_path, FileEntry& out_file_entry) noexcept;

        static ErrorCode forAllFiles(const String& path, FileEntryAction action, void* ref) noexcept;
        static ErrorCode forAllFilesRecursive(const String& path, FileEntryAction action, void* ref) noexcept;
        ErrorCode writeFileEntriesRecursive(const String& path, FileEntryFilterAction action, void* ref, bool relative_flag) noexcept;

        static String buildFilePath(const String& path, const String& file_name) noexcept;
        static bool findFilePath(const String& file_path, const String& alt_root_dir, String& out_file_path) noexcept;

        static ErrorCode fileUTI(const String& file_path, String& out_uti) noexcept;


        static ErrorCode removeFile(const String& file_path) noexcept { return removeFile(file_path.utf8()); }
        static ErrorCode removeFile(const char* file_path) noexcept;
        static ErrorCode removeDirAll(const String& dir_path) noexcept { return removeDirAll(dir_path.utf8()); }
        static ErrorCode removeDirAll(const char* dir_path) noexcept;

        static void checkCanOverwrite(const String& file_path, CanOverwrite can_overwrite);


        static ErrorCode changeBytes(const String& file_path, int32_t n, const int32_t* pos, const uint8_t* bytes) noexcept;
        static ErrorCode changeBytes(const String& file_path, int32_t pos, int32_t length, const uint8_t* bytes) noexcept;

        static ErrorCode toHex(const String& src_file_path, const String& dst_file_path) noexcept;


        static int32_t execFileAction(const String& dir_path, FileAction action, void* action_ref = nullptr, int32_t max_depth = kFileActionMaxRecursionDeoth, int32_t curr_depth = 0);

        #if defined(__APPLE__) && defined(__MACH__)
            static CFDataRef macos_cfDataRefFromFile(const String& file_path);
            static ErrorCode macos_writeCFPlistXML(const String& file_path, const CFPropertyListRef plist) noexcept;
        #endif

    protected:
        String m_file_path;                 ///< File path as a string
        std::fstream m_file_stream;         ///< File stream for reading/writing
        int64_t m_file_size = 0;            ///< File size in bytes

        bool m_big_endian = false;          ///< Indicates if the file uses big-endian format
        bool m_read_flag = false;           ///< True if the file is opened for reading
        bool m_write_flag = false;          ///< True if the file is opened for writing
        bool m_append_flag = false;         ///< True if the file is opened in append mode
        bool m_binary_flag = false;         ///< True if the file is opened in binary mode
        bool m_file_exists = false;         ///< True if the file exists
        bool m_can_overwrite = false;       ///< True if overwriting is allowed

        ErrorCode m_last_err_code = ErrorCode::None;    ///< Last encountered error code
        String m_last_err_message;          ///< Last encountered error message

        int32_t m_indent = 0;               ///< Indentation level for formatted output

        int32_t _m_utf8_seq_length = 0;     ///< Length of the current UTF-8 sequence
        char _m_utf8_buffer[5]{};           ///< Buffer for UTF-8 character processing

        Base64Data _m_base64_encoder;       ///< Base64 encoder instance
        bool _m_base64_encode_flag = false; ///< True if Base64 encoding is enabled

        char _m_write_buffer[kWriteBufferSize]{}; ///< Buffer for writing formatted strings

        List<int64_t> _m_pos_stack;         ///< Stack for storing file positions
        int32_t m_pos_stack_index = -1;     ///< Index of the current position in the stack
        int32_t m_curr_line_index = -1;
    };


} // End of namespace Grain

#endif // GrainFile_hpp
