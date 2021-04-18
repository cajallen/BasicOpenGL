#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#include "vec3.h"
#include "globals.h"
#include "log.h"
#include "player.h"
#include "key.h"

using namespace std;

struct Level {
	// up is defined as 0,0,1, so we typically ignore the third coordinate here
	vector<vec3i> walls;
	vector<vec3i> floors;
	vector<Key> keys;
	vector<Door> doors;
	Goal goal;
	Player player;
	vec3i start_pos;
	vec3i end_pos;

	Level();
	Level(string s);

	vector<Vertex> GenerateWalls();
	vector<Vertex> GenerateFloors();
	float GetHeightAt(float x, float y, float width);
	void Update(float delta);
	void Draw();
	void MovePlayer();
};

Level* LoadLevel(string file_name);

