//
//  Image.hpp
//
//  Created by Roald Christesen on 11.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 13.07.2025
//

#ifndef GrainImage_hpp
#define GrainImage_hpp

#include "Grain.hpp"
#include "Type/Object.hpp"
#include "Type/Type.hpp"
#include "Color/Color.hpp"
#include "Math/Vec2.hpp"
#include "Math/Mat3.hpp"
#include "2d/Dimension.hpp"
#include "Type/Range.hpp"
#include "2d/RangeRect.hpp"
#include "String/String.hpp"

#include "libraw/libraw.h"
#include <tiffio.h>


namespace Grain {

    class RGB;
    class RGBRamp;
    class RGBA;
    class Image;
    class GraphicContext;
    class Gradient;
    class ImageAccess;


    class Image : public Object {
        friend ImageAccess;

    public:
        enum {
            kErrUnsupportedBitDepth = 0,
            kErrUnsupportedColorModel,
            kErrNoColorSpace,
            kErrCGContextMissing,
            kErrUnableToCreateNSBitmapImageRep,
            kErrWebPEncodingFailed,
            kErrNoBufferForConversion
        };

        enum class FileType {
            Unknown = -1,
            PNG = 0,
            JPG,
            WEBP,
            TIFF,
            Count,
            First = 0,
            Last = TIFF
        };

        enum class PixelType {
            Undefined = -1,
            UInt8,
            UInt16,
            UInt32,
            Float
        };

        enum {
            kCFAPatternUnknown = 0,
            // Bayer pattern CFA modes
            kCFAPatternGRBG = 1,
            kCFAPatternRGGB = 2,
            kCFAPatternGBRG = 3,
            kCFAPatternBGGR = 4,

            kCFAPatternFirstBayer = 1,
            kCFAPatternLastBayer = 4,
        };

        // CFA pixel types
        enum {
            kCFA_G_R = 0,   ///< Green at red rows binary 00
            kCFA_G_B = 2,   ///< Green at blue rows binary 10
            kCFA_R = 1,     ///< binary 01
            kCFA_B = 3,     ///< binary 11
        };

        // CFA pixel mask bits
        enum {
            kCFAPixel_G_R = 1,      // 0001
            kCFAPixel_G_B = 4,      // 0100
            kCFAPixel_All_G = 5,    // 0101
            kCFAPixel_R = 2,        // 0010
            kCFAPixel_B = 8,        // 1000
            kCFAPixel_R_And_B = 10, // 1010
            kCFAPixel_Row_R = 3,    // 0011
            kCFAPixel_Row_B = 12,   // 1100
            kCFAPixel_Col_R = 6,    // 0110
            kCFAPixel_Col_B = 9,    // 1001
            kCFAPixel_All = 15,     // 1111
        };

    protected:
        Color::Model m_color_model = Color::Model::Undefined;
        PixelType m_pixel_type = PixelType::Undefined;
        PixelType m_png_fallback_pixel_type = PixelType::UInt8;
        int32_t m_width = 0;
        int32_t m_height = 0;
        bool m_float_type = false;
        bool m_has_alpha = false;
        bool m_compressed = false;
        int32_t _m_int_min = 0;
        int32_t _m_int_max = 0;
        float _m_float_min = 0.0f;
        float _m_float_max = 1.0f;
        uint16_t _m_bytes_per_component = 0;
        uint16_t _m_bits_per_component = 0;
        uint16_t _m_bytes_per_pixel = 0;
        uint32_t _m_pixel_count = 0;
        uint16_t _m_components_per_pixel = 0;
        size_t _m_mem_size = 0;                 ///< Size of pixel data memory in bytes
        uint32_t _m_pixel_data_step = 0;
        uint32_t _m_row_data_step = 0;
        uint64_t* _m_pixel_data = nullptr;

        // RAW meta data
        bool m_has_cam_to_xyz_matrix = false;
        bool m_has_rgb_to_cam_matrix = false;
        Mat3f m_cam_to_xyz_matrix{};            ///< Camera to XYZFile matrix
        Mat3f m_rgb_to_cam_matrix{};            ///< RGB to camera matrix

