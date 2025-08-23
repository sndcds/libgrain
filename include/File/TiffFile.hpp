//
//  TiffFile.hpp
//
//  Created by Roald Christesen on 18.11.2023
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 03.08.2025
//

/**
 *  Limitations:
 *  Does not support compressed data.
 *  Does not support BigTiff.
 *  Uses one strip only for image data.
 *
 *  Features:
 *  Can write GeoTiff tags.
 *  Supports uint8_t, uint16_t, uint32_t and float samples.
 *
 *  Information on TIFF and BigTIFF:
 *  https://www.awaresystems.be/imaging/tiff.html
 *  https://www.awaresystems.be/imaging/tiff/bigtiff.html
 *
 *  Information on GeoTIFF:
 *  https://github.com/opengeospatial/geotiff
 *  http://geotiff.maptools.org/spec/geotiff6.html
 *  https://www.geospatialworld.net/article/geotiff-a-standard-image-file-format-for-gis-applications/
 *  https://www.ogc.org/standard/geotiff/
 *  https://geotiffjs.github.io
 *  https://www.cogeo.org/
 *  https://openlayers.org/en/latest/examples/?q=GeoTIFF
 *  https://www.ogc.org/standard/geotiff/
 *  https://github.com/opengeospatial
 *  https://docs.geotools.org/latest/javadocs/org/geotools/coverage/grid/io/imageio/geotiff/codes/GeoTiffGCSCodes.html
 *
 *  Tools for GeoTIFF:
 *  - GDAL terminal program: gdalinfo dgm1.tiff
 */

#ifndef GrainTiffFile_hpp
#define GrainTiffFile_hpp

#include "Grain.hpp"
#include "File/File.hpp"
#include "Type/Type.hpp"
#include "Type/KeyValue.hpp"
#include "Type/Data.hpp"
#include "Math/Vec3.hpp"


namespace Grain {

    class Image;
    class Log;

    enum class TiffTag : uint16_t {
        ImageWidth = 256,
        ImageHeight = 257,
        BitsPerSample = 258,
        Compression = 259,
        PhotometricInterpretation = 262,
        FillOrder = 266,
        StripOffsets = 273,
        SamplesPerPixel = 277,
        RowsPerStrip = 278,
        StripByteCounts = 279,
        MinSampleValue = 280,
        MaxSampleValue = 281,
        XResolution = 282,
        YResolution = 283,
        PlanarConfig = 284,
        ResolutionUnit = 296,
        SampleFormat = 339,
        SMinSampleValue = 340,
        SMaxSampleValue = 341,

        GeoModelPixelScale = 33550,
        GeoModelTiepoint = 33922,
        GeoModelTransformation = 34264,
        GeoDirectory = 34735,
        GeoDoubleParams = 34736,
        GeoAsciiParams = 34737,

        GDAL_NoData = 42113
    };

    enum class GeoTiffKey : uint16_t {
        Ignore = 0,
        GTModelTypeGeoKey = 1024,
        GTRasterTypeGeoKey = 1025,
        GTCitationGeoKey = 1026,
        GeographicTypeGeoKey = 2048,
        GeogCitationGeoKey = 2049,
        GeogGeodeticDatumGeoKey = 2050,
        GeogPrimeMeridianGeoKey = 2051,
        GeogLinearUnitsGeoKey = 2052,
        GeogLinearUnitSizeGeoKey = 2053,
        GeogAngularUnitsGeoKey = 2054,
        GeogAngularUnitSizeGeoKey = 2055,
        GeogEllipsoidGeoKey = 2056,
        GeogSemiMajorAxisGeoKey = 2057,
        GeogSemiMinorAxisGeoKey = 2058,
        GeogInvFlatteningGeoKey = 2059,
        GeogAzimuthUnitsGeoKey = 2060,
        GeogPrimeMeridianLongGeoKey = 2061,
        ProjectedCSTypeGeoKey = 3072,
        PCSCitationGeoKey = 3073,
        ProjectionGeoKey = 3074,
        ProjCoordTransGeoKey = 3075,
        ProjLinearUnitsGeoKey = 3076,
        ProjLinearUnitSizeGeoKey = 3077,
        ProjStdParallel1GeoKey = 3078,
        ProjStdParallel2GeoKey = 3079,
        ProjNatOriginLongGeoKey = 3080,
        ProjNatOriginLatGeoKey = 3081,
        ProjFalseEastingGeoKey = 3082,
        ProjFalseNorthingGeoKey = 3083,
        ProjFalseOriginLongGeoKey = 3084,
        ProjFalseOriginLatGeoKey = 3085,
        ProjFalseOriginEastingGeoKey = 3086,
        ProjFalseOriginNorthingGeoKey = 3087,
        ProjCenterLongGeoKey = 3088,
        ProjCenterLatGeoKey = 3089,
        ProjCenterEastingGeoKey = 3090,
        ProjCenterNorthingGeoKey = 3091,
        ProjScaleAtNatOriginGeoKey = 3092,
        ProjScaleAtCenterGeoKey = 3093,
        ProjAzimuthAngleGeoKey = 3094,
        ProjStraightVertPoleLongGeoKey = 3095,
        VerticalCSTypeGeoKey = 4096,
        VerticalCitationGeoKey = 4097,
        VerticalDatumGeoKey = 4098,
        VerticalUnitsGeoKey = 4099,
        CoordinateEpochGeoKey = 5120
    };

