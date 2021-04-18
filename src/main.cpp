#include "main.h"

bool fullscreen = false;
float screen_width = 1900;
float screen_height = 1000;
SDL_Window* window;
ImGuiIO* io;
Level* level;
DrawSet ds_world;
DrawSet ds_models;
ModelRange model_world;
map<string, ModelRange> models;

// not extern
const char* glsl_version = "#version 130";

float aspect;
SDL_GLContext context;
vec3 light_pos(4,12,2);



GLuint LoadVertexShaderFromFile(string s) {
    ifstream in(s);
    string contents1((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    const char* vertex_source = contents1.c_str();

    GLuint shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader_id, 1, &vertex_source, NULL);
    glCompileShader(shader_id);

    // Let's double check the shader compiled
    GLint status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    if (!status) {
        char buffer[512];
        glGetShaderInfoLog(shader_id, 512, NULL, buffer);
        Log("Vertex Shader Compile Failed. Info:\n" + string(buffer));
    }

    return shader_id;
}
GLuint LoadFragmentShaderFromFile(string s) {
    ifstream in(s);
    string contents((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    const char* fragment_source = contents.c_str();

    GLuint shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader_id, 1, &fragment_source, NULL);
    glCompileShader(shader_id);

    // Double check the shader compiled
    GLint status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    if (!status) {
        char buffer[512];
        glGetShaderInfoLog(shader_id, 512, NULL, buffer);
        Log("Fragment Shader Compile Failed.Info:\n" + string(buffer));
    }

    return shader_id;
}

GLuint JoinShaders(GLuint vert, GLuint frag) {
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vert);
    glAttachShader(shaderProgram, frag);
    GLenum gl_error = glGetError();
    glBindFragDataLocation(shaderProgram, 0, "outColor");  // set output
    gl_error = glGetError();
    glLinkProgram(shaderProgram);                          // run the linker
    gl_error = glGetError();

    // Double check the shader linked
    GLint status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    gl_error = glGetError();
    if (status == GL_FALSE) {
        char buffer[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, buffer);
        Log("Link Failed. Info:\n" + string(buffer));
    }

    glDetachShader(shaderProgram, vert);
    glDetachShader(shaderProgram, frag);

    return shaderProgram;
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
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); // i don't want to look into fix atm
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR); // linear causes bleedthru
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    *out_texture = image_texture;

    return true;
}

void GenDefaultVAOVBO(GLuint* vao, GLuint* vbo, GLuint shader) {
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);

    GLint posAttrib = glGetAttribLocation(shader, "inPosition");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(0 * sizeof(vec3)));
    glEnableVertexAttribArray(posAttrib);

    GLint normAttrib = glGetAttribLocation(shader, "inNormal");
    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(1 * sizeof(vec3)));
    glEnableVertexAttribArray(normAttrib);

	GLint texCoordAttrib = glGetAttribLocation(shader, "inTexcoord");
    glVertexAttribPointer(texCoordAttrib, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)(2 * sizeof(vec3)));
    glEnableVertexAttribArray(texCoordAttrib);

	GLint tangentAttrib = glGetAttribLocation(shader, "inTangent");
    glVertexAttribPointer(tangentAttrib, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)(3 * sizeof(vec3)));
    glEnableVertexAttribArray(tangentAttrib);

	GLint bitangentAttrib = glGetAttribLocation(shader, "inBitangent");
    glVertexAttribPointer(bitangentAttrib, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)(4 * sizeof(vec3)));
    glEnableVertexAttribArray(bitangentAttrib);

    GLint tex_loc = glGetUniformLocation(shader, "tex");
    glUniform1i(tex_loc, 0);

    glm::mat4 model = glm::mat4(1);
    GLint uniModel = glGetUniformLocation(shader, "model");
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
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

