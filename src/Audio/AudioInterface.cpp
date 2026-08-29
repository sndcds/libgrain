#include "Audio/AudioInterface.hpp"

namespace Grain {

    bool AudioInterface::update() {
        devices_.clear();

        // Platform-specific enumeration comes later
        return true;
    }

    int AudioInterface::deviceCount() const {
        return static_cast<int>(devices_.size());
    }

    const AudioDevice& AudioInterface::device(int index) const {
        return devices_[index];
    }

    const AudioDevice* AudioInterface::defaultInput() const {
        if (default_input_index_ < 0) return nullptr;
        return &devices_[default_input_index_];
    }

    const AudioDevice* AudioInterface::defaultOutput() const {
        if (default_output_index_ < 0) return nullptr;
        return &devices_[default_output_index_];
    }

}