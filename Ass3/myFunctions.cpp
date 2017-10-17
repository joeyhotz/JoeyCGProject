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
#include <string> 
using namespace std;

void draw(Mesh& m);
void drawNormals(Mesh& m);
void drawVertexNormals(Mesh& m);
void fillCacheForTriangleNormals(Mesh& m);
void createNeighbourhoodStructure(Mesh& m);
void fillCacheColourTriangles(Mesh& m);
void numberOfConnectedComponents(Mesh& m);
void addToComponent(std::vector<int>& component, std::vector<int>& vertices_done, int vertex);
void translateComponents();
void fillCacheColourByComponents(Mesh& m);


std::vector<std::vector<int> > vertex_neighbours;
std::vector<std::vector<int> > vertex_triangles;
std::vector<std::vector<int> > connected_components;
std::vector<std::pair<Vec3Df, Vec3Df> > triangle_normals; //barycenter and triangle normal pair
std::vector<Vec3Df> coloursForTriangles;
std::vector<Vec3Df> coloursForColourByComponents;



bool draw_normals = false;
bool draw_vertex_normals = false;
bool colour_triangles = false;
bool colour_by_component = false;

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
	if (colour_by_component) colour_by_component = !colour_by_component;
	glutPostRedisplay();
}

/** Function that gets called on keypress 4 */
void myFunction4(Mesh& m) {
	colour_by_component = !colour_by_component;
	if (colour_triangles) colour_triangles = !colour_triangles;
	glutPostRedisplay();
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
	createNeighbourhoodStructure(m);
	numberOfConnectedComponents(m);
	fillCacheColourTriangles(m);
	fillCacheColourByComponents(m);
	if (colour_triangles == false && colour_by_component == false) {
		glColor3f(1.0, 1.0, 1.0);
		m.draw();
	}
	if (colour_triangles == true) m.drawWithColors(coloursForTriangles);
	if (draw_normals == true) drawNormals(m);
	if (draw_vertex_normals == true) drawVertexNormals(m);
	if (colour_by_component == true) m.drawWithColors(coloursForColourByComponents);
}

void fillCacheColourTriangles(Mesh& m) {
	if (coloursForTriangles.size() == 0) {
		coloursForTriangles.resize(m.vertices.size());

		for (int i = 0; i < m.vertices.size(); i++) {
			float number_vertices = m.vertices.size();
			float ratio = 1 / number_vertices;
			coloursForTriangles[i][0] = 1- ratio*i;
			coloursForTriangles[i][1] = ratio*i;
			coloursForTriangles[i][2] = 1;
		}
	}
}

void fillCacheColourByComponents(Mesh& m) {
	if (coloursForColourByComponents.size() == 0) {
		coloursForColourByComponents.resize(m.vertices.size());
		int count = 0;

		for (int c = 0; c < connected_components.size(); c++) {
			float r = ((double)rand() / (RAND_MAX));
			float g = ((double)rand() / (RAND_MAX));
			float b = ((double)rand() / (RAND_MAX));

			for (int i = 0; i < connected_components[c].size(); i++) {
				//going through each vertex in the component
				coloursForColourByComponents[count][0] = r;
				coloursForColourByComponents[count][1] = g;
				coloursForColourByComponents[count][2] = b;
				count++;
			}
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
		Vec3Df endpoint = vertex + vertexNormal * 0.07f;

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

void createNeighbourhoodStructure(Mesh& m) {
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

void numberOfConnectedComponents(Mesh& m) {
	std::vector<int> vertices_done;
	vertices_done.resize(m.vertices.size());
	fill(vertices_done.begin(), vertices_done.end(), 0);

	if (connected_components.size() == 0) {
		for (int i = 0; i < m.vertices.size(); i++) {
			if (vertices_done[i] == 0) {
				std::vector<int> new_component;
				new_component.resize(m.vertices.size());
				std::fill(new_component.begin(), new_component.end(), 0);
				addToComponent(new_component, vertices_done, i);
				connected_components.push_back(new_component);
			}
		}
	}
	translateComponents();
}

void translateComponents() {
	//this code just translates the list of components that are binary as to wether the vertex at the position of the list is in the component, and replaces it with the actual index

	std::vector<std::vector<int> > new_connected_component_list;
	new_connected_component_list.resize(connected_components.size());

	for (int i = 0; i < connected_components.size(); i++) {
		std::vector<int>& component_to_check = connected_components[i];
		for (int x = 0; x < component_to_check.size(); x++) {
			if (component_to_check[x] == 1) {
				new_connected_component_list[i].push_back(x);
			}
		}
	}
	connected_components = new_connected_component_list;
}

void addToComponent(std::vector<int>& component, std::vector<int>& vertices_done, int vertex) {

	component[vertex] = 1;
	vertices_done[vertex] = 1;

	for (int i = 0; i < vertex_neighbours[vertex].size(); i++) {
		if (component[vertex_neighbours[vertex][i]] == 0) {
			addToComponent(component, vertices_done, vertex_neighbours[vertex][i]);
		}
	}
}