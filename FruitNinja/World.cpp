#include "World.h"
#include "DebugCamera.h"
#include "Global.h"
#include "ArcheryCamera.h"
#include "GuardEntity.h"
#include "ProjectileEntity.h"
#include "ObstacleEntity.h"
#include "OctTree.h"
#include "DeferredShader.h"
#include "ButtonEntity.h"
#include "PlatformEntity.h"
#include "DebugShader.h"
#include <glm/gtx/rotate_vector.hpp>
#include "Skybox.h"
#include "SimpleTextureShader.h"
#include "TestSphere.h"
#include <functional>
#include <queue>
#include "LightEntity.h"
#include "WallOfDoomEntity.h"
#include "FallingEntity.h"
#include "DoorEntity.h"
#include "SpikeEntity.h"
#include "GateEntity.h"
#include "FrustrumCulling.h"
#include <iostream>
#include <fstream>
#include "ParticleShader.h"
#include "CinematicCamera.h"
#include "AudioManager.h"
#include "CollectableEntity.h"
#include <glm/gtc/matrix_access.inl>
#include "GuardPuppeteer.h"
#include "ExplosiveEntity.h"


#define FILE_TO_WORLD_SCALE 6.f
#define NUM_PERSISTENT 11
#include "EndScene.h"

using namespace std;
using namespace glm;

bool keys[1024];
bool mouse_buttons_pressed[8];
float actual_seconds_passed = 0;
float seconds_passed = 0;
float x_offset;
float y_offset;
float cached_le_distance = FLT_MAX;
float guard_far;
float guard_fov;
const float min_guard_far = 30.f;
const float max_guard_far = 80.f;
const float min_guard_fov = 45.f;
const float max_guard_fov = 60.f;
float relative = 0.f;
bool screen_changed;
int sw, sh;

bool debug_enabled = false;
float screen_width = SCREEN_WIDTH;
float screen_height = SCREEN_HEIGHT;

mat4 projection = mat4(perspective((float)radians(PLAYER_FOV), screen_width / screen_height, PLAYER_NEAR, PLAYER_FAR));
mat4 guard_projection = mat4(1.f);

static Camera* camera;
static DebugShader* debugShader;
bool time_stopped = false;
float game_speed = 1.0f;
int current_courtyard = 0;
static vector<std::function<void()>> debugShaderQueue;

float bow_strength = .5f;
int arrow_count = 9;
int starting_arrow_count = arrow_count;

float wall_height;

int health = MAX_HEALTH;
glm::vec3 directional_light = glm::vec3(0);

World::World()
{
	debugShader = new DebugShader("debugVert.glsl", "debugFrag.glsl");
	init();
	x_offset = 0;
	y_offset = 0;
	state = HIDDEN;

}

void World::init()
{
	debug_camera = new DebugCamera();
	player_camera = new PlayerCamera();
	archery_camera = new ArcheryCamera();
	cinematic_camera = new CinematicCamera();

	meshes.insert(pair<string, MeshSet*>("tower", new MeshSet(assetPath + "tower.dae")));
	meshes.insert(pair<string, MeshSet*>("wall", new MeshSet(assetPath + "wall.dae")));
	meshes.insert(pair<string, MeshSet*>("roof", new MeshSet(assetPath + "roof.dae")));
	meshes.insert(pair<string, MeshSet*>("interior_wall", new MeshSet(assetPath + "interiorWall.dae")));
    meshes.insert(pair<string, MeshSet*>("interior_wall_1x1", new MeshSet(assetPath + "interiorWall_1x1.dae")));
    meshes.insert(pair<string, MeshSet*>("interior_wall_1x1", new MeshSet(assetPath + "interiorWall_1x1.dae")));
    meshes.insert(pair<string, MeshSet*>("interior_wall_1x2", new MeshSet(assetPath + "interiorWall_1x2.dae")));
    meshes.insert(pair<string, MeshSet*>("interior_wall_1x3", new MeshSet(assetPath + "interiorWall_1x3.dae")));
    meshes.insert(pair<string, MeshSet*>("interior_wall_1x4", new MeshSet(assetPath + "interiorWall_1x4.dae")));
    meshes.insert(pair<string, MeshSet*>("interior_wall_1x5", new MeshSet(assetPath + "interiorWall_1x5.dae")));
    meshes.insert(pair<string, MeshSet*>("interior_wall_1x6", new MeshSet(assetPath + "interiorWall_1x6.dae")));
    meshes.insert(pair<string, MeshSet*>("interior_wall_1x7", new MeshSet(assetPath + "interiorWall_1x7.dae")));
	meshes.insert(pair<string, MeshSet*>("interior_wall_3x3", new MeshSet(assetPath + "interiorWall_3x3.dae")));
    meshes.insert(pair<string, MeshSet*>("exploding_barrel", new MeshSet(assetPath + "explodingBarrel.dae")));
    meshes.insert(pair<string, MeshSet*>("heart", new MeshSet(assetPath + "heart.dae")));
    meshes.insert(pair<string, MeshSet*>("doom_wall", new MeshSet(assetPath + "spikes_rotated.dae")));
    meshes.insert(pair<string, MeshSet*>("fire_arrow_pickup", new MeshSet(assetPath + "tempFireArrowPickup.dae")));
    meshes.insert(pair<string, MeshSet*>("single_arrow_pickup", new MeshSet(assetPath + "arrowPickup_single.dae")));
    meshes.insert(pair<string, MeshSet*>("triple_arrow_pickup", new MeshSet(assetPath + "arrowPickup_triple.dae")));
    meshes.insert(pair<string, MeshSet*>("spike", new MeshSet(assetPath + "spikes.dae")));
    meshes.insert(pair<string, MeshSet*>("spikes_floor", new MeshSet(assetPath + "spikes_floor.dae")));
    meshes.insert(pair<string, MeshSet*>("button", new MeshSet(assetPath + "button.dae")));
    meshes.insert(pair<string, MeshSet*>("button_base", new MeshSet(assetPath + "button_base.dae")));
    meshes.insert(pair<string, MeshSet*>("target", new MeshSet(assetPath + "target.dae")));
    meshes.insert(pair<string, MeshSet*>("target_rotated", new MeshSet(assetPath + "target_rotated.dae")));
    meshes.insert(pair<string, MeshSet*>("gate_spike", new MeshSet(assetPath + "gate_spikes.dae")));
    meshes.insert(pair<string, MeshSet*>("gate_base", new MeshSet(assetPath + "gate_base.dae")));
	meshes.insert(pair<string, MeshSet*>("door", new MeshSet(assetPath + "door.dae")));
    meshes.insert(pair<string, MeshSet*>("door_closed", new MeshSet(assetPath + "door_closed.dae")));
	meshes.insert(pair<string, MeshSet*>("ground", new MeshSet(assetPath + "ground.dae")));
	meshes.insert(pair<string, MeshSet*>("chewy", new MeshSet(assetPath + "ninja_final3.dae")));
	meshes.insert(pair<string, MeshSet*>("chewy_bb", new MeshSet(assetPath + "ninja_boundingbox.dae")));
	meshes.insert(pair<string, MeshSet*>("guard", new MeshSet(assetPath + "samurai2.dae")));
	meshes.insert(pair<string, MeshSet*>("blue_guard", new MeshSet(assetPath + "blue_samurai2.dae")));
	meshes.insert(pair<string, MeshSet*>("guard_bb", new MeshSet(assetPath + "samurai_bbox.obj")));
	meshes.insert(pair<string, MeshSet*>("guard_outer_bb", new MeshSet(assetPath + "outer_samurai_bbox.obj")));
	meshes.insert(pair<string, MeshSet*>("arrow", new MeshSet(assetPath + "arrow.dae")));
	meshes.insert(pair<string, MeshSet*>("arrow_bb", new MeshSet(assetPath + "arrow_boundingbox.dae")));
	meshes.insert(pair<string, MeshSet*>("arrow_pickup", new MeshSet(assetPath + "arrowPickup_boundingbox.dae")));
	meshes.insert(pair<string, MeshSet*>("unit_sphere", new MeshSet(assetPath + "UnitSphere.obj")));
	meshes.insert(pair<string, MeshSet*>("lantern", new MeshSet(assetPath + "lantern.dae")));
	meshes.insert(pair<string, MeshSet*>("lanternPole", new MeshSet(assetPath + "lanternPole.dae")));
	meshes.insert(pair<string, MeshSet*>("lantern_hook", new MeshSet(assetPath + "lantern_hook.dae")));
	meshes.insert(pair<string, MeshSet*>("lanternPole_boundingbox", new MeshSet(assetPath + "lanternPole_boundingbox.dae")));
	meshes.insert(pair<string, MeshSet*>("closedBarrel", new MeshSet(assetPath + "closedBarrel.dae")));
	meshes.insert(pair<string, MeshSet*>("box", new MeshSet(assetPath + "Box.dae")));
	meshes.insert(pair<string, MeshSet*>("skybox", new MeshSet(assetPath + "skybox.dae", GL_LINEAR, GL_CLAMP_TO_EDGE)));
	meshes.insert(pair<string, MeshSet*>("skybox_black", new MeshSet(assetPath + "skybox_black.dae", GL_LINEAR, GL_CLAMP_TO_EDGE)));
	meshes.insert(pair<string, MeshSet*>("flowerPlanter", new MeshSet(assetPath + "flowerPlanter.dae")));
	meshes.insert(pair<string, MeshSet*>("bushes", new MeshSet(assetPath + "bushes.dae")));
	meshes.insert(pair<string, MeshSet*>("bushes_boundingbox", new MeshSet(assetPath + "bushes_boundingbox.dae")));
	meshes.insert(pair<string, MeshSet*>("statue", new MeshSet(assetPath + "statue.dae")));
	meshes.insert(pair<string, MeshSet*>("platform", new MeshSet(assetPath + "platform.dae")));
	meshes.insert(pair<string, MeshSet*>("fence", new MeshSet(assetPath + "fence.dae")));
	meshes.insert(pair<string, MeshSet*>("pineapple", new MeshSet(assetPath + "pineapple.dae")));
	meshes.insert(pair<string, MeshSet*>("ending_ground", new MeshSet(assetPath + "ending_ground.dae")));
	meshes.insert(pair<string, MeshSet*>("fire_arrow_pickup", new MeshSet(assetPath + "tempFireArrowPickup.dae")));

	archery_camera = new ArcheryCamera(meshes.at("unit_sphere")->getMeshes().at(0));

	chewy = new ChewyEntity(vec3(0.f), meshes.at("chewy"), player_camera, archery_camera);
	chewy->setup_entity_box(meshes.at("chewy_bb"));
	chewy->list = SET_HIDE(chewy->list);

	_skybox= new Skybox(&camera, meshes.at("skybox"));
	_skybox->setScale(750.f);
	_skybox->list = UNSET_OCTTREE((_skybox->list));

	GameEntity* tower = new ObstacleEntity(vec3(-9.0544f * 30.f + 120.f, 0.0, 120.f), meshes.at("tower"));
	tower->setScale(30.0f);
	tower->list = UNSET_OCTTREE((tower->list));
	tower->collision_response = false;

	float wall_scale = 30.f;
	GameEntity* wall_left = new ObstacleEntity(vec3(120.f, 0.f, 240.f), meshes.at("wall"));
	wall_left->setScale(wall_scale);
	GameEntity* wall_right = new ObstacleEntity(vec3(120.f, 0.f, 0.f), meshes.at("wall"));
	wall_right->setScale(wall_scale);
	GameEntity* wall_front = new ObstacleEntity(vec3(0.f, 0.f, 120.f), meshes.at("wall"));
	wall_front->setScale(wall_scale);
	wall_front->setRotations(vec3(0.f, M_PI_2, 0.f));
	wall_front->swap_bounding_box_width_depth();
	GameEntity* wall_back = new ObstacleEntity(vec3(240.f, 0.f, 120.f), meshes.at("wall"));
	wall_back->setScale(wall_scale);
	wall_back->setRotations(vec3(0.f, M_PI_2, 0.f));
	wall_back->swap_bounding_box_width_depth();
	GameEntity* roof_left = new ObstacleEntity(vec3(120.f, 60.f, 240.f), meshes.at("roof"));
	roof_left->setScale(wall_scale);
	GameEntity* roof_right = new ObstacleEntity(vec3(120.f, 60.f, 0.f), meshes.at("roof"));
	roof_right->setScale(wall_scale);
	GameEntity* roof_front = new ObstacleEntity(vec3(0.f, 60.f, 120.f), meshes.at("roof"));
	roof_front->setScale(wall_scale);
	roof_front->setRotations(vec3(0.f, M_PI_2, 0.f));
	roof_front->swap_bounding_box_width_depth();
	GameEntity* roof_back = new ObstacleEntity(vec3(240.f, 60.f, 120.f), meshes.at("roof"));
	roof_back->setScale(wall_scale);
	roof_back->setRotations(vec3(0.f, M_PI_2, 0.f));
	roof_back->swap_bounding_box_width_depth();
	GameEntity* ground = new ObstacleEntity(vec3(120.f, -12.0f, 120.f), meshes.at("ground"));
	ground->setScale(30.f);

	//camera = cinematic_camera;
	//cinematic_camera->in_use = true;

	// Add these into a persistent entities list?
	// or you can just remove everything after their index since
	// they're pushed first (for swapping levels)
	entities.push_back(chewy);
    entities.push_back(ground);
	entities.push_back(tower);
	entities.push_back(wall_left);
	entities.push_back(wall_right);
	entities.push_back(wall_front);
	entities.push_back(wall_back);
	entities.push_back(roof_left);
	entities.push_back(roof_right);
	entities.push_back(roof_front);
	entities.push_back(roof_back);
	/*entities.push_back(courtyard1);
	entities.push_back(courtyard2);
	entities.push_back(courtyard3);
	entities.push_back(courtyard4);
	entities.push_back(courtyard5);*/

	setup_next_courtyard();

	hud = new HUD(chewy);

	defShader =  new DeferredShader("DeferredVertShader.glsl", "DeferredFragShader.glsl", _skybox);

	shaders.insert(pair<string, Shader*>("debugShader", debugShader));

	shaders.insert(pair<string, Shader*>("simpleShader", new SimpleTextureShader("simpleVert.glsl", "simpleFrag.glsl")));

	//shared_ptr<Shader> textDebugShader(new TextureDebugShader());
	//shaders.insert(pair<string, shared_ptr<Shader>>("textureDebugShader", textDebugShader));

	//entities.push_back(new ObstacleEntity(glm::vec3(35, 0, 35), meshes.at("gate_spikes")));
	//entities.back()->setScale(3.f);
	//entities.push_back(new ObstacleEntity(glm::vec3(35, 0, 35), meshes.at("gate_base")));
	//entities.back()->setScale(3.f);

	AudioManager::instance()->playAmbient(assetPath + "ynmg.mp3", 0.1f);
}

