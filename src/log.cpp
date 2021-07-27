#include "log.h"

namespace caj {

void log(string s) {
    log_strings.insert(log_strings.begin(), s);
    if (log_strings.size() > 50) {
        log_strings.pop_back();
    }
}

void show_log() {
    for (string s : log_strings) {
        ImGui::Text("%s", s.c_str());
    }
}

}  // namespace caj
