#include <memory>
#include <vector>

#include "Scene.h"

/// <summary>
/// �V�[���S�̂��Ǘ����郌�C���[���b�v�N���X<para></para>
/// ���C���[���N���X�̊Ǘ��ɉ����A���f���̃��[�h���s��<para></para>
/// </summary>
class SceneLayer
{
public:
    SceneLayer(VkDevice d);
    ~SceneLayer();

//***********************************************************
// �Q�b�^�[�Z�b�^�[��
//***********************************************************

    const std::shared_ptr<Scene>& GetActiveScene() const { return activeScene; }

    void SetActiveScene(const std::string& sceneName); //�V�[�����̏d���ɂ͖��Ή�
    void AddScene(const std::shared_ptr<Scene>& s) { scenes.push_back(s); }

//***********************************************************
// �֐���
//***********************************************************

    void Update(float deltaTime);

private:
    VkDevice dev;

    std::vector<std::shared_ptr<Scene>> scenes;
    std::shared_ptr<Scene>              activeScene;
};