#import "GUI/macos_ui_helper.h"
#import <Foundation/Foundation.h>


void GrainUIHelper_runOnMainThread(GrainUIHelperCallback callback, void* context) {
    dispatch_async(dispatch_get_main_queue(), ^{
        callback(context);
    });
}