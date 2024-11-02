#pragma once

#include <filesystem>
#include <optional>
#include <vector>


namespace sung {

    namespace fs = std::filesystem;


    struct ImgRefWorkConfigs {
        std::vector<fs::path> inputs_;
        std::optional<fs::path> output_dir_;
        double reduction_threshold_ = 1;
        bool inplace_ = false;
        bool recursive_ = false;
        bool allow_webp_ = false;
    };

}  // namespace sung
