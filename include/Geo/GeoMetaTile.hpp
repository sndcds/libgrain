//
//  GeoMetaTile.hpp
//
//  Created by Roald Christesen on from 05.09.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.08.2025
//

#ifndef GrainGeoMetaTile_hpp
#define GrainGeoMetaTile_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "File/File.hpp"
#include "Math/Vec2.hpp"
#include "2d/RangeRect.hpp"
#include "Core/Log.hpp"


namespace Grain {

    class Image;


    struct GeoMetaTileEntry {
        uint32_t m_offset;      ///< Offset of tile data from start of the file.
        uint32_t m_size;        ///< Size of tile data in bytes.
    };


    class GeoMetaTile : public File {

    public:
        enum {
            kErrUnsupportedCount = 0,
            kErrUnsupportedZoom,
            kErrTileMetaTileSizeMismatch,
            kErrTempFileNotFound
        };

        char m_magic[4];
        int32_t m_count;                        ///< Number of tiles included.
        int32_t m_x;                            ///< Lowest x position.
        int32_t m_y;                            ///< Lowest y position.
        int32_t m_zoom;                         ///< Zoom level.
        struct GeoMetaTileEntry* m_entries;     ///< One entry per tile.
        bool m_compressed = false;

    public:
        GeoMetaTile(const String& file_path) noexcept : File(file_path) {
        }

        ~GeoMetaTile() noexcept;

        const char* className() const noexcept override { return "GeoMetaTile"; }

        friend std::ostream& operator << (std::ostream& os, const GeoMetaTile& o) {
            os << "GeoMetaTile\n";
            os << "  count: " << o.m_count << ", zoom: " << "x, y: " << o.m_x << ", " << o.m_y;
            if (o.m_compressed) {
                os << ", compressed";
            }
            os << std::endl;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const GeoMetaTile* o) {
            o == nullptr ? os << "GeoMetaTile nullptr" : os << *o;
            return os;
        }


        void startRead() override {

            File::startRead();


            // Check the header.

            setPos(0);

            readStr(4, m_magic);
            if (std::strncmp(m_magic, "METZ", 4) == 0) {
                m_compressed = true;
            }
            else if (std::strncmp(m_magic, "META", 4) != 0) {
                throw ErrorCode::UnsupportedFileFormat;
            }

            m_count = readValue<int32_t>();
            if (m_count < 1) {
                throw Error::specific(kErrUnsupportedCount);
            }

            m_x = readValue<int32_t>();
            m_y = readValue<int32_t>();
            m_zoom = readValue<int32_t>();

            if (m_zoom < 1) {
                throw Error::specific(kErrUnsupportedZoom);
            }

            m_entries = (GeoMetaTileEntry*)malloc(sizeof(GeoMetaTileEntry) * m_count);
            if (m_entries == nullptr) {
                throw ErrorCode::MemCantAllocate;
            }

            for (int32_t i = 0; i < m_count; i++) {
                m_entries[i].m_offset = readValue<uint32_t>();
                m_entries[i].m_size = readValue<uint32_t>();
            }
        }

        static ErrorCode saveMetaTileFile(fourcc_t tile_order, int32_t zoom, int32_t tile_x, int32_t tile_y, const String& tiles_dir_path, const String& meta_file_path, const String& tile_name_format, const String& file_ext, bool create_dir_flag);

        static ErrorCode writeMetaTileFromImage(const String& file_path, Image* image, Image* tile_image, int32_t zoom, Vec2i tile_index, fourcc_t tile_order) noexcept;
    };


    class GeoMetaTileRange;

    typedef void (*GeoMetaTileAction)(GeoMetaTileRange* meta_tile_range, void* ref);


    /**
     *  @class GeoMetaTileRange
     *  @brief Defines a range of meta tiles used in cartographic map systems.
     *
     *  The `GeoMetaTileRange` class represents a range of meta tiles within a
     *  cartographic mapping system. A meta tile is a composite of smaller tiles,
     *  which enables efficient handling and rendering of map data. This class
     *  provides mechanisms for setting up tile sizes and defining ranges based on
     *  geographic bounds.
     */
    class GeoMetaTileRange : public Object {

    public:
        /**
         *  @brief Grid size of a meta tile.
         *
         *  Represents the number of tiles along one edge of a meta tile. This value is constant
         *  and determines the composition of meta tiles.
         */
        static constexpr int32_t kGridSize = 8;

    protected:
        int32_t m_zoom;                 ///< The zoom level of the tile range.
        int32_t m_tile_size;            ///< The size of an individual tile, in pixels.
        int32_t m_meta_tile_size;       ///< The size of a meta tile, calculated as `kGridSize * m_tile_size`.
        int64_t m_horizontal_tile_n;    ///< Number of tiles in horizontal (x) direction.
        int64_t m_vertical_tile_n;      ///< Number of tiles in vertical (y) direction.
        int64_t m_meta_tiles_needed;    ///< The number of meta tiles needed to cover the range.
        int64_t m_curr_index;           ///< The current index during iteration or processing.
        Vec2i m_curr_meta_index;        ///< The current meta index during iteration or processing.

