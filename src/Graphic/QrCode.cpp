//
//  StrokeStyle.cpp
//
//  Created by Roald Christesen on 01.09.2026
//  Copyright (C) 2026 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#include "Graphic/QrCode.hpp"
#include "Graphic/GraphicContext.hpp"


namespace Grain {

    void QrCode::drawImage(
        GraphicContext* gc,
        QrCodeDrawOptions& options,
        const String& value,
        const Rectd& rect,
        const RGBA fg_color,
        const RGBA bg_color
    ) noexcept {

        if (!gc) {
            return;
        }

        const qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(
            value.utf8(),
            static_cast<qrcodegen::QrCode::Ecc>(options.error_correction));

        const int32_t module_count = qr.getSize();
        const int32_t quiet_zone = options.quiet_zone;

        const double module_size =
            std::min(
                rect.width_ / (module_count + 2 * quiet_zone),
                rect.height_ / (module_count + 2 * quiet_zone)
            );

        const double qr_width = module_size * (module_count + 2 * quiet_zone);
        const double offset_x = rect.x_ + (rect.width_ - qr_width) / 2.0;
        const double offset_y = rect.y_ + (rect.height_ - qr_width) / 2.0;

        gc->setFillRGBA(bg_color);
        gc->fillRect(rect);

        gc->setFillRGBA(fg_color);

        for (int32_t y = 0; y < module_count; ++y) {
            for (int32_t x = 0; x < module_count; ++x) {
                if (!qr.getModule(x, y)) {
                    continue;
                }

                Rectd module_rect;
                module_rect.x_ = offset_x + (x + quiet_zone) * module_size;
                module_rect.y_ = offset_y + (y + quiet_zone) * module_size;
                module_rect.width_ = module_size;
                module_rect.height_ = module_size;

                gc->fillRect(module_rect);
            }
        }
    }

} // End of namespace Grain
