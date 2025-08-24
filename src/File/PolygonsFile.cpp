//
//  PolygonsFile.cpp
//
//  Created by Roald Christesen on 10.06.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#include "File/PolygonsFile.hpp"


namespace Grain {

    PolygonsFile::PolygonsFile(const String& file_path) noexcept : File(file_path) {
    }


    PolygonsFile::~PolygonsFile() noexcept {
    }


    ErrorCode PolygonsFile::readInfo() noexcept {
        auto result = ErrorCode::None;

        try {
            char buffer[4];

            startRead();

            // Check the header
            readStr(4, buffer);
            checkSignature(buffer, 4, "PLGN");

            // Check endianess
            readStr(2, buffer);
            setEndianBySignature(buffer);

            m_polygon_count = readValue<uint32_t>();
            if (m_polygon_count < 1) {
                Exception::throwSpecific(kErrNoPolygonsInFile);
            }

            if (!m_polygon_entries.reserve(m_polygon_count)) {
                Exception::throwStandard(ErrorCode::MemCantAllocate);
            }

            // Read bounding box
            m_bounding_box.readFromFile(*this);
            std::cout << "... m_bounding_box ... " << m_bounding_box << std::endl;

            // Read CRS
            m_srid = readValue<int64_t>();

            // Read all Polygon entries
            for (int32_t i = 0; i < m_polygon_count; i++) {
                PolygonsFileEntry entry;
                entry.m_file_pos = readValue<int64_t>();
                entry.m_bounding_box.readFromFile(*this);
                entry.m_part_count = readValue<int32_t>();
                entry.m_point_count = readValue<int32_t>();
                m_polygon_entries.push(&entry);
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }

        return result;
    }


} // End of namespace Grain
