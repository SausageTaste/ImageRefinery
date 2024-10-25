#include <map>
#include <vector>

#include <OpenImageIO/imagebufalgo.h>
#include <fmt/core.h>

#include "sung/imgref/filesys.hpp"
#include "sung/imgref/img_refinery.hpp"


namespace {

    namespace fs = std::filesystem;


    std::string make_str_lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    std::string do_work(const fs::path& path) {
        OIIO::ImageBuf img(path.string());
        const auto ok = img.read();
        if (!ok)
            return img.geterror();

        const auto width = img.spec().width;
        const auto height = img.spec().height;
        const auto nc = img.nchannels();
        const auto npixels = img.spec().image_pixels();

        sung::ImageSize2D img_dim(width, height);
        img_dim.resize_to_enclose(500, 500);
        const OIIO::ROI roi(
            0, img_dim.width(), 0, img_dim.height(), 0, 1, 0, img.nchannels()
        );

        fmt::print(
            "Resized: {}x{} -> {}x{}\n",
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
        harbor.build_webp("webp 100", resized, 100);
        harbor.build_webp("webp 80", resized, 80);

        /*
        {
            const auto [name, record] = *harbor.pick_the_smallest();
            const auto file_name_ext = fmt::format(
                "{}_{}.{}", path.stem().string(), name, record.file_ext_
            );
            const auto out_path = path.parent_path() / file_name_ext;
            std::fstream file(out_path, std::ios::out | std::ios::binary);
            file.write((const char*)record.data_.data(), record.data_.size());
        }
        */

        return "success";
    }

}  // namespace


int main() {
    sung::FileList file_list;
    file_list.file_filter_ = [](fs::path path) {
        static const std::set<std::string> allowed_exts = {
            ".png",
            ".jpg",
            ".jpeg",
            ".webp",
        };

        const auto ext = ::make_str_lower(path.extension().string());
        if (allowed_exts.find(ext) != allowed_exts.end())
            return true;

        return false;
    };

    file_list.add("D:/Downloads/Images", false);

    for (const auto& path : file_list.get_files()) {
        const auto result = ::do_work(path);
        fmt::print("{}: {}\n", path.string(), result);
    }

    return 0;
}
