#pragma once

#include <portable-file-dialogs.h>
#include <string>
#include <vector>

class Dialog {
public:
    static std::string openFile(const std::vector<std::string>& filters = {"All Files", "*"}) {
        pfd::open_file fileDialog("Select a file", ".", filters);
        auto result = fileDialog.result();
        if (!result.empty()) {
            return result[0];
        }
        return "";
    }

    static std::string saveFile(const std::vector<std::string>& filters = {"All Files", "*"}) {
        pfd::save_file fileDialog("Save file", ".", filters);
        return fileDialog.result();
    }

    static std::string selectFolder() {
        pfd::select_folder folderDialog("Select a folder", ".");
        return folderDialog.result();
    }
};