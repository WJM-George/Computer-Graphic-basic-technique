/* **************************
 * CSCI 420
 * Assignment 3 Raytracer
 * Name: <Wu Kam Man> SID:3344-7164-09
 * *************************
*/

#ifdef WIN32
#include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
#include <GL/gl.h>
#include <GL/glut.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <thread>
#include <iostream>
#ifdef WIN32
#define strcasecmp _stricmp
#endif

#include <imageIO.h>

#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100

char* filename = NULL;

// The different display modes.
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

// While solving the homework, it is useful to make the below values smaller for debugging purposes.
// The still images that you need to submit with the homework should be at the below resolution (640x480).
// However, for your own purposes, after you have solved the homework, you can increase those values to obtain higher-resolution images.
#define WIDTH 640
#define HEIGHT 480
#define SCALE_WH (WIDTH/static_cast<double>(HEIGHT))

// The field of view of the camera, in degrees.
#define fov 60.0

// Buffer to store the image when saving it to a JPEG.
unsigned char buffer[HEIGHT][WIDTH][3];

const double EPSILON = 1e-4;

struct Vertex
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double normal[3];
	double shininess;
};

struct Triangle
{
	Vertex v[3];
};

struct Sphere
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double shininess;
	double radius;
};

struct Light
{
	double position[3];
	double color[3];
};

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b);
void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b);
void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);

glm::vec3 cameraPos;
double viewMinX, viewMinY, viewMaxX, viewMaxY;
double viewHeight, viewWidth;

struct Ray {
	glm::vec3 origin;    // ray origin
	glm::vec3 direction; // direction

	Ray(const glm::vec3& orig, const glm::vec3& dir)
		: origin(orig), direction(glm::normalize(dir)) {}

	Ray()
		: origin(glm::vec3(0.0, 0.0, 0.0))
		, direction(glm::vec3(0.0, 0.0, 0.0))
	{}
};

struct Plane {
	glm::vec3 point;   // point on plane
	glm::vec3 normal;  // normal vector of plane
};

// get window showing boundary
void getViewBoundaries() {
	viewMaxX = SCALE_WH * tan(fov / 2 * glm::pi<double>() / 180);
	viewMinX = -viewMaxX;
	viewMaxY = tan(fov / 2 * glm::pi<double>() / 180);
	viewMinY = -viewMaxY;

	viewHeight = viewMaxY - viewMinY;
	viewWidth = viewMaxX - viewMinX;
}

// ray here is from camera to a specific pixel on the object
inline Ray generate_ray(const double x, const double y) {
	// get object pos:
	double&& objectX = viewMinX + (viewWidth * ((x + 0.5) / static_cast<double>(WIDTH)));
	double&& objectY = viewMinY + (viewHeight * ((y + 0.5) / static_cast<double>(HEIGHT)));
	// pos of the ray start point
	glm::vec3 objectPos(objectX, objectY, -1.0f);
	// ray direction
	glm::vec3 direction = glm::normalize(objectPos - cameraPos);
	return Ray(cameraPos, direction);
}

inline glm::vec3 calculate_triangle_normal(const Triangle& triangle) {
	glm::vec3 v0(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]);
	glm::vec3 v1(triangle.v[1].position[0], triangle.v[1].position[1], triangle.v[1].position[2]);
	glm::vec3 v2(triangle.v[2].position[0], triangle.v[2].position[1], triangle.v[2].position[2]);

	// edges
	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;

	// normal
	glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

	return normal;
}

inline double TriangleXYArea(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2) {
	return 0.5 * ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
}

inline double TriangleYZArea(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2) {
	return 0.5 * ((p1.y - p0.y) * (p2.z - p0.z) - (p2.y - p0.y) * (p1.z - p0.z));
}

inline double TriangleXZArea(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2) {
	return 0.5 * ((p1.x - p0.x) * (p2.z - p0.z) - (p2.x - p0.x) * (p1.z - p0.z));
}

//intersection point of a ray and a plane
inline double rayPlaneIntersection(const Ray& ray, const Plane& plane) {
	// if ray parallel to plane
	double dotProduct = glm::dot(plane.normal, ray.direction);
	if (glm::abs(dotProduct) < EPSILON) {
		return -1.0; // no intersect
	}

	// get d:  ax + by + cz = d
	double d = glm::dot(plane.normal, plane.point);
	// get intersection t (where ray intersects plane)
	double t = (d - glm::dot(plane.normal, ray.origin)) / dotProduct;

	return t;
}

