#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <sung/general/expected.hpp>


namespace sung::oiio {

    struct IImage2D {
        virtual ~IImage2D() = default;
    };


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
        int width_ = 0;
        int height_ = 0;
        bool animated_ = false;
        bool transparent_ = false;
        bool monochrome_ = false;
    };


    using ImgExpected = sung::Expected<std::unique_ptr<IImage2D>, std::string>;

    ImgExpected open_img(const std::filesystem::path& path);

    ImageProperties get_img_properties(const IImage2D& img);

    ImgExpected resize_img(const IImage2D& img, const ImageSize2D& img_dim);

    ImgExpected drop_alpha_ch(const IImage2D& img);

    ImgExpected merge_greyscale_channels(const IImage2D& img);


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
            const IImage2D& img,
            const int compression_level = 9
        );

        std::string build_jpeg(
            const std::string_view& name,
            const IImage2D& img,
            const int quality_level
        );

        std::string build_webp(
            const std::string_view& name,
            const IImage2D& img,
            const int compression_level = 100
        );

        std::string build_webp_lossless(
            const std::string_view& name, const IImage2D& img
        );

        void sort_by_size();

        using Iter_t = std::map<std::string, Record>::const_iterator;

        std::vector<std::pair<std::string, const Record*>> get_sorted_by_size() const;
        Iter_t pick_the_smallest() const;

    private:
        std::map<std::string, Record> data_;
    };

}  // namespace sung::oiio