        Vec2i m_tile_start;             ///< The starting tile position in the range (x, y).
        Vec2i m_tile_end;               ///< The ending tile position in the range (x, y).
        Vec2i m_first_tile;             ///< First tile when iterating.
        Vec2i m_curr_tile;              ///< The current tile position during iteration or processing.

        int32_t m_sn;
        bool m_reset_flag = true;

        GeoMetaTileAction m_action = nullptr;
        void* m_action_ref = nullptr;


    public:
        GeoMetaTileRange(int32_t zoom, const RangeRectd& bbox);

        const char* className() const noexcept override { return "GeoMetaTileRange"; }

        friend std::ostream& operator << (std::ostream& os, const GeoMetaTileRange* o) {
            o == nullptr ? os << "GeoMetaTileRange nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const GeoMetaTileRange& o) {
            o.log(os, 0, o.className());
            return os;
        }

        void log(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {
            Log log(os, indent);
            log.header(label);
            log << "zoom: " << m_zoom << ", tile size: " << m_tile_size << log.endl;
            log << "m tile size: " << m_tile_size << log.endl;
            log << "meta tile size: " << m_meta_tile_size << log.endl;
            log << "tile n: " << m_horizontal_tile_n << " x " << m_vertical_tile_n << log.endl;
            log << "meta tiles needed " << m_meta_tiles_needed << log.endl;
            log << "curr index: " << m_curr_index << log.endl;
            log << "sn: " << m_sn << log.endl;
            log << "reset flag: " << m_reset_flag << log.endl;
            log << "has action: " << log.boolValue(m_action != nullptr) << log.endl;

            log << "tile start: " << m_tile_start << ", end: " << m_tile_end << log.endl;
            log << "first tile: " << m_first_tile << ", curr: " << m_curr_tile << log.endl;
            log << "tile: " << m_tile_start << log.endl;
            log++;
        }

        void logCompact(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {
            Log log(std::cout, indent);
            log.label(label);
            log << "zoom: " << m_zoom << ", tiles needed: " << metaTilesNeeded() << " (" << m_horizontal_tile_n << ", " << m_vertical_tile_n << ")" << ", current: " << m_curr_index << log.endl;
        }

        void logCurrent(std::ostream& os, int32_t indent = 0, const char* label = nullptr) const {
            Log log(std::cout, indent);
            log.label(label);
            log << "index " << currIndex() << " of " << metaTilesNeeded() << ", tile index x: " << x() << ", tile index y: " << y() << log.endl;
        }



        void setByBbox(int32_t zoom, const RangeRectd& bbox) noexcept;
        void setSetupTileSize(int32_t tile_size) noexcept;
        void setTileAction(GeoMetaTileAction action, void* ref) noexcept;
        void iterateAllMetaTiles() noexcept;

        bool setStartIndex(int64_t start_index) noexcept;
        bool nextTilePos(int64_t end_index, Vec2i& out_tile_index) noexcept;
        bool nextTilePos(Vec2i& out_tile_index) noexcept { return nextTilePos(std::numeric_limits<int64_t>::max(), out_tile_index); }

        bool valid() const noexcept {
            if (m_zoom < 0 ||
                m_tile_size < 1 ||
                m_meta_tile_size != kGridSize * m_tile_size ||
                m_horizontal_tile_n < 1 ||
                m_vertical_tile_n < 1 ||
                m_meta_tiles_needed != m_horizontal_tile_n * m_vertical_tile_n ||
                m_curr_index < 0 || m_curr_index >= m_meta_tiles_needed ||
                m_sn > kGridSize) {
                return false;
            }
            else {
                return true;
            }
        }

        int32_t zoom() const noexcept { return m_zoom; }
        int32_t gridSize() const noexcept { return kGridSize; }
        int64_t metaTilesNeeded() const noexcept { return m_meta_tiles_needed; }
        int32_t metaTileSize() const noexcept { return m_meta_tile_size; }
        int64_t currIndex() const noexcept { return m_curr_index; }
        int64_t rest() const noexcept { return m_meta_tiles_needed - m_curr_index; }
        int64_t x() const noexcept { return m_curr_meta_index.m_x; }
        int64_t y() const noexcept { return m_curr_meta_index.m_y; }
        int64_t horizontalMetaTileCount() const noexcept { return m_horizontal_tile_n; }
        int64_t verticalMetaTileCount() const noexcept { return m_vertical_tile_n; }

        void wgs84EnvelopeBbox(RangeRectd& out_bbox) const noexcept;
    };


} // End of namespace Grain

#endif // GrainGeoMetaTile_hpp
