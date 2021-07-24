#pragma once

#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
#include <array>

#include "globals.h"
#include "vec3.h"

using namespace std;

namespace caj {

struct ivert {
    array<int, 9> indices;

    ivert(string s) {
		indices.fill(-1);
        int v_type = 0; // pos = 0, tex = 1, nor = 2
        int v_ind = 0; // pos1, pos2, pos3, etc.
        string::size_type next;
        
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            
        while (s.size() > 0) {
            if (isdigit(s.front())) {
                indices[v_ind * 3 + v_type] = stoi(s, &next) - 1;
                s = s.substr(next);
            }
            else if (s.front() == '/') {
                v_type++;
                s = s.substr(1);
            }
            else if (s.front() == ' ') {
                v_ind++;
                v_type = 0;
                s = s.substr(1);
            }
            else
                s = s.substr(1);
        }
    }
};

inline map<string, Material> parse_mtl(string file_name) {
	ifstream in(file_name, ios::in);
	IM_ASSERT(in && "mtl file not loaded");
	if (!in) return map<string, Material>{};

	string contents;
	in.seekg(0, ios::end);
	contents.resize(in.tellg());
	in.seekg(0, ios::beg);
	in.read(&contents[0], contents.size());
	in.close();

	istringstream iss(contents);

	map<string, Material> materials;
	Material current_material;
	while (!iss.eof()) {
		string key;
		iss >> key;
		if (key == "newmtl") {
			if (current_material.name != "")
				materials.insert({current_material.name, current_material});
			current_material = Material{};
			iss >> current_material.name;
		}
		else if (key == "Ns") {
			iss >> current_material.phong;
		}
		else if (key == "Ka") {
			vec3& ka = current_material.ambient_color;
			iss >> ka.x >> ka.y >> ka.z;
		}
		else if (key == "Kd") {
			vec3& kd = current_material.diffuse_color;
			iss >> kd.x >> kd.y >> kd.z;
		}
		else if (key == "Ks") {
			vec3& ks = current_material.specular_color;
			iss >> ks.x >> ks.y >> ks.z;
		}
		else if (key == "map_Ka" || key == "map_Kd") {
			getline(iss, current_material.base_color_tex_file);
			IM_ASSERT(current_material.base_color_tex_file.front() != '-' &&
				"Options in mtl files are not supported");
		}
		else if (key == "map_Ns") {
			getline(iss, current_material.specular_tex_file);
			IM_ASSERT(current_material.specular_tex_file.front() != '-' &&
				"Options in mtl files are not supported");
		}
		else if (key == "map_bump" || key == "bump" || key == "disp") {
			getline(iss, current_material.height_tex_file);
			IM_ASSERT(current_material.height_tex_file.front() != '-' &&
				"Options in mtl files are not supported");
		}
		else {
			iss.ignore(numeric_limits<streamsize>::max(), '\n');
		}
	}
	if (current_material.name != "")
		materials.insert({current_material.name, current_material});
	return materials;
}

inline vector<pair<vector<Vertex>, Material>> parse_obj(string folder_name, string file_name) {
	if (folder_name.back() != '/') folder_name.push_back('/');

	ifstream in(folder_name + file_name, ios::in);
	IM_ASSERT(in && "obj file not loaded");

	string contents;
	in.seekg(0, ios::end);
	contents.resize(in.tellg());
	in.seekg(0, ios::beg);
	in.read(&contents[0], contents.size());
	in.close();

	istringstream iss(contents);

	vector<vec3> positions;
	vector<vec3> normals;
	vector<vec3> texcoords;

	vector<pair<vector<Vertex>, Material>> meshes;
	map<string, Material> materials;
	vector<Vertex> current_vertices;
	Material current_material;

	while (!iss.eof()) {
		string key;
		iss >> key;
		if (key == "mtllib") {
			string mtllib;
			iss >> mtllib;
			materials = parse_mtl(folder_name + mtllib);
		}
		else if (key == "v") {
			vec3 position;
			iss >> position.x >> position.y >> position.z;
			positions.push_back(position);
		}
		else if (key == "vn") {
			vec3 normal;
			iss >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (key == "vt") {
			vec3 texcoord;
			iss >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (key == "usemtl") {
			if (current_vertices.size() > 0)
				meshes.push_back({current_vertices, current_material});
			string mat_key;
			iss >> mat_key;
			current_material = materials[mat_key];
			current_vertices.clear();
		}
		else if (key == "f") {
			string ivert_line;
        	getline(iss, ivert_line);
			ivert v(ivert_line);
			
			vec3 pos1 = positions[v.indices[0*3 + 0]];
			vec3 pos2 = positions[v.indices[1*3 + 0]];
			vec3 pos3 = positions[v.indices[2*3 + 0]];
			vec3 face_norm = cross(pos2 - pos1, pos3 - pos1);

			Vertex v1{
				pos1, 
				v.indices[0*3 + 1] != -1 ? texcoords[v.indices[0*3 + 1]] : vec3(pos1.x, pos1.y, 0.0),
				v.indices[0*3 + 2] != -1 ? normals[v.indices[0*3 + 2]] : face_norm
 			};
			Vertex v2{
				pos2, 
				v.indices[1*3 + 1] != -1 ? texcoords[v.indices[1*3 + 1]] : vec3(pos2.x, pos2.y, 0.0),
				v.indices[1*3 + 2] != -1 ? normals[v.indices[1*3 + 2]] : face_norm
 			};
			Vertex v3{
				pos3, 
				v.indices[2*3 + 1] != -1 ? texcoords[v.indices[2*3 + 1]] : vec3(pos3.x, pos3.y, 0.0),
				v.indices[2*3 + 2] != -1 ? normals[v.indices[2*3 + 2]] : face_norm
 			};
			Vertex::set_tangents(v1, v2, v3);
			current_vertices.push_back(v1);
			current_vertices.push_back(v2);
			current_vertices.push_back(v3);
		}
		else {
			iss.ignore(numeric_limits<streamsize>::max(), '\n');
		}
	}
	if (current_vertices.size() > 0)
		meshes.push_back({current_vertices, current_material});
	
	return meshes;
}



}  // namespace caj
