//
//  TiffFile.hpp
//
//  Created by Roald Christesen on 18.11.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "File/TiffFile.hpp"
#include "Image/Image.hpp"
#include "Core/Log.hpp"


namespace Grain {

    TiffFile::TiffFile(const String& file_path) noexcept : File(file_path) {
    }


    TiffFile::~TiffFile() noexcept {
    }


    ErrorCode TiffFile::writeImage(const Image* image, DataType data_type) noexcept {
        auto result = ErrorCode::None;

        try {
            if (!image) {
                throw ErrorCode::NullData;
            }

            uint16_t sample_format = SampleFormat_Undefined;
            uint16_t bits_per_sample = 0;

            m_component_count = image->componentCount();
            m_used_component_count = m_component_count;
            if (image->hasAlpha() && m_drop_alpha) {
                m_used_component_count--;
            }

            switch (data_type) {
                case DataType::Float:
                    bits_per_sample = 32;
                    sample_format = SampleFormat_IEEEFP;
                    break;
                case DataType::UInt8:
                    bits_per_sample = 8;
                    sample_format = SampleFormat_UInt;
                    break;
                case DataType::UInt16:
                    bits_per_sample = 16;
                    sample_format = SampleFormat_UInt;
                    break;
                case DataType::UInt32:
                    bits_per_sample = 32;
                    sample_format = SampleFormat_UInt;
                    break;
                default:
                    break;
            }

            if (data_type == DataType::Undefined) {
                if (image->isFloat()) {
                    switch (image->bitsPerComponent()) {
                        case 32:
                            bits_per_sample = 32;
                            data_type = DataType::Float;
                            sample_format = SampleFormat_IEEEFP;
                            break;
                    }
                }
                else {
                    switch (image->bitsPerComponent()) {
                        case 8:
                            bits_per_sample = 8;
                            data_type = DataType::UInt8;
                            sample_format = SampleFormat_UInt;
                            break;
                        case 16:
                            bits_per_sample = 16;
                            data_type = DataType::UInt16;
                            sample_format = SampleFormat_UInt;
                            break;
                        case 32:
                            bits_per_sample = 32;
                            data_type = DataType::UInt32;
                            sample_format = SampleFormat_UInt;
                            break;
                    }
                }
            }

            if (sample_format == SampleFormat_Undefined) {
                throw ErrorCode::UnsupportedDataType;
            }

            int32_t photometric = -1;
            switch (image->colorModel()) {
                case Color::Model::RGB:
                case Color::Model::RGBA:
                    photometric = Photometric_RGB;
                    break;
                case Color::Model::Lumina:
                case Color::Model::LuminaAlpha:
                    photometric = Photometric_MinIsBlack;
                    break;
                default:
                    throw ErrorCode::UnsupportedColorModel;
            }

            checkBeforeWriting();
            setBigEndian();


            // Create tempfiles for data and geoData
            // TODO: Use internal buffer instead of tempory file!

            String random_name;
            random_name.randomName(8);
            m_temp_data_file_path = m_file_path.fileDirPath() + "/_" + random_name + ".dat";
            m_temp_data_file = new (std::nothrow) File(m_temp_data_file_path);
            if (!m_temp_data_file) {
                throw ErrorCode::ClassInstantiationFailed;
            }
            m_temp_data_file->startReadWriteOverwrite();
            m_temp_data_file->setBigEndian(isBigEndian()); // Use same endianess!


            m_temp_geo_data_file_path = m_file_path.fileDirPath() + "/_" + random_name + "_geo.dat";
            m_temp_geo_data_file = new (std::nothrow) File(m_temp_geo_data_file_path);
            if (!m_temp_geo_data_file) {
                throw ErrorCode::ClassInstantiationFailed;
            }
            m_temp_geo_data_file->startReadWriteOverwrite();
            m_temp_geo_data_file->setBigEndian(isBigEndian()); // Use same endianess!

            // Write header
            if (m_big_endian) {
                char buffer[4] = { 'M', 'M', 0, 42 };
                writeChars(buffer, 4);
            }
            else {
                char buffer[4] = { 'I', 'I', 42, 0 };
                writeChars(buffer, 4);
            }

            writeValue<uint32_t>(8);

            // Prepare and write entries
            uint32_t samples_per_pixel = image->componentCount();
            if (image->hasAlpha() && m_drop_alpha) {
                samples_per_pixel--;
            }

            prepareEntry(TiffTag::ImageWidth, TiffType::Long, 1, image->width());
            prepareEntry(TiffTag::ImageHeight, TiffType::Long, 1, image->height());
            prepareEntry(TiffTag::BitsPerSample, TiffType::Short, 1, bits_per_sample);
            prepareEntry(TiffTag::RowsPerStrip, TiffType::Long, 1, image->height());
            prepareEntry(TiffTag::StripByteCounts, TiffType::Long, 1, static_cast<uint32_t>(image->memSize()));

            prepareEntry(TiffTag::SamplesPerPixel, TiffType::Short, 1, samples_per_pixel);
            prepareEntry(TiffTag::SampleFormat, TiffType::Short, 1, sample_format);
            prepareEntry(TiffTag::PhotometricInterpretation, TiffType::Short, 1, photometric);
            prepareEntry(TiffTag::PlanarConfig, TiffType::Short, 1, PlanarConfig_Contig);
            prepareEntry(TiffTag::StripOffsets, TiffType::Long, 1, 0);

            if (image->isGeoTiffMode()) {
                m_min_sample_values[0] = image->minSampleValue();
                m_max_sample_values[0] = image->maxSampleValue();

                prepareEntry(TiffTag::SMinSampleValue, TiffType::Double, m_used_component_count, 0,
                             m_temp_data_file->pos());
                m_temp_data_file->writeData<double>(m_min_sample_values, m_used_component_count);

                prepareEntry(TiffTag::SMaxSampleValue, TiffType::Double, m_used_component_count, 0,
                             m_temp_data_file->pos());
                m_temp_data_file->writeData<double>(m_max_sample_values, m_used_component_count);
            }

            // Geo Tiff
            if (image->isGeoTiffMode()) {
                prepareGeoEntry(GeoTiffKey::GTModelTypeGeoKey, 0, 1, TiffFile::GeoModelTypeProjected);
                prepareGeoEntry(GeoTiffKey::GTRasterTypeGeoKey, 0, 1, TiffFile::GeoRasterPixelIsArea);

                // Note! If GTModelTypeGeoKey != TiffFile::GeoModelTypeProjected, then GeogAngularUnitsGeoKey must be set
                // prepareGeoEntry(GeoTiffKey::GeogAngularUnitsGeoKey, 0, 1, TiffFile::GeoAngular_Degree);

                prepareGeoEntry(GeoTiffKey::ProjectedCSTypeGeoKey, 0, 1, image->geoSrid());

                if (m_geo_ascii_string.length() > 0) {
                    prepareEntry(TiffTag::GeoAsciiParams, TiffType::Ascii, m_geo_ascii_string.length(), 0);
                    m_temp_data_file->writeData<uint8_t>(reinterpret_cast<const uint8_t*>(m_geo_ascii_string.utf8()), m_geo_ascii_string.byteLength());
                }

                // Tie points, which comes with image
                for (int32_t tie_point_index = 0; tie_point_index < image->tiePointCount(); tie_point_index++) {
                    Vec3d raster_pos, model_pos;
                    image->tiePoint(tie_point_index, raster_pos, model_pos);
                    addGeoTiePoint(raster_pos, model_pos);
                }

                if (m_geo_tie_points.size() > 0) {
                    prepareEntry(TiffTag::GeoModelTiepoint, TiffType::Double, static_cast<uint32_t>(m_geo_tie_points.size() * 6), 0, m_temp_data_file->pos());
                    for (auto& tie_point : m_geo_tie_points) {
                        tie_point.m_raster_pos.writeToFile(*m_temp_data_file);
                        tie_point.m_model_pos.writeToFile(*m_temp_data_file);
                    }
                }

                prepareEntry(TiffTag::GeoModelPixelScale, TiffType::Double, 3, 0, m_temp_data_file->pos());
                m_geo_pixel_scale.writeToFile(*m_temp_data_file);

                if (m_geo_double_param_count > 0) {
                    prepareEntry(TiffTag::GeoDoubleParams, TiffType::Double, m_geo_double_param_count, 0, m_temp_data_file->pos());
                    for (int32_t i = 0; i < m_geo_double_param_count; i++) {
                        m_temp_data_file->writeValue<double>(0);
                    }
                }

                {
                    const char* buffer = "-999999";
                    prepareEntry(TiffTag::GDAL_NoData, TiffType::Ascii, static_cast<uint32_t>(strlen(buffer)) + 1, 0, m_temp_data_file->pos());
                    m_temp_data_file->writeData<uint8_t>(reinterpret_cast<const uint8_t*>(buffer), 8);
                }

                // GeoDirectory must allways be the last call
                if (m_geo_entry_preparations.size() > 0) {
                    uint32_t key_directory_size = static_cast<uint32_t>(m_geo_entry_preparations.size() * 4 + 4); // 4 shorts per entry + 4 short for header
                    prepareEntry(TiffTag::GeoDirectory, TiffType::Short, key_directory_size, 0);
                }
            }

            m_data_size = m_temp_data_file->pos();
            m_geo_data_size = m_temp_geo_data_file->pos();

            sortPreparedEntries();
            writePreparedEntries();
            sortPreparedGeoEntries();
            writePreparedGeoEntries();

            writeImageData(image, data_type);

            updateEntryData();

            m_temp_data_file->close();
            m_temp_geo_data_file->close();

            close();
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (const std::exception& e) {
            result = ErrorCode::StdCppException;
        }


        // Remove temporary files
        if (m_temp_data_file != nullptr) {
            // File::removeFile(m_temp_data_file_path);
            delete m_temp_data_file;
        }

        if (m_temp_geo_data_file != nullptr) {
            // File::removeFile(m_temp_geo_data_file_path);
            delete m_temp_geo_data_file;
        }

        return result;
    }


    void TiffFile::writeTag(TiffTag tag) {
        writeValue<uint16_t>(static_cast<uint16_t>(tag));
    }


    int32_t TiffFile::bytesForType(TiffType type) noexcept {
        switch (type) {
            case TiffType::Byte:
            case TiffType::Ascii:
            case TiffType::SByte:
            case TiffType::Undefine:
                return 1;
            case TiffType::Short:
            case TiffType::SShort:
                return 2;
            case TiffType::Long:
            case TiffType::SLong:
            case TiffType::Float:
                return 4;
            case TiffType::Rational:
            case TiffType::SRational:
            case TiffType::Double:
            case TiffType::Long8:
            case TiffType::SLong8:
            case TiffType::IFD8:
                return 8;
        }
        return 0;
    }


    void TiffFile::writeEntry(TiffEntryPreparation& entry_preparation) {
        auto e = &entry_preparation.m_entry;
        writeValue<uint16_t>(static_cast<uint16_t>(e->m_tag));
        writeValue<uint16_t>(static_cast<uint16_t>(e->m_type));
        writeValue<uint32_t>(e->m_count);

        if (entry_preparation.m_temp_file_pos < 0) {
            switch (bytesForType(e->m_type)) {
                case 1:
                    writeValue<uint32_t>(e->m_offset << 24);
                    break;
                case 2:
                    writeValue<uint32_t>(e->m_offset << 16);
                    break;
                case 4:
                    writeValue<uint32_t>(e->m_offset);
                    break;
            }
        }
        else {
            writeValue<uint32_t>(e->m_offset);
        }
    }


    void TiffFile::writeGeoEntry(GeoTiffEntry& entry) {
        writeValue<uint16_t>(static_cast<uint16_t>(entry.m_key));
        writeValue<uint16_t>(static_cast<uint16_t>(entry.m_location));
        writeValue<uint16_t>(entry.m_count);
        writeValue<uint16_t>(entry.m_offset);

        // TODO: Implement offset
    }


    void TiffFile::prepareEntry(TiffTag tag, TiffType type, uint32_t count, uint32_t value, int64_t temp_file_pos) {
        TiffEntryPreparation ep;
        ep.m_entry.m_tag = tag;
        ep.m_entry.m_type = type;
        ep.m_entry.m_count = count;
        ep.m_temp_file_pos = temp_file_pos;

        int64_t data_size = bytesForType(type) * count;

        if (data_size <= 4) {
            ep.m_entry.m_offset = value;
            ep.m_data_size = 0;
        }
        else {
            ep.m_entry.m_offset = 0;  // Not set.
            ep.m_data_size = data_size;
        }

        m_entry_preparations.push_back(ep);
    }


    void TiffFile::prepareGeoEntry(GeoTiffKey key, uint16_t location, uint16_t count, uint16_t offset, int64_t temp_file_pos) {
        GeoTiffEntryPreparation gep;
        gep.m_entry.m_key = key;
        gep.m_entry.m_location = location;
        gep.m_entry.m_count = count;
        gep.m_temp_file_pos = temp_file_pos;

        int64_t data_size = geoKeyBytes(key) * count;

        if (location == 0) {
            gep.m_entry.m_offset = offset;
            gep.m_pos_in_file = -1; // Not used
            gep.m_data_size = 0;
        }
        else {
            gep.m_entry.m_offset = 0;  // Not set
            gep.m_pos_in_file = 0; // Not set
            gep.m_data_size = data_size;
            m_data_size += data_size;
        }

        m_geo_entry_preparations.push_back(gep);
    }


    void TiffFile::sortPreparedEntries() {
        std::sort(m_entry_preparations.begin(), m_entry_preparations.end(), TiffEntryPreparation::tagComparator);
    }


    void TiffFile::sortPreparedGeoEntries() {
        std::sort(m_geo_entry_preparations.begin(), m_geo_entry_preparations.end(), GeoTiffEntryPreparation::tagComparator);
    }


    void TiffFile::writePreparedEntries() {
        uint16_t entry_count = m_entry_preparations.size();

        // Write IFD entry count
        writeValue<uint16_t>(entry_count);

        int64_t data_offset = HeaderSize + IFDEntryCountSize + m_entry_preparations.size() * IFDEntrySize + NextIFDPosSize;

        // Write entries
        for (auto& ep : m_entry_preparations) {
            ep.m_pos_in_file = pos();
            if (ep.m_temp_file_pos >= 0) {
                ep.m_entry.m_offset = static_cast<uint32_t>(ep.m_temp_file_pos + data_offset);
            }

            writeEntry(ep);
        }

        // Next IFD position
        writeValue<uint32_t>(0); // No other IFD

        // Write data
        m_temp_data_file->setPos(0);
        for (int64_t i = 0; i < m_data_size; i++) {
            writeValue<uint8_t>(m_temp_data_file->readValue<uint8_t>());
        }

        // Write Geo keys
        // TODO: !
    }


    void TiffFile::writePreparedGeoEntries() {
        uint16_t entry_count = m_geo_entry_preparations.size();
        if (entry_count > 0) {
            m_geo_directory_pos = pos(); // Save for later usage

            // Write GeoKeyDirectory header
            writeValue<uint16_t>(m_geo_key_directory_version);
            writeValue<uint16_t>(m_geo_key_revision);
            writeValue<uint16_t>(m_geo_minor_revision);
            writeValue<uint16_t>(entry_count); // Number of entries

            // Write entries
            for (auto& gep : m_geo_entry_preparations) {
                gep.m_pos_in_file = pos();
                writeGeoEntry(gep.m_entry);
            }
        }
    }


    void TiffFile::updateEntryData() {
        for (auto& ep : m_entry_preparations) {
            switch (ep.m_entry.m_tag) {
                case TiffTag::StripOffsets:
                    // Supports only images with a single strip
                    setPos(ep.m_pos_in_file + 8);
                    writeValue<uint32_t>(static_cast<uint32_t>(m_pixel_data_pos));
                    break;

                case TiffTag::GeoDirectory:
                    setPos(ep.m_pos_in_file + 8);
                    writeValue<uint32_t>(static_cast<uint32_t>(m_geo_directory_pos));
                    break;

                default:
                    break;
            }
        }
    }


    void TiffFile::writeImageData(const Image* image, DataType data_type) {
        if (image) {
            m_pixel_data_pos = pos(); // Save for later usage

            // Write data by converting type
            float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            ImageAccess ia((Image*)image, pixel);
            ia.setPos(0, 0);

            if (data_type == DataType::Float) {
                while (ia.stepY()) {
                    while (ia.stepX()) {
                        ia.read();
                        for (int32_t i = 0; i < m_used_component_count; i++) {
                            writeValue<float>(pixel[i]);
                        }
                    }
                }
            }
            else if (data_type == DataType::UInt8) {
                float max_value = static_cast<float>(std::numeric_limits<uint8_t>::max());
                while (ia.stepY()) {
                    while (ia.stepX()) {
                        ia.read();
                        for (int32_t i = 0; i < m_used_component_count; i++) {
                            writeValue<uint8_t>(static_cast<uint8_t>(round(pixel[i] * max_value)));
                        }
                    }
                }
            }
            else if (data_type == DataType::UInt16) {
                float max_value = static_cast<float>(std::numeric_limits<uint16_t>::max());
                while (ia.stepY()) {
                    while (ia.stepX()) {
                        ia.read();
                        for (int32_t i = 0; i < m_used_component_count; i++) {
                            writeValue<uint16_t>(static_cast<uint16_t>(round(pixel[i] * max_value)));
                        }
                    }
                }
            }
            else if (data_type == DataType::UInt32) {
                float max_value = static_cast<float>(std::numeric_limits<uint32_t>::max());
                while (ia.stepY()) {
                    while (ia.stepX()) {
                        ia.read();
                        for (int32_t i = 0; i < m_used_component_count; i++) {
                            writeValue<uint32_t>(static_cast<uint32_t>(round(pixel[i] * max_value)));
                        }
                    }
                }
            }
        }
    }


    void TiffFile::writeEntryData(TiffTag tag, const void* data, int32_t length) {
        if (data != nullptr) {
            for (auto& ep : m_entry_preparations) {
                if (ep.m_entry.m_tag == tag) {
                    savePos();
                    setPos(ep.m_pos_in_file + 8);
                    int64_t byte_count = static_cast<int64_t>(bytesForType(ep.m_entry.m_type)) * length;
                    writeData<uint8_t>(reinterpret_cast<const uint8_t*>(data), byte_count);
                    restorePos();
                }
            }
        }
    }


    void TiffFile::writeTiffDoubles(int32_t n, const double* data) {
        if (data != nullptr) {
            auto p = data;
            while (n > 0) {
                writeValue<double>(*p++);
                n--;
            }
        }
    }


    void TiffFile::addGeoTiePoint(const Vec3d& raster_pos, const Vec3d& model_pos) {
        GeoTiffTiePoint tie_point;
        tie_point.m_raster_pos = raster_pos;
        tie_point.m_model_pos = model_pos;

        m_geo_tie_points.push_back(tie_point);
    }


    void TiffFile::addGeoAscii(const char* str) {
        if (str != nullptr) {
            m_geo_ascii_string.append(str);
            m_geo_ascii_string.appendChar('|');
        }
    }


    TiffFileValidator::TiffFileValidator(const String& file_path) noexcept : File(file_path) {
        startRead();
    }


    TiffFileValidator::~TiffFileValidator() noexcept {
    }


    void TiffFileValidator::validate(Log& log) noexcept {
        try {
            checkBeforeReading();

            uint8_t buffer[100];
            readStr(4, (char*)buffer);

            if (buffer[0] == 'I' && buffer[1] == 'I' && buffer[2] == 42 && buffer[3] == 0) {
                setLittleEndian();
                std::cout << "File has little endian byte order (Intel).\n";
            }
            else if (buffer[0] == 'M' && buffer[1] == 'M' && buffer[2] == 0 && buffer[3] == 42) {
                setBigEndian();
                std::cout << "File has big endian byte order (Intel).\n";
            }
            else {
                std::cout << "Error in TIFF file header.\n";
                throw Error::specific(0);
            }

            int64_t ifd_pos = readValue<uint32_t>();
            while (ifd_pos >= 8) {
                ifd_pos = validateIFD(ifd_pos, log);
            }
        }
        catch (ErrorCode err) {
            // TODO: !
        }
    }


    int64_t TiffFileValidator::validateIFD(int64_t file_pos, Log& log) {
        int64_t next_ifd_pos = -1;

        m_ifd_count++;

        log << "IFD (" << m_ifd_count << ") at file position: " << file_pos << log.endl;
        log++;

        setPos(file_pos);

        uint16_t entry_count = readValue<uint16_t>();
        log << "IFD entries: " << entry_count << log.endl;
        log++;

        for (uint16_t i = 0; i < entry_count; i++) {
            log << "Entry (" << i + 1 << ") at file position: " << pos() << log.endl;
            log++;

            TiffTag tag = (TiffTag)readValue<uint16_t>();
            TiffType type = (TiffType)readValue<uint16_t>();
            uint32_t count = readValue<uint32_t>();

            int32_t byte_count = TiffFile::bytesForType(type);
            int32_t data_size = count * byte_count;
            uint32_t offset = 0;
            if (data_size <= 4) {
                if (byte_count == 1) {
                    offset = readValue<uint8_t>();
                    skip(3);
                }
                else if (byte_count == 2) {
                    offset = readValue<uint16_t>();
                    skip(2);
                }
                else if (byte_count >= 4) {
                    offset = readValue<uint32_t>();
                }
            }
            else {
                offset = readValue<uint32_t>();
            }

            log << "Tag: " << TiffFile::tagName(tag) << log.endl;
            log << "Type: " << TiffFile::typeName(type) << log.endl;
            log << "byte_count: " << byte_count << log.endl;
            log << "count: " << count << log.endl;
            log << "data_size: " << data_size << log.endl;
            log << "offset: " << offset << log.endl;

            /* Not validating or consuming out-of-line data here
            if (data_size > 4) {
                savePos();
                setPos(offset);
                if ((TiffType)type == TiffType::Double) {
                    skip(count * sizeof(double));
                }
                else if ((TiffType)type == TiffType::Ascii) {
                    skip(count);
                }
                restorePos();
            }
            */

            if (tag == TiffTag::GeoDirectory) {
                m_geo_directory_pos = offset;
            }

            log--;
        }

        log--;

        if (m_geo_directory_pos > 0) {
            savePos();
            validateGeo(m_geo_directory_pos, log);
            restorePos();
        }

        next_ifd_pos = readValue<uint32_t>();
        log << "Next IFD at file position: " << next_ifd_pos << log.endl;
        log--;

        return next_ifd_pos;
    }


    int64_t TiffFileValidator::validateGeo(int64_t file_pos, Log& log) {
        log << "GeoTIFF directory at file position: " << file_pos << log.endl;
        log++;

        setPos(file_pos);

        uint16_t key_directory_version = readValue<uint16_t>();
        uint16_t key_revision = readValue<uint16_t>();
        uint16_t minor_revision = readValue<uint16_t>();
        uint16_t keys_count = readValue<uint16_t>();

        log << "key_directory_version: " << key_directory_version << log.endl;
        log << "key_revision: " << key_revision << log.endl;
        log << "minor_revision: " << minor_revision << log.endl;
        log << "keys_count: " << keys_count << log.endl;

        for (uint16_t i = 0; i < keys_count; i++) {
            uint16_t key_id = readValue<uint16_t>();
            uint16_t tiff_tag_location = readValue<uint16_t>();
            uint16_t count = readValue<uint16_t>();
            uint16_t value_offset = readValue<uint16_t>();

            log << "Key (" << i + 1 << ") at file position: " << pos() << log.endl;
            log++;
            log << "key_id: " << key_id << " - " << TiffFile::geoKeyName(key_id) << log.endl;
            log << "tiff_tag_location: " << tiff_tag_location << log.endl;
            log << "count: " << count << log.endl;
            log << "value_offset: " << value_offset << log.endl;
            log--;
        }

        log--;

        return pos();
    }


} // End of namespace Grain