inline Plane get_triangle_plane(const Triangle& triangle) {
	glm::vec3 normal = calculate_triangle_normal(triangle);
	glm::vec3 point(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]);
	return { point, normal };
}

struct Intersection {
	glm::vec3 position;        // point
	glm::vec3 color_diffuse;   // diffuse color
	glm::vec3 color_specular;  // specular color
	glm::vec3 normal;          // normal vector
	double shininess;          // shininess

	Intersection()
		: position(0.0), color_diffuse(0.0), color_specular(0.0), normal(0.0), shininess(0.0) {}

	Intersection(const glm::vec3& pos, const glm::vec3& diffuse, const glm::vec3& specular,
		const glm::vec3& norm, double shine)
		: position(pos), color_diffuse(diffuse), color_specular(specular),
		normal(glm::normalize(norm)), shininess(shine) {}

	static Intersection None() {
		return Intersection();
	}

	static bool isNone(const Intersection& intersection) {
		return glm::length(intersection.normal) < EPSILON;
	}
};

// ray and polygon intersection:
Intersection GetTriangleRayIntersection(Ray& ray, const Triangle& triangle) {
	Plane trianglePlane = get_triangle_plane(triangle);
	double t = rayPlaneIntersection(ray, trianglePlane);
	if (t <= EPSILON) {
		return Intersection::None();
	}

	// intersection point and // 3 points in tri
	glm::vec3 intersectionPoint = ray.origin + glm::vec3(t * ray.direction.x, t * ray.direction.y, t * ray.direction.z);
	glm::vec3 v0(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]);
	glm::vec3 v1(triangle.v[1].position[0], triangle.v[1].position[1], triangle.v[1].position[2]);
	glm::vec3 v2(triangle.v[2].position[0], triangle.v[2].position[1], triangle.v[2].position[2]);

	// diff cases
	// Barycentric coordinates
	double alpha;
	double beta;
	double gamma;
	double totalArea;
	if (abs(glm::dot(trianglePlane.normal, glm::vec3(0.0f, 0.0f, 1.0f))) > EPSILON) {
		totalArea = TriangleXYArea(v0, v1, v2);
		alpha = TriangleXYArea(intersectionPoint, v1, v2) / totalArea;
		beta = TriangleXYArea(v0, intersectionPoint, v2) / totalArea;
		gamma = TriangleXYArea(v0, v1, intersectionPoint) / totalArea;
	}
	else if (abs(glm::dot(trianglePlane.normal, glm::vec3(1.0f, 0.0f, 0.0f))) > EPSILON)
	{
		totalArea = TriangleYZArea(v0, v1, v2);
		alpha = TriangleYZArea(intersectionPoint, v1, v2) / totalArea;
		beta = TriangleYZArea(v0, intersectionPoint, v2) / totalArea;
		gamma = TriangleYZArea(v0, v1, intersectionPoint) / totalArea;
	}
	else {
		totalArea = TriangleXZArea(v0, v1, v2);
		alpha = TriangleXZArea(intersectionPoint, v1, v2) / totalArea;
		beta = TriangleXZArea(v0, intersectionPoint, v2) / totalArea;
		gamma = TriangleXZArea(v0, v1, intersectionPoint) / totalArea;
	}

	if (alpha < 0.0 || beta < 0.0 || gamma < 0.0 || alpha + beta + gamma > 1 + EPSILON) {
		return Intersection::None();
	}

	// make each alpha,beta,gamma to corresponding attribute
	glm::vec3 resultPoint = glm::vec3(alpha * triangle.v[0].position[0], alpha * triangle.v[0].position[1], alpha * triangle.v[0].position[2]) +
		glm::vec3(beta * triangle.v[1].position[0], beta * triangle.v[1].position[1], beta * triangle.v[1].position[2]) +
		glm::vec3(gamma * triangle.v[2].position[0], gamma * triangle.v[2].position[1], gamma * triangle.v[2].position[2]);

	glm::vec3 resultDiffuse = glm::vec3(alpha * triangle.v[0].color_diffuse[0], alpha * triangle.v[0].color_diffuse[1], alpha * triangle.v[0].color_diffuse[2]) +
		glm::vec3(beta * triangle.v[1].color_diffuse[0], beta * triangle.v[1].color_diffuse[1], beta * triangle.v[1].color_diffuse[2]) +
		glm::vec3(gamma * triangle.v[2].color_diffuse[0], gamma * triangle.v[2].color_diffuse[1], gamma * triangle.v[2].color_diffuse[2]);

	glm::vec3 resultSpecular = glm::vec3(alpha * triangle.v[0].color_specular[0], alpha * triangle.v[0].color_specular[1], alpha * triangle.v[0].color_specular[2]) +
		glm::vec3(beta * triangle.v[1].color_specular[0], beta * triangle.v[1].color_specular[1], beta * triangle.v[1].color_specular[2]) +
		glm::vec3(gamma * triangle.v[2].color_specular[0], gamma * triangle.v[2].color_specular[1], gamma * triangle.v[2].color_specular[2]);

	glm::vec3 resultNormal = glm::vec3(alpha * triangle.v[0].normal[0], alpha * triangle.v[0].normal[1], alpha * triangle.v[0].normal[2]) +
		glm::vec3(beta * triangle.v[1].normal[0], beta * triangle.v[1].normal[1], beta * triangle.v[1].normal[2]) +
		glm::vec3(gamma * triangle.v[2].normal[0], gamma * triangle.v[2].normal[1], gamma * triangle.v[2].normal[2]);

	double resultShininess = alpha * triangle.v[0].shininess +
		beta * triangle.v[1].shininess +
		gamma * triangle.v[2].shininess;

	return Intersection(resultPoint, resultDiffuse, resultSpecular, resultNormal, resultShininess);
}