        bool m_geo_tiff_mode = false;
        int32_t m_geo_srid = 3857;
        List<Vec3d> m_tie_points;
        bool m_use_min_max_in_typed_tiff = false;
        Ranged m_value_range = { 0.0, 1.0 };

        ErrorCode m_last_err = ErrorCode::None;

        #if defined(__APPLE__) && defined(__MACH__)
            CGContextRef _m_cg_context_ref = nullptr;
            CGImageRef _m_cg_image_ref = nullptr;
        #endif

    public:
        Image() noexcept = default;
        explicit Image(const Image* image) noexcept;
        explicit Image(Image* image, int32_t width, int32_t height) noexcept;
        explicit Image(Color::Model color_model, int32_t width, int32_t height, PixelType pixel_type) noexcept;

        ~Image() noexcept override;


        [[nodiscard]] const char* className() const noexcept override { return "Image"; }

        friend std::ostream& operator << (std::ostream& os, const Image* o) {
            o == nullptr ? os << "Image nullptr" : os << *o;
            return os;
        }

        friend std::ostream& operator << (std::ostream& os, const Image& o) {
            os << "width x height: " << o.m_width << " x " << o.m_height << " pixel" << '\n';
            os << "float type: " << o.m_float_type << '\n';
            os << "has alpha: " << o.m_has_alpha << '\n';
            os << "bytes per component: " << o._m_bytes_per_component << '\n';
            os << "bits per component: " << o._m_bits_per_component << '\n';
            os << "bytes per pixel: " << o._m_bytes_per_pixel << '\n';
            os << "pixel count: " << o._m_pixel_count << '\n';
            os << "components per pixel: " << o._m_components_per_pixel << '\n';
            os << "mem size: " << o._m_mem_size << std::endl;
            return os;
        }


        [[nodiscard]] static Image* createLuminaFloat(int32_t width, int32_t height) noexcept {
            return new(std::nothrow) Image(Color::Model::Lumina, width, height, PixelType::Float);
        }

        [[nodiscard]] static Image* createLuminaAlphaFloat(int32_t width, int32_t height) noexcept {
            return new(std::nothrow) Image(Color::Model::LuminaAlpha, width, height, PixelType::Float);
        }

        [[nodiscard]] static Image* createRGBFloat(int32_t width, int32_t height) noexcept {
            return new(std::nothrow) Image(Color::Model::RGB, width, height, PixelType::Float);
        }

        [[nodiscard]] static Image* createRGBAFloat(int32_t width, int32_t height) noexcept {
            return new(std::nothrow) Image(Color::Model::RGBA, width, height, PixelType::Float);
        }


        [[nodiscard]] bool hasPixel() const noexcept { return _m_mem_size > 0 && _m_pixel_data != nullptr && m_width > 0 && m_height > 0; };
        [[nodiscard]] bool isUsable() const noexcept { return _m_mem_size > 0 && _m_pixel_data; };
        [[nodiscard]] bool hasAlpha() const noexcept { return m_has_alpha; }
        [[nodiscard]] bool isFloat() const noexcept { return m_float_type; }

        static int32_t maxComponentCount() { return 4; }

        [[nodiscard]] PixelType pixelType() const noexcept { return m_pixel_type; }
        [[nodiscard]] double defaultMaxLevel() const noexcept {
            switch (m_pixel_type) {
                case PixelType::UInt8:
                    return std::numeric_limits<uint8_t>::max();
                case PixelType::UInt16:
                    return std::numeric_limits<uint16_t>::max();
                case PixelType::UInt32:
                    return std::numeric_limits<uint32_t>::max();
                case PixelType::Float:
                default:
                    return 1.0;
            }
        }

        [[nodiscard]] Color::Model colorModel() const noexcept { return m_color_model; }

