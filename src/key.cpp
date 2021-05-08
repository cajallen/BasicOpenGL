#include "key.h"


float lerp (float from, float to, float amount) {
	return from * (1-amount) + to * amount;
}


void Entity::Update(float delta) {
	lifetime += delta;
}

void Entity::Draw(DrawSet ds) {
	Entity::UpdateUniforms(ds);
	glDrawArrays(GL_TRIANGLES, models[model_name].start_pos, models[model_name].size);
}

void Entity::UpdateUniforms(DrawSet ds) {
	GLuint amb_loc = glGetUniformLocation(ds.shader, "ambient_col");
	if (amb_loc != -1)
		glUniform3fv(amb_loc, 1, &amb_tint.x);
	
	GLuint dif_loc = glGetUniformLocation(ds.shader, "diffuse_col");
	if (dif_loc != -1)
		glUniform3fv(dif_loc, 1, &diff_tint.x);

	GLuint spc_loc = glGetUniformLocation(ds.shader, "specular_col");
	if (spc_loc != -1)
		glUniform3fv(spc_loc, 1, &spec_tint.x);

	GLuint phong_loc = glGetUniformLocation(ds.shader, "phong");
	if (phong_loc != -1)
		glUniform1f(phong_loc, phong);

	GLuint model_loc = glGetUniformLocation(ds.shader, "model");
	if (model_loc != -1)
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
}

Key::Key(vec3 pos, KEY_COLOR col, float rot_spd, float wid) : Entity(pos), color(col), rotation_speed(rot_spd), width(wid) {
	amb_tint = GetKeyColor(col, AMBIENT);
	diff_tint = GetKeyColor(col, DIFFUSE);
	spec_tint = GetKeyColor(col, SPECULAR);
	phong = GetKeyPhong(col);
	model_name = "key";
}

Door::Door(vec3 pos, KEY_COLOR col) : Entity(pos), color(col) {
	amb_tint = GetKeyColor(col, AMBIENT);
	diff_tint = GetKeyColor(col, DIFFUSE);
	spec_tint = GetKeyColor(col, SPECULAR);
	phong = GetKeyPhong(col);
	model_name = "door";
}
Goal::Goal(vec3 pos, float rot_spd) : Entity(pos), rotation_speed(rot_spd) {
	model_name = "goal";
}

void Key::Update(float delta) {
	Entity::Update(delta);
	rotation += delta * rotation_speed;
	model = glm::translate(glm::mat4(1), (glm::vec3) position);
	model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, PI / 2.f, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Door::Update(float delta) {
	Entity::Update(delta);
	model = glm::translate(glm::mat4(1), (glm::vec3) position);
	float opened_amount = opened_at == 0.0 ? 0.0 : fclamp(0.5 * (lifetime - opened_at), 0, 1);
	model = glm::translate(model, glm::vec3(0, 0, lerp(-0.25, 0.25, opened_amount)));
	model = glm::rotate(model, -PI/2, glm::vec3(0.f, 0.f, 1.f));
}

void Goal::Update(float delta) {
	Entity::Update(delta);
	rotation += delta * rotation_speed;
	model = glm::translate(model, (glm::vec3) position);
	model = glm::rotate(model, rotation, glm::vec3(1.f, 0.f, 0.f));
	model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
}

void Door::Open() {
	opened_at = lifetime;
}

bool Door::IsOpen() {
	return opened_at != 0;
}


