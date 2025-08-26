//
//  GeoTileRenderer.hpp
//
//  Created by Roald Christesen on from 06.04.2024
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 24.08.2025
//

//  TODO: Find allready rendered tiles, do not render, if found!
//  TODO: $clipping Extend for borders, circles ... that covers bigger areas.
//  TODO: Render in layer and compose, for blur, glow, lines, effects etc.
//  TODO: Render GeoJSON layers
//  TODO: Render SQLite layers
//  TODO: Render WMS/WFS layers

#ifndef GrainGeoTileRenderer_hpp
#define GrainGeoTileRenderer_hpp

#include "Grain.hpp"
#include "Core/Log.hpp"
#include "Type/KeyValue.hpp"
#include "2d/RangeRect.hpp"
#include "2d/Border.hpp"
#include "2d/Dimension.hpp"
#include "String/String.hpp"
#include "String/StringList.hpp"
#include "String/CSVData.hpp"
#include "Geo/Geo.hpp"
#include "Geo/GeoProj.hpp"
#include "Geo/GeoShape.hpp"
#include "Image/Image.hpp"
#include "Graphic/Font.hpp"
#include "Graphic/GraphicContext.hpp"
#include "File/PolygonsFile.hpp"
#include "Scripting/Lua.hpp"
#include "Scripting/Toml.hpp"
#include "Database/PostgreSQL.hpp"

// #include "LuaBridge.h" // TODO: !!!!!
#include <libpq-fe.h>


namespace Grain {


// Forward references
    class GeoTileRenderer;


    enum class GeoTileDrawMode {
        Undefined = 0,
        Stroke,
        Fill,
        FillStroke,
        StrokeFill,
        TextAtPoint
    };

    enum class GeoTileDrawShape {
        Undefined = 0,
        Circle,
        Square
    };


    /**
     *  @brief Render settings.
     *
     *  Contains all settings for rendering.
     *
     *  @note This struct should only contain copyable data.
     */
    class GeoTileRendererDrawSettings {
        friend class GeoTileRenderer;
        friend class GeoTileRendererLayer;

    protected:
        enum {
            kMaxStrokeDashLength = 32
        };

        GeoTileDrawMode m_draw_mode = GeoTileDrawMode::Fill;
        GeoTileDrawShape m_point_shape = GeoTileDrawShape::Circle;
        RGB m_fill_color = { 1.0f, 1.0f, 1.0f };
        RGB m_stroke_color = { 0.0f, 0.0f, 0.0f };
        RGB m_text_color = { 0.1f, 0.1f, 0.1f };
        String m_font_name = "Default";             ///< Font name
        float m_font_size = 12;                     ///< Font size
        Font* m_font = nullptr;                     ///< Pointer to allocated font or `nullptr`
        double m_fill_opacity = 0.0;
        double m_stroke_opacity = 0.0;
        double m_text_opacity = 0.0;
        double m_stroke_width = 10.0;
        double m_stroke_px_min = 0.5;
        double m_stroke_px_max = 100.0;
        double m_stroke_px_fix = -1.0;
        double m_stroke_dash_array[kMaxStrokeDashLength] = {0};
        int32_t m_stroke_dash_length = 0;
        double m_radius = 5.0;
        double m_radius_px_min = 0.0;
        double m_radius_px_max = 100.0;
        double m_radius_px_fix = -1.0;

        double m_fill_extend_width = 0.0;
        double m_fill_extend_px_fix = -1.0;

        StrokeCapStyle m_stroke_cap_style = StrokeCapStyle::Round;
        StrokeJoinStyle m_stroke_join_style = StrokeJoinStyle::Round;
        double m_stroke_miter_limit = 4.0;


        // TODO: dash array!
        // TODO: Join End, Cap

        GraphicContext::BlendMode m_blend_mode = GraphicContext::BlendMode::Normal;

        float m_stroke_width_px;
        float m_radius_px;

    public:
        GeoTileRendererDrawSettings() {}
        ~GeoTileRendererDrawSettings() {
            if (m_font != nullptr) {
                // delete m_font;
            }
        }