        [[nodiscard]] int32_t width() const noexcept { return m_width; }
        [[nodiscard]] int32_t height() const noexcept { return m_height; }
        [[nodiscard]] double diagonal() const noexcept { return Vec2d(m_width, m_height).length(); }
        [[nodiscard]] Dimensiond dimension() const noexcept { return Dimensiond(m_width, m_height); }
        [[nodiscard]] Vec2d center() const noexcept { return Vec2d(0.5 * m_width, 0.5 * m_height); }
        [[nodiscard]] Vec2d topLeft() const noexcept { return Vec2d(0.0, 0.0); }
        [[nodiscard]] Vec2d topRight() const noexcept { return Vec2d(m_width - 1, 0); }
        [[nodiscard]] Vec2d bottomRight() const noexcept { return Vec2d(m_width - 1, m_height - 1); }
        [[nodiscard]] Vec2d bottomLeft() const noexcept { return Vec2d(0.0, m_height - 1.0); }
        [[nodiscard]] double centerX() const noexcept { return m_width * 0.5; }
        [[nodiscard]] double centerY() const noexcept { return m_height * 0.5; }
        [[nodiscard]] Rectd rect() const noexcept { return Rectd(m_width, m_height); }

        [[nodiscard]] int64_t pixelCount() const noexcept { return _m_pixel_count; }
        [[nodiscard]] int64_t totalComponentCount() const noexcept { return _m_pixel_count * _m_components_per_pixel; }
        [[nodiscard]] int32_t componentCount() const noexcept { return _m_components_per_pixel; }
        [[nodiscard]] uint16_t bytesPerComponent() const noexcept { return _m_bytes_per_component; }
        [[nodiscard]] uint16_t bitsPerComponent() const noexcept { return _m_bits_per_component; }
        [[nodiscard]] uint16_t bitsPerPixel() const noexcept { return _m_components_per_pixel * _m_bits_per_component; }
        [[nodiscard]] uint16_t bytesPerPixel() const noexcept { return _m_bytes_per_pixel; }
        [[nodiscard]] uint32_t bytesPerRow() const noexcept { return _m_row_data_step; }
        [[nodiscard]] uint16_t componentsPerPixel() const noexcept { return _m_components_per_pixel; }
        [[nodiscard]] uint32_t pixelDataStep() const noexcept { return _m_pixel_data_step; };
        [[nodiscard]] size_t memSize() const noexcept { return _m_mem_size; };

        [[nodiscard]] const uint8_t* pixelDataPtr() const { return (uint8_t*)_m_pixel_data; }
        uint8_t* mutPixelDataPtr() { return (uint8_t*)_m_pixel_data; }

        uint8_t* pixelDataPtrAtRow(int32_t y) {
            if (y >= 0 && y < m_height) {
                return (uint8_t*)_m_pixel_data + y * _m_row_data_step;
            }
            else {
                return nullptr;
            }
        }



        [[nodiscard]] double aspectRatio() const { return m_height > 0 ? (double)m_width / (double)m_height : 1; }

        bool camToXYZMatrix(Mat3f& out_matrix) const noexcept;
        bool camToSRGBMatrix(Mat3f& out_matrix) const noexcept;


        void setGeoTiffMode() noexcept { m_use_min_max_in_typed_tiff = true; }
        [[nodiscard]] bool isGeoTiffMode() const noexcept { return m_use_min_max_in_typed_tiff; }

        void setGeoSrid(int32_t srid) noexcept { m_geo_srid = srid; }
        [[nodiscard]] int32_t geoSrid() const noexcept { return m_geo_srid; }

        void addTiePoint(const Vec3d& raster_pos, const Vec3d& model_pos) noexcept {
            try {
                m_tie_points.push(raster_pos);
                m_tie_points.push(model_pos);
            }
            catch (const std::exception& e) {
                // TODO: Message to app
            }
        }

        void tiePoint(int32_t index, Vec3d& raster_pos, Vec3d& model_pos) const noexcept {
            raster_pos = m_tie_points.elementAtIndex(index * 2);
            model_pos = m_tie_points.elementAtIndex(index * 2 + 1);
        }

        [[nodiscard]] int32_t tiePointCount() const noexcept { return (int32_t)m_tie_points.size() / 2; }

        void setSampleValueRange(double min, double max) noexcept {
            m_value_range.m_min = min;
            m_value_range.m_max = max;
            m_use_min_max_in_typed_tiff = true;
        }

        Ranged updateSampleValueRange() noexcept;
        [[nodiscard]] double minSampleValue() const noexcept { return m_value_range.m_min; }
        [[nodiscard]] double maxSampleValue() const noexcept { return m_value_range.m_max; }

