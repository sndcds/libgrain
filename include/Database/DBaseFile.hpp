//
//  DBaseFile.hpp
//
//  Created by Roald Christesen on 26.10.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

// TODO: Implement different data types like in method readString().


#ifndef GrainDBaseFile_hpp
#define GrainDBaseFile_hpp


#include "Grain.hpp"
#include "File/File.hpp"
#include "Type/Data.hpp"


namespace Grain {

    enum class DBaseFieldType : uint8_t {
        Undefined = 0,
        String = 'C',       ///< A string of characters, padded with spaces if shorter than the field length
        Integer = 'I',      ///< 32 bit integer, stored as little endian
        ShortInt = 'O',     ///< 16 bit integer, stored as little endian
        Float = 'F',        ///< Floating point number, stored as string, padded with spaces if shorter than the field length
        Numeric = 'N',      ///< Floating point number, stored as string, padded with spaces if shorter than the field length
        Date = 'D',         ///< Date stored as string in YYYYMMDD format
        DateTime = 'T',     ///< A date and time, stored as a number
        Currency = 'Y',     ///< Floating point number, stored as binary in (usually) 8 bytes
        Logical = 'L',      ///< A boolean value, stored as one of YyNnTtFf. May be set to ? if not initialized
        General = 'G',
        Memo = 'M',
        Blob = 'P'
    };


    /**
     *  @brief Standard dBASE Header.
     *
     *  Offsets of this header are the same in all versions of dBASE except dBASE 7.0.
     *
     *  @warning It is recommend not to access GrDBaseHeader directly.
     */
    struct DBaseHeader {
        uint8_t m_version;          ///< Byte: 0, dBase version
        uint8_t m_last_update[3];   ///< Byte: 1-3, date of last update
        uint32_t m_row_count;       ///< Byte: 4-7, number of rows in table
        uint16_t m_header_length;   ///< Byte: 8-9, number of bytes in the header
        uint16_t m_row_length;      ///< Byte: 10-11, number of bytes in each row
        uint8_t m_reserved1[2];     ///< Byte: 12-13, reserved, see specification of dBase databases
        uint8_t m_transaction;      ///< Byte: 14, Flag indicating incomplete transaction
        uint8_t m_encryption;       ///< Byte: 15, Encryption flag
        uint8_t m_reserved2[12];    ///< Byte: 16-27, reserved for dBASE in a multiuser environment
        uint8_t m_mdx;              ///< Byte: 28, Production MDX file flag
        uint8_t m_language;         ///< Byte: 29, Language driver ID, for Visual FoxPro
        uint8_t m_reserved3[2];     ///< Byte: 30-31, reserved, filled with zero
    };


    /**
     *  @brief Field Descriptor Array.
     *
     *  Offsets of this header are the same in all versions of dBASE.
     *
     *  @warning It is recommend not to access GrDBaseFiel directly.
     */
    struct DBaseField {
        char m_name[11];             ///< Byte: 0-10; field name in ASCII
        DBaseFieldType m_type;       ///< Byte: 11; field type in ASCII (C, D, L, M or N)
        uint32_t m_address;          ///< Byte: 12-15; field data address
        uint8_t length_;             ///< Byte: 16; field length in binary
        uint8_t m_decimals;          ///< Byte: 17; field decimal count in binary
        uint8_t m_reserved1[2];      ///< Byte: 18-30; reserved
        uint32_t offs_;
        uint8_t m_reserved2[7];
        uint8_t m_mdx;               ///< Byte: 31; Production MDX field flag
    };


    /**
     *  @brief Memo File Structure (.FPT)
     */


    /**
     *  @brief Memo Header Record.
     */
    struct DBaseMemoHeader {
        uint32_t m_block_address;     ///< 0-3 Location of next free block [1]
        uint8_t m_reserved1[2];       ///< 4-5 Unused
        uint16_t m_block_size;        ///< 6-7 Block size (bytes per block) [1]
        uint16_t m_reserved2[504];    ///< 8-511 Unused
    };


