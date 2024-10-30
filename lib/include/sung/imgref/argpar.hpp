#pragma once

#include <optional>
#include <string>

#include <sung/general/expected.hpp>

#include "sung/imgref/configs.hpp"
#include "sung/imgref/filesys.hpp"


namespace sung {

    // Returns error message if parsing failed
    // Returns empty optional if parsing succeeded
    std::optional<std::string> parse_args_img_ref(
        ImgRefWorkConfigs& out, int argc, char* argv[]
    );

    sung::Expected<ImgRefWorkConfigs, std::string> parse_args_img_ref(
        int argc, char* argv[]
    );

}  // namespace sung
