//
//  GeoMetaTile.hpp
//
//  Created by Roald Christesen on from 05.09.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Geo/GeoMetaTile.hpp"
#include "Geo/Geo.hpp"
#include "Image/Image.hpp"


namespace Grain {

    GeoMetaTile::~GeoMetaTile() noexcept {

    }


    /**
     *  @brief Save a meta-tile file from 8 x 8 individual tile files.
     *
     *  @param tile_order Can be 'row_' (RowMajor) or 'col_' (ColumnOrder).
     *  @param zoom Zoom level.
     *  @param tile_x Slippy map tile x index.
     *  @param tile_y Slippy map tile y index.
     *  @param tiles_dir_path Path to the directory containing the tile files.
     *  @param meta_file_path Path to the directory where the meta tile file will be saved.
     *  @param tile_name_format Format to interpret the tile file names in `tiles_dir_path`.
     *  @param file_ext The file exension, e.g. png
     *  @param create_dir_flag Flag indicating whether directories should be created if they do not exist.
     *
     *  @return ErrorCode::None on success, or an appropriate ErrorCode on failure.
     */
    ErrorCode GeoMetaTile::saveMetaTileFile(fourcc_t tile_order, int32_t zoom, int32_t tile_x, int32_t tile_y, const String& tiles_dir_path, const String& meta_file_path, const String& tile_name_format, const String& file_ext, bool create_dir_flag) {

        std::cout << "saveMetaTileFile zoom: " << zoom << ", tiles_dir_path: " << tiles_dir_path << ", meta_file_path: " << meta_file_path << ", tile_name_format: " << tile_name_format << ", file_ext: " << file_ext << std::endl;
        constexpr int32_t x_n = 8;
        constexpr int32_t y_n = 8;
        constexpr int32_t tile_n = x_n * y_n;

        auto result = ErrorCode::None;

        uint8_t* data = nullptr;

        try {
            if (create_dir_flag) {
                if (!File::makeDirs(meta_file_path.fileDirPath())) {
                    throw ErrorCode::FileDirNotCreated;
                }
            }

            char tile_name[256];
            char file_path[2560];

            File meta_file(meta_file_path);
            meta_file.startWriteOverwrite();

            meta_file.writeStr("META");
            meta_file.writeValue<int32_t>(tile_n);
            meta_file.writeValue<int32_t>(tile_x);
            meta_file.writeValue<int32_t>(tile_y);
            meta_file.writeValue<int32_t>(zoom);

            uint32_t tile_offs = static_cast<uint32_t>(meta_file.pos() + tile_n * sizeof(GeoMetaTileEntry));
            uint32_t tile_max_size = 0;

            // Write header
            for (int32_t y = 0; y < 8; y++) {
                for (int32_t x = 0; x < 8; x++) {
                    std::snprintf(tile_name, 256, tile_name_format.utf8(), 8 * x + y);
                    std::snprintf(file_path, 2560, "%s/%s.%s", tiles_dir_path.utf8(), tile_name, file_ext.utf8());
                    int32_t tile_file_size = 0;
                    if (File::fileExists(file_path)) {
                        File tile_file(file_path);
                        tile_file.startRead();
                        tile_file_size = static_cast<int32_t>(tile_file.size());
                        if (tile_file_size > static_cast<int32_t>(tile_max_size)) {
                            tile_max_size = tile_file_size;
                        }
                        tile_file.close();
                    }
                    meta_file.writeValue<int32_t>(tile_offs);
                    meta_file.writeValue<int32_t>(tile_file_size);
                    if (tile_file_size > static_cast<int32_t>(tile_max_size)) {
                        tile_max_size = tile_file_size;
                    }
                    tile_offs += tile_file_size;
                }
            }

            std::cout << "tile_max_size: " << tile_max_size << std::endl;
            if (tile_max_size <= 0) {
                return ErrorCode::None;
            }

            data = (uint8_t*)malloc(tile_max_size);
            if (!data) {
                throw ErrorCode::MemCantAllocate;
            }

            // Write data for all tiles
            for (int32_t y = 0; y < 8; y++) {
                for (int32_t x = 0; x < 8; x++) {
                    std::snprintf(tile_name, 256, tile_name_format.utf8(), 8 * x + y);
                    std::snprintf(file_path, 2560, "%s/%s.%s", tiles_dir_path.utf8(), tile_name, file_ext.utf8());
                    if (File::fileExists(file_path)) {
                        File tile_file(file_path);
                        tile_file.startRead();
                        auto tile_file_size = tile_file.size();
                        if (tile_file_size > 0) {
                            tile_file.read(tile_file_size, (uint8_t*)data);
                            meta_file.writeData<uint8_t>(data, tile_file_size);
                            tile_file.close();
                        }
                    }
                }
            }

            meta_file.close();
        }
        catch (ErrorCode err) {
            result = err;
        }

        free(data);

        return result;
    }


