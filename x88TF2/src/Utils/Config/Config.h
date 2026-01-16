#pragma once
#include <nlohmann/json.hpp>
#include <vector>
#include <typeinfo>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <string>

namespace Config
{
    struct ConfigVarInitializer
    {
        const char* m_name{ nullptr };
        void* m_ptr{ nullptr };
        size_t m_type_hash{ 0 };
        bool m_no_save{ false };
    };
    inline std::vector<ConfigVarInitializer> vars{};

    static std::string Obfuscate(const std::string& data) {
        std::string obfuscated = data;
        const char key = 0x5A; // Simple XOR key
        for (char& c : obfuscated) {
            c ^= key;
        }
        return obfuscated;
    }

    static std::string Deobfuscate(const std::string& data) {
        return Obfuscate(data); // XOR is symmetric
    }

    static void Save(const std::filesystem::path& path)
    {
        std::ofstream output_file(path, std::ios::binary);
        if (!output_file.is_open())
        {
            return;
        }
        nlohmann::json j{};
        for (const auto& var : vars)
        {
            if (var.m_no_save)
            {
                continue;
            }
            if (var.m_type_hash == typeid(bool).hash_code())
            {
                j[var.m_name] = *static_cast<bool*>(var.m_ptr);
            }
            if (var.m_type_hash == typeid(int).hash_code())
            {
                j[var.m_name] = *static_cast<int*>(var.m_ptr);
            }
            if (var.m_type_hash == typeid(float).hash_code())
            {
                j[var.m_name] = *static_cast<float*>(var.m_ptr);
            }
            if (var.m_type_hash == typeid(Color_t).hash_code())
            {
                auto clr{ *static_cast<Color_t*>(var.m_ptr) };
                j[var.m_name] = { clr.r, clr.g, clr.b, clr.a };
            }
            if (var.m_type_hash == typeid(std::string).hash_code())
            {
                j[var.m_name] = *static_cast<std::string*>(var.m_ptr);
            }
        }
        std::string json_str = j.dump(4);
        std::string obfuscated = Obfuscate(json_str);
        output_file << obfuscated;
        output_file.close();
    }
    static void Load(const std::filesystem::path& path)
    {
        std::ifstream input_file(path, std::ios::binary);
        if (!input_file.is_open())
        {
            return;
        }
        std::string obfuscated((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
        std::string json_str = Deobfuscate(obfuscated);
        nlohmann::json j = nlohmann::json::parse(json_str);
        for (const auto& var : vars)
        {
            if (var.m_no_save)
            {
                continue;
            }
            if (j.find(var.m_name) == j.end())
            {
                continue;
            }
            if (var.m_type_hash == typeid(bool).hash_code())
            {
                *static_cast<bool*>(var.m_ptr) = j[var.m_name];
            }
            if (var.m_type_hash == typeid(int).hash_code())
            {
                *static_cast<int*>(var.m_ptr) = j[var.m_name];
            }
            if (var.m_type_hash == typeid(float).hash_code())
            {
                *static_cast<float*>(var.m_ptr) = j[var.m_name];
            }
            if (var.m_type_hash == typeid(Color_t).hash_code())
            {
                Color_t clr{ j[var.m_name][0], j[var.m_name][1], j[var.m_name][2], j[var.m_name][3] };
                *static_cast<Color_t*>(var.m_ptr) = clr;
            }
            if (var.m_type_hash == typeid(std::string).hash_code())
            {
                *static_cast<std::string*>(var.m_ptr) = j[var.m_name];
            }
        }
        input_file.close();
    }

    static std::vector<std::string> RefreshConfigFiles() {
        std::vector<std::string> files;
        std::string dir = "configs";
        if (!std::filesystem::exists(dir)) std::filesystem::create_directory(dir);
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".cfg") {
                files.push_back(entry.path().stem().string());
            }
        }
        if (files.empty()) files.push_back("default");
        return files;
    }

    static void CreateConfig(const std::string& name) {
        if (name.empty()) return;
        std::filesystem::path path = "configs/" + name + ".cfg";
        Save(path);
    }

    static void SaveConfig(const std::string& name) {
        std::filesystem::path path = "configs/" + name + ".cfg";
        Save(path);
    }

    static void LoadConfig(const std::string& name) {
        std::filesystem::path path = "configs/" + name + ".cfg";
        if (std::filesystem::exists(path)) {
            Load(path);
        }
    }

    static void DeleteConfig(const std::string& name) {
        std::filesystem::path path = "configs/" + name + ".cfg";
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
    }
}
#define CFGVAR(var, val) inline auto var{ val }; \
namespace configvar_initializers\
{\
inline auto var##_initializer = []()\
{\
Config::vars.push_back(Config::ConfigVarInitializer{#var, &var, typeid(var).hash_code(), false });\
return true;\
}();\
}
#define CFGVAR_NOSAVE(var, val) inline auto var{ val }; \
namespace configvar_initializers\
{\
inline auto var##_initializer = []()\
{\
Config::vars.push_back(Config::ConfigVarInitializer{#var, &var, typeid(var).hash_code(), true });\
return true;\
}();\
}