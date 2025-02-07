#include <core/filesystem/file_dialog.hpp>
#include <engine3d-nfd/nfd.h>
#include <core/engine_logger.hpp>

namespace engine3d::filesystem{
    std::string LoadFromFileDialog(const std::string& p_Filter){
        char* output_path = nullptr;

        auto result = NFD_OpenDialog(p_Filter.c_str(), nullptr, &output_path);

        if(result == NFD_OKAY){
            return std::string(output_path);
        }

        ConsoleLogFatal("Error Loading File!");
        return "";
    }
};