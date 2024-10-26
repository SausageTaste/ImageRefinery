#include "sung/imgref/filesys.hpp"

#include <filesystem>
#include <functional>
#include <set>

#include <fmt/core.h>
#include <uni_algo/norm.h>


// FileList
namespace sung {

    FileList::FileList()
        : file_filter_([&](fs::path path) { return true; })
        , folder_filter_([&](fs::path path) { return true; }) {}

    void FileList::clear() { files_.clear(); }

    void FileList::add(const fs::path& path, bool recursive) {
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

    const std::set<fs::path>& FileList::get_files() const { return files_; }

    std::string FileList::make_text() const {
        return fmt::format(
            "{} files in {} locations",
            this->get_files().size(),
            this->make_locations().size()
        );
    }

    std::set<fs::path> FileList::make_locations() const {
        std::set<fs::path> out;
        for (const auto& x : files_) out.insert(x.parent_path());
        return out;
    }

    bool FileList::is_valid_file(const fs::directory_entry& entry) const {
        return entry.is_regular_file() && file_filter_(entry.path());
    }

    bool FileList::is_valid_file(const fs::path& path) const {
        return fs::is_regular_file(path) && file_filter_(path);
    }

    void FileList::add_dir(const fs::path& path) {
        if (!folder_filter_(path))
            return;

        for (const auto& e : fs::directory_iterator(path)) {
            if (this->is_valid_file(e)) {
                files_.insert(e.path());
            }
        }
    }

    void FileList::add_dir_recur(const fs::path& path) {
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

}  // namespace sung


// Free functions
namespace sung {

    std::string make_utf8_str(const fs::path& path) {
        const auto path_str = path.u8string();
        return std::string(
            reinterpret_cast<const char*>(path_str.c_str()), path_str.size()
        );
    }

    std::string normalize_utf8_str(const std::string& str) {
        return una::norm::to_nfc_utf8(str);
    }

    std::u8string normalize_utf8_str(const std::u8string& str) {
        return una::norm::to_nfc_utf8(str);
    }

    fs::path normalize_utf8_path(const fs::path& path) {
        return una::norm::to_nfc_utf8(path.u8string());
    }

}  // namespace sung
