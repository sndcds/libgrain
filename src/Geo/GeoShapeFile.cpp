//
//  GeoShapeFile.cpp
//
//  Created by Roald Christesen on from 05.07.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#include "Geo/GeoShapeFile.hpp"
#include "Geo/GeoShape.hpp"
#include "Geo/Geo.hpp"
#include "Geo/GeoProj.hpp"


namespace Grain {

    GeoShapeFile::GeoShapeFile(const String& file_path) : File(file_path) {
    }


    GeoShapeFile::~GeoShapeFile() noexcept {
    }


    void GeoShapeFile::start(int32_t flags) {

        auto result = ErrorCode::None;

        try {
            File::start(flags);

            // Read 100 bytes header
            // Note: Header stores data in mixed endian order!

            checkBeforeReading();

            setBigEndian();
            m_shape_file_code = readValue<int32_t>();
            skip(20);
            m_shape_file_length = readValue<int32_t>();
            setLittleEndian();
            m_shape_file_version = readValue<int32_t>();
            m_shape_type = (GeoShape::ShapeType)readValue<int32_t>();

            for (int32_t i = 0; i < 8; i++) {
                m_shape_bbox[i] = readValue<double>();
            }

            m_record_start_pos = pos();
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        Exception::throwStandard(result);
    }


    void GeoShapeFile::setGeoShape(GeoShape* geo_shape) noexcept {

        m_shape = geo_shape;
    }


    ErrorCode GeoShapeFile::_readAllPoints(ReadMode mode) noexcept {

        auto result = ErrorCode::None;

        int32_t record_index = 0;

        try {
            if (!m_shape) {
                Exception::throwSpecific(kErrMissingGeoShapePtr);
            }

            if (m_shape_type != GeoShape::ShapeType::Point) {
                Exception::throwSpecific(kErrWrongShapeType);
            }

            m_shape->setShapeType(m_shape_type);
            for (int32_t i = 0; i < 8; i++) {
                m_shape->m_shape_bbox[i] = m_shape_bbox[i];
            }


            switch (mode) {

                case ReadMode::Count:
                    m_shape->m_point_count = 0;
                    break;

                case ReadMode::Read:
                    if (m_shape->m_point_count < 0) {
                        Exception::throwSpecific(kErrNothingToRead);
                    }
                    m_shape->points_.reserve(m_shape->m_point_count);
                    if (m_shape->points_.capacity() != m_shape->m_point_count) {
                        throw ErrorCode::MemCantAllocate;
                    }
                    break;
            }

            this->setPos(m_record_start_pos);

            int64_t pos = m_record_start_pos;
            int64_t skip_n = 0;
            while (pos + skip_n < size()) {
                if (skip_n > 0) {
                    this->skip(skip_n);
                    skip_n = 0;
                }

                setBigEndian();

                int32_t record_number = this->readValue<int32_t>();
                int32_t content_length = this->readValue<int32_t>();

                if (content_length == 10) {  // Point in shape files consists of 10 words (20 bytes).

                    Vec2d point;

                    setLittleEndian();
                    auto record_shape_type = (GeoShape::ShapeType)this->readValue<int32_t>();
                    if (record_shape_type != GeoShape::ShapeType::Point) {
                        Exception::throwSpecific(kErrWrongShapeType);
                    }

                    switch (mode) {

                        case ReadMode::Count:
                            m_shape->m_point_count++;
                            skip_n = 16;
                            break;

                        case ReadMode::Read:
                            if (record_index >= m_shape->m_point_count) {
                                Exception::throwSpecific(kErrWantsToReadMoreThanExpected);
                            }
                            point.readFromFile(*this);
                            m_shape->points_.push_back(point);
                            m_shape->addPointToRange(point);
                            break;
                    }
                }
                else {
                    skip_n = content_length * 2;
                }

                record_index++;
                pos = this->pos();
            }
        }
        catch (ErrorCode err) {

            if (mode == ReadMode::Read && record_index == m_shape->m_point_count) {
                // If all points are there when in read mode, mark as succeeded.
                result = ErrorCode::None;
            }
            else {
                result = err;
            }
        }

        if (result != ErrorCode::None) {
            printf("GeoShapeFile::_readAllPoints result: %d\n", result);
        }


        return result;
    }


    ErrorCode GeoShapeFile::_readAllPolys(ReadMode mode) noexcept {

        // TODO: Show progress, how?

        auto result = ErrorCode::None;

        int32_t record_index = 0;

        try {
            if (!m_shape) {
                Exception::throwSpecific(kErrMissingGeoShapePtr);
            }

            if (m_shape_type != GeoShape::ShapeType::PolyLine && m_shape_type != GeoShape::ShapeType::Polygon) {
                Exception::throwSpecific(kErrWrongShapeType);
            }

            m_shape->setShapeType(m_shape_type);
            m_shape->updateClosedPathDrawing();
            for (int32_t i = 0; i < 8; i++) {
                m_shape->m_shape_bbox[i] = m_shape_bbox[i];
            }

            switch (mode) {

                case ReadMode::Count:
                    m_shape->m_poly_count = 0;
                    m_shape->m_part_count = 0;
                    m_shape->m_point_count = 0;
                    break;

                case ReadMode::Read:
                    if (m_shape->m_poly_count < 0) {
                        Exception::throwSpecific(kErrNothingToRead);
                    }
                    m_shape->m_parts.reserve(m_shape->m_part_count);
                    if (m_shape->m_parts.capacity() != m_shape->m_part_count) {
                        throw ErrorCode::MemCantAllocate;
                    }
                    m_shape->m_polys.reserve(m_shape->m_poly_count);
                    if (m_shape->m_polys.capacity() != m_shape->m_poly_count) {
                        throw ErrorCode::MemCantAllocate;
                    }
                    break;
            }

            this->setPos(m_record_start_pos);

            int32_t part_index = 0;
            int32_t point_index = 0;

            int64_t pos = m_record_start_pos;
            int64_t skip_n = 0;
            while (pos + skip_n < size()) {

                if (skip_n > 0) {
                    this->skip(skip_n);
                    skip_n = 0;
                }

                switch (mode) {

                    case ReadMode::Count:
                    {
                        m_shape->m_poly_count++;
                        this->skip(3 * 4 + 4 * 8);
                        int32_t part_n = this->readValue<int32_t>();
                        int32_t point_n = this->readValue<int32_t>();
                        m_shape->m_part_count += part_n;
                        m_shape->m_point_count += point_n;

                        skip_n = 4 * part_n + 8 * 2 * point_n;
                    }
                        break;

                    case ReadMode::Read:
                    {
                        if (record_index >= m_shape->m_poly_count) {
                            Exception::throwSpecific(kErrWantsToReadMoreThanExpected);
                        }

                        m_record_file_pos_table.push_back((int32_t)this->pos());

                        setBigEndian();

                        GeoShapePoly poly;

                        poly.m_shape = m_shape;
                        poly.m_record_number = this->readValue<int32_t>();
                        poly.m_content_length = this->readValue<int32_t>();

                        setLittleEndian();
                        poly.m_shape_type = (GeoShape::ShapeType)this->readValue<int32_t>();
                        if (poly.m_shape_type != GeoShape::ShapeType::PolyLine && poly.m_shape_type != GeoShape::ShapeType::Polygon) {
                            Exception::throwSpecific(kErrWrongShapeType);
                        }

                        poly.bbox_.readFromFile(*this);

                        poly.m_part_count = this->readValue<int32_t>();
                        poly.m_point_count = this->readValue<int32_t>();

                        poly.m_part_offset = part_index;
                        for (int32_t i = 0; i < poly.m_part_count; i++) {
                            int32_t part = this->readValue<int32_t>();
                            m_shape->m_parts.push_back(part);
                            part_index++;
                        }

                        poly.m_point_offset = point_index;
                        for (int32_t i = 0; i < poly.m_point_count; i++) {
                            Vec2d point;
                            point.readFromFile(*this);
                            m_shape->points_.push_back(point);
                            m_shape->addPointToRange(point);
                            point_index++;
                        }

                        m_shape->m_polys.push_back(poly);
                    }
                        break;
                }

                record_index++;
                pos = this->pos();
            }
        }
        catch (ErrorCode err) {
            if (mode == ReadMode::Read && record_index == m_shape->m_point_count) {
                // If all points are there when in read mode, mark as succeeded.
                result = ErrorCode::None;
            }
            else {
                result = err;
            }
        }


        if (mode == ReadMode::Count && m_shape->m_poly_count > 0 && result == ErrorCode::None) {
            m_record_file_pos_table.reserve(m_shape->m_poly_count);
        }

        if (result != ErrorCode::None) {
            printf("GeoShapeFile::_readAllPolys result: %d\n", result); // TODO: Messaging!
        }

        return result;
    }


    ErrorCode GeoShapeFile::callPointActionForAllPoints(void* action_ref) noexcept {

        auto result = ErrorCode::None;

        int32_t record_index = 0;

        try {
            if (!m_point_action) {
                Exception::throwSpecific(kErrMissingAction);
            }
            if (m_shape_type != GeoShape::ShapeType::Point) {
                Exception::throwSpecific(kErrWrongShapeType);
            }

            this->setPos(m_record_start_pos);

            int64_t pos = m_record_start_pos;
            int64_t skip_n = 0;
            while (pos + skip_n < size()) {
                if (skip_n > 0) {
                    this->skip(skip_n);
                    skip_n = 0;
                }

                setBigEndian();

                int32_t record_number = this->readValue<int32_t>();
                int32_t content_length = this->readValue<int32_t>();

                if (content_length == 10) {  // Point in shape files consists of 10 words (20 bytes).
                    setLittleEndian();
                    auto record_shape_type = (GeoShape::ShapeType)this->readValue<int32_t>();
                    if (record_shape_type != GeoShape::ShapeType::Point) {
                        Exception::throwSpecific(kErrWrongShapeType);
                    }
                    Vec2d point;
                    point.readFromFile(*this);
                    m_point_action(this, record_index, point, action_ref);
                }
                else {
                    skip_n = content_length * 2;
                }

                record_index++;
                pos = this->pos();
            }
        }
        catch (ErrorCode err) {
            result = err;
        }


        return result;
    }


/**
 *  @brief Converts a Shape file to a Polygon file format.
 *
 *  Reads data from a Shape file and converts its contents into the
 *  Polygon file format.
 *
 *  @param file_path The file path where the resulting Polygon file will be saved.
 *  @param dst_srid SRID, Spatial Reference System Identifier, in which the data is contained.
 *
 *  @note If the file already exists, it will be overwritten.
 */
    ErrorCode GeoShapeFile::convertToPolygonsFile(const String& file_path, int32_t dst_srid) noexcept {

        auto result = ErrorCode::None;

        List<uint64_t> file_record_pos_array;
        int64_t file_pos_number_of_polygons = -1;
        int64_t file_pos_first_record = -1;
        int32_t polygon_count = 0;

        try {
            startRead();

            if (m_shape_type != GeoShape::ShapeType::PolyLine && m_shape_type != GeoShape::ShapeType::Polygon) {
                Exception::throwSpecific(kErrWrongShapeType);
            }

            // Check if projection file exists
            String prj_file_path = file_path_;
            prj_file_path.replace(".shp", ".prj");
            if (!File::fileExists(prj_file_path)) {
                Exception::throwStandard(ErrorCode::FileNotFound);
            }

            // Setup the projection for transforming og all points
            GeoProj proj;
            auto err = proj.setSrcCrsByFile(prj_file_path);
            Exception::throwStandard(err);
            proj.setDstSRID(dst_srid);

            // Open file for writing
            File polygon_file(file_path);
            polygon_file.startWriteOverwrite();

            // Write signature
            polygon_file.writeStr("PLGN");
            polygon_file.writeEndianSignature();

            // Remember where to save information later
            file_pos_number_of_polygons = polygon_file.pos();
            // Polygon count, will be filled in later
            polygon_file.writeInt32(0);

            // Write bounding box for all polygons
            RangeRectd bbox(m_shape_bbox[0], m_shape_bbox[1], m_shape_bbox[2], m_shape_bbox[3]);
            proj.transform(bbox);
            bbox.writeToFile(polygon_file);

            // SRID for all coordinates in polygons file
            polygon_file.writeInt64(dst_srid);

            // Go through file two times
            for (int32_t pass = 0; pass < 2; pass++) {
                int32_t record_index = 0;
                int32_t part_index = 0;
                int32_t point_index = 0;
                int32_t max_part_count = 0;
                int32_t max_point_count = 0;
                int32_t total_part_count = 0;
                int32_t total_point_count = 0;
                int32_t total_parts_with_gt1_count = 0;

                setPos(m_record_start_pos);
                int64_t read_pos = m_record_start_pos;

                while (read_pos < size()) {
                    setBigEndian();

                    int32_t record_number = readValue<int32_t>();
                    int32_t content_length = readValue<int32_t>();

                    setLittleEndian();

                    auto shape_type = (GeoShape::ShapeType)readValue<int32_t>();
                    if (shape_type != GeoShape::ShapeType::PolyLine && shape_type != GeoShape::ShapeType::Polygon) {
                        Exception::throwSpecific(kErrWrongShapeType);
                    }

                    RangeRectd bbox;
                    bbox.readFromFile(*this);
                    proj.transform(bbox);

                    int32_t part_count = readValue<int32_t>();
                    int32_t point_count = readValue<int32_t>();

                    if (pass == 0) {

                        // Write polygon information.

                        if (record_index == 0) {
                            file_pos_first_record = polygon_file.pos();
                        }

                        polygon_file.writeInt64(0); // Placeholder for file position of record.
                        bbox.writeToFile(polygon_file);
                        polygon_file.writeInt32(part_count);
                        polygon_file.writeInt32(point_count);
                    }
                    else if (pass == 1) {
                        int64_t pos = polygon_file.pos();
                        if (file_record_pos_array.push(pos) == false) {
                            throw ErrorCode::MemCantGrow;
                        }
                    }

                    total_part_count += part_count;
                    total_point_count += point_count;

                    if (part_count > 1) {
                        total_parts_with_gt1_count++;
                    }

                    if (part_count > max_part_count) {
                        max_part_count = part_count;
                    }

                    if (point_count > max_point_count) {
                        max_point_count = point_count;
                    }


                    for (int32_t i = 0; i < part_count; i++) {
                        int32_t part_index = readValue<int32_t>();
                        if (pass == 1) {
                            polygon_file.writeInt32(part_index);
                        }
                        part_index++;
                    }

                    for (int32_t i = 0; i < point_count; i++) {
                        Vec2d point;
                        point.readFromFile(*this);
                        if (pass == 1) {
                            proj.transform(point);
                            point.writeToFile(polygon_file);
                        }

                        // std::cout << i << " point: " << point << std::endl;
                        point_index++;
                    }

                    record_index++;

                    read_pos = pos();
                }

                if (pass == 0) {
                    polygon_count = record_index;
                    if (file_record_pos_array.reserve(polygon_count) == false) {
                        throw ErrorCode::MemCantAllocate;
                    }
                }

                std::cout << "max_part_count: " << max_part_count << std::endl;
                std::cout << "max_point_count: " << max_point_count << std::endl;
                std::cout << "total_part_count: " << total_part_count << std::endl;
                std::cout << "total_point_count: " << total_point_count << std::endl;
                std::cout << "total_parts_with_gt1_count: " << total_parts_with_gt1_count << std::endl;
            }

            // Finish polygon_file.

            polygon_file.flush();
            polygon_file.setPos(file_pos_number_of_polygons);
            polygon_file.writeInt32(polygon_count);

            std::cout << "polygon_file_pos_first_record: " << file_pos_first_record << std::endl;
            for (int32_t i = 0; i < polygon_count; i++) {
                if (!(i % 100000)) {
                    std::cout << i << ": " << (file_pos_first_record + i * kPolygonsFileRecordSize) << std::endl;

                }
                polygon_file.flush();
                polygon_file.setPos(file_pos_first_record + i * kPolygonsFileRecordSize);
                polygon_file.writeInt64(file_record_pos_array[i]);
            }

            polygon_file.close();

            std::cout << "m_shape_file_length: " << m_shape_file_length << std::endl;
            std::cout << "m_shape_file_version: " << m_shape_file_version << std::endl;
            for (int32_t i = 0; i < 2; i++) {
                Vec2d point;
                point.set(m_shape_bbox[i * 2], m_shape_bbox[i * 2 + 1]);
                proj.transform(point);
                std::cout << "m_shape_bbox[" << i << "]: " << point << std::endl;
            }

            std::cout << "polygon_count: " << polygon_count << std::endl;
            std::cout << "polygon_file_record_pos_array.size(): " << file_record_pos_array.size() << std::endl;
        }
        catch (ErrorCode err) {
            std::cerr << "GeoShapeFile::convertToPolygonsFile() err: " << (int)err << std::endl;
            result = err;
        }

        return result;
    }


} // End of namespace Grain
