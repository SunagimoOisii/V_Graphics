#include <memory>
#include <vector>

#include "Scene.h"

/// <summary>
/// シーン全体を管理するレイヤーラップクラス<para></para>
/// レイヤー内クラスの管理に加え、モデルのロードも行う<para></para>
/// </summary>
class SceneLayer
{
public:
    SceneLayer(VkDevice d);
    ~SceneLayer();

//***********************************************************
// ゲッターセッター↓
//***********************************************************

    const std::shared_ptr<Scene>& GetActiveScene() const { return activeScene; }

    void SetActiveScene(const std::string& sceneName); //シーン名の重複には未対応
    void AddScene(const std::shared_ptr<Scene>& s) { scenes.push_back(s); }

//***********************************************************
// 関数↓
//***********************************************************

    void Update(float deltaTime);

private:
    VkDevice dev;

    std::vector<std::shared_ptr<Scene>> scenes;
    std::shared_ptr<Scene>              activeScene;
};