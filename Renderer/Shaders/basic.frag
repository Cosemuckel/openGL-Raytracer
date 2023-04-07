#version 330 core

in vec3 gl_FragCoord;
out vec4 fragColor;

struct Material {
	vec3 color;
	float smoothness;
	
	vec4 emission; // color, intensity	
};

struct Sphere {
	vec3 center;
	float radius;

	Material material;
};

struct Triangle {
	vec3 v0;
	float pad0;
	
	vec3 v1;
	float pad1;
	
	vec3 v2;
	float pad2;

	Material material;
};

struct Camera {
	vec3 position;
	float pad0;
	
	vec3 direction;
	float pad1;
	
	vec3 up;
	float pad2;

	vec3 right;
	float pad3;
};

layout(std140) uniform ObjectBuffer {

	vec2 resolution;
	int numSpheres;
	int numTriagngles;

	int maxBounces;
	int numSamples;
	float jitterStrenght;
	float pad1;

	Camera camera;

	Sphere spheres[10];
	Triangle triangles[100];
};

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Intersection {
	Material material;
	float dst;
	vec3 normal;
	vec3 position;
};

Intersection raySphere(Ray ray, Sphere sphere) {	
	Intersection intersection;
	intersection.dst = -1;

	vec3 oc = ray.origin - sphere.center;

	float a = dot(ray.direction, ray.direction);
	float b = 2.0 * dot(oc, ray.direction);
	float c = dot(oc, oc) - sphere.radius * sphere.radius;

	float discriminant = (b * b) - (4.0 * a * c);
		
	float dst = discriminant <= 0.0 ? -1 : (-b - sqrt(discriminant)) / (2.0 * a);

	intersection.dst = dst;
	intersection.position = ray.origin + ray.direction * dst;
	intersection.normal = normalize(intersection.position - sphere.center);
	intersection.material = sphere.material;

	return intersection;
}

Intersection rayTriangle(Ray ray, Triangle triangle) {
	Intersection intersection;
	intersection.dst = -1;

	vec3 e1 = triangle.v2 - triangle.v1;
	vec3 e2 = triangle.v0 - triangle.v2;
	vec3 e3 = triangle.v1 - triangle.v0;

	vec3 n = cross(e1, e2);
	float p = dot(n, triangle.v0);

	float dst = (p - dot(n, ray.origin)) / dot(n, ray.direction);

	vec3 point = ray.origin + dst * ray.direction;

	vec3 c1 = cross(e1, point - triangle.v1);
	vec3 c2 = cross(e2, point - triangle.v2);
	vec3 c3 = cross(e3, point - triangle.v0);
	
	if (dot(n, c1) >= 0 && dot(n, c2) >= 0 && dot(n, c3) >= 0) {
		intersection.dst = dst;
		intersection.material = triangle.material;
		intersection.position = point;
		intersection.normal = normalize(n);
	}
	
	return intersection;	
}

Intersection rayScene(Ray ray) {

	Intersection closestIntersection;
	closestIntersection.dst = -1;

	for (int i = 0; i < numSpheres; ++i) {
		Intersection intersection = raySphere(ray, spheres[i]);

		if (intersection.dst > 0 && (closestIntersection.dst < 0 || intersection.dst < closestIntersection.dst)) {
			closestIntersection = intersection;
		}
	}

	for (int i = 0; i < numTriagngles; ++i) {
		Intersection intersection = rayTriangle(ray, triangles[i]);

		if (intersection.dst > 0 && (closestIntersection.dst < 0 || intersection.dst < closestIntersection.dst)) {
			closestIntersection = intersection;
		}
	}
	
	return closestIntersection;
	
}

vec3 render(Ray ray) {
	Intersection intersection = rayScene(ray);
	
	if (intersection.dst > 0) {
		return intersection.material.color;
	}

	return vec3(0.5, 0.7, 1.0);	
}

float random(inout uint seed) {
	seed = seed * 747796405u + 2891336453u;
	uint res = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
	res = (res >> 22u) ^ res;
	return res / 4294967296.0;
}

vec3 randomDirection(inout uint seed) {
	float z = 1.0 - 2.0 * random(seed);
	float a = 6.28318530718 * random(seed);
	float r = sqrt(1.0 - (z * z));
	return vec3(r * cos(a), r * sin(a), z);
}

vec2 randomInCircle(inout uint seed) {
	float angle = random(seed) * 6.28318530718;
	vec2 point = vec2(cos(angle), sin(angle));
	return point * sqrt(random(seed));
}

vec3 trace(Ray ray, inout uint seed) {
	
	vec3 rayColor = vec3(1.f);
	vec3 totalLight = vec3(0.f);

	for (int i = 0; i < maxBounces; i++) {
		Intersection intersection = rayScene(ray);

		if (intersection.dst < 0.0) {
			totalLight += rayColor * vec3(0.5, 0.7, 1.0) * 0;
			break;
		}

		Material material = intersection.material;

		ray.origin = intersection.position + intersection.normal * 0.001;
		
		vec3 diffuseDir = normalize(intersection.normal + randomDirection(seed));
		vec3 specularDir = reflect(ray.direction, intersection.normal);
		ray.direction = mix(diffuseDir, specularDir, material.smoothness);
		
		vec3 emission = material.emission.xyz * material.emission.w;

		totalLight += rayColor * emission;
		rayColor *= material.color;

	}

	return totalLight;
}

vec3 renderRaytraced(inout uint seed, vec2 world) {
	vec3 color = vec3(0.0, 0.0, 0.0);

	Ray ray;
	ray.origin = camera.position; //The ray starts at the camera position

	for (int i = 0; i < numSamples; ++i) {
		
		vec2 jitter = randomInCircle(seed) * jitterStrenght / resolution.x;
		vec2 jitterWorld = world + jitter;
	
		ray.direction = normalize(camera.direction + jitterWorld.x * camera.right + jitterWorld.y * camera.up);
		color += trace(ray, seed);
	}

	return color / float(numSamples);
}

void main() {

	//UV coordinates ranging from (0,0) in the top left corner of the screen to (1,1) in the bottom right corner of the screen
	vec2 uv = gl_FragCoord.xy / resolution;

	//World coordinates ranging from (-1,-1) in the bottom left corner of the screen to (1,1) in the top right corner of the screen
	vec2 world = (gl_FragCoord.xy - resolution / 2.0) / resolution.y;

	uint pixelIndex = uint(gl_FragCoord.x + gl_FragCoord.y * resolution.x);
	
	vec3 color = renderRaytraced(pixelIndex, world);
	
	//vec3 color = render(ray);
	
	gl_FragColor = vec4(color, 1.0);

}