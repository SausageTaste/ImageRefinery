#include "sung/imgref/argpar.hpp"

#include <argparse/argparse.hpp>


namespace sung {

    std::optional<std::string> parse_args_img_ref(
        ImgRefWorkConfigs& out, int argc, char* argv[]
    ) {
        argparse::ArgumentParser p("Image Refinery");

        std::vector<std::string> inputs;
        p.add_argument("inputs")
            .help("Input image file and folder paths")
            .append()
            .store_into(inputs);

        p.add_argument("-o", "--output").help("Output folder path");

        p.add_argument("-i", "--inplace")
            .help("Replace input files with output files")
            .store_into(out.inplace_);

        p.add_argument("-r", "--recursive")
            .help("Walk into input directories recursively")
            .store_into(out.recursive_);

        p.add_argument("-t", "--threshold")
            .help("Reduction threshold")
            .default_value(0.9)
            .store_into(out.reduction_threshold_);

        p.add_argument("--webp")
            .help("Allow conversion to WebP format")
            .default_value(false)
            .implicit_value(true)
            .store_into(out.allow_webp_);

        try {
            p.parse_args(argc, argv);
        } catch (const std::exception& err) {
            return err.what();
        }

        for (auto& x : inputs) out.inputs_.emplace_back(x);

        if (p.is_used("--output")) {
            const auto output_dir_str = p.get<std::string>("--output");
            out.output_dir_ = fs::path(output_dir_str).lexically_normal();
        } else {
            out.output_dir_ = std::nullopt;
        }

        return std::nullopt;
    }

    sung::Expected<ImgRefWorkConfigs, std::string> parse_args_img_ref(
        int argc, char* argv[]
    ) {
        ImgRefWorkConfigs out;
        if (const auto err = parse_args_img_ref(out, argc, argv))
            return sung::unexpected(err.value());
        return out;
    }

}  // namespace sung
