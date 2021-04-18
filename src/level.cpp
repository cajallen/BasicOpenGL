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
				doors.push_back(Door{pos, GetColorFromChar(c)});
			}
			else {
				keys.push_back(Key{pos, GetColorFromChar(c), .2, 1});
				keys.back().pos.z -= .3;
			}
			floors.push_back(pos);
			pos.x++;
		}
		else {
			//Log("Invalid char in scene: \"" + string(c, 1) + "\"");
			pos.x++;
		}
	}
	// initialize player
	player.logic_pos = start_pos;
	player.height_at_pos = GetHeightAt(player.logic_pos.x, player.logic_pos.y, player.width);

	goal.pos = end_pos;
	goal.pos.z -= 0.50;
}

vector<Vertex> GenerateCube(vec3i pos, vec3 scale=vec3(1,1,1)) {
	float wall_z_coord = 3.0;
	vector<Vertex> vertices = {
		Vertex{pos + vec3(0b001), vec3::up(),      vec3(0, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b101), vec3::up(),      vec3(1, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b111), vec3::up(),      vec3(1, 1, 	wall_z_coord)},
		Vertex{pos + vec3(0b111), vec3::up(),      vec3(1, 1, 	wall_z_coord)},
		Vertex{pos + vec3(0b011), vec3::up(),      vec3(0, 1, 	wall_z_coord)},
		Vertex{pos + vec3(0b001), vec3::up(),      vec3(0, 0, 	wall_z_coord)},

		Vertex{pos + vec3(0b010), vec3::left(),    vec3(0, .5, 	wall_z_coord)},
		Vertex{pos + vec3(0b011), vec3::left(),    vec3(0, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b001), vec3::left(),    vec3(1, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b001), vec3::left(),    vec3(1, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b000), vec3::left(),    vec3(1, .5, 	wall_z_coord)},
		Vertex{pos + vec3(0b010), vec3::left(),    vec3(0, .5, 	wall_z_coord)},

		Vertex{pos + vec3(0b111), vec3::right(),   vec3(0, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b110), vec3::right(),   vec3(0, .5, 	wall_z_coord)},
		Vertex{pos + vec3(0b100), vec3::right(),   vec3(1, .5, 	wall_z_coord)},
		Vertex{pos + vec3(0b100), vec3::right(),   vec3(1, .5, 	wall_z_coord)},
		Vertex{pos + vec3(0b101), vec3::right(),   vec3(1, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b111), vec3::right(),   vec3(0, 0, 	wall_z_coord)},

		Vertex{pos + vec3(0b100), vec3::back(),    vec3(0, .5, 	wall_z_coord)},
		Vertex{pos + vec3(0b101), vec3::back(),    vec3(0, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b001), vec3::back(),    vec3(1, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b001), vec3::back(),    vec3(1, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b000), vec3::back(),    vec3(1, .5, 	wall_z_coord)},
		Vertex{pos + vec3(0b100), vec3::back(),    vec3(0, .5, 	wall_z_coord)},

		Vertex{pos + vec3(0b110), vec3::forward(), vec3(0, .5, 	wall_z_coord)},
		Vertex{pos + vec3(0b111), vec3::forward(), vec3(0, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b011), vec3::forward(), vec3(1, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b011), vec3::forward(), vec3(1, 0, 	wall_z_coord)},
		Vertex{pos + vec3(0b010), vec3::forward(), vec3(1, .5, 	wall_z_coord)},
		Vertex{pos + vec3(0b110), vec3::forward(), vec3(0, .5, 	wall_z_coord)},
	};
	for (int i = 0; i+2 < vertices.size(); i += 3) {
		SetTriTangents(vertices[i], vertices[i+1], vertices[i+2]);
	}
	return vertices;
}

vector<Vertex> GenerateFloor(vec3i pos) {
	float floor_z_coord = 0.0;
	vector<Vertex> vertices = {
		Vertex{pos + vec3(0b000), vec3::up(), vec3(0, 0, floor_z_coord)},
		Vertex{pos + vec3(0b100), vec3::up(), vec3(1, 0, floor_z_coord)},
		Vertex{pos + vec3(0b110), vec3::up(), vec3(1, 1, floor_z_coord)},
		Vertex{pos + vec3(0b110), vec3::up(), vec3(1, 1, floor_z_coord)},
		Vertex{pos + vec3(0b010), vec3::up(), vec3(0, 1, floor_z_coord)},
		Vertex{pos + vec3(0b000), vec3::up(), vec3(0, 0, floor_z_coord)},
	};
	SetTriTangents(vertices[0], vertices[1], vertices[2]);
	SetTriTangents(vertices[3], vertices[4], vertices[5]);
	return vertices;
}

