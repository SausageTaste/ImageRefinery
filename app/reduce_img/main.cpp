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


    std::string make_str_lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    std::string do_work(
        const fs::path& path,
        const sung::ExternalResultLoc& output_loc,
        bool webp
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
        if (webp)
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
        if (webp)
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

        for (auto& [name, record] : harbor.get_sorted_by_size()) {
            if (record->data_.size() > src_size)
                return "Result is larger than the source";

            auto file_name_ext = path.stem();
            file_name_ext += "_";
            file_name_ext += name;
            file_name_ext += ".";
            file_name_ext += record->file_ext_;
            file_name_ext = sung::normalize_utf8_path(file_name_ext);
            file_name_ext = path.parent_path() / file_name_ext;

            const auto out_path = output_loc.get_path_for(file_name_ext);
            sung::create_folder(out_path.parent_path());
            std::fstream file(out_path, std::ios::out | std::ios::binary);
            if (!file)
                return "Failed to open file";
            file.write((const char*)record->data_.data(), record->data_.size());

            break;
        }

        return "success";
    }

}  // namespace


int main(int argc, char* argv[]) {
    sung::ImgRefArgParser arg_parser;
    if (const auto parse_result = arg_parser.parse(argc, argv)) {
        fmt::print("{}\n", *parse_result);
        return 1;
    }

    return 0;

    sung::FileList file_list;
    file_list.file_filter_ = [](fs::path path) {
        static const std::set<std::string> allowed_exts = {
            ".png", ".jpg", ".jpeg", ".webp", ".gif",
        };

        const auto ext = ::make_str_lower(path.extension().string());
        if (allowed_exts.find(ext) != allowed_exts.end())
            return true;

        return false;
    };

    file_list.add("C:/Users/woos8/Desktop/Test Images", false);

    sung::ExternalResultLoc output_loc(
        file_list.get_longest_common_prefix(),
        "C:/Users/woos8/Desktop/ImageRefineryTest"
    );

    for (const auto& path : file_list.get_files()) {
        const auto result = ::do_work(path, output_loc, false);
        fmt::print(" * {}: {}\n", sung::make_utf8_str(path), result);
    }

    return 0;
}
