#include "SceneLayer/SceneLayer.h"

SceneLayer::SceneLayer(VkDevice d)
	: dev(d)
{
}

SceneLayer::~SceneLayer()
{
}

void SceneLayer::SetActiveScene(const std::string& name)
{
	for (const auto& s : scenes)
	{
		if (s->GetName() == name)
		{
			activeScene = s;
			return;
		}
	}
}

void SceneLayer::Update(float deltaTime)
{
	if (activeScene)
	{
		activeScene->Update(deltaTime);
	}
}