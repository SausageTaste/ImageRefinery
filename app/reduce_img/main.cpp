#include <vector>

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <fmt/core.h>

#include "sung/imgref/filesys.hpp"


namespace {

    namespace fs = std::filesystem;


    std::string make_str_lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    std::string strip_quotes(std::string str) {
        while (str.starts_with('"')) str = str.substr(1);
        while (str.ends_with('"')) str = str.substr(0, str.size() - 1);
        while (str.starts_with('\'')) str = str.substr(1);
        while (str.ends_with('\'')) str = str.substr(0, str.size() - 1);
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

        const OIIO::ROI roi(0, 640, 0, 480, 0, 1, 0, img.nchannels());
        const auto resized = OIIO::ImageBufAlgo::resize(img, nullptr, roi);
        if (resized.has_error())
            return resized.geterror();

        const auto new_name_ext = fmt::format(
            "{}_resized.png", path.stem().string()
        );
        const auto out_path = path.parent_path() / new_name_ext;

        OIIO::ImageSpec spec{
            roi.width(), roi.height(), nc, OIIO::TypeDesc::UINT8
        };
        spec["png:compressionLevel"] = 9;

        auto out = OIIO::ImageOutput::create(out_path.string());
        out->open(out_path.string(), spec);

        resized.write(
            out.get(),
            [](void* opaque_data, float portion_done) { return false; },
            nullptr
        );

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
        };

        const auto ext = ::make_str_lower(path.extension().string());
        if (allowed_exts.find(ext) != allowed_exts.end())
            return true;

        return false;
    };

    file_list.add("D:/Downloads/Images/ua.jpg", false);

    for (const auto& path : file_list.get_files()) {
        const auto result = ::do_work(path);
        break;
    }

    return 0;
}
