#pragma once

#include <map>
#include <set>
#include <vector>

#include <OpenImageIO/imagebuf.h>


namespace sung {

    class ImageSize2D {

    public:
        ImageSize2D(int width, int height);

        void resize_to_fit_into(double frame_w, double frame_h);
        void resize_to_enclose(double frame_w, double frame_h);

        void resize_for_jpeg();
        void resize_for_webp();

        int width() const;
        int height() const;

    private:
        double width_ = 0;
        double height_ = 0;
        std::set<double> factors_;
    };


    struct ImageProperties {
        bool animated_ = false;
        bool transparent_ = false;
    };


    class ImageAnalyser {

    public:
        ImageAnalyser(const OIIO::ImageBuf& img);

        bool is_animated() const;
        bool is_transparent() const;

        ImageProperties get_properties() const;

    private:
        const OIIO::ImageBuf& img_;
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

        std::string build_jpeg(
            const std::string_view& name,
            const OIIO::ImageBuf& img,
            const int quality_level
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
        std::map<std::string, Record> data_;
    };

}  // namespace sung
