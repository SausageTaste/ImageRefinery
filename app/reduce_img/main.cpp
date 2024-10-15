#include <filesystem>
#include <vector>

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <fmt/core.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>


namespace {

    class Widget {

    public:
        using btn_click_callback_t = std::function<void(Widget&)>;

        Widget() : screen_(ftxui::ScreenInteractive::TerminalOutput()) {
            input_path_ui_ = ftxui::Input(&input_path_data_, "Image path");

            btn_label_ = "Start";
            btn_ui_ = ftxui::Button(btn_label_, [this]() {
                if (btn_on_click_)
                    btn_on_click_(*this);
            });

            btn_clear_label_ = "Clear";
            btn_clear_ui_ = ftxui::Button(btn_clear_label_, [this]() {
                this->input_path_data_.clear();
                this->output_text_.clear();
            });

            components_ = ftxui::Container::Vertical({
                input_path_ui_,
                btn_ui_,
                btn_clear_ui_,
            });

            renderer_ = ftxui::Renderer(components_, [&] {
                return this->render_function();
            });
        }

        void start() { screen_.Loop(renderer_); }

        const std::string& get_input_path() const { return input_path_data_; }
        void set_input_path(const std::string& path) {
            input_path_data_ = path;
        }

        void set_output_text(const std::string& text) { output_text_ = text; }

        void set_callback_on_btn_click(btn_click_callback_t on_click) {
            btn_on_click_ = on_click;
        }

    private:
        ftxui::Element render_function() {
            ftxui::Elements elements;
            elements.push_back(
                ftxui::hbox(ftxui::text(" Path : "), input_path_ui_->Render())
            );
            elements.push_back(ftxui::hbox(
                btn_ui_->Render() | ftxui::xflex,
                btn_clear_ui_->Render() | ftxui::xflex
            ));
            elements.push_back(ftxui::separator());
            elements.push_back(ftxui::text(output_text_));

            return ftxui::vbox(elements) | ftxui::border;
        }

        std::string input_path_data_;
        ftxui::Component input_path_ui_;

        std::string btn_label_;
        btn_click_callback_t btn_on_click_;
        ftxui::Component btn_ui_;

        std::string btn_clear_label_;
        ftxui::Component btn_clear_ui_;

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

    std::string do_work(const std::filesystem::path& path) {
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

        // add suffix to the file name, before extension
        const auto resized_path =
            (path.parent_path() /
             (path.stem().string() + "_resized" + path.extension().string()));
        resized.write(resized_path.string());

        const auto median_filtered = OIIO::ImageBufAlgo::median_filter(resized);
        if (median_filtered.has_error())
            return median_filtered.geterror();

        const auto median_filtered_path =
            (path.parent_path() / (path.stem().string() + "_median_filtered" +
                                   path.extension().string()));
        median_filtered.write(median_filtered_path.string());

        return "success";
    }

}  // namespace


int main() {
    Widget widget;

    widget.set_callback_on_btn_click([](Widget& widget) {
        const auto path_str = ::strip_quotes(widget.get_input_path());
        const auto path = std::filesystem::u8path(path_str);
        const auto result = ::do_work(path);
        widget.set_output_text(result);
    });

    widget.start();
    return 0;


    return 0;
}
