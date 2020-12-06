#include "cfg_loader.h"
#include <fstream>
#include <algorithm>

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch) && !(ch == '\r') && !(ch == '\n') && !(ch == '\t');
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch) && !(ch == '\r') && !(ch == '\n') && !(ch == '\t');
    }).base(), s.end());
}

// trim from both ends (in place)
static inline std::string trim(std::string s) {
    ltrim(s);
    rtrim(s);
    return s;
}

static inline void trim_inplace(std::string& s) {
    ltrim(s);
    rtrim(s);
}

namespace config {
    bool Config::load(const char* file)
    {
        std::ifstream infile(file);
        if (!infile.is_open())
            return false;

        std::vector<std::string> comments_;
        std::string line;
        std::string current_section = "unspecified";
        while (std::getline(infile, line))
        {
            // store comments
            if (line.find('#') != std::string::npos) {
                comments_.push_back(line.substr(line.find('#')));
                line = line.substr(0, line.find('#'));
            }

            line = trim(line);
            if (line.size() == 0)
                continue;

            // key-value search
            size_t sep = line.find('=');
            if (sep != std::string::npos) {
                std::string key = trim(line.substr(0, sep));
                if (comments_.size() > 0)
                    config_comments_[current_section][key] = comments_;
                config_[current_section][key] = trim(line.substr(sep + 1, std::string::npos));
                comments_.clear();
                continue;
            }              

            // section search & change
            size_t block_beg = line.find('[');
            size_t block_end = line.find(']', block_beg);
            if (block_beg != std::string::npos && block_end != std::string::npos)
            {
                current_section = trim(line.substr(block_beg + 1, block_end - block_beg - 1));
                if (comments_.size() > 0)
                    config_comments_[current_section][""] = comments_;                
                comments_.clear();
                continue;
            }
        }
        return true;
    }

    bool Config::save(const char* file)  
    {
        std::ofstream outfile(file, std::ofstream::out);

        //unspecified
        if (config_.find("unspecified") != config_.end())
        {
            for (const auto& key_val : config_["unspecified"])
            {
                for (const auto& line : config_comments_["unspecified"][key_val.first])
                    outfile << line << std::endl;

                outfile << key_val.first << " = " << key_val.second << std::endl;
            }
        }

        //the rest
        for (const auto& sect : config_)
        {
            //section comments & section
            for (const auto& line : config_comments_[sect.first][""])
                outfile << line << std::endl;
            outfile << "[" << sect.first << "]" << std::endl;
            
            outfile << std::endl;
            
            for (const auto& key_val : sect.second)
            {//comments per key-value
                for (const auto& line : config_comments_[sect.first][key_val.first])
                    outfile << line << std::endl;

                outfile << key_val.first << " = " << key_val.second << std::endl;
            }
            outfile << std::endl;
        }

        outfile.close();
        return false;
    }
}