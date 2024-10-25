#include "sung/imgref/img_refinery.hpp"

#include <OpenImageIO/filesystem.h>


namespace sung {

    struct ImageExportHarbor::Record {
        std::vector<unsigned char> data_;
        std::string file_ext_;
    };


    ImageExportHarbor::ImageExportHarbor() {}

    ImageExportHarbor::~ImageExportHarbor() {}

    std::string ImageExportHarbor::build_png(
        const std::string_view& name,
        const OIIO::ImageBuf& img,
        const int compression_level
    ) {
        auto spec = img.spec();
        spec["png:compressionLevel"] = compression_level;

        auto it = data_.emplace(name, Record{});
        if (!it.second)
            return "Name already exists";

        auto& record = it.first->second;
        record.file_ext_ = "png";
        OIIO::Filesystem::IOVecOutput vecout{ record.data_ };

        auto out = OIIO::ImageOutput::create("png", &vecout);
        if (!out)
            return OIIO::geterror();
        if (!out->open("png", spec))
            return OIIO::geterror();

        img.write(
            out.get(),
            [](void* opaque_data, float portion_done) { return false; },
            nullptr
        );

        return {};
    }

    std::string ImageExportHarbor::build_webp(
        const std::string_view& name,
        const OIIO::ImageBuf& img,
        const int compression_level
    ) {
        auto spec = img.spec();
        spec["CompressionQuality"] = compression_level;

        auto it = data_.emplace(name, Record{});
        if (!it.second)
            return "Name already exists";

        auto& record = it.first->second;
        record.file_ext_ = "webp";
        OIIO::Filesystem::IOVecOutput vecout{ record.data_ };

        auto out = OIIO::ImageOutput::create("webp", &vecout);
        if (!out)
            return OIIO::geterror();
        if (!out->open("webp", spec))
            return OIIO::geterror();

        img.write(
            out.get(),
            [](void* opaque_data, float portion_done) { return false; },
            nullptr
        );

        return {};
    }

    std::string ImageExportHarbor::build_webp_lossless(
        const std::string_view& name, const OIIO::ImageBuf& img
    ) {
        auto spec = img.spec();
        spec["Compression"] = "lossless";

        auto it = data_.emplace(name, Record{});
        if (!it.second)
            return "Name already exists";

        auto& record = it.first->second;
        record.file_ext_ = "webp";
        OIIO::Filesystem::IOVecOutput vecout{ record.data_ };

        auto out = OIIO::ImageOutput::create("webp", &vecout);
        if (!out)
            return OIIO::geterror();
        if (!out->open("webp", spec))
            return OIIO::geterror();

        img.write(
            out.get(),
            [](void* opaque_data, float portion_done) { return false; },
            nullptr
        );

        return {};
    }

}  // namespace sung