        const Font* font(GeoTileRenderer* geo_renderer) noexcept;
    };


    class GeoTileRendererLayer : public Object {

        friend class GeoTileRenderer;

    public:
        enum {
            kMaxCustomFields = 100
        };

        enum class LayerType {
            Undefined = 0,
            PSQL,           ///< PSQL query to data in database
            Shape,          ///< ESRI Shape File Format
            Polygon,        ///< Grain Polygon File Format
            CSV             ///< Comma Separated Value File Format
        };

    protected:
        LayerType m_type = LayerType::PSQL;
        String m_type_name;
        String m_name;
        int32_t m_min_zoom = 1;                         ///< Start zoom level
        int32_t m_max_zoom = 20;                        ///< End zoom level
        bool m_resources_released_flag = false;         ///< Resources have been released

        int32_t m_srid;                                 ///< Spatial Reference System Identifier (SRID)
        bool m_ignore_proj = false;                     ///< Flag indicating, wether projection can be ignored

        String m_dir_path;                              ///< Directory path, eg. used when type is Shape or CSV
        String m_file_name;                             ///< File name, eg. used when type is Shape or CSV
        String m_used_file_path;                        ///< File path used

        String m_char_set = "UTF-8";

        String m_sql_identifier;                        ///< An optional identifier for a SQL connection (MySQL, PSQL, SQLite ...)
        String m_sql_query;                             ///< The SQL query
        String m_geometry_field;                        ///< Name of the geometry field to be used
        int32_t m_custom_field_count = 0;
        CSVDataColumnInfo* m_custom_field_infos = nullptr;

        String m_lua_script;                            ///< Stores the Lua script code
        bool m_has_lua_script = false;                  ///< Indicates whether a Lua script is present

        String m_draw_mode_name;
        String m_point_shape_name;

        GeoTileRendererDrawSettings m_draw_settings;


        GeoProj* m_proj = nullptr;                      ///< Projection for the layer

        // Data properties
        PSQLPropertyList* m_data_property_list = nullptr;

        // Database specific
        int32_t m_db_srid_field_index = -1;             ///< Index of field containing the SRID number
        int32_t m_db_wkb_field_index = -1;              ///< Index of field containing the WKB data

        bool m_db_field_names_scanned = false;          ///< List with all requested db fields scanned flag

        // Shape specific
        GeoShape* m_shape = nullptr;
        PolygonsFile* m_polygons_file = nullptr;

        // CSV specific
        int64_t m_csv_row_count{};          ///< Number of rows in CSV file
        int64_t m_csv_feature_count{};      ///< Number of valid features CSV file
        bool m_csv_ignore_header = false;   ///< Ignore header flag
        char m_csv_delimiter = ',';         ///< Delimiter
        char m_csv_quote = false;           ///< Quote

        int32_t m_x_field_index = -1;       ///< Used for accessing x coordinate values in CSV and other file formats
        int32_t m_y_field_index = -1;       ///< Used for accessing y coordinate values in CSV and other file formats
        double m_xy_scale = 1.0;            ///< Used for scaling x/y coordinate values
        int32_t m_radius_field_index = -1;  ///< Used for accessing radius values in CSV and other file formats

        CSVData m_csv_data;
        bool m_csv_must_read = true;

        // Statistics
        int64_t m_rendering_calls = 0;

        // Time measurement in nanoseconds
        int64_t m_total_data_access_time = 0;           ///< Database of File acccess time
        int64_t m_total_script_preparation_time = 0;    ///< Script (e.g. Lua) preparation time
        int64_t m_total_script_exec_time = 0;
        int64_t m_total_parse_time = 0;
        int64_t m_total_proj_time = 0;
        int64_t m_total_render_time = 0;

        int64_t m_total_db_rows_n = 0;      ///< Number of rows requested from the database
        int64_t m_total_point_n = 0;        ///< Number of points rendered
        int64_t m_total_stroke_n = 0;       ///< Number of strokes rendered
        int64_t m_total_fill_n = 0;         ///< Number of fills rendered
        int64_t m_total_text_n = 0;         ///< Number of texts rendered

