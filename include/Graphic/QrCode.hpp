//
//  StrokeStyle.hpp
//
//  Created by Roald Christesen on from 01.09.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 01.09.2026
//

#ifndef GrainQrCode_hpp
#define GrainQrCode_hpp


#include "Extern/qrcodegen.hpp"
#include "Color/RGBA.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    struct QrCodeDrawOptions;

    class QrCode {
    public:
        enum class ErrorCorrection {
            Low = 0,
            Medium,
            Quartile,
            High
        };

        // QR version 1–40
        static const int32_t VERSION_MIN = 1;
        static const int32_t VERSION_MAX = 40;

        // Mask, -1 = automatic, 0–7 = explicit mask
        static const int32_t MASK_MIN = -1;
        static const int32_t MASK_MAX = 7;
        static const int32_t MASK_AUTO = -1;

        static void drawImage(
            GraphicContext* gc,
            QrCodeDrawOptions& options,
            const String& value,
            const Rectd& rect,
            const RGBA fg_color,
            const RGBA bg_color) noexcept;

    protected:
    };

    struct QrCodeDrawOptions {
        QrCode::ErrorCorrection error_correction =
            QrCode::ErrorCorrection::Medium;

        int32_t min_version = QrCode::VERSION_MIN;
        int32_t max_version = QrCode::VERSION_MAX;;
        int32_t mask = QrCode::MASK_AUTO;
        bool boost_error_correction = true;
        int32_t quiet_zone = 4;
    };

} // End of namespace Grain

#endif // GrainQrCode_hpp