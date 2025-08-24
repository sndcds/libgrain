//
//  GeoTileRenderer.cpp
//
//  Created by Roald Christesen on from 06.04.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

// TODO: Check!
// Handling of errors during reading an interpreting of the config file.
// Handling of errors during the layer rendering.
// Time statistics, layer render time, sql query time, file loading time ...
// stroke-offset
// stroke-pattern
// Animation
// Render series of images based on a CSV File with locations or bounding boxes.
// Check all statistics!
// File read error, output file path in error message!
// PDF- and SVG-Rendering, Cairo
// Option: Tiles nur rendern, sofern noch nicht als Datei vorhanden!
// When Lesen aus Datei oder SQL fehlt schlägt, dann kurze Pause (konfigurierbar) und neuer versuch N-Mal.


#include "Geo/GeoTileRenderer.hpp"
#include "Geo/WKBParser.hpp"
#include "App/App.hpp"
#include "Graphic/GraphicContext.hpp"
#include "2d/GraphicCompoundPath.hpp"
#include "File/File.hpp"
#include "String/CSVString.hpp"
#include "String/StringList.hpp"
#include "Color/Gradient.hpp"
#include "Type/Type.hpp"
#include "Geo/GeoMetaTile.hpp"
#include "Database/PostgreSQL.hpp"

#include <stdlib.h>
#include <algorithm>


namespace Grain {

    const Font* GeoTileRendererDrawSettings::font(GeoTileRenderer* geo_renderer) noexcept {
        if (!m_font) {
            std::cout << "GeoTileRendererDrawSettings::font nullptr, m_font_name: " << m_font_name << std::endl;
            if (m_font_name.compareIgnoreCase("default") == 0) {
                std::cout << ".... 1 geo_renderer->m_default_font_name: " << geo_renderer->m_default_font_name << std::endl;
                m_font = new (std::nothrow) Font(geo_renderer->m_default_font_name, m_font_size);
            }
            else {
                std::cout << ".... 2\n";
                m_font = new (std::nothrow) Font(m_font_name, m_font_size);
            }
            std::cout << ".... 3\n" << m_font << std::endl;
        }

        return m_font;
    }


    GeoTileRendererLayer::GeoTileRendererLayer() {
    }


    GeoTileRendererLayer::~GeoTileRendererLayer() {
        delete m_proj;
        delete m_shape;
        delete m_polygons_file;
        delete m_custom_field_infos;
        delete m_data_property_list;
    }


    ErrorCode GeoTileRendererLayer::checkProj(int32_t dst_srid) {
        if (!m_proj) {
            // TODO: No projection needed, if m_src_crs == dst_crs!

            m_proj = new GeoProj(m_srid, dst_srid);
            if (!m_proj->isValid()) {
                Exception::throwSpecific(GeoTileRenderer::kErrDefaultRenderProjNotValid);
            }
        }

        return ErrorCode::None;
    }


    GeoTileRenderer::GeoTileRenderer() {
        m_default_font_name = "Helvetica Neue";
    }


    GeoTileRenderer::~GeoTileRenderer() {
        m_layers.clear();

        delete m_default_render_proj;
        delete m_render_image;

        _freeLua();
    }


    void GeoTileRenderer::setLastErrMessage(const String &message) noexcept {
        m_last_err_message = message;
    }


    ErrorCode GeoTileRenderer::readConfigFromToml(const String &file_path) noexcept {
        m_conf_err = ErrorCode::None;

        // TODO: Inform about unknown keys.
        // TODO: Check values for allowed range.

        try {
            m_config_path = file_path;

            m_toml.parseFile(file_path, Toml::Option::FileIncludes);

            TomlTable config_table;
            m_toml.asTable(config_table);

            m_title = config_table.stringOrThrow("title", kTomlErrTitle);

            m_render_mode_name = config_table.stringOrThrow("render-mode", kTomlErrRenderMode);
            setRenderModeByName(m_render_mode_name);

            if (m_render_mode == RenderMode::Image) {
                m_output_file_name = config_table.stringOrThrow("output-file-name", kTomlErrOutputFileName);
            }

            m_min_zoom = (int32_t)config_table.integerOrThrow("zoom-min", kTomlErrZoomMin);
            m_max_zoom = (int32_t)config_table.integerOrThrow("zoom-max", kTomlErrZoomMax);
            m_current_zoom = (int32_t)config_table.integerOr("image-zoom-level", 10, kTomlErrImageZoomLevel);

            if (m_max_zoom < m_min_zoom) {
                Exception::throwSpecificFormattedMessage(
                        kTomlErrZoomMismatch,
                        "zoom-max (%d) < zoom-min (%d)",
                        m_max_zoom, m_min_zoom);
            }

            if (m_current_zoom < kMinZoom || m_current_zoom > kMaxZoom) {
                Exception::throwSpecificFormattedMessage(
                        kTomlErrZoomMismatch,
                        "image-zoom-level (%d) < zoom_min (%d) or > zoom_max (%d)",
                        m_current_zoom, m_min_zoom, m_max_zoom);
            }


            // TODO: Validate parameters!

            auto config = m_toml._ttpParseResult(); // TODO: REMOVE!

            // PostgreSQL connections
            {
                TomlArray psql_db_array;
                config_table.arrayOrThrow("psql-db", kTomlErrPsqlDb, psql_db_array);

                for (auto& item : psql_db_array) {
                    auto table = item.asTableOrThrow(kTomlErrPsqlDb);

                    auto conn = m_psql_connections.addConnection();
                    if (!conn) {
                        Exception::throwStandard(ErrorCode::MemCantAllocate);
                    }

                    conn->m_identifier = table.stringOr("identifier", "", kTomlErrPsqlDb);
                    conn->m_host = table.stringOr("host", "", kTomlErrPsqlDb);
                    conn->m_port = (int32_t)table.integerOr("port", 5432, kTomlErrPsqlDb);
                    conn->m_db_name = table.stringOrThrow("db-name", kTomlErrPsqlDb);
                    conn->m_user = table.stringOrThrow("user", kTomlErrPsqlDb);
                    conn->m_password = table.stringOr("password", "", kTomlErrPsqlDb);

                    // TODO: Validate parameters!
                }
            }

            m_tile_size = (int32_t)config_table.integerOrThrow("tile-size", kTomlErrTileSize);
            m_output_path = config_table.stringOrThrow("output-path", kTomlErrOutputPath);
            m_output_file_format_name = config_table.stringOr("output-file-format", "png", kTomlErrOutputFileFormat);
            m_output_file_type = Image::fileTypeByFormatName(m_output_file_format_name);
            if (!Image::isKnownFileType(m_output_file_type)) {
                Exception::throwSpecificFormattedMessage(
                        kTomlErrOutputFileFormat,
                        "Unknown file format with name \"%s\"",
                        m_output_file_format_name.utf8());
            }
            m_output_file_ext = Image::fileTypeExtension(m_output_file_type);

            m_image_size.m_width = (int32_t)config_table.integerOrThrow("image-width", kTomlErrImageSize);
            m_image_size.m_height = (int32_t)config_table.integerOrThrow("image-height", kTomlErrImageSize);

            // Output image padding
            {
                double values[4]{};
                int32_t n = config_table.doublesOrThrow("image-padding", kTomlErrImagePadding, 4, values);
                if (n > 0 && !m_image_padding.set(values, n)) {
                    Exception::throwSpecificFormattedMessage(
                            kTomlErrImagePadding,
                            "'image-padding' should have 1, 2 or 4 values, but has %d",
                            n);
                }
            }

            // Bounds
            {
                double values[4]{};
                int32_t n = config_table.doublesOrThrow("bounds", kTomlErrBounds, 4, values);
                if (n != 4 || values[0] >= values[2] || values[1] >= values[3]) {
                    Exception::throwSpecificFormattedMessage(
                            kTomlErrBounds,
                            "Accepted format for 'bounds': [min_lon, min_lat, max_lon, max_lat] where min_lon < max_lon and min_lat < max_lat");
                }
                m_bounding_box.set(values, 4);
            }

            m_dst_srid = (int32_t)config_table.integerOr("destination-srid", 3857, kTomlErrDestinationSRID);

            // TODO: Validate m_dst_srid

            m_map_bg_opacity = std::clamp(config_table.doubleOr("map-background-opacity", 1.0, kTomlErrMapBackgroundOpacity), 0.0, 1.0);
            m_map_bg_color = config_table.rgbOrThrow("map-background-color", kTomlErrMapBackgroundColor);
            m_default_fill_color = config_table.rgbOr("default-fill-color", RGB(0.8, 0.8, 0.8), kTomlErrDefaultFillColor);
            m_default_stroke_color = config_table.rgbOr("default-stroke-color", RGB(0, 0, 0), kTomlErrDefaultStrokeColor);
            m_default_text_color = config_table.rgbOr("default-text-color", RGB(0, 0, 0), kTomlErrDefaultTextColor);

            m_image_use_alpha = m_map_bg_opacity < (1.0f - FLT_EPSILON);

            // TODO: Use TomlTable from here .... !

            /* TODO: Read colors from table
            // Colors from table
            {
                const auto& colors = config["colors"];
                if (colors.is_table()) {
                    for (const auto& [key, value] : *colors.as_table()) {
                        String color_name = key.data();
                        RGB default_color;
                        RGB color;
                        m_toml._rgbByNode(color_name.utf8(), value, default_color, color);
                        addColor(color_name, color);
                    }
                }
                else {
                    m_last_err_message.setFormatted(1000, "'colors' should be a table.");
                    m_toml.throwParserError(m_last_err_message.utf8());
                }
            }
             */


            // Post adjust some values ...

            if (m_render_mode == RenderMode::Image) {
                auto w = m_bounding_box.width();
                auto h = m_bounding_box.height();
                m_bounding_box.m_min_x -= w * m_image_padding.left() / 100;
                m_bounding_box.m_max_x += w * m_image_padding.right() / 100;
                m_bounding_box.m_min_y -= h * m_image_padding.top() / 100;
                m_bounding_box.m_max_y += h * m_image_padding.bottom() / 100;
            }


            /*
            std::cout << "title: " << m_title << std::endl;
            std::cout << "render-mode: " << m_render_mode_string << std::endl;
            std::cout << "zoom-min: " << m_min_zoom << ", zoom-max: " << m_max_zoom << ", zoom-level: " << m_current_zoom << std::endl;
            std::cout << "tile-size: " << m_tile_size << " (px)" << std::endl;
            std::cout << "output-path: " << m_output_path << std::endl;
            std::cout << "output-file-format: " << m_output_file_format_name << std::endl;
            std::cout << "image-width, image-height: " << m_image_size << std::endl;
            std::cout << "image-padding: " << m_image_padding << std::endl;
            std::cout << "bounds: " << m_bounding_box << std::endl;
            std::cout << "destination-srid: " << m_dst_srid << std::endl;
            std::cout << "map-background-opacity: " << m_map_bg_opacity << std::endl;
            std::cout << "map-background-color: " << m_map_bg_color << std::endl;
            std::cout << "Custom Colors:" << std::endl;
            std::cout << "  number of colors: " << m_color_n << std::endl;
            for (int32_t i = 0; i < m_color_n; i++) {
                std::cout << "  " << i << ": " << m_color_names.stringAtIndex(i) << ", " << m_colors.elementAtIndex(i) << std::endl;
            }
             */

            // Confugure layers
            {
                auto layers = m_toml.arrayByNameOrThrow("layer", kTomlErrNoLayers);
                for (const auto& layer : layers) {
                    _configLayer(layer);
                }
            }
        }
        catch (const Exception& e) {
            std::cout << "Exception: " << e.message() << std::endl;
            m_conf_err = e.code();
        }

        return m_conf_err;
    }