        int64_t m_total_pos_out_of_range = 0;   ///< Number of coordinates out of range


    public:
        GeoTileRendererLayer();
        virtual ~GeoTileRendererLayer();

        friend std::ostream& operator << (std::ostream& os, const GeoTileRendererLayer* o) {
            // TODO: Complete this!
            os << "GeoTileRendererLayer:";
            if (o != nullptr) {
                os << "\n  sql: " << o->m_sql_query.length() << " bytes of data";
                os << "\n  fill-color: " << o->m_draw_settings.m_fill_color;
                os << "\n  stroke-color: " << o->m_draw_settings.m_stroke_color;
                os << "\n  stroke-width: " << o->m_draw_settings.m_stroke_width << " pixel";
            }
            else {
                os << " nullptr";
            }
            return os;
        }

        bool isShape() const noexcept { return m_type == LayerType::Shape; }
        bool isSQL() const noexcept { return m_type == LayerType::PSQL; }

        const char* nameStr() const noexcept { return m_name.utf8(); }
        const char* sqlIdendifierStr() const noexcept { return m_sql_identifier.utf8(); }


        ErrorCode checkProj(int32_t dst_srid);
    };


    class GeoTileRenderer {
        friend class GeoTileRendererDrawSettings;

    public:
        enum class RenderMode {
            Undefined = 0,
            Image,
            Tiles,
            MetaTiles,
            Animation
        };

        enum {
            kMinZoom = 0,
            kMaxZoom = 20,
            kDefaultSRID = 4326,
            kPower2MaxZoom = 524288,
            kMeterPerTileZoom0 = 40075008,
            kDetailTypeMaxStrLength = 128,

            kMetaTileGridSize = 8
        };

        enum {
            kErrTileSizeNotPowerOfTwo = 1,
            kErrTileSizeOutOfRange,
            kErrImageSizeOutOfRange,
            kErrUnknownRenderMode,
            kErrUnknownLayerType,
            kErrLayerFileNotFound,
            kErrLuaInitFailed,
            kErrInvalidBounds,
            kErrInvalidImagePadding,
            kErrUnknownOutputFileFormat,
            kErrUnsupportedImageOutputFileType,
            kErrShapeInstantiationFailed,
            kErrPolygonsFileInstantiationFailed,
            kErrPSQLConnectionMissing,
            kErrPSQLConnectionFailed,
            kErrPSQLQueryFailed,
            kErrUnsupportedWKBType,
            kErrLuaScriptError,
            kErrLuaScriptProcessFunctionMissing,
            kErrLuaScriptErrorUnexpectedResultFromProcessFunction,
            kErrDBMissingRequiredFields,
            kErrTileOutputPathNotFound,
            kErrDefaultRenderProjNotValid,
            kErrUnableToAllocateRenderImage,
            kErrUnableToAllocateTileImage,
            kErrRenderImageDoesNotExist,
            kErrUnknownCustomFieldType,
            kErrUnknownCustomFieldUsage,
            kErrUnknownRenderer,
            kErrGraphicsContextFailed
        };

        enum {
            kTomlErrTitle,
            kTomlErrRenderMode,
            kTomlErrRenderer,
            kTomlErrOutputFileName,
            kTomlErrOutputPath,
            kTomlErrOutputFileFormat,
            kTomlErrBounds,
            kTomlErrZoomMin,
            kTomlErrZoomMax,
            kTomlErrImageZoomLevel,
            kTomlErrZoomMismatch,
            kTomlErrPsqlDb,
            kTomlErrDestinationSRID,
            kTomlErrTileSize,
            kTomlErrImageSize,
            kTomlErrImagePadding,
            kTomlErrMapBackgroundColor,
            kTomlErrMapBackgroundOpacity,
            kTomlErrDefaultFillColor,
            kTomlErrDefaultStrokeColor,
            kTomlErrDefaultTextColor,
            kTomlErrNoLayers
        };

    public:
        // Configurable properties
        Toml m_toml;

