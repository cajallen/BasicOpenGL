#include "level.h"


KEY_COLOR GetColorFromChar(char c) {
	switch(c) {
	case 'a':
	case 'A':
		return ADAMANT;
	case 'b':
	case 'B':
		return BRONZE;
	case 'c':
	case 'C':
		return COBALT;
	case 'd':
	case 'D':
		return OBSIDIAN;
	case 'e':
	case 'E':
		return TUNGSTEN;
	}
	return ADAMANT;
}


Level::Level() { }

Level::Level(string s) {
	vec3i pos = vec3i(0,0,0);

	// parse level & instance objects
	for (char c : s) {
		if (c == 'G' || c == 'S' || c == '0') {
			start_pos = c == 'S' ? pos : start_pos;
			end_pos = c == 'G' ? pos : end_pos;
			floors.push_back(pos);
			pos.x++;
		}
		else if (c == 'W') {
			walls.push_back(pos);
			pos.x++;
		}
		else if (c == '\n') {
			pos.x = 0;
			pos.y++;
		}
		else if (isalpha(c)) {
			if (isupper(c)) {
				doors.push_back(new Door((vec3) pos, GetColorFromChar(c)));
			}
			else {
				keys.push_back(new Key((vec3) pos, GetColorFromChar(c), 1.0, 0.2));
			}
			floors.push_back(pos);
			pos.x++;
		}
		else {
			Log("Invalid char in scene: \"" + string(c, 1) + "\"");
			pos.x++;
		}
	}
	// initialize player
	player.logic_pos = start_pos;
	player.height_at_pos = GetHeightAt(player.logic_pos.x, player.logic_pos.y, player.width);

	goal = new Goal(end_pos, 0.0);
}

Level::~Level() {
	delete goal;
	for (auto kp : keys)
		delete kp;
	for (auto dp : doors)
		delete dp;
}

vector<Vertex> GenerateCube(vec3i pos, vec3 scale=vec3(1,1,1)) {
	float wall_z_coord = 3.0;
	vector<Vertex> vertices = {
		Vertex{pos + scale*vec3(0b001), vec3::up(),      vec3(0, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b101), vec3::up(),      vec3(1, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b111), vec3::up(),      vec3(1, 1, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b111), vec3::up(),      vec3(1, 1, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b011), vec3::up(),      vec3(0, 1, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b001), vec3::up(),      vec3(0, 0, 		wall_z_coord)},

		Vertex{pos + scale*vec3(0b011), vec3::left(),    vec3(0, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b010), vec3::left(),    vec3(0, scale.z, 	wall_z_coord)},
		Vertex{pos + scale*vec3(0b001), vec3::left(),    vec3(1, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b000), vec3::left(),    vec3(1, scale.z, 	wall_z_coord)},
		Vertex{pos + scale*vec3(0b001), vec3::left(),    vec3(1, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b010), vec3::left(),    vec3(0, scale.z, 	wall_z_coord)},

		Vertex{pos + scale*vec3(0b110), vec3::right(),   vec3(0, scale.z, 	wall_z_coord)},
		Vertex{pos + scale*vec3(0b111), vec3::right(),   vec3(0, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b100), vec3::right(),   vec3(1, scale.z, 	wall_z_coord)},
		Vertex{pos + scale*vec3(0b101), vec3::right(),   vec3(1, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b100), vec3::right(),   vec3(1, scale.z, 	wall_z_coord)},
		Vertex{pos + scale*vec3(0b111), vec3::right(),   vec3(0, 0, 		wall_z_coord)},

		Vertex{pos + scale*vec3(0b100), vec3::back(),    vec3(0, scale.z, 	wall_z_coord)},
		Vertex{pos + scale*vec3(0b101), vec3::back(),    vec3(0, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b001), vec3::back(),    vec3(1, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b001), vec3::back(),    vec3(1, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b000), vec3::back(),    vec3(1, scale.z, 	wall_z_coord)},
		Vertex{pos + scale*vec3(0b100), vec3::back(),    vec3(0, scale.z, 	wall_z_coord)},

		Vertex{pos + scale*vec3(0b111), vec3::forward(), vec3(0, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b110), vec3::forward(), vec3(0, scale.z, 	wall_z_coord)},
		Vertex{pos + scale*vec3(0b011), vec3::forward(), vec3(1, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b010), vec3::forward(), vec3(1, scale.z, 	wall_z_coord)},
		Vertex{pos + scale*vec3(0b011), vec3::forward(), vec3(1, 0, 		wall_z_coord)},
		Vertex{pos + scale*vec3(0b110), vec3::forward(), vec3(0, scale.z, 	wall_z_coord)},
	};
	for (int i = 0; i+2 < vertices.size(); i += 3) {
		SetTriTangents(vertices[i], vertices[i+1], vertices[i+2]);
	}
	return vertices;
}

vector<Vertex> GenerateFloor(vec3i pos, vec3 scale=vec3(1,1,1)) {
	float floor_z_coord = 0.0;
	vector<Vertex> vertices = {
		Vertex{pos + scale*vec3(0b000), vec3::up(), vec3(0, 0, floor_z_coord)},
		Vertex{pos + scale*vec3(0b100), vec3::up(), vec3(1, 0, floor_z_coord)},
		Vertex{pos + scale*vec3(0b110), vec3::up(), vec3(1, 1, floor_z_coord)},
		Vertex{pos + scale*vec3(0b110), vec3::up(), vec3(1, 1, floor_z_coord)},
		Vertex{pos + scale*vec3(0b010), vec3::up(), vec3(0, 1, floor_z_coord)},
		Vertex{pos + scale*vec3(0b000), vec3::up(), vec3(0, 0, floor_z_coord)},
	};
	SetTriTangents(vertices[0], vertices[1], vertices[2]);
	SetTriTangents(vertices[3], vertices[4], vertices[5]);
	return vertices;
}

