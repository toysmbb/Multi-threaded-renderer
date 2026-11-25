#pragma once
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

inline std::filesystem::path FindShaderDir(const char* argv0)
{
    namespace fs = std::filesystem;

    std::vector<fs::path> candidates;
    if (argv0 && *argv0)
    {
        std::error_code ec;
        const fs::path exe = fs::weakly_canonical(fs::path(argv0), ec);
        if (!ec)
            candidates.push_back(exe.parent_path() / "shaders");
    }
    candidates.push_back(fs::current_path() / "shaders");

    for (const auto& dir : candidates)
        if (fs::exists(dir / "cube.vert"))
            return dir;

    std::string message = "Could not find the 'shaders' directory. Looked in:";
    for (const auto& dir : candidates)
        message += "\n  " + dir.string();
    throw std::runtime_error(message);
}
