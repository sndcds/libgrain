//
//  GeoShapeFile.hpp
//
//  Created by Roald Christesen on from 05.07.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#ifndef GrainGeoShapeFile_hpp
#define GrainGeoShapeFile_hpp

#include "Grain.hpp"
#include "File/File.hpp"
#include "2d/Rect.hpp"
#include "2d/RangeRect.hpp"
#include "Geo/GeoShape.hpp"


namespace Grain {

    class GeoProj;
    class GeoShapeFile;

    typedef void (*GeoShapeFilePointAction)(GeoShapeFile* shape_file, int32_t index, Vec2d& point, void* action_ref);


    /**
     *  @brief ESRI shape files.
     *
     *  http://shapelib.maptools.org
     */
    class GeoShapeFile : public File {

    public:
        enum class ReadMode {
            Count = 0,
            Read = 1
        };

        enum {
            kErrWrongShapeType = 0,
            kErrMissingGeoShapePtr,
            kErrMissingAction,
            kErrWantsToReadMoreThanExpected,
            kErrNothingToRead
        };

        enum {
            kPolygonsFileRecordSize = 48
        };

    protected:
        int32_t m_shape_file_code = 0;
        int32_t m_shape_file_length = -1;
        int32_t m_shape_file_version = -1;
        double m_shape_bbox[8]{};

        GeoShape::ShapeType m_shape_type = GeoShape::ShapeType::Undefined;
        GeoShape* m_shape = nullptr;

        int64_t m_record_start_pos = -1;
        std::vector<int32_t> m_record_file_pos_table;

        GeoShapeFilePointAction m_point_action = nullptr;

    public:
        GeoShapeFile(const String& file_path);
        ~GeoShapeFile() noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "GeoShapeFile"; }

        void start(int32_t flags) override;


        void setGeoShape(GeoShape* geo_shape) noexcept;

        [[nodiscard]] GeoShape::ShapeType shapeType() const noexcept { return m_shape_type; }
        [[nodiscard]] const char* shapeTypeName() const { return GeoShape::shapeTypeName(m_shape_type); }

        [[nodiscard]] ErrorCode readAllPoints() noexcept {
            auto err = _readAllPoints(ReadMode::Count);
            if (err != ErrorCode::None) {
                err = _readAllPoints(ReadMode::Read);
            }
            return err;
        }

        ErrorCode readAllPolys() noexcept {
            auto err = _countAllPolys();
            if (err != ErrorCode::None) {
                err = _readAllPolys();
            }
            return err;
        }

        ErrorCode _readAllPoints(ReadMode mode) noexcept;

        ErrorCode _readAllPolys(ReadMode mode = ReadMode::Read) noexcept;
        ErrorCode _countAllPolys() noexcept {
            return _readAllPolys(ReadMode::Count);
        }

        void setPointAction(GeoShapeFilePointAction action) noexcept { m_point_action = action; }

        ErrorCode callPointActionForAllPoints(void *action_ref = nullptr) noexcept;


        ErrorCode convertToPolygonsFile(const String& file_path, int32_t dst_srid) noexcept;
    };

} // End of namespace Grain.

#endif // GrainGeoShapeFile_hpp