        String m_title;                     ///< Title
        String m_config_path;               ///< Path to config file
        String m_output_path;               ///< Where rendered tiles will be saved
        String m_render_mode_name;
        int32_t m_tile_size = 256;          ///< Tile pixel size. Tiles have same height and width
        Dimensioni m_image_size = { 0, 0 }; ///< Image size, when rendering in image mode
        Borderd m_image_padding{};          ///< Image padding in pixels
        float m_image_quality = 0.8f;       ///< Image compression quality
        bool m_image_use_alpha = false;

        fourcc_t m_tile_order = 'row_';
        int32_t m_min_zoom = -1;            ///< Start zoom level, -1 means undefined
        int32_t m_max_zoom = -1;            ///< End zoom level, -1 means undefined
        RangeRectd m_bounding_box = { 0.0, 0.0, 0.0, 0.0 };   ///< Bounding box as lon/lat min and max values

        int32_t m_default_src_srid = 4326;          ///< Default source SRID
        int32_t m_dst_srid = 0;                     ///< Destination SRID
        GeoProj* m_default_render_proj = nullptr;   ///< Projection from default source CRS to destination CRS

        float m_map_bg_opacity = 1.0f;              ///< The background opacity, default is 100 % opaque, can be used for transparent maps for layering different aspects
        RGB m_map_bg_color;                         ///< The background color of the map, for earth, this would be the color for water
        RGB m_default_fill_color = { 0.5, 0.5, 0.5 };
        RGB m_default_stroke_color = { 0, 0, 0 };
        RGB m_default_text_color = { 0, 0, 0 };

        String m_default_font_name;
        float m_default_font_size = 12;

        int32_t m_color_n = 0;  // TODO: Create a new class for named colors?
        List<RGB> m_colors;
        StringList m_color_names;

        PSQLConnections m_psql_connections;
        ObjectList<GeoTileRendererLayer*> m_layers;    ///< A list with all layers to render

        RenderMode m_render_mode = RenderMode::Undefined;
        Dimensioni m_render_image_size = { 0, 0 };
        int32_t m_render_halo_size = 64;            ///< Extra pixels around the image to allow effects like blurring, shadows, or glow to extend beyond the image boundaries without visual artifacts
        Image* m_render_image = nullptr;
        Image* m_render_buffers[3]{nullptr};        ///< Immediate render buffers for rendering different aspects in separate buffers, then composing them into an image

        Vec2d m_render_lonlat_top_left;             ///< Top left corner as long/lat
        Vec2d m_render_lonlat_bottom_right;         ///< Bottom right corner as long/lat
        RangeRectd m_render_wgs84_bounding_box;     ///< Bounding box in WGS84 coordinates
        RangeRectd m_render_dst_bounding_box;       ///< Bounding box in destination SRID

        Vec2d m_render_top_left;                    ///< Top left corner in render coordinates
        Vec2d m_render_bottom_right;                ///< Bottom right corner in render coordinates
        String m_render_left_string;
        String m_render_right_string;
        String m_render_top_string;
        String m_render_bottom_string;

        double m_current_time;
        int32_t m_current_zoom;
        int32_t m_current_layer_index;

        double m_render_meter_per_pixel;

        ErrorCode m_conf_err = ErrorCode::None;

        // Errors
        String m_last_err_message;          // TODO: REMOVE!!! ///< Last error message

        // Lua
        Lua* m_lua = nullptr;
        int32_t m_lua_err_count = 0;        ///< Number of Lua script errors occured
        String m_last_lua_err;              ///< Last error from a Lua script
        int64_t m_current_element_count = 0;
        int64_t m_current_element_index = 0;

        // Database
        StringList m_sql_notices;           ///< Notices generated by sql query
        String m_last_failed_sql_query;     ///< Last failed SQL query
        String m_last_sql_err;              ///< Last error from a SQL query

        String m_renderer_name;             ///< Renderer to use, e.g. 'Cairo", 'MacCG"

        Image::FileType m_output_file_type;
        String m_output_file_name;          ///< File name without extension
        String m_output_file_format_name;   ///< File format. Can be png, jpg, tiff ...
        String m_output_file_ext;           ///< File extension, "png", "jpg" ...

