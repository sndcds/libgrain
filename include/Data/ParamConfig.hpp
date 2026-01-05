//
//  ParamConfig.hpp
//
//  Created by Roald Christesen on from 11.03.2020
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//

#ifndef GrainParamConfig_hpp
#define GrainParamConfig_hpp

#include "GUI/Components/Component.hpp"


namespace Grain {

/**
 *  @brief ParamConfig.
 *
 *  ParamConfig manages data for individual parameters, representing precise values within a specific range.
 *  Each parameter is identified by an ID and can optionally have a name for identification.
 *  This class facilitates the management of parameters with defined ranges and specific steps.
 */
class ParamConfig {

public:
    int32_t id_ = -1;              ///< Unique parameter id, must be >= 0
    const char* name_ = nullptr;   ///< Unique name, used to identify a parameter, used as parameter name in files
    Fix default_ = 0;              ///< Default value
    Fix min_ = 0;                  ///< Minimum value
    Fix max_ = 1;                  ///< Maximum value
    Fix offset_ = 0;               ///< Center value, used for graphical representations, user inteface etc
    int32_t precision_ = 0;        ///< Number of fractional digits
    Fix step_ = 1;
    Fix big_step_ = 10;

public:
    bool isValidUid() noexcept { return id_ >= 0; }

    static ParamConfig* configByName(ParamConfig* table, const char* name) {
        for (int32_t i = 0; table[i].id_ >= 0; i++) {
            if (strcmp(table[i].name_, name) == 0) {
                return &table[i];
            }
        }
        return nullptr;
    }

    static ParamConfig* configByUid(ParamConfig* table, int32_t id) {
        for (int32_t i = 0; table[i].id_ >= 0; i++) {
            if (table[i].id_ == id) {
                return &table[i];
            }
        }
        return nullptr;
    }

    void writeLineToDataFile(File* file, const Fix& value) {
        if (file) {
            if (value != default_) {
                file->writeLineFix(name_, value);
            }
        }
    }

};


class ParamGroup {

protected:
    int32_t param_count_ = 0;
    ParamConfig* param_config_data_ = nullptr;
    Fix* param_values_ = nullptr;

public:
    ParamGroup(ParamConfig* data) noexcept {
        param_count_ = 0;
        param_config_data_ = data;
        if (data != nullptr) {
            while (data[param_count_].id_ >= 0) {
                param_count_++;
            }
            param_values_ = (Fix*)std::malloc(sizeof(Fix) * param_count_);
            resetParams();
        }
    }

    ~ParamGroup() noexcept {
        std::free(param_values_);
    }

    int32_t length() const noexcept { return param_count_; }

    ParamConfig* ParamConfigAtIndex(int32_t index) const noexcept {
        if (param_config_data_ != nullptr && index >= 0 && index < param_count_) {
            return &param_config_data_[index];
        }
        else {
            return nullptr;
        }
    }

    void resetParams() noexcept {
        if (param_values_) {
            for (int32_t i = 0; i < param_count_; i++) {
                param_values_[i] = param_config_data_[i].default_;
            }
        }
    }

    bool isParamAtIndexDefault(int32_t index) const noexcept {
        if (auto param = ParamConfigAtIndex(index)) {
            return param_values_[index] == param->default_;
        }

        // TODO: Coming here is an error. Save errors for debugging!

        return false;
    }


    int32_t countNonDefault() const noexcept {
        int32_t n = 0;
        for (int32_t i = 0; i < param_count_; i++) {
            if (param_values_[i] != param_config_data_[i].default_)
                n++;
        }
        return n;
    }


    bool setParamByName(const char* name, const char* value) noexcept {
        if (auto p = ParamConfig::configByName(param_config_data_, name)) {
            Fix new_value(value);
            if (new_value != param_values_[p->id_]) {
                param_values_[p->id_] = new_value;
                return true;
            }
        }
        return false;
    }

