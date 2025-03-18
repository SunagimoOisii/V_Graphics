#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Model.h"

/// <summary>
/// ���f��,�J����,���C�g���ꊇ�Ǘ�����N���X
/// </summary>
class Scene
{
public:
    Scene(VkDevice d, const std::string& name);
    ~Scene();

//***********************************************************
// �Q�b�^�[�Z�b�^�[��
//***********************************************************

    const std::string GetName() const { return name; }
    //const std::shared_ptr<Camera>& GetCamera() const { return camera; }
    const std::vector<std::shared_ptr<Model>>& GetModels() const { return models; }
    //const std::vector<std::shared_ptr<Light>>& GetLights() const { return lights; }

    //void SetCamera(const std::shared_ptr<Camera>& cam) { camera = cam; }
    void AddModel(const std::shared_ptr<Model>& model) { models.push_back(model); }
    //void AddLight(const std::shared_ptr<Light>& light) { lights.push_back(light); }

//***********************************************************
// �֐���
//***********************************************************

    void Update(float deltaTime);

private:
//***********************************************************
// �ϐ���
//***********************************************************

    VkDevice dev;

    const std::string name;

    //std::shared_ptr<Camera> camera;
    std::vector<std::shared_ptr<Model>> models;
    //std::vector<std::shared_ptr<Light>> lights;
};
