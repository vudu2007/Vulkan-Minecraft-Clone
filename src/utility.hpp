#ifndef VMC_SRC_UTILITY_HPP
#define VMC_SRC_UTILITY_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace
{

std::filesystem::path getAssetsDir()
{
    int depth = 0;
    const int max_depth = 100; // Arbitrary value.
    auto curr_path = std::filesystem::current_path();

    while (depth++ < max_depth)
    {
        if (std::filesystem::exists(curr_path / "assets"))
        {
            return curr_path / "assets";
        }

        if (!curr_path.has_parent_path())
        {
            throw std::runtime_error("failed to find assets directory!");
        }

        curr_path = curr_path.parent_path();
    }

    throw std::runtime_error("failed to find assets directory due to exceeding max depth!");
}

std::filesystem::path ASSETS_PATH = getAssetsDir();

} // namespace

namespace VmcUtility
{

std::vector<char> readFile(const std::string& filename);
std::filesystem::path getAssetPath(const std::string& filename);

} // namespace VmcUtility

#endif // VMC_SRC_UTILITY_HPP
