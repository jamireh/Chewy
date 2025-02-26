#define _USE_MATH_DEFINES
#include <cmath>

#include "GameEntity.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/detail/func_vector_relational.hpp>
#include "World.h"
#include "AudioManager.h"
#include "main.h"

#define ANIMATE_TIME 3.f

using namespace glm;
using namespace std;

GameEntity::GameEntity(glm::vec3 position, MeshSet* mesh, bool collision_response) : mesh(mesh), rotations(glm::vec3(0.f, 0.f, 0.f)),
collision_response(collision_response)
{
    setup_entity_box();
    setPosition(position + vec3(0.f, bounding_box.half_height, 0.f));
    list = SET_DRAW(list);
    list = SET_OCTTREE(list);
    inner_bounding_box = bounding_box;

	/*boneTransformations.resize(mesh->getMeshes().size());
	for (int i = 0; i < boneTransformations.size(); i++)
	{
		boneTransformations[i].resize(mesh->getMeshes()[i]->boneWeights.size());
	}*/
	list = SET_WALL(list);
}

GameEntity::~GameEntity() {
}


vec3 GameEntity::turnAngle(vec3 cartesian)
{
    vec3 rot_angles(0, 0, 0);

    if (cartesian.x < 0)
        rot_angles.y = -1.0f * atan(cartesian.z / cartesian.x);
    else
        rot_angles.y = atan(cartesian.z / -cartesian.x) + M_PI;

    rot_angles.y -= M_PI / 2.f;

    return rot_angles;
}

float GameEntity::getScale()
{
    return scale.x;
}

void GameEntity::setScale(float entScale)
{
    scale = vec3(entScale, entScale, entScale);
    float y_offset = bounding_box.center.y - bounding_box.half_height;
    bounding_box.half_width *= scale.x;
    bounding_box.half_height *= scale.y;
    bounding_box.half_depth *= scale.z;
    bounding_box.center.y = bounding_box.half_height + y_offset;
	validAlignedModelMat = false;
	validModelMat = false;
}

void GameEntity::setScale(glm::vec3 entScale)
{
    float y_offset = bounding_box.center.y - bounding_box.half_height;
    bounding_box.half_width /= scale.x;
    bounding_box.half_height /= scale.y;
    bounding_box.half_depth /= scale.z;
    scale = entScale;
    bounding_box.half_width *= scale.x;
    bounding_box.half_height *= scale.y;
    bounding_box.half_depth *= scale.z;
    bounding_box.center.y = bounding_box.half_height + y_offset;
    validAlignedModelMat = false;
    validModelMat = false;
}

glm::vec3 GameEntity::getRotations()
{
    return rotations;
}

void GameEntity::setRotations(glm::vec3 rots)
{
    rotations = rots;
	validRotModelMat = false;
	validModelMat = false;
}

void GameEntity::swap_bounding_box_width_depth()
{
    if (!setup_inner)
    {
        float save_width = bounding_box.half_width;
        bounding_box.half_width = bounding_box.half_depth;
        bounding_box.half_depth = save_width;
    }
    else
    {
        float save_width = inner_bounding_box.half_width;
        inner_bounding_box.half_width = inner_bounding_box.half_depth;
        inner_bounding_box.half_depth = save_width;
    }
}

glm::vec3 GameEntity::getPosition()
{
    return bounding_box.center;
}

void GameEntity::setPosition(glm::vec3 position)
{
    bounding_box.center = position;
    inner_bounding_box.center = position;
	validAlignedModelMat = false;
	validModelMat = false;
}

void GameEntity::setup_entity_box()
{
    vector<Mesh*> meshes = mesh->getMeshes();
    vec3 lower_bound(FLT_MAX, FLT_MAX, FLT_MAX);
    vec3 upper_bound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int i = 0; i < meshes.size(); i++)
    {
        pair<vec3, vec3> bounds = meshes.at(i)->get_lower_and_upper_bounds();
        vec3 m_lower_bound(bounds.first);
        vec3 m_upper_bound(bounds.second);

        vec3 less = lessThan(m_lower_bound, lower_bound);
        vec3 greater = greaterThan(m_upper_bound, upper_bound);
        lower_bound.x = less.x ? m_lower_bound.x : lower_bound.x;
        lower_bound.y = less.y ? m_lower_bound.y : lower_bound.y;
        lower_bound.z = less.z ? m_lower_bound.z : lower_bound.z;
        upper_bound.x = greater.x ? m_upper_bound.x : upper_bound.x;
        upper_bound.y = greater.y ? m_upper_bound.y : upper_bound.y;
        upper_bound.z = greater.z ? m_upper_bound.z : upper_bound.z;
    }

    bounding_box.half_width = glm::distance(lower_bound.x, upper_bound.x) / 2.f;
    bounding_box.half_height = glm::distance(lower_bound.y, upper_bound.y) / 2.f;
    bounding_box.half_depth = glm::distance(lower_bound.z, upper_bound.z) / 2.f;
    inner_bounding_box = bounding_box;
}

