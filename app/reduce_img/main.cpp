#include <filesystem>
#include <vector>

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <fmt/core.h>


int main() {
    OIIO::ImageBuf img("D:/Downloads/Images/ua.jpg");
    const auto ok = img.read();
    if (!ok) {
        fmt::print("Failed to read image\n");
        return 1;
    }

    const auto width = img.spec().width;
    const auto height = img.spec().height;
    const auto nc = img.nchannels();
    const auto npixels = img.spec().image_pixels();

    const OIIO::ROI roi(0, 640, 0, 480, 0, 1, 0, img.nchannels());
    const auto resized = OIIO::ImageBufAlgo::resize(img, nullptr, roi);
    if (resized.has_error()) {
        fmt::print("Failed to resize image: {}\n", resized.geterror());
        return 1;
    }

    resized.write("D:/Downloads/Images/ua_resized.jpg");

    const auto median_filtered = OIIO::ImageBufAlgo::median_filter(resized);
    if (median_filtered.has_error()) {
        fmt::print(
            "Failed to apply median filter: {}\n", median_filtered.geterror()
        );
        return 1;
    }

    median_filtered.write("D:/Downloads/Images/ua_median_filtered.jpg");

    return 0;
}
