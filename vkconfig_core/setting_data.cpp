/*
 * Copyright (c) 2020 Valve Corporation
 * Copyright (c) 2020 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authors:
 * - Christophe Riccio <christophe@lunarg.com>
 */

#include "setting_data.h"

SettingData::SettingData(const std::string& key, const SettingType type) : key(key), type(type) { assert(!this->key.empty()); }

bool SettingData::Equal(const SettingData& other) const {
    if (this->key != other.key)
        return false;
    else if (this->type != other.type)
        return false;
    return true;
}

bool SettingDataBool::Equal(const SettingData& other) const {
    if (!SettingData::Equal(other)) return false;

    return this->value == static_cast<const SettingDataBool&>(other).value;
}

bool SettingDataInt::Equal(const SettingData& other) const {
    if (!SettingData::Equal(other)) return false;

    return this->value == static_cast<const SettingDataInt&>(other).value;
}

bool SettingDataString::Equal(const SettingData& other) const {
    if (!SettingData::Equal(other)) return false;

    return this->value == static_cast<const SettingDataString&>(other).value;
}

bool SettingDataIntRange::Equal(const SettingData& other) const {
    if (!SettingData::Equal(other)) return false;

    const SettingDataIntRange& data = static_cast<const SettingDataIntRange&>(other);

    return this->min_value == data.min_value && this->max_value == data.max_value;
}

bool SettingDataVector::Equal(const SettingData& other) const {
    if (!SettingData::Equal(other)) return false;

    const SettingDataVector& data = static_cast<const SettingDataVector&>(other);

    if (this->value.size() != data.value.size()) return false;

    for (std::size_t i = 0, n = this->value.size(); i < n; ++i) {
        if (std::find(data.value.begin(), data.value.end(), this->value[i].c_str()) == data.value.end()) return false;
    }

    return true;
}

std::shared_ptr<SettingData> CreateSettingData(const std::string& key, SettingType type) {
    assert(!key.empty());

    switch (type) {
        case SETTING_STRING:
            return std::shared_ptr<SettingData>(new SettingDataString(key));
        case SETTING_INT:
            return std::shared_ptr<SettingData>(new SettingDataInt(key));
        case SETTING_SAVE_FILE:
            return std::shared_ptr<SettingData>(new SettingDataFileSave(key));
        case SETTING_LOAD_FILE:
            return std::shared_ptr<SettingData>(new SettingDataFileLoad(key));
        case SETTING_SAVE_FOLDER:
            return std::shared_ptr<SettingData>(new SettingDataFolderSave(key));
        case SETTING_BOOL:
            return std::shared_ptr<SettingData>(new SettingDataBool(key));
        case SETTING_BOOL_NUMERIC_DEPRECATED:
            return std::shared_ptr<SettingData>(new SettingDataBoolNumeric(key));
        case SETTING_ENUM:
            return std::shared_ptr<SettingData>(new SettingDataEnum(key));
        case SETTING_FLAGS:
            return std::shared_ptr<SettingData>(new SettingDataFlags(key));
        case SETTING_INT_RANGE:
            return std::shared_ptr<SettingData>(new SettingDataIntRange(key));
        case SETTING_VUID_FILTER:
            return std::shared_ptr<SettingData>(new SettingDataVUIDFilter(key));
        default:
            assert(0);
            return std::shared_ptr<SettingData>();
    }
}

SettingData& SettingDataSet::Create(const std::string& key, SettingType type) {
    assert(!key.empty());
    assert(type >= SETTING_FIRST && type <= SETTING_LAST);

    // Find existing setting, avoid duplicated setting
    {
        SettingData* setting_data = this->Get(key.c_str());
        if (setting_data != nullptr) {
            return *setting_data;
        }
    }

    // Create a new setting
    {
        this->data.push_back(CreateSettingData(key, type));
        SettingData* setting_data = this->Get(key.c_str());
        assert(setting_data != nullptr);
        return *setting_data;
    }
}

SettingData* SettingDataSet::Get(const char* key) {
    for (std::size_t i = 0, n = this->data.size(); i < n; ++i) {
        if (this->data[i]->key == key) return this->data[i].get();
    }

    return nullptr;
}

const SettingData* SettingDataSet::Get(const char* key) const {
    for (std::size_t i = 0, n = this->data.size(); i < n; ++i) {
        if (this->data[i]->key == key) return this->data[i].get();
    }

    return nullptr;
}
