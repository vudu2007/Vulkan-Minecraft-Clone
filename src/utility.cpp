#include "utility.hpp"

#include <fstream>

std::vector<char> VmcUtility::readFile(const std::string& filename)
{
    // Read at the end and as a binary.
    // Starting at the end has a benefit of telling us the size.
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);

    // Return the beginning of the file.
    file.seekg(0);
    file.read(buffer.data(), file_size);

    // Finish the read.
    file.close();

    return buffer;
}

std::filesystem::path VmcUtility::getAssetPath(const std::string& filename)
{
    if (!std::filesystem::exists(ASSETS_PATH / filename))
    {
        throw std::runtime_error("failed to get asset: (" + filename + ")!");
    }

    return ASSETS_PATH / filename;
}