    enum class TiffType : uint16_t {
        Byte = 1,       ///< 8-bit unsigned integer
        Ascii = 2,      ///< 8-bit, NULL-terminated string
        Short = 3,      ///< 16-bit unsigned integer
        Long = 4,       ///< 32-bit unsigned integer
        Rational = 5,   ///< Two 32-bit unsigned integers
        SByte = 6,      ///< 8-bit signed integer
        Undefine = 7,   ///< 8-bit byte
        SShort = 8,     ///< 16-bit signed integer
        SLong = 9,      ///< 32-bit signed integer
        SRational = 10, ///< Two 32-bit signed integers
        Float = 11,     ///< 4-byte single-precision IEEE floating-point value
        Double = 12,    ///< 8-byte double-precision IEEE floating-point value

        Long8 = 16,     ///< BigTiff 8-byte unsigned integer
        SLong8 = 17,    ///< BigTiff 8-byte usigned integer
        IFD8 = 18       ///< BigTiff 8-byte unsigned IFD offset
    };

    struct TiffEntry {
        TiffTag m_tag;
        TiffType m_type;
        uint32_t m_count;
        uint32_t m_offset;
    };

    struct GeoTiffEntry {
        GeoTiffKey m_key;
        uint16_t m_location;
        uint16_t m_count;
        uint16_t m_offset;
    };

    struct TiffEntryPreparation {
        TiffEntry m_entry;
        int64_t m_pos_in_file;
        int64_t m_data_size;
        int64_t m_temp_file_pos;

        static bool tagComparator(const TiffEntryPreparation& ep1, const TiffEntryPreparation& ep2) {
            return ep1.m_entry.m_tag < ep2.m_entry.m_tag;
        }
    };

    struct GeoTiffEntryPreparation {
        GeoTiffEntry m_entry;
        int64_t m_pos_in_file;
        int64_t m_data_size;
        int64_t m_temp_file_pos;

        static bool tagComparator(const GeoTiffEntryPreparation& ep1, const GeoTiffEntryPreparation& ep2) {
            return ep1.m_entry.m_key < ep2.m_entry.m_key;
        }
    };

    struct GeoTiffTiePoint {
        Vec3d m_raster_pos;
        Vec3d m_model_pos;
    };


    class TiffFile : public File {
    public:
        enum {
            SampleFormat_Undefined = 0,
            SampleFormat_UInt = 1,
            SampleFormat_Int = 2,
            SampleFormat_IEEEFP = 3,
            SampleFormat_VOID = 4,
            SampleFormat_ComplexInt = 5,
            SampleFormat_ComplexIEEEFP = 6,

            Photometric_MinIsWhite = 0,
            Photometric_MinIsBlack = 1,
            Photometric_RGB = 2,
            Photometric_Palette = 3,
            Photometric_Mask = 4,
            Photometric_Separated = 5,
            Photometric_YCbCr = 6,
            Photometric_CIELAB = 8,
            Photometric_ICCLAB = 9,
            Photometric_ITULAB = 10,
            Photometric_LOGL = 32844,
            Photometric_LOGLUV = 32845,
            Photometric_CFA = 32803,
            Photometric_LinearRaw = 34892,
            Photometric_Depth = 51177,

            PlanarConfig_Contig = 1,
            PlanarConfig_Separate = 2,