void World::set_puppeteer(int courtyard) {
	if (_puppeteer)
		delete _puppeteer;
	_puppeteer = nullptr;
	//use puppeteer if more than 2 or 3 guards in a courtyard
	if (courtyard == 1 || courtyard == 2) {
		_puppeteer = new GuardPuppeteer(meshes["guard"]);
	}
}

void World::setup_next_courtyard(bool setup_cin_cam)
{
	// remove all non-persistent entities
	for (int i = NUM_PERSISTENT; i < entities.size(); i++) {
		should_del.push_back(entities[i]);
	}
	entities.erase(entities.begin() + NUM_PERSISTENT, entities.end());

	//theoretically, you won the previous level
	if (setup_cin_cam)
	{
		starting_arrow_count = arrow_count;
		player_camera->theta = -90.f;
		player_camera->phi = 0.f;
	}

    current_courtyard++;

    // these are supposedly the walls... lets hope the indices dont change!
    entities.at(3)->setScale(vec3(30.f));
    entities.at(4)->setScale(vec3(30.f));
    entities.at(5)->setScale(vec3(30.f));
    entities.at(6)->setScale(vec3(30.f));
	wall_height = entities.at(6)->bounding_box.half_height;
	// these are supposedly the roofs
	entities.at(7)->setPosition(vec3(entities.at(7)->getPosition().x, 66.f, entities.at(7)->getPosition().z));
	entities.at(8)->setPosition(vec3(entities.at(8)->getPosition().x, 66.f, entities.at(8)->getPosition().z));
	entities.at(9)->setPosition(vec3(entities.at(9)->getPosition().x, 66.f, entities.at(9)->getPosition().z));
	entities.at(10)->setPosition(vec3(entities.at(10)->getPosition().x, 66.f, entities.at(10)->getPosition().z));	

	push_courtyards(current_courtyard);

	set_puppeteer(current_courtyard);
    if (current_courtyard == 6)
    {
        endScene = new EndScene();
        return;
    }

	switch (current_courtyard)
	{
	case 1:
		setup_level(level_path + "first_courtyard.txt");
		setup_guard(level_path + "first_courtyard_guard.txt");
		//setup_guard(level_path + "first_courtyard_second_guard.txt");
		//setup_guard(level_path + "first_courtyard_third_guard.txt");
		//setup_guard(level_path + "first_courtyard_fourth_guard.txt");
		player_camera->movement(chewy);
		//if (setup_cin_cam)
			//loading_screen = new LoadingScreen("LoadScreen0.png", "Light is the Enemy!");
		setup_cinematic_camera(level_path + "first_courtyard_cinematic.txt", setup_cin_cam);
		break;
	case 2:
		setup_level(level_path + "second_courtyard.txt");
		setup_guard(level_path + "second_courtyard_first_guard.txt");
		setup_guard(level_path + "second_courtyard_second_guard.txt");
		setup_guard(level_path + "second_courtyard_third_guard.txt");
		setup_guard(level_path + "second_courtyard_fourth_guard.txt");
		player_camera->movement(chewy);
		if (setup_cin_cam)
			loading_screen = new LoadingScreen("LoadScreen1.png", "Color is Key!");
		setup_cinematic_camera(level_path + "second_courtyard_cinematic.txt", setup_cin_cam);
		break;
	case 3:
		setup_level(level_path + "third_courtyard.txt");
        load_button(level_path + "third_courtyard_button_one.txt");
		if (setup_cin_cam)
			loading_screen = new LoadingScreen("LoadScreen2.png", "Surprises Around Every Corner");
		player_camera->movement(chewy);
		break;
	case 4:
        setup_level(level_path + "fourth_courtyard.txt");
        setup_moving_platform(level_path + "fourth_courtyard_platform_one.txt");
        setup_moving_platform(level_path + "fourth_courtyard_platform_two.txt");
        setup_moving_platform(level_path + "fourth_courtyard_platform_three.txt");
        setup_moving_platform(level_path + "fourth_courtyard_platform_five.txt");
        setup_guard(level_path + "fourth_courtyard_first_guard.txt");
        setup_guard(level_path + "fourth_courtyard_second_guard.txt");
        load_button(level_path + "fourth_courtyard_button_one.txt");
        load_button(level_path + "fourth_courtyard_button_two.txt");
		player_camera->movement(chewy);
		setup_cinematic_camera(level_path + "fourth_courtyard_cinematic.txt", true);

        // these are supposedly the walls... lets hope the indices dont change!
        entities.at(3)->setScale(vec3(30.f, 40.f, 30.f));
        entities.at(4)->setScale(vec3(30.f, 40.f, 30.f));
        entities.at(5)->setScale(vec3(30.f, 40.f, 30.f));
        entities.at(6)->setScale(vec3(30.f, 40.f, 30.f));
		wall_height = entities.at(6)->bounding_box.half_height;
		// these are supposedly the roofs
		entities.at(7)->setPosition(vec3(entities.at(7)->getPosition().x, 86.f, entities.at(7)->getPosition().z));
		entities.at(8)->setPosition(vec3(entities.at(8)->getPosition().x, 86.f, entities.at(8)->getPosition().z));
		entities.at(9)->setPosition(vec3(entities.at(9)->getPosition().x, 86.f, entities.at(9)->getPosition().z));
		entities.at(10)->setPosition(vec3(entities.at(10)->getPosition().x, 86.f, entities.at(10)->getPosition().z));
		if (setup_cin_cam)
			loading_screen = new LoadingScreen("LoadScreen3.png", "Tighter Security");
		break;
	case 5:
        setup_level(level_path + "fifth_courtyard.txt");
        setup_moving_platform(level_path + "fifth_courtyard_platform_two.txt");
        setup_moving_platform(level_path + "fifth_courtyard_platform_three.txt");
        setup_moving_platform(level_path + "fifth_courtyard_platform_four.txt");
        load_button(level_path + "fifth_courtyard_button_one.txt");
        load_button(level_path + "fifth_courtyard_gate_drop.txt");
		if (setup_cin_cam)
			loading_screen = new LoadingScreen("LoadScreen4.png", "Don't Burn Yourself");
		break;
	}
}

