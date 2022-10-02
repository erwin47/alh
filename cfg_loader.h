#ifndef SIMPLE_CONFIG_HEADER
#define SIMPLE_CONFIG_HEADER

#include <vector>
#include <map>
#include <string>

#ifdef _WIN32
#include <string.h> 
#else
#include <strings.h>
#endif

#include <algorithm>
#include "compat.h"

namespace config {
    template<typename T>
    T string_to(const std::string& str);

    template<typename T>
    std::string to_string(const T& t);

    template<> inline int string_to<int>(const std::string& str) { return stoi(str); }
    template<> inline long string_to<long>(const std::string& str) { return stol(str); }
    template<> inline long long string_to<long long>(const std::string& str) { return stoll(str); }
    template<> inline float string_to<float>(const std::string& str) { return stof(str); }
    template<> inline double string_to<double>(const std::string& str) { return stod(str); }
    template<> inline long double string_to<long double>(const std::string& str) { return stold(str); }
    template<> inline std::string string_to<std::string>(const std::string& str) { return str; }
    template<> inline const char* string_to<const char*>(const std::string& str) { return str.c_str(); }

    template<> inline std::string to_string<int>(const int& val) { return std::to_string(val); }
    template<> inline std::string to_string<long>(const long& val) { return std::to_string(val); }
    template<> inline std::string to_string<long long>(const long long& val) { return std::to_string(val); }
    template<> inline std::string to_string<float>(const float& val) { return std::to_string(val); }
    template<> inline std::string to_string<double>(const double& val) { return std::to_string(val); }
    template<> inline std::string to_string<long double>(const long double& val) { return std::to_string(val); }
    template<> inline std::string to_string<std::string>(const std::string& str) { return str; }
    inline std::string to_string(const char* str) { return str; }



    class Config {

        std::map<std::string, 
            std::map<std::string, std::string>> config_;

        //to store comments from file
        std::map<std::string, 
            std::map<std::string, std::vector<std::string>>> config_comments_;


    public:
        /*class Classifications {
            std::set<std::string> men;
        } classifications;

        std::set<std::string> property_item_men_;
        */

        bool load(const char* file);
        bool save(const char* file);
    
        inline bool has(const std::string& param) const {  
            return config_.count("unspecified") > 0 && config_.at("unspecified").count(param) > 0;
        }
        inline bool has(const std::string& section, const std::string& param) const {  
            return config_.count(section) > 0 && config_.at(section).count(param) > 0;
        }
        inline std::map<std::string, std::string>& operator[](const std::string& str)  {  return config_.at(str);  }

        template<typename T>
        inline T get(const char* param_name, T def) const {
            return has(param_name) ? string_to<T>(config_.at("unspecified").at(param_name)) : def;
        }

        template<typename T>
        inline T get(const char* section, const char* param_name, T def) const {
            return has(section, param_name) ? string_to<T>(config_.at(section).at(param_name)) : def;
        }

        template<typename T>
        T get_case_insensitive(const char* section, const char* param_name, T def) const {
            
            std::string req(section);
            if (config_.find(req) == config_.end())
                return def;
            
            for (const auto& it : config_.at(req))
            {
                if (stricmp(it.first.c_str(), param_name) == 0)
                    return string_to<T>(it.second);
            }
            return def;
        }

        template<typename T>
        inline void set(const char* section, const char* param_name, T val) {
            config_[section][param_name] = to_string(val);
        }

        typedef std::map<std::string, std::map<std::string, std::string>>::iterator sect_iterator;
        typedef std::map<std::string, std::string>::iterator pair_iterator;

        sect_iterator section_begin() {  return config_.begin();  }
        sect_iterator section_end() {  return config_.end();  }
        pair_iterator pair_begin(const std::string& section) {  return config_[section].begin();  }
        pair_iterator pair_end(const std::string& section) {  return config_[section].end();  }

        void delete_section(const std::string& section) {
            config_.erase(section);
        }

        bool operator==(Config& cfg) {
            for (auto& sect : config_)
            {
                if (cfg.config_.find(sect.first) == cfg.config_.end())
                    return false;
                else 
                {
                   for (auto& pair : sect.second)
                   {
                      if (cfg.config_[sect.first].find(pair.first) == cfg.config_[sect.first].end())
                          return false;
                      else if (config_[sect.first][pair.first] != cfg.config_[sect.first][pair.first])
                          return false;
                   }
                }
            }
            return true;
        }
        
    };

    //template<> void Config::set<std::string>(const char* section, const char* param_name, std::string val) {
    //    config_[section][param_name] = val;
    //}

}

#endif //SIMPLE_CONFIG_HEADER