    /**
     *  @brief Writes a meta tile from the given image.
     *
     *  This function processes an image and writes the resulting meta tile
     *  to the specified file path. It uses `tile_image` to render the individual
     *  tiles that make up the meta tile.
     *
     *  @param file_path  The file path where the meta tile should be saved.
     *  @param image      Pointer to the main image to be processed.
     *  @param tile_image Pointer to the tile image used for rendering individual tiles.
     *  @param zoom       The zoom level for the meta tile.
     *  @param tile_index Slippy tile index, used in Meta file header.
     *  @param tile_order The tile order format, specified as a `fourcc_t` code:
     *                    - `'row_'` → Row-major order (tiles arranged row by row)
     *                    - `'col_'` → Column-major order (tiles arranged column by column)
     *
     *  @return ErrorCode indicating success or failure of the operation.
     */
    ErrorCode GeoMetaTile::writeMetaTileFromImage(const String& file_path, Image* image, Image* tile_image, int32_t zoom, Vec2i tile_index, fourcc_t tile_order) noexcept {

        // TODO: Support image file types, png, jpg, webp ...

        constexpr int32_t tile_x_n = 8;
        constexpr int32_t tile_y_n = 8;
        constexpr int32_t tile_n = tile_x_n * tile_y_n;
        constexpr int32_t tile_w = 256;
        constexpr int32_t tile_h = 256;
        constexpr int32_t tile_count = tile_x_n * tile_y_n;

        auto result = ErrorCode::None;
        uint8_t* tile_img_data = nullptr;

        try {
            if (!image) { throw Error::specific(1); }
            if (!tile_image) { throw Error::specific(2); }
            if (zoom < 0) { throw Error::specific(3); }

            if (tile_image->width() != tile_w || tile_image->height() != tile_h) {
                throw Error::specific(kErrTileMetaTileSizeMismatch);
            }

            if (image->width() != tile_image->width() * tile_x_n || image->height() != tile_image->height() * tile_y_n) {
                throw Error::specific(kErrTileMetaTileSizeMismatch);
            }


            // Step 1 - Save all tiles that are part of the meta tile.

            String temp_path = file_path.fileDirPath();

            float pixel[4];
            ImageAccess src_ia(image, pixel);
            ImageAccess dst_ia(tile_image, pixel);

            int32_t temp_index = 0;
            for (int32_t yi = 0; yi < tile_y_n; yi++) {
                for (int32_t xi = 0; xi < tile_x_n; xi++) {
                    src_ia.setRegion(xi * tile_w, yi * tile_h, tile_w, tile_h);
                    int32_t dst_y = 0;
                    while (src_ia.stepY()) {
                        int32_t dst_x = 0;
                        while (src_ia.stepX()) {
                            src_ia.read();
                            dst_ia.setPos(dst_x, dst_y);
                            dst_ia.write();
                            dst_x++;
                        }
                        dst_y++;
                    }
                    tile_image->writePng(temp_path + "/_temp_" + temp_index + ".png", 1.0f, true);
                    temp_index++;
                }
            }


            // Step 2 - Write the meta tile.

            File meta_file(file_path);
            meta_file.startWriteOverwrite();

            // Header.
            meta_file.writeStr("META");
            meta_file.writeValue<int32_t>(tile_n);
            meta_file.writeValue<int32_t>(tile_index.m_x);
            meta_file.writeValue<int32_t>(tile_index.m_y);
            meta_file.writeValue<int32_t>(zoom);

            uint32_t tile_offs = static_cast<uint32_t>(meta_file.pos() + tile_n * sizeof(GeoMetaTileEntry));
            uint32_t tile_img_max_size = 0;

            for (int32_t pass = 0; pass < 2; pass++) {

                temp_index = 0;
                for (int32_t yi = 0; yi < tile_y_n; yi++) {
                    for (int32_t xi = 0; xi < tile_x_n; xi++) {
                        String tile_img_file_path = temp_path + "/_temp_" + temp_index + ".png";
                        int32_t tile_img_file_size = 0;
                        if (File::fileExists(tile_img_file_path)) {
                            File tile_img_file(tile_img_file_path);
                            tile_img_file.startRead();
                            tile_img_file_size = static_cast<int32_t>(tile_img_file.size());

                            if (pass == 0) {
                                meta_file.writeValue<int32_t>(tile_offs);
                                meta_file.writeValue<int32_t>(tile_img_file_size);
                                if (tile_img_file_size > static_cast<int32_t>(tile_img_max_size)) {
                                    tile_img_max_size = tile_img_file_size;
                                }
                                tile_offs += tile_img_file_size;
                            }
                            else if (pass == 1) {
                                if (!tile_img_data) {
                                    tile_img_data = (uint8_t *) malloc(tile_img_max_size);
                                    if (!tile_img_data) {
                                        throw ErrorCode::MemCantAllocate;
                                    }
                                }
                                // Write image data for tile
                                if (!tile_img_data) {
                                    throw ErrorCode::MemCantAllocate;
                                }
                                tile_img_file.read(tile_img_file_size, (uint8_t*)tile_img_data);
                                meta_file.writeData<uint8_t>(tile_img_data, tile_img_file_size);
                            }

                            tile_img_file.close();
                            if (pass > 0) {
                                File::removeFile(tile_img_file_path);
                            }
                        }
                        else {
                            throw Error::specific(kErrTempFileNotFound);
                        }

                        if (tile_order == Type::fourcc('r', 'o', 'w', '_')) {
                            // Row-major order.
                            temp_index++;
                        }
                        else {
                            // Column-major order.
                            temp_index += tile_x_n;
                            if (temp_index >= tile_count) {
                                temp_index -= (tile_count - 1);
                            }
                        }
                    }
                }
            }

            meta_file.close();
        }
        catch (ErrorCode err) {
            result = err;
            std::cout << "!!! err: " << (int)err << std::endl;
        }

        free(tile_img_data);

        return result;
    }


