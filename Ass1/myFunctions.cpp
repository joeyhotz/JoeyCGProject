/** This file contains five functions that get
called when the user presses 1-5 and a draw( Mesh m )
function that gets called everytime the mesh needs to
be redrawn.

All functions get passed a Mesh object, which contains
the list of vertices and triangles amongst other things.

In particular, m.vertices is a vector that contains
a Vertex object for each vertex of the mesh. The vertex
object simply holds the 3d world coordinates.
The order of a vertex in this vector determines its
index.
m.triangles is a vector that contains one Triangle
object for each triangle of the mesh, which then
simply holds three integers, pointing to the indices
of the vertices that this triangle is made of.

The draw(Mesh m) function below gets called
everytime the screen has to be redrawn (e.g.
when the mouse has been used). For now it calls
a very simple draw function implemented in the
Mesh class, but you can change it in order to
visualize the results of the exercises.
A redraw can be force by calling
glutPostRedisplay(), which has to be done
once your visualization changes.

These two lists and the draw() function
should be all you need to solve the
exercises.

Feel free to implement your own auxilary functions,
you are not restricted to the functions shown below.*/

#include "Vec3D.h"
#include "mesh.h"
#include <GL/glut.h>
#include <list>
using namespace std;

void draw(Mesh& m);
void drawNormals(Mesh& m);
void drawVertexNormals(Mesh& m);
void fillCacheForTriangleNormals(Mesh& m);
void fillCacheForNeighbouringVerticesAndTriangles(Mesh& m);
void fillCacheColourTriangles(Mesh& m);

std::vector<std::vector<int> > vertex_neighbours;
std::vector<std::vector<int> > vertex_triangles;
std::vector<std::pair<Vec3Df, Vec3Df> > triangle_normals; //barycenter and triangle normal pair
std::vector<Vec3Df> colours;

bool draw_normals = false;
bool draw_vertex_normals = false;
bool colour_triangles = false;

/** Function that gets called on keypress 1 */
void myFunction1(Mesh& m) {
	draw_normals = !draw_normals;
	glutPostRedisplay();
}

/** Function that gets called on keypress 2 */
void myFunction2(Mesh& m) {
	draw_vertex_normals = !draw_vertex_normals;
	glutPostRedisplay();
}

/** Function that gets called on keypress 3 */
void myFunction3(Mesh& m) {
	colour_triangles = !colour_triangles;
	glutPostRedisplay();
}

/** Function that gets called on keypress 4 */
void myFunction4(Mesh& m) {

}

/** Function that gets called on keypress 5 */
void myFunction5(Mesh& m) {

}

/** Gets called once the mesh has to be drawn.
Currently calls the meshs draw function, which just
draws the faces in plain white.
With your OpenGL knowledge you could write similar
draw functions that highlight certain vertices,
edges or faces for better visualization of the
results of your functions above. */
void draw(Mesh& m) {
	fillCacheForTriangleNormals(m);
	fillCacheForNeighbouringVerticesAndTriangles(m);
	fillCacheColourTriangles(m);
	if (colour_triangles == false) {
		glColor3f(1.0, 1.0, 1.0);
		m.draw();
	}
	if (colour_triangles == true) m.drawWithColors(colours);

	if (draw_normals == true) drawNormals(m);
	if (draw_vertex_normals == true) drawVertexNormals(m);
}

void fillCacheColourTriangles(Mesh& m) {
	if (colours.size() == 0) {
		colours.resize(m.vertices.size());

		for (int i = 0; i < m.vertices.size(); i++) {
			float number_vertices = m.vertices.size();
			float ratio = 1 / number_vertices;
			colours[i][0] = 1- ratio*i;
			colours[i][1] = ratio*i;
			colours[i][2] = 1;
		}
	}
}

void drawVertexNormals(Mesh& m) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBegin(GL_LINES);
	glColor3f(0.8, 0.4, 0.1);

	for (int i = 0; i < vertex_triangles.size(); i++) {
		Vec3Df vertex = m.vertices[i].p;

		std::vector<int> vertexTriangles = vertex_triangles[i];

		Vec3Df vertexNormal = vertex;

		for (int x = 0; x < vertexTriangles.size(); x++) {
			vertexNormal = vertexNormal + triangle_normals[vertexTriangles[x]].second;
		}

		vertexNormal.normalize();
		Vec3Df endpoint = vertex + vertexNormal * 0.1f;

		glVertex3f(vertex[0], vertex[1], vertex[2]);
		glVertex3f(endpoint[0], endpoint[1], endpoint[2]);

	}
	glEnd();
	glPopAttrib();
}

void drawNormals(Mesh& m) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBegin(GL_LINES);
	glColor3f(0.2, 0.7, 0.3);

	for (int i = 0; i < triangle_normals.size(); i++) {

		Vec3Df barycenter = triangle_normals[i].first;

		Vec3Df norm = triangle_normals[i].second;

		Vec3Df endpoint = barycenter + norm * 0.05f;

		glVertex3f(barycenter[0], barycenter[1], barycenter[2]);
		glVertex3f(endpoint[0], endpoint[1], endpoint[2]);
	}
	glEnd();
	glPopAttrib();
}


void fillCacheForTriangleNormals(Mesh& m) {
	if (triangle_normals.size() == 0) {
		triangle_normals.resize(m.triangles.size());

		for (int i = 0; i < m.triangles.size(); i++) {
			int v1 = m.triangles[i].v[0];
			int v2 = m.triangles[i].v[1];
			int v3 = m.triangles[i].v[2];

			Vec3Df vert1 = m.vertices[v1].p;
			Vec3Df vert2 = m.vertices[v2].p;
			Vec3Df vert3 = m.vertices[v3].p;

			Vec3Df barycenter = ((vert1 + vert2 + vert3) / 3);

			Vec3Df norm = Vec3Df::crossProduct(vert1 - vert2, vert1 - vert3);
			norm.normalize();

			triangle_normals[i].first = barycenter;
			triangle_normals[i].second = norm;
		}
	}
}

void fillCacheForNeighbouringVerticesAndTriangles(Mesh& m) {
	if (vertex_triangles.size() == 0) {
		vertex_neighbours.resize(m.vertices.size());
		vertex_triangles.resize(m.vertices.size());

		for (int c = 0; c < m.vertices.size(); c++) {
			//looping through every vertex
			Vec3Df the_vertex = m.vertices[c].p; //the vertex to see what its neighbours are
			for (int i = 0; i < m.triangles.size(); i++) {
				//looping through every triangle

				//triangle vertices
				int v1 = m.triangles[i].v[0];
				int v2 = m.triangles[i].v[1];
				int v3 = m.triangles[i].v[2];

				if (m.vertices[v1].p == the_vertex || m.vertices[v2].p == the_vertex || m.vertices[v3].p == the_vertex) {
					vertex_triangles[c].push_back(i);
					if (m.vertices[v1].p != the_vertex) vertex_neighbours[c].push_back(v1);
					if (m.vertices[v2].p != the_vertex) vertex_neighbours[c].push_back(v2);
					if (m.vertices[v3].p != the_vertex) vertex_neighbours[c].push_back(v3);
				}
			}
		}
	}
}