        // Statistics
        int64_t m_total_render_time = 0;

        int64_t m_total_meta_tile_n = 0;
        int64_t m_total_tile_n = 0;
        int64_t m_total_db_rows_n = 0;
        int64_t m_total_point_n = 0;
        int64_t m_total_stroke_n = 0;
        int64_t m_total_fill_n = 0;

        File* m_log_file;


    public:
        GeoTileRenderer();
        ~GeoTileRenderer();

        friend std::ostream& operator << (std::ostream& os, const GeoTileRenderer* o) {
            o == nullptr ? os << "GeoTileRenderer nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const GeoTileRenderer& o) {
            Log l(os);
            l << "GeoTileRenderer:" << l.endl;
            l << "  title: " << o.m_title << l.endl;
            l << "  output-path: " << o.m_output_path << l.endl;
            l << "  tile-size: " << o.m_tile_size << l.endl;
            l << "  tile-order: " << l.fourCCValue(o.m_tile_order) << l.endl;
            l << "  zoom-min: " << o.m_min_zoom << l.endl;
            l << "  zoom-max: " << o.m_max_zoom << l.endl;
            l << "  default-src-srid: " << o.m_default_src_srid << l.endl;
            l << "  destination-srid: " << o.m_dst_srid << l.endl;
            l << "  bounds: " << o.m_bounding_box << l.endl;
            l << "  water-color: " << o.m_map_bg_color << l.endl;
            l << "  color count: " << o.m_colors.size() << l.endl;

            std::cout << o.m_psql_connections << std::endl;

            for (int32_t i = 0; i < o.m_layers.size(); i++) {
                std::cout << "  layer " << i << ":\n" << o.m_layers.elementAtIndex(i) << std::endl;
            }

            return os;
        }

        void setLastErrMessage(const String& message) noexcept;

        ErrorCode readConfigFromToml(const String& file_path) noexcept;
        void _configLayer(const TomlArrayItem& layer_item);

        bool setFileFormatByName(const String& file_format_name) noexcept;

        void setWaterColor(const RGB& color) noexcept { m_map_bg_color = color; }
        void setOutputPath(const String& output_path) noexcept { m_output_path = output_path; }
        void setZoomLevels(int32_t min_zoom, int32_t max_zoom) noexcept;
        void setBounds(double min_lon, double max_lon, double min_lat, double max_lat) noexcept;
        void setSourceSRID(int32_t srid) noexcept { m_default_src_srid = srid; }
        void setDestinationSRID(int32_t srid) noexcept { m_dst_srid = srid; }

        void setRenderMode(RenderMode render_mode) noexcept { m_render_mode = render_mode; }
        bool setRenderModeByName(const String& render_mode_name) noexcept {
            static const KeyIntPair items[] = {
                    { "tiles", (int32_t)RenderMode::Tiles },
                    { "meta-tiles", (int32_t)RenderMode::MetaTiles },
                    { "image", (int32_t)RenderMode::Image },
                    { "animation", (int32_t)RenderMode::Animation },
                    { nullptr, (int32_t)RenderMode::Undefined  }  // Sentinel item (end of list)
            };

            m_render_mode = (RenderMode)KeyIntPair::lookupValue(render_mode_name.utf8(), items);
            return m_render_mode == RenderMode::Undefined ? false : true;
        }
        void setRenderSize(int32_t width, int32_t height) noexcept {
            m_render_image_size.m_width = width;
            m_render_image_size.m_height = height;
        }

        void setRenderBoundsWGS84(const Vec2d& top_left, const Vec2d& bottom_right) noexcept;
        ErrorCode render() noexcept;
        void _updateMeterPerPixel() noexcept;
        void _renderLayers(GraphicContext& gc, RemapRectd& remap_rect);

        void _handleLuaError(int status, ErrorCode err);
        void _prepareLuaScriptForLayer(GeoTileRendererLayer* layer, GeoTileRendererDrawSettings* draw_settings, int64_t element_count);