    /**
     *  @brief Constructs a GeoMetaTileRange object with default parameters.
     *
     *  This constructor initializes a `GeoMetaTileRange` instance using the
     *  specified zoom level and geographic bounds. It sets the tile size to a default
     *  value of 256 pixels and uses the provided bounds to configure the range. The
     *  bounds must be specified in the SRID 4326 coordinate system, which uses
     *  geographic coordinates (latitude and longitude) in degrees.
     *
     *  @param zoom The zoom level at which the meta-tile range is being created.
     *              Higher zoom levels represent finer detail and smaller tiles.
     *  @param bbox A `RangeRectd` object representing the geographic bounds for
     *              the meta-tile range. The coordinates in this rectangle must be
     *              in the SRID 4326 coordinate system.
     */
    GeoMetaTileRange::GeoMetaTileRange(int32_t zoom, const RangeRectd& bbox) {

        setSetupTileSize(256);
        setByBbox(zoom, bbox);
    }


    /**
     *  @brief Sets the size of an individual tile and recalculates the meta tile size.
     *
     *  @param tile_size The size of an individual tile, in pixels.
     */
    void GeoMetaTileRange::setSetupTileSize(int32_t tile_size) noexcept {

        m_tile_size = tile_size;
        m_meta_tile_size = kGridSize * m_tile_size;
    }


    void GeoMetaTileRange::setTileAction(GeoMetaTileAction action, void* ref) noexcept {

        m_action = action;
        m_action_ref = ref;
    }


    /**
     *  @brief Sets the range of meta tiles based on a zoom level and geographic bounds.
     *
     *  @param zoom The zoom level for which the meta tile range is defined.
     *  @param bbox A reference to a `RangeRectd` object defining the geographic
     *              bounding box of the meta tile range.
     */
    void GeoMetaTileRange::setByBbox(int32_t zoom, const RangeRectd& bbox) noexcept {

        m_zoom = zoom;

        Geo::wgs84ToTileIndex(m_zoom, bbox.m_min_x, bbox.m_max_y, m_tile_start.m_x, m_tile_start.m_y);
        Geo::wgs84ToTileIndex(m_zoom, bbox.m_max_x, bbox.m_min_y, m_tile_end.m_x, m_tile_end.m_y);


        m_first_tile.m_x = m_tile_start.m_x & (~0b111);
        m_first_tile.m_y = m_tile_start.m_y & (~0b111);


        // Special cases for zoom levels less than 3.

        m_sn = 0;
        switch (zoom) {
            case 0: m_sn = 1; break;
            case 1: m_sn = 2; break;
            case 2: m_sn = 4; break;
            default: m_sn = kGridSize; break;
        }

        m_horizontal_tile_n = (m_tile_end.m_x - m_first_tile.m_x) / kGridSize + 1;
        m_vertical_tile_n = (m_tile_end.m_y - m_first_tile.m_y) / kGridSize + 1;

        m_meta_tiles_needed = m_horizontal_tile_n * m_vertical_tile_n;

        m_reset_flag = true;
        m_curr_index = 0;
    }


