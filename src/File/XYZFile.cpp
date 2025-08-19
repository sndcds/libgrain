//
//  XYZFile.hpp
//
//  Created by Roald Christesen on 17.11.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
#include "File/XYZFile.hpp"
#include "Image/Image.hpp"
#include "Geo/Geo.hpp"
#include "Geo/GeoProj.hpp"
#include "2d/Data/CVF2.hpp"
#include "2d/Data/CVF2File.hpp"
#include "Time/Timestamp.hpp"
#include "String/StringList.hpp"


namespace Grain {


    XYZFile::XYZFile(const String& file_path) noexcept : File(file_path) {
    }


    XYZFile::~XYZFile() noexcept {
    }


    /**
     *  @brief Scan an XYZ file.
     *
     *  Scans an XYZ file to perform the following tasks:
     *  1. Finds the number of lines in the file.
     *  2. Determines the coordinate range and stores it for later use.
     *
     *  @return `ErrorCode::None` if successful, or an error code otherwise.
     */
    ErrorCode XYZFile::scan() noexcept {
        if (m_scan_done) {
            return ErrorCode::None;
        }

        auto result = ErrorCode::None;
        bool check_x = true;
        bool check_y = true;
        bool has_x1 = false;
        bool has_x2 = false;
        bool has_y1 = false;
        bool has_y2 = false;

        double sum_z = 0.0;

        try {
            checkBeforeReading();

            m_line_count = 0;
            while (readLine(kMaxLineLength, m_line)) {
                if (m_line.isEmpty()) {
                    continue;
                }

                if (!m_last_coordinate.setByCSV(m_line, ' ')) {
                    throw ErrorCode::UnexpectedData;
                }

                if (m_line_count == 0) {
                    m_range = m_last_coordinate;
                }
                else {
                    m_range += m_last_coordinate;
                }

                const double x = m_last_coordinate.m_x.asDouble();
                const double y = m_last_coordinate.m_y.asDouble();
                sum_z += m_last_coordinate.zDouble();

                if (check_x) {
                    if (!has_x1) {
                        m_x1 = x;
                        has_x1 = true;
                    }
                    else if (!has_x2) {
                        if (fabs(x - m_x1) > std::numeric_limits<float>::epsilon()) {
                            m_x2 = x;
                            m_step_x = m_x2 - m_x1;
                            has_x2 = true;
                            check_x = false;
                        }
                    }
                }

                if (check_y) {
                    if (!has_y1) {
                        m_y1 = y;
                        has_y1 = true;
                    }
                    else if (!has_y2) {
                        if (fabs(y - m_y1) > std::numeric_limits<float>::epsilon()) {
                            m_y2 = y;
                            m_step_y = m_y2 - m_y1;
                            has_y2 = true;
                            check_y = false;
                        }
                    }
                }

                m_line_count++;
            }

            m_xyz_min_x = m_range.minX().asDouble();
            m_xyz_min_y = m_range.minY().asDouble();

            if (m_line_count > 0) {
                m_mean_z = sum_z / m_line_count;
            }
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::StdCppException;
        }

        m_scan_done = true;
        m_last_err_code = result;

        return result;
    }


