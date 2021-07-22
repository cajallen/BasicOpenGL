#include "main.h"


namespace caj {

bool fullscreen = false;
float screen_width = 1900;
float screen_height = 1000;
SDL_Window* window;
ImGuiIO* io;

// not extern
const char* glsl_version = "#version 330";
SDL_GLContext context;

vec3 light_dir = vec3(-1,-1,-2).normalized();
Player player;

void initialize_environment() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        IM_ASSERT("SDL_Init failed" && 0);

    // Ask SDL to get a recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // Create a window (offsetx, offsety, width, height, flags)
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
	window = SDL_CreateWindow("Basic OpenGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, window_flags);
	player.window = window;

    // Create a context to draw in
    context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
        IM_ASSERT("gladLoadGLLoader failed" && 0);
}

void initialize_imgui() {
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init(glsl_version);
}


// vertex_member_flags should contain the index of the members
// in Vertex that will be used in the shader. The shader should 
// layout parallel to Vertex, even if that skips an index.
void initialize_shader(Shader* shader, int vertex_member_flags) {
	glUseProgram(shader->program);
	glGenVertexArrays(1, &shader->vao);
	glBindVertexArray(shader->vao);

	for (auto& [id, uni] : shader->uniforms) {
		uni = glGetUniformLocation(shader->program, id.c_str());
	}

	for (int i = 0; i < sizeof(Vertex) / sizeof(vec3); i++) {
		if (vertex_member_flags >> i | 0b1) {
			glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(i * sizeof(vec3)));
			glEnableVertexAttribArray(i);
		}
	}

	shader->initialization(shader);
}


void initialize_depthmaps(GLuint* framebuffer, GLuint* depthmap_tex) {
	glActiveTexture(GL_TEXTURE1);

	glGenFramebuffers(1, framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);

	glGenTextures(1, depthmap_tex);
	glBindTexture(GL_TEXTURE_2D, *depthmap_tex); // TODO: this is permanently bound on texture unit 1 for now...
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depthmap_tex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	IM_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
}


// This function is modified from opengl-tutorial.org
GLuint load_program(const char* vertex_file_path, const char* fragment_file_path) {
	// Create the shaders
	GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_id = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	string vertex_source;
	ifstream vertex_stream(vertex_file_path, ios::in);
	if (vertex_stream.is_open()) {
		stringstream sstr;
		sstr << vertex_stream.rdbuf();
		vertex_source = sstr.str();
		vertex_stream.close();
	}
	else {
		log(format("Failed to open {}\n", vertex_file_path));
	}

	// Read the Fragment Shader code from the file
	string fragment_source;
	ifstream fragment_stream(fragment_file_path, ios::in);
	if (fragment_stream.is_open()) {
		stringstream sstr;
		sstr << fragment_stream.rdbuf();
		fragment_source = sstr.str();
		fragment_stream.close();
	}

	GLint Result = GL_FALSE;
	int err_length;
	// Compile Vertex Shader
	char const * vertex_source_ptr = vertex_source.c_str();
	glShaderSource(vertex_id, 1, &vertex_source_ptr , NULL);
	glCompileShader(vertex_id);
	// Check Vertex Shader
	glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(vertex_id, GL_INFO_LOG_LENGTH, &err_length);
	if (err_length > 0) {
		vector<char> vert_err(err_length+1);
		glGetShaderInfoLog(vertex_id, err_length, NULL, &vert_err[0]);
		log(format("{}\n", &vert_err[0]));
	}
	// Compile Fragment Shader
	char const * fragment_source_ptr = fragment_source.c_str();
	glShaderSource(fragment_id, 1, &fragment_source_ptr , NULL);
	glCompileShader(fragment_id);
	// Check Fragment Shader
	glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(fragment_id, GL_INFO_LOG_LENGTH, &err_length);
	if (err_length > 0) {
		vector<char> frag_err(err_length+1);
		glGetShaderInfoLog(fragment_id, err_length, NULL, &frag_err[0]);
		log(format("{}\n", &frag_err[0]));
	}
	// Link the program
	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);
	glLinkProgram(program_id);
	// Check the program
	glGetProgramiv(program_id, GL_LINK_STATUS, &Result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &err_length);
	if (err_length > 0) {
		vector<char> shader_err(err_length+1);
		glGetProgramInfoLog(program_id, err_length, NULL, &shader_err[0]);
		log(format("{}\n", &shader_err[0]));
	}

	glDetachShader(program_id, vertex_id);
	glDetachShader(program_id, fragment_id);
	
	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);

	return program_id;
}

