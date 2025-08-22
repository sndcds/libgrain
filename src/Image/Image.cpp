//
//  Image.cpp
//
//  Created by Roald Christesen on 11.11.2015
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Image/Image.hpp"
#include "Image/ICCProfiles.hpp"
#include "Color/Gradient.hpp"
#include "Color/RGB.hpp"
#include "Color/HSV.hpp"
#include "Color/RGBLUT1.hpp"
#include "Color/CIEXYZ.hpp"
#include "Color/RGBRamp.hpp"
#include "Graphic/GraphicContext.hpp"
#include "Core/Log.hpp"

#include "File/TiffFile.hpp"
#include "2d/Data/CVF2.hpp"

#include <libraw/libraw.h>
#include <tiffio.h>
#include <cairo/cairo.h>
#include <webp/encode.h>
#include <png.h>
#include <jpeglib.h>


#if defined(__APPLE__) && defined(__MACH__)
    #include <CoreImage/CoreImage.h>
    #include <CoreGraphics/CoreGraphics.h>

    namespace Grain {
        Image* _mac_loadImageFromFile(const String& file_path, Image::PixelType pixel_type);
    } // namespace Grain
#endif


namespace Grain {

    ImageAccess::ImageAccess(Image* image, float* transfer_ptr) {
        undefine();

        if (image && image->isUsable()) {
            m_image = image;
            m_color_model = image->m_color_model;
            m_pixel_type = image->m_pixel_type;
            m_component_count = image->componentCount();

            m_width = image->m_width;
            m_height = image->m_height;

            _m_pixel_data_ptr = image->mutPixelDataPtr();
            _m_curr_ptr = image->mutPixelDataPtr();
            _m_pixel_data_step = image->pixelDataStep();
            _m_row_data_step = image->bytesPerRow();
            _m_plane_data_step = _m_row_data_step * m_height;

            setRegion(0, 0, m_width, m_height);

            if (transfer_ptr) {
                setTransferPtr_r32(transfer_ptr);
            }
            else {
                setTransferPtr_r32(_m_component_values_float);
            }

            if (!transfer_ptr) {
                clear();
            }

            m_usable = true;
        }
    }


    ErrorCode ImageAccess::setBySetupInfo(const ImageAccessSetupInfo& info) {
        undefine();

        m_width = info.m_width;
        m_height = info.m_height;
        m_color_model = info.m_color_model;
        m_component_count = info.m_component_count;
        m_pixel_type = info.m_pixel_type;

        _m_pixel_data_ptr = info.m_pixel_data_ptr;
        _m_pixel_data_step = info.m_pixel_data_step;
        _m_row_data_step = info.m_row_data_step;
        _m_plane_data_step = info.m_plane_data_step;

        setRegion(0, 0, m_width, m_height);

        setTransferPtr_r32(_m_component_values_float);

        if (m_width > 0 && m_height > 0 &&
            m_color_model != Color::Model::Undefined &&
            m_component_count > 0 &&
            m_pixel_type != Image::PixelType::Undefined &&
            _m_pixel_data_ptr &&
            _m_pixel_data_step > 0 &&
            _m_row_data_step > 0 &&
            _m_plane_data_step > 0) {
            m_usable = true;
        }
        else {
            m_usable = false;
        }

        return ErrorCode::None;
    }


    void ImageAccess::undefine() {
        m_usable = false;
        m_image = nullptr;
        _m_transfer_read_func = &ImageAccess::_transfer_dummy;
        _m_transfer_write_func = &ImageAccess::_transfer_dummy;
        m_x = m_y = m_width = m_height = 0;
    }


    bool ImageAccess::setPos(int32_t x, int32_t y) {
        m_x = std::clamp<int32_t>(x, m_region_x1, m_region_x2);
        m_y = std::clamp<int32_t>(y, m_region_y1, m_region_y2);
        _m_x_loop_start = true;
        _m_y_loop_start = true;
        _updatePtr();
        return true;
    }


    void ImageAccess::resetRegion() {
        setRegion(0, 0, m_width, m_height);
    }


    void ImageAccess::setRegion(int32_t x, int32_t y, int32_t width, int32_t height) {
        m_region_x1 = std::clamp<int32_t>(x, 0, m_width - 1);
        m_region_y1 = std::clamp<int32_t>(y, 0, m_height - 1);
        m_region_x2 = std::clamp<int32_t>(x + width - 1, m_region_x1, m_width - 1);
        m_region_y2 = std::clamp<int32_t>(y + height - 1, m_region_y1, m_height - 1);
        m_region_width = m_region_x2 - m_region_x1 + 1;
        m_region_height = m_region_y2 - m_region_y1 + 1;
        setPos(m_region_x1, m_region_y1);
    }


    bool ImageAccess::stepX() {
        if (m_x >= m_region_x2) {
            m_x = m_region_x1;
            _m_x_loop_start = true;
            _updatePtr();
            return false;
        }
        if (_m_x_loop_start) {
            _m_x_loop_start = false;
        }
        else {
            m_x++;
            _m_curr_ptr += _m_pixel_data_step;
        }
        return true;
    }


    bool ImageAccess::stepY() {
        if (m_y >= m_region_y2) {
            m_y = m_region_y1;
            _m_y_loop_start = true;
            _updatePtr();
            return false;
        }
        if (_m_y_loop_start) {
            _m_y_loop_start = false;
        }
        else {
            m_y++;
            _m_curr_ptr += _m_row_data_step;
        }
        return true;
    }


    void ImageAccess::setTransferPtr_u8(uint8_t* ptr) {
        _m_value_ptr_u8 = ptr;
        _m_transfer_read_func = &ImageAccess::_transfer_dummy;
        _m_transfer_write_func = &ImageAccess::_transfer_dummy;

        switch (m_color_model) {
            case Color::Model::Lumina:
                switch (m_pixel_type) {
                    case Image::PixelType::UInt8:
                        _m_transfer_read_func = &ImageAccess::_transfer_r1_u8_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w1_u8_to_u8;
                        break;
                    case Image::PixelType::UInt16:
                        _m_transfer_read_func = &ImageAccess::_transfer_r1_u16_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w1_u8_to_u16;
                        break;
                    case Image::PixelType::Float:
                        _m_transfer_read_func = &ImageAccess::_transfer_r1_r32_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w1_u8_to_r32;
                        break;
                    default:
                        break;
                }

                break;

            case Color::Model::LuminaAlpha:
                switch (m_pixel_type) {
                    case Image::PixelType::UInt8:
                        _m_transfer_read_func = &ImageAccess::_transfer_r2_u8_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w2_u8_to_u8;
                        break;
                    case Image::PixelType::UInt16:
                        _m_transfer_read_func = &ImageAccess::_transfer_r2_u16_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w2_u8_to_u16;
                        break;
                    case Image::PixelType::Float:
                        _m_transfer_read_func = &ImageAccess::_transfer_r2_r32_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w2_u8_to_r32;
                        break;
                    default:
                        break;
                }

                break;

            case Color::Model::RGB:
            case Color::Model::HSV:
            case Color::Model::XYZ:
            case Color::Model::YUV:
            case Color::Model::L_a_b:
                switch (m_pixel_type) {
                    case Image::PixelType::UInt8:
                        _m_transfer_read_func = &ImageAccess::_transfer_r3_u8_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w3_u8_to_u8;
                        break;
                    case Image::PixelType::UInt16:
                        _m_transfer_read_func = &ImageAccess::_transfer_r3_u16_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w3_u8_to_u16;
                        break;
                    case Image::PixelType::Float:
                        _m_transfer_read_func = &ImageAccess::_transfer_r3_r32_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w3_u8_to_r32;
                        break;
                    default: break;
                }

                break;

            case Color::Model::CMYK:
            case Color::Model::RGBA:
                switch (m_pixel_type) {
                    case Image::PixelType::UInt8:
                        _m_transfer_read_func = &ImageAccess::_transfer_r4_u8_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w4_u8_to_u8;
                        break;
                    case Image::PixelType::UInt16:
                        _m_transfer_read_func = &ImageAccess::_transfer_r4_u16_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w4_u8_to_u16;
                        break;
                    case Image::PixelType::Float:
                        _m_transfer_read_func = &ImageAccess::_transfer_r4_r32_to_u8;
                        _m_transfer_write_func = &ImageAccess::_transfer_w4_u8_to_r32;
                        break;
                    default: break;
                }

                break;

            case Color::Model::Bayer:
                // TODO: Implement!
                break;

            default:
                break;
        }
    }


    void ImageAccess::setTransferPtr_r32(float* ptr) {
        _m_value_ptr_float = ptr;
        _m_transfer_read_func = &ImageAccess::_transfer_dummy;
        _m_transfer_write_func = &ImageAccess::_transfer_dummy;

        switch (m_color_model) {
            case Color::Model::Lumina:
                switch (m_pixel_type) {
                    case Image::PixelType::UInt8:
                        _m_transfer_read_func = &ImageAccess::_transfer_r1_u8_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w1_r32_to_u8;
                        break;
                    case Image::PixelType::UInt16:
                        _m_transfer_read_func = &ImageAccess::_transfer_r1_u16_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w1_r32_to_u16;
                        break;
                    case Image::PixelType::Float:
                        _m_transfer_read_func = &ImageAccess::_transfer_r1_r32_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w1_r32_to_r32;
                        break;
                    default:
                        break;
                }

                break;

            case Color::Model::LuminaAlpha:
                switch (m_pixel_type) {
                    case Image::PixelType::UInt8:
                        _m_transfer_read_func = &ImageAccess::_transfer_r2_u8_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w2_r32_to_u8;
                        break;
                    case Image::PixelType::UInt16:
                        _m_transfer_read_func = &ImageAccess::_transfer_r2_u16_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w2_r32_to_u16;
                        break;
                    case Image::PixelType::Float:
                        _m_transfer_read_func = &ImageAccess::_transfer_r2_r32_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w2_r32_to_r32;
                        break;
                    default:
                        break;
                }

                break;

            case Color::Model::RGB:
            case Color::Model::HSV:
            case Color::Model::XYZ:
            case Color::Model::YUV:
            case Color::Model::L_a_b:
                switch (m_pixel_type) {
                    case Image::PixelType::UInt8:
                        _m_transfer_read_func = &ImageAccess::_transfer_r3_u8_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w3_r32_to_u8;
                        break;
                    case Image::PixelType::UInt16:
                        _m_transfer_read_func = &ImageAccess::_transfer_r3_u16_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w3_r32_to_u16;
                        break;
                    case Image::PixelType::Float:
                        _m_transfer_read_func = &ImageAccess::_transfer_r3_r32_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w3_r32_to_r32;
                        break;
                    default:
                        break;
                }

                break;

            case Color::Model::CMYK:
            case Color::Model::RGBA:
                switch (m_pixel_type) {
                    case Image::PixelType::UInt8:
                        _m_transfer_read_func = &ImageAccess::_transfer_r4_u8_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w4_r32_to_u8;
                        break;
                    case Image::PixelType::UInt16:
                        _m_transfer_read_func = &ImageAccess::_transfer_r4_u16_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w4_r32_to_u16;
                        break;
                    case Image::PixelType::Float:
                        _m_transfer_read_func = &ImageAccess::_transfer_r4_r32_to_r32;
                        _m_transfer_write_func = &ImageAccess::_transfer_w4_r32_to_r32;
                        break;
                    default:
                        break;
                }

                break;

            case Color::Model::Bayer:
                // TODO: Implement
                break;

            default:
                break;
        }
    }


