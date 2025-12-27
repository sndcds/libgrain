#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "Grain.hpp"
#include "App/App.hpp"


namespace Grain {

    void _macosApp_start() {
        @autoreleasepool {
            [NSApplication sharedApplication];
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
            [NSApp activateIgnoringOtherApps:YES];
            [NSApp run];
        }
    }


    void _macosApp_addMenu() {
        @autoreleasepool {
            NSMenu* mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];
            [NSApp setMainMenu:mainMenu];

            // App Menu
            NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
            [mainMenu addItem:appMenuItem];

            NSMenu* appMenu = [[NSMenu alloc] initWithTitle:@"App"];
            [appMenuItem setSubmenu:appMenu];

            NSString* appName = [[NSProcessInfo processInfo] processName];
            NSString* quitTitle = [NSString stringWithFormat:@"Quit %@", appName];
            SEL quitSel = @selector(terminate:);

            NSMenuItem* quitItem = [[NSMenuItem alloc] initWithTitle:quitTitle
                                                              action:quitSel
                                                       keyEquivalent:@"q"];
            [appMenu addItem:quitItem];

            // File Menu
            NSMenuItem* fileMenuItem = [[NSMenuItem alloc] init];
            [mainMenu addItem:fileMenuItem];

            NSMenu* fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
            [fileMenuItem setSubmenu:fileMenu];

            NSMenuItem* newItem = [[NSMenuItem alloc] initWithTitle:@"New"
                                                             action:@selector(newDocument:)
                                                      keyEquivalent:@"n"];
            [fileMenu addItem:newItem];
        }
    }

    void _macosApp_beep() {
        NSBeep();
    }

    void _macosApp_updateScreenInfos(App* app) {

        NSArray* ns_screens = [NSScreen screens];

        auto screen_count = static_cast<int32_t>([ns_screens count]);
        if (screen_count > kMaxScreenCount) {
            screen_count = kMaxScreenCount;
        }

        if (screen_count > 0) {
            app->smallest_screen_index_ = -1;      // Undefined
            app->largest_screen_index_ = -1;       // Undefined
            app->total_screen_pixel_count_ = 0;

            int32_t min_pixel_count = std::numeric_limits<int32_t>::max();
            int32_t max_pixel_count = std::numeric_limits<int32_t>::min();
            int32_t curr_pixel_count = 0;

            for (int32_t i = 0; i < screen_count; i++) {
                auto screen = new (std::nothrow) Screen();
                app->screens_.push(screen);
                if (screen) {
                    NSScreen* ns_screen = [ns_screens objectAtIndex:(NSUInteger) i];
                    NSRect ns_screen_rect = [ns_screen frame];
                    NSRect ns_visible_rect = [ns_screen visibleFrame];

                    screen->width_ = static_cast<int32_t>(ns_screen_rect.size.width);
                    screen->height_ = static_cast<int32_t>(ns_screen_rect.size.height);
                    screen->visible_width_ = static_cast<int32_t>(ns_visible_rect.size.width);
                    screen->visible_height_ = static_cast<int32_t>(ns_visible_rect.size.height);
                    screen->ns_screen_ = (__bridge void*) ns_screen;

                    curr_pixel_count = screen->pixelCount();

                    if (curr_pixel_count > max_pixel_count) {
                        max_pixel_count = curr_pixel_count;
                        app->largest_screen_index_ = i;
                    }

                    if (curr_pixel_count < min_pixel_count) {
                        min_pixel_count = curr_pixel_count;
                        app->smallest_screen_index_ = i;
                    }
                    app->total_screen_pixel_count_ += curr_pixel_count;
                }
            }
        }
    }

    void _macosApp_copyToPasteboard(String* string, int64_t character_index, int64_t character_length) {
        int64_t byte_index, byte_length;
        if (string->byteRangeFromCharacterRange(character_index, character_length, byte_index, byte_length)) {
            NSString* ns_string = [[NSString alloc] initWithBytes:string->utf8AtIndex(character_index) length:byte_length encoding:NSUTF8StringEncoding];
            [[NSPasteboard generalPasteboard] clearContents];
            [[NSPasteboard generalPasteboard] setString:ns_string forType:NSPasteboardTypeString];
            [ns_string release];
        }
    }

    int32_t _macosApp_pasteFromPasteboard(String* string, int64_t character_index) {
        NSString* ns_string = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
        if (ns_string != nil) {
            if (string->insertAtCharacterIndex([ns_string UTF8String], character_index)) {
                return [ns_string length];
            }
        }
        return 0;
    }

}