        #if defined(__APPLE__) && defined(__MACH__)
            [[nodiscard]] CGBitmapInfo macos_cgBitmapInfo() const noexcept;
            CGImageRef macos_cgImageRef() noexcept {
                macos_buildCGImageRef();
                return _m_cg_image_ref;
            }
        #endif

        [[nodiscard]] bool sameSize(const Image* image) const noexcept;
        [[nodiscard]] bool sameFormat(const Image* image) const noexcept;


        bool beginDraw() noexcept;
        void endDraw() noexcept;
        bool graphicContext(GraphicContext* out_gc) noexcept;


        void clearBlack() noexcept;
        void clearWhite() noexcept;
        void clear(const RGB& color) noexcept;
        void clear(const RGBA& color) noexcept;
        void clear(float c0, float c1, float c2, float c3) noexcept;
        void clearAlpha(float alpha) noexcept;

        bool copyDataFromImage(Image* image);

        void fillHueWheelRect(float saturation = 1, float value = 0.7f) noexcept;
        void fillAudioLocationRect(const RGBRamp& color_ramp) noexcept;

        ErrorCode drawImage(Image* image, const Rectd& rect) noexcept;
        ErrorCode drawImage(Image* image) noexcept;

        void flipHorizontal() noexcept;
        void flipVertical() noexcept;


        void normalize() noexcept;
        void clampFloat() noexcept;
        void linearToGamma() noexcept;

        ErrorCode applyMatrix(const Mat3f& matrix) noexcept;
        ErrorCode applyFilter() noexcept;

        ErrorCode convolution(int32_t channel, const Dimensioni& kernel_size, const float* kernel_data, Image& out_image) noexcept;
        void floodFill(const Vec2i& pos, const RGB& color, Image& out_image) noexcept;


        [[nodiscard]] Image* extractRegion(const Recti& region) noexcept;
        ErrorCode downscale(Image* dst_image) noexcept;


        /* TODO !!!!!!
        bool copyToNSBitmapImageRep(NSBitmapImageRep* bitmap_rep);
        bool copyFromNSBitmapImageRep(NSBitmapImageRep* bitmap_rep);
        NSBitmapImageRep* createNSBitmapImageRep();
        NSBitmapImageRep* createNSBitmapImageRep1(bool use_alpha);
        NSBitmapImageRep* createNSBitmapImageRepWithBPS(int16_t bits_per_sample);
        */

#if defined(__APPLE__) && defined(__MACH__)
        bool macos_buildCGImageRef() noexcept;
        void macos_releaseCGImageRef() noexcept;
#endif

        ErrorCode writeImage(const String& file_path, fourcc_t type, float quality, bool use_alpha) noexcept;

        ErrorCode writeTiff(const String& file_path, float quality = 1.0f, bool use_alpha = true);
        ErrorCode writePng(const String& file_path, int32_t compression_level = 0, bool use_alpha = true);
        ErrorCode writeJpg(const String& file_path, float quality);
        ErrorCode writeWebP(const String& file_path, float quality, bool use_alpha);
        ErrorCode writeTypedTiff(const String& file_path, Image::PixelType pixel_type, bool drop_alpha = false) noexcept;

        ErrorCode writeCVF2File(const String& cvf2_file_path, int32_t srid, const RangeRectFix& bbox, LengthUnit length_unit, int32_t z_decimals, int32_t min_digits, int32_t max_digits) noexcept;


        [[nodiscard]] static FileType fileTypeByFormatName(const String& file_format_name) noexcept;
        [[nodiscard]] static bool isKnownFileType(FileType file_type) noexcept;
        [[nodiscard]] static const char* fileTypeExtension(FileType file_type) noexcept;

        ErrorCode copyImageData(const ImageAccess& src_image_access) noexcept;

        [[nodiscard]] static Image* createFromFile(const String& file_path, Image::PixelType pixel_type);
        [[nodiscard]] static Image* createFromRawFile(const String& file_path, Image::PixelType pixel_type);
        [[nodiscard]] Image* copyWithNewSettings(Color::Model color_model, PixelType pixel_type) noexcept;


