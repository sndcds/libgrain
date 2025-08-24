//
//  PolygonsFile.hpp
//
//  Created by Roald Christesen on 10.06.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

#ifndef GrainPolygonsFile_hpp
#define GrainPolygonsFile_hpp

#include "Grain.hpp"
#include "2d/RangeRect.hpp"
#include "Type/List.hpp"
#include "File/File.hpp"


namespace Grain {

    struct PolygonsFileEntry {
        int64_t m_file_pos{};           ///< Position of polygon data in file
        RangeRectd m_bounding_box;      ///< Bounding box for the polygon
        int32_t m_part_count{};         ///< Number of parts in polygon
        int32_t m_point_count{};        ///< Number of point in polygon
    };


    class PolygonsFile : public File {

    public:
        enum {
            kErrNoPolygonsInFile = 0,
        };

    protected:
        uint32_t m_polygon_count{};                 ///< Number of polygons in file
        List<PolygonsFileEntry> m_polygon_entries;  ///< All file entries
        RangeRectd m_bounding_box;                  ///< Bound box in WGS84/EPSG:4326 coordinates
        int64_t m_srid;                             ///< SRID, Spatial Reference System Identifier

    public:
        PolygonsFile(const String& file_path) noexcept;
        ~PolygonsFile() noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "PolygonsFile"; }

        friend std::ostream& operator << (std::ostream& os, const PolygonsFile* o) {
            o == nullptr ? os << "PolygonsFile nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const PolygonsFile& o) {
            os << "polygon_count: " << o.m_polygon_count;
            os << ", bounding_box: " << o.m_bounding_box;
            os << ", srid: " << o.m_srid;
            return os;
        }

        [[nodiscard]] uint32_t polygonCount() const noexcept { return m_polygon_count; }

        [[nodiscard]] const PolygonsFileEntry* entryPtrAtIndex(int32_t index) const noexcept {
            return m_polygon_entries.elementPtrAtIndex(index);
        }

        void printEntryInfo(std::ostream& os, int32_t entry_index) {
            auto entry = entryPtrAtIndex(entry_index);
            if (entry != nullptr) {
                os << "file_pos: " << entry->m_file_pos;
                os << ", part_count: " << entry->m_part_count;
                os << ", point_count: " << entry->m_point_count;
                os << ", bounding_box: " << entry->m_bounding_box << std::endl;
            }
        }

        ErrorCode readInfo() noexcept;
    };


} // End of namespace Grain

#endif // GrainPolygonsFile_hpp