void World::push_courtyards(int current_courtyard)
{
	float wall_scale = 30.f;

	for (int i = 1; i <= 5; i++)
	{
		if (i == current_courtyard)
			continue;
		
		glm::vec3 offset_xz = glm::vec3(9.7808f * wall_scale - 120, 0.f, -120.f);
		glm::vec3 offset_y = glm::vec3(0.f, (i - current_courtyard) * wall_scale, 0.f);
		glm::vec3 offset_wally = glm::vec3(0.f, -current_courtyard * wall_scale, 0.f);
		float angle = (current_courtyard - i) * glm::radians(72.f);

		GameEntity* wall_left = new ObstacleEntity(rotateY(vec3(120.f, 0.f, 240.f) + offset_xz + offset_wally, angle), meshes.at("wall"));
		wall_left->setScale(vec3(wall_scale, wall_scale * (i / 2.f + 1.f), wall_scale));
		wall_left->setRotations(vec3(0.f, angle, 0.f));
		wall_left->setPosition(wall_left->getPosition() - offset_xz);
		wall_left->list = UNSET_OCTTREE(wall_left->list);
		GameEntity* wall_right = new ObstacleEntity(rotateY(vec3(120.f, 0.f, 0.f) + offset_xz + offset_wally, angle), meshes.at("wall"));
		wall_right->setScale(vec3(wall_scale, wall_scale * (i / 2.f + 1.f), wall_scale));
		wall_right->setRotations(vec3(0.f, angle, 0.f));
		wall_right->setPosition(wall_right->getPosition() - offset_xz);
		wall_right->list = UNSET_OCTTREE(wall_right->list);
		GameEntity* wall_front = new ObstacleEntity(rotateY(vec3(0.f, 0.f, 120.f) + offset_xz + offset_wally, angle), meshes.at("wall"));
		wall_front->setScale(vec3(wall_scale, wall_scale * (i / 2.f + 1.f), wall_scale));
		wall_front->setRotations(vec3(0.f, M_PI_2 + angle, 0.f));
		wall_front->setPosition(wall_front->getPosition() - offset_xz);
		wall_front->list = UNSET_OCTTREE(wall_front->list);
		GameEntity* wall_back = new ObstacleEntity(rotateY(vec3(240.f, 0.f, 120.f) + offset_xz + offset_wally, angle), meshes.at("wall"));
		wall_back->setScale(vec3(wall_scale, wall_scale * (i / 2.f + 1.f), wall_scale));
		wall_back->setRotations(vec3(0.f, M_PI_2 + angle, 0.f));
		wall_back->setPosition(wall_back->getPosition() - offset_xz);
		wall_back->list = UNSET_OCTTREE(wall_back->list);
		GameEntity* roof_left = new ObstacleEntity(rotateY(vec3(120.f, 60.f, 240.f) + offset_xz + offset_y, angle), meshes.at("roof"));
		roof_left->setScale(wall_scale);
		roof_left->setRotations(vec3(0.f, angle, 0.f));
		roof_left->setPosition(roof_left->getPosition() - offset_xz);
		roof_left->list = UNSET_OCTTREE(roof_left->list);
		GameEntity* roof_right = new ObstacleEntity(rotateY(vec3(120.f, 60.f, 0.f) + offset_xz + offset_y, angle), meshes.at("roof"));
		roof_right->setScale(wall_scale);
		roof_right->setRotations(vec3(0.f, angle, 0.f));
		roof_right->setPosition(roof_right->getPosition() - offset_xz);
		roof_right->list = UNSET_OCTTREE(roof_right->list);
		GameEntity* roof_front = new ObstacleEntity(rotateY(vec3(0.f, 60.f, 120.f) + offset_xz + offset_y, angle), meshes.at("roof"));
		roof_front->setScale(wall_scale);
		roof_front->setRotations(vec3(0.f, M_PI_2 + angle, 0.f));
		roof_front->setPosition(roof_front->getPosition() - offset_xz);
		roof_front->list = UNSET_OCTTREE(roof_front->list);
		GameEntity* roof_back = new ObstacleEntity(rotateY(vec3(240.f, 60.f, 120.f) + offset_xz + offset_y, angle), meshes.at("roof"));
		roof_back->setScale(wall_scale);
		roof_back->setRotations(vec3(0.f, M_PI_2 + angle, 0.f));
		roof_back->setPosition(roof_back->getPosition() - offset_xz);
		roof_back->list = UNSET_OCTTREE(roof_back->list);
		GameEntity* ground = new ObstacleEntity(rotateY(vec3(120.f, -12.0f, 120.f) + offset_xz + offset_y, angle), meshes.at("ground"));
		ground->setScale(wall_scale);
		ground->setRotations(vec3(0.f, angle, 0.f));
		ground->setPosition(ground->getPosition() - offset_xz);
		ground->list = UNSET_OCTTREE(ground->list);

		entities.push_back(wall_left);
		entities.push_back(wall_right);
		entities.push_back(wall_front);
		entities.push_back(wall_back);
		entities.push_back(roof_left);
		entities.push_back(roof_right);
		entities.push_back(roof_front);
		entities.push_back(roof_back);
		entities.push_back(ground);
	}

	entities.at(2)->setRotations(vec3(0.f, (current_courtyard - 1) * glm::radians(72.f), 0.f));
	entities.at(2)->setPosition(vec3(entities.at(2)->getPosition().x, entities.at(2)->bounding_box.half_height + (1 - current_courtyard) * wall_scale, entities.at(2)->getPosition().z));
}