        [[nodiscard]] static int32_t pixelTypeByteSize(PixelType pixel_type) {
            switch (pixel_type) {
                case PixelType::UInt8:
                    return 1;
                case PixelType::UInt16:
                    return 2;
                case PixelType::UInt32:;
                case PixelType::Float:
                    return 4;
                default:
                    return 0;
            }
        }

        [[nodiscard]] static int32_t pixelTypeBitCount(PixelType pixel_type) {
            switch (pixel_type) {
                case PixelType::UInt8:
                    return 8;
                case PixelType::UInt16:
                    return 16;
                case PixelType::UInt32:;
                case PixelType::Float: return 32;
                default:
                    return 0;
            }
        }

        [[nodiscard]] static DataType pixelTypeDataType(PixelType pixel_type) {
            switch (pixel_type) {
                case PixelType::UInt8:
                    return DataType::UInt8;
                case PixelType::UInt16:
                    return DataType::UInt16;
                case PixelType::UInt32:;
                    return DataType::UInt32;
                case PixelType::Float:
                    return DataType::Float;
                default:
                    return DataType::Undefined;
            }
        }

        void _set(Color::Model color_model, int32_t width, int32_t height, Image::PixelType pixel_type);
        void _malloc();
        void _free();
    };


    struct ImageAccessSetupInfo {
        int32_t m_width = 0;
        int32_t m_height = 0;
        Image::PixelType m_pixel_type = Image::PixelType::Undefined;
        Color::Model m_color_model = Color::Model::Undefined;
        int32_t m_component_count = 0;
        uint8_t* m_pixel_data_ptr = nullptr;
        uint32_t m_pixel_data_step = 0;
        uint32_t m_row_data_step = 0;
        uint32_t m_plane_data_step = 0;
    };

    class ImageAccess {
    protected:
        const Image* m_image = nullptr;
        Color::Model m_color_model = Color::Model::Undefined;
        Image::PixelType m_pixel_type = Image::PixelType::Undefined;
        int32_t m_component_count = 0;
        bool m_usable = false;

        int32_t m_x;               ///< Current pixel x position
        int32_t m_y;               ///< Current pixel y position
        int32_t m_width;
        int32_t m_height;
        int32_t m_region_x1;        ///< Start of region
        int32_t m_region_y1;
        int32_t m_region_x2;        ///< End of region
        int32_t m_region_y2;
        int32_t m_region_width;    ///< Dimension of region
        int32_t m_region_height;

        bool _m_x_loop_start, _m_y_loop_start;

        uint8_t* _m_pixel_data_ptr;
        uint8_t* _m_curr_ptr;
        uint32_t _m_pixel_data_step;
        uint32_t _m_row_data_step;
        uint32_t _m_plane_data_step;

        uint8_t* _m_value_ptr_u8;
        float* _m_value_ptr_float;

        float _m_component_values_float[4];
        uint8_t _m_component_values_u8[4];

        void (ImageAccess::*_m_transfer_write_func)();
        void (ImageAccess::*_m_transfer_read_func)();


    public:
        ImageAccess() = default;
        ImageAccess(Image* image, float* transfer_ptr = nullptr);

        ErrorCode setBySetupInfo(const ImageAccessSetupInfo& info);

        void undefine();

        bool isUsable() const { return m_usable; }

        inline int32_t x() const { return m_x; }
        inline float xNrm() const { return static_cast<float>(m_x) / m_width; }
        inline int32_t flippedX() const { return m_width - m_x - 1; }
        inline int32_t y() const { return m_y; }
        inline float yNrm() const { return static_cast<float>(m_y) / m_height; }
        inline int32_t flippedY() const { return m_height - m_y - 1; }
        inline int32_t width() const { return m_width; }
        inline int32_t height() const { return m_height; }
        inline void pos(Vec2i& out_pos) const { out_pos.m_x = m_x; out_pos.m_y = m_y; }
        inline void pos(Vec2d& out_pos) const { out_pos.m_x = m_x; out_pos.m_y = m_y; }
        inline int32_t regionWidth() const { return m_region_width; }
        inline int32_t regionHeight() const { return m_region_height; }

