#ifndef _RAYTRACER_IMGUI_EXTRA_H
#define _RAYTRACER_IMGUI_EXTRA_H

#include "imgui.h"

namespace ImGui {

inline bool DragDoubleN(const char* label, void* p_data, int components, float v_speed = 0.1, 
	const double p_min = 0.0, const double p_max = 0.0, const char* format = "%.2f", ImGuiSliderFlags flags = 0) {
	return ImGui::DragScalarN(label, ImGuiDataType_Double, p_data, components, v_speed, &p_min, &p_max, format, flags);
}

}

#endif