void UpdateShaderCameraUniforms(DrawSet ds) {
	glUseProgram(ds.shader);

	glm::mat4 view = level->player.GetViewMatrix();
	GLint uniView = glGetUniformLocation(ds.shader, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(3.1416f / 3, aspect, 0.1f, 100.0f);
	GLint uniProj = glGetUniformLocation(ds.shader, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	GLint viewPos = glGetUniformLocation(ds.shader, "view_pos");
	glUniform3fv(viewPos, 1, &level->player.camera_pos.x);

	GLint lightPos = glGetUniformLocation(ds.shader, "light_pos");
	glUniform3fv(lightPos, 1, &light_pos.x);
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

void DrawWorld() {
    glUseProgram(ds_world.shader);
	glBindVertexArray(ds_world.vao);
	glDrawArrays(GL_TRIANGLES, model_world.start_pos, model_world.size);
}

int main(int argc, char* argv[]) {
    InitializeEnv();

	GLuint tex_id;
	glActiveTexture(GL_TEXTURE0);

	LoadAtlasFromFiles({
		"assets/floor.png", "assets/floor_specular.png", "assets/floor_normal.png",
		"assets/wall_top.png", "assets/wall_top_specular.png", "assets/wall_top_normal.png", 
		"assets/noise.png"}, &tex_id);

    { // initialize world geometry
		ds_world.vertex = LoadVertexShaderFromFile("src/shader/main.vert");
        ds_world.fragment = LoadFragmentShaderFromFile("src/shader/main.frag");
        ds_world.shader = JoinShaders(ds_world.vertex, ds_world.fragment);
		glUseProgram(ds_world.shader);


		GenDefaultVAOVBO(&ds_world.vao, &ds_world.vbo, ds_world.shader);

		GLint tex_loc = glGetUniformLocation(ds_world.shader, "tex");
		glUniform1i(tex_loc, 0);

		glm::mat4 model = glm::mat4(1);
        GLint uniModel = glGetUniformLocation(ds_world.shader, "model");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		vector<Vertex> geometry_data = LoadLevelFromFile("assets/scenes/set.txt");
        model_world.size = geometry_data.size();
		glBindBuffer(GL_ARRAY_BUFFER, ds_world.vbo);
		glBufferData(GL_ARRAY_BUFFER, geometry_data.size() * sizeof(Vertex), geometry_data.data(), GL_STATIC_DRAW);
	}

    { // initialize model geometry
        ds_models.vertex = LoadVertexShaderFromFile("src/shader/untextured.vert");
        ds_models.fragment = LoadFragmentShaderFromFile("src/shader/untextured.frag");
        ds_models.shader = JoinShaders(ds_models.vertex, ds_models.fragment);
        glUseProgram(ds_models.shader);

        GenDefaultVAOVBO(&ds_models.vao, &ds_models.vbo, ds_models.shader);

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

    glEnable(GL_DEPTH_TEST);

	InitializeImGui();

    io = &ImGui::GetIO();

	float last_frame = 0.0;
	float this_frame = 0.0;
    bool quit = false;
    while (!quit) {
		SDL_Event windowEvent;
        while (SDL_PollEvent(&windowEvent)) {
            ImGui_ImplSDL2_ProcessEvent(&windowEvent);
			level->player.HandleInput(&windowEvent);
            if (windowEvent.type == SDL_QUIT)
                quit = true;
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
                quit = true;
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f)
                fullscreen = !fullscreen;
            if (windowEvent.type == SDL_WINDOWEVENT_RESIZED) {
                screen_width = windowEvent.window.data1;
                screen_height = windowEvent.window.data2;
                aspect = screen_width / (float)screen_height;
            }
            SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        ImGui::Begin("Log");
        {
			static string level_string = "assets/scenes/set.txt";
			ImGui::InputText("Level", &level_string);
			ImGui::SameLine();
			if (ImGui::SmallButton("Load")) {
                vector<Vertex> geometry_data = LoadLevelFromFile(level_string);
                model_world.size = geometry_data.size();
                glBindBuffer(GL_ARRAY_BUFFER, ds_world.vbo);
                glBufferData(GL_ARRAY_BUFFER, geometry_data.size() * sizeof(Vertex), geometry_data.data(), GL_STATIC_DRAW);
			}
			ImGui::DragFloat3("Light Pos", &light_pos.x, 0.1);
			for (Key& key : level->keys) {
				string input_string = "Key "+string(KEY_COLOR_STR[key.col])+" Pos";
				ImGui::DragFloat3(input_string.c_str(), &key.pos.x, 0.1);
			};
			for (Door& door : level->doors) {
				string input_string = "Door "+string(KEY_COLOR_STR[door.col])+" Pos";
				ImGui::DragFloat3(input_string.c_str(), &door.pos.x, 0.1);
			};
		}
		ImGui::End();
        ShowLog();

		this_frame = SDL_GetTicks() / 1000.f;
        float delta = this_frame - last_frame;
		last_frame = SDL_GetTicks() / 1000.f; // get last frame after we are done with logic, not rendering
        
        level->Update(delta);

        ImGui::Render();

		UpdateShaderCameraUniforms(ds_world);
		UpdateShaderCameraUniforms(ds_models);

        // Clear the screen to default color
        glClearColor(0.4f, 0.5f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DrawWorld();
        level->Draw();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);  // Double buffering
    }

    // Clean Up
    glDeleteProgram(ds_world.shader);
    glDeleteShader(ds_world.fragment);
    glDeleteShader(ds_world.vertex);
	glDeleteProgram(ds_models.shader);
    glDeleteShader(ds_models.fragment);
    glDeleteShader(ds_models.vertex);

    glDeleteBuffers(1, &ds_world.vbo);
    glDeleteVertexArrays(1, &ds_world.vao);
	glDeleteBuffers(1, &ds_models.vbo);
    glDeleteVertexArrays(1, &ds_models.vao);

    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}