Intersection GetSphereRayIntersection(Ray& ray, const Sphere& sphere) {
	glm::vec3 rayPos = ray.origin;
	glm::vec3 rayDirection = glm::normalize(ray.direction);
	glm::vec3 spherePos = glm::vec3(sphere.position[0], sphere.position[1], sphere.position[2]);
	const double radius = sphere.radius;

	glm::vec3 rayToSphere = rayPos - spherePos;

	const double squareRadius = radius * radius;
	const double dotOfRayAndRToS = static_cast<double>(glm::dot(rayDirection, rayToSphere));

	// sphere at back of ray start point, or surround start point:
	if (dotOfRayAndRToS > 0 || static_cast<double>(glm::dot(rayToSphere, rayToSphere)) < squareRadius) {
		return Intersection::None();
	}

	// shortest side from ray to sphere center:
	glm::vec3 verticalSide = rayToSphere - glm::vec3(dotOfRayAndRToS * rayDirection[0], dotOfRayAndRToS * rayDirection[1], dotOfRayAndRToS * rayDirection[2]);
	const double verticalSideSquare = static_cast<double>(glm::dot(verticalSide, verticalSide));
	if (verticalSideSquare > squareRadius) {
		return Intersection::None();
	}

	// dist : plane <-> ray in and exit the sphere:
	const double dist = glm::sqrt(squareRadius - verticalSideSquare);
	// intersect point to sphere center:
	glm::vec3 intersectVec = verticalSide - glm::vec3(rayDirection.x * dist, rayDirection.y * dist, rayDirection.z * dist);
	glm::vec3 resultNormal_sphere = glm::normalize(glm::vec3(intersectVec.x / radius, intersectVec.y / radius, intersectVec.z / radius));

	glm::vec3 resultDiffuse_sphere = glm::vec3(sphere.color_diffuse[0], sphere.color_diffuse[1], sphere.color_diffuse[2]);
	glm::vec3 resultSpecular_sphere = glm::vec3(sphere.color_specular[0], sphere.color_specular[1], sphere.color_specular[2]);

	return Intersection(spherePos + intersectVec, resultDiffuse_sphere, resultSpecular_sphere, resultNormal_sphere, sphere.shininess);
}

