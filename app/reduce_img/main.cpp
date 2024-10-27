#include <fstream>
#include <map>
#include <vector>

#include <fmt/core.h>
#include <sung/general/stringtool.hpp>

#include "sung/imgref/filesys.hpp"
#include "sung/imgref/img_refinery.hpp"


namespace {

    namespace fs = std::filesystem;


    std::string make_str_lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    std::string do_work(const fs::path& path, bool webp) {
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

        const fs::path output_dir = "C:/Users/woos8/Desktop/ImageRefineryTest";

        for (auto& [name, record] : harbor.get_sorted_by_size()) {
            auto file_name_ext = path.stem();
            file_name_ext += "_";
            file_name_ext += name;
            file_name_ext += ".";
            file_name_ext += record->file_ext_;
            file_name_ext = sung::normalize_utf8_path(file_name_ext);

            const auto out_path = output_dir / file_name_ext;
            std::fstream file(out_path, std::ios::out | std::ios::binary);
            file.write((const char*)record->data_.data(), record->data_.size());

            break;
        }

        return "success";
    }

}  // namespace


int main() {
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

    for (const auto& path : file_list.get_files()) {
        const auto result = ::do_work(path, false);
        fmt::print(" * {}: {}\n", sung::make_utf8_str(path), result);
    }

    return 0;
}