    uint8_t* ImageAccess::ptrAt(int32_t x, int32_t y) {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return nullptr;
        }
        else {
            return &_m_pixel_data_ptr[(int64_t)x * (int64_t)_m_pixel_data_step + (int64_t)y * (int64_t)_m_row_data_step];
        }
    }


    void ImageAccess::readInterpolated(const Vec2d& pos) {
        auto saved_ptr = _m_curr_ptr;
        auto vp = _m_value_ptr_float;

        auto x0 = static_cast<int32_t>(pos.m_x);
        auto y0 = static_cast<int32_t>(pos.m_y);

        float xf1 = static_cast<float>(pos.m_x) - static_cast<float>(x0);
        float xf0 = 1.0f - xf1;
        float yf1 = static_cast<float>(pos.m_y) - static_cast<float>(y0);
        float yf0 = 1.0f - yf1;

        float cooef[2][2] = {
                { xf0 * yf0, xf1 * yf0 },
                { xf0 * yf1, xf1 * yf1 }
        };

        float sum[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        for (int32_t yi = 0; yi < 2; yi++) {
            for (int32_t xi = 0; xi < 2; xi++) {
                if ((_m_curr_ptr = ptrAt(x0 + xi, y0 + yi))) {
                    read();
                    auto c = cooef[yi][xi];
                    for (int32_t i = 0; i < m_component_count; i++) {
                        sum[i] += vp[i] * c;
                    }
                }
            }
        }

        for (int32_t i = 0; i < m_component_count; i++) {
            vp[i] = sum[i];
        }

        _m_curr_ptr = saved_ptr;
    }


    void ImageAccess::clear() {
        switch (m_color_model) {
            case Color::Model::Lumina:
                _m_value_ptr_float[0] = 0.0f;
                break;
            case Color::Model::LuminaAlpha:
                _m_value_ptr_float[0] = 0.0f;
                _m_value_ptr_float[1] = 1.0f;
                break;
            case Color::Model::RGB:
                _m_value_ptr_float[0] = _m_value_ptr_float[1] = _m_value_ptr_float[2] = 0.0f;
                break;
            case Color::Model::RGBA:
                _m_value_ptr_float[0] = _m_value_ptr_float[1] = _m_value_ptr_float[2] = 0.0f;
                _m_value_ptr_float[3] = 1.0f;
                break;
            case Color::Model::CMYK:
                _m_value_ptr_float[0] = _m_value_ptr_float[1] = _m_value_ptr_float[2] = _m_value_ptr_float[3] = 0.0f;
                break;
            case Color::Model::YUV:
                _m_value_ptr_float[0] = _m_value_ptr_float[1] = _m_value_ptr_float[2] = 0.0f;
                _m_value_ptr_float[0] = _m_value_ptr_float[1] = _m_value_ptr_float[2] = 0.0f;
                break;
            default:
                _m_value_ptr_float[0] = _m_value_ptr_float[1] = _m_value_ptr_float[2] = _m_value_ptr_float[3] = 0.0f;
                break;
        }
    }


    void ImageAccess::setRGB(const Vec2i& pos, const RGB& color, float alpha) noexcept {
        int32_t x = pos.m_x;
        int32_t y = pos.m_y;

        if (x >= 0 && x < m_width && y >= 0 && y< m_height) {
            auto saved_ptr = _m_curr_ptr;
            float alpha_inv = 1.0f - alpha;
            _m_curr_ptr = ptrAt(x, y);
            if (_m_curr_ptr) {
                read();
                _m_value_ptr_float[0] = _m_value_ptr_float[0] * alpha_inv + color.m_data[0] * alpha;
                _m_value_ptr_float[1] = _m_value_ptr_float[1] * alpha_inv + color.m_data[1] * alpha;
                _m_value_ptr_float[2] = _m_value_ptr_float[2] * alpha_inv + color.m_data[2] * alpha;
                write();
            }

            _m_curr_ptr = saved_ptr;
        }
    }


    void ImageAccess::setRGBInterpolated(const Vec2d& pos, const RGB& color, float alpha) noexcept {
        bool x_neg = pos.m_x < 0;
        bool y_neg = pos.m_y < 0;

        auto x0 = static_cast<int32_t>(pos.m_x);
        if (x_neg) {
            x0 -= 1;
        }
        int32_t x1 = x0 + 1;
        auto y0 = static_cast<int32_t>(pos.m_y);
        if (y_neg) {
            y0 -= 1;
        }
        int32_t y1 = y0 + 1;

        if (x0 >= m_width || x1 < 0 || y0 >= m_height || y1 < 0) {
            return;
        }

        float xf1 = static_cast<float>(pos.m_x) - static_cast<float>(x0);
        float xf0 = 1.0f - xf1;
        float yf1 = static_cast<float>(pos.m_y) - static_cast<float>(y0);
        float yf0 = 1.0f - yf1;

        RGB* d = (RGB*)_m_value_ptr_float;

        auto saved_ptr = _m_curr_ptr;

        _m_curr_ptr = ptrAt(x0, y0);
        if (_m_curr_ptr) {
            read();
            d->setBlend(color, xf0 * yf0 * alpha);
            write();
        }

        _m_curr_ptr = ptrAt(x1, y0);
        if (_m_curr_ptr) {
            read();
            d->setBlend(color, xf1 * yf0 * alpha);
            write();
        }

        _m_curr_ptr = ptrAt(x0, y1);
        if (_m_curr_ptr) {
            read();
            d->setBlend(color, xf0 * yf1 * alpha);
            write();
        }

        _m_curr_ptr = ptrAt(x1, y1);
        if (_m_curr_ptr) {
            read();
            d->setBlend(color, xf1 * yf1 * alpha);
            write();
        }

        _m_curr_ptr = saved_ptr;
    }


    void ImageAccess::_updatePtr() {
        _m_curr_ptr = &_m_pixel_data_ptr[(int64_t)m_x * (int64_t)_m_pixel_data_step + (int64_t)m_y * (int64_t)_m_row_data_step];
    }


    void ImageAccess::_transfer_dummy() {
    }


    void ImageAccess::_transfer_r1_u8_to_u8() {
        *_m_value_ptr_u8 = *((uint8_t*)_m_curr_ptr);
    }


    void ImageAccess::_transfer_r1_u16_to_u8() {
        *_m_value_ptr_u8 = (uint8_t)(*((uint16_t*)_m_curr_ptr) >> 8);
    }


    void ImageAccess::_transfer_r1_r32_to_u8() {
        *_m_value_ptr_u8 = (uint8_t)(*((float*)_m_curr_ptr) * std::numeric_limits<uint8_t>::max());
    }


    void ImageAccess::_transfer_r2_u8_to_u8() {
        auto s = static_cast<uint8_t*>(_m_curr_ptr);
        uint8_t* d = _m_value_ptr_u8;
        *d++ = *s;
        *d = s[1];
    }


    void ImageAccess::_transfer_r2_u16_to_u8() {
        auto s = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        uint8_t* d = _m_value_ptr_u8;
        *d++ = (uint8_t)(*s >> 8);
        *d = (uint8_t)(s[1] >> 8);
    }


    void ImageAccess::_transfer_r2_r32_to_u8() {
        auto s = reinterpret_cast<float*>(_m_curr_ptr);
        uint8_t* d = _m_value_ptr_u8;
        *d++ = (uint8_t)(*s * std::numeric_limits<uint8_t>::max());
        *d = (uint8_t)(s[1] * std::numeric_limits<uint8_t>::max());
    }


    void ImageAccess::_transfer_r3_u8_to_u8() {
        auto s = static_cast<uint8_t*>(_m_curr_ptr);
        uint8_t* d = _m_value_ptr_u8;
        *d++ = *s;
        *d++ = s[1];
        *d = s[2];
    }


    void ImageAccess::_transfer_r3_u16_to_u8() {
        auto s = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        uint8_t* d = _m_value_ptr_u8;
        *d++ = (uint8_t)(*s >> 8);
        *d++ = (uint8_t)(s[1] >> 8);
        *d = (uint8_t)(s[2] >> 8);
    }


    void ImageAccess::_transfer_r3_r32_to_u8() {
        auto s = reinterpret_cast<float*>(_m_curr_ptr);
        uint8_t* d = _m_value_ptr_u8;
        *d++ = (uint8_t)(*s * std::numeric_limits<uint8_t>::max());
        *d++ = (uint8_t)(s[1] * std::numeric_limits<uint8_t>::max());
        *d = (uint8_t)(s[2] * std::numeric_limits<uint8_t>::max());
    }


    void ImageAccess::_transfer_r4_u8_to_u8() {
        auto s = reinterpret_cast<uint8_t*>(_m_curr_ptr);
        uint8_t* d = _m_value_ptr_u8;
        *d++ = *s;
        *d++ = s[1];
        *d = s[2];
    }


    void ImageAccess::_transfer_r4_u16_to_u8() {
        auto s = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        uint8_t* d = _m_value_ptr_u8;
        *d++ = (uint8_t)(*s >> 8);
        *d++ = (uint8_t)(s[1] >> 8);
        *d = (uint8_t)(s[2] >> 8);
    }


    void ImageAccess::_transfer_r4_r32_to_u8() {
        auto s = reinterpret_cast<float*>(_m_curr_ptr);
        uint8_t* d = _m_value_ptr_u8;
        *d++ = (uint8_t)(*s * std::numeric_limits<uint8_t>::max());
        *d++ = (uint8_t)(s[1] * std::numeric_limits<uint8_t>::max());
        *d = (uint8_t)(s[2] * std::numeric_limits<uint8_t>::max());
    }


    void ImageAccess::_transfer_r1_u8_to_r32() {
        *_m_value_ptr_float = ((float)*((uint8_t*)_m_curr_ptr)) / std::numeric_limits<uint8_t>::max();
    }


    void ImageAccess::_transfer_r1_u16_to_r32() {
        *_m_value_ptr_float = ((float)*((uint16_t*)_m_curr_ptr)) / std::numeric_limits<uint16_t>::max();
    }


    void ImageAccess::_transfer_r1_r32_to_r32() {
        *_m_value_ptr_float = *((float*)_m_curr_ptr);
    }


    void ImageAccess::_transfer_r2_u8_to_r32() {
        auto s = reinterpret_cast<uint8_t*>(_m_curr_ptr);
        float* d = _m_value_ptr_float;
        *d++ = (float)*s / std::numeric_limits<uint8_t>::max();
        *d = (float)s[1] / std::numeric_limits<uint8_t>::max();
    }


    void ImageAccess::_transfer_r2_u16_to_r32() {
        auto s = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        float* d = _m_value_ptr_float;
        *d++ = (float)*s / std::numeric_limits<uint16_t>::max();
        *d = (float)s[1] / std::numeric_limits<uint16_t>::max();
    }


    void ImageAccess::_transfer_r2_r32_to_r32() {
        auto s = reinterpret_cast<float*>(_m_curr_ptr);
        float* d = _m_value_ptr_float;
        *d++ = *s;
        *d = s[1];
    }


    void ImageAccess::_transfer_r3_u8_to_r32() {
        auto s = reinterpret_cast<uint8_t*>(_m_curr_ptr);
        float* d = _m_value_ptr_float;
        *d++ = (float)*s / std::numeric_limits<uint8_t>::max();
        *d++ = (float)s[1] / std::numeric_limits<uint8_t>::max();
        *d = (float)s[2] / std::numeric_limits<uint8_t>::max();
    }


    void ImageAccess::_transfer_r3_u16_to_r32() {
        auto s = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        float* d = _m_value_ptr_float;
        *d++ = (float)*s / std::numeric_limits<uint16_t>::max();
        *d++ = (float)s[1] / std::numeric_limits<uint16_t>::max();
        *d = (float)s[2] / std::numeric_limits<uint16_t>::max();
    }


    void ImageAccess::_transfer_r3_r32_to_r32() {
        auto s = reinterpret_cast<float*>(_m_curr_ptr);
        float* d = _m_value_ptr_float;
        *d++ = *s;
        *d++ = s[1];
        *d = s[2];
    }


    void ImageAccess::_transfer_r4_u8_to_r32() {
        auto s = reinterpret_cast<uint8_t*>(_m_curr_ptr);
        float* d = _m_value_ptr_float;
        *d++ = (float)*s / std::numeric_limits<uint8_t>::max();
        *d++ = (float)s[1] / std::numeric_limits<uint8_t>::max();
        *d++ = (float)s[2] / std::numeric_limits<uint8_t>::max();
        *d = (float)s[3] / std::numeric_limits<uint8_t>::max();
    }


    void ImageAccess::_transfer_r4_u16_to_r32() {
        auto s = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        float* d = _m_value_ptr_float;
        *d++ = (float)*s / std::numeric_limits<uint16_t>::max();
        *d++ = (float)s[1] / std::numeric_limits<uint16_t>::max();
        *d++ = (float)s[2] / std::numeric_limits<uint16_t>::max();
        *d = (float)s[3] / std::numeric_limits<uint16_t>::max();
    }


    void ImageAccess::_transfer_r4_r32_to_r32() {
        auto s = reinterpret_cast<float*>(_m_curr_ptr);
        float* d = _m_value_ptr_float;
        *d++ = *s;
        *d++ = s[1];
        *d++ = s[2];
        *d = s[3];
    }


    void ImageAccess::_transfer_w1_u8_to_u8() {
        *((uint8_t*)_m_curr_ptr) = *_m_value_ptr_u8;
    }


    void ImageAccess::_transfer_w1_u8_to_u16() {
        *((uint16_t*)_m_curr_ptr) = ((uint16_t)*_m_value_ptr_u8) << 8;
    }


    void ImageAccess::_transfer_w1_u8_to_r32() {
        *((float*)_m_curr_ptr) = ((float)*_m_value_ptr_u8) / std::numeric_limits<uint8_t>::max();
    }


    void ImageAccess::_transfer_w2_u8_to_u8() {
        uint8_t* s = _m_value_ptr_u8;
        auto d = reinterpret_cast<uint8_t*>(_m_curr_ptr);
        *d = *s++;
        d[1] = *s;
    }


    void ImageAccess::_transfer_w2_u8_to_u16() {
        uint8_t* s = _m_value_ptr_u8;
        auto d = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        *d = (uint16_t)(*s++) << 8;
        d[1] = (uint16_t)(*s) << 8;
    }


    void ImageAccess::_transfer_w2_u8_to_r32() {
        uint8_t* s = _m_value_ptr_u8;
        auto d = reinterpret_cast<float*>(_m_curr_ptr);
        *d = (float)(*s++) / std::numeric_limits<uint8_t>::max();
        d[1] = (float)(*s) / std::numeric_limits<uint8_t>::max();
    }


    void ImageAccess::_transfer_w3_u8_to_u8() {
        uint8_t* s = _m_value_ptr_u8;
        auto d = reinterpret_cast<uint8_t*>(_m_curr_ptr);
        *d = *s++;
        d[1] = *s++;
        d[2] = *s;
    }


    void ImageAccess::_transfer_w3_u8_to_u16() {
        uint8_t* s = _m_value_ptr_u8;
        auto d = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        *d = (uint16_t)(*s++) << 8;
        d[1] = (uint16_t)(*s++) << 8;
        d[2] = (uint16_t)(*s) << 8;
    }


    void ImageAccess::_transfer_w3_u8_to_r32() {
        uint8_t* s = _m_value_ptr_u8;
        auto d = reinterpret_cast<float*>(_m_curr_ptr);
        *d = (float)(*s++) / std::numeric_limits<uint8_t>::max();
        d[1] = (float)(*s++) / std::numeric_limits<uint8_t>::max();
        d[2] = (float)(*s) / std::numeric_limits<uint8_t>::max();
    }


    void ImageAccess::_transfer_w4_u8_to_u8() {
        uint8_t* s = _m_value_ptr_u8;
        auto d = reinterpret_cast<uint8_t*>(_m_curr_ptr);
        *d = *s++;
        d[1] = *s++;
        d[2] = *s++;
        d[3] = *s;
    }


    void ImageAccess::_transfer_w4_u8_to_u16() {
        uint8_t* s = _m_value_ptr_u8;
        auto d = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        *d = (uint16_t)(*s++) << 8;
        d[1] = (uint16_t)(*s++) << 8;
        d[2] = (uint16_t)(*s++) << 8;
        d[3] = (uint16_t)(*s) << 8;
    }


    void ImageAccess::_transfer_w4_u8_to_r32() {
        uint8_t* s = _m_value_ptr_u8;
        auto d = reinterpret_cast<float*>(_m_curr_ptr);
        *d = (float)(*s++) / std::numeric_limits<uint8_t>::max();
        d[1] = (float)(*s++) / std::numeric_limits<uint8_t>::max();
        d[2] = (float)(*s++) / std::numeric_limits<uint8_t>::max();
        d[3] = (float)(*s) / std::numeric_limits<uint8_t>::max();
    }


    void ImageAccess::_transfer_w1_r32_to_u8() {
        *((uint8_t*)_m_curr_ptr) = (uint8_t)(*_m_value_ptr_float * std::numeric_limits<uint8_t>::max());
    }


    void ImageAccess::_transfer_w1_r32_to_u16() {
        *((uint16_t*)_m_curr_ptr) = (uint16_t)(*_m_value_ptr_float * std::numeric_limits<uint16_t>::max());
    }


    void ImageAccess::_transfer_w1_r32_to_r32() {
        *((float*)_m_curr_ptr) = *_m_value_ptr_float;
    }


    void ImageAccess::_transfer_w2_r32_to_u8() {
        float* s = _m_value_ptr_float;
        auto d = reinterpret_cast<uint8_t*>(_m_curr_ptr);
        *d = (uint8_t)(*s++ * std::numeric_limits<uint8_t>::max());
        d[1] = (uint8_t)(*s * std::numeric_limits<uint8_t>::max());
    }


    void ImageAccess::_transfer_w2_r32_to_u16() {
        float* s = _m_value_ptr_float;
        auto d = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        *d = (uint16_t)(*s++ * std::numeric_limits<uint16_t>::max());
        d[1] = (uint16_t)(*s * std::numeric_limits<uint16_t>::max());
    }


    void ImageAccess::_transfer_w2_r32_to_r32() {
        float* s = _m_value_ptr_float;
        auto d = reinterpret_cast<float*>(_m_curr_ptr);
        *d = *s++;
        d[1] = *s;
    }


    void ImageAccess::_transfer_w3_r32_to_u8() {
        float* s = _m_value_ptr_float;
        auto d = reinterpret_cast<uint8_t*>(_m_curr_ptr);
        *d = (uint8_t)(*s++ * std::numeric_limits<uint8_t>::max());
        d[1] = (uint8_t)(*s++ * std::numeric_limits<uint8_t>::max());
        d[2] = (uint8_t)(*s * std::numeric_limits<uint8_t>::max());
    }


    void ImageAccess::_transfer_w3_r32_to_u16() {
        float* s = _m_value_ptr_float;
        auto d = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        *d = (uint16_t)(*s++ * std::numeric_limits<uint16_t>::max());
        d[1] = (uint16_t)(*s++ * std::numeric_limits<uint16_t>::max());
        d[2] = (uint16_t)(*s * std::numeric_limits<uint16_t>::max());
    }


    void ImageAccess::_transfer_w3_r32_to_r32() {
        float* s = _m_value_ptr_float;
        auto d = reinterpret_cast<float*>(_m_curr_ptr);
        *d = *s++;
        d[1] = *s++;
        d[2] = *s;
    }


    void ImageAccess::_transfer_w4_r32_to_u8() {
        float* s = _m_value_ptr_float;
        auto d = reinterpret_cast<uint8_t*>(_m_curr_ptr);
        *d = (uint8_t)(*s++ * std::numeric_limits<uint8_t>::max());
        d[1] = (uint8_t)(*s++ * std::numeric_limits<uint8_t>::max());
        d[2] = (uint8_t)(*s++ * std::numeric_limits<uint8_t>::max());
        d[3] = (uint8_t)(*s * std::numeric_limits<uint8_t>::max());
    }


    void ImageAccess::_transfer_w4_r32_to_u16() {
        float* s = _m_value_ptr_float;
        auto d = reinterpret_cast<uint16_t*>(_m_curr_ptr);
        *d = (uint16_t)(*s++ * std::numeric_limits<uint16_t>::max());
        d[1] = (uint16_t)(*s++ * std::numeric_limits<uint16_t>::max());
        d[2] = (uint16_t)(*s++ * std::numeric_limits<uint16_t>::max());
        d[3] = (uint16_t)(*s * std::numeric_limits<uint16_t>::max());
    }


    void ImageAccess::_transfer_w4_r32_to_r32() {
        float* s = _m_value_ptr_float;
        auto d = reinterpret_cast<float*>(_m_curr_ptr);
        *d = *s++;
        d[1] = *s++;
        d[2] = *s++;
        d[3] = *s;
    }


    /**
     *  @brief Create a new image object with the same settings as a given source image.
     *
     *  This constructor initializes a new Image object based on the settings and configuration
     *  of an existing source image. It provides a convenient way to duplicate the settings of
     *  an image without duplicating the image data itself.
     *
     *  @param [in,out] image A pointer to the source image from which settings are copied.
     */
    Image::Image(const Image *image) noexcept : Object() {
        if (image) {
            _set(image->m_color_model, image->m_width, image->m_height, image->m_pixel_type);
            _malloc();
        }
    }


    /**
     *  @brief Create a new image object with custom dimensions, based on a source image.
     *
     *  This constructor initializes a new Image object with the specified width and height, using
     *  settings and configuration from the provided source image. It allows you to create a new
     *  image with custom dimensions while inheriting settings from the source image.
     *
     *  @param [in,out] image A pointer to the source image from which settings are copied.
     *  @param width The width of the new image.
     *  @param height The height of the new image.
     */
    Image::Image(Image *image, int32_t width, int32_t height) noexcept : Object() {
        if (image) {
            _set(image->m_color_model, width, height, image->m_pixel_type);
            _malloc();
        }
    }


    Image::Image(Color::Model color_model, int32_t width, int32_t height, PixelType pixel_type) noexcept : Object() {
        _set(color_model, width, height, pixel_type);
        _malloc();
    }

/* TODO !!!!!
#if defined(__APPLE__) && defined(__MACH__)
    Image::Image(NSImage *ns_image, Image::PixelType data_type) noexcept : Object() {

        _m_pixel_data = nullptr;
        _m_mem_size = 0;


        NSData *ns_image_data = [[NSData alloc] initWithData:[ns_image TIFFRepresentation]];
        NSBitmapImageRep *bitmap_rep = [[NSBitmapImageRep alloc] initWithData:ns_image_data];

        NSColorSpace* s_color_space = [bitmap_rep colorSpace];
        if (s_color_space == nil) {
            return;
        }

        NSInteger s_width = [bitmap_rep pixelsWide];
        NSInteger s_height = [bitmap_rep pixelsHigh];
        // NSInteger s_bits_per_pixel = [bitmap_rep bitsPerPixel];  // Unused
        NSInteger s_bits_per_sample = [bitmap_rep bitsPerSample];
        NSInteger s_number_of_planes = [bitmap_rep numberOfPlanes];
        // NSInteger s_samples_per_pixel = [bitmap_rep samplesPerPixel];  // Unused
        NSBitmapFormat s_bitmap_format = [bitmap_rep bitmapFormat];
        BOOL s_has_alpha = [bitmap_rep hasAlpha];
        // NSInteger s_bytes_per_row = [bitmap_rep bytesPerRow];  // Unused

        NSColorSpaceModel s_color_space_model = [s_color_space colorSpaceModel];

        Color::Model s_color_model;
        Image::PixelType s_data_type = Image::PixelType::Undefined;


        switch (s_color_space_model) {
            case NSColorSpaceModelGray:
                s_color_model = s_has_alpha ? Color::Model::LuminaAlpha : Color::Model::Lumina;
                break;

            case NSColorSpaceModelRGB:
                s_color_model = s_has_alpha ? Color::Model::RGBA : Color::Model::RGB;
                break;

            case NSColorSpaceModelCMYK:
                s_color_model = Color::Model::CMYK;
                if (s_has_alpha)
                    return;
                break;

            case NSColorSpaceModelLAB:
                s_color_model = Color::Model::L_a_b;
                if (s_has_alpha)
                    return;
                break;

            default:
                return;
        }


        switch (s_bits_per_sample) {
            case 8:
                s_data_type = Image::PixelType::UInt8;
                break;

            case 16:
                s_data_type = Image::PixelType::UInt16;
                break;

            case 32:
                if (s_bitmap_format & NSBitmapFormatFloatingPointSamples)
                    s_data_type = Image::PixelType::Float;
                else
                    return;
                break;

            default:
                return;
        }


        _set(s_color_model, (uint32_t)s_width, (uint32_t)s_height, data_type);
        _malloc();


        copyFromNSBitmapImageRep(bitmap_rep);

        [bitmap_rep release];
        [ns_image_data release];
    }
#endif
*/

    Image::~Image() noexcept {
        if (_m_pixel_data) {
            _free();
        }

#if defined(__APPLE__) && defined(__MACH__)
        if (_m_cg_context_ref) {
            CGContextRelease(_m_cg_context_ref);
        }
        macos_releaseCGImageRef();
#endif
    }


    bool Image::camToXYZMatrix(Mat3f& out_matrix) const noexcept {
        if (!m_has_cam_to_xyz_matrix) {
            out_matrix.identity();
            return false;
        }
        else {
            out_matrix = m_cam_to_xyz_matrix;
            return true;
        }
    }


    bool Image::camToSRGBMatrix(Mat3f& out_matrix) const noexcept {
        if (!m_has_rgb_to_cam_matrix) {
            out_matrix.identity();
            return false;
        }
        else {
            out_matrix = m_rgb_to_cam_matrix;
            return true;
        }
    }


    Ranged Image::updateSampleValueRange() noexcept {
        m_value_range.m_min = std::numeric_limits<float>::max();
        m_value_range.m_max = std::numeric_limits<float>::lowest();

        if (isUsable()) {
            float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            ImageAccess ia(this, pixel);

            if (m_color_model == Color::Model::Lumina || m_color_model == Color::Model::LuminaAlpha) {
                while (ia.stepY()) {
                    while (ia.stepX()) {
                        ia.read();
                        if (pixel[0] < m_value_range.m_min) { m_value_range.m_min = pixel[0]; }
                        if (pixel[0] > m_value_range.m_max) { m_value_range.m_max = pixel[0]; }
                    }
                }
            }
            else if (m_color_model == Color::Model::RGB || m_color_model == Color::Model::RGBA) {
                while (ia.stepY()) {
                    while (ia.stepX()) {
                        ia.read();
                        if (pixel[0] < m_value_range.m_min) { m_value_range.m_min = pixel[0]; }
                        if (pixel[1] < m_value_range.m_min) { m_value_range.m_min = pixel[1]; }
                        if (pixel[2] < m_value_range.m_min) { m_value_range.m_min = pixel[2]; }
                        if (pixel[0] > m_value_range.m_max) { m_value_range.m_max = pixel[0]; }
                        if (pixel[1] > m_value_range.m_max) { m_value_range.m_max = pixel[1]; }
                        if (pixel[2] > m_value_range.m_max) { m_value_range.m_max = pixel[2]; }
                    }
                }
            }
        }

        return m_value_range;
    }


#if defined(__APPLE__) && defined(__MACH__)
    CGBitmapInfo Image::macos_cgBitmapInfo() const noexcept {

        CGBitmapInfo bitmap_info = 0x0;

        if (isFloat()) {
            bitmap_info |= kCGBitmapFloatComponents;
        }

        if (hasAlpha()) {
            bitmap_info |= kCGImageAlphaPremultipliedLast;
        }
        else {
            bitmap_info |= kCGImageAlphaNone;
        }


        switch (bitsPerComponent()) {
            case 16:
                bitmap_info |= BYTE_ORDER == BIG_ENDIAN ? kCGBitmapByteOrder16Big : kCGBitmapByteOrder16Little;
                break;

            case 32:
                bitmap_info |= BYTE_ORDER == BIG_ENDIAN ? kCGBitmapByteOrder32Big : kCGBitmapByteOrder32Little;
                break;
        }

        return bitmap_info;
    }
#endif


    bool Image::sameSize(const Image *image) const noexcept {

        return image && m_width == image->m_width && m_height == image->m_height;
    }


    bool Image::sameFormat(const Image *image) const noexcept {

        return image && m_pixel_type == image->m_pixel_type && m_color_model == image->m_color_model;
    }


    bool Image::beginDraw() noexcept {

        if ((m_color_model != Color::Model::RGB && m_color_model != Color::Model::RGBA) ||bitsPerComponent() > 32) {
            return false;
        }
        else {
            return true;
        }
    }


    void Image::endDraw() noexcept {
    }


#if defined(__APPLE__) && defined(__MACH__)
    bool Image::graphicContext(GraphicContext *out_gc) noexcept {

        try {
            if (!out_gc) {
                throw ErrorCode::BadArgs;
            }

            if (bitsPerComponent() > 32) {
                throw Error::specific(kErrUnsupportedBitDepth);
            }

            if (m_color_model != Color::Model::RGB &&
                m_color_model != Color::Model::RGBA &&
                m_color_model != Color::Model::Lumina &&
                m_color_model != Color::Model::LuminaAlpha) {
                throw Error::specific(kErrUnsupportedColorModel);
            }

            if (!_m_cg_context_ref) {

                CGColorSpaceRef cg_color_space = nil;

                if (m_color_model == Color::Model::RGB || m_color_model == Color::Model::RGBA) {
                    cg_color_space= CGColorSpaceCreateDeviceRGB();
                }
                else if (m_color_model == Color::Model::Lumina || m_color_model == Color::Model::LuminaAlpha) {
                    cg_color_space= CGColorSpaceCreateDeviceGray();
                }

                if (cg_color_space == nil) {
                    throw Error::specific(kErrNoColorSpace);
                }

                CGBitmapInfo cg_bitmap_info = macos_cgBitmapInfo();
                _m_cg_context_ref = CGBitmapContextCreate((void*)_m_pixel_data, m_width, m_height, bitsPerComponent(), bytesPerRow(), cg_color_space, cg_bitmap_info);

                if (_m_cg_context_ref) {
                    CGColorSpaceRelease(cg_color_space);
                    CGAffineTransform flipVertical = CGAffineTransformMake(1, 0, 0, -1, 0, m_height);
                    CGContextConcatCTM(_m_cg_context_ref, flipVertical);
                }

                CGColorSpaceRelease(cg_color_space);
            }

            if (!_m_cg_context_ref) {
                throw Error::specific(kErrCGContextMissing);
            }

            if (out_gc) {
                out_gc->m_cg_context = _m_cg_context_ref;
            }

            /* TODO !!!!!
            // Create an NSGraphicsContext from CGContextRef and set it as the active context
            NSGraphicsContext *ns_graphics_context = [NSGraphicsContext graphicsContextWithCGContext:_m_cg_context_ref flipped:true];
            [NSGraphicsContext setCurrentContext:ns_graphics_context];
            */
        }
        catch (ErrorCode err) {
        }

        return _m_cg_context_ref;
    }
#else
    bool Image::graphicContext(GraphicContext *out_gc) noexcept {
        // TODO: Implement linux version
        return false;
    }
#endif


    void Image::clearBlack() noexcept {

        clear(RGBA(0, 0, 0, 1));
    }


    void Image::clearWhite() noexcept {

        clear(RGBA(1, 1, 1, 1));
    }


    void Image::clear(const RGB& color) noexcept {

        switch (m_color_model) {
            case Color::Model::RGB:
            case Color::Model::RGBA:
                clear(color.red(), color.green(), color.blue(), 1);
                break;

            case Color::Model::CMYK:
                clear(0, 0, 0, 0);
                break;

            default:
                break;
        }
    }


    void Image::clear(const RGBA& color) noexcept {

        switch (m_color_model) {
            case Color::Model::Lumina:
                clear(color.lumina(), 0, 0, 0);
                break;

            case Color::Model::LuminaAlpha:
                clear(color.lumina(), 1, 0, 0);
                break;

            case Color::Model::RGB:
                clear(color.red(), color.green(), color.blue(), 1);
                break;

            case Color::Model::RGBA:
                clear(color.red(), color.green(), color.blue(), color.alpha());
                break;

            case Color::Model::CMYK:
                // TODO: Convert RGB to CMYK!
                clear(0, 0, 0, 1);
                break;

            default:
                break;
        }
    }


    void Image::clear(float c0, float c1, float c2, float c3) noexcept {

        if (isUsable()) {
            float pixel[4] = { c0, c1, c2, c3 };
            ImageAccess ia(this, pixel);

            while (ia.stepY()) {
                while (ia.stepX()) {
                    ia.write();
                }
            }
        }
    }


    void Image::clearAlpha(float alpha) noexcept {

        if (isUsable()) {
            int32_t component_offset = 0;
            int32_t component_step = 0;

            switch (m_color_model) {
                case Color::Model::LuminaAlpha: component_step = 2; break;
                case Color::Model::RGBA: component_step = 4; break;
                default: return;
            }

            if (m_pixel_type == PixelType::UInt8) {
                auto p = (uint8_t*)_m_pixel_data;
                p += component_offset;
                auto v = static_cast<uint8_t>(alpha * std::numeric_limits<uint8_t>::max());
                for (int32_t i = 0; i < _m_pixel_count; i++) {
                    *p = v;
                    p += component_step;
                }
            }
            else if (m_pixel_type == PixelType::UInt16) {
                auto p = (uint16_t*)_m_pixel_data;
                p += component_offset;
                auto v = static_cast<uint16_t>(alpha * std::numeric_limits<uint16_t>::max());
                for (int32_t i = 0; i < _m_pixel_count; i++) {
                    *p = v;
                    p += component_step;
                }
            }
            else if (m_pixel_type == PixelType::Float) {
                auto p = (float*)_m_pixel_data;
                p += component_offset;
                for (int32_t i = 0; i < _m_pixel_count; i++) {
                    *p = alpha;
                    p += component_step;
                }
            }
        }
    }


    /**
     *  @brief Copy image data, if type of source and destination is similar.
     */
    bool Image::copyDataFromImage(Image* image) {

        if (image) {
            if (isUsable() && image->isUsable() && sameSize(image) && sameFormat(image)) {
                if (memcpy(mutPixelDataPtr(), image->pixelDataPtr(), memSize())) {
                    return true;
                }
            }
        }

        return false;
    }


    void Image::fillHueWheelRect(float saturation, float value) noexcept {

        HSV hsv_color(0, saturation, value);

        float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ImageAccess ia(this, pixel);

        while (ia.stepY()) {
            while (ia.stepX()) {
                float x = 0.5f - (float)(m_width - ia.x() - 1) / (m_width - 1);
                float y = 0.5f - (float)(m_height - ia.y() - 1) / (m_height - 1);

                float angle = (float)(std::atan2(x, y) * 180 / std::numbers::pi);
                hsv_color.setHue(angle / 360);
                Color::hsv_to_rgb(hsv_color.dataPtr(), pixel);

                ia.write();
            }
        }
    }


    void Image::fillAudioLocationRect(const RGBRamp& color_ramp) noexcept {

        float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ImageAccess ia(this, pixel);

        Vec3d p; // Position as x, y, z
        while (ia.stepY()) {
            p.m_y = 2.0 * (double)ia.y() / (m_height - 1.0) - 1.0;

            while (ia.stepX()) {
                p.m_x = 2.0 * (double)ia.x() / (m_width - 1.0) - 1.0;
                Vec3d l = p.posToLoc();  // Location as angle, distance, elevation
                color_ramp.lookupRing(l.m_x / 360.0f, pixel); // m_x = angle

                // Desaturate color ralated to distance
                float f = std::clamp<float>(l.m_y, 0.0f, 1.0f); // m_y = distance
                float f_inv = 1.0f - f;
                pixel[0] = f * pixel[0] + f_inv;
                pixel[1] = f * pixel[1] + f_inv;
                pixel[2] = f * pixel[2] + f_inv;

                ia.write();
            }
        }
    }


    ErrorCode Image::drawImage(Image *image) noexcept {

        return drawImage(image, Rectd(0, 0, m_width, m_height));
    }


    ErrorCode Image::drawImage(Image *image, const Rectd& rect) noexcept {

        auto result = ErrorCode::None;

        try {
            if (!image) {
                throw ErrorCode::NullData;
            }

            if (m_color_model != Color::Model::RGB &&
                m_color_model != Color::Model::RGBA &&
                m_color_model != Color::Model::Lumina &&
                m_color_model != Color::Model::LuminaAlpha) {
                throw ErrorCode::UnsupportedColorModel;
            }

            GraphicContext gc;
            gc.setImage(image);
            beginDraw();

            gc.setStrokeRGB({ 1, 0, 0 });
            gc.setStrokeWidth(2);
            gc.strokeRect(rect);

            gc.drawImage(image, rect);

            endDraw();
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


    void Image::flipHorizontal() noexcept {

        if (isUsable()) {

            ImageAccess ia(this);
            //TODO: Implement!
        }
    }


    void Image::flipVertical() noexcept {

        // TODO: Implement!
    }


    void Image::normalize() noexcept {

        updateSampleValueRange();

        float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ImageAccess ia(this, pixel);

        if (ia.isUsable()) {
            if (m_color_model == Color::Model::Lumina || m_color_model == Color::Model::LuminaAlpha) {
                float range = m_value_range.m_max - m_value_range.m_min;
                if (range > std::numeric_limits<float>::epsilon()) {
                    float s = 1.0 / range;
                    while (ia.stepY()) {
                        while (ia.stepX()) {
                            ia.read();
                            pixel[0] = (pixel[0] - m_value_range.m_min) * s;
                            ia.write();
                        }
                    }
                }
            }
            else if (m_color_model == Color::Model::RGB || m_color_model == Color::Model::RGBA) {
                float range = m_value_range.m_max - m_value_range.m_min;
                if (range > std::numeric_limits<float>::epsilon()) {
                    float s = 1.0 / range;
                    while (ia.stepY()) {
                        while (ia.stepX()) {
                            ia.read();
                            pixel[0] = (pixel[0] - m_value_range.m_min) * s;
                            pixel[1] = (pixel[1] - m_value_range.m_min) * s;
                            pixel[2] = (pixel[2] - m_value_range.m_min) * s;
                            ia.write();
                        }
                    }
                }
            }
        }

        m_value_range.m_min = 0;
        m_value_range.m_max = defaultMaxLevel();
    }


    void Image::clampFloat() noexcept {

        if (isUsable() && m_pixel_type != PixelType::Float) {
            return;
        }

        auto p = (float *)this->pixelDataPtr();
        for (int32_t i = 0; i < totalComponentCount(); i++) {
            if (*p < 0.0f) {
                *p = 0.0f;
            }
            else if (*p > 1.0f) {
                *p = 1.0f;
            }
            p++;
        }
    }


    void Image::linearToGamma() noexcept {

        if (isUsable() && m_pixel_type != PixelType::Float) {
            return;
        }

        auto p = (float *)this->pixelDataPtr();
        for (int32_t i = 0; i < totalComponentCount(); i++) {
            *p = Color::linear_to_gamma(*p);
            p++;
        }
    }


    void Image::unpremultiplyRGBA() noexcept {
        if (hasAlpha()) {
            float pixel[4];
            ImageAccess ia(this, pixel);

            while (ia.stepY()) {
                while (ia.stepX()) {
                    ia.read();
                    if (pixel[3] > 0.0f) {
                        float f = 1.0f / pixel[3];
                        pixel[0] *= f;
                        pixel[1] *= f;
                        pixel[2] *= f;
                    }
                    else {
                        pixel[0] = pixel[1] = pixel[2] = 0.0f;
                    }
                    ia.write();
                }
            }
        }
    }


    ErrorCode Image::applyMatrix(const Mat3f &matrix) noexcept {

        float pixel[4];
        ImageAccess ia(this, pixel);

        auto* m = matrix.dataPtr();
        while (ia.stepY()) {
            while (ia.stepX()) {
                ia.read();
                auto r = m[0] * pixel[0] + m[1] * pixel[1] + m[2] * pixel[2];
                auto g = m[3] * pixel[0] + m[4] * pixel[1] + m[5] * pixel[2];
                auto b = m[6] * pixel[0] + m[7] * pixel[1] + m[8] * pixel[2];
                pixel[0] = r;
                pixel[1] = g;
                pixel[2] = b;
                ia.write();
            }
        }

        return ErrorCode::None;
    }


    ErrorCode Image::applyFilter() noexcept {

        /*

     TODO: Implement!!!
     this->macos_cgImageRef();

     CGImageRef applyFilterToCGImage(CGImageRef inputImage, NSString *filterName, NSDictionary<NSString *, id> *parameters) {
     @autoreleasepool {
     // Step 1: Convert CGImageRef to CIImage
     CIImage *ciImage = [[CIImage alloc] initWithCGImage:inputImage];

     // Step 2: Create and configure the filter
     CIFilter *filter = [CIFilter filterWithName:filterName];
     if (!filter) {
     NSLog(@"Filter %@ not found", filterName);
     return NULL;
     }
     [filter setValue:ciImage forKey:kCIInputImageKey];

     // Apply additional parameters to the filter
     for (NSString *key in parameters) {
     [filter setValue:parameters[key] forKey:key];
     }

     // Get the filtered image
     CIImage *outputCIImage = filter.outputImage;
     if (!outputCIImage) {
     NSLog(@"Failed to apply filter %@ to image", filterName);
     return NULL;
     }

     // Step 3: Render the CIImage to a new CGImageRef
     CIContext *context = [CIContext context];
     CGImageRef outputCGImage = [context createCGImage:outputCIImage fromRect:outputCIImage.extent];

     return outputCGImage; // Caller is responsible for releasing this CGImageRef
     }
     }

     // Example usage
     CGImageRef originalImage = ...; // Your CGImageRef
     CGImageRef filteredImage = applyFilterToCGImage(originalImage, @"CISepiaTone", @{kCIInputIntensityKey: @0.8});

     // Don't forget to release the filtered image if not using ARC
     if (filteredImage) {
     CGImageRelease(filteredImage);
     }
     */
        return ErrorCode::None;
    }


    ErrorCode Image::convolution(int32_t channel, const Dimensioni& kernel_size, const float *kernel_data, Image& out_image) noexcept {

        auto result = ErrorCode::None;

        try {
            if (m_pixel_type != PixelType::Float) { Error::throwSpecific(0); }
            if (!out_image.sameFormat(this)) { Error::throwSpecific(1); }
            if (!out_image.sameSize(this)) { Error::throwSpecific(2); };
            if (!kernel_data) { Error::throwSpecific(3); }
            if (kernel_size.m_width < 1 || kernel_size.m_height < 1) { Error::throwSpecific(4); }
            if (!(kernel_size.m_width & 0x1)) { Error::throwSpecific(5); }
            if (!(kernel_size.m_height & 0x1)) { Error::throwSpecific(6); }

            int32_t kw = kernel_size.m_width;
            int32_t kh = kernel_size.m_height;
            int32_t xo = (kw - 1) / 2;
            int32_t yo = (kh - 1) / 2;

            float* src_pixel_ptr = (float*)pixelDataPtr();
            float *dst_pixel_ptr = (float*)out_image.pixelDataPtr();

            // TODO: !!!!!
            float* sp[kernel_size.m_height];  // Source pointers TODO: Refactor!

            int32_t slo = 0;  // Source line offset
            for (int32_t y = yo; y < m_height - yo; y++) {
                float* dp = &dst_pixel_ptr[y * m_width + xo];  // Destination pointer
                int32_t cslo = slo;  // Current source line offest
                for (int32_t i = 0; i < kernel_size.m_height; i++) {
                    sp[i] = &src_pixel_ptr[cslo];
                    cslo += m_width;
                }

                for (int32_t x = xo; x < m_width - xo; x++) {
                    float v = 0.0f;
                    const float *k = kernel_data;
                    for (int32_t ky = 0; ky < kh; ky++) {
                        for (int32_t kx = 0; kx < kw; kx++) {
                            v += sp[ky][kx] * *k++;
                        }
                        sp[ky]++;
                    }
                    *dp++ = v;
                }

                slo += m_width;
            }
        }
        catch (ErrorCode err) {
            result = err;
        }

        return result;
    }


/**
 *  @brief Function that returns true if the given pixel is valid.
 */
    bool isValid(int screen[][8], int m, int n, int x, int y, int prev_c, int new_c) {

        if (x < 0 || x >= m || y < 0 || y >= n || screen[x][y] != prev_c || screen[x][y] == new_c) {
            return false;
        }
        else {
            return true;
        }
    }


    void Image::floodFill(const Vec2i& pos, const RGB& color, Image& out_image) noexcept {

        // TODO: Implement!
    }


    Image *Image::extractRegion(const Recti& region) noexcept {

        Image *region_image = nullptr;

        Recti image_rect(0, 0, m_width, m_height);
        auto intersection = image_rect.intersection(region);

        if (intersection.usable()) {
            region_image = new(std::nothrow) Image(this, intersection.width(), intersection.height());
            if (region_image) {
                float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                ImageAccess ia_src(this, pixel);
                ImageAccess ia_des(region_image, pixel);
                while (ia_des.stepY()) {
                    while (ia_des.stepX()) {
                        ia_src.setPos(intersection.m_x + ia_des.x(), intersection.m_y + ia_des.y());
                        ia_src.read();
                        ia_des.write();
                    }
                }
            }
        }

        return region_image;
    }


    ErrorCode Image::downscale(Image *dst_image) noexcept {

        if (!dst_image) {
            return ErrorCode::NullData;
        }

        if (dst_image == this) {
            return ErrorCode::MemPointsToItself;
        }

        if (m_color_model != Color::Model::Lumina && m_color_model != Color::Model::LuminaAlpha &&
            m_color_model != Color::Model::RGB && m_color_model != Color::Model::RGBA) {
            return ErrorCode::UnsupportedColorModel;
        }

        if (!sameFormat(dst_image)) {
            return ErrorCode::FormatMismatch;
        }


        int32_t src_width = width();
        int32_t src_height = height();
        int32_t dst_width = dst_image->width();
        int32_t dst_height = dst_image->height();

        float x_ratio = (float)src_width / dst_width;
        float y_ratio = (float)src_height / dst_height;
        int32_t x_block_size = (int32_t)x_ratio;
        int32_t y_block_size = (int32_t)y_ratio;
        int32_t x_rest = src_width % x_block_size;
        int32_t y_rest = src_height % y_block_size;

        int32_t x_block_count = src_width / x_block_size + (x_rest > 0 ? 1 : 0);
        int32_t y_block_count = src_height / y_block_size + (y_rest > 0 ? 1 : 0);
        int32_t block_pixel_count = x_block_size * y_block_size;


        if (x_block_size < 1 || y_block_size < 1) {
            return Error::specific(0);
        }

        auto temp_image = new(std::nothrow) Image(dst_image, x_block_count, y_block_count);
        if (!temp_image) {
            return ErrorCode::MemCantAllocate;
        }

        float src_pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        float temp_pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ImageAccess ia_src(this, src_pixel);
        ImageAccess ia_temp(temp_image, temp_pixel);

        int32_t cn = componentCount();
        float scale = 1.0f / block_pixel_count;
        int32_t y_block_offs = 0;

        while (ia_temp.stepY()) {
            int32_t x_block_offs = 0;

            while (ia_temp.stepX()) {
                for (int32_t ci = 0; ci < cn; ci++) {
                    temp_pixel[ci] = 0.0f;
                }

                for (int32_t i = 0; i < y_block_size; i++) {
                    for (int32_t j = 0; j < x_block_size; j++) {
                        ia_src.setPos(x_block_offs + j, y_block_offs + i);
                        ia_src.read();
                        for (int32_t ci = 0; ci < cn; ci++) {
                            temp_pixel[ci] += src_pixel[ci];
                        }
                    }
                }

                for (int32_t ci = 0; ci < cn; ci++) {
                    temp_pixel[ci] *= scale;
                }

                ia_temp.write();
                x_block_offs += x_block_size;
            }

            y_block_offs += y_block_size;
        }

        float dst_pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ImageAccess dst_access(dst_image, dst_pixel);

        Vec2d pos;
        float y_step = (float)temp_image->height() / (float)dst_height * (float)src_height / (float)(y_block_count * y_block_size);
        float x_step = (float)temp_image->width() / (float)dst_width * (float)src_width / (float)(x_block_count * x_block_size);

        while (dst_access.stepY()) {
            pos.m_x = 0.0f;

            while (dst_access.stepX()) {
                ia_temp.readInterpolated(pos);

                for (int32_t ci = 0; ci < cn; ci++) {
                    float v = temp_pixel[ci];

                    if (v > 1.0f) {
                        v = 1.0f;
                    }
                    else if (v < 0.0f) {
                        v = 0.0f;
                    }

                    dst_pixel[ci] = v;
                }

                dst_access.write();
                pos.m_x += x_step;
            }

            pos.m_y += y_step;
        }

        delete temp_image;

        return ErrorCode::None;
    }


    /**
     *  @brief Creates an NSBitmapImageRep from the internal image pixel data.
     *
     *  This function generates a macOS-native `NSBitmapImageRep` object using the
     *  internal pixel buffer of the image. It creates a Core Graphics bitmap
     *  context, converts it into a CGImage, and then wraps it in an
     *  `NSBitmapImageRep` suitable for display or export.
     *
     *  Although the `use_alpha` parameter is provided, it is forcibly set to `true`
     *  within the function to ensure that the output includes an alpha channel (RGBA).
     *
     *  @return A pointer to a newly allocated `NSBitmapImageRep` object
     *          representing the image, or `nil` if the image is not usable (e.g.,
     *          empty or invalid state).
     *
     *  @note The function assumes `_m_pixel_data` contains valid image data and
     *        that the image dimensions and bit depth are correctly set.
     */
    /* TODO !!!!!!
   NSBitmapImageRep *Image::createNSBitmapImageRep() {

       if (!isUsable()) {
           return nil;
       }

       if (dataType() != Image::PixelType::Float && dataType() != Image::PixelType::UInt8 && dataType() != Image::PixelType::UInt16) {
           return nil;
       }

       std::cout << "....1\n";
       const size_t width_px = static_cast<size_t>(width());
       const size_t height_px = static_cast<size_t>(height());
       const size_t bytes_per_component = isFloat() ? sizeof(float) : 1;
       const size_t components_per_pixel = componentCount();
       const size_t bytes_per_pixel = components_per_pixel * bytes_per_component;
       const size_t bytes_per_row = width_px * bytes_per_pixel;

       std::cout << "width_px: " << width_px << std::endl;
       std::cout << "height_px: " << height_px << std::endl;
       std::cout << "bytes_per_component: " << bytes_per_component << std::endl;
       std::cout << "components_per_pixel: " << components_per_pixel << std::endl;
       std::cout << "bytes_per_pixel: " << bytes_per_pixel << std::endl;
       std::cout << "bytes_per_row: " << bytes_per_row << std::endl;


       CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();


       CGBitmapInfo bitmap_info{};
       bitmap_info |= kCGBitmapByteOrderDefault;

       if (isFloat()) {
           bitmap_info |= kCGBitmapFloatComponents;
       }

       if (hasAlpha()) {
           bitmap_info |= kCGImageAlphaPremultipliedLast;
       }
       else {
           bitmap_info |= kCGImageAlphaNone;
       }


       CGContextRef context = CGBitmapContextCreate(
               _m_pixel_data,
               width(),
               height(),
               bitsPerComponent(),
               bytesPerRow(),
               color_space,
               bitmap_info
       );

       if (!context) {
//        return nil;
       }


       CGImageRef cg_image = CGBitmapContextCreateImage(context);
       NSBitmapImageRep *bitmap_rep = [[NSBitmapImageRep alloc] initWithCGImage:cg_image];

       std::cout << "....7\n";
       // Cleanup
       CGImageRelease(cg_image);
       CGContextRelease(context);
       CGColorSpaceRelease(color_space);

       std::cout << "....8\n";
       return bitmap_rep;
   }
     */

    /* TODO !!!!!!!
    NSBitmapImageRep *Image::createNSBitmapImageRep1(bool use_alpha) {

        if (!isUsable()) return nil;

        NSBitmapImageRep *bitmap_rep = nil;
        NSInteger bps = bitsPerComponent();
        NSInteger spp = componentCount();
        NSInteger pixel_bits = spp * bps;
        NSInteger row_bytes = width() * (pixel_bits / 8);

        NSString *color_space_name = nil;
        switch (colorModel()) {
            case Color::Model::Lumina:
            case Color::Model::LuminaAlpha:
                color_space_name = NSDeviceWhiteColorSpace;
                break;
            case Color::Model::RGB:
            case Color::Model::RGBA:
                color_space_name = NSDeviceRGBColorSpace;
                break;
            default:
                return nil;
        }

        auto used_image = this;
        bool free_image = false;

        // Adjust spp and pixel_bits if alpha is disabled
        if (hasAlpha() && !use_alpha) {
            used_image = nullptr;
            if (colorModel() == Color::Model::RGBA) {
                used_image = Image::createRGBFloat(m_width, m_height);
            }

            if (!used_image) {
                return nil;
            }

            free_image = true;

            float pixel[4];
            ImageAccess src_ia(this, pixel);
            ImageAccess dst_ia(used_image, pixel);
            while (src_ia.stepY()) {
                while (src_ia.stepX()) {
                    src_ia.read();
                    dst_ia.setPos(src_ia.x(), src_ia.y());
                    dst_ia.write();
                }
            }

            spp -= 1;
            pixel_bits = spp * bps;
            row_bytes = width() * (pixel_bits / 8);
        }

    std::cout << "hasAlpha(): " << hasAlpha() << std::endl;
    std::cout << "use_alpha: " << use_alpha << std::endl;
    std::cout << "bps: " << bps << std::endl;
    std::cout << "spp: " << spp << std::endl;
    std::cout << "pixel_bits: " << pixel_bits << std::endl;
    std::cout << "row_bytes: " << row_bytes << std::endl;

        unsigned char *plane_ptr[4]{};
        plane_ptr[0] = (unsigned char*)used_image->pixelDataPtr();

        NSUInteger bitmap_format = isFloat() ? NSBitmapFormatFloatingPointSamples : 0;
        bitmap_rep =
        [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:plane_ptr
        pixelsWide:width()
        pixelsHigh:height()
        bitsPerSample:bps
        samplesPerPixel:spp
        hasAlpha:use_alpha
        isPlanar:false
        colorSpaceName:color_space_name
        bitmapFormat:bitmap_format
        bytesPerRow:row_bytes
        bitsPerPixel:pixel_bits];

        if (free_image) {
            delete used_image;
        }

        return bitmap_rep;
    }
    */


    /* TODO !!!!!!!
    NSBitmapImageRep *Image::createNSBitmapImageRepWithBPS(int16_t bits_per_sample) {

        // TODO: Error with unequal sizes!

        if (!isUsable()) {
            return nil;
        }

        bool use_pixel_data = false;

        switch (bits_per_sample) {
            case 8:
                if (m_pixel_type == Image::PixelType::UInt8) {
                    use_pixel_data = true;
                }
                break;

            case 16:
                if (m_pixel_type == Image::PixelType::UInt16) {
                    use_pixel_data = true;
                }
                break;

            default:
                return nil;
        }


        NSInteger spp = _m_components_per_pixel;
        BOOL has_alpha = NO;
        NSInteger pixel_bits = _m_components_per_pixel * bits_per_sample;
        NSInteger row_bytes = m_width * (pixel_bits / 8);
        NSString *color_space_name = nil;


        switch (m_color_model) {
            case Color::Model::Lumina:
                color_space_name = NSDeviceWhiteColorSpace;  // NSCalibratedWhiteColorSpace
                break;

            case Color::Model::LuminaAlpha:
                color_space_name = NSDeviceWhiteColorSpace;  // NSCalibratedWhiteColorSpace
                break;

            case Color::Model::RGB:
                color_space_name = NSDeviceRGBColorSpace;  // NSCalibratedRGBColorSpace
                break;

            case Color::Model::RGBA:
                color_space_name = NSDeviceRGBColorSpace;  // NSCalibratedRGBColorSpace
                break;

            case Color::Model::CMYK:
                color_space_name = NSDeviceCMYKColorSpace;
                break;

            default:
                return nil;
        }


        unsigned char **planes = nullptr;

        if (use_pixel_data) {
            unsigned char *plane_ptr[4]{};
            plane_ptr[0] = (unsigned char*)_m_pixel_data;
            planes = plane_ptr;
        }

        NSBitmapImageRep *bitmap_rep = nil;

        bitmap_rep =
        [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:planes
        pixelsWide:m_width
        pixelsHigh:m_height
        bitsPerSample:bits_per_sample
        samplesPerPixel:spp
        hasAlpha:has_alpha
        isPlanar:false
        colorSpaceName:color_space_name
        bytesPerRow:row_bytes
        bitsPerPixel:pixel_bits];

        if (!planes && bitmap_rep != nil) {
            copyToNSBitmapImageRep(bitmap_rep);
        }

        return bitmap_rep;
    }
     */

/* TODO !!!!!!
    bool Image::copyToNSBitmapImageRep(NSBitmapImageRep *bitmap_rep) {

        // TODO: Compare pixelsWide, pixelsHigh, bitsPerSample, samplesPerPixel, isPlanar, hasAlpha!

        if (![bitmap_rep bitmapData]) {
            return false;
        }

        if (_m_components_per_pixel != 1 && _m_components_per_pixel != 3 && _m_components_per_pixel != 4) {
            return false;
        }

        if (m_pixel_type == Image::PixelType::UInt8) {
            if ([bitmap_rep bitsPerSample] == 16) {

                uint8_t* s = (uint8_t*)pixelDataPtr();
                uint16_t* d = (uint16_t*)([bitmap_rep bitmapData]);

                int32_t n = totalComponentCount();
                while (n--) {
                    *d++ = (uint16_t)*s++ << 8;
                }
            }
        }
        else if (m_pixel_type == Image::PixelType::UInt16) {
            if ([bitmap_rep bitsPerSample] == 8) {

                uint16_t* s = (uint16_t*)pixelDataPtr();
                uint8_t* d = (uint8_t*)([bitmap_rep bitmapData]);

                int32_t n = totalComponentCount();
                while (n--) {
                    *d++ = (uint16_t)(*s++ >> 8);
                }
            }
        }
        else if (m_pixel_type == Image::PixelType::Float) {
            float* s = (float*)pixelDataPtr();

            if ([bitmap_rep bitsPerSample] == 16) {
                uint16_t* d = (uint16_t*)([bitmap_rep bitmapData]);
                int32_t n = totalComponentCount();
                while (n--) {
                    *d++ = (uint16_t)(std::clamp<float>(*s, 0.0f, 1.0f) * std::numeric_limits<uint16_t>::max());
                    s++;
                }
            }
            else if ([bitmap_rep bitsPerSample] == 8) {
                uint8_t* d = (uint8_t*)([bitmap_rep bitmapData]);
                int32_t n = totalComponentCount();
                while (n--) {
                    *d++ = (uint8_t)(std::clamp<float>(*s, 0.0f, 1.0f) * std::numeric_limits<uint8_t>::max());
                    s++;
                }
            }
        }

        return true;
    }
*/

/* TODO !!!!!!

    bool Image::copyFromNSBitmapImageRep(NSBitmapImageRep *bitmap_rep) {

        if (bitmap_rep == nil) {
            return false;
        }

        float pixel_values[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ImageAccess src_access(bitmap_rep);
        ImageAccess dst_access(this, pixel_values);

        src_access.setTransferPtr_r32(pixel_values);

        while (src_access.stepY()) {
            while (src_access.stepX()) {
                src_access.read();
                dst_access.setPos(src_access.x(), src_access.y());
                dst_access.write();
            }
        }

        return true;
    }
*/

#if defined(__APPLE__) && defined(__MACH__)
    bool Image::macos_buildCGImageRef() noexcept {

        macos_releaseCGImageRef();

        CGColorSpaceRef cg_color_space;
        switch (m_color_model) {
            case Color::Model::Lumina:
            case Color::Model::LuminaAlpha:
            case Color::Model::Bayer:
                cg_color_space = CGColorSpaceCreateDeviceGray();
                break;

            default:
                cg_color_space = CGColorSpaceCreateDeviceRGB();
                break;
        }

        CFDataRef cg_data = CFDataCreateWithBytesNoCopy(NULL, (const UInt8*)_m_pixel_data, _m_mem_size, kCFAllocatorNull);
        CGDataProviderRef cg_data_provider = CGDataProviderCreateWithCFData(cg_data);
        CGBitmapInfo cg_bitmap_info = macos_cgBitmapInfo();

        _m_cg_image_ref = CGImageCreate(m_width, m_height,
                                        bitsPerComponent(),
                                        bitsPerPixel(),
                                        bytesPerRow(),
                                        cg_color_space,
                                        cg_bitmap_info,
                                        cg_data_provider,
                                        NULL,
                                        true,
                                        kCGRenderingIntentDefault);

        CGDataProviderRelease(cg_data_provider);
        CFRelease(cg_data);
        CGColorSpaceRelease(cg_color_space);

        return _m_cg_image_ref;
    }
#endif


#if defined(__APPLE__) && defined(__MACH__)
    void Image::macos_releaseCGImageRef() noexcept {

        if (_m_cg_image_ref) {
            CGImageRelease(_m_cg_image_ref);
            _m_cg_image_ref = nullptr;
        }
    }
#endif


    ErrorCode Image::writeTypedTiff(const String& file_path, Image::PixelType pixel_type, bool drop_alpha) noexcept {
        auto result = ErrorCode::None;
        TiffFile *tiff_file = nullptr;

        try {
            tiff_file = new TiffFile(file_path);
            if (!tiff_file) { throw ErrorCode::MemCantAllocate; }

            tiff_file->startWriteOverwrite();
            tiff_file->setDropAlpha(drop_alpha);

            tiff_file->writeImage(this, Image::pixelTypeDataType(pixel_type));
        }
        catch (ErrorCode err) {
            result = err;
        }

        delete tiff_file;

        return result;
    }


    /**
     *  @brief Writes the image to a file with the specified format and compression settings.
     *
     *  This function saves the image to the given file path using the specified format, bit depth,
     *  and compression factor. The compression factor is applicable to lossy formats like JPEG or HEIC.
     *
     *  @param file_path The path where the image will be saved.
     *  @param type The FourCC code representing the image format (e.g., 'jpg ', 'png ', 'webp').
     *  @param quality The compression level (only for lossy formats):
     *         - `1.0` → Highest quality, least compression.
     *         - `0.0` → Lowest quality, maximum compression.
     *         - Ignored for lossless formats such as PNG.
     *  @return `ErrorCode::None` if the image is saved successfully; otherwise, returns an error code.
     *
     *  @note The function automatically determines if compression is applicable based on the file type.
     *  @note If the file type does not support the specified bit depth, it may be adjusted automatically.
     */
    ErrorCode Image::writeImage(const String& file_path, fourcc_t type, float quality, bool use_alpha) noexcept {

        /* TODO !!!!!!!
         *  Can evt. be dropped due to the usage of other os independent libs
        auto result = ErrorCode::None;

        NSBitmapImageFileType ns_file_type = NSBitmapImageFileTypeJPEG;
        switch (type) {
            case 'tiff': ns_file_type = NSBitmapImageFileTypeTIFF; break;
            case 'bmp ': ns_file_type = NSBitmapImageFileTypeBMP; break;
            case 'gif ': ns_file_type = NSBitmapImageFileTypeGIF; break;
            case 'jpg ': ns_file_type = NSBitmapImageFileTypeJPEG; break;
            case 'png ': ns_file_type = NSBitmapImageFileTypePNG; break;
            case 'jpg2': ns_file_type = NSBitmapImageFileTypeJPEG2000; break;
            default:
                return ErrorCode::BadArgs;
        }

        @autoreleasepool {
                NSBitmapImageRep *bitmap_rep = createNSBitmapImageRep();
                if (bitmap_rep != nil) {

                    NSDictionary *properties = @{
                        NSImageCompressionFactor: @(quality),  // Adjust compression factor if needed
                                NSImageColorSyncProfileData: [[NSColorSpace sRGBColorSpace] ICCProfileData],
                        // NSImageColorSyncProfileData: [standardSRGBColorSpace ICCProfileData]
                    };

                    NSString *ns_file_path = [[NSString alloc] initWithUTF8String:file_path.utf8()];
                    NSData *data = [bitmap_rep representationUsingType:ns_file_type properties:properties];

                    if ([data writeToFile:ns_file_path atomically: NO] == false) {
                        result = ErrorCode::FileCantWrite;
                    }

                    [bitmap_rep release];
                    [ns_file_path release];
                }
                else {
                    result = Error::specific(kErrUnableToCreateNSBitmapImageRep);
                }
        }

        return result;
         */
        return ErrorCode::None;
    }


    ErrorCode Image::writeTiff(const String& file_path, float quality, bool use_alpha) {

        auto result = ErrorCode::None;

        TIFF* tif = TIFFOpen(file_path.utf8(), "w");
        if (!tif) {
            return ErrorCode::FileCantWrite;
        }

        int32_t samples_per_pixel = 0;
        int32_t bits_per_sample = 0;
        int32_t tiff_photometric = -1;
        bool has_alpha = false;
        int32_t sample_format = -1;

        switch (m_color_model) {
            case Color::Model::Lumina:
                samples_per_pixel = 1;
                tiff_photometric = PHOTOMETRIC_MINISBLACK;
                break;
            case Color::Model::RGB:
                samples_per_pixel = 3;
                tiff_photometric = PHOTOMETRIC_RGB;
                break;
            case Color::Model::RGBA:
                samples_per_pixel = 3;
                tiff_photometric = PHOTOMETRIC_RGB;
                has_alpha = true;
                break;
            default:
                break;
        }

        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        switch (m_pixel_type) {
            case Image::PixelType::UInt8:
                sample_format = SAMPLEFORMAT_UINT;
                bits_per_sample = 8;
                break;
            case Image::PixelType::UInt16:
                sample_format = SAMPLEFORMAT_UINT;
                bits_per_sample = 16;
                break;
            case Image::PixelType::UInt32:
            case Image::PixelType::Float:
                sample_format = SAMPLEFORMAT_IEEEFP;
                bits_per_sample = 32;
                break;
            default:
                break;
        }

        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, m_width);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, m_height);
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, sample_format);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samples_per_pixel);
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE); // No compression
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1); // Write one strip for simplicity
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, tiff_photometric);
        TIFFSetField(tif, TIFFTAG_ICCPROFILE, sizeof(kICC_sRGB2_data), kICC_sRGB2_data);

        if (has_alpha) {
            // Declare alpha channel explicitly
            uint16_t extraSample = EXTRASAMPLE_UNASSALPHA;
            TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, 1, &extraSample);
        }


        // Write each scanline
        for (int32_t row = 0; row < m_height; ++row) {
            void* row_data_ptr = pixelDataPtrAtRow(row);
            if (TIFFWriteScanline(tif, row_data_ptr, row, 0) < 0) {
                result = ErrorCode::FileCantWrite;
                break;
            }
        }

        TIFFClose(tif);

        return result;
    }


    ErrorCode Image::writeJpg(const String& file_path, float quality) {

        auto result = ErrorCode::None;

        if (m_color_model != Color::Model::RGB &&
            m_color_model != Color::Model::RGBA &&
            m_color_model != Color::Model::Lumina &&
            m_color_model != Color::Model::LuminaAlpha) {
            return ErrorCode::UnsupportedColorModel;
        }

        if (m_pixel_type != PixelType::UInt8 || m_color_model != Color::Model::RGB) {
            // Pixel data must be converted to uint8_t RGB
            Image* temp_image = copyWithNewSettings(Color::Model::RGB, Image::PixelType::UInt8);
            if (!temp_image) {
                return Error::specific(10);
            }
            result = temp_image->writeJpg(file_path, quality);
            delete temp_image;
            return result;
        }

        FILE* fp = nullptr;

        try {
            fp = fopen(file_path.utf8(), "wb");
            if (!fp) { throw ErrorCode::FileCantCreate; }

            jpeg_compress_struct cinfo;
            jpeg_error_mgr jerr;

            cinfo.err = jpeg_std_error(&jerr);
            jpeg_create_compress(&cinfo);

            jpeg_stdio_dest(&cinfo, fp);

            cinfo.image_width = width();
            cinfo.image_height = height();
            cinfo.input_components = 3;
            cinfo.in_color_space = JCS_RGB;

            jpeg_set_defaults(&cinfo);
            jpeg_set_quality(&cinfo, static_cast<int32_t>(quality * 100), TRUE);

            jpeg_start_compress(&cinfo, TRUE);

            while (cinfo.next_scanline < cinfo.image_height) {
                JSAMPROW row_ptr = pixelDataPtrAtRow(cinfo.next_scanline);
                if (!row_ptr) { throw ErrorCode::FileCantCreate; }
                jpeg_write_scanlines(&cinfo, &row_ptr, 1);
            }

            jpeg_finish_compress(&cinfo);
            jpeg_destroy_compress(&cinfo);
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        // Clean up
        {
            if (fp) {
                fclose(fp);
            }
        }

        return result;
    }


    /**
     *  @brief Writes the image to a PNG file.
     *
     *  @param compression_level Compression level, can be an integer between:
     *                           0 → No compression (fastest, largest file)
     *                           1–8 → Balanced compression
     *                           9 → Maximum compression (slowest, smallest file)
     *
     *  @param use_alpha Whether to include the alpha channel.
     *                   Pass `true` to preserve alpha, or `false` to discard it.
     *
     *  @return ErrorCode indicating success or failure.
     */
    ErrorCode Image::writePng(const String& file_path, int32_t compression_level, bool use_alpha) {

        auto result = ErrorCode::None;

        if (m_color_model != Color::Model::RGB &&
            m_color_model != Color::Model::RGBA &&
            m_color_model != Color::Model::Lumina &&
            m_color_model != Color::Model::LuminaAlpha) {
            return ErrorCode::UnsupportedColorModel;
        }

        if ((m_pixel_type != PixelType::UInt8 && m_pixel_type != PixelType::UInt16) ||
            (m_color_model == Color::Model::LuminaAlpha && !use_alpha) ||
            (m_color_model == Color::Model::RGBA && !use_alpha)) {

            // Pixel data must be converted
            Color::Model use_color_model = m_color_model;
            if (m_color_model == Color::Model::LuminaAlpha && !use_alpha) {
                use_color_model = Color::Model::Lumina;
            }
            else if (m_color_model == Color::Model::RGBA && !use_alpha) {
                use_color_model = Color::Model::RGB;
            }

            if (use_color_model == m_color_model && m_png_fallback_pixel_type == m_pixel_type) {
                // Nothing changed, this is unexpected behaviour!
                return ErrorCode::UnexpectedBehaviour;
            }

            Image* temp_image = copyWithNewSettings(use_color_model, m_png_fallback_pixel_type);
            if (!temp_image) { return Error::specific(10); }

            result = temp_image->writePng(file_path, compression_level, use_alpha);
            delete temp_image;
            return result;
        }


        FILE* fp = nullptr;
        png_structp png_ptr = nullptr;
        png_infop info_ptr = nullptr;
        png_bytep *row_pointers = nullptr;

        try {
            fp = fopen(file_path.utf8(), "wb");
            if (!fp) { throw ErrorCode::FileCantCreate; }

            png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
            if (!png_ptr) { throw Error::specific(1); }

            png_set_compression_level(png_ptr, compression_level);

            info_ptr = png_create_info_struct(png_ptr);
            if (!info_ptr) { throw Error::specific(2); }

            if (setjmp(png_jmpbuf(png_ptr))) {
                throw Error::specific(3);
            }

            png_init_io(png_ptr, fp);

            int color_type;
            switch (m_color_model) {
                case Color::Model::Lumina:
                    color_type = PNG_COLOR_TYPE_GRAY;
                    break;
                case Color::Model::LuminaAlpha:
                    color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
                    break;
                case Color::Model::RGB:
                    color_type = PNG_COLOR_TYPE_RGB;
                    break;
                case Color::Model::RGBA:
                    color_type = PNG_COLOR_TYPE_RGBA;
                    break;
                default:
                    throw Error::specific(99);
            }

            int bit_depth = bitsPerComponent();
            int width_px = width();
            int height_px = height();
            // int32_t channels = componentCount();
            // int32_t bytes_per_pixel = bytesPerPixel();
            int bytes_per_row = bytesPerRow();


            std::cout << "bit_depth: " << bit_depth << std::endl;
            std::cout << "width_px: " << width_px << std::endl;
            std::cout << "height_px: " << height_px << std::endl;
            // std::cout << "channels: " << channels << std::endl;
            // std::cout << "bytes_per_pixel: " << bytes_per_pixel << std::endl;
            std::cout << "bytes_per_row: " << bytes_per_row << std::endl;


            png_set_IHDR(
                    png_ptr, info_ptr,
                    width_px, height_px,
                    bit_depth,
                    color_type,
                    PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT,
                    PNG_FILTER_TYPE_DEFAULT
            );


            png_write_info(png_ptr, info_ptr);


#if BYTE_ORDER == LITTLE_ENDIAN
            png_set_swap(png_ptr);
#endif

            row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height_px);
            if (!row_pointers) { throw Error::specific(4); }

            for (int y = 0; y < height_px; y++) {
                row_pointers[y] = pixelDataPtrAtRow(y);
            }

            png_write_image(png_ptr, row_pointers);
            png_write_end(png_ptr, nullptr);
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        // Clean up
        {

            png_destroy_write_struct(&png_ptr, &info_ptr);

            if (fp) {
                fclose(fp);
            }

            free(row_pointers);
        }

        return result;
    }



    ErrorCode Image::writeWebP(const String& file_path, float quality, bool use_alpha) {

        auto result = ErrorCode::None;
        size_t webp_size = 0;
        uint8_t *webp_data = nullptr;
        uint8_t *byte_data = nullptr;
        bool lossless = quality > 0.99f;
        bool has_alpha = false;
        bool encode_with_alpha = false;

        try {
            int32_t stride = 0;
            if (m_color_model == Color::Model::RGB) {
                stride = 3;
                byte_data = (uint8_t*)std::malloc(m_width * m_height * stride);
            }
            else if (m_color_model == Color::Model::RGBA) {
                has_alpha = use_alpha;
                stride = use_alpha ? 4 : 3;
                encode_with_alpha = use_alpha;
                byte_data = (uint8_t*)std::malloc(m_width * m_height * stride);
            }
            else if (m_color_model == Color::Model::Lumina) {
                // Pixel data must be converted
                Image* temp_image = copyWithNewSettings(Color::Model::RGB, PixelType::UInt8);
                if (!temp_image) {
                    return Error::specific(10);
                }
                result = temp_image->writeWebP(file_path, quality, use_alpha);
                delete temp_image;
                return result;
            }
            else if (m_color_model == Color::Model::LuminaAlpha) {
                // Pixel data must be converted
                Image* temp_image = copyWithNewSettings(Color::Model::RGBA, PixelType::UInt8);
                if (!temp_image) {
                    return Error::specific(10);
                }
                result = temp_image->writeWebP(file_path, quality, use_alpha);
                delete temp_image;
                return result;
            }
            else {
                throw ErrorCode::UnsupportedColorModel;
            }

            if (!byte_data) { throw Error::specific(kErrNoBufferForConversion); }

            float pixel[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
            ImageAccess ia(this, pixel);

            if (has_alpha && encode_with_alpha) {
                uint8_t* d = byte_data;
                while (ia.stepY()) {
                    while (ia.stepX()) {
                        ia.read();
                        *d++ = Type::floatToUInt8(pixel[0]);
                        *d++ = Type::floatToUInt8(pixel[1]);
                        *d++ = Type::floatToUInt8(pixel[2]);
                        *d++ = Type::floatToUInt8(pixel[3]);
                    }
                }
                if (lossless) {
                    webp_size = WebPEncodeLosslessRGBA(byte_data, m_width, m_height, m_width * stride, &webp_data);
                }
                else {
                    webp_size = WebPEncodeRGBA(byte_data, m_width, m_height, m_width * stride, quality * 100, &webp_data);
                }
            }
            else {
                uint8_t* d = byte_data;
                while (ia.stepY()) {
                    while (ia.stepX()) {
                        ia.read();
                        *d++ = Type::floatToUInt8(pixel[0]);
                        *d++ = Type::floatToUInt8(pixel[1]);
                        *d++ = Type::floatToUInt8(pixel[2]);
                    }
                }
                if (lossless) {
                    webp_size = WebPEncodeLosslessRGB(byte_data, m_width, m_height, m_width * stride, &webp_data);
                }
                else {
                    webp_size = WebPEncodeRGB(byte_data, m_width, m_height, m_width * stride, quality * 100, &webp_data);
                }
            }


            if (webp_size == 0) { throw Error::specific(kErrWebPEncodingFailed); }

            File::removeFile(file_path);
            File file(file_path);
            file.startWriteOverwrite();
            file.write8BitData(webp_data, webp_size);
            file.flush();
            file.close();
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Unknown;
        }

        // Cleanup
        if (byte_data) {
            std::free(byte_data);
        }

        if (webp_data) {
            WebPFree(webp_data);
        }

        return result;
    }


    /**
     *  @brief Convert a XYZFile file to a ValueGrid file.
     *
     *  @param cvf2_file_path File path of output file in ValueGrid format.
     *  @param srid Spatial Reference System Identifier (SRID) used for geographic applications.
     *  @param bbox Bounding box of the data.
     *  @param length_unit Unit for the data in the generated ValueGrid file.
     *  @param z_decimals Number of significant decimal places.
     *  @param min_digits Parameter for compression.
     *  @param max_digits Parameter for compression.
     *
     *  @return An error code or `ErrorCode::None`.
     */
    ErrorCode Image::writeCVF2File(const String& cvf2_file_path, int32_t srid, const RangeRectFix& bbox, LengthUnit length_unit, int32_t z_decimals, int32_t min_digits, int32_t max_digits) noexcept {

        /* TODO !!!!!!
        auto result = ErrorCode::None;

        ValueGrid *ValueGrid = nullptr;

        try {
            if (z_decimals < 1 || z_decimals > 9) {
                throw ErrorCode::BadArgs;
            }

            int64_t img_width = width();
            int64_t img_height = height();

            if (hasPixel() != true) {
                throw ErrorCode::NoData;
            }

            ValueGrid = new(std::nothrow) ValueGrid((int32_t)img_width, (int32_t)img_height, length_unit, min_digits, max_digits);
            if (!ValueGrid) {
                throw ErrorCode::ClassInstantiationFailed;
            }

            ValueGrid->setSRID(srid);
            ValueGrid->setBbox(bbox);
            ValueGrid->openFileToWrite(cvf2_file_path);


            // Convert all values
            updateSampleValueRange();

            auto value_range = m_value_range.m_max - m_value_range.m_min;
            float scale = 1.0;
            if (value_range > std::numeric_limits<float>::epsilon()) {
                scale = 1.0 / value_range;
            }
            float offs = m_value_range.m_min;

            float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            ImageAccess ia(this, pixel);

            while (ia.stepY()) {
                while (ia.stepX()) {
                    ia.read();
                    Fix p = offs + pixel[0] * scale;
                    ValueGrid->pushValueToData(ia.x(), ia.y(), p.asInt64(z_decimals));
                }
            }

            ValueGrid->encodeData();
            ValueGrid->finish();
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Fatal;
        }

        delete ValueGrid;

        return result;
         */

        return ErrorCode::None;
    }


    Image::FileType Image::fileTypeByFormatName(const String& file_format_name) noexcept {

        static const char *knownFileFormats[] = {
                "png", "jpg", "webp", "tiff", nullptr
        };
        static const FileType knownFileTypes[] = {
                FileType::PNG, FileType::JPG, FileType::WEBP, FileType::TIFF
        };

        int32_t index = 0;
        while (knownFileFormats[index]) {
            if (file_format_name.compareIgnoreCase(knownFileFormats[index]) == 0) {
                return knownFileTypes[index];
            }
            index++;
        }

        return FileType::Unknown;
    }


    bool Image::isKnownFileType(FileType file_type) noexcept {

        return file_type >= FileType::First && file_type < FileType::Count;
    }


    const char *Image::fileTypeExtension(FileType file_type) noexcept {

        constexpr const char *unknown_extension = "";
        constexpr const char *known_extensions[] = {
                "png", "jpg", "webp", "tiff", ""
        };

        if (isKnownFileType(file_type)) {
            return known_extensions[(int32_t)file_type];
        }
        else {
            return unknown_extension;
        }
    }


    ErrorCode Image::copyImageData(const ImageAccess& src_image_access) noexcept {

        if (src_image_access.width() != m_width || src_image_access.height() != m_height) {
            return Error::specific(0);
        }

        float pixel_values[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ImageAccess dst_ia(this, pixel_values);
        ImageAccess src_ia = src_image_access;
        src_ia.setTransferPtr_r32(pixel_values);

        while (src_ia.stepY()) {
            while (src_ia.stepX()) {
                src_ia.read();
                dst_ia.setPos(src_ia.x(), src_ia.y());
                dst_ia.write();
            }
        }

        return ErrorCode::None;
    }


    Image *Image::createFromFile(const String& file_path, Image::PixelType data_type) {
        #if defined(__APPLE__) && defined(__MACH__)
            return _mac_loadImageFromFile(file_path, data_type);
        #else
            #warning "loadImageFromFile is only implemented for macOS
            return nullptr;
        #endif
    }


    Image *Image::createFromRawFile(const String& file_path, Image::PixelType data_type) {
        Image *image = nullptr;
        LibRaw lr;

        // TODO: !!! This is experimental code ...

        int ret;
        if ((ret = lr.open_file(file_path.utf8())) != LIBRAW_SUCCESS) {
            fprintf(stderr, "Cannot open %s: %s\n", file_path.utf8(), libraw_strerror(ret));
            return nullptr;
        }

        if ((ret = lr.unpack()) != LIBRAW_SUCCESS) {
            fprintf(stderr, "Cannot unpack %s: %s\n", file_path.utf8(), libraw_strerror(ret));
            return nullptr;
        }

        auto &c = lr.imgdata.color;
        std::cout << "Cam → XYZFile:\n";
        for (int32_t i = 0; i < 3; i++) {
            for (int32_t j = 0; j < 3; j++) {
                std::cout << c.cam_xyz[i][j] << " | ";
            }
        }
        std::cout << "\nCam → sRGB:\n";
        for (int32_t i = 0; i < 3; i++) {
            for (int32_t j = 0; j < 4; j++) {
                std::cout << c.rgb_cam[i][j] << " | ";
            }
        }
        std::cout << "\nWB:\n";
        for (int32_t i = 0; i < 4; i++) {
            std::cout << c.pre_mul[i] << " | ";
        }
        std::cout << "\nCanonical WB:\n";
        for (int32_t i = 0; i < 4; i++) {
            std::cout << c.cam_mul[i] << " | ";
        }

        // lr.subtract_black();
        // RawProcessor.raw2image();
        // RawProcessor.dcraw_process();

        if (data_type == PixelType::UInt16) {
            image = new(std::nothrow) Image(Color::Model::Lumina, lr.imgdata.sizes.width, lr.imgdata.sizes.height, Image::PixelType::UInt16);

            if (image) {
                auto s = (uint16_t*)lr.imgdata.rawdata.raw_image;
                auto d = (uint16_t*)image->pixelDataPtr();
                int32_t s_pitch = lr.imgdata.sizes.raw_pitch / sizeof(uint16_t);
                size_t d_size = sizeof(uint16_t) * image->width();
                size_t d_pitch = image->width();
                for (int32_t y = 0; y < image->height(); y++) {
                    memcpy(d, s, d_size);
                    d += d_pitch;
                    s += s_pitch;
                }
            }
            lr.recycle();
            return image;
        }

        image = new(std::nothrow) Image(Color::Model::Lumina, lr.imgdata.sizes.width, lr.imgdata.sizes.height, Image::PixelType::Float);


        // Camera to XYZFile matrix
        image->m_has_cam_to_xyz_matrix = false;
        image->m_cam_to_xyz_matrix.clear();
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                float v = lr.imgdata.color.cam_xyz[i][j];
                if (v != 0.0f) {
                    image->m_has_cam_to_xyz_matrix = true;
                }
                image->m_cam_to_xyz_matrix.m_data[i][j] = v;
            }
        }


        // RGB to camera matrix
        image->m_has_rgb_to_cam_matrix = false;
        image->m_rgb_to_cam_matrix.clear();
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 4; j++) {
                float v = lr.imgdata.color.rgb_cam[i][j];
                if (v != 0.0f) {
                    image->m_has_rgb_to_cam_matrix = true;
                }
                image->m_rgb_to_cam_matrix.m_data[i][j] = v;
            }
        }

        int black_level = lr.imgdata.color.black;
        std::cout << "black_level: " << black_level << '\n';
        int white_level = lr.imgdata.color.maximum;
        std::cout << "white_level: " << white_level << '\n';

        int linear_max[4];
        for (int i = 0; i < 4; ++i) {
            std::cout << "linear_max[" << i << "]: " << lr.imgdata.rawdata.color.linear_max[i] << std::endl;
        }

        // Black level(s)
        for (int i = 0; i < 4; ++i) {
            std::cout << "Black level [" << i << "]: "
                      << lr.imgdata.color.cblack[i] << '\n';
        }


        image->m_png_fallback_pixel_type = Image::PixelType::UInt16;

        // ushort* s = &RawProcessor.imgdata.image[0][0];

        // int32_t pixelCount = image->pixelCount();
        // int32_t sPixelIndex = 0;

        if (!lr.imgdata.rawdata.raw_image) {
            std::cerr << "raw_image is NULL!" << std::endl;
            return nullptr;
        }

        std::cout << "raw_width: " << lr.imgdata.sizes.raw_width << std::endl;
        std::cout << "raw_height: " << lr.imgdata.sizes.raw_height << std::endl;
        std::cout << "width: " << lr.imgdata.sizes.width << std::endl;
        std::cout << "height: " << lr.imgdata.sizes.height << std::endl;

        std::cout << "RAW w: " << lr.imgdata.rawdata.sizes.raw_width << std::endl;
        std::cout << "RAW h: " << lr.imgdata.rawdata.sizes.raw_height << std::endl;
        std::cout << "RAW p: " << lr.imgdata.rawdata.sizes.raw_pitch << std::endl;

        uint16_t* raw = lr.imgdata.rawdata.raw_image;
        int width = lr.imgdata.sizes.raw_width;
        int height = lr.imgdata.sizes.raw_height;
        int pitch = width;
        float scale = 1.0f / static_cast<float>(white_level);

        std::cout << "raw_width: " << width << std::endl;
        std::cout << "raw_height: " << height << std::endl;
        std::cout << "raw: " << (long)raw << std::endl;

        constexpr fourcc_t grbg = Type::fourcc('G', 'R', 'B', 'G');
        constexpr fourcc_t rggb = Type::fourcc('R', 'G', 'G', 'B');
        constexpr fourcc_t gbrg = Type::fourcc('G', 'B', 'R', 'G');
        constexpr fourcc_t bggr = Type::fourcc('B', 'G', 'G', 'R');
        constexpr fourcc_t rgbg = Type::fourcc('R', 'G', 'B', 'G'); // Lumix S5 returns this, which must be interpreted as 'RGGB'!

        fourcc_t cfa_pattern_code = 0x0;
        for (int32_t i = 0; i < 4; i++) {
            cfa_pattern_code <<= 8;
            cfa_pattern_code |= lr.imgdata.idata.cdesc[i];
        }

        Log l;
        l << "cfa_pattern_code: " << l.fourCCValue(cfa_pattern_code) << Log::endl;
        std::cout << "cfa_pattern_code: " << cfa_pattern_code << std::endl;

        int32_t cfa[2][2]{};

        int32_t cfa_pattern = kCFAPatternUnknown;
        switch (cfa_pattern_code) {
            case grbg:
                cfa[0][0] = 1; cfa[0][1] = 0; cfa[1][0] = 2; cfa[1][1] = 1;
                cfa_pattern = kCFAPatternGRBG;
                break;
            case rggb:
            case rgbg: // Lumix S5 returns this, which must be interpreted as 'RGGB'!
                cfa[0][0] = 0; cfa[0][1] = 1; cfa[1][0] = 1; cfa[1][1] = 2;
                cfa_pattern = kCFAPatternRGGB;
                break;
            case gbrg:
                cfa[0][0] = 1; cfa[0][1] = 2; cfa[1][0] = 0; cfa[1][1] = 1;
                cfa_pattern = kCFAPatternGBRG;
                break;
            case bggr:
                cfa[0][0] = 2; cfa[0][1] = 1; cfa[1][0] = 1; cfa[1][1] = 0;
                cfa_pattern = kCFAPatternBGGR;
                break;
        }


        auto d = (float *)image->pixelDataPtr();

        pitch = static_cast<int>(lr.imgdata.sizes.raw_pitch / sizeof(uint16_t));

        float pixel[4];
        ImageAccess ia(image, pixel);

        uint16_t mmin[2][2] = { { 435, 451 }, { 451, 441 } };
        uint16_t mmax[2][2] = { { 16336, 16336 }, { 16336, 16336 } };

        float cfa_scale[2][2];
        for (int32_t y = 0; y < 2; y++) {
            for (int32_t x = 0; x < 2; x++) {
                int32_t color = cfa[y % 2][x % 2];
                cfa_scale[y][x] = (1.0f / static_cast<float>(mmax[y % 2][x % 2] - mmin[y % 2][x % 2])) * c.pre_mul[color];
                cfa_scale[y][x] = (1.0f / static_cast<float>(mmax[y % 2][x % 2])) * c.pre_mul[color];
            }
        }


        uint16_t min[2][2] = {
                { std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::max() },
                { std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::max() }
        };
        uint16_t max[2][2] = { { 0, 0 }, { 0, 0 } };


        for (int32_t y = 0; y < height; y++) {
            std::cout << y << std::endl;
            for (int32_t x = 0; x < width; x++) {
                ia.setPos(x, y);

                int32_t idx = y * pitch + x;

                if (raw[idx] < min[y % 2][x % 2]) { min[y % 2][x % 2] = raw[idx]; }
                if (raw[idx] > max[y % 2][x % 2]) { max[y % 2][x % 2] = raw[idx]; }

                // int32_t color = cfa[y % 2][x % 2];
                // pixel[0] = (float)raw[idx] * cfa_scale[y % 2][x % 2];
                pixel[0] = (float)(raw[idx] - mmin[y % 2][x % 2]) * cfa_scale[y % 2][x % 2];
                if (pixel[0] < 0.0f) { pixel[0] = 0.0f; }
                else if (pixel[0] > 1.0f) { pixel[0] = 1.0f; }
                ia.write();
            }
        }

        std::cout << "\n... min: \n";
        for (int32_t y = 0; y < 2; y++) {
            for (int32_t x = 0; x < 2; x++) {
                int32_t color = cfa[y % 2][x % 2];
                std::cout << min[y][x] << " | ";
            }
        }

        std::cout << "\n... max: \n";
        for (int32_t y = 0; y < 2; y++) {
            for (int32_t x = 0; x < 2; x++) {
                int32_t color = cfa[y % 2][x % 2];
                std::cout << max[y][x] << " | ";
            }
        }

        return image;
    }


    Image *Image::copyWithNewSettings(
            Color::Model color_model,
            PixelType data_type)
    noexcept {

        // TODO: Choose between 601 and 709 grayscale conversion!
        auto image = new (std::nothrow) Image(color_model, m_width, m_height, data_type);

        if (image) {
            Color::Model s_color_model = m_color_model;
            Color::Model d_color_model = color_model;
            auto s_data_type = m_pixel_type;
            auto d_data_type = data_type;

            if (s_color_model == Color::Model::Lumina) {
                int32_t s_step = 1;
                if (d_color_model == Color::Model::Lumina) {
                    int32_t d_step = 1;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina uint8_t to Lumina uint8_t
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina uint8_t to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0]) << 8;
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina uint8_t to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina uint16_t to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] >> 8);
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina uint16_t to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina uint16_t to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina float to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina float to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina float to Lumina float
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                    }
                }
                else if (d_color_model == Color::Model::LuminaAlpha) {
                    int32_t d_step = 2;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina uint8_t to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                d[1] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina uint8_t to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0]) << 8;
                                d[1] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina uint8_t to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                d[1] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina uint16_t to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] >> 8);
                                d[1] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina uint16_t to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                d[1] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina uint16_t to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                d[1] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina float to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                d[1] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina float to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                d[1] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina float to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                d[1] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
                else if (d_color_model == Color::Model::RGB) {
                    int32_t d_step = 3;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina uint8_t to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina uint8_t to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint16_t>(s[0]) << 8;
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina uint8_t to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina uint16_t to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint8_t>(s[0] >> 8);
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina uint16_t to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina uint16_t to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina float to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina float to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina float to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
                else if (d_color_model == Color::Model::RGBA) {
                    int32_t d_step = 4;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina uint8_t to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                d[3] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina uint8_t to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint16_t>(s[0]) << 8;
                                d[3] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina uint8_t to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                d[3] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina uint16_t to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint8_t>(s[0] >> 8);
                                d[3] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina uint16_t to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                d[3] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina uint16_t to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                d[3] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // Lumina float to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                d[3] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // Lumina float to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                d[3] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // Lumina float to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                d[3] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
            }
            else if (s_color_model == Color::Model::LuminaAlpha) {
                int32_t s_step = 2;
                if (d_color_model == Color::Model::Lumina) {
                    int32_t d_step = 1;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha uint8_t to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha uint8_t to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0]) << 8;
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha uint8_t to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha uint16_t to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] >> 8);
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha uint16_t to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha uint16_t to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha float to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha float to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha float to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
                else if (d_color_model == Color::Model::LuminaAlpha) {
                    int32_t d_step = 2;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha uint8_t to LuminaAlpha uint8_t
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha uint8_t to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0]) << 8;
                                d[1] = static_cast<uint16_t>(s[1]) << 8;
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha uint8_t to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                d[1] = static_cast<float>(s[1]) / std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha uint16_t to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] >> 8);
                                d[1] = static_cast<uint8_t>(s[1] >> 8);
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha uint16_t to LuminaAlpha uint16_t
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha uint16_t to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                d[1] = static_cast<float>(s[1]) / std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha float to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                d[1] = static_cast<uint8_t>(s[1] * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha float to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                d[1] = static_cast<uint16_t>(s[1] * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha float to LuminaAlpha float
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                    }
                }
                else if (d_color_model == Color::Model::RGB) {
                    int32_t d_step = 3;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha uint8_t to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha uint8_t to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint16_t>(s[0]) << 8;
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha uint8_t to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha uint16_t to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint8_t>(s[0] >> 8);
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha uint16_t to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha uint16_t to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha float to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha float to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha float to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
                else if (d_color_model == Color::Model::RGBA) {
                    int32_t d_step = 4;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha uint8_t to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                d[3] = s[1];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha uint8_t to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint16_t>(s[0]) << 8;
                                d[3] = static_cast<uint16_t>(s[1]) << 8;
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha uint8_t to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                d[3] = d[2] = static_cast<float>(s[1]) / std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha uint16_t to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint8_t>(s[0] >> 8);
                                d[3] = static_cast<uint8_t>(s[1] >> 8);;
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha uint16_t to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                d[3] = s[1];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha uint16_t to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                d[3] = static_cast<float>(s[1]) / std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // LuminaAlpha float to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                d[3] = static_cast<uint8_t>(s[1] * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // LuminaAlpha float to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                d[3] = static_cast<uint16_t>(s[1] * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // LuminaAlpha float to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = d[1] = d[2] = s[0];
                                d[3] = s[1];
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
            }
            else if (s_color_model == Color::Model::RGB) {
                int32_t s_step = 3;
                if (d_color_model == Color::Model::Lumina) {
                    int32_t d_step = 1;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB uint8_t to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::u8_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB uint8_t to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::u8_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB uint8_t to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::u8_to_lumina_709(s[0], s[1], s[2]);
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB uint16_t to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::u16_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB uint16_t to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::u16_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB uint16_t to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::float_to_lumina_709(s[0], s[1], s[2]);
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB float to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::float_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB float to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::float_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB float to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::float_to_lumina_709(s[0], s[1], s[2]);
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
                else if (d_color_model == Color::Model::LuminaAlpha) {
                    int32_t d_step = 2;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB uint8_t to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::u8_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                d[1] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB uint8_t to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::u8_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                d[1] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB uint8_t to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::u8_to_lumina_709(s[0], s[1], s[2]);
                                d[1] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB uint16_t to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::u16_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                d[1] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB uint16_t to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::u16_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                d[1] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB uint16_t to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::u16_to_lumina_709(s[0], s[1], s[2]);
                                d[1] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB float to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::float_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                d[1] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB float to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::float_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                d[1] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB float to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::float_to_lumina_709(s[0], s[1], s[2]);
                                d[1] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
                else if (d_color_model == Color::Model::RGB) {
                    int32_t d_step = 3;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB uint8_t to RGB uint8_t
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB uint8_t to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0]) << 8;
                                d[1] = static_cast<uint16_t>(s[1]) << 8;
                                d[2] = static_cast<uint16_t>(s[2]) << 8;
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB uint8_t to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                d[1] = static_cast<float>(s[1]) / std::numeric_limits<uint8_t>::max();
                                d[2] = static_cast<float>(s[2]) / std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB uint16_t to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] >> 8);
                                d[1] = static_cast<uint8_t>(s[1] >> 8);
                                d[2] = static_cast<uint8_t>(s[2] >> 8);
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB uint16_t to RGB uint16_t
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB uint16_t to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                d[1] = static_cast<float>(s[1]) / std::numeric_limits<uint16_t>::max();
                                d[2] = static_cast<float>(s[2]) / std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB float to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                d[1] = static_cast<uint8_t>(s[1] * std::numeric_limits<uint8_t>::max());
                                d[2] = static_cast<uint8_t>(s[2] * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB float to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                d[1] = static_cast<uint16_t>(s[1] * std::numeric_limits<uint16_t>::max());
                                d[2] = static_cast<uint16_t>(s[2] * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB float to RGB float
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                    }
                }
                else if (d_color_model == Color::Model::RGBA) {
                    int32_t d_step = 4;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB uint8_t to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                d[1] = s[1];
                                d[2] = s[2];
                                d[3] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB uint8_t to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0]) << 8;
                                d[1] = static_cast<uint16_t>(s[1]) << 8;
                                d[2] = static_cast<uint16_t>(s[2]) << 8;
                                d[3] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB uint8_t to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                d[1] = static_cast<float>(s[1]) / std::numeric_limits<uint8_t>::max();
                                d[2] = static_cast<float>(s[2]) / std::numeric_limits<uint8_t>::max();
                                d[3] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB uint16_t to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] >> 8);
                                d[1] = static_cast<uint8_t>(s[1] >> 8);
                                d[2] = static_cast<uint8_t>(s[2] >> 8);
                                d[3] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB uint16_t to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                d[1] = s[1];
                                d[2] = s[2];
                                d[3] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB uint16_t to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                d[1] = static_cast<float>(s[1]) / std::numeric_limits<uint16_t>::max();
                                d[2] = static_cast<float>(s[2]) / std::numeric_limits<uint16_t>::max();
                                d[3] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGB float to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                d[1] = static_cast<uint8_t>(s[1] * std::numeric_limits<uint8_t>::max());
                                d[2] = static_cast<uint8_t>(s[2] * std::numeric_limits<uint8_t>::max());
                                d[3] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGB float to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                d[1] = static_cast<uint16_t>(s[1] * std::numeric_limits<uint16_t>::max());
                                d[2] = static_cast<uint16_t>(s[2] * std::numeric_limits<uint16_t>::max());
                                d[3] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGB float to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                d[1] = s[1];
                                d[2] = s[2];
                                d[3] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
            }
            else if (s_color_model == Color::Model::RGBA) {
                int32_t s_step = 4;
                if (d_color_model == Color::Model::Lumina) {
                    int32_t d_step = 1;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA uint8_t to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::u8_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA uint8_t to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::u8_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA uint8_t to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::u8_to_lumina_709(s[0], s[1], s[2]);
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA uint16_t to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::u16_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA uint16_t to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::u16_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA uint16_t to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::u16_to_lumina_709(s[0], s[1], s[2]);
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA float to Lumina uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::float_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA float to Lumina uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::float_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA float to Lumina float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::float_to_lumina_709(s[0], s[1], s[2]);
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
                else if (d_color_model == Color::Model::LuminaAlpha) {
                    int32_t d_step = 2;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA uint8_t to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::u8_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                d[1] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA uint8_t to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::u8_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                d[1] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA uint8_t to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::u8_to_lumina_709(s[0], s[1], s[2]);
                                d[1] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA uint16_t to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::u16_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                d[1] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA uint16_t to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::u16_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                d[1] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA uint16_t to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::u16_to_lumina_709(s[0], s[1], s[2]);
                                d[1] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA float to LuminaAlpha uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(RGB::float_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint8_t>::max());
                                d[1] = std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA float to LuminaAlpha uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(RGB::float_to_lumina_709(s[0], s[1], s[2]) * std::numeric_limits<uint16_t>::max());
                                d[1] = std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA float to LuminaAlpha float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = RGB::float_to_lumina_709(s[0], s[1], s[2]);
                                d[1] = 1.0f;
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
                else if (d_color_model == Color::Model::RGB) {
                    int32_t d_step = 3;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA uint8_t to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                d[1] = s[1];
                                d[2] = s[2];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA uint8_t to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0]) << 8;
                                d[1] = static_cast<uint16_t>(s[1]) << 8;
                                d[2] = static_cast<uint16_t>(s[2]) << 8;
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA uint8_t to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                d[1] = static_cast<float>(s[1]) / std::numeric_limits<uint8_t>::max();
                                d[2] = static_cast<float>(s[2]) / std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA uint16_t to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] >> 8);
                                d[1] = static_cast<uint8_t>(s[1] >> 8);
                                d[2] = static_cast<uint8_t>(s[2] >> 8);
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA uint16_t to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                d[1] = s[1];
                                d[2] = s[2];
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA uint16_t to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                d[1] = static_cast<float>(s[1]) / std::numeric_limits<uint16_t>::max();
                                d[2] = static_cast<float>(s[2]) / std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA float to RGB uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                d[1] = static_cast<uint8_t>(s[1] * std::numeric_limits<uint8_t>::max());
                                d[2] = static_cast<uint8_t>(s[2] * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA float to RGB uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                d[1] = static_cast<uint16_t>(s[1] * std::numeric_limits<uint16_t>::max());
                                d[2] = static_cast<uint16_t>(s[2] * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA float to RGB float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = s[0];
                                d[1] = s[1];
                                d[2] = s[2];
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                }
                else if (d_color_model == Color::Model::RGBA) {
                    int32_t d_step = 4;
                    if (s_data_type == PixelType::UInt8) {
                        auto s = (uint8_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA uint8_t to RGBA uint8_t
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA uint8_t to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0]) << 8;
                                d[1] = static_cast<uint16_t>(s[1]) << 8;
                                d[2] = static_cast<uint16_t>(s[2]) << 8;
                                d[3] = static_cast<uint16_t>(s[3]) << 8;
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA uint8_t to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint8_t>::max();
                                d[1] = static_cast<float>(s[1]) / std::numeric_limits<uint8_t>::max();
                                d[2] = static_cast<float>(s[2]) / std::numeric_limits<uint8_t>::max();
                                d[3] = static_cast<float>(s[3]) / std::numeric_limits<uint8_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::UInt16) {
                        auto s = (uint16_t*)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA uint16_t to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] >> 8);
                                d[1] = static_cast<uint8_t>(s[1] >> 8);
                                d[2] = static_cast<uint8_t>(s[2] >> 8);
                                d[3] = static_cast<uint8_t>(s[3] >> 8);
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA uint16_t to RGBA uint16_t
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA uint16_t to RGBA float
                            auto d = (float *)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<float>(s[0]) / std::numeric_limits<uint16_t>::max();
                                d[1] = static_cast<float>(s[1]) / std::numeric_limits<uint16_t>::max();
                                d[2] = static_cast<float>(s[2]) / std::numeric_limits<uint16_t>::max();
                                d[3] = static_cast<float>(s[3]) / std::numeric_limits<uint16_t>::max();
                                s += s_step;
                                d += d_step;
                            }
                        }
                    }
                    else if (s_data_type == PixelType::Float) {
                        auto s = (float *)_m_pixel_data;
                        if (d_data_type == PixelType::UInt8) {
                            // RGBA float to RGBA uint8_t
                            auto d = (uint8_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint8_t>(s[0] * std::numeric_limits<uint8_t>::max());
                                d[1] = static_cast<uint8_t>(s[1] * std::numeric_limits<uint8_t>::max());
                                d[2] = static_cast<uint8_t>(s[2] * std::numeric_limits<uint8_t>::max());
                                d[3] = static_cast<uint8_t>(s[3] * std::numeric_limits<uint8_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::UInt16) {
                            // RGBA float to RGBA uint16_t
                            auto d = (uint16_t*)image->_m_pixel_data;
                            for (int32_t i = 0; i < _m_pixel_count; i++) {
                                d[0] = static_cast<uint16_t>(s[0] * std::numeric_limits<uint16_t>::max());
                                d[1] = static_cast<uint16_t>(s[1] * std::numeric_limits<uint16_t>::max());
                                d[2] = static_cast<uint16_t>(s[2] * std::numeric_limits<uint16_t>::max());
                                s += s_step;
                                d += d_step;
                            }
                        }
                        else if (d_data_type == PixelType::Float) {
                            // RGBA float to RGBA float
                            memcpy(image->_m_pixel_data, _m_pixel_data, _m_mem_size);
                        }
                    }
                }
            }
        }

        return image;
    }


