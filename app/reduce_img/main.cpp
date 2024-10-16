#include <filesystem>
#include <set>
#include <vector>

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <fmt/core.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>


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


    class FileList {

    public:
        FileList()
            : file_filter_([&](fs::path path) { return true; })
            , folder_filter_([&](fs::path path) { return true; }) {}

        void clear() { files_.clear(); }

        void add(const fs::path& path, bool recursive) {
            if (fs::is_directory(path)) {
                if (recursive) {
                    this->add_dir_recur(path);
                } else {
                    this->add_dir(path);
                }
            } else if (this->is_valid_file(path)) {
                files_.insert(path);
            }
        }

        const std::set<fs::path>& get_files() const { return files_; }

        std::string make_text() const {
            return fmt::format(
                "{} files in {} locations",
                this->get_files().size(),
                this->make_locations().size()
            );
        }

        std::set<fs::path> make_locations() const {
            std::set<fs::path> out;
            for (const auto& x : files_) out.insert(x.parent_path());
            return out;
        }

        std::function<bool(fs::path)> file_filter_;
        std::function<bool(fs::path)> folder_filter_;

    private:
        bool is_valid_file(const fs::directory_entry& entry) const {
            return entry.is_regular_file() && file_filter_(entry.path());
        }

        bool is_valid_file(const fs::path& path) const {
            return fs::is_regular_file(path) && file_filter_(path);
        }

        void add_dir(const fs::path& path) {
            if (!folder_filter_(path))
                return;

            for (const auto& e : fs::directory_iterator(path)) {
                if (this->is_valid_file(e)) {
                    files_.insert(e.path());
                }
            }
        }

        void add_dir_recur(const fs::path& path) {
            if (!folder_filter_(path))
                return;

            for (const auto& e : fs::directory_iterator(path)) {
                if (e.is_directory()) {
                    this->add_dir_recur(e.path());
                } else if (this->is_valid_file(e)) {
                    files_.insert(e.path());
                }
            }
        }

        std::set<fs::path> files_;
    };

}  // namespace


int main() {
    ::Widget widget;
    ::FileList file_list;

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

    widget.start();
    return 0;
}