void World::delayed_lose_condition()
{
	state = SPOTTED;

	vec3 p_pos = player_camera->cameraPosition;
	vec3 look = player_camera->lookAtPoint;
	vec3 g_dir = chewy->getPosition() - player_camera->lookAtPoint;
	cinematic_camera->init({ p_pos, p_pos + vec3(0.0, 5.0, 0.0), look + vec3(0.0, 5.0, 0.0) + g_dir * .25f,
		look + vec3(0.0, 5.0, 0.0) + g_dir * .5f},
		{ look, look + g_dir * .5f, look + g_dir * .75f, chewy->getPosition()}, 10.f);
	camera->in_use = false;
	camera = cinematic_camera;
	camera->in_use = true;
	chewy->isDead = true;
}

void World::lose_condition()
{
	current_courtyard--;
    health--;
    if (health <= 0)
    {
        health = 4;
        current_courtyard = 0;
    }
	setup_next_courtyard(false);
	arrow_count = starting_arrow_count;
	//set player camera sorta
	camera = player_camera;
	debug_camera->in_use = false;
	player_camera->in_use = true;
	archery_camera->in_use = false;
}

void World::setup_cinematic_camera(string file_path, bool setup_cin_cam)
{
	run_cinematic_camera = setup_cin_cam;
	ifstream level_file;
	level_file.open(file_path);

	string current_line;
	int current_row = 0;
	int height_level = -1;
	vec3 camera_pos_array[10];
	vec3 look_at_array[10];
	vector<vec3> camera_positions;
	vector<vec3> look_at_positions;

	while (!level_file.eof()) // runs through every line
	{
		getline(level_file, current_line);
		if (current_line == "________________________________________")
		{
			current_row = 0;
			height_level++;
		}
		for (int i = 0; i < current_line.length(); i++)
		{
			glm::vec3 world_position = FILE_TO_WORLD_SCALE * vec3(i, height_level, current_row) + vec3(FILE_TO_WORLD_SCALE / 2.f, 0.f, FILE_TO_WORLD_SCALE / 2.f);
			if (isalpha(current_line.at(i)))
				camera_pos_array[current_line.at(i) - 'a'] = world_position;
			else if (isdigit(current_line.at(i)))
				look_at_array[current_line.at(i) - '0'] = world_position;
		}
		current_row++;
	}

	float temp = chewy->bounding_box.center.y;

	chewy->bounding_box.center.y = chewy->bounding_box.half_height + starting_platform_height;
	player_camera->movement(chewy);
	camera_positions.push_back(player_camera->cameraPosition);
	look_at_positions.push_back(player_camera->lookAtPoint);


	for (int i = 0; i < 10; i++)
	{
		if (camera_pos_array[i] != vec3(0))
			camera_positions.push_back(glm::vec3(camera_pos_array[i]));
		if (look_at_array[i] != vec3(0))
			look_at_positions.push_back(glm::vec3(look_at_array[i]));
	}
	camera_positions.push_back(player_camera->cameraPosition);
	look_at_positions.push_back(player_camera->lookAtPoint);

	if (!run_cinematic_camera || chewy->isDead)
	{
		camera->in_use = false;
		camera = player_camera;
		camera->in_use = true;
	}
	else {
		if (camera != nullptr)
		{
			camera->in_use = false;
		}
		cinematic_camera->init(camera_positions, look_at_positions, 40.f);
		camera = cinematic_camera;
		cinematic_camera->in_use = true;
	}

	chewy->bounding_box.center.y = temp;

	level_file.close();
}

void World::setup_level(string file_path, bool animate_elements)
{
	ifstream level_file;
	level_file.open(file_path);

	string current_line;
	int current_row = 0;
	int height_level = -1;
	int animate_index = entities.size();

	while (!level_file.eof()) // runs through every line
	{
		getline(level_file, current_line);
		if (current_line == "________________________________________")
		{
			current_row = 0;
			height_level++;
		}
		for (int i = 0; i < current_line.length(); i++)
		{
            if (height_level >= 0)
            {
                glm::vec3 world_position = FILE_TO_WORLD_SCALE * vec3(i, height_level, current_row) + vec3(FILE_TO_WORLD_SCALE / 2.f, 0.f, FILE_TO_WORLD_SCALE / 2.f);
                setup_token(current_line.at(i), world_position);
            }
		}
		current_row++;
	}
	num_doors = 0;
	level_file.close();
	if (animate_elements && animate_index < entities.size())
	{
		for (int i = animate_index; i < entities.size(); i++)
		{
			entities.at(i)->startAnimate();
		}
	}
}

