#pragma once
#include <vector>
#include <memory>
#include "GameEntity.h"
#include "World.h"
#include <glm/glm.hpp>
#include <iostream>
#include "ChewyEntity.h"

static pair<bool, float> obb_ray(glm::vec3 origin, glm::vec3 direction, EntityBox bb)
{
	glm::vec3 center = bb.center;
	glm::vec3 h = glm::vec3(bb.half_width, bb.half_height, bb.half_depth);

	float tMin = std::numeric_limits<float>::min();
	float tMax = std::numeric_limits<float>::max();
	glm::vec3 p = center - origin;
	for (int i = 0; i < 3; i++)
	{
		glm::vec3 ai(0.f);
		if (i == 0)
			ai = glm::vec3(1.f, 0.f, 0.f);
		if (i == 1)
			ai = glm::vec3(0.f, 1.f, 0.f);
		if (i == 2)
			ai = glm::vec3(0.f, 0.f, 1.f);

		float e = glm::dot(ai, p);
		float f = glm::dot(ai, direction);

		if (abs(f) > 0.0001f)
		{
			float t1 = (e + h[i]) / f;
			float t2 = (e - h[i]) / f;

			if (t1 > t2)
			{
				//swap
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}

			tMin = fmax(tMin, t1);
			tMax = fmin(tMax, t2);
			if (tMin > tMax || tMax < 0) {
				return std::pair<bool, float>(false, 0);
			}
		}
		else if (-1.0f * e - h[i] > 0 || h[i] - e < 0)
			return std::pair<bool, float>(false, 0);
	}
	if (tMin > 0)
		return std::pair<bool, float>(true, tMin);
	else
		return std::pair<bool, float>(false, tMax);
}

static glm::vec4 normalize_plane(glm::vec4& p1)
{
	glm::vec4 p1_c(p1);
	float mag = glm::length(glm::vec3(p1_c));
	return p1_c / mag;
}

static int PlaneAABBIntersect(EntityBox bb, glm::vec4& p)
{

    glm::vec3 c = bb.center;
	glm::vec3 h = glm::vec3(bb.half_width, bb.half_height, bb.half_depth); // CHECK HERE IF FRUSTUM BREAKS

	float e = h.x * fabs(p.x) + h.y * fabs(p.y) + h.z * fabs(p.z);
	float s = glm::dot(c, glm::vec3(p)) + p.w;

	if ((s - e) > 0)
		return 0;
	if ((s + e) > 0)
		return 1;
	return 2;
}

static std::vector<GameEntity*> get_objects_in_view(std::vector<GameEntity*> entities, glm::mat4& view_mat, bool chewy_check = false)
{
	glm::vec4 p_planes[6];
	std::vector<GameEntity*> toReturn;
	glm::mat4 combo_mat;
	if (chewy_check)
		combo_mat = guard_projection * view_mat;
	else
		combo_mat = projection * view_mat;

	//left clipping plane
	p_planes[0].x = combo_mat[0][3] + combo_mat[0][0];
	p_planes[0].y = combo_mat[1][3] + combo_mat[1][0];
	p_planes[0].z = combo_mat[2][3] + combo_mat[2][0];
	p_planes[0].w = combo_mat[3][3] + combo_mat[3][0];
	// Right clipping plane
	p_planes[1].x = combo_mat[0][3] - combo_mat[0][0];
	p_planes[1].y = combo_mat[1][3] - combo_mat[1][0];
	p_planes[1].z = combo_mat[2][3] - combo_mat[2][0];
	p_planes[1].w = combo_mat[3][3] - combo_mat[3][0];
	// Top clipping plane
	p_planes[2].x = combo_mat[0][3] - combo_mat[0][1];
	p_planes[2].y = combo_mat[1][3] - combo_mat[1][1];
	p_planes[2].z = combo_mat[2][3] - combo_mat[2][1];
	p_planes[2].w = combo_mat[3][3] - combo_mat[3][1];
	// Bottom clipping plane
	p_planes[3].x = combo_mat[0][3] + combo_mat[0][1];
	p_planes[3].y = combo_mat[1][3] + combo_mat[1][1];
	p_planes[3].z = combo_mat[2][3] + combo_mat[2][1];
	p_planes[3].w = combo_mat[3][3] + combo_mat[3][1];
	// Near clipping plane
	p_planes[4].x = combo_mat[0][3] + combo_mat[0][2];
	p_planes[4].y = combo_mat[1][3] + combo_mat[1][2];
	p_planes[4].z = combo_mat[2][3] + combo_mat[2][2];
	p_planes[4].w = combo_mat[3][3] + combo_mat[3][2];
	// Far clipping plane
	p_planes[5].x = combo_mat[0][3] - combo_mat[0][2];
	p_planes[5].y = combo_mat[1][3] - combo_mat[1][2];
	p_planes[5].z = combo_mat[2][3] - combo_mat[2][2];
	p_planes[5].w = combo_mat[3][3] - combo_mat[3][2];

	bool chewy_not_found = 0;

	for (int i = 0; i < entities.size(); i++)
	{
		bool in_frustrum = true;
		GameEntity* entity = entities.at(i);
		if ((chewy_check && HIDE_BEHIND(entity->list)) || !chewy_check)
		{
			for (int j = 0; j < 6; j++)
			{
				if (PlaneAABBIntersect(entity->bounding_box, p_planes[j]) == 2)
				{
					in_frustrum = false;
					break;
				}
			}
			if (in_frustrum)
			{
				toReturn.push_back(entity);
			}
			else
			{
				ChewyEntity* chewy = dynamic_cast<ChewyEntity*>(entity);
				if (chewy != nullptr)
				{
					chewy_not_found = true;
				}
			}
		}
	}
	if (chewy_check && chewy_not_found)
		toReturn.clear();
	return toReturn;
}