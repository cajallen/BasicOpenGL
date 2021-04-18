#pragma once

#include <math.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace objl {
struct Vector2 {
	Vector2() : X(0), Y(0) { }
	Vector2(float X_, float Y_) : X(X_), Y(Y_) { }
	bool operator==(const Vector2& other) const { return (this->X == other.X && this->Y == other.Y); }
	bool operator!=(const Vector2& other) const { return !(this->X == other.X && this->Y == other.Y); }
	Vector2 operator+(const Vector2& right) const { return Vector2(this->X + right.X, this->Y + right.Y); }
	Vector2 operator-(const Vector2& right) const { return Vector2(this->X - right.X, this->Y - right.Y); }
	Vector2 operator*(const float& other) const { return Vector2(this->X * other, this->Y * other); }

	float X;
	float Y;
};


struct Vector3 {
	Vector3() : X(0), Y(0), Z(0) { }
	Vector3(float X_, float Y_, float Z_) : X(X_), Y(Y_), Z(Z_) { }
	bool operator==(const Vector3& other) const { return (this->X == other.X && this->Y == other.Y && this->Z == other.Z); }
	bool operator!=(const Vector3& other) const { return !(this->X == other.X && this->Y == other.Y && this->Z == other.Z); }
	Vector3 operator+(const Vector3& right) const { return Vector3(this->X + right.X, this->Y + right.Y, this->Z + right.Z); }
	Vector3 operator-(const Vector3& right) const { return Vector3(this->X - right.X, this->Y - right.Y, this->Z - right.Z); }
	Vector3 operator*(const float& other) const { return Vector3(this->X * other, this->Y * other, this->Z * other); }
	Vector3 operator/(const float& other) const { return Vector3(this->X / other, this->Y / other, this->Z / other); }

	float X;
	float Y;
	float Z;
};

struct Vertex {
	Vector3 Position;
	Vector3 Normal;
	Vector2 TextureCoordinate;
};

// Structure: Mesh
//
// Description: A Simple Mesh Object that holds
//	a name, a vertex list, and an index list
struct Mesh {
	// Default Constructor
	Mesh() {}
	// Variable Set Constructor
	Mesh(vector<Vertex>& _Vertices, vector<unsigned int>& _Indices) {
		Vertices = _Vertices;
		Indices = _Indices;
	}
	// Mesh Name
	string MeshName;
	// Vertex List
	vector<Vertex> Vertices;
	// Index List
	vector<unsigned int> Indices;
};

// Namespace: Math
namespace math {
inline Vector3 CrossV3(const Vector3 a, const Vector3 b) {
	return Vector3(a.Y * b.Z - a.Z * b.Y, a.Z * b.X - a.X * b.Z, a.X * b.Y - a.Y * b.X);
}

inline float MagnitudeV3(const Vector3 in) {
	return (sqrtf(powf(in.X, 2) + powf(in.Y, 2) + powf(in.Z, 2)));
}

inline float DotV3(const Vector3 a, const Vector3 b) {
	return (a.X * b.X) + (a.Y * b.Y) + (a.Z * b.Z);
}

inline float AngleBetweenV3(const Vector3 a, const Vector3 b) {
	float angle = DotV3(a, b);
	angle /= (MagnitudeV3(a) * MagnitudeV3(b));
	return angle = acosf(angle);
}

inline Vector3 ProjV3(const Vector3 a, const Vector3 b) {
	Vector3 bn = b / MagnitudeV3(b);
	return bn * DotV3(a, bn);
}
}  // namespace math

