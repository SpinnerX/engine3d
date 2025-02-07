#pragma once
#include <string>

namespace engine3d::filesystem{
    /**
     * @param filter
     * @note Defines the filter of items to pick from the file dialog
     * @example passing "obj;jpg" will only allow to select these files with those extensions
    */
    std::string LoadFromFileDialog(const std::string& p_Filter);
};