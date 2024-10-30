#pragma once

#include <filesystem>
#include <vector>


namespace sung {

    namespace fs = std::filesystem;


    struct ImgRefWorkConfigs {
        std::vector<fs::path> inputs_;
        fs::path output_dir_;
        bool recursive_ = false;
        bool allow_webp_ = false;
    };

}  // namespace sung
