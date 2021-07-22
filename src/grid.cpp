#include "grid.h"

namespace caj {

vector<Vertex> generate_grid(int divisions) {
    float z_coord = 0.0;
    float cell_width = 1.0 / divisions;
    caj::log(format("{}", cell_width));
    vector<Vertex> vertices;
    vertices.reserve(pow(divisions + 1, 2) * 6);
    for (float y = 0.0; y < divisions; y += 1.0) {
        for (float x = 0.0; x < divisions; x += 1.0) {
            float left = x * cell_width;
            float right = (x + 1.0) * cell_width;
            float top = y * cell_width;
            float bottom = (y + 1.0) * cell_width;
            vertices.push_back(Vertex{vec3(left, top, 0.0), vec3::up(), vec3(left, top, z_coord)});
            vertices.push_back(Vertex{vec3(right, top, 0.0), vec3::up(), vec3(right, top, z_coord)});
            vertices.push_back(Vertex{vec3(left, bottom, 0.0), vec3::up(), vec3(left, bottom, z_coord)});
            vertices.push_back(Vertex{vec3(right, top, 0.0), vec3::up(), vec3(right, top, z_coord)});
            vertices.push_back(Vertex{vec3(right, bottom, 0.0), vec3::up(), vec3(right, bottom, z_coord)});
            vertices.push_back(Vertex{vec3(left, bottom, 0.0), vec3::up(), vec3(left, bottom, z_coord)});
        }
    }
    for (int i = 0; i + 2 < vertices.size(); i += 3)
		Vertex::set_tangents(vertices[i], vertices[i + 1], vertices[i + 2]);
    return vertices;
}

}  // namespace caj
