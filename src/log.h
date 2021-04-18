#pragma once

// Log is primarily it's own file so dependencies can include it without circular inclusion problems.

#include <SDL.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <glad/glad.h>
#include <string>
#include <vector>

using namespace std;

static inline vector<string> log_strings;

void Log(string s);
void ShowLog();