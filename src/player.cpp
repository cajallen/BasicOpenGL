#include "player.h"

void Player::handle_input(SDL_Event* event) {
	auto io = ImGui::GetIO();
	if (io.WantCaptureMouse) using_mouse = false;
	if (!io.WantCaptureMouse && (event->type == SDL_MOUSEMOTION) && using_mouse && !warped) {
		int s_wid, s_hei;
		SDL_GetWindowSize(window, &s_wid, &s_hei);
		mouse_diff_x = (s_wid / 2) - event->motion.x;
		mouse_diff_y = (s_hei / 2) - event->motion.y;
		if (mouse_diff_x != 0 || mouse_diff_y != 0) {
			SDL_WarpMouseInWindow(window, s_wid / 2, s_hei / 2);
		}
	}
	if ((!io.WantCaptureKeyboard && event->type == SDL_KEYDOWN) || event->type == SDL_KEYUP) {	 // handle wasd input
		bool key_down = event->type == SDL_KEYDOWN;
		char key = event->key.keysym.sym;
		if (key == SDLK_SPACE)
			wasd_input[UP_INDEX] = key_down ? 1 : 0;
		if (key == SDLK_x)
			wasd_input[DOWN_INDEX] = key_down ? 1 : 0;
		if (key == SDLK_w)
			wasd_input[FORWARD_INDEX] = key_down ? 1 : 0;
		if (key == SDLK_s)
			wasd_input[BACKWARD_INDEX] = key_down ? 1 : 0;
		if (key == SDLK_d)
			wasd_input[RIGHT_INDEX] = key_down ? 1 : 0;
		if (key == SDLK_a)
			wasd_input[LEFT_INDEX] = key_down ? 1 : 0;
		if (key == SDLK_q && !key_down) {
			int s_wid, s_hei;
			SDL_GetWindowSize(window, &s_wid, &s_hei);
			using_mouse = !using_mouse;
			if (using_mouse) {
				SDL_WarpMouseInWindow(window, s_wid / 2, s_hei / 2);  // avoid jump
				warped = true;
			}
		}
	}
}


// updates position and camera direction
void Player::update(float delta) {
	move(delta);

	if (using_mouse)
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);

	float yaw = look_dir.yaw();
	float pitch = look_dir.pitch();

	yaw += (pan_speed * mouse_diff_x);
	pitch += (pan_speed * mouse_diff_y);

	ImGui::Begin("Log");
	yaw *= RAD2DEG;
	pitch *= RAD2DEG;

	pitch = fclamp(pitch, -89.5, 89.5);

	if (ImGui::TreeNode("Player")) {
		ImGui::SetNextItemWidth(-ImGui::CalcTextSize("Position").x - ImGui::GetStyle().WindowPadding.x);
		ImGui::DragFloat3("Position##Player", &logic_pos.x, 0.05);
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() / 2.0 - ImGui::CalcTextSize("Yaw").x);
		ImGui::DragFloat("Yaw##Player", &yaw, 0.2);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-ImGui::CalcTextSize("Pitch").x - ImGui::GetStyle().WindowPadding.x);
		ImGui::DragFloat("Pitch##Player", &pitch, 0.2, -89.5, 89.5);
		ImGui::DragFloat("Camera Speed", &pan_speed, 0.0001, 0.0005, 0.05, "%.4f");
		ImGui::DragFloat("Move Speed", &noclip_speed, 0.05);
		ImGui::TreePop();
	}
	yaw *= DEG2RAD;
	pitch *= DEG2RAD;
	ImGui::End();

	look_dir = YawPitch(yaw, pitch);

	mouse_diff_x = 0;
	mouse_diff_y = 0;
	warped = false;
}

void Player::move(float delta) {
	vec3 up = vec3(0, 0, 1);
	vec3 forward = look_dir.normalized();
	vec3 right = cross(look_dir, vec3(0, 0, 1)).normalized();
	int forward_input = wasd_input[FORWARD_INDEX] ? 1 : 0;
	int right_input = wasd_input[RIGHT_INDEX] ? 1 : 0;
	int up_input = wasd_input[UP_INDEX] ? 1 : 0;
	forward_input += wasd_input[BACKWARD_INDEX] ? -1 : 0;
	right_input += wasd_input[LEFT_INDEX] ? -1 : 0;
	up_input += wasd_input[DOWN_INDEX] ? -1 : 0;
	logic_pos += up * noclip_speed * delta * up_input;
	logic_pos += forward * noclip_speed * delta * forward_input;
	logic_pos += right * noclip_speed * delta * right_input;

	camera_pos = logic_pos;
}

glm::mat4 Player::get_view_matrix() const {
	vec3 look_at = camera_pos + look_dir;
	glm::mat4 view = glm::lookAt((glm::vec3)camera_pos, (glm::vec3)look_at, glm::vec3(0.0f, 0.0f, 1.0f));
	return view;
}
