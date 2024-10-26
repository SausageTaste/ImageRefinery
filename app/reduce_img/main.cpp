#include <map>
#include <vector>

#include <OpenImageIO/imagebufalgo.h>
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

    std::string do_work(const fs::path& path) {
        OIIO::ImageBuf img(sung::make_utf8_str(path));
        const auto ok = img.read();
        if (!ok)
            return img.geterror();

        const auto& spec = img.spec();
        const auto width = spec.width;
        const auto height = spec.height;
        const auto nc = img.nchannels();
        const auto npixels = spec.image_pixels();

        const sung::ImageAnalyser anal{ img };
        if (anal.is_animated())
            fmt::print("Animated image\n");
        if (anal.is_transparent())
            fmt::print("Transparent image\n");

        sung::ImageSize2D img_dim(width, height);
        img_dim.resize_for_jpeg();
        img_dim.resize_for_webp();
        img_dim.resize_to_enclose(2000, 2000);
        const OIIO::ROI roi(
            0, img_dim.width(), 0, img_dim.height(), 0, 1, 0, img.nchannels()
        );

        fmt::print(
            "Resize: {}x{} -> {}x{}\n",
            width,
            height,
            img_dim.width(),
            img_dim.height()
        );

        const auto resized = OIIO::ImageBufAlgo::resize(img, nullptr, roi);
        if (resized.has_error())
            return OIIO::geterror();

        sung::ImageExportHarbor harbor;
        harbor.build_png("png", resized, 9);
        harbor.build_jpeg("jpeg 80", resized, 80);
        harbor.build_webp("webp 80", resized, 80);

        const fs::path output_dir = "C:/Users/woos8/Desktop/ImageRefineryTest";

        for (auto [name, record] : harbor) {
            auto file_name_ext = path.stem();
            file_name_ext += "_";
            file_name_ext += name;
            file_name_ext += ".";
            file_name_ext += record.file_ext_;
            file_name_ext = sung::normalize_utf8_path(file_name_ext);

            const auto out_path = output_dir / file_name_ext;
            fmt::print(
                "{} ({})\n",
                sung::make_utf8_str(out_path),
                sung::format_bytes(record.data_.size())
            );
            std::fstream file(out_path, std::ios::out | std::ios::binary);
            file.write((const char*)record.data_.data(), record.data_.size());
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
        const auto result = ::do_work(path);
        fmt::print(" * {}: {}\n", sung::make_utf8_str(path), result);
    }

    return 0;
}
