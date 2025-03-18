#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/// <summary>
/// カメラとしての座標,注視点,ワールド座標系における上向きの単位ベクトルの情報と、
/// それらをもとにビュー行列を計算する機能を持つクラス
/// </summary>
class Camera
{
public:
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up);
    ~Camera();

//***********************************************************
// ゲッターセッター↓
//***********************************************************

    glm::vec3 GetPosition()   const { return position; };
    glm::vec3 GetTarget()     const { return target; };

    void SetPosition(const glm::vec3& pos) { position = pos; };
    void SetTarget(const glm::vec3& tar) { target = tar; };

//***********************************************************
// 関数↓
//***********************************************************

    glm::mat4 CalcViewMatrix() const;

private:
//***********************************************************
// 変数↓
//***********************************************************

    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
};