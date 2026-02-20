#pragma once

#if defined(__APPLE__) && defined(__MACH__)
#import <CoreGraphics/CoreGraphics.h>
#endif

namespace GrainAppleGraphics {

    void drawQuadrilateralImage(CGContextRef cgContext, CGImageRef cgImage,float alpha, const CGPoint points[4]);

} // End of namespace GrainAppleGraphics