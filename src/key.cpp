#include "key.h"


float lerp (float from, float to, float amount) {
	return from * (1-amount) + to * amount;
}

void Key::Update(float delta) {
	rotation += delta * rotation_speed;
}

void Key::Draw() {
	vec3 amb_col = GetKeyColor(col, AMBIENT);
	GLint amb_loc = glGetUniformLocation(ds_models.shader, "ambient_col");
	glUniform3fv(amb_loc, 1, &amb_col.x);
	
	vec3 dif_col = GetKeyColor(col, DIFFUSE);
	GLint dif_loc = glGetUniformLocation(ds_models.shader, "diffuse_col");
	glUniform3fv(dif_loc, 1, &dif_col.x);

	vec3 spc_col = GetKeyColor(col, SPECULAR);
	GLint spc_loc = glGetUniformLocation(ds_models.shader, "specular_col");
	glUniform3fv(spc_loc, 1, &spc_col.x);

	float phong = GetKeyPhong(col);
	GLint phong_loc = glGetUniformLocation(ds_models.shader, "phong");
	glUniform1f(phong_loc, phong);

	int special = DoKeySpecial(col);
	GLint special_loc = glGetUniformLocation(ds_models.shader, "do_special_specular");
	glUniform1i(special_loc, special);

	glm::mat4 model = glm::mat4(1);
	model = glm::translate(model, (glm::vec3) pos);
	model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, PI / 2.f, glm::vec3(0.0f, 1.0f, 0.0f));
	GLint uniModel = glGetUniformLocation(ds_models.shader, "model");
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLES, models["key"].start_pos, models["key"].size);
}

void Door::Open() {
	opened_at = seconds_alive;
}

bool Door::IsOpen() {
	return opened_at != 0;
}

void Door::Update(float delta) {
	seconds_alive += delta;
}

void Door::Draw() {
	vec3 amb_col = GetKeyColor(col, AMBIENT);
	GLint amb_loc = glGetUniformLocation(ds_models.shader, "ambient_col");
	glUniform3fv(amb_loc, 1, &amb_col.x);

	vec3 dif_col = GetKeyColor(col, DIFFUSE);
	GLint dif_loc = glGetUniformLocation(ds_models.shader, "diffuse_col");
	glUniform3fv(dif_loc, 1, &dif_col.x);

	vec3 spc_col = GetKeyColor(col, SPECULAR);
	GLint spc_loc = glGetUniformLocation(ds_models.shader, "specular_col");
	glUniform3fv(spc_loc, 1, &spc_col.x);

	float phong = GetKeyPhong(col);
	GLint phong_loc = glGetUniformLocation(ds_models.shader, "phong");
	glUniform1f(phong_loc, phong);

	int special = DoKeySpecial(col);
	GLint special_loc = glGetUniformLocation(ds_models.shader, "do_special_specular");
	glUniform1i(special_loc, special);

	glm::mat4 model = glm::mat4(1);
	model = glm::translate(model, (glm::vec3) pos);
	float opened_amount = opened_at == 0.0 ? 0.0 : fclamp(0.5 * (seconds_alive - opened_at), 0, 1);
	model = glm::translate(model, glm::vec3(-.5, 0, lerp(-.5, .0, opened_amount)));
	model = glm::rotate(model, PI / 2.f, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, 3 * PI / 2.f, glm::vec3(0.0f, 1.0f, 0.0f));
	GLint uniModel = glGetUniformLocation(ds_models.shader, "model");
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLES, models["door"].start_pos, models["door"].size);
}

void Goal::Update(float delta) {
	seconds_alive += delta;
}

void Goal::Draw() {
	vec3 amb_col = vec3(0.4, 0.38, 0.35);
	GLint amb_loc = glGetUniformLocation(ds_models.shader, "ambient_col");
	glUniform3fv(amb_loc, 1, &amb_col.x);

	vec3 dif_col = vec3(0.6, 0.58, 0.52);
	GLint dif_loc = glGetUniformLocation(ds_models.shader, "diffuse_col");
	glUniform3fv(dif_loc, 1, &dif_col.x);

	vec3 spc_col = vec3(0.3, 0.28, 0.25);
	GLint spc_loc = glGetUniformLocation(ds_models.shader, "specular_col");
	glUniform3fv(spc_loc, 1, &spc_col.x);

	float phong = 3;
	GLint phong_loc = glGetUniformLocation(ds_models.shader, "phong");
	glUniform1f(phong_loc, phong);

	int special = false;
	GLint special_loc = glGetUniformLocation(ds_models.shader, "do_special_specular");
	glUniform1i(special_loc, special);

	glm::mat4 model = glm::mat4(1);
	model = glm::translate(model, (glm::vec3) pos);
	//model = glm::translate(model, glm::vec3(-.5, 0, lerp(-.5, .0, opened_amount)));
	//model = glm::rotate(model, rotation_speed * seconds_alive, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, PI / 2.f, glm::vec3(1.f, 0.f, 0.f));
	//model = glm::rotate(model, PI / 2.f, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
	GLint uniModel = glGetUniformLocation(ds_models.shader, "model");
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLES, models["goal"].start_pos, models["goal"].size);
}