// Namespace: Algorithm
namespace algorithm {
inline Vector3 operator*(const float& left, const Vector3& right) {
	return Vector3(right.X * left, right.Y * left, right.Z * left);
}

inline bool SameSide(Vector3 p1, Vector3 p2, Vector3 a, Vector3 b) {
	Vector3 cp1 = math::CrossV3(b - a, p1 - a);
	Vector3 cp2 = math::CrossV3(b - a, p2 - a);

	if (math::DotV3(cp1, cp2) >= 0)
		return true;
	else
		return false;
}

inline Vector3 GenTriNormal(Vector3 t1, Vector3 t2, Vector3 t3) {
	Vector3 u = t2 - t1;
	Vector3 v = t3 - t1;

	Vector3 normal = math::CrossV3(u, v);

	return normal;
}

inline bool inTriangle(Vector3 point, Vector3 tri1, Vector3 tri2, Vector3 tri3) {
	bool within_tri_prisim = SameSide(point, tri1, tri2, tri3) && SameSide(point, tri2, tri1, tri3) && SameSide(point, tri3, tri1, tri2);

	if (!within_tri_prisim)
		return false;

	Vector3 n = GenTriNormal(tri1, tri2, tri3);

	Vector3 proj = math::ProjV3(point, n);

	if (math::MagnitudeV3(proj) == 0)
		return true;
	else
		return false;
}

inline void split(const string& in, vector<string>& out, string token) {
	out.clear();

	string temp;

	for (int i = 0; i < int(in.size()); i++) {
		string test = in.substr(i, token.size());

		if (test == token) {
			if (!temp.empty()) {
				out.push_back(temp);
				temp.clear();
				i += (int)token.size() - 1;
			} else {
				out.push_back("");
			}
		} else if (i + token.size() >= in.size()) {
			temp += in.substr(i, token.size());
			out.push_back(temp);
			break;
		} else {
			temp += in[i];
		}
	}
}

inline string tail(const string& in) {
	size_t token_start = in.find_first_not_of(" \t");
	size_t space_start = in.find_first_of(" \t", token_start);
	size_t tail_start = in.find_first_not_of(" \t", space_start);
	size_t tail_end = in.find_last_not_of(" \t");
	if (tail_start != string::npos && tail_end != string::npos) {
		return in.substr(tail_start, tail_end - tail_start + 1);
	} else if (tail_start != string::npos) {
		return in.substr(tail_start);
	}
	return "";
}

inline string firstToken(const string& in) {
	if (!in.empty()) {
		size_t token_start = in.find_first_not_of(" \t");
		size_t token_end = in.find_first_of(" \t", token_start);
		if (token_start != string::npos && token_end != string::npos) {
			return in.substr(token_start, token_end - token_start);
		} else if (token_start != string::npos) {
			return in.substr(token_start);
		}
	}
	return "";
}

template <class T>
inline const T& getElement(const vector<T>& elements, string& index) {
	int idx = stoi(index);
	if (idx < 0)
		idx = int(elements.size()) + idx;
	else
		idx--;
	return elements[idx];
}
}  // namespace algorithm

class Loader {
 public:
	Loader() {}
	~Loader() {}