    /**
     *  @brief Memo Block Header and Memo Text.
     */
    struct DBaseMemoBlockTop {
        uint32_t m_signature;        ///< 0-3 Idicates the type of data in the block, 0 = picture, 1 = text
        uint32_t m_block_length;     ///< 4-7 Length [1] of memo (in bytes)
        // 8 - n Memo text (n = length)
    };


    /**
     *  @brief File Reader for dBase files.
     *
     *  @section limitations Limitations
     *
     *  The class does not handle Memo field and files.
     */

    // TODO: Implement functionality to create and write into dbase files.

    class DBaseFile : public File {

    protected:
        uint32_t m_real_filesize;       ///< the pysical size of the file, as stated from filesystem
        uint32_t m_calc_filesize;       ///< the calculated filesize
        DBaseHeader* m_header;          ///< header of .dbf file
        DBaseField* m_columns;          ///< array of field specification
        uint32_t m_column_count;        ///< number of columns
        uint8_t m_integrity[7];         ///< integrity could be: valid, invalid
        int32_t curr_row_index_;        ///< Current Row index
        char errmsg[254];               ///< errorhandler, maximum of 254 characters. // TODO: Replace

        bool m_has_variable_length_fields = false;
        Data* _m_temp_buffer = nullptr;

    public:
        DBaseFile(const String& file_path);
        ~DBaseFile() noexcept;

        const char* className() const noexcept override { return "DBaseFile"; }


        void close() override;

        virtual void start(int32_t flags) override;


        void readAll();

        bool hasVariableLengthFields() const noexcept { return m_has_variable_length_fields; }

        void dbVersionString(int32_t version, class String& out_string) const noexcept;
        void readHeaderInfo();
        void writeHeaderInfo();
        void readColumnsInfo();
        void writeColumnsInfo();

        uint32_t rowCount() const noexcept {
            if (m_header->m_row_count > 0) {
                return m_header->m_row_count;
            }
            else {  // TODO: Error message!
                return -1;
            }
            return 0;
        }

        uint32_t columnCount() const noexcept {
            if (m_header->m_header_length > 0) {
                return ((m_header->m_header_length - sizeof(DBaseHeader) - 1) / sizeof(DBaseField));
            }
            else {  // TODO: Error message!
                return -1;
            }
            return 0;
        }


        bool hasColumn(int32_t column_index) const noexcept {
            return column_index >= 0 && column_index < m_column_count;
        }

        bool hasRow(int32_t row_index) const noexcept {
            return row_index >= 0 && row_index < m_header->m_row_count;
        }

        bool hasField(int32_t row_index, int32_t column_index) const noexcept {
            return hasColumn(column_index) & hasRow(row_index);
        }

        bool columnName(int32_t column_index, String& out_name) const noexcept {
            if (hasColumn(column_index)) {
                return out_name.setByStr(m_columns[column_index].m_name, 10);
            }
            else {
                out_name = "invalid";
                return false;
            }
        }

        int32_t columnLength(int32_t column_index) const noexcept {
            return hasColumn(column_index) ? m_columns[column_index].length_ : -1;
        }

        uint32_t columnOffset(int32_t column_index) const noexcept {
            return hasColumn(column_index) ? m_columns[column_index].offs_ : -1;
        }

        DBaseFieldType columnType(int32_t column_index) const noexcept {
            return hasColumn(column_index) ? m_columns[column_index].m_type : DBaseFieldType::Undefined;
        }

        uint8_t columnDecimals(int32_t column_index) const noexcept {
            return hasColumn(column_index) ? m_columns[column_index].m_decimals : -1;
        }

        uint32_t columnAddress(int32_t column_index) const noexcept {
            return hasColumn(column_index) ? m_columns[column_index].m_address : -1;
        }

