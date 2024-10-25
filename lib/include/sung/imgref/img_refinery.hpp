#pragma once

#include <map>
#include <vector>

#include <OpenImageIO/imagebuf.h>


namespace sung {

    class ImageExportHarbor {

    public:
        ImageExportHarbor();
        ~ImageExportHarbor();

        // Returns empty string on success, error message otherwise.
        std::string build_png(
            const std::string_view& name,
            const OIIO::ImageBuf& img,
            const int compression_level = 9
        );

        std::string build_webp(
            const std::string_view& name,
            const OIIO::ImageBuf& img,
            const int compression_level = 100
        );

        std::string build_webp_lossless(
            const std::string_view& name, const OIIO::ImageBuf& img
        );

    private:
        struct Record;
        std::map<std::string, Record> data_;
    };

}  // namespace sung