/* TODO: !
GLenum Image::glGetType() {

    switch (m_pixel_type) {
        case Image::PixelType::UInt8: return GL_UNSIGNED_BYTE;
        case Image::PixelType::UInt16: return GL_UNSIGNED_SHORT;
        case Image::PixelType::Float: return GL_FLOAT;

        default:
            return 0;    // Undefined
    }
}
*/


/* TODO: !
GLenum Image::glGetFormat() {

    switch (getComponentsPerPixel()) {
        case 1: return GL_RED;
        case 2: return GL_RG;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;

        default:
            return 0;    // Undefined
    }
}
*/


/* TODO: Metal!
bool Image::glTexture(GLenum textureUnitID, bool initMode) {

    int16_t depth = getPlaneCount();
    GLenum type = glGetType();

    return GrGL::set2DTexture(textureUnitID, m_width, m_height, depth, getPixelDataPtr(), type, initMode);
}
*/


    void Image::_set(Color::Model color_model, int32_t width, int32_t height, PixelType data_type) {
        m_color_model = color_model;
        m_width = width;
        m_height = height;
        m_pixel_type = data_type;

        _m_int_min = _m_int_max = 0;
        _m_float_min = 0;
        _m_float_max = 1;

        _m_pixel_count = width * height;
        _m_bytes_per_component = pixelTypeByteSize(data_type);
        _m_bits_per_component = pixelTypeBitCount(data_type);
        _m_components_per_pixel = Color::modelComponentsPerPixel(color_model);
        _m_bytes_per_pixel = _m_bytes_per_component * _m_components_per_pixel;
        _m_mem_size = (uint64_t)_m_pixel_count * _m_bytes_per_pixel;
        _m_pixel_data_step = _m_bytes_per_pixel;
        _m_row_data_step = _m_pixel_data_step * width;

        _m_pixel_data = nullptr;

        m_float_type = false;

        switch (data_type) {
            case Image::PixelType::UInt8:
                _m_int_max = std::numeric_limits<uint8_t>::max();
                break;

            case Image::PixelType::UInt16:
                _m_int_max = std::numeric_limits<uint16_t>::max();
                break;

            case Image::PixelType::Float:
                m_float_type = true;
                break;

            default:
                _m_bytes_per_component = 0;
                _m_mem_size = 0;
                break;
        }


        switch (color_model) {
            case Color::Model::LuminaAlpha:
            case Color::Model::RGBA:
                m_has_alpha = true;
                break;

            default:
                m_has_alpha = false;
                break;
        }
    }


    void Image::_malloc() {

        _m_pixel_data = _m_mem_size > 0 ? (uint64_t*)std::malloc(_m_mem_size) : nullptr;
    }


    void Image::_free() {

        std::free(_m_pixel_data);
        _m_pixel_data = nullptr;
        _m_mem_size = 0;
    }


} // End of namespace Grain