        void _setLuaValueByPSQLProperty(const PSQLProperty* property);

        PSQLConnection* _psqlConnForLayer(GeoTileRendererLayer* layer);
        void _renderPSQLLayer(GeoTileRendererLayer* layer, GraphicContext& gc, RemapRectd& remap_rect);

        void _renderShapeLayer(GeoTileRendererLayer* layer, GraphicContext& gc, RemapRectd& remap_rect);

        void _renderPolygonLayer(GeoTileRendererLayer* layer, GraphicContext& gc, RemapRectd& remap_rect);
        void _releasePolygonLayerResouces(GeoTileRendererLayer* layer);

        void _renderCSVLayer(GeoTileRendererLayer* layer, GraphicContext& gc, RemapRectd& remap_rect);


        void _setupGCDrawing(GraphicContext& gc, GeoTileRendererDrawSettings& draw_settings);


        GeoTileRendererLayer* addLayer() noexcept;
        void addColor(const String& name, const RGB& color) noexcept;


        ErrorCode _initLua() noexcept;
        void _freeLua() noexcept;
        static void _lua_rgbFromStack(lua_State* l, int32_t offs, RGB& out_rgb);
        static int _lua_checkZoom(lua_State* l);
        static int _lua_getProperty(lua_State* l);
        static int _lua_setProperty(lua_State* l);

        ErrorCode startRenderer() noexcept;
        ErrorCode renderTiles() noexcept;
        ErrorCode renderStill() noexcept;


        // Utils

        void buildFilePath(const String& dir_path, const String& file_name, String& out_file_path) {
            if (dir_path.length() < 1) {
                out_file_path = m_config_path.fileDirPath() + "/" + file_name;
            }
            else {
                out_file_path = dir_path + "/" + file_name;
            }
            std::cout << "buildFilePath: " << out_file_path << std::endl;
        }

        double meterToPixel(double value) const noexcept {
            return m_render_meter_per_pixel > FLT_EPSILON ? value / m_render_meter_per_pixel : 0.0;
        }

        double meterToPixel(double value, double fix, double min, double max) const noexcept {
            if (fix > 0.0) {
                return fix;
            }
            else {
                double px = m_render_meter_per_pixel > FLT_EPSILON ? value / m_render_meter_per_pixel : 0.0;
                return px < min ? min : px > max ? max : px;
            }
        }



        static GeoTileDrawMode drawModeFromName(const char* name) noexcept {
            static const char* names[] = {
                    "stroke",
                    "fill",
                    "fill-stroke",
                    "stroke-fill",
                    "text-at-point",
                    nullptr
            };
            int32_t index = 0;
            while (names[index] != nullptr) {
                if (strcmp(name, names[index]) == 0) {
                    return (GeoTileDrawMode)(index + (int32_t)GeoTileDrawMode::Stroke);
                }
                index++;
            }
            return GeoTileDrawMode::Undefined;
        }

        static GeoTileDrawShape drawShapeFromName(const char* name) noexcept {
            static const char* names[] = { "circle", "square", nullptr };
            int32_t index = 0;
            while (names[index] != nullptr) {
                if (strcmp(name, names[index]) == 0) {
                    return (GeoTileDrawShape)(index + (int32_t)GeoTileDrawShape::Circle);
                }
                index++;
            }
            return GeoTileDrawShape::Undefined;
        }

        static bool drawModeHasFill(GeoTileDrawMode draw_mode) noexcept {
            return
                    draw_mode == GeoTileDrawMode::Fill ||
                    draw_mode == GeoTileDrawMode::FillStroke ||
                    draw_mode == GeoTileDrawMode::StrokeFill;
        }

        static bool drawModeHasStroke(GeoTileDrawMode draw_mode) noexcept {
            return
                    draw_mode == GeoTileDrawMode::Stroke ||
                    draw_mode == GeoTileDrawMode::FillStroke ||
                    draw_mode == GeoTileDrawMode::StrokeFill;
        }
    };


} // End of namespace Grain

#endif // GrainGeoTileRenderer_hpp