        double xFactor() const { return m_region_width > 0 ? (double)(m_x - m_region_x1) / (m_region_width - 1) : 1; }
        double yFactor() const { return m_region_height > 0 ? (double)(m_y - m_region_y1) / (m_region_height - 1) : 1; }

        inline bool isOddRow() const { return y() & 0x1; };
        inline bool isEvenRow() const { return !(y() & 0x1); };

        bool setX(int32_t x) { return setPos(x, y()); }
        bool setY(int32_t y) { return setPos(x(), y); }
        bool setPos(int32_t x, int32_t y);
        bool setPos(const ImageAccess& image_access) { return setPos(image_access.m_x, image_access.m_y); }
        bool setPos(const Vec2i& pos) { return setPos(pos.m_x, pos.m_y); }
        void resetRegion();
        void setRegion(int32_t x, int32_t y, int32_t width, int32_t height);
        void setRegion(const Recti& rect) { setRegion(rect.m_x, rect.m_y, rect.m_width, rect.m_height); }
        bool stepX();
        bool stepY();

        void setTransferPtr_u8(uint8_t* ptr);
        void setTransferPtr_r32(float* ptr);

        uint8_t* ptrAt(int32_t x, int32_t y);

        inline void read() { (this->*_m_transfer_read_func)(); }
        void readInterpolated(const Vec2d& pos);
        inline void write() { (this->*_m_transfer_write_func)(); }
        void clear();

        void setRGB(const Vec2i& pos, const RGB& color, float alpha = 1) noexcept;
        void setRGBInterpolated(const Vec2d& pos, const RGB& color, float alpha = 1) noexcept;

        void invert();


    private:
        void _updatePtr();

        void _transfer_dummy();

        void _transfer_r1_u8_to_u8();
        void _transfer_r1_u16_to_u8();
        void _transfer_r1_r32_to_u8();
        void _transfer_r2_u8_to_u8();
        void _transfer_r2_u16_to_u8();
        void _transfer_r2_r32_to_u8();
        void _transfer_r3_u8_to_u8();
        void _transfer_r3_u16_to_u8();
        void _transfer_r3_r32_to_u8();
        void _transfer_r4_u8_to_u8();
        void _transfer_r4_u16_to_u8();
        void _transfer_r4_r32_to_u8();

        void _transfer_r1_u8_to_r32();
        void _transfer_r1_u16_to_r32();
        void _transfer_r1_r32_to_r32();
        void _transfer_r2_u8_to_r32();
        void _transfer_r2_u16_to_r32();
        void _transfer_r2_r32_to_r32();
        void _transfer_r3_u8_to_r32();
        void _transfer_r3_u16_to_r32();
        void _transfer_r3_r32_to_r32();
        void _transfer_r4_u8_to_r32();
        void _transfer_r4_u16_to_r32();
        void _transfer_r4_r32_to_r32();

        void _transfer_w1_u8_to_u8();
        void _transfer_w1_u8_to_u16();
        void _transfer_w1_u8_to_r32();
        void _transfer_w2_u8_to_u8();
        void _transfer_w2_u8_to_u16();
        void _transfer_w2_u8_to_r32();
        void _transfer_w3_u8_to_u8();
        void _transfer_w3_u8_to_u16();
        void _transfer_w3_u8_to_r32();
        void _transfer_w4_u8_to_u8();
        void _transfer_w4_u8_to_u16();
        void _transfer_w4_u8_to_r32();

        void _transfer_w1_r32_to_u8();
        void _transfer_w1_r32_to_u16();
        void _transfer_w1_r32_to_r32();
        void _transfer_w2_r32_to_u8();
        void _transfer_w2_r32_to_u16();
        void _transfer_w2_r32_to_r32();
        void _transfer_w3_r32_to_u8();
        void _transfer_w3_r32_to_u16();
        void _transfer_w3_r32_to_r32();
        void _transfer_w4_r32_to_u8();
        void _transfer_w4_r32_to_u16();
        void _transfer_w4_r32_to_r32();
    };


    typedef void (*ImageProcessingFunc)(Image* image, void* ref);


} // End of namespace Grain

#endif // GrainImage_hpp
