#include "World.h"
#include <stddef.h>
#include <GFX/RenderCore.h>
#include <iostream>

namespace Minecraft::Internal {
	World::World()
	{
	}
	World::~World()
	{
	}
	void World::init()
	{
		is_loading = true;
		player = new Player();
		sky = new Rendering::Sky();
		timer.reset();
		timeData.age = 0;
		timeData.timeOfDay = 0;
		chunkMap.clear();
		sun_tex = GFX::g_TextureManager->loadTex("./assets/minecraft/textures/environment/sun.png", GFX_FILTER_NEAREST, GFX_FILTER_NEAREST, false);
		sun = new Rendering::SkyLight(sun_tex);
		mun_tex = GFX::g_TextureManager->loadTex("./assets/minecraft/textures/environment/moon.png", GFX_FILTER_NEAREST, GFX_FILTER_NEAREST, false);
		mun = new Rendering::SkyLight(mun_tex);
		lastPos = { -1000, -1000 };
	}
	void World::cleanup()
	{
		if (chunkMap.size() > 0) {
			for (auto& [v, c] : chunkMap) {
				delete c;
			}
			chunkMap.clear();
		}
		chunkMap.clear();
	}
	void World::update()
	{
		if (!is_loading) {
			timer.deltaTime();

			if(timer.elapsed() > 0.050f){
				timer.reset();
				tickUpdate();
			}
			player->update();
		}
	}
	void World::tickUpdate()
	{
		timeData.age++;
		timeData.timeOfDay++;
		sky->Update(timeData.timeOfDay);
		sun->Update(timeData.timeOfDay);
		mun->Update(timeData.timeOfDay + 12000);

		if (timeData.age % 5 == 0) {
			//Only 4x per frame
			chunkGenUpdate();
		}
	}
	
	void World::chunkGenUpdate()
	{
		//WORLD MANAGEMENT
		glm::ivec3 v = { (float)((int)(player->x / (16.0f))), (float)((int)(player->z / (16.0f))), (float)((int)(player->y / (16.0f))) };

		//Generate a 5x5 of meshes
		glm::vec2 topLeft = { v.x - 2, v.y - 2 };
		glm::vec2 botRight = { v.x + 2, v.y + 2 };

		std::vector <mc::Vector3i> needed;
		needed.clear();

		//Needed
		for (int x = topLeft.x; x <= botRight.x; x++) {
			for (int z = topLeft.y; z <= botRight.y; z++) {
				needed.push_back({ x, z, 0 });
			}
		}

		//Excess
		for (auto& [pos, chunk] : chunkMap) {
			bool need = false;
			for (auto& v : needed) {
				if (v == pos) {
					//Is needed
					need = true;
				}
			}
		}

		//Make new meshes - check if already exist
		for (auto chk : needed) {
			
			//One, it exists
			if (chunkMap.find(chk) != chunkMap.end()) {
				auto chunkStack = chunkMap[chk];

				//Now we gotta check if the section exists
				for (int i = v.z - 2; i <= v.z + 2; i++) {

					if (i >= 0 && i < 16) {
						//Now we can expect results.
						auto chunkSects = chunkStack->getSection(i);
						if (chunkSects != NULL) {
							//Okay so it exists
							//Check if the mesh exists
							if (chunkSects->mesh == NULL) {
								std::cout << "GEN MESH" << std::endl;

								chunkSects->mesh = new ChunkMesh();
							}
						}
					}
				}

			}
		}

	}

	void World::draw()
	{
		GFX::g_RenderCore->set3DMode();
		sky->Draw(player->getCamera());

		sun->Draw(player->getCamera());
		mun->Draw(player->getCamera());

		GFX::g_RenderCore->setDefault2DMode();
		player->draw();
	}

	World* g_World = NULL;
}