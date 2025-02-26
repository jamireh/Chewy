#pragma once
#include <glm/glm.hpp>
#include <vector>


class GuardPathingComponent
{
public:
	std::vector<glm::vec3> control_points;

	GuardPathingComponent(std::vector<glm::vec3> control_points, float move_speed, bool linear_curve);
	GuardPathingComponent() {}
	glm::vec3 get_direction();

	void reverse();
private:
	int current_curve;
	bool linear_curve;
	float current_distance;
	float move_speed;
	float current_time;
	float time_elapsed;
	void change_path();

	glm::vec3 get_rom_catmull_direction();
	glm::vec3 get_linear_direction();
};