            HeaderSize = 8,
            IFDEntryCountSize = 2,
            IFDEntrySize = 12,
            NextIFDPosSize = 4,
            GeoHeaderSize = 4 * 2,
            GeoEntrySize =  4 * 2,

            // GeoTIFF
            GeoModelTypeProjected = 1,       ///< Projection Coordinate System
            GeoModelTypeGeographic = 2,      ///< Geographic latitude-longitude System
            GeoModelTypeGeocentric = 3,      ///< Geocentric (X, Y, Z) Coordinate System
            GeoModelTypeUserDefined = 32767, ///< A user-defined model type (custom interpretation needed)

            GeoRasterPixelIsArea  = 1,
            GeoRasterPixelIsPoint = 2,

            // Angular Units
            GeoAngular_Radian = 9101,
            GeoAngular_Degree = 9102,
            GeoAngular_Arc_Minute = 9103,
            GeoAngular_Arc_Second = 9104,
            GeoAngular_Grad = 9105,
            GeoAngular_Gon = 9106,
            GeoAngular_DMS = 9107,
            GeoAngular_DMS_Hemisphere = 9108,
        };

    protected:
        bool m_drop_alpha = false;
        uint32_t m_ifd_offset = -1;      // Undefined

        String m_temp_data_file_path;
        File* m_temp_data_file = nullptr;
        int64_t m_data_size = 0;

        String m_temp_geo_data_file_path;
        File* m_temp_geo_data_file = nullptr;
        int64_t m_geo_data_size = 0;

        std::vector<TiffEntryPreparation> m_entry_preparations;
        std::vector<int64_t> m_strip_offsets;
        int64_t m_pixel_data_pos = -1;

        std::vector<GeoTiffEntryPreparation> m_geo_entry_preparations;
        std::vector<GeoTiffTiePoint> m_geo_tie_points;
        String m_geo_ascii_string;
        Vec3d m_geo_pixel_scale = Vec3d(1.0, 1.0, 0.0);
        int64_t m_geo_directory_pos = -1;
        int32_t m_geo_double_param_count = 0;
        int32_t m_geo_ascii_param_count = 0;
        uint16_t m_geo_key_directory_version = 1;
        uint16_t m_geo_key_revision = 1;
        uint16_t m_geo_minor_revision = 1;

        int32_t m_component_count = 0;
        int32_t m_used_component_count = 0;
        double m_min_sample_values[4]{};    ///< Minimum value in up to four channels
        double m_max_sample_values[4]{};    ///< Maximum value in up to four channels

    public:
        TiffFile(const String& file_path) noexcept;
        ~TiffFile() noexcept;

        [[nodiscard]] const char* className() const noexcept override { return "TiffFile"; }

        static const char* typeName(TiffType type) noexcept {
            static const KeyIntPair items[] = {
                    { "Byte", 1 },
                    { "Ascii", 2 },
                    { "Short", 3 },
                    { "Long", 4 },
                    { "Rational", 5 },
                    { "SByte", 6 },
                    { "Undefine", 7 },
                    { "SShort", 8 },
                    { "SLong", 9 },
                    { "SRational", 10 },
                    { "Float", 11 },
                    { "Double", 12 },
                    { "Long8", 16 },
                    { "SLong8", 17 },
                    { "IFD8", 18 },
                    { nullptr, -1 }, // Sentinel item (end of list)
            };
            static const char* unknown = "Unknown TIFF type";
            return KeyIntPair::lookupKey(static_cast<int32_t>(type), items, unknown);
        }

