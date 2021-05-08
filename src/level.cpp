#include "level.h"

float tile_height = 0.25;


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
		if (c == 'G' || c == 'S') {
			start_pos = c == 'S' ? pos : start_pos;
			end_pos = c == 'G' ? pos : end_pos;
			heightmap.push_back(vec3i(pos.x, pos.y, 0));
			pos.x++;
		}
		else if (isdigit(c)) {
			heightmap.push_back(vec3i(pos.x, pos.y, c - '0'));
			pos.x++;
		}
		else if (c == '!') {
			heightmap.push_back(vec3i(pos.x, pos.y, -1));
			pos.x++;
		}
		else if (c == '#') {
			heightmap.push_back(vec3i(pos.x, pos.y, -2));
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
			heightmap.push_back(vec3i(pos.x, pos.y, 0));
			pos.x++;
		}
		else if (c == ' ') {
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

vector<Vertex> GenerateFloor(vec3i pos) {
	float wall_z_coord = pos.z > 0 ? 3.0 : 0.0;
	vec3 posi = vec3(pos.x, pos.y, pos.z * 0.25);
	vector<Vertex> vertices = {
	Vertex{ posi + vec3(-.5, -.5, 0.), vec3::up(), vec3(0, 0, wall_z_coord) },
	Vertex{ posi + vec3( .5, -.5, 0.), vec3::up(), vec3(1, 0, wall_z_coord) },
	Vertex{ posi + vec3( .5,  .5, 0.), vec3::up(), vec3(1, 1, wall_z_coord) },
	Vertex{ posi + vec3( .5,  .5, 0.), vec3::up(), vec3(1, 1, wall_z_coord) },
	Vertex{ posi + vec3(-.5,  .5, 0.), vec3::up(), vec3(0, 1, wall_z_coord) },
	Vertex{ posi + vec3(-.5, -.5, 0.), vec3::up(), vec3(0, 0, wall_z_coord) }
	};

	for (int i = 0; i + 2 < vertices.size(); i += 3) {
		SetTriTangents(vertices[i], vertices[i + 1], vertices[i + 2]);
	}
	return vertices;
}

vector<Vertex> GenerateFace(vec3i pos1, vec3i pos2) {
	vec3i pos_diff = pos1 - pos2;
	vec3 wall_center = vec3((pos1.x + pos2.x) / 2.0, (pos1.y + pos2.y) / 2.0, 0);
	int keep_dir = pos_diff.z > 0 ? -1 : 1;

	vec3 norm;
	if (abs(pos_diff.x) == 1)
		norm = keep_dir * pos_diff.x * vec3::right();
	else if (abs(pos_diff.y) == 1)
		norm = keep_dir * pos_diff.y * vec3::forward();
	else 
		IM_ASSERT(false && "GenerateFace called with bad pos1/pos2");

	vector<Vertex> vertices{};
	while (pos1.z != pos2.z) {
		float wall_z_coord = pos1.z < 0 ? 3.0 : 3.0;
		vec3 min_pos;
		vec3 max_pos;
		min_pos.x = wall_center.x + 0.5 * min(pos_diff.y, -pos_diff.y);
		max_pos.x = wall_center.x + 0.5 * max(pos_diff.y, -pos_diff.y);
		min_pos.y = wall_center.y + 0.5 * min(pos_diff.x, -pos_diff.x);
		max_pos.y = wall_center.y + 0.5 * max(pos_diff.x, -pos_diff.x);
		min_pos.z = 0.25 * min(pos1.z, pos1.z + keep_dir);
		max_pos.z = 0.25 * max(pos1.z, pos1.z + keep_dir);
		vertices.push_back(Vertex{ vec3(max_pos.x, max_pos.y, min_pos.z), norm, vec3(1, min_pos.z, wall_z_coord) }); // bottom right
		vertices.push_back(Vertex{ vec3(max_pos.x, max_pos.y, max_pos.z), norm, vec3(1, max_pos.z, wall_z_coord) }); // top right
		vertices.push_back(Vertex{ vec3(min_pos.x, min_pos.y, min_pos.z), norm, vec3(0, min_pos.z, wall_z_coord) }); // bottom left
		vertices.push_back(Vertex{ vec3(min_pos.x, min_pos.y, max_pos.z), norm, vec3(0, max_pos.z, wall_z_coord) }); // top left
		vertices.push_back(Vertex{ vec3(min_pos.x, min_pos.y, min_pos.z), norm, vec3(0, min_pos.z, wall_z_coord) }); // bottom left
		vertices.push_back(Vertex{ vec3(max_pos.x, max_pos.y, max_pos.z), norm, vec3(1, max_pos.z, wall_z_coord) }); // top right
		pos1.z += keep_dir;
	}

	for (int i = 0; i + 2 < vertices.size(); i += 3) {
		SetTriTangents(vertices[i], vertices[i + 1], vertices[i + 2]);
	}
	return vertices;
}

vector<Vertex> GenerateCube(vec3i pos, vec3 scale=vec3(1,1,1)) {

	GenerateFace(vec3i(3, 0, 4), vec3i(3, 1, 2));

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
/*
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
*/
vector<Vertex> Level::GenerateWalls() {
	vector<Vertex> vertices{};
	for (vec3i pos1 : heightmap) {
		auto v3it1 = find_if(heightmap.begin(), heightmap.end(), [pos1](vec3i oth) { return oth.x == pos1.x + 1 && oth.y == pos1.y; });
		vec3i pos2 = v3it1 == heightmap.end() ? vec3i(pos1.x + 1, pos1.y, -3) : *v3it1;
		
		auto v3it2 = find_if(heightmap.begin(), heightmap.end(), [pos1](vec3i oth) { return oth.x == pos1.x && oth.y == pos1.y + 1; });
		vec3i pos3 = v3it2 == heightmap.end() ? vec3i(pos1.x, pos1.y + 1, -3) : *v3it2;

		vector<Vertex> right_face = GenerateFace(pos1, pos2);
		vector<Vertex> down_face = GenerateFace(pos1, pos3);

		vertices.insert(vertices.end(), right_face.begin(), right_face.end());
		vertices.insert(vertices.end(), down_face.begin(), down_face.end());

		// CHECK FOR UNFINISHED UPLEFT
		auto v3it3 = find_if(heightmap.begin(), heightmap.end(), [pos1](vec3i oth) { return oth.x == pos1.x - 1 && oth.y == pos1.y; });
		if (v3it3 == heightmap.end()) {
			vector<Vertex> left_face = GenerateFace(pos1, vec3i(pos1.x - 1, pos1.y, -3));
			vertices.insert(vertices.end(), left_face.begin(), left_face.end());
		}
		auto v3it4 = find_if(heightmap.begin(), heightmap.end(), [pos1](vec3i oth) { return oth.x == pos1.x && oth.y == pos1.y - 1; });
		if (v3it4 == heightmap.end()) {
			vector<Vertex> left_face = GenerateFace(pos1, vec3i(pos1.x, pos1.y - 1, -3));
			vertices.insert(vertices.end(), left_face.begin(), left_face.end());
		}
	}
	return vertices;
}

vector<Vertex> Level::GenerateFloors() {
	vector<Vertex> vertices{};
	for (vec3i floor : heightmap) {
		vector<Vertex> f_verts = GenerateFloor(floor);
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
				height = fmax(height, 1.75);
		vector<vec3i>::iterator v3it = find_if(heightmap.begin(), heightmap.end(), [pos](vec3i oth) { return oth.x == pos.x && oth.y == pos.y; } );
		if (v3it != heightmap.end())
			height = fmax(height, 0.25 * v3it->z);
	}
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
		ImGui::OpenPopup("Start");
	}
	bool temp = true;
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Start", &temp, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Your friend got lost. Find him!\nQ: Toggle mouse\nWASD: Movement\nZ: Fly\nEsc: Exit");
		ImGui::EndPopup();
	}
	else {
		displayed_start = true;
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
			key->position.z -= 0.5;
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
			ImGui::OpenPopup("End");
		}
	}
	bool temp2 = true;
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("End", &temp2, ImGuiWindowFlags_AlwaysAutoResize)) {
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