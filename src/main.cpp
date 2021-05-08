#include "main.h"

bool fullscreen = false;
float screen_width = 1900;
float screen_height = 1000;
SDL_Window* window;
ImGuiIO* io;
Level* level;
DrawSet ds_main;
DrawSet ds_models;
DrawSet ds_shadows;
ModelRange model_world;
map<string, ModelRange> models;

// not extern
const char* glsl_version = "#version 330";

float aspect;
SDL_GLContext context;
vec3 light_dir(-1,-1,-2);


struct {
	GLuint model, vp, db_vp, view_pos, light_dir, amb, diff, spec, phong;
} main_shader_loc;

struct {
	GLuint model, vp;
} depth_shader_loc;

// This function is modified from opengl-tutorial.org
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
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
		Log(format("Failed to open {}\n", vertex_file_path));
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
		Log(format("{}\n", &vert_err[0]));
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
		Log(format("{}\n", &frag_err[0]));
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
		Log(format("{}\n", &shader_err[0]));
	}

	glDetachShader(program_id, vertex_id);
	glDetachShader(program_id, fragment_id);
	
	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);

	return program_id;
}

bool LoadAtlasFromFiles(vector<string> file_names, GLuint* out_texture) {
	struct TempImage {
		string file_name;
		int wid, hei;
		unsigned char* data;

		TempImage(string fn) : file_name(fn) {
            data = stbi_load(fn.c_str(), &wid, &hei, NULL, 4);
			if (data == NULL) { Log(fn + " failed to load"); }
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
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, size, size, file_names.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

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
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    *out_texture = image_texture;
    return true;
}

void GenDefaultVertexObjects(GLuint* vao, GLuint* vbo, GLuint shader) {
	glUseProgram(shader);

    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);
    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(0 * sizeof(vec3)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(1 * sizeof(vec3)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)(2 * sizeof(vec3)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)(3 * sizeof(vec3)));
    glEnableVertexAttribArray(3);
	
	glBindVertexArray(0);

	GLuint tex_loc = glGetUniformLocation(shader, "tex");
	GLuint shadow_tex_loc = glGetUniformLocation(shader, "shadow_tex");
	main_shader_loc.model = glGetUniformLocation(shader, "model");
	main_shader_loc.vp = glGetUniformLocation(shader, "VP");
	main_shader_loc.db_vp = glGetUniformLocation(shader, "DepthBiasVP");
	main_shader_loc.view_pos = glGetUniformLocation(shader, "view_pos");
	main_shader_loc.light_dir = glGetUniformLocation(shader, "light_dir");
	main_shader_loc.amb = glGetUniformLocation(shader, "ambient_col");
	main_shader_loc.diff = glGetUniformLocation(shader, "diffuse_col");
	main_shader_loc.spec = glGetUniformLocation(shader, "specular_col");
	main_shader_loc.phong = glGetUniformLocation(shader, "phong");

    glUniform1i(tex_loc, 0);
	glUniform1i(shadow_tex_loc, 1);
}

void GenShadowsVertexObjects(GLuint* vao, GLuint* vbo, GLuint shader) {
	glUseProgram(shader);
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);
    //glGenBuffers(1, vbo);
    //glBindBuffer(GL_ARRAY_BUFFER, *vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	depth_shader_loc.model = glGetUniformLocation(shader, "model");
	depth_shader_loc.vp = glGetUniformLocation(shader, "VP");
}

vector<Vertex> LoadLevelFromFile(string file_name) {
	delete level;
	level = LoadLevel(file_name);
	if (level->walls.size() == 0) {
		Log(file_name + " is invalid level");
        return {};
	}
	
	vector<Vertex> wall_data = level->GenerateWalls();
	vector<Vertex> floor_data = level->GenerateFloors();
    vector<Vertex> geometry_data;
    geometry_data.reserve(wall_data.size() + floor_data.size());
    geometry_data.insert(geometry_data.end(), wall_data.begin(), wall_data.end());
    geometry_data.insert(geometry_data.end(), floor_data.begin(), floor_data.end());

    return geometry_data;
}

int InitializeEnv() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        IM_ASSERT("SDL_Init failed" && 0);

    // Ask SDL to get a recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // Create a window (offsetx, offsety, width, height, flags)
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
	window = SDL_CreateWindow("Project 4, Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, window_flags);
    aspect = screen_width / (float)screen_height;

    // Create a context to draw in
    context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
        IM_ASSERT("gladLoadGLLoader failed" && 0);
	
	return 0;
}

