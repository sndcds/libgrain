//
//  DBaseFile.cpp
//
//  Created by Roald Christesen on 26.10.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#include "Database/DBaseFile.hpp"


namespace Grain {

    DBaseFile::DBaseFile(const String& file_path) : File(file_path) {
    }


    DBaseFile::~DBaseFile() noexcept {
        delete _m_temp_buffer;
    }


    void DBaseFile::close() {
        if (m_header != nullptr) {
            std::free(m_header);
        }

        if (m_columns != nullptr) {
            std::free(m_columns);
        }

        File::close();
    }


    void DBaseFile::start(int32_t flags) {
        auto result = ErrorCode::None;

        try {
            File::start(flags);

            m_header = (DBaseHeader*)std::malloc(sizeof(DBaseHeader));
            if (!m_header) {
                throw ErrorCode::MemCantAllocate;
            }

            m_columns = nullptr;
            _m_temp_buffer = new Data(1024);

            setLittleEndian();
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        Exception::throwStandard(result);
    }


    void DBaseFile::readAll() {
        readHeaderInfo();
        readColumnsInfo();
        curr_row_index_ = 0;
    }


    void DBaseFile::dbVersionString(int32_t version, String& out_string) const noexcept {
        switch (version) {
            case 0x02:  // Without memo fields.
                out_string = "FoxBase";
                break;
            case 0x03:  // Without memo fields.
                out_string = "FoxBase+/dBASE III+";
                break;
            case 0x04:  // Without memo fields.
                out_string = "dBASE IV";
                break;
            case 0x05:  // Without memo fields.
                out_string = "dBASE 5.0";
                break;
            case 0x83:
                out_string = "FoxBase+/dBASE III+";
                break;
            case 0x8B:
                out_string = "dBASE IV";
                break;
            case 0x30:  // Without memo fields.
                out_string = "Visual FoxPro";
                break;
            case 0xF5:  // Without memo fields.
                out_string = "FoxPro 2.0";
                break;

            default:
                out_string.setFormatted(100, "Unknown, code 0x%.2X", version);
        }
    }


    void DBaseFile::readHeaderInfo() {
        m_header->m_version = readValue<uint8_t>();
        read(3, m_header->m_last_update);
        m_header->m_row_count = readValue<int32_t>();
        m_header->m_header_length = readValue<int16_t>();
        m_header->m_row_length = readValue<int16_t>();
        read(2, m_header->m_reserved1);
        m_header->m_transaction = readValue<uint8_t>();
        m_header->m_encryption = readValue<uint8_t>();
        read(12, m_header->m_reserved2);
        m_header->m_mdx = readValue<uint8_t>();
        m_header->m_language = readValue<uint8_t>();
        read(2, m_header->m_reserved3);
    }


    void DBaseFile::writeHeaderInfo() {
        #pragma message("DBaseFile::writeHeaderInfo() must be implemented.")
        /*
        time_t ps_calendar_time;
        struct tm* ps_local_tm;

        DB_HEADER* newheader = malloc(sizeof(DB_HEADER));
        if(NULL == newheader) {
            return -1;
        }
        memcpy(newheader, header, sizeof(DB_HEADER));

        ps_calendar_time = time(NULL);
        if(ps_calendar_time != (time_t)(-1)) {
            ps_local_tm = localtime(&ps_calendar_time);
            newheader->last_update[0] = ps_local_tm->tm_year;
            newheader->last_update[1] = ps_local_tm->tm_mon+1;
            newheader->last_update[2] = ps_local_tm->tm_mday;
        }

        newheader->header_length = rotate2b(newheader->header_length);
        newheader->record_length = rotate2b(newheader->record_length);
        newheader->records = rotate4b(newheader->records);

        // Make sure the header is written at the beginning of the file
        // because this function is also called after each record has been written.
        lseek(p_dbf->dbf_fh, 0, SEEK_SET);
        if ((write( p_dbf->dbf_fh, newheader, sizeof(DB_HEADER))) == -1 ) {
            std::free(newheader);
            return -1;
        }
        std::free(newheader);
        return 0;
        */
    }


    void DBaseFile::readColumnsInfo() {
        m_column_count = columnCount();
        m_has_variable_length_fields = false;

        m_columns = (DBaseField*)std::malloc(sizeof(DBaseField) * m_column_count);
        if (!m_columns) {
            throw ErrorCode::MemCantAllocate;
        }

        setPos(sizeof(DBaseHeader));

        for (int32_t i = 0; i < m_column_count; i++) {
            auto field = &m_columns[i];

            read(11, (uint8_t*)field->m_name);

            field->m_type = (DBaseFieldType)readValue<uint8_t>();
            field->m_address = readValue<uint32_t>();
            field->length_ = readValue<uint8_t>();
            if (field->length_ == 0) {
                m_has_variable_length_fields = true;
            }

            field->m_decimals = readValue<uint8_t>();
            read(2, field->m_reserved1);
            field->offs_ = readValue<uint32_t>();
            read(7, field->m_reserved1);
            field->m_mdx = readValue<uint8_t>();

            field++;
        }

        // Update field offsets
        int32_t offset = 1; // Skip first byte which indicates whether a record is deleted or not

        for (uint32_t i = 0; i < m_column_count; i++) {
            m_columns[i].offs_ = offset;
            offset += m_columns[i].length_;
        }
    }


    void DBaseFile::writeColumnsInfo() {
        #pragma message("DBaseFile::writeColumnsInfo() must be implemented.")
        /*
            lseek(p_dbf->dbf_fh, sizeof(DB_HEADER), SEEK_SET);

            if ((write( p_dbf->dbf_fh, fields, numfields * sizeof(DB_FIELD))) == -1 ) {
                perror(_("In function dbf_WriteFieldInfo(): "));
                return -1;
            }

            write(p_dbf->dbf_fh, "\r\0", 2);
         */
    }


} // End of namespace Grain
