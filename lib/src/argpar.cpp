#include "sung/imgref/argpar.hpp"

#include <argparse/argparse.hpp>


namespace sung {

    std::optional<std::string> ImgRefArgParser::parse(int argc, char* argv[]) {
        argparse::ArgumentParser p("Image Refinery");

        p.add_argument("inputs")
            .help("Input image file are folder paths")
            .append()
            .store_into(inputs_);

        try {
            p.parse_args(argc, argv);
        } catch (const std::exception& err) {
            return err.what();
        }
    }

}  // namespace sung