bool load_atlas(vector<string> file_names, GLuint* out_texture) {
	struct TempImage {
		string file_name;
		int wid, hei;
		unsigned char* data;

		TempImage(string fn) : file_name(fn) {
            data = stbi_load(fn.c_str(), &wid, &hei, NULL, 4);
			if (data == NULL) { log(fn + " failed to load"); }
		}
	};
	vector<TempImage> images;
	int size = 0;
	for (string file_name : file_names) {
		images.push_back(TempImage(file_name));
		size = max(size, images.back().wid);
		size = max(size, images.back().hei);
	}
	// Create a OpenGL texture identifier
    GLuint image_texture;

    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, image_texture);
	// resize the texture
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, size, size, file_names.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	// load in the individual layers
    int current_z = 0;
    for (TempImage& img : images) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, current_z, img.wid, img.hei, 1, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
        current_z++;
        stbi_image_free(img.data);
    }

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    *out_texture = image_texture;
    return true;
}


bool draw_light_dir_widget() {
	float yaw = light_dir.yaw();
	float pitch = light_dir.pitch();

	bool changed = false;

	if (ImGui::TreeNode("Light")) {
		yaw *= RAD2DEG;
		pitch *= RAD2DEG;
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() / 2.0 - ImGui::CalcTextSize("Yaw").x);
		changed |= ImGui::DragFloat("Yaw##Light", &yaw, 0.2);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-ImGui::CalcTextSize("Pitch").x - ImGui::GetStyle().WindowPadding.x);
		changed |= ImGui::DragFloat("Pitch##Light", &pitch, 0.2, -175, -5);

		yaw *= DEG2RAD;
		pitch *= DEG2RAD;

		ImGui::TreePop();
	}

	light_dir = YawPitch(yaw, pitch);
	return changed;
}

void draw_object(Shader shader, ModelRange mr) {
    glUseProgram(shader.program);
	glBindVertexArray(shader.vao);

	shader.pre_object_render(&shader);
	shader.pre_render(&shader);
	// glDraw uses the vertex buffer that was bound when we enabled the attrib arrays...
	glDrawArrays(GL_TRIANGLES, mr.start_pos, mr.size);
	glBindVertexArray(0);
}

GLuint create_atlas(vector<string> file_names) {
	glActiveTexture(GL_TEXTURE0);
	GLuint tex_id;
	load_atlas(file_names, &tex_id);
	return tex_id;
}


bool handle_input() {
	SDL_Event windowEvent;
	while (SDL_PollEvent(&windowEvent)) {
		ImGui_ImplSDL2_ProcessEvent(&windowEvent);
		player.handle_input(&windowEvent);
		if (windowEvent.type == SDL_QUIT)
			return true;
		if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
			return true;
		if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f)
			fullscreen = !fullscreen;
		if (windowEvent.type == SDL_WINDOWEVENT_RESIZED) {
			screen_width = windowEvent.window.data1;
			screen_height = windowEvent.window.data2;
		}
		SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
	}
	return false;
}

