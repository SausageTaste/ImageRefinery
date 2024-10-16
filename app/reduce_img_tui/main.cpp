#include <vector>

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <fmt/core.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "sung/imgref/filesys.hpp"


namespace {

    namespace fs = std::filesystem;


    std::string make_str_lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }


    class Button {

    public:
        void init(const std::string& label) {
            ui_ = ftxui::Button(label, [this]() {
                if (on_click_)
                    on_click_();
            });
        }

        ftxui::Component ui_;
        std::function<void()> on_click_;
    };


    class Widget {

    public:
        using btn_click_callback_t = std::function<void()>;

        Widget() : screen_(ftxui::ScreenInteractive::TerminalOutput()) {
            input_path_ui_ = ftxui::Input(&input_path_data_, "Image path");

            checkbox_recur_ui_ = ftxui::Checkbox(
                "Recursive", &checkbox_recur_data_
            );

            btn_add_.init("Add");
            btn_clear_.init("Clear");
            btn_start_.init("Start");

            components_ = ftxui::Container::Vertical({
                input_path_ui_,
                checkbox_recur_ui_,
                btn_add_.ui_,
                btn_clear_.ui_,
                btn_start_.ui_,
            });

            renderer_ = ftxui::Renderer(components_, [&] {
                return this->render_function();
            });
        }

        void start() { screen_.Loop(renderer_); }

        const std::string& get_input_path() const { return input_path_data_; }

        bool get_checkbox_recur() const { return checkbox_recur_data_; }

        void set_output_text(const std::string& text) { output_text_ = text; }

        Button btn_add_;
        Button btn_clear_;
        Button btn_start_;

    private:
        ftxui::Element render_function() {
            ftxui::Elements elements;
            elements.push_back(
                ftxui::hbox(ftxui::text(" Path : "), input_path_ui_->Render())
            );
            elements.push_back(ftxui::separator());
            elements.push_back(checkbox_recur_ui_->Render());
            elements.push_back(ftxui::hbox(
                btn_add_.ui_->Render() | ftxui::xflex,
                btn_clear_.ui_->Render() | ftxui::xflex
            ));
            elements.push_back(ftxui::text(output_text_));
            elements.push_back(ftxui::separator());
            elements.push_back(btn_start_.ui_->Render());

            return ftxui::vbox(elements) | ftxui::border;
        }

        std::string input_path_data_;
        ftxui::Component input_path_ui_;

        bool checkbox_recur_data_ = false;
        ftxui::Component checkbox_recur_ui_;

        std::string output_text_;

        ftxui::Component components_;
        ftxui::Component renderer_;
        ftxui::ScreenInteractive screen_;
    };


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
    ::Widget widget;
    sung::FileList file_list;

    widget.set_output_text(file_list.make_text());

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

    widget.btn_add_.on_click_ = [&]() {
        const auto path_str = ::strip_quotes(widget.get_input_path());
        const auto path = fs::u8path(path_str);
        file_list.add(path, widget.get_checkbox_recur());
        widget.set_output_text(file_list.make_text());
    };

    widget.btn_clear_.on_click_ = [&]() {
        file_list.clear();
        widget.set_output_text(file_list.make_text());
    };

    widget.btn_start_.on_click_ = [&]() {
        for (const auto& path : file_list.get_files()) {
            const auto result = ::do_work(path);
            break;
        }

        widget.set_output_text("All files processed successfully");
    };

    widget.start();
    return 0;
}
