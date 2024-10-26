#include "sung/imgref/img_refinery.hpp"

#include <OpenImageIO/filesystem.h>


namespace {

    int round_int(const double x) { return static_cast<int>(std::round(x)); }

}  // namespace


// ImageSize2D
namespace sung {

    ImageSize2D::ImageSize2D(int width, int height)
        : width_(width), height_(height) {}

    void ImageSize2D::resize_to_fit_into(double frame_w, double frame_h) {
        const auto width_ratio = frame_w / width_;
        const auto height_ratio = frame_h / height_;
        const auto ratio = std::min(width_ratio, height_ratio);
        factors_.insert(ratio);
    }

    void ImageSize2D::resize_to_enclose(double frame_w, double frame_h) {
        const auto width_ratio = frame_w / width_;
        const auto height_ratio = frame_h / height_;
        const auto ratio = std::max(width_ratio, height_ratio);
        factors_.insert(ratio);
    }

    void ImageSize2D::resize_for_jpeg() {
        constexpr double MAX_LEN = 65535;
        this->resize_to_fit_into(MAX_LEN, MAX_LEN);
    }

    void ImageSize2D::resize_for_webp() {
        constexpr double MAX_LEN = 16383;
        this->resize_to_fit_into(MAX_LEN, MAX_LEN);
    }

    int ImageSize2D::width() const {
        const auto factor = *factors_.begin();
        if (factor < 1)
            return ::round_int(width_ * factor);
        else
            return ::round_int(width_);
    }

    int ImageSize2D::height() const {
        const auto factor = *factors_.begin();
        if (factor < 1)
            return ::round_int(height_ * factor);
        else
            return ::round_int(height_);
    }

}  // namespace sung


// ImageExportHarbor
namespace sung {

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

    std::string ImageExportHarbor::build_jpeg(
        const std::string_view& name,
        const OIIO::ImageBuf& img,
        const int quality_level
    ) {
        auto spec = img.spec();
        spec["CompressionQuality"] = quality_level;

        auto it = data_.emplace(name, Record{});
        if (!it.second)
            return "Name already exists";

        auto& record = it.first->second;
        record.file_ext_ = "jpg";
        OIIO::Filesystem::IOVecOutput vecout{ record.data_ };

        auto out = OIIO::ImageOutput::create("jpeg", &vecout);
        if (!out)
            return OIIO::geterror();
        if (!out->open("jpeg", spec))
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

    ImageExportHarbor::Iter_t ImageExportHarbor::begin() const {
        return data_.begin();
    }

    ImageExportHarbor::Iter_t ImageExportHarbor::end() const {
        return data_.end();
    }

    ImageExportHarbor::Iter_t ImageExportHarbor::pick_the_smallest() const {
        return std::min_element(
            data_.begin(),
            data_.end(),
            [](const auto& lhs, const auto& rhs) {
                return lhs.second.data_.size() < rhs.second.data_.size();
            }
        );
    }

}  // namespace sung
