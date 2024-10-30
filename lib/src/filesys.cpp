#include "sung/imgref/filesys.hpp"

#include <filesystem>
#include <functional>
#include <set>

#include <fmt/core.h>
#include <uni_algo/norm.h>


namespace {

    sung::fs::path get_deepest_folder(const sung::fs::path& path) {
        if (sung::fs::is_directory(path))
            return path;
        else
            return get_deepest_folder(path.parent_path());
    }

}  // namespace


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

    fs::path FileList::get_longest_common_prefix() const {
        if (files_.empty())
            return {};

        auto it = files_.begin();
        auto prefix = *it;
        for (++it; it != files_.end(); ++it) {
            const auto& path = *it;
            const auto& prefix_str = prefix.u8string();
            const auto& path_str = path.u8string();
            const auto len = std::min(prefix_str.size(), path_str.size());
            size_t i = 0;
            for (; i < len; ++i) {
                if (prefix_str[i] != path_str[i])
                    break;
            }
            prefix = fs::path(prefix_str.substr(0, i));
        }

        return ::get_deepest_folder(prefix);
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


// ExternalResultLoc
namespace sung {

    ExternalResultLoc::ExternalResultLoc(
        const fs::path& input_root, const fs::path& output_dir
    )
        : input_root_(input_root), output_dir_(output_dir) {}

    fs::path ExternalResultLoc::get_path_for(const fs::path& path) const {
        const auto rel_path = path.lexically_relative(input_root_);
        const auto out = output_dir_ / rel_path;
        return out;
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

    void create_folder(const fs::path& path) {
        const auto parent = path.parent_path();
        if (!fs::exists(parent))
            create_folder(parent);

        fs::create_directories(path);
    }

    std::optional<fs::path> make_fol_path_with_suffix(const fs::path& path) {
        if (!fs::exists(path))
            return path;

        for (int i = 1; i < 1000; ++i) {
            auto new_path = path;
            new_path += fmt::format("_{:0>3}", i);
            if (!fs::exists(new_path))
                return new_path;
        }

        return path;
    }

}  // namespace sung
