//
// Created by 任兀华 on 2025/8/12.
//

#ifndef GATESERVER_CONFIGMANAGER_H
#define GATESERVER_CONFIGMANAGER_H

#include "const.h"

struct SectionInfo {
    SectionInfo() = default;
    ~SectionInfo(){
        section_data_.clear();
    }

    SectionInfo(const SectionInfo& src) {
        section_data_ = src.section_data_;
    }

    SectionInfo& operator = (const SectionInfo& src) {
        if (&src == this) {
            return *this;
        }

        this->section_data_ = src.section_data_;
        return *this;
    }

    std::map<std::string, std::string> section_data_;
    std::string  operator[](const std::string  &key) {
        if (section_data_.find(key) == section_data_.end()) {
            return "";
        }
        // 这里可以添加一些边界检查
        return section_data_[key];
    }
};

class ConfigManager
{
public:
    ~ConfigManager() {
        config_map_.clear();
    }

    // 单例
    static ConfigManager& GetInstance() {
        static ConfigManager instance;
        return instance;
    }

    // 索引运算符
    SectionInfo operator[](const std::string& section) {
        if (config_map_.find(section) == config_map_.end()) {
            return SectionInfo();
        }
        return config_map_[section];
    }

    // 拷贝构造函数
    ConfigManager(const ConfigManager& src) {
        this->config_map_ = src.config_map_;
    }

    // 赋值运算符
    ConfigManager& operator=(const ConfigManager& src) {
        if (&src == this) {
            return *this;
        }

        this->config_map_ = src.config_map_;
        return *this;
    };

private:
    ConfigManager();
    // 存储section和key-value对的map  
    std::map<std::string, SectionInfo> config_map_;
};


#endif //GATESERVER_CONFIGMANAGER_H