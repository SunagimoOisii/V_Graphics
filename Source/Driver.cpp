#include <glm/glm.hpp>

#include "Driver.h"

#include "Utilities.h"

#include "KeyboardManager.h"
#include "MouseManager.h"

#include "Camera.h"
#include "CameraController.h"

//************************************************************************************************************************
// public��
//************************************************************************************************************************

Driver::Driver()
{
	rr = std::make_unique<ResourceRegistry>();
	rr->RegisterResource<int>(RegistryKeys::MaxFramesInFlight, MAX_FRAMES_IN_FLIGHT);

	camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	//�V�X�e���������C���[������
	systemIntegrationLayer = std::make_unique<SystemIntegrationLayer>(*rr.get(),
		true, INIT_WIDTH, INIT_HEIGHT, WINDOW_TITLE);

	//�f�o�C�X���ۉ����C���[������
	hardwareAbstractionLayer = std::make_unique<HardwareAbstractionLayer>(*rr.get());

	//�`�惊�\�[�X���C���[������
	drawResourceLayer = std::make_unique<DrawResourceLayer>(*rr.get());

	//MVPUniformBuffer�A�R�̔|���̒��_,�C���f�b�N�X�o�b�t�@���쐬(ResourceRegistry�ɓo�^)
	drawResourceLayer->CreateMVPUniformBuffers(MAX_FRAMES_IN_FLIGHT);
	vertexBuffer = drawResourceLayer->CreateVertexBuffer(vertices);
	indexBuffer  = drawResourceLayer->CreateIndexBuffer(indices);
	drawResourceLayer->LoadTexture(textureNames[0], "Textures\\testTexture.png");
	std::cout << "����e�N�X�`���͂P���̂ݑΉ�" << std::endl;

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

	//�V�[�����C���[������
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

	//��ʂ̃��C���[���疾���I�ɉ������
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
// private��
//************************************************************************************************************************

bool Driver::Tick(float& deltaTime)
{
//endTime��startTime�̍��Ōo�ߎ��Ԃ��v����,�w��FPS�ɒB�����true
	endTime = std::chrono::high_resolution_clock::now();
	if (std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() == 0) return false;

	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000000.0f;
	if (deltaTime < (1.0f / (FrameRate + 0.01f))) return false; //�؂�グ�덷��h�����߂�0.01f�𑫂�(�ڕWFPS��葁���X�V�������s����)

	startTime = endTime;
	deltaTime = (deltaTime > MaxDeltaTime) ? MaxDeltaTime : deltaTime;

	return true;
}