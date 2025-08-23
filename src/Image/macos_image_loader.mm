#include "Image/Image.hpp"
#import <Cocoa/Cocoa.h>

namespace Grain {

    void _setImageAccessToNSBitmapImageRep(ImageAccess& image_access, NSBitmapImageRep *bitmap_rep);


    Image* _mac_loadImageFromFile(const String& file_path, Image::PixelType pixel_type) {

        std::cout << "_mac_loadImageFromFile: " << file_path << '\n';

        Image* image = nullptr;
        NSImage* ns_image = nil;
        NSData* ns_image_data = nil;
        NSBitmapImageRep* ns_bitmap_rep = nil;

        try {
            ns_image = [[NSImage alloc] initWithContentsOfFile:[NSString stringWithUTF8String:file_path.utf8()]];
            if (ns_image == nil) {
                return nullptr;
            }

            // Read the NSImage
            ns_image_data = [[NSData alloc] initWithData:[ns_image TIFFRepresentation]];
            ns_bitmap_rep = [[NSBitmapImageRep alloc] initWithData:ns_image_data];

            NSColorSpace* ns_color_space = [ns_bitmap_rep colorSpace];
            if (ns_color_space == nil) {
                throw Error::specific(0);
            }

            NSInteger ns_width = [ns_bitmap_rep pixelsWide];
            NSInteger ns_height = [ns_bitmap_rep pixelsHigh];
            NSInteger ns_bits_per_pixel = [ns_bitmap_rep bitsPerPixel];  // Unused
            NSInteger ns_bits_per_sample = [ns_bitmap_rep bitsPerSample];
            NSInteger ns_number_of_planes = [ns_bitmap_rep numberOfPlanes];
            NSInteger ns_samples_per_pixel = [ns_bitmap_rep samplesPerPixel];  // Unused
            NSBitmapFormat ns_bitmap_format = [ns_bitmap_rep bitmapFormat];
            BOOL ns_has_alpha = [ns_bitmap_rep hasAlpha];
            NSInteger ns_bytes_per_row = [ns_bitmap_rep bytesPerRow];  // Unused

            std::cout << "ns_width: " << ns_width << '\n';
            std::cout << "ns_height: " << ns_height << '\n';
            std::cout << "ns_bits_per_pixel: " << ns_bits_per_pixel << '\n';
            std::cout << "ns_bits_per_sample: " << ns_bits_per_sample << '\n';
            std::cout << "ns_number_of_planes: " << ns_number_of_planes << '\n';
            std::cout << "ns_samples_per_pixel: " << ns_samples_per_pixel << '\n';
            std::cout << "ns_has_alpha: " << ns_has_alpha << '\n';
            std::cout << "ns_bytes_per_row: " << ns_bytes_per_row << '\n';

            NSColorSpaceModel ns_color_space_model = [ns_color_space colorSpaceModel];

            Color::Model src_color_model;

            switch (ns_color_space_model) {
                case NSColorSpaceModelGray:
                    src_color_model = ns_has_alpha ? Color::Model::LuminaAlpha : Color::Model::Lumina;
                    break;

                case NSColorSpaceModelRGB:
                    src_color_model = ns_has_alpha ? Color::Model::RGBA : Color::Model::RGB;
                    break;

                case NSColorSpaceModelCMYK:
                    src_color_model = Color::Model::CMYK;
                    if (ns_has_alpha) {
                        throw Error::specific(1);
                    }
                    break;

                case NSColorSpaceModelLAB:
                    src_color_model = Color::Model::L_a_b;
                    if (ns_has_alpha) {
                        throw Error::specific(2);
                    }
                    break;

                default:
                    throw Error::specific(3);
            }


            switch (ns_bits_per_sample) {
                case 8:
                    break;

                case 16:
                    break;

                case 32:
                    if (!(ns_bitmap_format & NSBitmapFormatFloatingPointSamples)) {
                        throw Error::specific(4);
                    }
                    break;

                default:
                    throw Error::specific(5);
            }

            image = new (std::nothrow) Image(src_color_model, static_cast<uint32_t>(ns_width), static_cast<uint32_t>(ns_height), pixel_type);
            if (!image) {
                throw Error::specific(6);
            }

            // Copy pixel data from NSImage to image
            ImageAccess src_image_access;
            _setImageAccessToNSBitmapImageRep(src_image_access, ns_bitmap_rep);
            image->copyImageData(src_image_access);
        }
        catch (ErrorCode err) {
            if (image) {
                delete image;
                image = nullptr;
            }
        }

        // Cleanup
        [ns_bitmap_rep release];
        [ns_image_data release];
        [ns_image release];

        return image;
    }


