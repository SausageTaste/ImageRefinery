#include "sung/imgref/filesys.hpp"

#include <filesystem>
#include <functional>
#include <set>

#include <fmt/core.h>
#include <uni_algo/norm.h>
#include <sung/general/stringtool.hpp>


namespace {

    sung::fs::path get_deepest_folder(const sung::fs::path& path) {
        if (sung::fs::is_directory(path))
            return path;
        else
            return get_deepest_folder(path.parent_path());
    }

    std::string make_str_lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

}  // namespace


// AllowedExtFileFilter
namespace sung {

    bool AllowedExtFileFilter::operator()(const fs::path& path) const {
        auto ext = make_utf8_str(path.extension().u8string());
        ext = ::make_str_lower(ext);
        return allowed_exts_.find(ext) != allowed_exts_.end();
    }

    void AllowedExtFileFilter::add_allowed_ext(std::string ext) {
        ext = ::make_str_lower(ext);
        if (!ext.starts_with("."))
            ext = "." + ext;

        allowed_exts_.insert(ext);
    }

}  // namespace sung


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
            files_.insert(fs::canonical(path));
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
                files_.insert(fs::canonical(e.path()));
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
                files_.insert(fs::canonical(e.path()));
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
        if (path.empty())
            return;

        const auto parent = path.parent_path();
        if (!fs::exists(parent))
            create_folder(parent);

        fs::create_directories(path);
    }

    fs::path replace_ext(const fs::path& path, const fs::path& new_ext) {
        auto new_path = path;
        new_path.replace_extension(new_ext);
        return new_path;
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


// FilePathMap
namespace sung {

    FilePathMap::FilePathMap(const fs::path& src) : src_(src) {}

    const fs::path& FilePathMap::add_with_suffix(
        const std::string& suffix, const sung::ExternalResultLoc& out_loc
    ) {
        auto file_name_ext = src_.stem();
        file_name_ext += "_";
        file_name_ext += suffix;
        file_name_ext = sung::normalize_utf8_path(file_name_ext);
        file_name_ext = src_.parent_path() / file_name_ext;

        const auto out_path = out_loc.get_path_for(file_name_ext);
        return files_.insert(out_path).first.operator*();
    }

    const fs::path* FilePathMap::select_best() const {
        if (files_.size() == 1)
            return &(*files_.begin());

        return nullptr;
    }

    sung::Expected<fs::path, std::string> FilePathMap::replace_src() const {
        auto sel = this->select_best();
        if (!sel)
            return sung::unexpected("No best file selected");

        const auto new_path = sung::replace_ext(src_, sel->extension());

        if (!fs::remove(src_))
            return sung::unexpected("Failed to remove old file");

        try {
            fs::rename(*sel, new_path);
        } catch (const std::exception& e) {
            return sung::unexpected(fmt::format(
                "Failed to move file ('{}' -> '{}'): '{}'",
                sung::make_utf8_str(*sel),
                sung::make_utf8_str(new_path),
                e.what()
            ));
        }

        return new_path;
    }

}  // namespace sung