void GameEntity::setup_inner_entity_box(MeshSet* mesh)
{
    vector<Mesh*> meshes = mesh->getMeshes();
    vec3 lower_bound(FLT_MAX, FLT_MAX, FLT_MAX);
    vec3 upper_bound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int i = 0; i < meshes.size(); i++)
    {
        pair<vec3, vec3> bounds = meshes.at(i)->get_lower_and_upper_bounds();
        vec3 m_lower_bound(bounds.first);
        vec3 m_upper_bound(bounds.second);

        vec3 less = lessThan(m_lower_bound, lower_bound);
        vec3 greater = greaterThan(m_upper_bound, upper_bound);
        lower_bound.x = less.x ? m_lower_bound.x : lower_bound.x;
        lower_bound.y = less.y ? m_lower_bound.y : lower_bound.y;
        lower_bound.z = less.z ? m_lower_bound.z : lower_bound.z;
        upper_bound.x = greater.x ? m_upper_bound.x : upper_bound.x;
        upper_bound.y = greater.y ? m_upper_bound.y : upper_bound.y;
        upper_bound.z = greater.z ? m_upper_bound.z : upper_bound.z;
    }

    // correct for previous bounding box center
    inner_bounding_box.center.y -= inner_bounding_box.half_height;

    inner_bounding_box.half_width = glm::distance(lower_bound.x, upper_bound.x) / 2.f;
    inner_bounding_box.half_height = glm::distance(lower_bound.y, upper_bound.y) / 2.f;
    inner_bounding_box.half_depth = glm::distance(lower_bound.z, upper_bound.z) / 2.f;

    // new center
    inner_bounding_box.center.y += inner_bounding_box.half_height;

	setup_inner = true;
}

void GameEntity::setup_entity_box(MeshSet* mesh)
{
    vector<Mesh*> meshes = mesh->getMeshes();
    vec3 lower_bound(FLT_MAX, FLT_MAX, FLT_MAX);
    vec3 upper_bound(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int i = 0; i < meshes.size(); i++)
    {
        pair<vec3, vec3> bounds = meshes.at(i)->get_lower_and_upper_bounds();
        vec3 m_lower_bound(bounds.first);
        vec3 m_upper_bound(bounds.second);

        vec3 less = lessThan(m_lower_bound, lower_bound);
        vec3 greater = greaterThan(m_upper_bound, upper_bound);
        lower_bound.x = less.x ? m_lower_bound.x : lower_bound.x;
        lower_bound.y = less.y ? m_lower_bound.y : lower_bound.y;
        lower_bound.z = less.z ? m_lower_bound.z : lower_bound.z;
        upper_bound.x = greater.x ? m_upper_bound.x : upper_bound.x;
        upper_bound.y = greater.y ? m_upper_bound.y : upper_bound.y;
        upper_bound.z = greater.z ? m_upper_bound.z : upper_bound.z;
    }

    // correct for previous bounding box center
    bounding_box.center.y -= bounding_box.half_height;

    bounding_box.half_width = glm::distance(lower_bound.x, upper_bound.x) / 2.f;
    bounding_box.half_height = glm::distance(lower_bound.y, upper_bound.y) / 2.f;
    bounding_box.half_depth = glm::distance(lower_bound.z, upper_bound.z) / 2.f;

    // new center
    bounding_box.center.y += bounding_box.half_height;
}

void GameEntity::collision(GameEntity* entity)
{
    
}

glm::mat4 GameEntity::getModelMat()
{
	if (!validModelMat) {
        modelMat = getAlignedModelMat() * getRotMat();
		validModelMat = true;
	}

    return modelMat;
}

glm::mat4 GameEntity::getAlignedModelMat()
{
	if (!validAlignedModelMat) {
		mat4 model_trans;
		if (setup_inner)
		{
			model_trans = translate(mat4(1.0f), inner_bounding_box.center - vec3(0.f, inner_bounding_box.half_height, 0.f));
		}
		else
		{
			model_trans = translate(mat4(1.0f), bounding_box.center - vec3(0.f, bounding_box.half_height, 0.f));
		}
		mat4 model_scale = glm::scale(mat4(1.0f), scale);

		alignedModelMat = model_trans * model_scale;
		validAlignedModelMat = true;
	}

    return alignedModelMat;
}

glm::mat4 GameEntity::getRotMat()
{
	if (!validRotModelMat) {
		mat4 model_rot_x = rotate(mat4(1.0f), rotations.x, vec3(1.f, 0.f, 0.f));
		mat4 model_rot_y = rotate(mat4(1.0f), rotations.y, vec3(0.f, 1.f, 0.f));
		mat4 model_rot_z = rotate(mat4(1.0f), rotations.z, vec3(0.f, 0.f, 1.f));

		rotModelMat = model_rot_y * model_rot_z * model_rot_x;
		validRotModelMat = true;
	}

	return rotModelMat;
}

void GameEntity::update()
{
	static float sound_time = .05;
	static FMOD::Channel* sound;
	//static glm::vec3 startingPosition = getPosition();
	if (!setup_inner)
	{
		inner_bounding_box = bounding_box;
	}

	if (animate)
	{
		time_elapsed += seconds_passed;

		if (time_elapsed < ANIMATE_TIME)
		{
			float m_change = wall_height / ANIMATE_TIME;
			setPosition(getPosition() + vec3(0.f, m_change * seconds_passed, 0.f));
		}
		else
		{
			animate = false;
			setPosition(animateCachePosition);
		}
	}
}

void GameEntity::startAnimate()
{
	if (!animate)
	{
		animateCachePosition = getPosition();
		animate = true;
		time_elapsed = 0;
		setPosition(getPosition() - vec3(0.f, wall_height, 0.f));
	}
}

std::vector<std::vector<glm::mat4>>* GameEntity::getBoneTransformations() {
	return NULL;
}