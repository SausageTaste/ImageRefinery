#pragma once

#include <map>
#include <set>
#include <vector>

#include <OpenImageIO/imagebuf.h>


namespace sung {

    class ImageSize2D {

    public:
        ImageSize2D(double width, double height);

        void resize_to_fit_into(double frame_w, double frame_h);
        void resize_to_enclose(double frame_w, double frame_h);

        int width() const;
        int height() const;

    private:
        double width_ = 0;
        double height_ = 0;
        std::set<double> factors_;
    };


    class ImageExportHarbor {

    public:
        struct Record {
            std::vector<unsigned char> data_;
            std::string file_ext_;
        };


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

        using Iter_t = std::map<std::string, Record>::const_iterator;

        Iter_t begin() const;
        Iter_t end() const;
        Iter_t pick_the_smallest() const;

    private:
        struct Record;
        std::map<std::string, Record> data_;
    };

}  // namespace sung