vector<Vertex> Level::GenerateWalls() {
	vector<Vertex> vertices{};
	for (vec3i wall : walls) {
		vector<Vertex> w_verts = GenerateCube(wall, vec3(1,1,0.5));
		vertices.insert(vertices.end(), w_verts.begin(), w_verts.end());
	}
	return vertices;
}

vector<Vertex> Level::GenerateFloors() {
	vector<Vertex> vertices{};
	for (vec3i floor : floors) {
		vector<Vertex> f_verts = GenerateFloor(floor, vec3(1,1,0.5));
		vertices.insert(vertices.end(), f_verts.begin(), f_verts.end());
	}
	return vertices;
}

float Level::GetHeightAt(float x, float y, float width) {
	vec3i bl(round(x - width), round(y - width), 0);
	vec3i br(round(x + width), round(y - width), 0);
	vec3i tl(round(x - width), round(y + width), 0);
	vec3i tr(round(x + width), round(y + width), 0);
	
	float height = -.5;
	for (vec3i pos : {bl, br, tl, tr}) {
		if (auto door_it = find_if(doors.begin(), doors.end(), [pos](Door* lhs) { return lhs->position == (vec3) pos; }); door_it != doors.end())
			if (!player.keys[(*door_it)->color])
				height = fmax(height, 1.45);
		if (find(walls.begin(), walls.end(), pos) != walls.end())
			height = fmax(height, .25);
	}

	if (find(floors.begin(), floors.end(), vec3i(round(x),round(y),0)) != floors.end())
		height = fmax(height, -.25);
	return height;
}


Level* LoadLevel(string file_name) {
	ifstream ifs(file_name);
	if (ifs.is_open()) {
    	string contents((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    	return new Level(contents);
	}
	else {
		return new Level();
	}
}

void Level::Update(float delta) {

	static bool displayed_start = false;
	if (!displayed_start) {
		displayed_start = true;
		ImGui::OpenPopup("StartScreen");
	}
	bool temp;
	if (ImGui::BeginPopupModal("StartScreen", &temp)) {
		ImGui::Text("Your friend got lost. Find him!");
		ImGui::EndPopup();
	}
	player.Update(delta);
	if (!player.noclip)
		MovePlayer();

	for (Key* key : keys)
		key->Update(delta);
    for (Door* door : doors)
		door->Update(delta);
	goal->Update(delta);

	for (Key* key : keys) {
		float offset = -0.15;
		if (player.keys[key->color]) {
			offset += 0.1;
			key->position = player.logic_pos;
			key->position.z -= 0.3;
			vec3 ld = player.look_dir;
			ld.z = 0;
			ld = ld.normalized();
			vec3 right = cross(ld, vec3::up());
			key->position += right * offset;
			key->rotation = ld.yaw() + PI / 2.f;
		}
	}

	vec3 p_xypos = player.logic_pos;
	p_xypos.z = 0;
	for (Key* key : keys) {
		vec3 k_xypos = key->position;
		k_xypos.z = 0;
		if ((p_xypos - k_xypos).mag() < key->width)
			player.keys[key->color] = true;
	}
	for (Door* door : doors) {
		vec3 d_xypos = door->position;
		d_xypos.z = 0;
		if (player.keys[door->color] && (d_xypos - p_xypos).mag() < 2 && !door->IsOpen()) {
			door->Open();
		} 
	}
	static bool displayed_win = false;
	vec3 g_xypos = goal->position;
	g_xypos.z = 0;
	if ((p_xypos - g_xypos).mag() < 0.3) {
		if (!displayed_win) {
			displayed_win = true;
			ImGui::OpenPopup("WinScreen");
		}
	}

	if (ImGui::BeginPopup("WinScreen")) {
		ImGui::Text("You took too long :(");
		ImGui::EndPopup();
	}
}

void Level::MovePlayer() {
	float height_at_desired = GetHeightAt(player.des_pos.x, player.logic_pos.y, player.width);
	if (height_at_desired <= player.GetFeetHeight() + 0.001) {
		// set desired position, update height
		player.logic_pos = vec3(player.des_pos.x, player.logic_pos.y, player.logic_pos.z);
		player.height_at_pos = height_at_desired;
	}

	// move y, use x cause it's set correctly or it failed and we shouldn't use
	height_at_desired = GetHeightAt(player.logic_pos.x, player.des_pos.y, player.width);
	if (height_at_desired <= player.GetFeetHeight() + 0.001) {
		// set desired position, update height
		player.logic_pos = vec3(player.logic_pos.x, player.des_pos.y, player.logic_pos.z);
		player.height_at_pos = height_at_desired;
	}
}

void Level::Draw(DrawSet ds) {
	glUseProgram(ds.shader);
	glBindVertexArray(ds.vao);

	for (Key* key : keys) {
		key->Draw(ds);
	}
	for (Door* door : doors)
		door->Draw(ds);
	goal->Draw(ds);
	glBindVertexArray(0);
}