    void _setImageAccessToNSBitmapImageRep(ImageAccess& image_access, NSBitmapImageRep *bitmap_rep) {

        ImageAccessSetupInfo ia_setup_info;
        ia_setup_info.m_width = static_cast<uint32_t>([bitmap_rep pixelsWide]);
        ia_setup_info.m_height = static_cast<uint32_t>([bitmap_rep pixelsHigh]);

        if (ia_setup_info.m_width < 1 || ia_setup_info.m_height < 1) {
            return;
        }

        NSColorSpace* ns_color_space = [bitmap_rep colorSpace];
        if (ns_color_space == nil) {
            return;
        }

        NSInteger ns_bits_per_pixel = [bitmap_rep bitsPerPixel];
        NSInteger ns_bits_per_sample = [bitmap_rep bitsPerSample];
        // NSInteger ns_number_of_planes = [bitmap_rep numberOfPlanes]; // Unused

        // TODO: ns_number_of_planes != is an error!

        NSBitmapFormat ns_bitmap_format = [bitmap_rep bitmapFormat];
        BOOL ns_has_alpha = [bitmap_rep hasAlpha];
        NSInteger ns_bytes_per_row = [bitmap_rep bytesPerRow];

        NSColorSpaceModel ns_color_space_model = [ns_color_space colorSpaceModel];

        switch (ns_color_space_model) {
            case NSColorSpaceModelGray:
                ia_setup_info.m_color_model = ns_has_alpha ? Color::Model::LuminaAlpha : Color::Model::Lumina;
                ia_setup_info.m_component_count = ns_has_alpha ? 2 : 1;
                break;

            case NSColorSpaceModelRGB:
                ia_setup_info.m_color_model = ns_has_alpha ? Color::Model::RGBA : Color::Model::RGB;
                ia_setup_info.m_component_count = ns_has_alpha ? 4 : 3;
                break;

            case NSColorSpaceModelCMYK:
                ia_setup_info.m_color_model = Color::Model::CMYK;
                ia_setup_info.m_component_count = 4;
                if (ns_has_alpha)
                    return;
                break;

            case NSColorSpaceModelLAB:
                ia_setup_info.m_color_model = Color::Model::L_a_b;
                ia_setup_info.m_component_count = 3;
                if (ns_has_alpha)
                    return;
                break;

            default:
                return;
        }


        switch (ns_bits_per_sample) {
            case 8:
                ia_setup_info.m_pixel_type = Image::PixelType::UInt8;
                break;

            case 16:
                ia_setup_info.m_pixel_type = Image::PixelType::UInt16;
                break;

            case 32:
                if (ns_bitmap_format & NSBitmapFormatFloatingPointSamples) {
                    ia_setup_info.m_pixel_type = Image::PixelType::Float;
                }
                else {
                    return;
                }
                break;

            default:
                return;
        }


        ia_setup_info.m_pixel_data_ptr = static_cast<uint8_t*>([bitmap_rep bitmapData]);
        ia_setup_info.m_pixel_data_step = static_cast<uint32_t>(ns_bits_per_pixel) / 8;
        ia_setup_info.m_row_data_step = ns_bytes_per_row > 0 ? static_cast<uint32_t>(ns_bytes_per_row) : ia_setup_info.m_pixel_data_step * ia_setup_info.m_width;
        ia_setup_info.m_plane_data_step = ia_setup_info.m_row_data_step * ia_setup_info.m_height;

        image_access.setBySetupInfo(ia_setup_info);
    }


} // namespace Grain
