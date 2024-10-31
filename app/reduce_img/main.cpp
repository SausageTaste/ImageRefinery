#include <fstream>
#include <map>
#include <vector>

#include <fmt/core.h>
#include <sung/general/stringtool.hpp>

#include "sung/imgref/argpar.hpp"
#include "sung/imgref/filesys.hpp"
#include "sung/imgref/img_refinery.hpp"


namespace {

    namespace fs = std::filesystem;


    std::string do_work(
        const fs::path& path,
        const sung::ExternalResultLoc& output_loc,
        const sung::ImgRefWorkConfigs& configs
    ) {
        const auto src_size = fs::file_size(path);
        auto img = sung::oiio::open_img(path);
        if (!img)
            return img.error();

        const auto props = sung::oiio::get_img_properties(**img);
        if (props.animated_)
            return "Animated image not supported";

        sung::oiio::ImageSize2D img_dim(props.width_, props.height_);
        img_dim.resize_for_jpeg();
        img_dim.resize_to_enclose(2000, 2000);
        if (configs.allow_webp_)
            img_dim.resize_for_webp();

        auto mod = sung::oiio::resize_img(**img, img_dim);
        if (!mod)
            return mod.error();

        if (!props.transparent_) {
            mod = sung::oiio::drop_alpha_ch(**mod);
            if (!mod)
                return mod.error();
        }

        sung::oiio::ImageExportHarbor harbor;
        if (configs.allow_webp_)
            harbor.build_webp("webp 80", **mod, 80);
        if (props.transparent_)
            harbor.build_png("png", **mod, 9);
        else
            harbor.build_jpeg("jpeg 80", **mod, 80);

        if (props.monochrome_ && !props.transparent_) {
            mod = sung::oiio::merge_greyscale_channels(**mod);
            if (!mod)
                return mod.error();
            harbor.build_jpeg("jpeg 80 monochrome", **mod, 80);
        }

        sung::FilePathMap img_map{ path };
        for (auto& [name, record] : harbor.get_sorted_by_size()) {
            if (record->data_.size() >= src_size * 0.9)
                return "Not enough reduction";

            const auto out_path = img_map.add_with_suffix(
                fmt::format("{}.{}", name, record->file_ext_), output_loc
            );

            sung::create_folder(out_path.parent_path());
            std::fstream file(out_path, std::ios::out | std::ios::binary);
            if (!file)
                return "Failed to open file";
            file.write((const char*)record->data_.data(), record->data_.size());

            break;
        }

        if (configs.inplace_) {
            const auto res = img_map.replace_src();
            if (!res)
                return "Failed to replace img: " + res.error();
        }

        return "success";
    }

}  // namespace


int main(int argc, char* argv[]) {
    const auto args_expected = sung::parse_args_img_ref(argc, argv);
    if (!args_expected.has_value()) {
        fmt::print("{}\n", args_expected.error());
        return 1;
    }
    const auto& configs = args_expected.value();

    sung::AllowedExtFileFilter file_filter;
    file_filter.add_allowed_ext(".png");
    file_filter.add_allowed_ext(".jpg");
    file_filter.add_allowed_ext(".jpeg");
    file_filter.add_allowed_ext(".webp");
    file_filter.add_allowed_ext(".gif");

    sung::FileList file_list;
    file_list.file_filter_ = file_filter;

    for (const auto& path : configs.inputs_) {
        file_list.add(path, configs.recursive_);
    }

    const sung::ExternalResultLoc output_loc(
        file_list.get_longest_common_prefix(),
        *sung::make_fol_path_with_suffix(
            configs.output_dir_.value_or(fs::temp_directory_path() / "imgref")
        )
    );

    for (const auto& path : file_list.get_files()) {
        const auto result = ::do_work(path, output_loc, configs);
        fmt::print(" * {}: {}\n", sung::make_utf8_str(path), result);
    }

    return 0;
}