vector<Vertex> Level::GenerateWalls() {
	vector<Vertex> vertices{};
	for (vec3i wall : walls) {
		vector<Vertex> w_verts = GenerateCube(wall);
		vertices.insert(vertices.end(), w_verts.begin(), w_verts.end());
	}
	return vertices;
}

vector<Vertex> Level::GenerateFloors() {
	vector<Vertex> vertices{};
	for (vec3i floor : floors) {
		vector<Vertex> w_verts = GenerateFloor(floor);
		vertices.insert(vertices.end(), w_verts.begin(), w_verts.end());
	}
	return vertices;
}

float Level::GetHeightAt(float x, float y, float width) {
	vec3i bl(round(x - width), round(y - width), 0);
	vec3i br(round(x + width), round(y - width), 0);
	vec3i tl(round(x - width), round(y + width), 0);
	vec3i tr(round(x + width), round(y + width), 0);
	
	float height = -.9;
	for (vec3i pos : {bl, br, tl, tr}) {
		if ( vector<Door>::iterator door_it = find_if(doors.begin(), doors.end(), [pos](Door& lhs) { return lhs.pos == (vec3) pos; }); door_it != doors.end())
			if (!player.keys[door_it->col])
				height = fmax(height, 2);
		if (find(walls.begin(), walls.end(), pos) != walls.end())
			height = fmax(height, 0);
	}

	if (find(floors.begin(), floors.end(), vec3i(round(x),round(y),0)) != floors.end())
		height = fmax(height, -.5);
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
	if (ImGui::BeginPopup("StartScreen")) {
		ImGui::Text("Your friend got lost. Find him!");
		ImGui::EndPopup();
	}

	player.Update(delta);
	if (!player.noclip)
		MovePlayer();

	for (Key& key : keys)
		key.Update(delta);
    for (Door& door : doors)
		door.Update(delta);
	goal.Update(delta);

	vec3 p_xypos = player.logic_pos;
	p_xypos.z = 0;
	for (Key& key : keys) {
		vec3 k_xypos = key.pos;
		k_xypos.z = 0;
		if ((p_xypos - k_xypos).mag() < key.width)
			player.keys[key.col] = true;
	}
	for (Door& door : doors) {
		vec3 d_xypos = door.pos;
		d_xypos.z = 0;
		if (player.keys[door.col] && (d_xypos - p_xypos).mag() < 2 && !door.IsOpen()) {
			door.Open();
		} 
	}
	static bool displayed_win = false;
	vec3 g_xypos = goal.pos;
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
	if (height_at_desired <= player.GetFeetHeight()) {
		// set desired position, update height
		player.logic_pos = vec3(player.des_pos.x, player.logic_pos.y, player.logic_pos.z);
		player.height_at_pos = height_at_desired;
	}

	// move y, use x cause it's set correctly or it failed and we shouldn't use
	height_at_desired = GetHeightAt(player.logic_pos.x, player.des_pos.y, player.width);
	if (height_at_desired <= player.GetFeetHeight()) {
		// set desired position, update height
		player.logic_pos = vec3(player.logic_pos.x, player.des_pos.y, player.logic_pos.z);
		player.height_at_pos = height_at_desired;
	}
}

void Level::Draw() {
	glUseProgram(ds_models.shader);
	glBindVertexArray(ds_models.vao);

	float offset = -0.1;
	for (Key& key : keys) {
		if (player.keys[key.col]) {
			offset += 0.1;
			key.pos = player.logic_pos;
			key.pos.z -= 0.3;
			vec3 ld = player.look_dir;
			ld.z = 0;
			ld = ld.normalized();
			vec3 right = cross(ld, vec3::up());
			key.pos += right * offset;
			key.rotation = ld.yaw() + PI / 2.f;
		}
		key.Draw();
	}
	for (Door& door : doors)
		door.Draw();
	goal.Draw();
}