    /**
     *  @brief Create an image representing the heights of an XYZ file as grayscales.
     *
     *  The image could be saved in GeoTIFF file format.
     *
     *  @param file_path The path to the XYZ file, which is the source for the values.
     *  @param src_srid The source coordinate system.
     *  @param dst_srid The destination coordinate system.
     *  @return A pointer to the created image, or nullptr if the image couldn't be created.
     */
    Image* XYZFile::createImageFromXYZFile(const String& file_path, int32_t src_srid, int32_t dst_srid) noexcept {
        XYZFile* xyz_file1 = nullptr;
        XYZFile* xyz_file2 = nullptr;
        Image* image = nullptr;
        GeoProj* proj = nullptr;

        try {
            // Transformation
            if (src_srid != 0 && dst_srid != 0) {
                proj = new(std::nothrow) GeoProj();
                if (proj == nullptr) {
                    throw ErrorCode::ClassInstantiationFailed;
                }

                proj->setSrcSRID(src_srid);
                proj->setDstSRID(dst_srid);
                proj->_update();

                if (!proj->isValid()) {
                    throw ErrorCode::InvalidProjection;
                }
            }

            // First pass, scan
            xyz_file1 = new(std::nothrow) XYZFile(file_path);
            if (xyz_file1 == nullptr) {
                throw ErrorCode::ClassInstantiationFailed;
            }

            xyz_file1->startReadAscii();
            xyz_file1->scan();
            xyz_file1->close();

            RangeCubeFix range = xyz_file1->range();

            // Calculate image size
            int64_t image_width = static_cast<int64_t>(range.width().asInt64()) + 1;
            int64_t image_height = static_cast<int64_t>(range.height().asInt64()) + 1;
            if (image_width < 1) {
                throw Error::specific(kErrImageWidthLessThanOne);
            }
            if (image_height < 1) {
                throw Error::specific(kErrImageHeightLessThanOne);
            }
            if (image_width * image_height > 32000 * 32000) {
                throw Error::specific(kErrImageSizeToBig);
            }

            // Second pass, process
            xyz_file2 = new(std::nothrow) XYZFile(file_path);
            if (xyz_file2 == nullptr) {
                throw ErrorCode::ClassInstantiationFailed;
            }

            xyz_file2->startReadAscii();

            // Let the new instance know the allready scanned range.
            xyz_file2->m_range = xyz_file1->m_range;
            xyz_file2->m_step_x = xyz_file1->m_step_x;
            xyz_file2->m_step_y = xyz_file1->m_step_y;

            image = Image::createLuminaFloat(static_cast<int32_t>(image_width), static_cast<int32_t>(image_height));
            if (image == nullptr) {
                throw Error::specific(6);
            }

            // Mark all pixels as unused
            image->clear(-100000, 0, 0, 0);
            image->setSampleValueRange(range.m_min_z.asDouble(), range.m_max_z.asDouble());

            image->setGeoSrid(dst_srid);

            // Add tie points
            Vec2d tie_point_pos;
            if (proj != nullptr) {
                Vec2d min_pos, max_pos;
                proj->transform(Vec2d(range.m_min_x.asDouble(), range.m_min_y.asDouble()), min_pos);
                proj->transform(Vec2d(range.m_max_x.asDouble(), range.m_max_y.asDouble()), max_pos);

                // Top-left corner (raster: 0,0)
                image->addTiePoint(Vec3d(0, 0, 0), Vec3d(min_pos.m_x, max_pos.m_y, 0));

                // Top-right corner (raster: image_width, 0)
                image->addTiePoint(Vec3d(image_width, 0, 0), Vec3d(max_pos.m_x, max_pos.m_y, 0));

                // Bottom-left corner (raster: 0, image_height)
                image->addTiePoint(Vec3d(0, image_height, 0), Vec3d(min_pos.m_x, min_pos.m_y, 0));
            }

            float pixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            ImageAccess ia(image, pixel);

            // Read all lines from XYZ-file.
            String xyz_line;
            double xyz_min_x = xyz_file2->m_range.m_min_x.asDouble();
            double xyz_min_y = xyz_file2->m_range.m_min_y.asDouble();

            while (xyz_file2->readTrimmedLine(xyz_line)) {
                if (xyz_line.length() > 0) {
                    Vec3d xyz_coord;
                    if (!xyz_coord.setByCSV(xyz_line, ' ')) {
                        throw ErrorCode::UnexpectedData;
                    }

                    int32_t x = static_cast<int32_t>(round(xyz_coord.m_x - xyz_min_x));
                    int32_t y = image_height - 1 - static_cast<int32_t>(round(xyz_coord.m_y - xyz_min_y));

                    auto p = (float*)ia.ptrAt(x, y);
                    if (p != nullptr) {
                        *p = xyz_coord.z();
                    }
                }
            }

            xyz_file2->close();
        }
        catch (ErrorCode err) {
            delete image;
            image = nullptr;
        }

        delete xyz_file1;
        delete xyz_file2;
        delete proj;

        return image;
    }