        /* TODO: Check!
        void setField(DBaseField* field, DBaseFieldType type, const char* name, uint8_t len, uint8_t dec) noexcept {
            memset(field, 0, sizeof(DBaseField));
            field->mType = type;
            strncpy(field->mName, name, 11);
            field->mLength = len;
            field->mDecimals = dec;
        }
         */

        const char* dateStr() const noexcept {
            static char date[10];
            if (m_header->m_last_update[0]) {
                std::snprintf(date, 10, "%d-%02d-%02d",
                              1900 + m_header->m_last_update[0],
                              m_header->m_last_update[1],
                              m_header->m_last_update[2]);
                return date;
            }
            else {
                return "";
            }
        }

        int32_t headerSize() const noexcept {
            if (m_header->m_header_length > 0) {
                return m_header->m_header_length;
            }
            else {
                // TODO: Error message!
                return -1;
            }
        }

        int32_t rowLength() const noexcept {
            if (m_header->m_row_length > 0) {
                return m_header->m_row_length;
            }
            else {
                // TODO: Error message!
                return -1;
            }
        }

        int32_t version() {
            if (m_header->m_version == 0 ) {
                // TODO: Error message!
                return -1;
            }
            else {
                return m_header->m_version;
            }
        }

        int32_t isMemo() {
            if (m_header->m_version == 0 ) {
                // TODO: Error message!
                return -1;
            }
            else {
                return (m_header->m_version & 128) == 128 ? 1 : 0;
            }
        }

        int32_t setRowIndex(int32_t row_index) {
            if (row_index >= static_cast<int32_t>(m_header->m_row_count)) {
                return -1;
            }
            else if (row_index < 0) {
                if (m_header->m_row_count + row_index < 0) {
                    return -2;
                }
                else {
                    curr_row_index_ = static_cast<int32_t>(m_header->m_row_count) + row_index;
                }
            }
            else {
                curr_row_index_ = row_index;
            }
            return curr_row_index_;
        }

        int32_t readField(int32_t row_index, int32_t column_index, void* out_data) {
            // TODO: check! column_index has to be used!

            if (row_index < 0 || row_index >= m_header->m_row_count) {
                return -1;
            }

            if (column_index < 0 || column_index >= columnCount()) {
                return -2;
            }

            setPos(_rowFilePos(row_index));
            read(m_header->m_row_length, (uint8_t*)out_data);

            return row_index;
        }

        int32_t readRecord(int32_t row_index, void* out_data) {
            if (row_index < 0 || row_index >= m_header->m_row_count) {
                return -1;
            }

            setPos(_rowFilePos(row_index));
            read(m_header->m_row_length, (uint8_t*)out_data);

            return row_index;
        }

        int32_t readNextRecord(void* out_data) {
            if (curr_row_index_ >= m_header->m_row_count) {
                return -1;
            }

            setPos(_rowFilePos(curr_row_index_));
            read(m_header->m_row_length, (uint8_t*)out_data);
            curr_row_index_++;

            return curr_row_index_ - 1;
        }

        int32_t readString(int32_t row_index, int32_t column_index, String& out_string) {
            if (hasField(row_index, column_index)) {
                auto pos = _fieldFilePos(row_index, column_index);
                auto length = columnLength(column_index);
                setPos(pos);
                if (_m_temp_buffer->checkCapacity(length) == nullptr) {
                    throw ErrorCode::MemCantAllocate;
                }
                read(length, _m_temp_buffer->data());
                out_string.setByData(_m_temp_buffer, length);
                return static_cast<int32_t>(out_string.length());
            }

            return 0;
        }

        /* TODO: Implement!
         int32_t appendRecord(char* record, int32_t len) {
        }
         */

        int64_t _rowFilePos(int32_t row_index) {
            return (int64_t)row_index * m_header->m_row_length + m_header->m_header_length;
        }

        int64_t _fieldFilePos(int32_t row_index, int32_t column_index) {
            return _rowFilePos(row_index) + m_columns[column_index].offs_;
        }
    };


} // End of namespace Grain

#endif // GrainDBaseFile_hpp
