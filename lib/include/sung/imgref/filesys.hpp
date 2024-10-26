#pragma once

#include <filesystem>
#include <functional>
#include <set>


namespace sung {

    namespace fs = std::filesystem;


    std::string make_utf8_str(const fs::path& path);

    std::string normalize_utf8_str(const std::string& str);
    std::u8string normalize_utf8_str(const std::u8string& str);
    fs::path normalize_utf8_path(const fs::path& path);


    class FileList {

    public:
        FileList();

        void clear();
        void add(const fs::path& path, bool recursive);

        const std::set<fs::path>& get_files() const;

        std::string make_text() const;
        std::set<fs::path> make_locations() const;

        std::function<bool(fs::path)> file_filter_;
        std::function<bool(fs::path)> folder_filter_;

    private:
        bool is_valid_file(const fs::directory_entry& entry) const;
        bool is_valid_file(const fs::path& path) const;

        void add_dir(const fs::path& path);
        void add_dir_recur(const fs::path& path);

        std::set<fs::path> files_;
    };

}  // namespace sung