	// Load a file into the loader
	bool LoadFile(string Path) {
		if (Path.substr(Path.size() - 4, 4) != ".obj")
			return false;

		ifstream file(Path);

		if (!file.is_open())
			return false;

		LoadedMeshes.clear();
		LoadedVertices.clear();
		LoadedIndices.clear();

		vector<Vector3> Positions;
		vector<Vector2> TCoords;
		vector<Vector3> Normals;

		vector<Vertex> Vertices;
		vector<unsigned int> Indices;

		vector<string> MeshMatNames;

		bool listening = false;
		string meshname;

		Mesh tempMesh;

		string curline;
		while (getline(file, curline)) {
			// Generate a Mesh Object or Prepare for an object to be created
			if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g" || curline[0] == 'g') {
				if (!listening) {
					listening = true;

					if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g") {
						meshname = algorithm::tail(curline);
					} else {
						meshname = "unnamed";
					}
				} else {
					if (!Indices.empty() && !Vertices.empty()) {
						// Create Mesh
						tempMesh = Mesh(Vertices, Indices);
						tempMesh.MeshName = meshname;

						// Insert Mesh
						LoadedMeshes.push_back(tempMesh);

						// Cleanup
						Vertices.clear();
						Indices.clear();
						meshname.clear();

						meshname = algorithm::tail(curline);
					} else {
						if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g") {
							meshname = algorithm::tail(curline);
						} else {
							meshname = "unnamed";
						}
					}
				}
			}
			// Generate a Vertex Position
			if (algorithm::firstToken(curline) == "v") {
				vector<string> spos;
				Vector3 vpos;
				algorithm::split(algorithm::tail(curline), spos, " ");

				vpos.X = stof(spos[0]);
				vpos.Y = stof(spos[1]);
				vpos.Z = stof(spos[2]);

				Positions.push_back(vpos);
			}
			if (algorithm::firstToken(curline) == "vt") {
				vector<string> stex;
				Vector2 vtex;
				algorithm::split(algorithm::tail(curline), stex, " ");

				vtex.X = stof(stex[0]);
				vtex.Y = stof(stex[1]);

				TCoords.push_back(vtex);
			}
			// Generate a Vertex Normal;
			if (algorithm::firstToken(curline) == "vn") {
				vector<string> snor;
				Vector3 vnor;
				algorithm::split(algorithm::tail(curline), snor, " ");

				vnor.X = stof(snor[0]);
				vnor.Y = stof(snor[1]);
				vnor.Z = stof(snor[2]);

				Normals.push_back(vnor);
			}
			// Generate a Face (vertices & indices)
			if (algorithm::firstToken(curline) == "f") {
				// Generate the vertices
				vector<Vertex> vVerts;
				GenVerticesFromRawOBJ(vVerts, Positions, TCoords, Normals, curline);

				// Add Vertices
				for (int i = 0; i < int(vVerts.size()); i++) {
					Vertices.push_back(vVerts[i]);

					LoadedVertices.push_back(vVerts[i]);
				}

				vector<unsigned int> iIndices;

				VertexTriangluation(iIndices, vVerts);

				// Add Indices
				for (int i = 0; i < int(iIndices.size()); i++) {
					unsigned int indnum = (unsigned int)((Vertices.size()) - vVerts.size()) + iIndices[i];
					Indices.push_back(indnum);

					indnum = (unsigned int)((LoadedVertices.size()) - vVerts.size()) + iIndices[i];
					LoadedIndices.push_back(indnum);
				}
			}
		}

		// Deal with last mesh

		if (!Indices.empty() && !Vertices.empty()) {
			// Create Mesh
			tempMesh = Mesh(Vertices, Indices);
			tempMesh.MeshName = meshname;

			// Insert Mesh
			LoadedMeshes.push_back(tempMesh);
		}

		file.close();

		if (LoadedMeshes.empty() && LoadedVertices.empty() && LoadedIndices.empty()) {
			return false;
		} else {
			return true;
		}
	}

	// Loaded Mesh Objects
	vector<Mesh> LoadedMeshes;
	// Loaded Vertex Objects
	vector<Vertex> LoadedVertices;
	// Loaded Index Positions
	vector<unsigned int> LoadedIndices;

 private:
	void GenVerticesFromRawOBJ(vector<Vertex>& oVerts,
							   const vector<Vector3>& iPositions,
							   const vector<Vector2>& iTCoords,
							   const vector<Vector3>& iNormals,
							   string icurline) {
		vector<string> sface, svert;
		Vertex vVert;
		algorithm::split(algorithm::tail(icurline), sface, " ");

		bool noNormal = false;

		// For every given vertex do this
		for (int i = 0; i < int(sface.size()); i++) {
			// See What type the vertex is.
			int vtype;

			algorithm::split(sface[i], svert, "/");

			// Check for just position - v1
			if (svert.size() == 1) {
				// Only position
				vtype = 1;
			}

			// Check for position & texture - v1/vt1
			if (svert.size() == 2) {
				// Position & Texture
				vtype = 2;
			}

			// Check for Position, Texture and Normal - v1/vt1/vn1
			// or if Position and Normal - v1//vn1
			if (svert.size() == 3) {
				if (svert[1] != "") {
					// Position, Texture, and Normal
					vtype = 4;
				} else {
					// Position & Normal
					vtype = 3;
				}
			}

			// Calculate and store the vertex
			switch (vtype) {
			case 1:	 // P
			{
				vVert.Position = algorithm::getElement(iPositions, svert[0]);
				vVert.TextureCoordinate = Vector2(0, 0);
				noNormal = true;
				oVerts.push_back(vVert);
				break;
			}
			case 2:	 // P/T
			{
				vVert.Position = algorithm::getElement(iPositions, svert[0]);
				vVert.TextureCoordinate = algorithm::getElement(iTCoords, svert[1]);
				noNormal = true;
				oVerts.push_back(vVert);
				break;
			}
			case 3:	 // P//N
			{
				vVert.Position = algorithm::getElement(iPositions, svert[0]);
				vVert.TextureCoordinate = Vector2(0, 0);
				vVert.Normal = algorithm::getElement(iNormals, svert[2]);
				oVerts.push_back(vVert);
				break;
			}
			case 4:	 // P/T/N
			{
				vVert.Position = algorithm::getElement(iPositions, svert[0]);
				vVert.TextureCoordinate = algorithm::getElement(iTCoords, svert[1]);
				vVert.Normal = algorithm::getElement(iNormals, svert[2]);
				oVerts.push_back(vVert);
				break;
			}
			default: {
				break;
			}
			}
		}

		// take care of missing normals
		// these may not be truly acurate but it is the
		// best they get for not compiling a mesh with normals
		if (noNormal) {
			Vector3 A = oVerts[0].Position - oVerts[1].Position;
			Vector3 B = oVerts[2].Position - oVerts[1].Position;

			Vector3 normal = math::CrossV3(A, B);

			for (int i = 0; i < int(oVerts.size()); i++) {
				oVerts[i].Normal = normal;
			}
		}
	}

	// Triangulate a list of vertices into a face by printing
	//	inducies corresponding with triangles within it
	void VertexTriangluation(vector<unsigned int>& oIndices, const vector<Vertex>& iVerts) {
		// If there are 2 or less verts,
		// no triangle can be created,
		// so exit
		if (iVerts.size() < 3) {
			return;
		}
		// If it is a triangle no need to calculate it
		if (iVerts.size() == 3) {
			oIndices.push_back(0);
			oIndices.push_back(1);
			oIndices.push_back(2);
			return;
		}

		// Create a list of vertices
		vector<Vertex> tVerts = iVerts;

		while (true) {
			// For every vertex
			for (int i = 0; i < int(tVerts.size()); i++) {
				// pPrev = the previous vertex in the list
				Vertex pPrev;
				if (i == 0) {
					pPrev = tVerts[tVerts.size() - 1];
				} else {
					pPrev = tVerts[i - 1];
				}

				// pCur = the current vertex;
				Vertex pCur = tVerts[i];

				// pNext = the next vertex in the list
				Vertex pNext;
				if (i == tVerts.size() - 1) {
					pNext = tVerts[0];
				} else {
					pNext = tVerts[i + 1];
				}

				// Check to see if there are only 3 verts left
				// if so this is the last triangle
				if (tVerts.size() == 3) {
					// Create a triangle from pCur, pPrev, pNext
					for (int j = 0; j < int(tVerts.size()); j++) {
						if (iVerts[j].Position == pCur.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pPrev.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pNext.Position)
							oIndices.push_back(j);
					}

					tVerts.clear();
					break;
				}
				if (tVerts.size() == 4) {
					// Create a triangle from pCur, pPrev, pNext
					for (int j = 0; j < int(iVerts.size()); j++) {
						if (iVerts[j].Position == pCur.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pPrev.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pNext.Position)
							oIndices.push_back(j);
					}

					Vector3 tempVec;
					for (int j = 0; j < int(tVerts.size()); j++) {
						if (tVerts[j].Position != pCur.Position && tVerts[j].Position != pPrev.Position &&
							tVerts[j].Position != pNext.Position) {
							tempVec = tVerts[j].Position;
							break;
						}
					}

					// Create a triangle from pCur, pPrev, pNext
					for (int j = 0; j < int(iVerts.size()); j++) {
						if (iVerts[j].Position == pPrev.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pNext.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == tempVec)
							oIndices.push_back(j);
					}

					tVerts.clear();
					break;
				}

				// If Vertex is not an interior vertex
				float angle = math::AngleBetweenV3(pPrev.Position - pCur.Position, pNext.Position - pCur.Position) * (180 / 3.14159265359);
				if (angle <= 0 && angle >= 180)
					continue;

				// If any vertices are within this triangle
				bool inTri = false;
				for (int j = 0; j < int(iVerts.size()); j++) {
					if (algorithm::inTriangle(iVerts[j].Position, pPrev.Position, pCur.Position, pNext.Position) &&
						iVerts[j].Position != pPrev.Position && iVerts[j].Position != pCur.Position &&
						iVerts[j].Position != pNext.Position) {
						inTri = true;
						break;
					}
				}
				if (inTri)
					continue;

				// Create a triangle from pCur, pPrev, pNext
				for (int j = 0; j < int(iVerts.size()); j++) {
					if (iVerts[j].Position == pCur.Position)
						oIndices.push_back(j);
					if (iVerts[j].Position == pPrev.Position)
						oIndices.push_back(j);
					if (iVerts[j].Position == pNext.Position)
						oIndices.push_back(j);
				}

				// Delete pCur from the list
				for (int j = 0; j < int(tVerts.size()); j++) {
					if (tVerts[j].Position == pCur.Position) {
						tVerts.erase(tVerts.begin() + j);
						break;
					}
				}

				// reset i to the start
				// -1 since loop will add 1 to it
				i = -1;
			}

			// if no triangles were created
			if (oIndices.size() == 0)
				break;

			// if no more vertices
			if (tVerts.size() == 0)
				break;
		}
	}
};
}  // namespace objl