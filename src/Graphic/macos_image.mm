// Objective-C++ (.mm)
#import "Graphic/macos_image.h"
#import <CoreImage/CoreImage.h>
#import <CoreGraphics/CoreGraphics.h>

namespace GrainAppleGraphics {

void drawQuadrilateralImage(CGContextRef cgContext,
                            CGImageRef cgImage,
                            float alpha,
                            const CGPoint pts[4])
{
    @autoreleasepool {
        CIImage* ciImage = [CIImage imageWithCGImage:cgImage];

        CIFilter* filter = [CIFilter filterWithName:@"CIPerspectiveTransform"];
        [filter setValue:ciImage forKey:kCIInputImageKey];
        [filter setValue:[CIVector vectorWithCGPoint:pts[0]] forKey:@"inputTopLeft"];
        [filter setValue:[CIVector vectorWithCGPoint:pts[1]] forKey:@"inputTopRight"];
        [filter setValue:[CIVector vectorWithCGPoint:pts[2]] forKey:@"inputBottomRight"];
        [filter setValue:[CIVector vectorWithCGPoint:pts[3]] forKey:@"inputBottomLeft"];

        CIImage* transformed = filter.outputImage;

        CIFilter* alphaFilter = [CIFilter filterWithName:@"CIColorMatrix"];
        [alphaFilter setValue:transformed forKey:kCIInputImageKey];
        [alphaFilter setValue:[CIVector vectorWithX:0 Y:0 Z:0 W:alpha]
                       forKey:@"inputAVector"];

        CIImage* finalImage = alphaFilter.outputImage;

        CIContext* ctx =
            [CIContext contextWithCGContext:cgContext options:nil];

        [ctx drawImage:finalImage
                inRect:finalImage.extent
              fromRect:finalImage.extent];
    }
}

} // namespace GrainAppleGraphics