        static const char* tagName(TiffTag tag) noexcept {
            static const KeyIntPair items[] = {
                    { "NewSubfileType", 254 },
                    { "SubfileType", 255 },
                    { "ImageWidth", 256 },
                    { "ImageLength", 257 },
                    { "BitsPerSample", 258 },
                    { "Compression", 259 },
                    { "PhotometricInterpretation", 262 },
                    { "Threshholding", 263 },
                    { "CellWidth", 264 },
                    { "CellLength", 265 },
                    { "FillOrder", 266 },
                    { "DocumentName", 269 },
                    { "ImageDescription", 270 },
                    { "Make", 271 },
                    { "Model", 272 },
                    { "StripOffsets", 273 },
                    { "Orientation", 274 },
                    { "SamplesPerPixel", 277 },
                    { "RowsPerStrip", 278 },
                    { "StripByteCounts", 279 },
                    { "MinSampleValue", 280 },
                    { "MaxSampleValue", 281 },
                    { "XResolution", 282 },
                    { "YResolution", 283 },
                    { "PlanarConfiguration", 284 },
                    { "PageName", 285 },
                    { "XPosition", 286 },
                    { "YPosition", 287 },
                    { "FreeOffsets", 288 },
                    { "FreeByteCounts", 289 },
                    { "GrayResponseUnit", 290 },
                    { "GrayResponseCurve", 291 },
                    { "T4Options", 292 },
                    { "T6Options", 293 },
                    { "ResolutionUnit", 296 },
                    { "PageNumber", 297 },
                    { "TransferFunction", 301 },
                    { "Software", 305 },
                    { "DateTime", 306 },
                    { "Artist", 315 },
                    { "HostComputer", 316 },
                    { "Predictor", 317 },
                    { "WhitePoint", 318 },
                    { "PrimaryChromaticities", 319 },
                    { "ColorMap", 320 },
                    { "HalftoneHints", 321 },
                    { "TileWidth", 322 },
                    { "TileLength", 323 },
                    { "TileOffsets", 324 },
                    { "TileByteCounts", 325 },
                    { "InkSet", 332 },
                    { "InkNames", 333 },
                    { "NumberOfInks", 334 },
                    { "DotRange", 336 },
                    { "TargetPrinter", 337 },
                    { "ExtraSamples", 338 },
                    { "SampleFormat", 339 },
                    { "SMinSampleValue", 340 },
                    { "SMaxSampleValue", 341 },
                    { "TransferRange", 342 },
                    { "JPEGTables", 347 },
                    { "JPEGProc", 512 },
                    { "JPEGInterchangeFormat", 513 },
                    { "JPEGInterchangeFormatLngth", 514 },
                    { "JPEGRestartInterval", 515 },
                    { "JPEGLosslessPredictors", 517 },
                    { "JPEGPointTransforms", 518 },
                    { "JPEGQTables", 519 },
                    { "JPEGDCTables", 520 },
                    { "JPEGACTables", 521 },
                    { "YCbCrCoefficients", 529 },
                    { "YCbCrSubSampling", 530 },
                    { "YCbCrPositioning", 531 },
                    { "ReferenceBlackWhite", 532 },
                    { "Copyright", 33432 },
                    { "PhotoshopResources", 34377 },
                    { "ExifIFDPointer", 34665 },
                    { "GPSInfoIFDPointer", 34675 },
                    { "GeoModelPixelScale", 33550 },
                    { "GeoModelTiepoint", 33922 },
                    { "GeoModelTransformation", 34264 },
                    { "GeoDirectory", 34735 },
                    { "GeoDoubleParams", 34736 },
                    { "GeoAsciiParams", 34737 },
                    { "GDAL_NoData", 42113 },
                    { nullptr, 0 } // Sentinel item (end of list)
            };
            static const char* unknown = "Unknown TIFF tag";
            return KeyIntPair::lookupKey(static_cast<int32_t>(tag), items, unknown);
        }