int nearestObjCharacter(Ray& ray, Intersection& outputIntersection) {
	// nearest intersection distance
	double nearestDist = static_cast<double>(FLT_MAX);
	int objectID = -1; // indicate which object is it
	outputIntersection = Intersection::None();

	for (int i = 0; i < num_spheres; ++i) {
		Intersection intersection = GetSphereRayIntersection(ray, spheres[i]);
		if (!(Intersection::isNone(intersection))) {
			glm::vec3 displacement = intersection.position - ray.origin;
			double dist = glm::length(displacement);

			if (dist < nearestDist) { // if dound make it nearest dist
				nearestDist = dist;
				objectID = i;
				outputIntersection = std::move(intersection);
			}
		}
	}

	for (int i = 0; i < num_triangles; ++i) {
		Intersection intersection = GetTriangleRayIntersection(ray, triangles[i]);
		if (!(Intersection::isNone(intersection))) {
			glm::vec3 displacement = intersection.position - ray.origin;
			double dist = glm::length(displacement);

			if (dist < nearestDist) {
				nearestDist = dist;
				objectID = i + num_spheres; // to differ from the triangles
				outputIntersection = std::move(intersection);
			}
		}
	}

	return objectID;
}

// soft shadow setting
double computeShadowFactor(const glm::vec3& lightPos, const glm::vec3& intersectionPos, const glm::vec3& intersectionNormal, int shadowSamples) {
	double shadowFactor = 0.0;

	for (int s = 0; s < shadowSamples; ++s) {
		glm::vec3 multiLightPos = lightPos + glm::vec3(
			(rand() % 100 / 100.0f - 0.5f) * 0.4f, // x
			(rand() % 100 / 100.0f - 0.5f) * 0.4f, // y
			(rand() % 100 / 100.0f - 0.5f) * 0.4f  // z
		);

		Intersection shadowIntersection;
		//Ray shadowRay(intersectionPos + glm::normalize(multiLightPos - intersectionPos),
			//multiLightPos - intersectionPos);
		glm::vec3 shadowDir = glm::normalize(multiLightPos - intersectionPos);
		Ray shadowRay(intersectionPos + glm::vec3(shadowDir.x * EPSILON, shadowDir.y * EPSILON, shadowDir.z * EPSILON), shadowDir);

		int objShadowID = nearestObjCharacter(shadowRay, shadowIntersection);

		double distToFirstObj = glm::length(multiLightPos - shadowIntersection.position);
		double distToTargetSurface = glm::length(multiLightPos - intersectionPos);

		// Check if point is in shadow
		if (objShadowID == -1 || distToFirstObj >= distToTargetSurface - EPSILON) {
			shadowFactor += 1.0f;
		}
	}

	return shadowFactor / shadowSamples;
}

void PhongShading(double colorOut[3], const Intersection& intersection, int depth) {
	if (depth <= 0) return;

	glm::vec3 normal = glm::normalize(intersection.normal);
	glm::vec3 viewVec = glm::normalize(cameraPos - intersection.position);

	for (int i = 0; i < num_lights; ++i) {
		Light& light = lights[i];

		double a = 1;
		double b = 0;
		double c = 0;

		glm::vec3 lightPos(light.position[0], light.position[1], light.position[2]);

		// soft shadow setting
		int shadowSamples = 9; // # of rays
		double shadowFactor = computeShadowFactor(lightPos, intersection.position, intersection.normal, shadowSamples);

		Intersection shadowIntersection;
		Ray pointToLight(intersection.position, lightPos - intersection.position);

		int objShadowID = nearestObjCharacter(pointToLight, shadowIntersection);
		double distLightToFirstObj = glm::sqrt(glm::dot(lightPos - shadowIntersection.position,
			lightPos - shadowIntersection.position));
		double distLightToTargerSurf = glm::sqrt(glm::dot(lightPos - intersection.position,
			lightPos - intersection.position));

		double attenuation = 1.0 / (a + b * distLightToTargerSurf + c * distLightToTargerSurf * distLightToTargerSurf);

		// point in shadow no phong
		if (objShadowID != -1 &&
			distLightToFirstObj < distLightToTargerSurf - EPSILON) {
			continue;
		}

		// diffuse:
		glm::vec3 l = glm::normalize(lightPos - intersection.position);
		double dotNL = glm::dot(normal, l);
		dotNL = glm::max(dotNL, 0.0); // non negative
		for (int j = 0; j < 3; ++j) {
			colorOut[j] += shadowFactor * attenuation * intersection.color_diffuse[j] * dotNL * light.color[j];
		}

		// specular:
		glm::vec3 partOfRef = glm::vec3(2.0 * dotNL * normal.x, 2.0 * dotNL * normal.y, 2.0 * dotNL * normal.z);
		glm::vec3 reflect = glm::normalize(partOfRef - l);
		double dotVR = glm::dot(viewVec, reflect);
		dotVR = glm::max(dotVR, 0.0);
		for (int j = 0; j < 3; ++j) {
			colorOut[j] += shadowFactor * attenuation * intersection.color_specular[j] * pow(dotVR, intersection.shininess) * light.color[j];
		}
	}
	// ambient
	for (int j = 0; j < 3; ++j) {
		colorOut[j] += ambient_light[j];
	}

	// reflective:
	glm::vec3 reflectedDir = glm::reflect(-viewVec, normal);
	Ray reflectedRay(intersection.position + glm::vec3(reflectedDir.x * EPSILON, reflectedDir.y * EPSILON, reflectedDir.z * EPSILON), reflectedDir);

	Intersection reflectedIntersection;
	int objID = nearestObjCharacter(reflectedRay, reflectedIntersection);

	double reflectedColor[3] = { colorOut[0], colorOut[1], colorOut[2] };
	if (objID != -1) {
		PhongShading(reflectedColor, reflectedIntersection, depth - 1);
	}

	//intersection.color_specular[j]
	for (int j = 0; j < 3; ++j) {
		colorOut[j] = (1.0 - intersection.color_specular[j]) * colorOut[j]
			+ intersection.color_specular[j] * reflectedColor[j];
	}
}