    /**
     *  @brief Iterates over all tiles in the meta tile range and invokes a user-defined function.
     *
     *  This method traverses all tiles within the range defined by the `GeoMetaTileRange` instance.
     *  For each tile, a user-defined function is called, allowing custom operations to be performed
     *  on the tile. The range and tile dimensions are determined by the current meta tile configuration.
     *
     *  @note The user-defined function must be set up before invoking this method. Ensure the function
     *        is compatible with the tile data structure and processing logic.
     *
     *  @exception noexcept This method is guaranteed not to throw exceptions.
     */
    void GeoMetaTileRange::iterateAllMetaTiles() noexcept {

        Log log(std::cout);

        for (m_curr_tile.m_y = m_first_tile.m_y, m_curr_meta_index.m_y = 0;
             m_curr_tile.m_y <= m_tile_end.m_y;
             m_curr_tile.m_y += kGridSize, m_curr_meta_index.m_y++) {

            for (m_curr_tile.m_x = m_first_tile.m_x, m_curr_meta_index.m_x = 0;
                 m_curr_tile.m_x <= m_tile_end.m_x;
                 m_curr_tile.m_x += kGridSize, m_curr_meta_index.m_x++) {

                if (m_action) {
                    m_action(this, m_action_ref);
                }
                log << "index: " << m_curr_index << ": " << m_curr_tile << std::endl;

                m_curr_index++;
            }
        }
    }


    bool GeoMetaTileRange::setStartIndex(int64_t start_index) noexcept {

        if (start_index < 0 || start_index >= m_meta_tiles_needed) {
            // Invalid index, reset to default state.
            m_curr_index = 0;
            m_curr_tile = m_first_tile;
            m_curr_meta_index.set(0, 0);
            return false;
        }
        else {
            m_curr_index = start_index;

            // Compute the new meta tile position.
            int32_t tile_x_offset = static_cast<int32_t>(m_curr_index % m_horizontal_tile_n);
            int32_t tile_y_offset = static_cast<int32_t>(m_curr_index / m_horizontal_tile_n);

            m_curr_tile.m_x = m_first_tile.m_x + tile_x_offset * kGridSize;
            m_curr_tile.m_y = m_first_tile.m_y + tile_y_offset * kGridSize;

            m_curr_meta_index.set(tile_x_offset, tile_y_offset);

            m_reset_flag = false;

            return true;
        }
    }


    /**
     *  @brief Retrieves the next tile in the meta tile range iteration.
     *
     *  This method provides the position of the next tile in the range defined by the
     *  `GeoMetaTileRange` instance. The position is written to the provided `out_tile` parameter.
     *  The return value indicates whether there are more tiles to iterate over.
     *
     *  @param[out] out_tile_index A reference to a `Vec2d` object where the position of the next tile
     *                             will be stored if a tile is available.
     *  @return `true` if there are more tiles to iterate over, otherwise `false` when the
     *          iteration is complete.
     *
     *  @note The method updates an internal state to track iteration progress. Ensure the iteration
     *        state is correctly initialized before calling this method (e.g., by using `setByBounds`).
     *        This method can be used as an alternative to the `run` method.
     */
    bool GeoMetaTileRange::nextTilePos(int64_t end_index, Vec2i& out_tile_index) noexcept {

        if (m_reset_flag) {
            m_curr_index = 0;
            m_curr_tile = m_first_tile;
            m_curr_meta_index.set(0, 0);
            m_reset_flag = false;
        }
        else {
            m_curr_tile.m_x += kGridSize;
            m_curr_meta_index.m_x++;
            if (m_curr_meta_index.m_x >= m_horizontal_tile_n) {
                m_curr_meta_index.m_x = 0;
                m_curr_tile.m_x = m_first_tile.m_x;
                m_curr_tile.m_y += kGridSize;
                m_curr_meta_index.m_y++;
            }
            if (m_curr_meta_index.m_y >= m_vertical_tile_n) {
                out_tile_index.set(-1, -1);
                return false;
            }

            m_curr_index++;
        }

        out_tile_index.m_x = m_curr_tile.m_x;
        out_tile_index.m_y = m_curr_tile.m_y;

        return m_curr_index < m_meta_tiles_needed;
    }


    void GeoMetaTileRange::wgs84EnvelopeBbox(RangeRectd& out_bbox) const noexcept {

        Vec2i tile_min = m_first_tile;
        Vec2i tile_max = m_first_tile;
        tile_max.translate(kGridSize * static_cast<int32_t>(m_horizontal_tile_n), kGridSize * static_cast<int32_t>(m_vertical_tile_n));

        Vec2d latlon1;
        Vec2d latlon2;
        Geo::wgs84FromTileIndex(m_zoom, tile_min.m_x, tile_max.m_y, latlon1.m_x, latlon1.m_y);
        Geo::wgs84FromTileIndex(m_zoom, tile_max.m_x, tile_min.m_y, latlon2.m_x, latlon2.m_y);

        out_bbox.m_min_x = latlon1.m_x;
        out_bbox.m_min_y = latlon1.m_y;
        out_bbox.m_max_x = latlon2.m_x;
        out_bbox.m_max_y = latlon2.m_y;
    }


} // End of namespace Grain
