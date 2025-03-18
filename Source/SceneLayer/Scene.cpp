#include "SceneLayer/Scene.h"

Scene::Scene(VkDevice d, const std::string& name)
	: dev(d)
	, name(name)
{
}

Scene::~Scene()
{
}

void Scene::Update(float deltaTime)
{
}