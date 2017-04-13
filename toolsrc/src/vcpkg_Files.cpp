#include "pch.h"
#include "vcpkg_Files.h"
#include "vcpkg_System.h"

namespace vcpkg::Files
{
    static const std::regex FILESYSTEM_INVALID_CHARACTERS_REGEX = std::regex(R"([\/:*?"<>|])");

    struct RealFilesystem : Filesystem
    {
        virtual Expected<std::string> read_contents(const fs::path& file_path) const override
        {
            std::fstream file_stream(file_path, std::ios_base::in | std::ios_base::binary);
            if (file_stream.fail())
            {
                return std::errc::no_such_file_or_directory;
            }

            file_stream.seekg(0, file_stream.end);
            auto length = file_stream.tellg();
            file_stream.seekg(0, file_stream.beg);

            if (length > SIZE_MAX)
            {
                return std::errc::file_too_large;
            }

            std::string output;
            output.resize(static_cast<size_t>(length));
            file_stream.read(&output[0], length);
            file_stream.close();

            return std::move(output);
        }
        virtual Expected<std::vector<std::string>> read_lines(const fs::path& file_path) const override
        {
            std::fstream file_stream(file_path, std::ios_base::in | std::ios_base::binary);
            if (file_stream.fail())
            {
                return std::errc::no_such_file_or_directory;
            }

            std::vector<std::string> output;
            std::string line;
            while (std::getline(file_stream, line))
            {
                output.push_back(line);
            }
            file_stream.close();

            return std::move(output);
        }
        virtual fs::path find_file_recursively_up(const fs::path & starting_dir, const std::string & filename) const override
        {
            fs::path current_dir = starting_dir;
            for (; !current_dir.empty(); current_dir = current_dir.parent_path())
            {
                const fs::path candidate = current_dir / filename;
                if (exists(candidate))
                {
                    break;
                }
            }

            return current_dir;
        }

        virtual std::vector<fs::path> get_files_recursive(const fs::path & dir) const override
        {
            std::vector<fs::path> ret;

            fs::stdfs::recursive_directory_iterator b(dir), e{};
            for (; b != e; ++b)
            {
                ret.push_back(b->path());
            }

            return ret;
        }
        virtual std::vector<fs::path> get_files_non_recursive(const fs::path & dir) const override
        {
            std::vector<fs::path> ret;

            fs::stdfs::directory_iterator b(dir), e{};
            for (; b != e; ++b)
            {
                ret.push_back(b->path());
            }

            return ret;
        }

        virtual void write_lines(const fs::path & file_path, const std::vector<std::string>& lines) override
        {
            std::fstream output(file_path, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            for (const std::string& line : lines)
            {
                output << line << "\n";
            }
            output.close();
        }

        virtual void rename(const fs::path & oldpath, const fs::path & newpath) override
        {
            fs::stdfs::rename(oldpath, newpath);
        }
        virtual bool remove(const fs::path & path) override
        {
            return fs::stdfs::remove(path);
        }
        virtual bool remove(const fs::path & path, std::error_code& ec) override
        {
            return fs::stdfs::remove(path, ec);
        }
        virtual std::uintmax_t remove_all(const fs::path & path, std::error_code& ec) override
        {
            return fs::stdfs::remove_all(path, ec);
        }
        virtual bool exists(const fs::path & path) const override
        {
            return fs::stdfs::exists(path);
        }
        virtual bool is_directory(const fs::path & path) const override
        {
            return fs::stdfs::is_directory(path);
        }
        virtual bool is_regular_file(const fs::path & path) const override
        {
            return fs::stdfs::is_regular_file(path);
        }
        virtual bool is_empty(const fs::path & path) const override
        {
            return fs::stdfs::is_empty(path);
        }
        virtual bool create_directory(const fs::path & path, std::error_code & ec) override
        {
            return fs::stdfs::create_directory(path, ec);
        }
        virtual void copy(const fs::path & oldpath, const fs::path & newpath, fs::copy_options opts) override
        {
            fs::stdfs::copy(oldpath, newpath, opts);
        }
        virtual bool copy_file(const fs::path & oldpath, const fs::path & newpath, fs::copy_options opts, std::error_code & ec) override
        {
            return fs::stdfs::copy_file(oldpath, newpath, opts, ec);
        }

        virtual fs::file_status status(const fs::path& path, std::error_code& ec) const override
        {
            return fs::stdfs::status(path, ec);
        }
        virtual void write_contents(const fs::path& file_path, const std::string& data) override
        {
            FILE* f = nullptr;
            auto ec = _wfopen_s(&f, file_path.native().c_str(), L"wb");
            Checks::check_exit(VCPKG_LINE_INFO, ec == 0);
            auto count = fwrite(data.data(), sizeof(data[0]), data.size(), f);
            fclose(f);

            Checks::check_exit(VCPKG_LINE_INFO, count == data.size());
        }
    };

    Filesystem & get_real_filesystem()
    {
        static RealFilesystem real_fs;
        return real_fs;
    }

    bool has_invalid_chars_for_filesystem(const std::string& s)
    {
        return std::regex_search(s, FILESYSTEM_INVALID_CHARACTERS_REGEX);
    }

    void print_paths(const std::vector<fs::path>& paths)
    {
        System::println("");
        for (const fs::path& p : paths)
        {
            System::println("    %s", p.generic_string());
        }
        System::println("");
    }
}
