#pragma once

#include <vector>

#include "globals.h"
#include "vec3.h"
#include "obj_loader.h"

using namespace std;

inline vector<Vertex> LoadModelFromFile(string s, int layer) {
	objl::Loader loader;
    bool temp = loader.LoadFile(s);
	IM_ASSERT(temp && "Model failed to load");

	vector<Vertex> vertices;
	vertices.reserve(loader.LoadedIndices.size());
	for (int i : loader.LoadedIndices) {
		objl::Vertex v = loader.LoadedVertices.at(i);
		float z_coord = 6.0;
		vertices.push_back(Vertex{v.Position, v.Normal, vec3(v.TextureCoordinate.X, v.TextureCoordinate.Y, z_coord)});
	}
	for (int i = 0; i + 2 < vertices.size(); i += 3) {
		SetTriTangents(vertices[i], vertices[i + 1], vertices[i + 2]);
	}
	return vertices;
}