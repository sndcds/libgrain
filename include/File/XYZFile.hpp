//
//  XYZFile.hpp
//
//  Created by Roald Christesen on 17.11.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.08.2025
//

#ifndef GrainXYZFile_hpp
#define GrainXYZFile_hpp

#include "Grain.hpp"
#include "File/File.hpp"
#include "Math/Vec3.hpp"
#include "3d/RangeCube.hpp"
#include "Core/Log.hpp"


namespace Grain {

    class XYZFile;
    class Image;
    class ImageAccess;


    typedef bool (*XYZFileLineAction)(XYZFile* file, void* ref);


    class XYZFile : public File {

    public:
        enum {
            kMaxLineLength = 1000,      ///< Max length of buffer for reading a single text line of the file
            kMaxXYZWidth = 100000,
            kMaxXYZHeight = 100000
        };

        enum {
            kErrPresisionOutOfRange = 0,
            kErrXYZFileInstantiationFailed,
            kErrCVF2DirectoryNotFound,
            kErrNoFilesToConvert,
            kErrImageWidthLessThanOne,
            kErrImageHeightLessThanOne,
            kErrImageSizeToBig
        };

    protected:
        String m_delimiter{ " " };      ///< Delimiter to use. Default is a space character
        int32_t m_line_count = 0;       ///< Number of lines in file
        String m_line;                  ///< Buffer for line reading
        Vec3Fix m_last_coordinate;      ///< Buffer for coordinate reading
        RangeCubeFix m_range;           ///< Range of XYZ values in file, present after call to `scan()`
        double m_xyz_min_x;
        double m_xyz_min_y;
        double m_x1, m_x2, m_y1, m_y2;  ///< First two values in x and y direction
        double m_step_x = -1.0;         ///< Horizontal step size of first two values in x direction
        double m_step_y = -1.0;         ///< Vertical step size of first two values in y direction
        double m_mean_z = 0.0;          ///< Mean z value
        bool scan_done_ = false;        ///< Signas, that scan is allready done
        ErrorCode m_last_err_code = ErrorCode::None; ///< Code of the last error occured

    public:
        XYZFile(const String& file_path) noexcept;
        ~XYZFile() noexcept;

        const char* className() const noexcept override { return "XYZFile"; }

        void log(Log& l) const {
            l << "delimiter: " << m_delimiter << Log::endl;
            l << "line_count: " << m_line_count << Log::endl;
            l << "range: " << m_range << Log::endl;
            l << "xyz_min_x: " << m_xyz_min_x << Log::endl;
            l << "xyz_min_y: " << m_xyz_min_y << Log::endl;
            l << "mean_z: " << m_mean_z << Log::endl;
            l << "x1: " << m_x1 << Log::endl;
            l << "x2: " << m_x2 << Log::endl;
            l << "y1: " << m_y1 << Log::endl;
            l << "y2: " << m_y2 << Log::endl;
            l << "step_x: " << m_step_x << Log::endl;
            l << "step_y: " << m_step_y << Log::endl;
            l << "scan_done: " << scan_done_ << Log::endl;
            l << "last_err_code: " << (int)m_last_err_code << Log::endl;
        }


        ErrorCode scan() noexcept;

        /**
         *  @brief Number of lines in file.
         */
        int32_t lineCount() const noexcept { return m_line_count; }

        /**
         *  @brief Range of xand y in file.
         */
        RangeCubeFix range() const noexcept { return m_range; }

        /**
         *  @brief Mean of all Z values in file.
         */
        double meanZ() const noexcept { return m_mean_z; }

        /**
         *  @brief Code of the last error occured.
         */
        ErrorCode lastErr() const noexcept { return m_last_err_code; }

        static Image* createImageFromXYZFile(const String& file_path, int32_t src_srid, int32_t dst_srid) noexcept;

        static ErrorCode xyzFileToCVF2File(const String& xyz_file_path, const String& cvf2_file_path, int32_t cvf2_srid, LengthUnit cvf2_unit, int32_t z_decimals, int32_t min_digits, int32_t max_digits) noexcept;
    };


} // End of namespace Grain

#endif // GrainXYZFile_hpp
