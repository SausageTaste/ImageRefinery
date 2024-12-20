#include "sung/imgref/img_refinery.hpp"

#include <OpenImageIO/filesystem.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "sung/imgref/filesys.hpp"


namespace {

    int round_int(const double x) { return static_cast<int>(std::round(x)); }

}  // namespace


namespace {

    class OIIOImage2D : public sung::oiio::IImage2D {

    public:
        OIIOImage2D(const OIIO::string_view path) : img_(path) {}

        OIIO::ImageBuf& get() { return img_; }
        const OIIO::ImageBuf& get() const { return img_; }

    private:
        OIIO::ImageBuf img_;
    };


    bool is_img_transparent(const OIIO::ImageBuf& img) {
        const auto& spec = img.spec();

        auto alpha = spec.alpha_channel;
        if (alpha >= spec.nchannels) {
            auto& ch_names = spec.channelnames;
            const auto it = std::find(ch_names.begin(), ch_names.end(), "A");
            if (it != ch_names.end())
                alpha = static_cast<int>(std::distance(ch_names.begin(), it));
            else
                return false;
        }

        if (alpha < 0)
            return false;
        else if (OIIO::ImageBufAlgo::isConstantChannel(img, alpha, 1.f, 0.1f))
            return false;
        else
            return true;
    }

    bool is_img_monochrome(const OIIO::ImageBuf& img) {
        auto roi = OIIO::get_roi(img.spec());
        roi.chend = std::min(3, roi.chend);  // only test RGB, not alpha
        return OIIO::ImageBufAlgo::isMonochrome(img, 0.075f, roi);
    }

}  // namespace


// ImageSize2D
namespace sung::oiio {

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

    void ImageSize2D::resize_for_i16p() {
        this->resize_to_fit_into(1206, 2622);
    }

    void ImageSize2D::resize_for_i16p_pages() {
        if (width_ > height_)
            this->resize_to_fit_into(1206 * 2, 2622);
        else
            this->resize_to_fit_into(1206, 2622);
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

}  // namespace sung::oiio


// ImageExportHarbor
namespace sung::oiio {

    ImageExportHarbor::ImageExportHarbor() {}

    ImageExportHarbor::~ImageExportHarbor() {}

    std::string ImageExportHarbor::build_png(
        const std::string_view& name,
        const IImage2D& img_ptr,
        const int compression_level
    ) {
        const auto& img = dynamic_cast<const OIIOImage2D&>(img_ptr).get();

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
        const IImage2D& img_ptr,
        const int quality_level
    ) {
        const auto& img = dynamic_cast<const OIIOImage2D&>(img_ptr).get();

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
        const IImage2D& img_ptr,
        const int compression_level
    ) {
        const auto& img = dynamic_cast<const OIIOImage2D&>(img_ptr).get();

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
        const std::string_view& name, const IImage2D& img_ptr
    ) {
        const auto& img = dynamic_cast<const OIIOImage2D&>(img_ptr).get();

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

    std::vector<std::pair<std::string, const ImageExportHarbor::Record*>>
    ImageExportHarbor::get_sorted_by_size() const {
        std::vector<std::pair<std::string, const Record*>> sorted;
        sorted.reserve(data_.size());
        for (const auto& [name, record] : data_)
            sorted.push_back({ name, &record });

        std::sort(
            sorted.begin(),
            sorted.end(),
            [](const auto& lhs, const auto& rhs) {
                return lhs.second->data_.size() < rhs.second->data_.size();
            }
        );

        return sorted;
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

}  // namespace sung::oiio


// namespace sung::oiio
namespace sung::oiio {

    ImgExpected open_img(const std::filesystem::path& path) {
        auto ptr = std::make_unique<::OIIOImage2D>(make_utf8_str(path));
        auto& img = ptr->get();
        if (!img.read())
            sung::unexpected(img.geterror());

        return std::move(ptr);
    }

    ImageProperties get_img_properties(const IImage2D& img) {
        ImageProperties props;

        const auto& img_buf = dynamic_cast<const OIIOImage2D&>(img).get();
        const auto& spec = img_buf.spec();

        props.width_ = spec.width;
        props.height_ = spec.height;
        props.animated_ = 0 != spec.get_int_attribute("oiio:Movie", 0);
        props.transparent_ = ::is_img_transparent(img_buf);
        props.monochrome_ = ::is_img_monochrome(img_buf);

        return props;
    }

    ImgExpected resize_img(const IImage2D& img, const ImageSize2D& img_dim) {
        const auto& img_buf = dynamic_cast<const OIIOImage2D&>(img).get();
        const OIIO::ROI roi(
            0,
            img_dim.width(),
            0,
            img_dim.height(),
            0,
            1,
            0,
            img_buf.nchannels()
        );

        auto out = std::make_unique<OIIOImage2D>("");
        const auto res = OIIO::ImageBufAlgo::resize(
            out->get(), img_buf, nullptr, roi
        );
        if (!res)
            return sung::unexpected(OIIO::geterror());

        return std::move(out);
    }

    ImgExpected drop_alpha_ch(const IImage2D& img_ptr) {
        const auto& img = dynamic_cast<const OIIOImage2D&>(img_ptr).get();
        auto& spec = img.spec();

        auto out = std::make_unique<OIIOImage2D>("");
        if (spec.alpha_channel < 0) {
            out->get().copy(img);
            return std::move(out);
        }

        if (spec.nchannels != 4) {
            return sung::unexpected(fmt::format(
                "Cannot drop alpha channel if nchannel is {}", spec.nchannels
            ));
        }

        if (!OIIO::ImageBufAlgo::channels(out->get(), img, 3, {}))
            return sung::unexpected(OIIO::geterror());

        return std::move(out);
    }

    ImgExpected merge_greyscale_channels(const IImage2D& img_ptr) {
        const auto& img = dynamic_cast<const OIIOImage2D&>(img_ptr).get();
        auto out = std::make_unique<OIIOImage2D>("");
        if (!OIIO::ImageBufAlgo::channels(out->get(), img, 1, {}))
            return sung::unexpected(OIIO::geterror());

        return std::move(out);
    }

}  // namespace sung::oiio
