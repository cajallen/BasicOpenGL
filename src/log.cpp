#include "log.h"

void Log(string s) {
    log_strings.insert(log_strings.begin(), s);
    if (log_strings.size() > 50) {
        log_strings.pop_back();
    }
}

void ShowLog() {
    ImGui::Begin("Log"); {
        for (string s : log_strings) {
            ImGui::Text("%s", s.c_str());
        }
    }
    ImGui::End();
}