void World::setup_token(char obj_to_place, glm::vec3 placement_position)
{
	vec3 rots = vec3(0.f);
	bool flag = false; // used for door orientation and accessability
	switch (obj_to_place)
	{
    case 'a':
		entities.push_back(new CollectableEntity(placement_position + vec3(0.0f, FILE_TO_WORLD_SCALE / 2.0f, 0.0f), meshes["single_arrow_pickup"], ARROW_TYPE));
        entities.back()->setup_entity_box(meshes["arrow_pickup"]);
        break;
    case 'A':

		entities.push_back(new CollectableEntity(placement_position + vec3(0.0f, FILE_TO_WORLD_SCALE / 2.0f, 0.0f), meshes["triple_arrow_pickup"], ARROW_TYPE, true, 3));
        entities.back()->setup_entity_box(meshes["arrow_pickup"]);
        break;
    // dont' use 'B'
	case 'C': // set chewy's position
		chewy->setPosition(placement_position + vec3(0.f, chewy->bounding_box.half_height + 0.05f, 0.f));
		break;
	case 'd': // door
        if (placement_position.z < 120.f)
        {
            placement_position.z -= 2.65f;
            entities.push_back(new DoorEntity(placement_position, meshes.at("door_closed"), flag));
        }
		else
		{
			placement_position.z += 2.65f;
			rots = vec3(0.f, M_PI, 0.f);
			flag = true;
            entities.push_back(new DoorEntity(placement_position, meshes.at("door"), flag));
		}
		
		if (num_doors == 0)
		{
			starting_platform_height = placement_position.y;
			num_doors++;
		}
        entities.back()->setScale(3.f);
        entities.back()->setRotations(rots);
        break;
    case 'D': // "down" or south facing lantern on wall
        entities.push_back(new LightEntity(placement_position,
            meshes.at("lantern_hook"), 300.f, meshes.at("unit_sphere"), vec3(1.0, 0.5, 0.0)));
        rots = entities.back()->getRotations();
        rots.y = M_PI_2;
        entities.back()->setRotations(rots);
        entities.back()->swap_bounding_box_width_depth();
        entities.back()->setPosition(entities.back()->getPosition() - vec3(0.f, 0.f, 3.f - entities.back()->inner_bounding_box.half_depth));
        entities.back()->list = UNSET_WALL(entities.back()->list);
        dynamic_cast<LightEntity*>(entities.back())->light->pos = entities.back()->getPosition();
        break;
    case 'e': // static guard facing east
		entities.push_back(new GuardEntity(placement_position, meshes.at("guard"), _puppeteer, vec3(1.0f, 0.f, 0.f)));
		entities.back()->setup_entity_box(meshes["guard_outer_bb"]);
		entities.back()->setup_inner_entity_box(meshes["guard_bb"]);
        break;
    case 'E': // static guard facing east Armored
		entities.push_back(new GuardEntity(placement_position, meshes.at("blue_guard"), _puppeteer, vec3(1.0f, 0.f, 0.f), true));
		entities.back()->setup_entity_box(meshes["guard_outer_bb"]);
		entities.back()->setup_inner_entity_box(meshes["guard_bb"]);
        break;
    case 'f': // falling box
        entities.push_back(new FallingEntity(placement_position, meshes["box"]));
        entities.back()->setScale(3.f);
        break;
    case 'F': // statue and flower bed
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("flowerPlanter")));
        entities.back()->setScale(6.f);
        entities.back()->list = SET_HIDE((entities.back()->list));
		entities.push_back(new ObstacleEntity(placement_position + vec3(0.f, .94483f * 6.f, 0.f), meshes.at("bushes")));
		entities.back()->setup_entity_box(meshes.at("bushes_boundingbox"));
		entities.back()->setScale(6.f);
		entities.back()->list = SET_HIDE((entities.back()->list));
		entities.push_back(new ObstacleEntity(placement_position, meshes.at("statue")));
		entities.back()->setScale(6.f);
		entities.back()->list = SET_HIDE((entities.back()->list));
		entities.back()->setRotations(vec3(0.f, M_PI, 0.f));
		break;
    case '<':
        entities.push_back(new GateEntity(placement_position, meshes.at("gate_spike")));
        entities.back()->setScale(3.f);
        entities.back()->swap_bounding_box_width_depth();
        rots.y = M_PI_2;
        entities.back()->setRotations(rots);
        entities.back()->setPosition(entities.back()->getPosition() - vec3(3.f - entities.back()->inner_bounding_box.half_width, 0.f, 0.f));
        entities.back()->list = UNSET_WALL(entities.back()->list);
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("gate_base")));
        entities.back()->setScale(3.f);
        entities.back()->swap_bounding_box_width_depth();
        rots.y = M_PI_2;
        entities.back()->setRotations(rots);
        entities.back()->setPosition(entities.at(entities.size() - 2)->getPosition() - vec3(0.f, entities.at(entities.size() - 2)->bounding_box.half_height, 0.f));
        entities.back()->list = UNSET_OCTTREE((entities.back()->list));
        entities.back()->list = UNSET_WALL(entities.back()->list);
        break;
    case '>':
        entities.push_back(new GateEntity(placement_position, meshes.at("gate_spike")));
        entities.back()->setScale(3.f);
        entities.back()->swap_bounding_box_width_depth();
        rots.y = M_PI_2;
        entities.back()->setRotations(rots);
        entities.back()->setPosition(entities.back()->getPosition() + vec3(3.f - entities.back()->inner_bounding_box.half_width, 0.f, 0.f));
        entities.back()->list = UNSET_WALL(entities.back()->list);
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("gate_base")));
        entities.back()->setScale(3.f);
        entities.back()->swap_bounding_box_width_depth();
        rots.y = M_PI_2;
        entities.back()->setRotations(rots);
        entities.back()->setPosition(entities.at(entities.size() - 2)->getPosition() - vec3(0.f, entities.at(entities.size() - 2)->bounding_box.half_height, 0.f));
        entities.back()->list = UNSET_OCTTREE((entities.back()->list));
        entities.back()->list = UNSET_WALL(entities.back()->list);
        break;
    case 'G':
        entities.push_back(new GateEntity(placement_position, meshes.at("gate_spike")));
        entities.back()->setScale(3.f);
        entities.back()->setPosition(entities.back()->getPosition() + vec3(0.f, 0.f, 3.f - entities.back()->inner_bounding_box.half_depth));
        entities.back()->list = UNSET_WALL(entities.back()->list);
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("gate_base")));
        entities.back()->setScale(3.f);
        entities.back()->setPosition(entities.back()->getPosition() + vec3(0.f, 0.f, 3.f - entities.back()->inner_bounding_box.half_depth));
        entities.back()->list = UNSET_OCTTREE((entities.back()->list));
        break;
    case 'h': // heart pickup
        entities.push_back(new CollectableEntity(placement_position + vec3(0.0f, FILE_TO_WORLD_SCALE / 2.0f, 0.0f), meshes["heart"], HEART_TYPE));
        entities.back()->setup_entity_box(meshes["arrow_pickup"]);
        break;
    case 'H': //3x3 inner wall
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("interior_wall_3x3"))); 
        if (((((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && (((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)) ||
            (!(((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && !(((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)))
            entities.back()->setRotations(vec3(0.f, M_PI_2, 0.f));
        entities.back()->setScale(3.f);
        entities.back()->list = SET_HIDE((entities.back()->list));
        break;
    case 'k': // fence
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("fence")));
        rots = entities.back()->getRotations();
        rots.y = M_PI_2;
        entities.back()->setRotations(rots);
        entities.back()->setScale(3.f);
        entities.back()->swap_bounding_box_width_depth();
        entities.back()->list = UNSET_WALL(entities.back()->list);
        break;
    case 'K': // fence
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("fence")));
        entities.back()->setScale(3.f);
        entities.back()->list = UNSET_WALL(entities.back()->list);
        break;
    case 'l': // Lantern Pole with Lantern
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("lanternPole")));
        entities.back()->setup_entity_box(meshes.at("lanternPole_boundingbox"));
        entities.back()->list = UNSET_WALL(entities.back()->list);
        entities.push_back(new LightEntity(placement_position + vec3(0.f, 5.9f, 0.8f),
            meshes.at("lantern"), 300.f, meshes.at("unit_sphere"), vec3(1.0, 0.5, 0.0)));
        rots = entities.back()->getRotations();
        rots.y = M_PI_2;
        entities.back()->setRotations(rots);
        entities.back()->list = UNSET_WALL(entities.back()->list);
        break;
    case 'L': // left facing lantern on wall
        entities.push_back(new LightEntity(placement_position,
            meshes.at("lantern_hook"), 300.f, meshes.at("unit_sphere"), vec3(1.0, 0.5, 0.0)));
        entities.back()->setPosition(entities.back()->getPosition() + vec3(3.f - entities.back()->inner_bounding_box.half_width, 0.f, 0.f));
        entities.back()->list = UNSET_WALL(entities.back()->list);
        dynamic_cast<LightEntity*>(entities.back())->light->pos = entities.back()->getPosition();
        break;
	case 'n': // static guard facing north
		entities.push_back(new GuardEntity(placement_position, meshes.at("guard"), _puppeteer, vec3(0.0f, 0.f, -1.f)));
		entities.back()->setup_entity_box(meshes["guard_outer_bb"]);
		entities.back()->setup_inner_entity_box(meshes["guard_bb"]);
		break;
    case 'N': // static guard facing north armored
		entities.push_back(new GuardEntity(placement_position, meshes.at("blue_guard"), _puppeteer, vec3(0.0f, 0.f, -1.f), true));
		entities.back()->setup_entity_box(meshes["guard_outer_bb"]);
		entities.back()->setup_inner_entity_box(meshes["guard_bb"]);
        break;
    case 'O': // barrel
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("closedBarrel")));
        entities.back()->setScale(3.f);
        entities.back()->list = SET_HIDE((entities.back()->list));
        break;
    case 'p': //fire arrow pickup
        entities.push_back(new CollectableEntity(placement_position, meshes.at("fire_arrow_pickup"), FIRE_ARROW_TYPE, true));
        entities.back()->setup_entity_box(meshes["arrow_pickup"]);
        break;
    case 'P':
        entities.push_back(new DoorEntity(placement_position, meshes.at("door"), true));
        entities.back()->setRotations(vec3(0.f, M_PI_2, 0.f));
        entities.back()->setScale(3.f);
        break;
    case 'Q':
        entities.push_back(new ExplosiveEntity(placement_position, meshes.at("exploding_barrel"), 6.f));
        entities.back()->setScale(3.f);
        break;
    case 'R': // right facing lantern on wall
        entities.push_back(new LightEntity(placement_position,
            meshes.at("lantern_hook"), 300.f, meshes.at("unit_sphere"), vec3(1.0, 0.5, 0.0)));
        rots = entities.back()->getRotations();
        rots.y = M_PI;
        entities.back()->setRotations(rots);
        entities.back()->setPosition(entities.back()->getPosition() - vec3(3.f - entities.back()->inner_bounding_box.half_width, 0.f, 0.f));
        entities.back()->list = UNSET_WALL(entities.back()->list);
        dynamic_cast<LightEntity*>(entities.back())->light->pos = entities.back()->getPosition();
        break;
	case 's': // static guard facing south
		entities.push_back(new GuardEntity(placement_position, meshes.at("guard"), _puppeteer, vec3(0.0f, 0.f, 1.f)));
		entities.back()->setup_entity_box(meshes["guard_outer_bb"]);
		entities.back()->setup_inner_entity_box(meshes["guard_bb"]);
		break;
    case 'S': // static guard facing south armored
		entities.push_back(new GuardEntity(placement_position, meshes.at("blue_guard"), _puppeteer, vec3(0.0f, 0.f, 1.f), true));
		entities.back()->setup_entity_box(meshes["guard_outer_bb"]);
		entities.back()->setup_inner_entity_box(meshes["guard_bb"]);
        break;
    case 't': // falling stone... its a trap
        entities.push_back(new FallingEntity(placement_position, meshes["interior_wall_1x1"]));
        entities.back()->setScale(3.f);
        break;
    case 'U': // "up" or north facing lantern on wall
        entities.push_back(new LightEntity(placement_position,
            meshes.at("lantern_hook"), 300.f, meshes.at("unit_sphere"), vec3(1.0, 0.5, 0.0)));
        rots = entities.back()->getRotations();
        rots.y = -M_PI_2;
        entities.back()->setRotations(rots);
        entities.back()->swap_bounding_box_width_depth();
        entities.back()->setPosition(entities.back()->getPosition() + vec3(0.f, 0.f, 3.f - entities.back()->inner_bounding_box.half_depth));
        entities.back()->list = UNSET_WALL(entities.back()->list);
        dynamic_cast<LightEntity*>(entities.back())->light->pos = entities.back()->getPosition();
        break;
    case 'v': // spikes on the floor
        entities.push_back(new SpikeEntity(placement_position, meshes.at("spikes_floor"), this));
        entities.back()->setScale(3.f);
        break;
    case 'V': // spikes
        entities.push_back(new SpikeEntity(placement_position, meshes.at("spike"), this));
        entities.back()->setScale(3.f);
        break;
	case 'w': // static guard facing west
		entities.push_back(new GuardEntity(placement_position, meshes.at("guard"), _puppeteer, vec3(-1.0f, 0.f, 0.f)));
		entities.back()->setup_entity_box(meshes["guard_outer_bb"]);
		entities.back()->setup_inner_entity_box(meshes["guard_bb"]);
		break;
    case 'W': // static guard facing west Armored
        entities.push_back(new GuardEntity(placement_position, meshes.at("blue_guard"), _puppeteer, vec3(-1.0f, 0.f, 0.f), true));
		entities.back()->setup_entity_box(meshes["guard_outer_bb"]);
		entities.back()->setup_inner_entity_box(meshes["guard_bb"]);
        break;
	case 'X': // crate
		entities.push_back(new ObstacleEntity(placement_position, meshes.at("box")));
		entities.back()->setScale(3.f);
		entities.back()->list = SET_HIDE((entities.back()->list));
		break;
    case'{':
        entities.push_back(new WallOfDoomEntity(placement_position, meshes.at("doom_wall"), vec3(-1.f, 0.f, 0.f)));
        entities.back()->setScale(3.f);
        entities.back()->setRotations(vec3(0.f, -M_PI_2, 0.f));
        break;
    case '1':
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("interior_wall_1x1")));
        entities.back()->setScale(3.f);
        entities.back()->list = SET_HIDE((entities.back()->list));
        if (((((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && (((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)) ||
            (!(((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && !(((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)))
            entities.back()->setRotations(vec3(0.f, M_PI_2, 0.f));
        break;
    case '2':
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("interior_wall_1x2")));
        entities.back()->setScale(3.f);
        entities.back()->list = SET_HIDE((entities.back()->list));
        if (((((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && (((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)) ||
            (!(((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && !(((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)))
            entities.back()->setRotations(vec3(0.f, M_PI_2, 0.f));
        break;
    case '3':
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("interior_wall_1x3")));
        entities.back()->setScale(3.f);
        entities.back()->list = SET_HIDE((entities.back()->list));
        if (((((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && (((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)) ||
            (!(((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && !(((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)))
            entities.back()->setRotations(vec3(0.f, M_PI_2, 0.f));
        break;
    case '4':
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("interior_wall_1x4")));
        entities.back()->setScale(3.f);
        entities.back()->list = SET_HIDE((entities.back()->list));
        if (((((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && (((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)) ||
            (!(((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && !(((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)))
            entities.back()->setRotations(vec3(0.f, M_PI_2, 0.f));
        break;
    case '5':
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("interior_wall_1x5")));
        entities.back()->setScale(3.f);
        entities.back()->list = SET_HIDE((entities.back()->list));
        if (((((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && (((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)) ||
            (!(((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && !(((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)))
            entities.back()->setRotations(vec3(0.f, M_PI_2, 0.f));
        break;
    case '6':
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("interior_wall_1x6")));
        entities.back()->setScale(3.f);
        entities.back()->list = SET_HIDE((entities.back()->list));
        if (((((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && (((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)) ||
            (!(((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && !(((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)))
            entities.back()->setRotations(vec3(0.f, M_PI_2, 0.f));
        break;
    case '7':
        entities.push_back(new ObstacleEntity(placement_position, meshes.at("interior_wall_1x7")));
        entities.back()->setScale(3.f);
        entities.back()->list = SET_HIDE((entities.back()->list));
        if (((((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && (((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)) ||
            (!(((int)placement_position.x / (int)FILE_TO_WORLD_SCALE) % 2) && !(((int)placement_position.z / (int)FILE_TO_WORLD_SCALE) % 2)))
            entities.back()->setRotations(vec3(0.f, M_PI_2, 0.f));
        break;
	}
}

void World::load_button(string file_path)
{
    ifstream level_file;
    level_file.open(file_path);

    string mesh_name;
    vector<string> on_press_levels;
    vector<string> on_press_platforms;
    vector<string> other_button_files;
    vector<string> on_press_cinematic_file;

    string current_line;
    int current_row = 0;
    int height_level = -1;
    bool open_gate = false;
    bool wall_of_doom = false;

    while (!level_file.eof()) // runs through every line
    {
        getline(level_file, current_line);
        if (height_level < 0 && current_line.length() > 1) // saving all things of importance
        {
            string load_path = current_line.substr(2, current_line.length() - 1);
            switch (current_line.at(0))
            {
            case 'C':
                on_press_cinematic_file.push_back(level_path + load_path);
                break;
            case 'M': // only happens once, name of the mesh for this button
                mesh_name = load_path;
                break;
            case 'B': // button file
                other_button_files.push_back(level_path + load_path);
                break;
            case 'P': // platform file
                on_press_platforms.push_back(level_path + load_path);
                break;
            case 'L': // level file
                on_press_levels.push_back(level_path + load_path);
                break;
            case 'G':
                open_gate = true;
                break;
            case '{':
                wall_of_doom = true;
                break;
            }
        }
        if (current_line == "________________________________________")
        {
            current_row = 0;
            height_level++;
        }
        if (height_level >= 0)
        {
            for (int i = 0; i < current_line.length(); i++)
            {
                glm::vec3 world_position = FILE_TO_WORLD_SCALE * vec3(i, height_level, current_row) + vec3(FILE_TO_WORLD_SCALE / 2.f, 0.f, FILE_TO_WORLD_SCALE / 2.f);
                if (current_line.at(i) == 'B')
                {
                    entities.push_back(new ButtonEntity(world_position, meshes[mesh_name], on_press_levels, on_press_platforms, other_button_files, on_press_cinematic_file, open_gate, wall_of_doom));
                }
            }
            current_row++;
        }
    }

    level_file.close();
}

void World::setup_guard(string file_path)
{
	ifstream level_file;
	level_file.open(file_path);

	string current_line;
	int current_row = 0;
    int height_level = -1;

	vec3 control_points[10];
	vec3 starting_position;
	bool linear = false;
    bool isArmored = false;

	while (!level_file.eof()) // runs through every line
	{
		getline(level_file, current_line);
        if (current_line == "________________________________________")
        {
            current_row = 0;
            height_level++;
        }
		for (int i = 0; i < current_line.length(); i++)
		{
			glm::vec3 world_position = FILE_TO_WORLD_SCALE * vec3(i, height_level, current_row) + vec3(FILE_TO_WORLD_SCALE / 2.f, 0.f, FILE_TO_WORLD_SCALE / 2.f);
			switch (current_line.at(i))
			{
			case 'G':
				starting_position = world_position;
				break;
			case 'g':
				starting_position = world_position;
				linear = true;
				break;
            case 'B':
                starting_position = world_position;
                linear = true;
                isArmored = true;
                break;
			case '0':
				control_points[0] = world_position;
				break;
			case '1':
				control_points[1] = world_position;
				break;
			case '2':
				control_points[2] = world_position;
				break;
			case '3':
				control_points[3] = world_position;
				break;
			case '4':
				control_points[4] = world_position;
				break;
			case '5':
				control_points[5] = world_position;
				break;
			case '6':
				control_points[6] = world_position;
				break;
			case '7':
				control_points[7] = world_position;
				break;
			case '8':
				control_points[8] = world_position;
				break;
			case '9':
				control_points[9] = world_position;
				break;
			}
		}
		current_row++;
	}

	vector<vec3> spline_points;
	spline_points.push_back(starting_position);
	for (int i = 0; i < 10; i++)
	{
		if (control_points[i] != vec3(0))
			spline_points.push_back(glm::vec3(control_points[i]));
		else
			break;
	}
    GuardEntity* guard_ent;
    if (isArmored)
		guard_ent = new GuardEntity(starting_position, meshes["blue_guard"], _puppeteer, spline_points, 4.f, linear, isArmored);
    else
		guard_ent = new GuardEntity(starting_position, meshes["guard"], _puppeteer, spline_points, 4.f, linear);
	entities.push_back(guard_ent);
	entities.back()->setup_entity_box(meshes["guard_outer_bb"]);
	entities.back()->setup_inner_entity_box(meshes["guard_bb"]);
	level_file.close();
}

void World::setup_moving_platform(string file_path)
{
	ifstream level_file;
	level_file.open(file_path);

	string current_line;
	int current_row = 0;
    int height_level = -1;
    bool tile = false;

	vec3 control_points[10];
	vec3 starting_position;

	while (!level_file.eof()) // runs through every line
	{
		getline(level_file, current_line);
		if (current_line == "________________________________________")
		{
			current_row = 0;
			height_level++;
		}
		for (int i = 0; i < current_line.length(); i++)
		{
			glm::vec3 world_position = FILE_TO_WORLD_SCALE * vec3(i, height_level, current_row) + vec3(FILE_TO_WORLD_SCALE / 2.f, 0.f, FILE_TO_WORLD_SCALE / 2.f);
			switch (current_line.at(i))
			{
			case 'P':
				starting_position = world_position;
				break;
            case 'T':
                starting_position = world_position;
                tile = true;
                break;
			case '0':
				control_points[0] = world_position;
				break;
			case '1':
				control_points[1] = world_position;
				break;
			case '2':
				control_points[2] = world_position;
				break;
			case '3':
				control_points[3] = world_position;
				break;
			case '4':
				control_points[4] = world_position;
				break;
			case '5':
				control_points[5] = world_position;
				break;
			case '6':
				control_points[6] = world_position;
				break;
			case '7':
				control_points[7] = world_position;
				break;
			case '8':
				control_points[8] = world_position;
				break;
			case '9':
				control_points[9] = world_position;
				break;
			}
		}
		current_row++;
	}

	vector<vec3> spline_points;
	spline_points.push_back(starting_position);
	for (int i = 0; i < 10; i++)
	{
		if (control_points[i] != vec3(0))
			spline_points.push_back(glm::vec3(control_points[i]));
		else
			break;
	}
    PlatformEntity* platform = new PlatformEntity(starting_position, meshes["platform"], spline_points, 10.f);
    if (tile)
        platform->setScale(6.f);
    else
        platform->setScale(4.f);
    entities.push_back(platform);
	level_file.close();
}

void World::shootArrows()
{

	static bool held = false;
	bool shot = false;
	for (auto it = entities.begin(); it != entities.end(); ++it) {
		ProjectileEntity* p_test = dynamic_cast<ProjectileEntity*>(*it);
		if (p_test != nullptr) {
			shot = true;
		}
	}
	
	defShader->arcShader.enabled = !shot;

	if ((keys[GLFW_KEY_E] || mouse_buttons_pressed[0]) && archery_camera->in_use && !held && !shot && arrow_count > 0)
	{
		held = true;
	}
	if (held && !(keys[GLFW_KEY_E] || mouse_buttons_pressed[0]) && arrow_count > 0)
	{
		//entities.push_back(new FireArrowEntity(meshes["arrow"], archery_camera));
		if (_fiery_arrows) {
			entities.push_back(new FireArrowEntity(meshes["arrow"], archery_camera));
		}
		else {
			entities.push_back(new ProjectileEntity(meshes["arrow"], archery_camera));
		}
		vec3 center = entities.back()->getPosition();
		entities.back()->setup_entity_box(meshes.at("arrow_pickup"));
		entities.back()->setup_inner_entity_box(meshes.at("arrow_bb"));
		dynamic_cast<ProjectileEntity*>(entities.back())->dist = glm::distance(entities.back()->inner_bounding_box.center, center);
		entities.back()->bounding_box.center = entities.back()->inner_bounding_box.center;
		arrow_count--;
		held = false;
		AudioManager::instance()->play3D(assetPath + "WW_Arrow_Shoot.wav", chewy->getPosition(), 1.0f, false);
	}

	mouse_buttons_pressed[0] = false;
}

void World::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (screen_changed) {
		screen_changed = false;
		screen_width = sw;
		screen_height = sh;
		mat4 projection = mat4(perspective((float)radians(PLAYER_FOV), screen_width / screen_height, PLAYER_NEAR, PLAYER_FAR));

		defShader->resizeWindow();
	}

	if (loading_screen == nullptr)
	{
		glUseProgram(0);
		//static bool usePhong = false;

		DebugCamera* d_test = dynamic_cast<DebugCamera*>(camera);
		vector<GameEntity*> in_view;

        // MERGE STUFF HAPPENED HERE
	////otherwise deferred rendering
	//vector<Light*> lights;
	//for (int i = 0; i < entities.size(); i++) {
	//	//even if lantern culled still need light from it
 //       if (typeid(*entities[i]) == typeid(LightEntity) && SHOULD_DRAW(entities[i]->list))
 //       {
	//		LightEntity* le = dynamic_cast<LightEntity*>(entities[i]);
	//		if (le->light) lights.push_back(le->light);
		PlayerCamera* p_test = dynamic_cast<PlayerCamera*>(camera);
		if (p_test != nullptr)
		{
			p_test->reorient(entities, chewy);
		}
		glUseProgram(0);

		//otherwise deferred rendering
		vector<Light*> lights;
		for (int i = 0; i < entities.size(); i++) {
			//even if lantern culled still need light from it
			if (typeid(*entities[i]) == typeid(LightEntity) && SHOULD_DRAW(entities[i]->list)) {
				LightEntity* le = dynamic_cast<LightEntity*>(entities[i]);
				if (le->light) lights.push_back(le->light);
			}
			//if there's an arrow have archery camera follow it and make game slow-mo
			/*if (typeid(*entities[i]) == typeid(ProjectileEntity)) {
				archery_camera->cameraPosition = entities[i]->bounding_box.center - archery_camera->cameraFront * 4.0f;
			}*/

			if (!SHOULD_DRAW(entities[i]->list)) {
				should_del.push_back(entities[i]);
				entities.erase(entities.begin() + i);
				i--;
			}
		}
		for (int i = 0; i < entities.size(); i++)
		{
			if (!SHOULD_DRAW(entities[i]->list)) {
				entities.erase(entities.begin() + i);
				i--;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(defShader->getProgramID());
		glViewport(0, 0, screen_width, screen_height);
		defShader->draw(camera, entities, lights);

		glUseProgram(0);
		if (debug_enabled)
		{
			glUseProgram(debugShader->getProgramID());
			for (int i = 0; i < entities.size(); i++)
			{
				EntityBox box = entities.at(i)->bounding_box;
				vector<pair<vec3, vec3>> points = box.get_line_segments();
				for (int j = 0; j < points.size(); j++)
				{
					draw_line(points.at(j).first, points.at(j).second, vec3(1.f, 0.f, 0.f));
				}

				if (entities.at(i)->setup_inner)
				{
					EntityBox box = entities.at(i)->inner_bounding_box;
					vector<pair<vec3, vec3>> points = box.get_line_segments();
					for (int j = 0; j < points.size(); j++)
					{
						draw_line(points.at(j).first, points.at(j).second, vec3(0.f, 0.f, 1.f));
					}
				}
			}
			glUseProgram(0);
		}

		glUseProgram(debugShader->getProgramID());
		for (int i = debugShaderQueue.size() - 1; i >= 0; i--)
		{
			debugShaderQueue.at(i)();
		}
		if (!time_stopped)
		{
			debugShaderQueue.clear();
		}
		glUseProgram(0);

		if (endScene == nullptr)
			hud->draw();
		else
			endScene->draw();
	}
	else
		loading_screen->draw();
}

void World::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void World::mouse_callback(GLFWwindow* window, double x_position, double y_position)
{
	x_offset = x_position;
	y_offset = -1.f * y_position;

	glfwSetCursorPos(window, 0, 0);
}

void World::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
		mouse_buttons_pressed[button] = true;
}

void World::convert_to_collectible(ProjectileEntity* p)
{
	entities.push_back(new CollectableEntity(p->getPosition(), meshes.at("arrow"), ARROW_TYPE, false, 1));
	entities.back()->bounding_box = p->bounding_box;
	entities.back()->inner_bounding_box = p->inner_bounding_box;
	//entities.back()->inner_bounding_box.center -= p->dist * normalize(p->movement.velocity);
	entities.back()->setup_inner = true;
	dynamic_cast<CollectableEntity*>(entities.back())->custom_rotate(p->rot);
	entities.back()->list = UNSET_OCTTREE(entities.back()->list);
}

void World::set_chewy_light_distance(float dist, float le_hv_length)
{
    if (dist < cached_le_distance)
    {
        cached_le_distance = dist;
        relative = 1.f - (dist / le_hv_length);

        float m_fov = max_guard_fov - min_guard_fov;
        guard_fov = m_fov * relative + min_guard_fov;

        float m_far = max_guard_far - min_guard_far;
        guard_far = m_far * relative + min_guard_far;
    }
}

void World::change_camera()
{
	if (keys[GLFW_KEY_1] && DEBUG_MODE)
	{
		camera = debug_camera;
		debug_camera->in_use = true;
		player_camera->in_use = false;
		archery_camera->in_use = false;

	}
	else if (keys[GLFW_KEY_2] || (mouse_buttons_pressed[1] && !player_camera->in_use))
	{
		if (archery_camera->in_use)
		{
			//player_camera->phi = archery_camera->phi;
			player_camera->theta = archery_camera->theta;
			player_camera->mouse_update();
		}
		camera = player_camera;
		debug_camera->in_use = false;
		player_camera->in_use = true;
		archery_camera->in_use = false;
	}
	else if (keys[GLFW_KEY_3] || (mouse_buttons_pressed[1] && player_camera->in_use))
	{
		if (player_camera->in_use)
		{
			archery_camera->phi = 0.f;
			archery_camera->theta = player_camera->theta;
			archery_camera->mouse_update();
		}
		camera = archery_camera;
		debug_camera->in_use = false;
		player_camera->in_use = false;
		archery_camera->in_use = true;
	}

	mouse_buttons_pressed[1] = false;
}

void World::enable_debugging()
{
	if (keys[GLFW_KEY_Z])
		debug_enabled = true;
	if (keys[GLFW_KEY_X])
		debug_enabled = false;
}

void World::skip_level()
{
	
	if (mouse_buttons_pressed[2]) {
		if (DEBUG_MODE) {
			setup_next_courtyard();
		}
		else {
			if (keys[GLFW_KEY_PAGE_DOWN]) {
				setup_next_courtyard();
			}
		}

		mouse_buttons_pressed[2] = false;
	}

	if (DEBUG_MODE) {
		//jon's middle click doesn't work
		if (keys[GLFW_KEY_7])
		{
			current_courtyard = 3;
			setup_next_courtyard();
		}
		if (keys[GLFW_KEY_6])
		{
			current_courtyard = 3;
			setup_next_courtyard();
		}
		//faster level design
		if (keys[GLFW_KEY_8])
		{
			current_courtyard--;
			setup_next_courtyard();
		}
	}
}

void World::flip_controls()
{
	if (player_camera != nullptr) {
		if (keys[GLFW_KEY_G]) {
			player_camera->flippedControls = false;
		}
		else if (keys[GLFW_KEY_H]) {
			player_camera->flippedControls = true;
		}
	}
}

void World::cancel_cinematic()
{
	if (keys[GLFW_KEY_0])
	{
		run_cinematic_camera = false;
		camera = player_camera;
		player_camera->in_use = true;
	}
}
void World::update_key_callbacks()
{
	if (run_cinematic_camera)
	{
		cancel_cinematic();
		if (cinematic_camera->pathing.done)
		{
			run_cinematic_camera = false;
			camera = player_camera;
			player_camera->in_use = true;
		}
	}
	else if (state != SPOTTED)
	{
		change_camera();
		flip_controls();
		skip_level();
		if (DEBUG_MODE) {
			enable_debugging();
			stop_time();
		}
	}
	camera->movement(chewy);
	x_offset = 0;
	y_offset = 0;
}

void World::stop_time()
{
	if (keys[GLFW_KEY_T])
		time_stopped = true;
	if (keys[GLFW_KEY_Y])
		time_stopped = false;
}


void World::update()
{
	static float start_time = 0.0;
    if (keys[GLFW_KEY_RIGHT_CONTROL])
	{
		endScene = new EndScene();
	}
	if (keys[GLFW_KEY_6])
	{
		current_courtyard = 4;
		setup_next_courtyard(false);
	}
	//jon's middle click doesn't work
	if (keys[GLFW_KEY_7])
	{
		setup_next_courtyard(false);
	}
	//faster level design
	if (keys[GLFW_KEY_8])
	{
		current_courtyard--;
		setup_next_courtyard(false);
	}
	if (endScene == nullptr)
	{
		if (loading_screen == nullptr)
		{
			shootArrows();
			vector<GameEntity*> guards;

			for (int i = 0; i < entities.size(); i++)
				if (state == SPOTTED && cinematic_camera->pathing.done) {
					lose_condition();
					chewy->isCaught = false;
					chewy->isDead = false;
					chewy->animComponent.basicAnimation.changeToLoopingAnimation(STANDING_START, STANDING_START + STANDING_DURATION);
					chewy->animComponent.currentAnimtion = standing;
					state = HIDDEN;
				}
			if (_puppeteer){
				_puppeteer->update();
			}
			for (int i = 0; i < entities.size(); i++)
			{
				entities[i]->update();
				GuardEntity* guard_temp = dynamic_cast<GuardEntity*>(entities[i]);
				if (guard_temp != nullptr && !guard_temp->is_dying())
				{
					entities[i]->update();
					GuardEntity* guard_temp = dynamic_cast<GuardEntity*>(entities[i]);
					if (guard_temp != nullptr)
					{
						guards.push_back(entities[i]);
					}
				}
			}

			actual_seconds_passed = (glfwGetTime() - start_time) * game_speed;

			if (!time_stopped)
			{
				seconds_passed = actual_seconds_passed;
			}
			else
			{
				seconds_passed = 0.f;
			}
			start_time = glfwGetTime();

			guard_fov = min_guard_fov;
			guard_far = min_guard_far;
			cached_le_distance = FLT_MAX;
			Voxel vox(vec3(0, 0.f, 0.f), vec3(250.f, 250.f, 250.f));
			OctTree world_oct_tree(vox, entities);
			world_oct_tree.handle_collisions();
			guard_projection = mat4(perspective((float)radians(guard_fov), screen_width / screen_height, GUARD_NEAR, guard_far));
			for (int i = 0; i < guards.size(); i++)
			{
				GuardEntity* guard_temp = dynamic_cast<GuardEntity*>(guards[i]);
				if (guard_temp != nullptr && guard_temp->check_view(chewy, entities) && state != SPOTTED) {
					zoom_on_guard(guard_temp);
				}
			}
			for (int i = 0; i < should_del.size(); i++) {
				delete should_del[i];
			}
			should_del.clear();
			update_key_callbacks();
			if (_skybox != nullptr)
				_skybox->update();

			AudioManager::instance()->updateListener(camera->cameraPosition, camera->cameraFront, camera->cameraUp);
		}
		else
		{
			actual_seconds_passed = (glfwGetTime() - start_time) * game_speed;
			if (!time_stopped)
			{
				seconds_passed = actual_seconds_passed;
			}
			else
			{
				seconds_passed = 0.f;
			}
			start_time = glfwGetTime();

			if (keys[GLFW_KEY_ENTER])
			{

				delete loading_screen;
				loading_screen = nullptr;

			}
		}
	}
	else
	{
		actual_seconds_passed = (glfwGetTime() - start_time) * game_speed;

		if (!time_stopped)
		{
			seconds_passed = actual_seconds_passed;
		}
		else
		{
			seconds_passed = 0.f;
		}
		start_time = glfwGetTime();
		endScene->update();
	}
}

void World::zoom_on_guard(GameEntity* guard_temp)
{
	AudioManager::instance()->play3D(assetPath + "jons_breakthrough_performance.wav", guard_temp->getPosition(), 10.0f, false);
	state = SPOTTED;

	vec3 p_pos = player_camera->cameraPosition;
	vec3 look = player_camera->lookAtPoint;
	vec3 g_dir = guard_temp->getPosition() - player_camera->lookAtPoint;
	cinematic_camera->init({ p_pos, p_pos + vec3(0.0, 5.0, 0.0), look + vec3(0.0, 5.0, 0.0) + g_dir * .25f,
		look + vec3(0.0, 5.0, 0.0) + g_dir * .5f, look + vec3(0.0, 5.0, 0.0) + g_dir * .5f, look + vec3(0.0, 5.0, 0.0) + g_dir * .5f },
		{ look, look + g_dir * .5f, look + g_dir * .75f, guard_temp->getPosition(), guard_temp->getPosition(), guard_temp->getPosition() }, 10.f);
	camera->in_use = false;
	camera = cinematic_camera;
	camera->in_use = true;
	chewy->isCaught = true;
}

void World::scroll_callback(GLFWwindow* window, double x_pos, double y_pos)
{
	if (dynamic_cast<PlayerCamera*>(camera))
	{
		PlayerCamera* radius_changer = dynamic_cast<PlayerCamera*>(camera);
		if (radius_changer)
			radius_changer->update_radius(y_pos);
	}
	else if (dynamic_cast<ArcheryCamera*>(camera))
	{
		bow_strength -= y_pos * .05;
		bow_strength = bow_strength < 0.01 ? 0.01 : bow_strength;
		bow_strength = bow_strength > 1.0 ? 1.0 : bow_strength;
	}
}

void World::draw_line(vec3 p1, vec3 p2, vec3 color)
{
	glUseProgram(debugShader->getProgramID());
	debugShaderQueue.push_back([=](){debugShader->drawLine(p1, p2, color, camera->getViewMatrix()); });
	glUseProgram(0);
}

void World::draw_point(vec3 p, vec3 color, float radius)
{
	glUseProgram(debugShader->getProgramID());
	debugShaderQueue.push_back([=](){debugShader->drawPoint(p, color, radius, camera->getViewMatrix());  });
	glUseProgram(0);
}

void World::draw_box(EntityBox* box, vec3 color)
{
	vector<pair<vec3, vec3>> points = box->get_line_segments();
	for (int j = 0; j < points.size(); j++)
	{
		draw_line(points.at(j).first, points.at(j).second, vec3(1.f, 0.f, 0.f));
	}
}

World::~World() {
	for (int i = 0; i < entities.size(); i++) {
		delete entities[i];
	}
	delete _skybox;
	delete hud;
	for (auto it = shaders.begin(); it != shaders.end(); ++it) {
		delete it->second;
	}
	delete debug_camera;
	delete player_camera;
	delete archery_camera;
	delete cinematic_camera;
	for (auto it = meshes.begin(); it != meshes.end(); ++it) {
		delete it->second;
	}
	if (_puppeteer)
		delete _puppeteer;
	delete defShader;
}


GameState World::getState() {
	return state;
}

void World::resize_window(GLFWwindow* window, int w, int h) {
	screen_changed = true;
	sw = w;
	sh = h;
}

void World::addExplosion(glm::vec3 pos) {
	defShader->addExplosion(pos);
	AudioManager::instance()->play3D(assetPath + "explosion.wav", pos, 20.0, false);
}

void World::change_camera(Camera* newCamera)
{
	camera = newCamera;
}

void World::enableFireArrows() {
	_fiery_arrows = true;
}