        static const char* geoKeyName(uint16_t key) noexcept {
            static const KeyIntPair items[] = {
                    { "GTModelTypeGeoKey", 1024 },
                    { "GTRasterTypeGeoKey", 1025 },
                    { "GTCitationGeoKey", 1026 },
                    { "GeographicTypeGeoKey", 2048 },
                    { "GeogCitationGeoKey", 2049 },
                    { "GeogGeodeticDatumGeoKey", 2050 },
                    { "GeogPrimeMeridianGeoKey", 2051 },
                    { "GeogLinearUnitsGeoKey", 2052 },
                    { "GeogLinearUnitSizeGeoKey", 2053 },
                    { "GeogAngularUnitsGeoKey", 2054 },
                    { "GeogAngularUnitSizeGeoKey", 2055 },
                    { "GeogEllipsoidGeoKey", 2056 },
                    { "GeogSemiMajorAxisGeoKey", 2057 },
                    { "GeogSemiMinorAxisGeoKey", 2058 },
                    { "GeogInvFlatteningGeoKey", 2059 },
                    { "GeogAzimuthUnitsGeoKey", 2060 },
                    { "GeogPrimeMeridianLongGeoKey", 2061 },
                    { "ProjectedCSTypeGeoKey", 3072 },
                    { "PCSCitationGeoKey", 3073 },
                    { "ProjectionGeoKey", 3074 },
                    { "ProjCoordTransGeoKey", 3075 },
                    { "ProjLinearUnitsGeoKey", 3076 },
                    { "ProjLinearUnitSizeGeoKey", 3077 },
                    { "ProjStdParallel1GeoKey", 3078 },
                    { "ProjStdParallel2GeoKey", 3079 },
                    { "ProjNatOriginLongGeoKey", 3080 },
                    { "ProjNatOriginLatGeoKey", 3081 },
                    { "ProjFalseEastingGeoKey", 3082 },
                    { "ProjFalseNorthingGeoKey", 3083 },
                    { "ProjFalseOriginLongGeoKey", 3084 },
                    { "ProjFalseOriginLatGeoKey", 3085 },
                    { "ProjFalseOriginEastingGeoKey", 3086 },
                    { "ProjFalseOriginNorthingGeoKey", 3087 },
                    { "ProjCenterLongGeoKey", 3088 },
                    { "ProjCenterLatGeoKey", 3089 },
                    { "ProjCenterEastingGeoKey", 3090 },
                    { "ProjCenterNorthingGeoKey", 3091 },
                    { "ProjScaleAtNatOriginGeoKey", 3092 },
                    { "ProjScaleAtCenterGeoKey", 3093 },
                    { "ProjAzimuthAngleGeoKey", 3094 },
                    { "ProjStraightVertPoleLongGeoKey", 3095 },
                    { "VerticalCSTypeGeoKey", 4096 },
                    { "VerticalCitationGeoKey", 4097 },
                    { "VerticalDatumGeoKey", 4098 },
                    { "VerticalUnitsGeoKey", 4099 },
                    { "CoordinateEpochGeoKey", 5120 },
                    { nullptr, 0 } // Sentinel item (end of list)
            };
            static const char* unknown = "Unknown GeoTIFF key";
            return KeyIntPair::lookupKey(static_cast<int32_t>(key), items, unknown);
        }