namespace impl {


void main_initialization(Shader* shader) {
	glUniform1i(shader->uniforms["tex"], 0); // texture unit 0
	glUniform1i(shader->uniforms["heightmap_tex"], 0); // texture unit 0
	glUniform1i(shader->uniforms["depthmap_tex"], 1); // texture unit 1
}

void main_pre_object_render(Shader* shader) {
	vec3 ones(1,1,1);
	glm::mat4 one(1);

	if (shader->uniforms.count("ambient_col") == 1)
		glUniform3fv(shader->uniforms["ambient_col"], 1, &ones.x);
	
	if (shader->uniforms.count("specular_col") == 1)
		glUniform3fv(shader->uniforms["specular_col"], 1, &ones.x);
	
	if (shader->uniforms.count("diffuse_col") == 1)
		glUniform3fv(shader->uniforms["diffuse_col"], 1, &ones.x);

	if (shader->uniforms.count("phong") == 1)
		glUniform1f(shader->uniforms["phong"], 10.0);

	if (shader->uniforms.count("model") == 1)
		glUniformMatrix4fv(shader->uniforms["model"], 1, GL_FALSE, glm::value_ptr(one));
}


void main_pre_render(Shader* shader) {
	// We could optimize this slightly by moving these lighting components out 
	// to only update when the lighting is changed. 
	glm::vec3 light_inv_dir = glm::vec3(-light_dir.x, -light_dir.y, -light_dir.z);
	glm::mat4 depth_proj = glm::ortho(-12.f, 12.f, -12.f, 12.f, -8.f, 8.f);
	glm::mat4 depth_view = glm::lookAt(light_inv_dir, glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 depth_vp = depth_proj * depth_view;

	glm::mat4 bias(
		0.5, 0.0, 0.0, 0.0, 
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	glm::mat4 depth_bias_vp = bias * depth_vp;

	glUniform3fv(shader->uniforms["light_dir"], 1, &light_dir.x);
	glUniformMatrix4fv(shader->uniforms["db_vp"], 1, GL_FALSE, glm::value_ptr(depth_bias_vp));

	glm::mat4 view = player.get_view_matrix();
	glm::mat4 proj = glm::perspective(3.1416f / 3, screen_width/screen_height, 0.1f, 100.0f);
	glm::mat4 vp = proj*view;

	glUniformMatrix4fv(shader->uniforms["vp"], 1, GL_FALSE, glm::value_ptr(vp));
	glUniform3fv(shader->uniforms["view_pos"], 1, &player.camera_pos.x);

}


void depthmap_initialization(Shader* shader) {

}


void depthmap_pre_object_render(Shader* shader) {
	glm::mat4 one(1);

	if (shader->uniforms.count("model") == 1)
		glUniformMatrix4fv(shader->uniforms["model"], 1, GL_FALSE, glm::value_ptr(one));
}


void depthmap_pre_render(Shader* shader) {
	glm::vec3 light_inv_dir = glm::vec3(-light_dir.x, -light_dir.y, -light_dir.z);
	glm::mat4 depth_proj = glm::ortho(-12.f, 12.f, -12.f, 12.f, -8.f, 8.f);
	glm::mat4 depth_view = glm::lookAt(light_inv_dir, glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 depth_vp = depth_proj * depth_view;

	glUniformMatrix4fv(shader->uniforms["vp"], 1, GL_FALSE, glm::value_ptr(depth_vp));
}

void initialize_shaders(Shader* main_shader, Shader* depthmap_shader) {
	GLuint main_program = caj::load_program("src/shader/main.vert", "src/shader/main.frag");
	*main_shader = Shader{ main_program, 0,
		{{"tex", -1}, {"depthmap_tex", -1}, {"model", -1},
		{"vp", -1}, {"db_vp", -1}, {"view_pos", -1},
		{"light_dir", -1}, {"ambient_col", -1}, {"diffuse_col", -1},
		{"specular_col", -1}, {"phong", -1}, {"heightmap_tex", -1}},
		&main_initialization,
		&main_pre_render,
		&main_pre_object_render
	};
	GLuint depthmap_program = caj::load_program("src/shader/depth_rtt.vert", "src/shader/depth_rtt.frag");
	*depthmap_shader = Shader{depthmap_program, 0,
		{{"model", -1}, {"vp", -1}},
		&depthmap_initialization,
		&depthmap_pre_render,
		&depthmap_pre_object_render
	};
	caj::initialize_shader(main_shader, 0b1111);
	caj::initialize_shader(depthmap_shader, 0b0001);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initialize_geometry(GLuint vbo, ModelRange* model_range) {
	vector<Vertex> model_data = caj::generate_grid(255);
	model_range->start_pos = 0;
	model_range->size = model_data.size();

	glBufferData(GL_ARRAY_BUFFER, model_data.size() * sizeof(Vertex), model_data.data(), GL_STATIC_DRAW);
}

void draw_menus(GLuint depth_tex) {
	show_log();

	ImGui::Begin("ShadowMap");
	{
		ImTextureID imgui_shadowmap_id = ImTextureID(GLuint(depth_tex));
		{
            ImVec2 pos = ImGui::GetCursorScreenPos();
			ImVec2 size = ImGui::GetContentRegionAvail();
            ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
            ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
            ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
            ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
            ImGui::Image(imgui_shadowmap_id, size, uv_min, uv_max, tint_col, border_col);
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                float region_sz = 32.0f;
                float region_x = ImGui::GetIO().MousePos.x - pos.x - region_sz * 0.5f;
                float region_y = ImGui::GetIO().MousePos.y - pos.y - region_sz * 0.5f;
                float zoom = 4.0f;
                if (region_x < 0.0f) { region_x = 0.0f; }
                else if (region_x > size.x - region_sz) { region_x = size.x - region_sz; }
                if (region_y < 0.0f) { region_y = 0.0f; }
                else if (region_y > size.y - region_sz) { region_y = size.y - region_sz; }
                ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
                ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
                ImVec2 uv0 = ImVec2((region_x) / size.x, (region_y) / size.y);
                ImVec2 uv1 = ImVec2((region_x + region_sz) / size.x, (region_y + region_sz) / size.y);
                ImGui::Image(imgui_shadowmap_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
                ImGui::EndTooltip();
            }
        }
	}
	ImGui::End();
}


void draw_depthmaps(GLuint framebuffer, Shader shader, ModelRange model_range) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, 4096, 4096);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_object(shader, model_range);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_main(Shader shader, ModelRange model_range) {
	glViewport(0,0, screen_width, screen_height);
	glClearColor(0.4f, 0.5f, 0.6f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_object(shader, model_range);
}


}  // namespace impl
}  // namespace caj


int main(int argc, char* argv[]) {
	Shader main_shader, depthmap_shader;
	GLuint framebuffer, depthmap_tex, atlas_tex, vbo;
	ModelRange geometry;

	caj::initialize_environment();
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// fill buffer
	caj::impl::initialize_geometry(vbo, &geometry);
	// set up attributes
	caj::impl::initialize_shaders(&main_shader, &depthmap_shader);
	atlas_tex = caj::create_atlas({
		"assets/floor.png", "assets/floor_specular.png", "assets/floor_normal.png", "assets/floor_height.png"
	});
	caj::initialize_depthmaps(&framebuffer, &depthmap_tex);
	caj::initialize_imgui();

	static int last_ticks = SDL_GetTicks();

    bool quit = false;
    while (!quit) {
		quit = caj::handle_input();
		if (quit) break;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(caj::window);
		ImGui::NewFrame();

		// logic update
		caj::player.update((SDL_GetTicks() - last_ticks)/1000.0);
		last_ticks = SDL_GetTicks();

		caj::impl::draw_menus(depthmap_tex);

		caj::impl::draw_depthmaps(framebuffer, depthmap_shader, geometry);
		caj::impl::draw_main(main_shader, geometry);
		// TODO: how do we determine which objects to render 

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(caj::window);
	}
	SDL_Quit();
    return 0;
}