    void writeAllParams(File* file) {
        if (file && file->canWrite()) {
            for (int32_t i = 0; i < param_count_; i++) {
                if (param_values_[i] != param_config_data_[i].default_)
                    file->writeLineFix(param_config_data_[i].name_, param_values_[i]);
            }
        }
    }

    bool boolValue(int32_t index) const noexcept {
        return (param_values_ != nullptr && index >= 0 && index < param_count_) ? (bool)param_values_[index].asInt32() : false;
    }

    int32_t intValue(int32_t index) const noexcept {
        return (param_values_ != nullptr && index >= 0 && index < param_count_) ? param_values_[index].asInt32() : 0;
    }

    double realValue(int32_t index) const noexcept {
        return (param_values_ != nullptr && index >= 0 && index < param_count_) ? param_values_[index].asDouble() : 0;
    }

    Fix fixValue(int32_t index) const noexcept {
        return (param_values_ != nullptr && index >= 0 && index < param_count_) ? param_values_[index] : Fix(0);
    }

    bool setParamBool(int32_t index, bool value) noexcept {
        return setParamInt(index, (int32_t)value);
    }

    bool setParamInt(int32_t index, int32_t value) noexcept {
        if (param_values_ != nullptr && index >= 0 && index < param_count_) {
            ParamConfig* sud = &param_config_data_[index];
            int32_t min = sud->min_.asInt32();
            int32_t max = sud->max_.asInt32();
            if (param_values_[index].setInt32(value < min ? min : (value > max ? max : value))) {
                return true;
            }
        }
        return false;
    }

    bool setParamDouble(int32_t index, double value) noexcept {
        if (param_values_ != nullptr && index >= 0 && index < param_count_) {
            ParamConfig* sud = &param_config_data_[index];
            double min = sud->min_.asDouble();
            double max = sud->max_.asDouble();
            if (param_values_[index].setDouble(value < min ? min : (value > max ? max : value))) {
                return true;
            }
        }
        return false;
    }

    bool setParamFix(int32_t index, const Fix& value) noexcept {
        if (param_values_ != nullptr && index >= 0 && index < param_count_) {
            ParamConfig* sud = &param_config_data_[index];
            Fix min = sud->min_;
            Fix max = sud->max_;
            if (param_values_[index].set(value < min ? min : (value > max ? max : value))) {
                return true;
            }
        }
        return false;
    }


    static int32_t countNonDefault(ParamGroup* pg) noexcept {
        return pg ? pg->countNonDefault() : 0;
    }

    static void writeAllParams(ParamGroup* pg, File* file) {
        if (pg) { pg->writeAllParams(file); }
    }

    static bool boolValue(ParamGroup* pg, int32_t index) noexcept {
        return pg ? pg->boolValue(index) : false;
    }

    static int32_t intValue(ParamGroup* pg, int32_t index) noexcept {
        return pg ? pg->intValue(index) : 0;
    }

    static double realValue(ParamGroup* pg, int32_t index) noexcept {
        return pg ? pg->realValue(index) : 0;
    }

    static Fix fixValue(ParamGroup* pg, int32_t index) noexcept {
        return pg ? pg->fixValue(index) : 0;
    }


    static bool setParamBool(ParamGroup* pg, int32_t index, bool value) noexcept {
        return setParamInt(pg, index, (int32_t)value);
    }

    static bool setParamInt(ParamGroup* pg, int32_t index, int32_t value) noexcept {
        return pg ? pg->setParamInt(index, value) : false;
    }

    static bool setParamDouble(ParamGroup* pg, int32_t index, double value) noexcept {
        return pg ? pg->setParamDouble(index, value) : false;
    }

    static bool setParamFix(ParamGroup* pg, int32_t index, const Fix& value) noexcept {
        return pg ? pg->setParamFix(index, value) : false;
    }

    static bool setParamByName(ParamGroup* pg, const char* name, const char* value) noexcept {
        return pg ? pg->setParamByName(name, value) : false;
    }
};


} // End of namespace Grain

#endif // GrainParamConfig_hpp