        static int32_t geoKeyBytes(GeoTiffKey key) noexcept {
            struct KeyIntPair { GeoTiffKey key; int32_t value; };
            static const KeyIntPair table[] = {
                    { GeoTiffKey::GTModelTypeGeoKey, 2 },
                    { GeoTiffKey::GTRasterTypeGeoKey, 2 },
                    { GeoTiffKey::GTCitationGeoKey, 0 },
                    { GeoTiffKey::GeographicTypeGeoKey, 2 },
                    { GeoTiffKey::GeogCitationGeoKey, 2 },
                    { GeoTiffKey::GeogGeodeticDatumGeoKey, 2 },
                    { GeoTiffKey::GeogPrimeMeridianGeoKey, 2 },
                    { GeoTiffKey::GeogLinearUnitsGeoKey, 2 },
                    { GeoTiffKey::GeogLinearUnitSizeGeoKey, 0 },
                    { GeoTiffKey::GeogAngularUnitsGeoKey, 2 },
                    { GeoTiffKey::GeogAngularUnitSizeGeoKey, 0 },
                    { GeoTiffKey::GeogEllipsoidGeoKey, 2 },
                    { GeoTiffKey::GeogSemiMajorAxisGeoKey, 0 },
                    { GeoTiffKey::GeogSemiMinorAxisGeoKey, 0 },
                    { GeoTiffKey::GeogInvFlatteningGeoKey, 0 },
                    { GeoTiffKey::GeogAzimuthUnitsGeoKey, 2 },
                    { GeoTiffKey::GeogPrimeMeridianLongGeoKey, 0 },
                    { GeoTiffKey::ProjectedCSTypeGeoKey, 2 },
                    { GeoTiffKey::PCSCitationGeoKey, 0 },
                    { GeoTiffKey::ProjectionGeoKey, 2 },
                    { GeoTiffKey::ProjCoordTransGeoKey, 2 },
                    { GeoTiffKey::ProjLinearUnitsGeoKey, 2 },
                    { GeoTiffKey::ProjLinearUnitSizeGeoKey, 2 },      // TODO: Check!
                    { GeoTiffKey::ProjStdParallel1GeoKey, 2 },        // TODO: Check!
                    { GeoTiffKey::ProjStdParallel2GeoKey, 2 },        // TODO: Check!
                    { GeoTiffKey::ProjNatOriginLongGeoKey, 2 },       // TODO: Check!
                    { GeoTiffKey::ProjNatOriginLatGeoKey, 2 },        // TODO: Check!
                    { GeoTiffKey::ProjFalseEastingGeoKey, 2 },        // TODO: Check!
                    { GeoTiffKey::ProjFalseNorthingGeoKey, 2 },       // TODO: Check!
                    { GeoTiffKey::ProjFalseOriginLongGeoKey, 2 },     // TODO: Check!
                    { GeoTiffKey::ProjFalseOriginLatGeoKey, 2 },      // TODO: Check!
                    { GeoTiffKey::ProjFalseOriginEastingGeoKey, 2 },  // TODO: Check!
                    { GeoTiffKey::ProjFalseOriginNorthingGeoKey, 2 }, // TODO: Check!
                    { GeoTiffKey::ProjCenterLongGeoKey, 2 },          // TODO: Check!
                    { GeoTiffKey::ProjCenterLatGeoKey, 2 },           // TODO: Check!
                    { GeoTiffKey::ProjCenterEastingGeoKey, 2 },       // TODO: Check!
                    { GeoTiffKey::ProjCenterNorthingGeoKey, 2 },      // TODO: Check!
                    { GeoTiffKey::ProjScaleAtNatOriginGeoKey, 2 },    // TODO: Check!
                    { GeoTiffKey::ProjScaleAtCenterGeoKey, 2 },       // TODO: Check!
                    { GeoTiffKey::ProjAzimuthAngleGeoKey, 2 },        // TODO: Check!
                    { GeoTiffKey::ProjStraightVertPoleLongGeoKey, 2 },// TODO: Check!
                    { GeoTiffKey::VerticalCSTypeGeoKey, 2 },          // TODO: Check!
                    { GeoTiffKey::VerticalCitationGeoKey, 2 },        // TODO: Check!
                    { GeoTiffKey::VerticalDatumGeoKey, 2 },           // TODO: Check!
                    { GeoTiffKey::VerticalUnitsGeoKey, 2 },           // TODO: Check!
                    { GeoTiffKey::CoordinateEpochGeoKey, 2 },         // TODO: Check!
                    { (GeoTiffKey)0, -1 }
            };

            int32_t index = 0;
            while (static_cast<int32_t>(table[index].key) > 0) {
                if (key == table[index].key) {
                    break;
                }
                index++;
            }
            return table[index].value;
        }

        bool dropAlpha() const noexcept { return m_drop_alpha; }
        void setDropAlpha(bool drop_alpha) noexcept { m_drop_alpha = drop_alpha; }

        [[nodiscard]] static int32_t bytesForType(TiffType type) noexcept;

        ErrorCode writeImage(const Image* image, DataType data_type = DataType::Undefined) noexcept;

        void writeTag(TiffTag tag);
        void writeEntry(TiffEntryPreparation& entry_preparation);
        void writeGeoEntry(GeoTiffEntry& entry);

        void prepareEntry(TiffTag tag, TiffType type, uint32_t count, uint32_t value, int64_t temp_file_pos = -1);
        void prepareGeoEntry(GeoTiffKey key, uint16_t location, uint16_t count, uint16_t offset, int64_t temp_file_pos = -1);
        void sortPreparedEntries();
        void sortPreparedGeoEntries();
        void writePreparedEntries();
        void writePreparedGeoEntries();
        void updateEntryData();
        void writeImageData(const Image* image, DataType data_type);
        void writeEntryData(TiffTag tag, const void* data, int32_t length);
        void writeTiffDoubles(int32_t n, const double* data);

        void addGeoTiePoint(const Vec3d& raster_pos, const Vec3d& model_pos);
        void addGeoAscii(const char* str);
    };


    class TiffFileValidator : File {
    protected:
        int32_t m_ifd_count = 0;
        int64_t m_geo_directory_pos = 0;

    public:
        TiffFileValidator(const String& file_path) noexcept;
        ~TiffFileValidator() noexcept;

        void validate(Log& log) noexcept;
        int64_t validateIFD(int64_t file_pos, Log& log);
        int64_t validateGeo(int64_t file_pos, Log& log);
    };


} // End of namespace Grain

#endif // GrainTiffFile_hpp
