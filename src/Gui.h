#pragma once

#include <glm/glm.hpp>

//There is a small color picker in the top left corner of the window, it has a size of 1 / 4 of the window width and 1 / 4 of the window height.
//And a small margin of 1 / 32 of the window width in both directions.
//The picker represents hue in the x direction and saturation in the y direction.

glm::vec3 hsv2rgb(const glm::vec3& rgb) {
	float r = rgb.x;
	float g = rgb.y;
	float b = rgb.z;

	float h = r;
	float s = g;
	float v = b;

	int i = int(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);

	switch (i % 6) {
	case 0: r = v, g = t, b = p; break;
	case 1: r = q, g = v, b = p; break;
	case 2: r = p, g = v, b = t; break;
	case 3: r = p, g = q, b = v; break;
	case 4: r = t, g = p, b = v; break;
	case 5: r = v, g = p, b = q; break;
	}

	return glm::vec3(r, g, b);
}

bool selectColor(const double mouseX, const double mouseY, glm::vec3& color) {
	const static float margin = 15.f;
	const static float pickerWidth = 200.f;
	const static float pickerHeight = 160.f;

	//Check if the mouse is in the color picker
	if (mouseX < margin || mouseX > margin + pickerWidth || mouseY < margin || mouseY > margin + pickerHeight) {
		return false;
	}

	//Calculate the color
	float hue = (mouseX - margin) / pickerWidth;
	float saturation = (mouseY - margin) / pickerHeight;
	
	color = hsv2rgb(glm::vec3(hue, 1.f - saturation, 1.0f));
	
	return true;
}