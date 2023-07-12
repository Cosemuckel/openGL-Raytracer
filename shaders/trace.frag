#version 330 core

out vec4 fragColor;

struct Material {
	vec3 color;
	float smoothness;
	
	vec3 emission;
	float padding;
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

	vec3 edge1;
	float pad3;

	vec3 edge2;
	float pad4;

	vec3 normal;
	float pad5;

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
	int numTriangles;

	int maxBounces;
	int numSamples;
	float jitterStrenght;
	float pad1;

	Camera camera;

	Sphere spheres[2];
	Triangle triangles[30];
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

void raySphere(Ray ray, Sphere sphere, inout float distance, inout bool hit) {

	vec3 oc = ray.origin - sphere.center;

	float a = dot(ray.direction, ray.direction);
	float b = 2.0 * dot(oc, ray.direction);
	float c = dot(oc, oc) - sphere.radius * sphere.radius;

	float discriminant = (b * b) - (4.0 * a * c);
		
	float dst = discriminant <= 0.0 ? -1 : (-b - sqrt(discriminant)) / (2.0 * a);
	
	if (dst > 0 && (distance < 0 || dst < distance)) {
		distance = dst;
		hit = true;
	}

}

void rayTriangle(Ray ray, Triangle triangle, inout float distance, inout bool hit) {
	vec3 edge1 = triangle.edge1;
	vec3 edge2 = triangle.edge2;

	vec3 p = cross(ray.direction, edge2);
	float det = dot(edge1, p);

	if (det < 10e-6) return;

	vec3 t = ray.origin - triangle.v0;
	
	float u = dot(t, p);

	if (det < u || u < 0.0f) return;
	
	float invDet = 1.0f / det;
	u *= invDet;

	vec3 q = cross(t, edge1);	
	
	float v = dot(ray.direction, q) * invDet;

	if (v < 0.0f || u + v > 1.0f) return;

	float dst = dot(edge2, q) * invDet;
	
	if (dst > 0 && (distance < 0 || dst < distance)) {
		distance = dst;
		hit = true;
	}
}


Intersection rayScene(Ray ray) {

	int closestHitSphereIndex = -1;
	float closestHitSphereDistance = -1;
	int closestHitTriangleIndex = -1;
	float closestHitTriangleDistance = -1;
	bool hit = false;

	for (int i = 0; i < numSpheres; ++i) {

		Sphere sphere = spheres[i];
						
		raySphere(ray, sphere, closestHitSphereDistance, hit);

		if (hit) {
			closestHitSphereIndex = i;
			hit = false;
		}
		
	}

	for (int i = 0; i < numTriangles; ++i) {		
	
		Triangle triangle = triangles[i];
	
		rayTriangle(ray, triangle, closestHitTriangleDistance, hit);
	
		if (hit) {
			closestHitTriangleIndex = i;
			hit = false;
		}
	}

	bool triangleHit = closestHitTriangleDistance > 0;

	bool sphereCloser = closestHitSphereDistance > 0 && (!triangleHit || closestHitSphereDistance < closestHitTriangleDistance);

	Intersection intersection;

	if (sphereCloser) {
		intersection.material = spheres[closestHitSphereIndex].material;
		intersection.dst = closestHitSphereDistance;
		intersection.normal = normalize(ray.origin + ray.direction * closestHitSphereDistance - spheres[closestHitSphereIndex].center);
	} else if (triangleHit) {
		intersection.material = triangles[closestHitTriangleIndex].material;
		intersection.dst = closestHitTriangleDistance;
		intersection.normal = triangles[closestHitTriangleIndex].normal;
	} else {
		intersection.dst = -1;
	}	
	
	intersection.position = ray.origin + ray.direction * intersection.dst;
	
	return intersection;	
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
	float r = sqrt(random(seed));
	float a = 6.28318530718 * random(seed);
	return vec2(r * cos(a), r * sin(a));
}

vec3 trace(Ray ray, inout uint seed) {
	
	vec3 rayColor = vec3(1.f);
	vec3 totalLight = vec3(0.f);
	Intersection intersection;
	
	for (int i = 0; i < maxBounces; ++i) {
	
		intersection = rayScene(ray);

		if (intersection.dst < 0.0) {
			break;
		}

		Material material = intersection.material;

		ray.origin = intersection.position + intersection.normal * 0.001;
		
		vec3 diffuseDir = normalize(intersection.normal + randomDirection(seed));
		vec3 specularDir = reflect(ray.direction, intersection.normal);
		ray.direction = mix(diffuseDir, specularDir, material.smoothness);
		
		totalLight += rayColor * material.emission;
		rayColor *= material.color;

	}

	return totalLight;
}

vec3 renderRaytraced(inout uint seed, vec2 world) {
	vec3 color = vec3(0.0);

	Ray ray;
	ray.origin = camera.position; //The ray starts at the camera position

	for (int i = 0; i < numSamples; ++i) {
		 
		vec2 jitter = randomInCircle(seed) * jitterStrenght;
		vec2 jitterWorld = world + jitter;
	
		ray.direction = normalize(camera.direction + jitterWorld.x * camera.right + jitterWorld.y * camera.up);
				
		color += trace(ray, seed);
		
	}

	return color / float(numSamples);
}

int renderMode() {
	float margin = 15.0;
	vec2 size = vec2(200.0, 160.0);

	// A small region in the top left corner is reserved for the UI
	if (gl_FragCoord.x <= margin || (resolution.y - gl_FragCoord.y) <= margin || 
	    gl_FragCoord.x >= size.x + margin || (resolution.y - gl_FragCoord.y) >= size.y + margin) {
		// outside
		return 0;
	} else if (gl_FragCoord.x < margin + 1.0 || (resolution.y - gl_FragCoord.y) < margin + 1.0 || 
	           gl_FragCoord.x > size.x + margin - 1.0 || (resolution.y - gl_FragCoord.y) > size.y + margin - 1.0) {
		// on the border
		return -1;
	} else {
		// inside
		return 1;
	}
}


vec3 hsv2rgb(vec3 c) {
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {

	int mode = renderMode();

	if (mode == 1) {
		float margin = 15.0;
		vec2 size = vec2(200.0, 160.0);
		
		vec2 pos = vec2(gl_FragCoord.x - margin, (resolution.y - gl_FragCoord.y) - margin);
		vec2 uv = pos / size;

		fragColor = vec4(hsv2rgb(vec3(uv.x, sqrt(1 - uv.y), 1.f)), 1.f);
		
		return;
	}
	
	if (mode == -1) {
		fragColor = vec4(0.f);
		return;
	}

	//World coordinates ranging from (-1,-1) in the bottom left corner of the screen to (1,1) in the top right corner of the screen
	vec2 world = (gl_FragCoord.xy - resolution / 2.0) / resolution.y;
	
	uint pixelIndex = uint(gl_FragCoord.x + gl_FragCoord.y * resolution.x);
	
	fragColor = vec4(renderRaytraced(pixelIndex, world), 1.0f);

}