    void GeoTileRenderer::_configLayer(const TomlArrayItem& layer_item) {
        auto layer_table = layer_item.asTableOrThrow(11111); // TODO: Message!

        auto layer = addLayer();
        if (!layer) {
            Exception::throwStandard(ErrorCode::MemCantAllocate);
        }

        // m_toml._printTableKeys(layer_node);

        layer->m_name = layer_table.stringOrThrow("name", 11111);
        std::cout << "layer->m_name: " << layer->m_name << std::endl;


        // Layer type
        layer->m_type_name = layer_table.stringOrThrow("type", 11111);
        std::cout << "layer->m_type_name: " << layer->m_type_name << std::endl;


        bool geometry_field_needed = false;
        bool dir_needed = false;
        bool file_needed = false;

        if (layer->m_type_name.compare("psql") == 0) {
            layer->m_type = GeoTileRendererLayer::LayerType::PSQL;
            layer->m_sql_query = layer_table.stringOrThrow("query", 11111);
            geometry_field_needed = true;
        }
        else if (layer->m_type_name.compare("shape") == 0) {
            layer->m_type = GeoTileRendererLayer::LayerType::Shape;
            dir_needed = file_needed = true;
        }
        else if (layer->m_type_name.compare("polygon") == 0) {
            layer->m_type = GeoTileRendererLayer::LayerType::Polygon;
            dir_needed = file_needed = true;
        }
        else if (layer->m_type_name.compare("csv") == 0) {
            layer->m_type = GeoTileRendererLayer::LayerType::CSV;
            dir_needed = file_needed = true;
        }
        else {
            Exception::throwSpecific(kErrUnknownLayerType);
        }


        if (dir_needed) {
            layer->m_dir_path = layer_table.stringOrThrow("dir", 11111);
        }
        else {
            layer->m_dir_path.clear();
        }

        if (file_needed) {
            layer->m_file_name = layer_table.stringOrThrow("file", 11111);

            buildFilePath(layer->m_dir_path, layer->m_file_name, layer->m_used_file_path);

            if (!File::fileExists(layer->m_used_file_path)) {
                m_last_err_message.setFormatted(1000, "'file' %s/%s not found in layer \"%s\".", layer->m_dir_path.utf8(), layer->m_file_name.utf8(), layer->m_name.utf8());
                m_toml.throwParserError(m_last_err_message.utf8());
            }
        }

        layer->m_sql_identifier = layer_table.stringOr("psql-identifier", "", 11111);

        if (geometry_field_needed) {
            layer->m_geometry_field = layer_table.stringOrThrow("geometry-field", 11111);
        }

        layer->m_csv_ignore_header = layer_table.booleanOr("ignore-header", false, 11111);


        layer->m_char_set = layer_table.stringOr("char-set", "UTF-8", 11111);

        // TODO: Check, if m_char_set is known! char-set
        // TODO: Check, if quote is known!


        String string = layer_table.stringOr("delimiter", ",", 11111);
        if (string.byteLength() != 1) {
            Exception::throwMessage(
                    ErrorCode::TomlParseError,
                    "'delimiter' must be a single ASCII character");
        }
        layer->m_csv_delimiter = string.firstAsciiChar();

        string = layer_table.stringOr("quote", "\"", 11111);
        if (string.byteLength() != 1) {
            Exception::throwMessage(
                    ErrorCode::TomlParseError,
                    "'quote' must be a single ASCII character");
        }
        layer->m_csv_quote = string.firstAsciiChar();


        layer->m_xy_scale = layer_table.doubleOr("xy-scale", 1.0, 11111);

        layer->m_radius_field_index = layer_table.integerOr("radius-field", -1, 11111);

        // TODO: Check lat, lon and radius field indices, must be >= 0 and < max number of custom fields.

        layer->m_min_zoom = (int32_t)layer_table.integerOr("zoom-min", kMinZoom, 11111);
        layer->m_max_zoom = (int32_t)layer_table.integerOr("zoom-max", kMaxZoom, 11111);
        if (layer->m_min_zoom < kMinZoom || layer->m_min_zoom > kMaxZoom ||
            layer->m_max_zoom < layer->m_min_zoom || layer->m_max_zoom > kMaxZoom) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlParseError,
                    "zoom level mismatch (zoom-min: %d, zoom-max: %d) in layer \"%s\".",
                    layer->m_min_zoom,
                    layer->m_max_zoom,
                    layer->m_name.utf8());
        }

        layer->m_srid = (int32_t)layer_table.integerOr("srid", kDefaultSRID, 11111);

        layer->m_draw_mode_name = layer_table.stringOrThrow("draw-mode", 11111);

        layer->m_draw_settings.m_draw_mode = drawModeFromName(layer->m_draw_mode_name.utf8());
        if (layer->m_draw_settings.m_draw_mode == GeoTileDrawMode::Undefined) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlParseError,
                    "draw-mode \"%s\" is unknown in layer \"%s\".",
                    layer->m_draw_mode_name.utf8(),
                    layer->m_name.utf8());
        }

        layer->m_point_shape_name = layer_table.stringOr("point-shape", "circle", 11111);
        layer->m_draw_settings.m_point_shape = drawShapeFromName(layer->m_point_shape_name.utf8());
        if (layer->m_draw_settings.m_point_shape == GeoTileDrawShape::Undefined) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlParseError,
                    "point-shape \"%s\" is unknown in layer \"%s\".",
                    layer->m_point_shape_name.utf8(),
                    layer->m_name.utf8());
        }


        layer->m_draw_settings.m_fill_color = layer_table.rgbOr("fill-color", m_default_fill_color, 11111);
        layer->m_draw_settings.m_fill_extend_width = layer_table.doubleOr("fill-extent-width", 0.0, 11111);
        layer->m_draw_settings.m_fill_extend_px_fix = layer_table.doubleOr("fill-extent-fix", -1.0, 11111);
        layer->m_draw_settings.m_fill_opacity = layer_table.doubleOr("fill-opacity", 1.0, 11111);

        layer->m_draw_settings.m_stroke_color = layer_table.rgbOr("stroke-color", m_default_stroke_color, 11111);
        layer->m_draw_settings.m_stroke_opacity = layer_table.doubleOr("stroke-opacity", 1.0, 11111);

        layer->m_draw_settings.m_stroke_width = layer_table.doubleOr("stroke-width", 10.0, 11111);
        layer->m_draw_settings.m_stroke_px_min = layer_table.doubleOr("stroke-px-min", 0.1, 11111);
        layer->m_draw_settings.m_stroke_px_max = layer_table.doubleOr("stroke-px-max", 10.0, 11111);
        layer->m_draw_settings.m_stroke_px_fix = layer_table.doubleOr("stroke-px-fix", -1.0, 11111);

        /* TODO: INCLUDE AGAIN!
        layer->m_draw_settings.m_stroke_dash_length = m_toml.allDoublesAt(layer_node, "stroke-dash", Toml::kOptional, GeoTileRendererDrawSettings::kMaxStrokeDashLength, layer->m_draw_settings.m_stroke_dash_array);
          */

        layer->m_draw_settings.m_text_color = layer_table.rgbOr("text-color", m_default_text_color, 11111);
        layer->m_draw_settings.m_text_opacity = layer_table.doubleOr("text-opacity", 1.0, 11111);

        layer->m_draw_settings.m_radius = layer_table.doubleOr("radius", 10.0, 11111);
        layer->m_draw_settings.m_radius_px_fix = layer_table.doubleOr("radius-px-fix", -1, 11111);
        layer->m_draw_settings.m_radius_px_min = layer_table.doubleOr("radius-px-min", 0.0, 11111);
        layer->m_draw_settings.m_radius_px_max = layer_table.doubleOr("radius-px-max", 1000000.0, 11111);


        // TODO: Check `radius`, must be >= 0.0.

        String blend_mode_name = layer_table.stringOr("blend-mode", "normal", 11111);
        layer->m_draw_settings.m_blend_mode = GraphicContext::blendModeByName(blend_mode_name.utf8());
        if (layer->m_draw_settings.m_blend_mode == GraphicContext::BlendMode::Undefined) {
            Exception::throwFormattedMessage(
                    ErrorCode::TomlParseError,
                    "blend-mode \"%s\" is unknown in layer \"%s\".",
                    blend_mode_name.utf8(),
                    layer->m_name.utf8());
        }

        layer->m_lua_script = layer_table.stringOr("script", "", 11111);
        layer->m_has_lua_script = layer->m_lua_script.length() > 0;

        // Custom fields

        std::cout << layer->m_name << " check custom-fields\n";
        if (layer_table.hasItem("custom-fields")) {

            std::cout << "custom-fields exists\n";

            TomlArray custom_fields;
            layer_table.arrayOrThrow("custom-fields", 11111, custom_fields);

            int32_t field_count = custom_fields.size();
            std::cout << "custom_fields count: " << field_count <<std::endl;
            layer->m_custom_field_infos = new CSVDataColumnInfo[field_count + 1];
            layer->m_custom_field_infos[field_count].m_index = -1;

            int32_t field_index = 0;
            for (const auto& field : custom_fields) {
                const auto field_description = field.asTableOrThrow(111111);

                int32_t index = (int32_t)field_description.integerOrThrow("index", 222222);
                String name = field_description.stringOrThrow("name", 222222);
                String type_name = field_description.stringOrThrow("type", 222222);
                String usage = field_description.stringOr("usage", "", 222222);

                layer->m_custom_field_infos[field_index].set(index, name.utf8(), type_name.utf8(), usage.utf8());

                if (layer->m_custom_field_infos[field_index].m_type == CSVDataColumnInfo::DataType::Unknown) {
                    Exception::throwFormattedMessage(
                            ErrorCode::TomlParseError,
                            "Unknown custom field type: \"%s\"",
                            type_name.utf8());
                }

                switch (layer->m_custom_field_infos[field_index].m_usage) {
                    case CSVDataColumnInfo::Usage::X:
                        layer->m_x_field_index = field_index;
                        std::cout << "layer->m_x_field_index: " << layer->m_x_field_index << std::endl;
                        break;
                    case CSVDataColumnInfo::Usage::Y:
                        layer->m_y_field_index = field_index;
                        std::cout << "layer->m_y_field_index: " << layer->m_y_field_index << std::endl;
                        break;
                    default:
                        // throw Error::specificError(kErrUnknownCustomFieldUsage);
                        break;
                }

                layer->m_custom_field_count++;
                field_index++;
            }
        }

        /* TODO: !!!! */
        std::cout << "Layer: " << std::endl;
        std::cout << "  name: " << layer->m_name << std::endl;
        std::cout << "  type: " << layer->m_type_name << std::endl;
        if (dir_needed) {
            std::cout << "  dir: " << layer->m_dir_path << std::endl;
        }
        if (file_needed) {
            std::cout << "  file: " << layer->m_file_name << std::endl;
        }
        if (geometry_field_needed) {
            std::cout << "  geometry-field: " << layer->m_geometry_field << std::endl;
        }
        std::cout << "  ignore-header: " << layer->m_csv_ignore_header << std::endl;
        std::cout << "  x-field, y-field: " << layer->m_x_field_index << ", " << layer->m_y_field_index << std::endl;
        std::cout << "  radius-field: " << layer->m_radius_field_index << std::endl;
        std::cout << "  zoom-min, zoom-max: " << layer->m_min_zoom << ", " << layer->m_max_zoom << std::endl;
        std::cout << "  srid: " << layer->m_srid << std::endl;

        std::cout << "  draw-mode: " << layer->m_draw_mode_name << std::endl;
        std::cout << "  point-shape: " << layer->m_point_shape_name << std::endl;
        std::cout << "  blend-mode: " << blend_mode_name << std::endl;
        std::cout << "  fill-color: " << layer->m_draw_settings.m_fill_color << std::endl;
        std::cout << "  fill-extent-width: " << layer->m_draw_settings.m_fill_extend_width << std::endl;
        std::cout << "  fill-extent-fix: " << layer->m_draw_settings.m_fill_extend_px_fix << std::endl;
        std::cout << "  fill-opacity: " << layer->m_draw_settings.m_fill_opacity << std::endl;
        std::cout << "  stroke-color: " << layer->m_draw_settings.m_stroke_color << std::endl;
        std::cout << "  stroke-opacity: " << layer->m_draw_settings.m_stroke_opacity << std::endl;
        std::cout << "  stroke-width: " << layer->m_draw_settings.m_stroke_width << std::endl;
        std::cout << "  stroke-px-min: " << layer->m_draw_settings.m_stroke_px_min << std::endl;
        std::cout << "  stroke-px-max: " << layer->m_draw_settings.m_stroke_px_max << std::endl;
        std::cout << "  stroke-px-fix: " << layer->m_draw_settings.m_stroke_px_fix << std::endl;
        std::cout << "  stroke-dash-length: " << layer->m_draw_settings.m_stroke_dash_length << std::endl;
        for (int32_t i = 0; i < layer->m_draw_settings.m_stroke_dash_length; i++) {
            std::cout << "    " << i << ": " << layer->m_draw_settings.m_stroke_dash_array[i] << std::endl;
        }

        std::cout << "  text-color: " << layer->m_draw_settings.m_text_color << std::endl;
        std::cout << "  text-opacity: " << layer->m_draw_settings.m_text_opacity << std::endl;
        std::cout << "  radius: " << layer->m_draw_settings.m_radius << std::endl;
        std::cout << "  has Lua script: " << layer->m_has_lua_script << std::endl;
        //


        // TODO: Script variablen austauschen!
        //   layer->m_setup_script.replace("style.fill", "superstyle->fill");
    }


    void GeoTileRenderer::setZoomLevels(int32_t min_zoom, int32_t max_zoom) noexcept {
        m_min_zoom = min_zoom;
        m_max_zoom = max_zoom;
    }


    void GeoTileRenderer::setBounds(double min_lat, double max_lat, double min_lon, double max_lon) noexcept {
        m_bounding_box.m_min_x = min_lat;
        m_bounding_box.m_max_x = max_lat;
        m_bounding_box.m_min_y = min_lon;
        m_bounding_box.m_max_y = max_lon;
    }


    void GeoTileRenderer::setRenderBoundsWGS84(const Vec2d &top_left, const Vec2d &bottom_right) noexcept {

        m_render_lonlat_top_left.m_x = std::clamp(top_left.m_x, Geo::kMinLonDeg, Geo::kMaxLonDeg);
        m_render_lonlat_bottom_right.m_x = std::clamp(bottom_right.m_x, Geo::kMinLonDeg, Geo::kMaxLonDeg);

        if (top_left.m_y > bottom_right.m_y) {
            m_render_lonlat_bottom_right.m_y = std::clamp(top_left.m_y, Geo::kMinLatDeg, Geo::kMaxLatDeg);
            m_render_lonlat_top_left.m_y = std::clamp(bottom_right.m_y, Geo::kMinLatDeg, Geo::kMaxLatDeg);
        }
        else {
            m_render_lonlat_top_left.m_y = std::clamp(top_left.m_y, Geo::kMinLatDeg, Geo::kMaxLatDeg);
            m_render_lonlat_bottom_right.m_y = std::clamp(bottom_right.m_y, Geo::kMinLatDeg, Geo::kMaxLatDeg);
        }

        m_render_wgs84_bounding_box.m_min_x = m_render_lonlat_top_left.m_x;
        m_render_wgs84_bounding_box.m_max_x = m_render_lonlat_bottom_right.m_x;
        m_render_wgs84_bounding_box.m_min_y = m_render_lonlat_top_left.m_y;
        m_render_wgs84_bounding_box.m_max_y = m_render_lonlat_bottom_right.m_y;
    }


    GeoTileRendererLayer *GeoTileRenderer::addLayer() noexcept {
        auto layer = new(std::nothrow) GeoTileRendererLayer();
        if (layer) {
            m_layers.push(layer);
        }

        return layer;
    }


    void GeoTileRenderer::addColor(const String& name, const RGB& color) noexcept {
        m_color_names.pushString(name);
        m_colors.push(color);
        m_color_n++;
    }


    /**
     *  @brief Check if zoom is in range.
     */
    int GeoTileRenderer::_lua_checkZoom(lua_State *l) {
        bool result = true;

        int arg_n = lua_gettop(l);  // Number of arguments
        auto tile_renderer = (GeoTileRenderer*)Lua::getGlobalPointer(l, "_tile_renderer_ptr");

        int64_t zoom_min = 0;
        int64_t zoom_max = 128;

        if (tile_renderer && arg_n >= 2) {
            Lua::integerFromStack(l, 1, zoom_min);
            Lua::integerFromStack(l, 2, zoom_max);

            if (tile_renderer->m_current_zoom < zoom_min || tile_renderer->m_current_zoom > zoom_max) {
                result = false;
            }
        }

        lua_pushboolean(l, result);  // Push the boolean result onto the Lua stack
        return 1;  // Return the number of results (1 boolean value in this case)
    }


    /**
     *  @brief Set a property.
     */
    int GeoTileRenderer::_lua_getProperty(lua_State *l) {
        // Check if the argument count is correct (1 argument expected)
        if (lua_gettop(l) != 1) {
            lua_pushstring(l, "Expected exactly one argument (the property name)");
            lua_error(l);
            return 0;  // Not reached, but makes it clear that error stops execution
        }

        // Check if the argument is a string (the property name)
        if (!lua_isstring(l, 1)) {
            lua_pushstring(l, "Expected a string as the first argument");
            lua_error(l);
            return 0;
        }

        // Get the property name from Lua
        const char *property_name = lua_tostring(l, 1);

        // Compare the property name and push the corresponding value
        auto renderer = (GeoTileRenderer *)Lua::getGlobalPointer(l, "rendererPointer");
        if (!renderer) {
            return 0;
        }

        renderer->m_layers.elementAtIndex(renderer->m_current_layer_index);

        // TODO: Implement!!!!!
        /*
        m_layers[m_current_layer_index];
                if (std::string(propertyName) == "intProperty") {
                    lua_pushinteger(l, m_propertyValueInt);  // Push the int value
                }
                else if (std::string(propertyName) == "doubleProperty") {
                    lua_pushnumber(l, m_propertyValueDouble);  // Push the double value
                }
                else if (std::string(propertyName) == "stringProperty") {
                    lua_pushstring(l, m_propertyValueString.c_str());  // Push the string value
                }
                else {
                    // If the property name does not match any known property, return an error
                    lua_pushstring(l, "Unknown property name");
                    lua_error(l);
                }

                // Return 1 to indicate that 1 value has been pushed onto the stack
                return 1;
            }
         */

        return 1;   // TODO: !!!!!
    }


    /**
     *  @brief Set a property.
     */
    int GeoTileRenderer::_lua_setProperty(lua_State *l) {
        int arg_n = lua_gettop(l);  // Number of arguments

        const char *name = luaL_checkstring(l, 1);

        auto tile_renderer = (GeoTileRenderer*)Lua::getGlobalPointer(l, "_tile_renderer_ptr");
        if (tile_renderer) {
        }

        auto draw_settings = (GeoTileRendererDrawSettings*)Lua::getGlobalPointer(l, "_map_renderer_draw_settings");
        if (draw_settings) {
            if (strcmp(name, "draw-mode") == 0) {
                const char *str = Lua::stringFromStack(l, 2);
                draw_settings->m_draw_mode = drawModeFromName(str);
            }
            else if (strcmp(name, "stroke-width") == 0) {
                Lua::doubleFromStack(l, 2, draw_settings->m_stroke_width);
            }
            else if (strcmp(name, "stroke-opacity") == 0) {
                Lua::doubleFromStack(l, 2, draw_settings->m_stroke_opacity);
            }
            else if (strcmp(name, "stroke-color") == 0) {
                Lua::rgbFromStack(l, arg_n, 2, draw_settings->m_stroke_color);
            }
            else if (strcmp(name, "fill-opacity") == 0) {
                Lua::doubleFromStack(l, 2, draw_settings->m_fill_opacity);
            }
            else if (strcmp(name, "fill-color") == 0) {
                Lua::rgbFromStack(l, arg_n, 2, draw_settings->m_fill_color);
            }
            else if (strcmp(name, "text-opacity") == 0) {
                Lua::doubleFromStack(l, 2, draw_settings->m_text_opacity);
            }
            else if (strcmp(name, "text-color") == 0) {
                Lua::rgbFromStack(l, arg_n, 2, draw_settings->m_text_color);
            }
            else if (strcmp(name, "radius") == 0) {
                Lua::doubleFromStack(l, 2, draw_settings->m_radius);
            }
            else if (strcmp(name, "blend-mode") == 0) {
                const char *str = Lua::stringFromStack(l, 2);
                draw_settings->m_blend_mode = GraphicContext::blendModeByName(str);
            }

            // TODO: Complete properties ...
        }
        else {
            // TODO: Handle error!
        }

        return 0;  // Number of return values
    }


    /**
     *  @brief Prepare for Lua support.
     */
    ErrorCode GeoTileRenderer::_initLua() noexcept {
        auto result = ErrorCode::None;

        try {
            m_lua = new Lua();
            if (!m_lua) {
                Exception::throwStandard(ErrorCode::LuaInstantiationFailed);
            }

            m_lua->setGlobalPointer("_tile_renderer_ptr", this);

            m_lua->addGlobalTable("map_renderer");
            m_lua->registerLuaFunction("map_renderer", "checkZoom", GeoTileRenderer::_lua_checkZoom);
            m_lua->registerLuaFunction("map_renderer", "setProperty", GeoTileRenderer::_lua_setProperty);

            m_lua->addGlobalTable("map_layer");
        }
        catch (const Exception& e) {
            std::cout << "GeoTileRenderer::_initLua() err: " << (int)e.code() << std::endl;
            result = e.code();
        }

        return result;
    }


    void GeoTileRenderer::_freeLua() noexcept {
        // delete m_lua;    // TODO: !!!!
    }


    ErrorCode GeoTileRenderer::startRenderer() noexcept {
        Timestamp ts;
        ErrorCode err;

        _initLua(); // TODO: Can be optimiezed to be called only if Lua scripts are involved!

        switch (m_render_mode) {
            case RenderMode::Tiles:
            case RenderMode::MetaTiles:
                err = renderTiles();
                break;

            case RenderMode::Image:
                err = renderStill();
                break;

            default:
                err = Error::specific(kErrUnknownRenderMode);
                break;
        }

        // Statistics
        m_total_render_time = ts.elapsedMilliseconds();

        // Verbose output
        // TODO: Verbose flags and variable output stream!

        std::cout << "***** Render statistics:\n *****";
        std::cout << "render mode: " << m_render_mode_name << std::endl;
        std::cout << "total render time: " << (0.001 * m_total_render_time) << " sec." << std::endl;

        std::cout << "database rows queried: " << m_total_db_rows_n << std::endl;
        std::cout << "rendered elements:\n";
        std::cout << "  points: " << m_total_point_n << std::endl;
        std::cout << "  strokes: " << m_total_stroke_n << std::endl;
        std::cout << "  fills: " << m_total_fill_n << std::endl;

        if (m_render_mode == RenderMode::Tiles) {
            std::cout << "total meta tiles: " << m_total_meta_tile_n << std::endl;
            std::cout << "total tiles: " << m_total_tile_n << std::endl;
        }


        std::cout << "Layers:\n";

        String text;
        int32_t index = 0;
        for (auto& layer : m_layers) {
            std::cout << index << ": " << layer->m_name;
            text.setElapsedTimeText(layer->m_total_data_access_time);
            std::cout << ", access: " << text;
            text.setElapsedTimeText(layer->m_total_script_preparation_time + layer->m_total_script_exec_time);
            std::cout << ", script: " << text;
            text.setElapsedTimeText(layer->m_total_render_time);
            std::cout << ", render: " << text << std::endl;
            index++;
        }

        std::cout << std::endl;

        // Cleanup
        delete m_render_image;
        m_render_image = nullptr;

        m_layers.clear();   // TODO: Release all layers!

        _freeLua();

        return ErrorCode::None;
    }


    /**
     *  @brief Render tiles.
     *
     *  Render meta-tiles an optionally split them into single tiles.
     *  Meta tiles are large tiles used in the rendering process of OpenStreetMap
     *  (OSM) data. Each meta tile is typically 2048 x 2048 pixels in size, covering
     *  an area equivalent to 8 x 8 ordinary OSM tiles.
     *  These ordinary OSM tiles, typically sized at 256 x 256 pixels each, make up
     *  the underlying grid of the map.
     *  Meta tiles allow for more efficient rendering by processing larger areas at
     *  once, reducing the number of individual tile requests.
     */
    ErrorCode GeoTileRenderer::renderTiles() noexcept {
        auto result = ErrorCode::None;

        Image *tile_image = nullptr;
        String meta_temp_dir;

        bool use_meta_tile = m_render_mode == RenderMode::MetaTiles;

        try {
            // Allocate image for a single tile, which will be saved to file

            tile_image = Image::createRGBAFloat(m_tile_size, m_tile_size);
            if (!tile_image) {
                Exception::throwSpecific(kErrUnableToAllocateTileImage);
            }

            // Render all zoom levels as configured
            for (m_current_zoom = m_min_zoom; m_current_zoom <= m_max_zoom; m_current_zoom++) {
                // Compute tile start and end information
                // Set the bounds, which area has to be rendered

                // TODO: Use MetaTileRange!!!!!!

                Vec2i tile_start, tile_end;
                Geo::wgs84ToTileIndex(m_current_zoom, m_bounding_box.m_min_x, m_bounding_box.m_max_y, tile_start.m_x, tile_start.m_y);
                Geo::wgs84ToTileIndex(m_current_zoom, m_bounding_box.m_max_x, m_bounding_box.m_min_y, tile_end.m_x, tile_end.m_y);

                int32_t tile_start_x = tile_start.m_x & (~0b111);
                int32_t tile_start_y = tile_start.m_y & (~0b111);

                // Special cases for zoom levels less than 3
                int32_t sn = 0;
                switch (m_current_zoom) {
                    case 0: sn = 1; break;
                    case 1: sn = 2; break;
                    case 2: sn = 4; break;
                    default: sn = kMetaTileGridSize; break;
                }

                int32_t x_iterations = (tile_end.m_x - tile_start_x) / kMetaTileGridSize + 1;
                int32_t y_iterations = (tile_end.m_y - tile_start_y) / kMetaTileGridSize + 1;
                int32_t meta_tiles_needed = x_iterations * y_iterations;
                int32_t meta_tile_n = 0;

                setRenderSize(m_tile_size * kMetaTileGridSize, m_tile_size * kMetaTileGridSize);

                // ImageAccess for tile image
                float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                ImageAccess tile_ia(tile_image, pixel);

                Vec2i tile_index;  // Current top left tile inside meta tile
                for (tile_index.m_y = tile_start_y; tile_index.m_y <= tile_end.m_y; tile_index.m_y += kMetaTileGridSize) {
                    for (tile_index.m_x = tile_start_x; tile_index.m_x <= tile_end.m_x; tile_index.m_x += kMetaTileGridSize) {
                        // TODO: !!!!
                        std::cout << tile_index << "meta-tiles needed: " << meta_tiles_needed << ", rendered: " << meta_tile_n << ", rest: " << (meta_tiles_needed - meta_tile_n) << std::endl;

                        // Preparation for rendering a single meta-tile
                        Vec2d lonlat;
                        Vec2d wgs84_top_left;
                        Vec2d wgs84_bottom_right;

                        Geo::wgs84FromTileIndex(m_current_zoom, tile_index, wgs84_top_left);
                        Geo::wgs84FromTileIndex(m_current_zoom, Vec2i(tile_index.m_x + kMetaTileGridSize, tile_index.m_y + kMetaTileGridSize), wgs84_bottom_right);

                        setRenderBoundsWGS84(wgs84_top_left, wgs84_bottom_right);

                        // Render the meta-tile, composed of 8 x 8 ordinary tiles

                        auto err = render();
                        if (err != ErrorCode::None) {
                            Exception::throwSpecific(1);    // TODO: !!!!!
                        }

                        // Split the meta-tile into 8 x 8 tiles and save to files as separate tiles
                        if (use_meta_tile) {
                            // For meta-tiles, create a temporary directory
                            meta_temp_dir = m_output_path + "/_temp_" + m_current_zoom + "_" + tile_index.m_y + "_" + tile_index.m_x;

                            std::cout << "meta_temp_dir: " << meta_temp_dir << std::endl;
                            if (!File::makeDirs(meta_temp_dir)) {
                                m_last_err_message.setFormatted(2560, "Temporary directory %s does not exist.", meta_temp_dir.utf8());
                                Exception::throwStandard(ErrorCode::FileDirNotFound);
                            }
                        }

                        ImageAccess meta_ia(m_render_image, pixel);

                        int32_t sub_tile_saved = 0;
                        for (int32_t sy = 0; sy < sn; sy++) {
                            for (int32_t sx = 0; sx < sn; sx++) {
                                Vec2i sub_tile = tile_index;
                                sub_tile.m_x = tile_index.m_x + sx;
                                sub_tile.m_y = tile_index.m_y + sy;

                                bool tile_explicit_needed =
                                        sub_tile.m_x >= tile_start.m_x &&
                                        sub_tile.m_x <= tile_end.m_x &&
                                        sub_tile.m_y >= tile_start.m_y &&
                                        sub_tile.m_y <= tile_end.m_y;

                                if (use_meta_tile || tile_explicit_needed) {
                                    meta_ia.setRegion(sx * m_tile_size, sy * m_tile_size, m_tile_size, m_tile_size);
                                    int32_t x = 0;
                                    int32_t y = 0;
                                    while (meta_ia.stepY()) {
                                        while (meta_ia.stepX()) {
                                            meta_ia.read();
                                            tile_ia.setPos(x, y);
                                            tile_ia.write();
                                            x++;
                                        }
                                        x = 0;
                                        y++;
                                    }

                                    // Build file path for the current sub-tile
                                    String dir_path;
                                    String file_name;
                                    String file_path;

                                    if (m_render_mode == RenderMode::Tiles) {  // Slippy map
                                        auto err = Geo::slippyTilePathForTile(m_output_path.utf8(), m_current_zoom, sub_tile, m_output_file_ext, dir_path, file_name);
                                        Exception::throwStandard(err);

                                        // Create necessary directories
                                        if (!File::makeDirs(dir_path)) {
                                            Exception::throwStandard(ErrorCode::FileDirNotFound);
                                        }

                                        file_path = dir_path + "/" + file_name;
                                    }
                                    else if (m_render_mode == RenderMode::MetaTiles) {
                                        file_path = meta_temp_dir + "/_tile_" + (sx + sy * 8) + "." + m_output_file_ext;
                                    }


                                    switch (m_output_file_type) {
                                        case Image::FileType::PNG:
                                            err = tile_image->writePng(file_path, m_image_quality, m_image_use_alpha);
                                            break;

                                        case Image::FileType::JPG:
                                            // TODO: Set compression parameters!
                                            err = tile_image->writeJpg(file_path, m_image_quality);
                                            break;

                                        case Image::FileType::WEBP:
                                            err = tile_image->writeWebP(file_path, m_image_quality, m_image_use_alpha);
                                            break;

                                        default:
                                            Exception::throwSpecific(kErrUnsupportedImageOutputFileType);
                                            break;
                                    }

                                    Exception::throwStandard(err);

                                    sub_tile_saved++;
                                    m_total_tile_n++;
                                }
                            }
                        }

                        if (use_meta_tile) {
                            // Collect all tiles in a meta-tile
                            String meta_dir_path;
                            String meta_file_name;
                            Geo::metaTilePathForTile(m_output_path, m_current_zoom, tile_index, "meta", meta_dir_path, meta_file_name);

                            GeoMetaTile::saveMetaTileFile(m_tile_order, m_current_zoom, tile_start_x, tile_start_y, meta_temp_dir, meta_dir_path + "/" + meta_file_name, "_tile_%d", m_output_file_ext, true);

                            if (File::removeDirAll(meta_temp_dir) != ErrorCode::None) {
                                // TODO: Eventually keep the temp files?
                            }
                        }

                        meta_tile_n++;
                        m_total_meta_tile_n++;
                    }
                }
            }
        }
        catch (const Exception& e) {
            std::cout << "ErrorCode: GeoTileRenderer::renderTiles() err: " << (int)e.code() << std::endl;
        }
        catch (...) {
            std::cout << "ErrorCode: GeoTileRenderer::renderTiles() unknown!" << std::endl;
            result = ErrorCode::Unknown;
        }

        // Cleanup
        delete tile_image;

        return result;
    }


    /**
     *  @brief Render image.
     *
     */
    ErrorCode GeoTileRenderer::renderStill() noexcept {
        auto result = ErrorCode::None;

        // TODO: Log über layer, die in der aktuellen Image Zoomstufe nicht gerendert werden!

        try {

            if (m_image_size.width() < 1 || m_image_size.height() < 1) {
                Exception::throwSpecific(kErrImageSizeOutOfRange);
            }

            // Set the bounds to the area which has to be rendered
            Vec2d top_left(m_bounding_box.m_min_x, m_bounding_box.m_max_y);
            Vec2d bottom_right(m_bounding_box.m_max_x, m_bounding_box.m_min_y);

            // Prepare and start rendering
            setRenderSize(m_image_size.width(), m_image_size.height());
            setRenderBoundsWGS84(top_left, bottom_right);

            auto err = render();
            if (err != ErrorCode::None) {
                Exception::throwSpecific(1);    // TODO: Code!!!!
            }

            String file_path = m_output_path + "/" + m_output_file_name + "." + m_output_file_ext;

            switch (m_output_file_type) {
                case Image::FileType::PNG:
                    m_render_image->writePng(file_path, m_image_quality, m_image_use_alpha);
                    break;

                case Image::FileType::JPG:
                    m_render_image->writeJpg(file_path, m_image_quality);
                    break;

                case Image::FileType::WEBP:
                    m_render_image->writeWebP(file_path, m_image_quality, m_image_use_alpha);
                    break;

                case Image::FileType::TIFF:
                    m_render_image->writeTypedTiff(file_path, Image::PixelType::UInt16);
                    // m_render_image->writeTiff(file_path, m_image_quality, m_image_use_alpha);
                    // TODO: Customize bit depth!
                    // m_render_image->writeTypedTiff(m_output_path + "/test_image_8bit.tiff", Type::Class::UInt8);
                    // m_render_image->writeTypedTiff(m_output_path + "/test_image_16bit.tiff", Type::Class::UInt16);
                    break;

                default:
                    Exception::throwSpecific(kErrUnsupportedImageOutputFileType);
            }
        }
        catch (const Exception& e) {
            result = e.code();
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        // Cleanup

        return result;
    }


    /**
     *  @brief Update internal variables for a zoom level.
     */
    void GeoTileRenderer::_updateMeterPerPixel() noexcept {
        if (m_render_mode == RenderMode::Tiles) {
            m_render_meter_per_pixel = (double)kMeterPerTileZoom0 / pow(2, m_current_zoom) / m_tile_size;
        }
        else {
            m_render_meter_per_pixel = Geo::haversineDistanceAtLat(m_bounding_box.centerY(), m_bounding_box.minX(), m_bounding_box.maxX(), Geo::kEarthRadius_m) / m_render_image_size.width();
        }

        std::cout << "m_render_meter_per_pixel: " << m_render_meter_per_pixel << std::endl;
    }


    /**
     *  @brief Start rendering.
     */
    ErrorCode GeoTileRenderer::render() noexcept {
        auto result = ErrorCode::None;

        try {
            if (!m_default_render_proj) {
                m_default_render_proj = new GeoProj(m_default_src_srid, m_dst_srid);
                if (!m_default_render_proj->isValid()) {
                    Exception::throwSpecific(kErrDefaultRenderProjNotValid);
                }
            }

            Rectd src_rect, dst_rect;

            m_default_render_proj->transform(m_render_lonlat_top_left, m_render_top_left);
            if (m_render_lonlat_bottom_right.m_y >= 180.0) {
                m_render_lonlat_bottom_right.m_y = 180.0 - 0.00001;
            }
            m_default_render_proj->transform(m_render_lonlat_bottom_right, m_render_bottom_right);

            m_render_dst_bounding_box.m_min_x = m_render_top_left.m_x;
            m_render_dst_bounding_box.m_min_y = m_render_top_left.m_y;
            m_render_dst_bounding_box.m_max_x = m_render_bottom_right.m_x;
            m_render_dst_bounding_box.m_max_y = m_render_bottom_right.m_y;

            /* TODO: debug */
            std::cout << "GeoTileRenderer::render()" << std::endl;
            std::cout << "  m_render_dst_bounding_box: " << m_render_dst_bounding_box << std::endl;
            std::cout << "  m_render_lonlat_top_left: " << m_render_lonlat_top_left << std::endl;
            std::cout << "  m_render_lonlat_bottom_right: " << m_render_lonlat_bottom_right << std::endl;
            std::cout << "  m_render_top_left: " << m_render_top_left << std::endl;
            std::cout << "  m_render_bottom_right: " << m_render_bottom_right << std::endl;

            m_render_left_string += m_render_top_left.m_x;
            m_render_right_string += m_render_bottom_right.m_x;
            m_render_top_string += m_render_top_left.m_y;
            m_render_bottom_string += m_render_bottom_right.m_y;

            // m_render_left_string.setFormatted(100, "%.8f", m_render_top_left.m_x);
            // m_render_right_string.setFormatted(100, "%.8f", m_render_bottom_right.m_x);
            // m_render_top_string.setFormatted(100, "%.8f", m_render_top_left.m_y);
            // m_render_bottom_string.setFormatted(100, "%.8f", m_render_bottom_right.m_y);

            std::cout << "  m_render_left_string: " << m_render_left_string << std::endl;
            std::cout << "  m_render_right_string: " << m_render_right_string << std::endl;
            std::cout << "  m_render_top_string: " << m_render_top_string << std::endl;
            std::cout << "  m_render_bottom_string: " << m_render_bottom_string << std::endl;


            src_rect.m_x = m_render_top_left.m_x;
            src_rect.m_y = m_render_top_left.m_y;
            src_rect.m_width = m_render_bottom_right.m_x - m_render_top_left.m_x;
            src_rect.m_height = m_render_bottom_right.m_y - m_render_top_left.m_y;

            std::cout << "  src_rect: " << src_rect << std::endl;

            dst_rect.m_x = 0;
            dst_rect.m_width = m_render_image_size.width();
            dst_rect.m_height = dst_rect.m_width * m_render_image_size.aspectRatio();
            dst_rect.m_y = -0.5 * (dst_rect.m_height - m_render_image_size.height());

            if (dst_rect.m_y < 0.0) {
                dst_rect.m_y = 0.0;
                dst_rect.m_height = m_render_image_size.height();
                dst_rect.m_width = dst_rect.m_height / m_render_image_size.aspectRatio();
                dst_rect.m_x = -0.5 * (dst_rect.m_width - m_render_image_size.width());
            }

            std::cout << "  dst_rect: " << dst_rect << std::endl; // TODO: Debug

            if (m_render_mode == RenderMode::Tiles) {
                if (m_current_zoom < 3) {
                    switch (m_current_zoom) {
                        case 2: dst_rect.scaleSize(1.0 / 2); break;
                        case 1: dst_rect.scaleSize(1.0 / 4); break;
                        case 0: dst_rect.scaleSize(1.0 / 8); break;
                    }
                }
            }
            else if (m_render_mode == RenderMode::Image) {
                auto fitted_rect = src_rect.fitRect(dst_rect, FitMode::Cover);
                double w_ratio = std::fabs(fitted_rect.m_width) / dst_rect.m_width;
                double h_ratio = std::fabs(fitted_rect.m_height) / dst_rect.m_height;

                std::cout << "  fitted_rect: " << fitted_rect << std::endl; // TODO: Debug
                std::cout << "  w_ratio: " << w_ratio << std::endl; // TODO: Debug
                std::cout << "  h_ratio: " << h_ratio << std::endl; // TODO: Debug

                if (w_ratio > h_ratio) {
                    dst_rect.m_height /= w_ratio;
                }
                else {
                    dst_rect.m_width /= h_ratio;
                }

                setRenderSize(dst_rect.m_width, dst_rect.m_height);
                // std::cout << "  dst_rect: " << dst_rect << std::endl; // TODO: Debug
            }

            RemapRectd remap_rect(src_rect, dst_rect, true);

            _updateMeterPerPixel();

            if (!m_render_image) {
                m_render_image = Image::createRGBAFloat(m_render_image_size.width(), m_render_image_size.height());
                if (!m_render_image) {
                    Exception::throwSpecific(kErrUnableToAllocateRenderImage);
                }
            }

            std::cout << "m_render_image: " << m_render_image << std::endl;

            if (m_render_image->beginDraw()) {
                m_render_image->clear(RGBA(m_map_bg_color, m_map_bg_opacity));
                GraphicContext gc;
                gc.setImage(m_render_image);
                m_render_image->beginDraw();
                _renderLayers(gc, remap_rect);
                m_render_image->endDraw();
            }
        }
        catch (const Exception& e) {
            result = e.code();
            std::cerr << "GeoTileRenderer::render() err: " << (int)e.code() << std::endl; // TODO: Handle error!
        }

        // Cleanup

        return result;
    }


    /**
     *  @brief Render all layers.
     */
    void GeoTileRenderer::_renderLayers(GraphicContext &gc, RemapRectd &remap_rect) {
        m_current_layer_index = 0;

        std::cout << "Render layers at zoom level: " << m_current_zoom << std::endl;

        for (auto& layer : m_layers) {
            layer->m_ignore_proj = layer->m_srid == m_dst_srid;

            if (m_current_zoom >= layer->m_min_zoom && m_current_zoom <= layer->m_max_zoom) {
                std::cout << "Layer " << m_current_layer_index << std::endl;
                std::cout << "  m_name: " << layer->m_name << std::endl;
                std::cout << "  m_type_name: " << layer->m_type_name << std::endl;
                std::cout << "  m_min_zoom: " << layer->m_min_zoom << std::endl;
                std::cout << "  m_max_zoom: " << layer->m_max_zoom << std::endl;
                std::cout << "  m_custom_field_count: " << layer->m_custom_field_count << std::endl;

                switch (layer->m_type) {
                    case GeoTileRendererLayer::LayerType::PSQL:
                        _renderPSQLLayer(layer, gc, remap_rect);
                        break;

                    case GeoTileRendererLayer::LayerType::Shape:
                        _renderShapeLayer(layer, gc, remap_rect);
                        break;

                    case GeoTileRendererLayer::LayerType::Polygon:
                        _renderPolygonLayer(layer, gc, remap_rect);
                        break;

                    case GeoTileRendererLayer::LayerType::CSV:
                        _renderCSVLayer(layer, gc, remap_rect);
                        break;

                    default:
                        break;
                }

                layer->m_rendering_calls++;
            }
            else if (!layer->m_resources_released_flag && m_current_zoom > layer->m_max_zoom) {
                // Release any resources, which not will be used anymore

                switch (layer->m_type) {
                    case GeoTileRendererLayer::LayerType::PSQL:
                        // TODO: Check!
                        break;

                    case GeoTileRendererLayer::LayerType::Shape:
                        // TODO: Check!
                        break;

                    case GeoTileRendererLayer::LayerType::Polygon:
                        _releasePolygonLayerResouces(layer);
                        break;

                    case GeoTileRendererLayer::LayerType::CSV:
                        // TODO: Check!
                        break;

                    default:
                        break;
                }

            }

            m_current_layer_index++;
        }
    }


    void GeoTileRenderer::_handleLuaError(int status, ErrorCode err) {
        if (status != LUA_OK) {
            m_lua_err_count++;
            m_last_lua_err = lua_tostring(m_lua->luaState(), -1);

            // Remove error message from the stack
            lua_pop(m_lua->luaState(), 1);

            throw err;
        }
    }


    /**
     *  @brief Prepare Lua script access in rendering function for a layer.
     */
    void GeoTileRenderer::_prepareLuaScriptForLayer(GeoTileRendererLayer *layer, GeoTileRendererDrawSettings *draw_settings, int64_t element_count) {
        if (!layer->m_has_lua_script) {
            // No Lua script is used in the layer, return immediately
            return;
        }

        Timestamp ts;

        // Load the Lua code
        int status = luaL_loadstring(m_lua->luaState(), layer->m_lua_script.utf8());
        _handleLuaError(status, Error::specific(kErrLuaScriptError));

        // Compiled chunk is now at the top of the stack, no need to pop unless you want to discard it
        status = lua_pcall(m_lua->luaState(), 0, 1, 0);
        std::cout << " lua_pcall status: " << status << std::endl;
        _handleLuaError(status, Error::specific(kErrLuaScriptError));

        m_lua->setGlobalPointer("_map_renderer_draw_settings", draw_settings);

        m_lua->openTable("map_renderer");

        lua_pushstring(m_lua->luaState(), "zoom");
        lua_pushinteger(m_lua->luaState(), m_current_zoom);
        lua_settable(m_lua->luaState(), -3);

        lua_pushstring(m_lua->luaState(), "time");
        lua_pushinteger(m_lua->luaState(), m_current_time);
        lua_settable(m_lua->luaState(), -3);

        lua_pushstring(m_lua->luaState(), "layer_index");
        lua_pushinteger(m_lua->luaState(), m_current_layer_index);
        lua_settable(m_lua->luaState(), -3);

        lua_pop(m_lua->luaState(), 1);

        m_lua->openTable("map_layer");

        lua_pushstring(m_lua->luaState(), "name");
        lua_pushstring(m_lua->luaState(), layer->m_name.utf8());
        lua_settable(m_lua->luaState(), -3);

        lua_pop(m_lua->luaState(), 1);

        layer->m_total_script_preparation_time += ts.elapsedMilliseconds();
    }


    void GeoTileRenderer::_setLuaValueByPSQLProperty(const PSQLProperty *property) {
        // TODO: Move to Lua class!?

        lua_pushstring(m_lua->luaState(), property->m_name.utf8());

        switch (property->m_type) {
            case PSQLPropertyType::Boolean:
                lua_pushboolean(m_lua->luaState(), property->m_integer != 0);
                break;

            case PSQLPropertyType::Integer:
                lua_pushinteger(m_lua->luaState(), property->m_integer);
                break;

            case PSQLPropertyType::Double:
                lua_pushnumber(m_lua->luaState(), property->m_double);
                break;

            case PSQLPropertyType::Numeric:
                lua_pushnumber(m_lua->luaState(), property->m_double);
                break;

            case PSQLPropertyType::String:
                lua_pushstring(m_lua->luaState(), property->m_string.utf8());
                break;

            default:
                lua_pushnil(m_lua->luaState());
                break;
        }

        lua_settable(m_lua->luaState(), -3);
    }


    /**
     *  @brief Get the PSQLConnection for a layer.
     */
    PSQLConnection *GeoTileRenderer::_psqlConnForLayer(GeoTileRendererLayer *layer) {
        PSQLConnection *psql_connection = m_psql_connections.connectionByIdentifier(layer->m_sql_identifier);
        if (!psql_connection) {
            psql_connection = m_psql_connections.firstConnection();
        }

        if (psql_connection) {
            auto err = psql_connection->open();
        }

        return psql_connection;
    }


    /**
     *  @brief Render a layer from WKB data in a PSQL database.
     */
    void GeoTileRenderer::_renderPSQLLayer(GeoTileRendererLayer *layer, GraphicContext &gc, RemapRectd &remap_rect) {
        auto result = ErrorCode::None;
        Timestamp timestamp;

        auto err = layer->checkProj(m_dst_srid);
        if (err != ErrorCode::None) {
            std::cout << "  layer->checkProj err: " << (int)err << std::endl;  // For debugging only
        }

        // Replace variables in SQL query
        String sql = layer->m_sql_query;
        if (layer->m_sql_query.find("{{") >= 0) {
            if (layer->m_srid == m_dst_srid) {
                sql.replace("{{clipping}}", "ST_Intersects({{geometry-field}}, ST_MakeEnvelope({{min-x}}, {{min-y}}, {{max-x}}, {{max-y}}, {{destination-srid}}))");
            }
            else {
                sql.replace("{{clipping}}", "ST_Intersects(ST_Transform({{geometry-field}}, {{destination-srid}}), ST_MakeEnvelope({{min-x}}, {{min-y}}, {{max-x}}, {{max-y}}, {{destination-srid}}))");
            }

            sql.replace("{{geometry-field}}", layer->m_geometry_field);

            sql.replace("{{min-x}}", m_render_left_string);
            sql.replace("{{max-x}}", m_render_right_string);
            sql.replace("{{min-y}}", m_render_top_string);
            sql.replace("{{max-y}}", m_render_bottom_string);

            char dst_srid_str[20];
            std::snprintf(dst_srid_str, 20, "%d", m_dst_srid);
            sql.replace("{{destination-srid}}", dst_srid_str);

            std::cout << sql << std::endl;

            // TODO: Replace other variables?
        }

        // std::cout << "SQL: " << sql << std::endl;  // For debugging only


        PSQLConnection *psql_connection = nullptr;
        PGresult *sql_result = nullptr;
        bool gc_saved_flag = false;

        try {

            // Execute SQL query, binary result
            psql_connection = _psqlConnForLayer(layer);
            if (!psql_connection) {
                Exception::throwSpecificFormattedMessage(
                        kErrPSQLConnectionMissing,
                        "Database connection missing for PSQL layer \"%s\", identifier: \"%s\"",
                        layer->nameStr(),
                        layer->sqlIdendifierStr());
            }

            auto psql_conn = (PGconn *)psql_connection->_m_pg_conn_ptr;
            if (PQstatus(psql_conn) != CONNECTION_OK) {
                Exception::throwSpecificFormattedMessage(
                        kErrPSQLConnectionFailed,
                        "Database connection failed for PSQL layer \"%s\", identifier: \"%s\"",
                        layer->nameStr(),
                        layer->sqlIdendifierStr());
            }

            sql_result = PQexecParams(psql_conn, sql.utf8(), 0, NULL, NULL, NULL, NULL, 1);

            if (PQresultStatus(sql_result) != PGRES_TUPLES_OK) {
                m_last_sql_err = PQerrorMessage(psql_conn);
                m_last_failed_sql_query = sql;
                std::cout << m_last_failed_sql_query << std::endl;
                Exception::throwSpecific(kErrPSQLQueryFailed);
            }


            auto row_count = PQntuples(sql_result);
            int32_t field_count = PQnfields(sql_result);

            // layer->m_total_data_access_time += timestamp.elapsedMilliseconds(); TODO: !!!!


            if (!layer->m_db_field_names_scanned) {
                // Scan all field names for accessing them in Lua scripts
                if (field_count > 0) {
                    layer->m_data_property_list = new PSQLPropertyList(field_count);
                    if (!layer->m_data_property_list) {
                        Exception::throwStandard(ErrorCode::MemCantAllocate);
                    }

                    for (int32_t field_index = 0; field_index < field_count; field_index++) {

                        auto field_name = PQfname(sql_result, field_index);  // Note: Does allways return the real name and not the alias

                        std::cout << "............. field_name: " << field_name << std::endl;

                        auto property = layer->m_data_property_list->mutPropertyPtrAtIndex(field_index);
                        property->m_name.set(field_name);
                        property->m_psql_type = (PSQLType)PQftype(sql_result, field_index);

                        if (strcmp(field_name, "wkb") == 0) {
                            layer->m_db_wkb_field_index = field_index;
                        }
                        else if (strcmp(field_name, "srid") == 0) {
                            layer->m_db_srid_field_index = field_index;
                        }
                    }
                }

                if (layer->m_db_wkb_field_index < 0 || layer->m_db_srid_field_index < 0) {
                    Exception::throwSpecific(kErrDBMissingRequiredFields);
                }

                layer->m_db_field_names_scanned = true;
            }

            layer->m_total_db_rows_n += row_count;
            m_total_db_rows_n += row_count;


            // Load Lua script, preparation, setup and process
            GeoTileRendererDrawSettings draw_settings;

            _prepareLuaScriptForLayer(layer, &draw_settings, row_count);

            // Rendering
            gc.save();
            gc_saved_flag = true;

            // Process each row
            int32_t wkb_field_index = layer->m_db_wkb_field_index;
            int32_t srid_field_index = layer->m_db_srid_field_index;

            /*
             *  `fill_extend_width` is used to render polygons without gaps between neighboring polygons.
             *  In some datasets, tiny gaps can appear between polygons when they are rendered with fill only.
             *  Rendering a border with the same color can help in this scenario.
             *  However, this approach does not work when the fill contains patterns, gradients, or transparency.
             */

            double fill_extend_width = meterToPixel(layer->m_draw_settings.m_fill_extend_width, layer->m_draw_settings.m_fill_extend_px_fix, 0.0, 1000.0);
            bool fill_extend_flag =  fill_extend_width > 0.005;

            for (int row_index = 0; row_index < row_count; row_index++) {
                // Initial draw settings from layer
                draw_settings = layer->m_draw_settings;

                m_current_element_index = row_index;

                // Get the WKB data
                char *wkb_data = PQgetvalue(sql_result, row_index, wkb_field_index);
                int32_t wkb_data_size = PQgetlength(sql_result, row_index, wkb_field_index);

                if (layer->m_has_lua_script) {
                    // Prepare for processing row through Lua script
                    m_lua->openTable("map_layer");

                    lua_pushstring(m_lua->luaState(), "row");
                    lua_pushinteger(m_lua->luaState(), row_index);
                    lua_settable(m_lua->luaState(), -3);

                    auto property_list = layer->m_data_property_list;

                    for (int32_t field_index = 0; field_index < field_count; field_index++) {
                        auto type = (PSQLType)PQftype(sql_result, field_index);
                        char *data = PQgetvalue(sql_result, row_index, field_index);
                        int32_t size = PQgetlength(sql_result, row_index, field_index);
                        if (PQgetisnull(sql_result, row_index, field_index)) {
                            type = PSQLType::Undefined;
                        }

                        if (field_index == wkb_field_index) {
                            // The WKB field will not be exposed to Lua
                            lua_pushlstring(m_lua->luaState(), data, size);  // Push WKB binary data as a Lua string
                            lua_setfield(m_lua->luaState(), -2, "wkb");      // Assign it to map_layer.wkb
                        }
                        else {
                            property_list->setPropertyAtIndexByPSQLBinaryData(field_index, type, data, size);
                            _setLuaValueByPSQLProperty(property_list->mutPropertyPtrAtIndex(field_index));
                        }
                    }

                    // After the loop, pop the 'properties' table off the stack
                    lua_pop(m_lua->luaState(), 1);

                    auto process_result = m_lua->callFunction("process");

                    // layer->m_total_script_exec_time += timestamp.elapsedMilliseconds();  TODO: !!!!!

                    if (process_result == 0) {
                        continue;  // This row doesn´t render, go to next row in loop
                    }
                }

                // WKB Parsing
                bool render_flag = false;
                bool render_as_point = false;
                bool render_as_path = false;

                GraphicCompoundPath compound_path;
                Vec2d point;

                {
                    WKBParser wkbParser;
                    wkbParser.setBinaryData((uint8_t*)wkb_data, wkb_data_size);

                    if (wkbParser.isPoint()) {
                        wkbParser.readVec2(point);
                        remap_rect.mapVec2(point);
                        render_flag = true;
                        render_as_point = true;
                    }
                    else if (wkbParser.isLineString() || wkbParser.isPolygon() || wkbParser.isMultiLineString() || wkbParser.isMultiPolygon()) {
                        compound_path.buildFromWKB(wkbParser, remap_rect);
                        render_flag = true;
                        render_as_path = true;
                    }
                    else {
                        m_last_err_message.setFormatted(1000, "Unsupported WKB type %s on layer.", wkbParser.typeName());
                        std::cout << m_last_err_message << std::endl;
                        Exception::throwSpecific(kErrUnsupportedWKBType);
                    }
                    // layer->m_total_parse_time += timestamp.elapsedMilliseconds();   // TODO: !!!!!
                }

                if (render_flag) {
                    _setupGCDrawing(gc, draw_settings);

                    int32_t fill_n = 0;  // Count fills
                    int32_t stroke_n = 0;  // Count strokes

                    if (render_as_point) {
                        switch (draw_settings.m_draw_mode) {
                            case GeoTileDrawMode::Fill:
                                gc.fillCircle(point, draw_settings.m_radius_px);
                                fill_n = 1;
                                break;

                            case GeoTileDrawMode::Stroke:
                                gc.strokeCircle(point, draw_settings.m_radius_px);
                                stroke_n = 1;
                                break;

                            case GeoTileDrawMode::FillStroke:
                                gc.fillCircle(point, draw_settings.m_radius_px);
                                gc.strokeCircle(point, draw_settings.m_radius_px);
                                fill_n = 1;
                                stroke_n = 1;
                                break;

                            case GeoTileDrawMode::StrokeFill:
                                gc.strokeCircle(point, draw_settings.m_radius_px);
                                gc.fillCircle(point, draw_settings.m_radius_px);
                                fill_n = 1;
                                stroke_n = 1;
                                break;

                            case GeoTileDrawMode::TextAtPoint: {
                                const char* str = layer->m_data_property_list->stringFromPropertyAtIndex(0);
                                if (str) {
                                    gc.drawText(str, point, layer->m_draw_settings.font(this), layer->m_draw_settings.m_text_color);
                                }
                                break;
                            }

                            default:
                                break;
                        }

                        layer->m_total_point_n++;
                    }
                    else if (render_as_path) {
                        // TODO: Stroke, Opacity

                        switch (draw_settings.m_draw_mode) {
                            case GeoTileDrawMode::Fill:
                                compound_path.fill(gc);
                                if (fill_extend_flag) {
                                    // Workaround to close gaps between polygons
                                    gc.setStrokeRGBAndAlpha(draw_settings.m_fill_color, draw_settings.m_fill_opacity);
                                    gc.setStrokeWidth(fill_extend_width);
                                    compound_path.stroke(gc);
                                }
                                fill_n = 1;
                                break;

                            case GeoTileDrawMode::Stroke:
                                compound_path.stroke(gc);
                                stroke_n = 1;
                                break;

                            case GeoTileDrawMode::FillStroke:
                                compound_path.fill(gc);
                                compound_path.stroke(gc);
                                fill_n = 1;
                                stroke_n = 1;
                                break;

                            case GeoTileDrawMode::StrokeFill:
                                compound_path.stroke(gc);
                                compound_path.fill(gc);
                                fill_n = 1;
                                stroke_n = 1;
                                break;

                            default:
                                break;
                        }
                    }

                    layer->m_total_fill_n += fill_n;
                    layer->m_total_stroke_n += stroke_n;

                    m_total_fill_n += fill_n;
                    m_total_stroke_n += stroke_n;
                }
            }
        }
        catch (const Exception& e) {
            std::cout << "Exception code: " << (int)e.code() << std::endl;
        }
        catch (...) {
            std::cout << "renderPSQLLayer err: Unknown\n";  // TODO:
            result = ErrorCode::Unknown;
        }

        // Cleanup
        if (gc_saved_flag) {
            gc.restore();
        }

        if (sql_result) {
            PQclear(sql_result);
        }

        if (psql_connection) {
            if ( psql_connection->m_psql_notices.size() > 0) {
                int32_t i = 0;
                for (auto notice : psql_connection->m_psql_notices) {
                    std::cout << i << ": " << notice << std::endl;
                    i++;
                }
            }
        }

        layer->m_total_render_time += timestamp.elapsedMilliseconds();

        Exception::throwStandard(result);
    }


    /**
     *  @brief Render a layer from data in a ESRI Shape file.
     */
    void GeoTileRenderer::_renderShapeLayer(GeoTileRendererLayer *layer, GraphicContext &gc, RemapRectd &remap_rect) {
        Timestamp timestamp;

        // Check if shape must be loaded

        if (!layer->m_shape) {
            layer->m_shape = new(std::nothrow) GeoShape();
            if (!layer->m_shape) {
                Exception::throwSpecific(kErrShapeInstantiationFailed);
            }

            auto err = layer->m_shape->initWithShapeAndProjection(layer->m_used_file_path, m_dst_srid);
            Exception::throwStandard(err);

            // TODO: Statistics!
        }

        auto shape = layer->m_shape;

        switch (layer->m_draw_settings.m_draw_mode) {
            case GeoTileDrawMode::Fill:
                shape->setDrawModeFill();
                break;

            case GeoTileDrawMode::Stroke:
                shape->setDrawModeStroke();
                break;

            case GeoTileDrawMode::FillStroke:
                shape->setDrawModeFillStroke();
                break;

            case GeoTileDrawMode::StrokeFill:
                shape->setDrawModeStrokeFill();
                break;

            default:
                break;
        }

        RGB fill_color = layer->m_draw_settings.m_fill_color;
        RGB stroke_color = layer->m_draw_settings.m_stroke_color;

        if (drawModeHasFill(layer->m_draw_settings.m_draw_mode)) {
            gc.setFillRGBAndAlpha(layer->m_draw_settings.m_fill_color, 1.0f);
        }

        _setupGCDrawing(gc, layer->m_draw_settings);

        gc.save();

        gc.setBlendMode(layer->m_draw_settings.m_blend_mode);
        shape->setPointRadius(layer->m_draw_settings.m_radius_px);
        shape->drawAll(gc, remap_rect);

        // TODO: Statistics!

        gc.restore();

        layer->m_total_render_time += timestamp.elapsedMilliseconds();
    }


    /**
     *  @brief Render a layer from data in a Grain Polygon File.
     */
    void GeoTileRenderer::_renderPolygonLayer(GeoTileRendererLayer *layer, GraphicContext &gc, RemapRectd &remap_rect) {
        Timestamp timestamp;

        // TODO: Check SRID/CRS ... what to do, if destination is different frm polygon files SRID?
        // Check if polygon must be loaded

        if (!layer->m_polygons_file) {
            // Load all polygon records
            String file_path = layer->m_dir_path + "/" + layer->m_file_name;
            layer->m_polygons_file = new(std::nothrow) PolygonsFile(file_path);
            if(!layer->m_polygons_file) {
                Exception::throwSpecific(kErrPolygonsFileInstantiationFailed);
            }
            auto err = layer->m_polygons_file->readInfo();
            Exception::throwStandard(err);

            err = layer->checkProj(m_dst_srid);
            Exception::throwStandard(err);

            // layer->m_total_data_access_time += timestamp.elapsedMilliseconds(); TODO: !!!!!

            std::cout << "Polygon: " << layer->m_polygons_file << std::endl;
        }

        auto polygons_file = layer->m_polygons_file;

        // Colors and other drawing parameters
        RGB fill_color = layer->m_draw_settings.m_fill_color;
        RGB stroke_color = layer->m_draw_settings.m_stroke_color;

        _setupGCDrawing(gc, layer->m_draw_settings);

        int32_t overlap_n = 0;
        for (int32_t polygon_index = 0; polygon_index < polygons_file->polygonCount(); polygon_index++) {
            auto entry = polygons_file->entryPtrAtIndex(polygon_index);
            if (!entry) {
                // TODO: !!!!
                m_last_err_message.setFormatted(1000, "GeoTileRenderer::_renderPolygonLayer() polygon_index: %d", polygon_index);
                std::cout << "polygon_index: " << polygon_index << std::endl;
                Exception::throwStandard(ErrorCode::Fatal);
            }

            bool overlaps = m_render_dst_bounding_box.overlaps(entry->m_bounding_box);

            if (overlaps) {
                polygons_file->setPos(entry->m_file_pos);

                int32_t part_indices[1000]; // TODO: Max amount?
                if (entry->m_part_count > 1000) {
                    Exception::throwStandard(ErrorCode::Fatal);
                }

                for (int32_t part_index = 0; part_index < entry->m_part_count; part_index++) {
                    part_indices[part_index] = polygons_file->readValue<int32_t>();
                }

                for (int32_t part_index = 0; part_index < entry->m_part_count; part_index++) {
                    int32_t point_count = 0;
                    if (part_index == entry->m_part_count - 1) {
                        point_count = entry->m_point_count - part_indices[part_index];
                    }
                    else {
                        point_count = part_indices[part_index + 1] - part_indices[part_index];
                    }

                    gc.beginPath();
                    for (int32_t point_index = 0; point_index < point_count; point_index++) {

                        Vec2d point;
                        point.readFromFile(*polygons_file);

                        // TODO: Transform, if necessary ... if (polygons->m_crs ...)

                        remap_rect.mapVec2(point);

                        if (point_index == 0) {
                            gc.moveTo(point);
                        }
                        else {
                            gc.lineTo(point);
                        }
                    }
                    gc.closePath();
                    gc.fillPath();  // TODO: fill, stroke, fill-stroke, stroke-fill, pattern, gradient ...
                }

                layer->m_total_fill_n++;
                overlap_n++;
            }
        }

        layer->m_total_render_time += timestamp.elapsedMilliseconds();
    }


    /**
     *  @brief Release resources for polygon layer.
     */
    void GeoTileRenderer::_releasePolygonLayerResouces(GeoTileRendererLayer *layer) {
        if (layer->m_polygons_file) {
            delete layer->m_polygons_file;
            layer->m_polygons_file = nullptr;
        }

        layer->m_resources_released_flag = true;
    }


    /**
     *  @brief Render a layer from CSV data.
     */
    void GeoTileRenderer::_renderCSVLayer(GeoTileRendererLayer *layer, GraphicContext &gc, RemapRectd &remap_rect) {
        Timestamp timestamp;

        // TODO: How can data be defined and become a property? Color, Size, ...
        // TODO: Statistics!
        // TODO: Optimize custom field data!

        int32_t fill_n = 0;     // Count fills
        int32_t stroke_n = 0;   // Count strokes

        // Check if CSV data must be loaded
        if (layer->m_csv_must_read) {
            try {
                layer->m_csv_data.setDelimiter(layer->m_csv_delimiter);
                layer->m_csv_data.setQuote(layer->m_csv_quote);

                auto err = layer->m_csv_data.createFromFile(layer->m_used_file_path, layer->m_custom_field_infos, layer->m_csv_ignore_header);
                Exception::throwStandard(err);
                layer->m_csv_feature_count = layer->m_csv_data.rowCount();
            }
            catch (const Exception& e) {
                std::cerr << "GeoTileRenderer::_renderCSVLayer err: " << (int)e.code() << std::endl;  // TODO: Error Handling!!!
            }

            ErrorCode err = layer->checkProj(m_dst_srid);
            Exception::throwStandard(err);

            layer->m_csv_must_read = false;
        }

        gc.save();

        RGB color;  // TODO: ...
        color.set24bit(0xFF5A08);  // TODO: ...
        color.set24bit(0xffff00);  // TODO: ...

        Gradient gradient; // TODO: ...
        gradient.addStop(0, RGBA(1, 0, 0, 1)); // TODO: ...
        gradient.addStop(0.5, RGBA(1, 0, 0, 0.5)); // TODO: ...
        gradient.addStop(1, RGBA(1, 0, 0, 0.0)); // TODO: ...

        int32_t x_field_index = layer->m_x_field_index;
        int32_t y_field_index = layer->m_y_field_index;
        int32_t radius_field_index = layer->m_radius_field_index;

        /* TODO: !!!!
        std::cout << "x_field_index: " << x_field_index << std::endl;
        std::cout << "y_field_index: " << y_field_index << std::endl;
        std::cout << "layer->m_custom_field_count: " << layer->m_custom_field_count << std::endl;
         */

        bool has_pos_fields = x_field_index >= 0 && x_field_index < layer->m_custom_field_count && y_field_index >= 0 && y_field_index < layer->m_custom_field_count;
        bool has_radius_field = radius_field_index >= 0;
        bool ignore_proj = layer->m_ignore_proj;
        bool src_is_4326 = layer->m_srid == 4326;

        Rectd render_rect = m_render_image->rect();


        GeoTileRendererDrawSettings draw_settings; // <<<<

        _prepareLuaScriptForLayer(layer, &draw_settings, layer->m_csv_feature_count);


        for (int64_t row_index = 0; row_index < layer->m_csv_feature_count; row_index++) {

            draw_settings = layer->m_draw_settings;
            Vec2d src_pos, pos;

            if (has_pos_fields) {
                src_pos.m_x = layer->m_csv_data.doubleValue(row_index, x_field_index);
                src_pos.m_y = layer->m_csv_data.doubleValue(row_index, y_field_index);
                src_pos *= layer->m_xy_scale;
                // std::cout << std::fixed << "src_pos: " << src_pos << std::endl; TODO: !!!

                if (src_is_4326 && !GeoProj::isWGS84Pos(src_pos)) {
                    layer->m_total_pos_out_of_range++;
                    // Ignore feature if `pos` is outside valid WGS84 lon/lat range
                    continue;
                }

                if (ignore_proj) {
                    pos = src_pos;
                }
                else {
                    layer->m_proj->transform(src_pos, pos);
                }
                // std::cout << std::fixed << "  projected pos: " << pos << std::endl; TODO: !!!

                remap_rect.mapVec2(pos);
                // std::cout << std::fixed << "  mapped pos: " << pos << std::endl; TODO: !!!
            }


            if (has_radius_field) {
                draw_settings.m_radius_px = layer->m_csv_data.doubleValue(row_index, radius_field_index);
            }

            if (layer->m_has_lua_script) {
                m_lua->openTable("map_layer");

                lua_pushstring(m_lua->luaState(), "row");
                lua_pushinteger(m_lua->luaState(), row_index);
                lua_settable(m_lua->luaState(), -3);

                // Loop to dynamically add properties
                int32_t field_count = layer->m_custom_field_count;

                for (int32_t field_index = 0; field_index < field_count; ++field_index) {
                    lua_pushstring(m_lua->luaState(), layer->m_custom_field_infos[field_index].m_key);

                    switch (layer->m_custom_field_infos[field_index].m_type) {
                        case CSVDataColumnInfo::DataType::Int64:
                            lua_pushinteger(m_lua->luaState(), layer->m_csv_data.int64Value(row_index, field_index));
                            break;

                        case CSVDataColumnInfo::DataType::Double:
                            lua_pushnumber(m_lua->luaState(), layer->m_csv_data.doubleValue(row_index, field_index));
                            break;

                        case CSVDataColumnInfo::DataType::String:
                            lua_pushstring(m_lua->luaState(), layer->m_csv_data.strValue(row_index, field_index));
                            break;

                        case CSVDataColumnInfo::DataType::WKB:
                            // TODO: Implement!
                            break;

                        default:
                            lua_pushnil(m_lua->luaState());
                            break;
                    }

                    lua_settable(m_lua->luaState(), -3);
                }


                /*
                auto property_list = layer->m_data_property_list;
                for (int32_t field_index = 0; field_index < field_count; field_index++) {

                    auto type = (PSQLType)PQftype(sql_result, field_index);
                    char *data = PQgetvalue(sql_result, row_index, field_index);
                    int32_t size = PQgetlength(sql_result, row_index, field_index);
                    if (PQgetisnull(sql_result, row_index, field_index)) {
                        type = PSQLType::Undefined;
                    }

                    if (field_index == wkb_field_index) {
                        // The WKB field will not be exposed to Lua
                        lua_pushlstring(m_lua->luaState(), data, size);  // Push WKB binary data as a Lua string
                        lua_setfield(m_lua->luaState(), -2, "wkb");      // Assign it to map_layer.wkb
                    }
                    else {
                        property_list->setPropertyAtIndexByPSQLBinaryData(field_index, type, data, size);
                        _setLuaValueByPSQLProperty(property_list->mutablePropertyPtrAtIndex(field_index));
                    }
                }

                // After the loop, pop the 'properties' table off the stack
                lua_pop(m_lua->luaState(), 1);
                */
                auto process_result = m_lua->callFunction("process");

                // layer->m_total_script_exec_time += timestamp.elapsedMilliseconds();  TODO: !!!!!

                if (process_result == 0) {
                    continue;  // This row doesn´t render, go to next row in loop
                }

                // TODO: Implement!!!!
                /*

                        bool skip_row = false;

                        m_current_element_index = row_index;

                        int32_t field_count = layer->m_csv_data.columCount();

                        // Set global variables for each data field in row, excluding WKB data

                        // TODO: Can this be once?

                        std::vector<int32_t> element_counts(field_count);  // Create storage for element counts

                        // Prepare Lua namespace only once
                        auto tile_namespace = luabridge::getGlobalNamespace(m_lua->luaState()).beginNamespace("tile");

                        // Loop to dynamically add properties
                        for (int32_t field_index = 0; field_index < field_count; ++field_index) {
                            std::string property_name = "element_count_" + std::to_string(field_index);

                            // Add each property with unique name and value reference
                            tile_namespace.addProperty(property_name.c_str(), &element_counts[field_index]);
                        }

                        // End the namespace after all properties are added
                        tile_namespace.endNamespace();


                        for (int32_t field_index = 0; field_index < field_count; field_index++) {
                            //!!!!    layer->m_csv_data.setLuaGlobal(m_lua, "layer_info", nullptr, row_index, field_index);
                        }


                        // Call process function in Lua script
                        luabridge::LuaRef process = luabridge::getGlobal(m_lua->luaState(), "process");
                        if (!process.isFunction()) {
                            Exception::throwStandard(ErrorCode::LuaInitFailed);
                        }

                        luabridge::LuaResult lua_result = luabridge::call(process);
                        if (!lua_result) {
                            std::cout << lua_result.errorMessage() << std::endl;  // TODO: !!!!
                            Exception::throwStandard(ErrorCode::LuaFailure);
                        }
                        bool process_result = lua_result[0];
                        if (!process_result) {
                            skip_row = true;
                        }


                        lua_getglobal(lua_state, "process");
                        if (lua_pcall(lua_state, 0, 1, 0) != LUA_OK) {
                            m_last_lua_err = lua_tostring(lua_state, -1);
                            lua_pop(lua_state, 1);  // Remove error message from the stack
                            std::cout << "m_last_lua_err 3: " << m_last_lua_err << std::endl;
                            throw Error::specificError(kErrLuaScriptError);
                        }
                        else {
                            bool process_result = false;
                            if (lua_isboolean(lua_state, -1)) {
                                process_result = lua_toboolean(lua_state, -1);
                            }
                            else {
                                throw Error::specificError(kErrLuaScriptErrorUnexpectedResultFromProcessFunc);
                            }
                            lua_pop(lua_state, 1);  // Remove the return value from the stack

                            if (!process_result) {
                                skip_row = true;
                            }
                        }

                        // layer->m_total_script_exec_time += timestamp.milliseconds(); TODO: !!!!!

                        if (skip_row) {
                            continue;  // This row doesn´t render, go to next row in loop
                        }
                        */
            }


            // TODO: Check, how bounds should be extended for correct rendering!
            double radius_px = meterToPixel(draw_settings.m_radius, draw_settings.m_radius_px_fix, draw_settings.m_radius_px_min, draw_settings.m_radius_px_max);
            Rectd bounds(pos.m_x - radius_px, pos.m_y - radius_px, radius_px * 2, radius_px * 2);

            if (bounds.intersect(render_rect)) {
                std::cout << pos << std::endl;
                Gradient gradient;
                gradient.addStop(0, RGB(1.0f, 0.5f, 0.3f));
                gradient.addStop(0.5, RGB(1, 1, 0));
                gradient.addStop(1, RGB(1, 0, 0));
                // gradient.lookupFromLUT(pow(Math::remapnorm(1, 4, bruecken_zustandsnote), 0.3), draw_settings.m_fill_color);   // TODO:!!!!

                // Initial draw settings from layer
                _setupGCDrawing(gc, draw_settings);

                switch (draw_settings.m_draw_mode) {
                    case GeoTileDrawMode::Stroke:
                        gc.strokeCircle(pos, draw_settings.m_radius_px);
                        stroke_n += 1;
                        break;

                    case GeoTileDrawMode::Fill:
                        gc.fillCircle(pos, draw_settings.m_radius_px);
                        fill_n += 1;
                        break;

                    case GeoTileDrawMode::FillStroke:
                        gc.fillCircle(pos, draw_settings.m_radius_px);
                        gc.strokeCircle(pos, draw_settings.m_radius_px);
                        fill_n += 1;
                        stroke_n += 1;
                        break;

                    case GeoTileDrawMode::StrokeFill:
                        gc.strokeCircle(pos, draw_settings.m_radius_px);
                        gc.fillCircle(pos, draw_settings.m_radius_px);
                        fill_n += 1;
                        stroke_n += 1;
                        break;

                    default:
                        break;
                }

                // gc.drawTextInt((int32_t)row_index, pos, App::uiFont(), RGB(0, 0, 0));
            }
        }

        layer->m_total_fill_n += fill_n;
        layer->m_total_fill_n += fill_n;
        layer->m_total_stroke_n += stroke_n;

        m_total_fill_n += fill_n;
        m_total_stroke_n += stroke_n;

        gc.setBlendMode(GraphicContext::BlendMode::Normal);
        gc.restore();

        layer->m_total_render_time += timestamp.elapsedMilliseconds();
    }


    void GeoTileRenderer::_setupGCDrawing(GraphicContext &gc, GeoTileRendererDrawSettings &draw_settings) {
        draw_settings.m_radius_px = meterToPixel(draw_settings.m_radius, draw_settings.m_radius_px_fix, draw_settings.m_radius_px_min, draw_settings.m_radius_px_max);

        if (drawModeHasFill(draw_settings.m_draw_mode)) {
            gc.setFillRGBAndAlpha(draw_settings.m_fill_color, draw_settings.m_fill_opacity);
        }

        if (drawModeHasStroke(draw_settings.m_draw_mode)) {
            draw_settings.m_stroke_width_px = meterToPixel(draw_settings.m_stroke_width, draw_settings.m_stroke_px_fix, draw_settings.m_stroke_px_min, draw_settings.m_stroke_px_max);

            gc.setStrokeRGBAndAlpha(draw_settings.m_stroke_color, draw_settings.m_stroke_opacity);
            gc.setStrokeWidth(draw_settings.m_stroke_width_px);
            gc.setStrokeCapStyle(draw_settings.m_stroke_cap_style);
            gc.setStrokeJoinStyle(draw_settings.m_stroke_join_style);
            gc.setStrokeMiterLimit(draw_settings.m_stroke_miter_limit);
            gc.setStrokeDash(draw_settings.m_stroke_dash_length, draw_settings.m_stroke_dash_array, draw_settings.m_stroke_width_px);
        }

        gc.setBlendMode(draw_settings.m_blend_mode);
    }


}  // End of namespace Grain
