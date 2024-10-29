#pragma once

#include <optional>
#include <string>
#include <vector>


namespace sung {

    class ImgRefArgParser {

    public:
        // Returns error message if parsing failed
        // Returns empty optional if parsing succeeded
        std::optional<std::string> parse(int argc, char* argv[]);

        std::vector<std::string> inputs_;
    };

}  // namespace sung
