#include <glm/glm.hpp>

#include "Driver.h"

#include "Utilities.h"

#include "KeyboardManager.h"
#include "MouseManager.h"

#include "Camera.h"
#include "CameraController.h"

//************************************************************************************************************************
// public↓
//************************************************************************************************************************

Driver::Driver()
{
	rr = std::make_unique<ResourceRegistry>();
	rr->RegisterResource<int>(RegistryKeys::MaxFramesInFlight, MAX_FRAMES_IN_FLIGHT);

	camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	//システム統合レイヤー初期化
	systemIntegrationLayer = std::make_unique<SystemIntegrationLayer>(*rr.get(),
		true, INIT_WIDTH, INIT_HEIGHT, WINDOW_TITLE);

	//デバイス抽象化レイヤー初期化
	hardwareAbstractionLayer = std::make_unique<HardwareAbstractionLayer>(*rr.get());

	//描画リソースレイヤー初期化
	drawResourceLayer = std::make_unique<DrawResourceLayer>(*rr.get());

	//MVPUniformBuffer、３つの板ポリの頂点,インデックスバッファを作成(ResourceRegistryに登録)
	drawResourceLayer->CreateMVPUniformBuffers(MAX_FRAMES_IN_FLIGHT);
	vertexBuffer = drawResourceLayer->CreateVertexBuffer(vertices);
	indexBuffer  = drawResourceLayer->CreateIndexBuffer(indices);
	drawResourceLayer->LoadTexture(textureNames[0], "Textures\\testTexture.png");
	std::cout << "現状テクスチャは１枚のみ対応" << std::endl;

	renderingLayer = std::make_unique<RenderingLayer>(*rr.get(), *camera.get(),
		vertexBuffer, indexBuffer, indices,
		[&](std::string textureName) {return drawResourceLayer->GetTextureImageView(textureName);},
		[&](std::string textureName) {return drawResourceLayer->GetTextureSampler(textureName);},
		[&](const std::vector<VkImage>& scImgs, VkFormat f) {return drawResourceLayer->CreateSwapChainImageViews(scImgs, f);},
		[&](VkExtent2D e) {return drawResourceLayer->CreateDepthAttachmentImage(e);},
		[&](uint32_t currentImg, const Utils::MVPUBO& ubo) {return drawResourceLayer->UploadMVPUniformBuffers(currentImg, ubo);});

	auto& winRef	= *rr->FindResource<GLFWwindow*>(RegistryKeys::Window);
	keyboardManager = std::make_unique<KeyboardManager>(winRef);
	mouseManager	= std::make_unique<MouseManager>(winRef);

	//シーンレイヤー初期化
	auto dev = rr->FindResource<VkDevice>(RegistryKeys::Device);
	sceneLayer = std::make_unique<SceneLayer>(dev);

	cameraController = std::make_unique<CameraController>(*camera.get(), 2.5f);

	keyboardManager->RegisterKey(GLFW_KEY_W);
	keyboardManager->RegisterKey(GLFW_KEY_A);
	keyboardManager->RegisterKey(GLFW_KEY_S);
	keyboardManager->RegisterKey(GLFW_KEY_D);
	keyboardManager->RegisterKey(GLFW_KEY_SPACE);
	keyboardManager->RegisterKey(GLFW_KEY_LEFT_SHIFT);
}

Driver::~Driver()
{
	auto dev = rr->FindResource<VkDevice>(RegistryKeys::Device);
	vkDeviceWaitIdle(dev);

	//上位のレイヤーから明示的に解放する
	sceneLayer.reset();
	renderingLayer.reset();

	if (vertexBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(dev, vertexBuffer, nullptr);
		vertexBuffer = VK_NULL_HANDLE;
	}
	if (indexBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(dev, indexBuffer, nullptr);
		indexBuffer = VK_NULL_HANDLE;
	}

	drawResourceLayer.reset();
	hardwareAbstractionLayer.reset();
	systemIntegrationLayer.reset();

	glfwTerminate();
}

void Driver::Loop()
{
	if (Tick(deltaTime) == false)
	{
		return;
	}

	keyboardManager->Update();
	mouseManager->Update();

	cameraController->Update(*keyboardManager, *mouseManager, deltaTime);

	renderingLayer->Render(loopFlag, deltaTime);
}

//************************************************************************************************************************
// private↓
//************************************************************************************************************************

bool Driver::Tick(float& deltaTime)
{
//endTimeとstartTimeの差で経過時間を計測し,指定FPSに達すればtrue
	endTime = std::chrono::high_resolution_clock::now();
	if (std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() == 0) return false;

	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000000.0f;
	if (deltaTime < (1.0f / (FrameRate + 0.01f))) return false; //切り上げ誤差を防ぐために0.01fを足す(目標FPSより早く更新処理を行える)

	startTime = endTime;
	deltaTime = (deltaTime > MaxDeltaTime) ? MaxDeltaTime : deltaTime;

	return true;
}