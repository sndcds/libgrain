#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    // Main-thread dispatch: callback gets a void* context
    typedef void (*GrainUIHelperCallback)(void* context);
    void GrainUIHelper_runOnMainThread(GrainUIHelperCallback callback, void* context);

#ifdef __cplusplus
}
#endif