bool isAntialiasing = true;

void draw_scene()
{
	getViewBoundaries();

	int sampleGridSize = isAntialiasing ? 3 : 1; // 3x3 supersampling
	float sampleOffset = 1.0f / sampleGridSize;  // offset for sampling ray

	for (unsigned int x = 0; x < WIDTH; x++)
	{
		glPointSize(2.0);
		glBegin(GL_POINTS);

		for (unsigned int y = 0; y < HEIGHT; y++)
		{
			glm::vec3 accumulatedColor(0.0f, 0.0f, 0.0f);

			for (int i = 0; i < sampleGridSize; i++)
			{
				for (int j = 0; j < sampleGridSize; j++)
				{
					float sampleX = x + (i + 0.5f) * sampleOffset; // sub-pixel x-coordinate
					float sampleY = y + (j + 0.5f) * sampleOffset; // sub-pixel y-coordinate

					Ray cameraRay = generate_ray(sampleX, sampleY);
					Intersection intersection;

					int objectID = nearestObjCharacter(cameraRay, intersection);
					glm::vec3 color(1.0f, 1.0f, 1.0f); // background color

					if (objectID != -1)
					{
						double colorOut[3] = { 0.0, 0.0, 0.0 }; //shadow black
						int recurrsionDepth = 5;
						PhongShading(colorOut, intersection, recurrsionDepth);
						color = glm::vec3(colorOut[0], colorOut[1], colorOut[2]);
					}

					accumulatedColor += color;
				}
			}

			//do average
			glm::vec3 color = accumulatedColor / float(sampleGridSize * sampleGridSize);

			// RGB
			double r = color.r * 255;
			double g = color.g * 255;
			double b = color.b * 255;
			if (r > 255) { r = 255; }
			else if (r < 0) { r = 0; }
			if (b > 255) { b = 255; }
			else if (b < 0) { b = 0; }
			if (g > 255) { g = 255; }
			else if (g < 0) { g = 0; }

			plot_pixel(x, y, r, g, b);
		}

		glEnd();
		glFlush();
	}

	printf("Ray tracing completed.\n");
	fflush(stdout);
}

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
	glVertex2i(x, y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	buffer[y][x][0] = r;
	buffer[y][x][1] = g;
	buffer[y][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	plot_pixel_display(x, y, r, g, b);
	if (mode == MODE_JPEG)
		plot_pixel_jpeg(x, y, r, g, b);
}

void save_jpg()
{
	printf("Saving JPEG file: %s\n", filename);

	ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
	if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
		printf("Error in saving\n");
	else
		printf("File saved successfully\n");
}

void parse_check(const char* expected, char* found)
{
	if (strcasecmp(expected, found))
	{
		printf("Expected '%s ' found '%s '\n", expected, found);
		printf("Parsing error; abnormal program abortion.\n");
		exit(0);
	}
}

void parse_doubles(FILE* file, const char* check, double p[3])
{
	char str[100];
	fscanf(file, "%s", str);
	parse_check(check, str);
	fscanf(file, "%lf %lf %lf", &p[0], &p[1], &p[2]);
	printf("%s %lf %lf %lf\n", check, p[0], p[1], p[2]);
}

void parse_rad(FILE* file, double* r)
{
	char str[100];
	fscanf(file, "%s", str);
	parse_check("rad:", str);
	fscanf(file, "%lf", r);
	printf("rad: %f\n", *r);
}

void parse_shi(FILE* file, double* shi)
{
	char s[100];
	fscanf(file, "%s", s);
	parse_check("shi:", s);
	fscanf(file, "%lf", shi);
	printf("shi: %f\n", *shi);
}

int loadScene(char* argv)
{
	FILE* file = fopen(argv, "r");
	if (!file)
	{
		printf("Unable to open input file %s. Program exiting.\n", argv);
		exit(0);
	}

	int number_of_objects;
	char type[50];
	Triangle t;
	Sphere s;
	Light l;
	fscanf(file, "%i", &number_of_objects);

	printf("number of objects: %i\n", number_of_objects);

	parse_doubles(file, "amb:", ambient_light);

	for (int i = 0; i < number_of_objects; i++)
	{
		fscanf(file, "%s\n", type);
		printf("%s\n", type);
		if (strcasecmp(type, "triangle") == 0)
		{
			printf("found triangle\n");
			for (int j = 0; j < 3; j++)
			{
				parse_doubles(file, "pos:", t.v[j].position);
				parse_doubles(file, "nor:", t.v[j].normal);
				parse_doubles(file, "dif:", t.v[j].color_diffuse);
				parse_doubles(file, "spe:", t.v[j].color_specular);
				parse_shi(file, &t.v[j].shininess);
			}

			if (num_triangles == MAX_TRIANGLES)
			{
				printf("too many triangles, you should increase MAX_TRIANGLES!\n");
				exit(0);
			}
			triangles[num_triangles++] = t;
		}
		else if (strcasecmp(type, "sphere") == 0)
		{
			printf("found sphere\n");

			parse_doubles(file, "pos:", s.position);
			parse_rad(file, &s.radius);
			parse_doubles(file, "dif:", s.color_diffuse);
			parse_doubles(file, "spe:", s.color_specular);
			parse_shi(file, &s.shininess);

			if (num_spheres == MAX_SPHERES)
			{
				printf("too many spheres, you should increase MAX_SPHERES!\n");
				exit(0);
			}
			spheres[num_spheres++] = s;
		}
		else if (strcasecmp(type, "light") == 0)
		{
			printf("found light\n");
			parse_doubles(file, "pos:", l.position);
			parse_doubles(file, "col:", l.color);

			if (num_lights == MAX_LIGHTS)
			{
				printf("too many lights, you should increase MAX_LIGHTS!\n");
				exit(0);
			}
			lights[num_lights++] = l;
		}
		else
		{
			printf("unknown type in scene description:\n%s\n", type);
			exit(0);
		}
	}
	return 0;
}

void display()
{
}

void init()
{
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, WIDTH, 0, HEIGHT, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
	// Hack to make it only draw once.
	static int once = 0;
	if (!once)
	{
		draw_scene();
		if (mode == MODE_JPEG)
			save_jpg();
	}
	once = 1;
}

int main(int argc, char** argv)
{
	if ((argc < 2) || (argc > 3))
	{
		printf("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
		exit(0);
	}
	if (argc == 3)
	{
		mode = MODE_JPEG;
		filename = argv[2];
	}
	else if (argc == 2)
		mode = MODE_DISPLAY;

	glutInit(&argc, argv);
	loadScene(argv[1]);

	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WIDTH, HEIGHT);
	int window = glutCreateWindow("Ray Tracer");
#ifdef __APPLE__
	// This is needed on recent Mac OS X versions to correctly display the window.
	glutReshapeWindow(WIDTH - 1, HEIGHT - 1);
#endif
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	init();
	glutMainLoop();
}