void InitializeImGui() {
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void DrawWorld(DrawSet ds) {
    glUseProgram(ds.shader);
	glBindVertexArray(ds.vao);

	vec3 ones(1,1,1);
	glm::mat4 one(1);

	GLuint amb_loc = glGetUniformLocation(ds.shader, "ambient_col");
	if (amb_loc != -1)
		glUniform3fv(amb_loc, 1, &ones.x);
	
	GLuint dif_loc = glGetUniformLocation(ds.shader, "diffuse_col");
	if (dif_loc != -1)
		glUniform3fv(dif_loc, 1, &ones.x);

	GLuint spc_loc = glGetUniformLocation(ds.shader, "specular_col");
	if (spc_loc != -1)
		glUniform3fv(spc_loc, 1, &ones.x);

	GLuint phong_loc = glGetUniformLocation(ds.shader, "phong");
	if (phong_loc != -1)
		glUniform1f(phong_loc, 10.0);

	GLuint model_loc = glGetUniformLocation(ds.shader, "model");
	if (model_loc != -1)
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(one));

	//glBindBuffer(GL_ARRAY_BUFFER, ds_main.vbo);
	glDrawArrays(GL_TRIANGLES, model_world.start_pos, model_world.size);
	glBindVertexArray(0);
}

void UpdateLightDir() {
	glm::vec3 light_inv_dir = glm::vec3(-light_dir.x, -light_dir.y, -light_dir.z);
	glm::mat4 depth_proj = glm::ortho(-8.f, 8.f, -8.f, 8.f, -8.f, 8.f);
	glm::mat4 depth_view = glm::lookAt(light_inv_dir, glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 depth_vp = depth_proj * depth_view;

	glm::mat4 bias(
		0.5, 0.0, 0.0, 0.0, 
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	glm::mat4 depth_bias_vp = bias * depth_vp;

	glUseProgram(ds_shadows.shader);
	glUniformMatrix4fv(depth_shader_loc.vp, 1, GL_FALSE, glm::value_ptr(depth_vp));

	glUseProgram(ds_main.shader);
	glUniform3fv(main_shader_loc.light_dir, 1, &light_dir.x);
	glUniformMatrix4fv(main_shader_loc.db_vp, 1, GL_FALSE, glm::value_ptr(depth_bias_vp));
}

void DrawMenus() {
	ImGui::ShowDemoWindow();
	ImGui::Begin("Log");
	{
		static string level_string = "assets/scenes/set.txt";
		ImGui::InputText("Level", &level_string);
		ImGui::SameLine();
		if (ImGui::SmallButton("Load")) {
			vector<Vertex> geometry_data = LoadLevelFromFile(level_string);
			//model_world.size = geometry_data.size();
			//glBindBuffer(GL_ARRAY_BUFFER, ds_main.vbo);
			//glBufferData(GL_ARRAY_BUFFER, geometry_data.size() * sizeof(Vertex), geometry_data.data(), GL_STATIC_DRAW);
		}
		if (ImGui::DragFloat3("Light dir", &light_dir.x, 0.1)) {
			UpdateLightDir();
		}
		for (Key* key : level->keys) {
			string input_string = "Key " + string(KEY_COLOR_STR[key->color]) + " Pos";
			ImGui::DragFloat3(input_string.c_str(), &key->position.x, 0.1);
		};
		for (Door* door : level->doors) {
			string input_string = "Door " + string(KEY_COLOR_STR[door->color]) + " Pos";
			ImGui::DragFloat3(input_string.c_str(), &door->position.x, 0.1);
		};
	}

	ImGui::End();
	ShowLog();
}

bool HandleInput() {
	SDL_Event windowEvent;
	while (SDL_PollEvent(&windowEvent)) {
		ImGui_ImplSDL2_ProcessEvent(&windowEvent);
		level->player.HandleInput(&windowEvent);
		if (windowEvent.type == SDL_QUIT)
			return true;
		if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
			return true;
		if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f)
			fullscreen = !fullscreen;
		if (windowEvent.type == SDL_WINDOWEVENT_RESIZED) {
			screen_width = windowEvent.window.data1;
			screen_height = windowEvent.window.data2;
			aspect = screen_width / (float)screen_height;
		}
		SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
	}
	return false;
}
int main(int argc, char* argv[]) {
    InitializeEnv();

	glActiveTexture(GL_TEXTURE0);
	GLuint tex_id;
	LoadAtlasFromFiles({
		"assets/floor.png", "assets/floor_specular.png", "assets/floor_normal.png",
		"assets/wall_top.png", "assets/wall_top_specular.png", "assets/wall_top_normal.png", 
		"assets/noise.png", "assets/noise.png", "assets/noise.png"}, &tex_id
	);
	// END TEXTURE0

	ds_main.shader = LoadShaders("src/shader/main.vert", "src/shader/main.frag");
	ds_shadows.shader = LoadShaders("src/shader/depth_rtt.vert", "src/shader/depth_rtt.frag");
	
    // initialize world geometry
	GenDefaultVertexObjects(&ds_main.vao, &ds_main.vbo, ds_main.shader);
	GenShadowsVertexObjects(&ds_shadows.vao, &ds_shadows.vbo, ds_shadows.shader);

	vector<Vertex> geometry_data = LoadLevelFromFile("assets/scenes/set.txt");
	model_world.start_pos = 0;
    model_world.size = geometry_data.size();

	// TODO: Start buffering data before all of the models are read.

	vector<Vertex> model_data;
	for (string model_name : { "key", "door", "goal" }) {
		model_data = LoadModelFromFile("assets/models/" + model_name + ".obj", 6);
		ModelRange range;
		range.start_pos = geometry_data.size();
		geometry_data.insert(geometry_data.end(), model_data.begin(), model_data.end());
		range.size = geometry_data.size() - range.start_pos;
		models.insert({ model_name, range });
	}

	glBindBuffer(GL_ARRAY_BUFFER, ds_main.vbo);
	glBufferData(GL_ARRAY_BUFFER, geometry_data.size() * sizeof(Vertex), geometry_data.data(), GL_STATIC_DRAW);

	//glBindBuffer(GL_ARRAY_BUFFER, ds_shadows.vbo);
	//glBufferData(GL_ARRAY_BUFFER, geometry_data.size() * sizeof(Vertex), geometry_data.data(), GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glActiveTexture(GL_TEXTURE1);

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	GLuint depth_tex;
	glGenTextures(1, &depth_tex);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	UpdateLightDir();

/*
    { // initialize model geometry
		ds_models.shader = LoadShaders("src/shader/untextured.vert", "src/shader/untextured.frag");
        glUseProgram(ds_models.shader);

        GenDefaultVertexObjects(&ds_models.vao, &ds_models.vbo, ds_models.shader);

        GLint tex_loc = glGetUniformLocation(ds_models.shader, "tex");
        glUniform1i(tex_loc, 0);

        glm::mat4 model = glm::mat4(1);
        GLint uniModel = glGetUniformLocation(ds_models.shader, "model");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

        vector<Vertex> geometry_data;
        vector<Vertex> model_data;

        for (string model_name : { "key", "door", "goal" }) {
		    model_data = LoadModelFromFile("assets/models/" + model_name + ".obj", 6);
            ModelRange range;
            range.start_pos = geometry_data.size();
            geometry_data.insert(geometry_data.end(), model_data.begin(), model_data.end());
		    range.size = geometry_data.size() - range.start_pos;
            models.insert({ model_name, range });
        }

		glBindBuffer(GL_ARRAY_BUFFER, ds_models.vbo);
		glBufferData(GL_ARRAY_BUFFER, geometry_data.size() * sizeof(Vertex), geometry_data.data(), GL_STATIC_DRAW);
	}
*/
	
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_BLEND);
	

	InitializeImGui();

    io = &ImGui::GetIO();
	float last_frame = 0.0;
	float this_frame = 0.0;
    bool quit = false;
    while (!quit) {
		quit = HandleInput();

		// START DRAW SHADOWMAP
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glViewport(0, 0, 4096, 4096);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// update depth_mvp if light direction changes
		DrawWorld(ds_shadows);
		level->Draw(ds_shadows);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// END DRAW SHADOWMAP
		
		this_frame = SDL_GetTicks() / 1000.f;
		float delta = this_frame - last_frame;
		last_frame = SDL_GetTicks() / 1000.f; // get last frame after we are done with logic, not rendering

		ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

		level->Update(delta);

		DrawMenus();
		ImGui::Render();
		
		// START DRAW MAIN
		glViewport(0,0, screen_width, screen_height);

		glClearColor(0.4f, 0.5f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// UPDATE UNIFORMS
		glUseProgram(ds_main.shader);
		glm::mat4 view = level->player.GetViewMatrix();
		glm::mat4 proj = glm::perspective(3.1416f / 3, aspect, 0.1f, 100.0f);
		glm::mat4 vp = proj*view;

		glUniformMatrix4fv(main_shader_loc.vp, 1, GL_FALSE, glm::value_ptr(vp));
		glUniform3fv(main_shader_loc.view_pos, 1, &level->player.camera_pos.x);
		// UPDATE UNIFORMS

		DrawWorld(ds_main);
		level->Draw(ds_main);
		// END DRAW MAIN

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);  // Double buffering

/*		UpdateShaderCameraUniforms(ds_main);
		UpdateShaderCameraUniforms(ds_models);

		DrawWorld(ds_main.shader);
        level->Draw(ds_models.shader);
*/
    }

    // Clean Up
    glDeleteProgram(ds_main.shader);
	glDeleteProgram(ds_models.shader);

    glDeleteBuffers(1, &ds_main.vbo);
    glDeleteVertexArrays(1, &ds_main.vao);
	glDeleteBuffers(1, &ds_models.vbo);
    glDeleteVertexArrays(1, &ds_models.vao);

    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}