    /**
     *  @brief Converts an XYZ file to a ValueGrid file.
     *
     *  This function reads a file in XYZ format and converts it to a ValueGrid format.
     *
     *  @param xyz_file_path Path to the input XYZ file.
     *  @param cvf2_file_path Path to the output ValueGrid file.
     *  @param srid Spatial Reference System ID (SRID) of the data.
     *  @param unit Unit of measurement for the data (e.g., meters, feet).
     *  @param z_decimals Number of decimal places to retain from the Z values in the XYZ file.
     *  @param min_digits Minimum precision level for ValueGrid compression.
     *  @param max_digits Maximum precision level for ValueGrid compression.
     *
     *  @return Returns an error code (`ErrorCode::None` on success, or an error code on failure).
     */
    ErrorCode XYZFile::xyzFileToCVF2File(
            const String& xyz_file_path,
            const String& cvf2_file_path,
            int32_t srid,
            LengthUnit unit,
            int32_t z_decimals,
            int32_t min_digits,
            int32_t max_digits
    ) noexcept {
        auto result = ErrorCode::None;
        XYZFile* xyz_file = nullptr;
        CVF2* cvf2 = nullptr;

        try {
            if (z_decimals < 1 || z_decimals > 9) {
                throw Error::specific(kErrPresisionOutOfRange);
            }

            xyz_file = new(std::nothrow) XYZFile(xyz_file_path);
            if (!xyz_file) {
                throw Error::specific(kErrXYZFileInstantiationFailed);
            }

            xyz_file->startReadAscii();
            xyz_file->scan();
            xyz_file->rewind();

            auto xyz_range = xyz_file->range();
            int64_t xyz_width = xyz_range.width().asInt64() + 1;
            int64_t xyz_height = xyz_range.height().asInt64() + 1;

            std::cout << "xyz_range: " << xyz_range << std::endl;
            std::cout << "xyz_width: " << xyz_width << std::endl;
            std::cout << "xyz_height: " << xyz_height << std::endl;

            if (xyz_width < 1 || xyz_height < 1 ||
                xyz_width > kMaxXYZWidth || xyz_height > kMaxXYZHeight) {
                throw ErrorCode::UnsupportedDimension;
            }

            cvf2 = new(std::nothrow) CVF2(static_cast<int32_t>(xyz_width), static_cast<int32_t>(xyz_height), unit, min_digits, max_digits);
            if (cvf2 == nullptr) {
                throw ErrorCode::ClassInstantiationFailed;
            }

            std::cout << "srid: " << srid << std::endl;

            cvf2->setSRID(srid);
            cvf2->setBbox(xyz_range);
            cvf2->openFileToWrite(cvf2_file_path);

            xyz_file->checkBeforeReading();

            int32_t xyz_line_count = 0;
            String xyz_line;
            Vec3Fix xyz_coord;
            double xyz_min_x = xyz_file->m_range.m_min_x.asDouble();
            double xyz_min_y = xyz_file->m_range.m_min_y.asDouble();

            while (xyz_file->readTrimmedLine(xyz_line)) {
                if (xyz_line.length() > 0) {
                    if (!xyz_coord.setByCSV(xyz_line, ' ')) {
                        throw ErrorCode::UnexpectedData;
                    }
                    int32_t x = static_cast<int32_t>(round(xyz_coord.m_x.asDouble() - xyz_min_x));
                    int32_t y = static_cast<int32_t>(round(xyz_coord.m_y.asDouble() - xyz_min_y));
                    cvf2->pushValueToData(x, y, xyz_coord.m_z.asInt64(z_decimals));
                    xyz_line_count++;
                }
            }

            std::cout << "xyz_line_count: " << xyz_line_count << std::endl;

            cvf2->encodeData();
            cvf2->finish();
        }
        catch (ErrorCode err) {
            result = err;
        }
        catch (...) {
            result = ErrorCode::Fatal;
        }

        // Cleanup
        if (xyz_file != nullptr) {
            xyz_file->close();
            delete xyz_file;
        }

        delete cvf2;

        return result;
    }


}  // End of namespace Grain
