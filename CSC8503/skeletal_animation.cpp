//#include <glad/glad.h>
//
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//
//#include "shader.h"
//#include "../NewOpenGL/camera.h"
//#include "animator.h"
//#include "model_animation.h"
//#include "Instantiate.h"
//
//#include <stb_image.h>
//
//#include <vector>
//#include <iostream>
//#include <random>
//
//
//using namespace OpenGL;
//
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void mouse_callback(GLFWwindow* window, double xpos, double ypos);
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void processInput(GLFWwindow* window);
//unsigned int loadCubemap(std::vector<std::string> faces);
//unsigned int loadTexture(const char* path);
//
//
//
////È«¾ÖÉèÖÃ-------------------------------------------------------------------------------------
//const unsigned int SCR_WIDTH = 1920;
//const unsigned int SCR_HEIGHT = 1080;
//
//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
//float lastX = SCR_WIDTH / 2.0f;
//float lastY = SCR_HEIGHT / 2.0f;
//bool firstMouse = true;
//
//float deltaTime = 0.0f;
//float lastFrame = 0.0f;
////È«¾ÖÉèÖÃ-------------------------------------------------------------------------------------
//
//
//
//
//int main()
//{
//	//OpenGL³õÊ¼»¯ÉèÖÃ-------------------------------------------------------------------------------------
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
//	if (window == NULL)
//	{
//		std::cout << "Failed to create GLFW window" << std::endl;
//		glfwTerminate();
//		return -1;
//	}
//	glfwMakeContextCurrent(window);
//	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//	glfwSetCursorPosCallback(window, mouse_callback);
//	glfwSetScrollCallback(window, scroll_callback);
//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//	{
//		std::cout << "Failed to initialize GLAD" << std::endl;
//		return -1;
//	}
//	stbi_set_flip_vertically_on_load(true);
//	glEnable(GL_DEPTH_TEST);
//	//OpenGL³õÊ¼»¯ÉèÖÃ-------------------------------------------------------------------------------------
//
//
//
//
//	//Ä£ÐÍºÍ¶¯»­ÉèÖÃ-------------------------------------------------------------------------------------
//	Shader modelShader("../NewOpenGL/Shaders/anim_model_Instance.vs", "../NewOpenGL/Shaders/anim_model.fs");
//
//	Model animationModel("../Assets/tiger_monster_Useable/scene.gltf");
//	Animation animation("../Assets/tiger_monster_Useable/scene.gltf",&animationModel);
//	std::vector<Animator> animators;
//	int animNumber = animation.animations.size();
//	for (const auto& pair : animation.animations) {
//		Animation* anim = pair.second;
//		Animator animator(anim);
//		animators.push_back(animator);
//	}
//
//	//Ä£ÐÍºÍ¶¯»­ÉèÖÃ-------------------------------------------------------------------------------------
//
//
//
//
//
//
//
//	//Ëæ»úÉèÖÃ-------------------------------------------------------------------------------------
//	const int NUM_MODELS = 500;
//	std::vector<glm::vec3> modelPositions;
//
//	std::random_device rd;
//	std::mt19937 gen(rd());
//	std::uniform_real_distribution<float> distX(-40.0f, 40.0f); // ÏÞÖÆXÖá·¶Î§
//	std::uniform_real_distribution<float> distZ(-40.0f, 40.0f); // ÏÞÖÆZÖá·¶Î§
//
//	for (int i = 0; i < NUM_MODELS; ++i) {
//		float x = distX(gen);
//		float z = distZ(gen);
//		modelPositions.push_back(glm::vec3(x, 0.0f, z));  // Y ÉèÎª 0£¬·ÅÔÚµØÃæÉÏ
//	}
//	//Ëæ»úÉèÖÃ-------------------------------------------------------------------------------------
//
//
//
//
//
//
//	// ÊµÀý»¯äÖÈ¾¶ÔÏó-------------------------------------------------------------------------------------
//	Instantiate instanceRenderer(animationModel);
//
//	// Ìí¼Ó 100 ¸öËæ»úÎ»ÖÃµÄÊµÀý
//	for (int i = 0; i < NUM_MODELS; ++i) {
//		glm::mat4 model = glm::mat4(1.0f);
//		model = glm::translate(model, modelPositions[i]);
//		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//		model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));
//
//		instanceRenderer.AddInstance(model);
//	}
//
//	// Íê³ÉÊµÀý»¯»º³åÇøÉèÖÃ
//	instanceRenderer.SetupInstances();
//	// ÊµÀý»¯äÖÈ¾¶ÔÏó-------------------------------------------------------------------------------------
//
//
//
//
//
//
//
//	// ÊµÀý»¯¶¯»­¿ØÖÆ-------------------------------------------------------------------------------------
//	GLuint boneSSBO;
//	const int MAX_BONES = 200; 
//	const int MAX_INSTANCES = 10000;  
//
//	glGenBuffers(1, &boneSSBO);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, boneSSBO);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_INSTANCES * MAX_BONES * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, boneSSBO);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//	// ÊµÀý»¯¶¯»­¿ØÖÆ-------------------------------------------------------------------------------------
//
//
//
//
//
//
//
//
//	//Ìì¿ÕºÐÉèÖÃ-------------------------------------------------------------------------------------
//	Shader skyboxShader("../NewOpenGL/Shaders/skybox.vs", "../NewOpenGL/Shaders/skybox.fs");
//
//
//	std::vector<std::string> faces = {
//	"../Assets/skybox/right.jpg", 
//	"../Assets/skybox/left.jpg", 
//	"../Assets/skybox/bottom.jpg",
//	"../Assets/skybox/top.jpg",
//	"../Assets/skybox/front.jpg", 
//	"../Assets/skybox/back.jpg"
//	};
//
//	int cubemapTexture = loadCubemap(faces);
//
//	float skyboxVertices[] = {
//		// Positions          
//		-1.0f,  1.0f, -1.0f,
//		-1.0f, -1.0f, -1.0f,
//		 1.0f, -1.0f, -1.0f,
//		 1.0f, -1.0f, -1.0f,
//		 1.0f,  1.0f, -1.0f,
//		-1.0f,  1.0f, -1.0f,
//
//		-1.0f, -1.0f,  1.0f,
//		-1.0f, -1.0f, -1.0f,
//		-1.0f,  1.0f, -1.0f,
//		-1.0f,  1.0f, -1.0f,
//		-1.0f,  1.0f,  1.0f,
//		-1.0f, -1.0f,  1.0f,
//
//		 1.0f, -1.0f, -1.0f,
//		 1.0f, -1.0f,  1.0f,
//		 1.0f,  1.0f,  1.0f,
//		 1.0f,  1.0f,  1.0f,
//		 1.0f,  1.0f, -1.0f,
//		 1.0f, -1.0f, -1.0f,
//
//		-1.0f, -1.0f,  1.0f,
//		-1.0f,  1.0f,  1.0f,
//		 1.0f,  1.0f,  1.0f,
//		 1.0f,  1.0f,  1.0f,
//		 1.0f, -1.0f,  1.0f,
//		-1.0f, -1.0f,  1.0f,
//
//		-1.0f,  1.0f, -1.0f,
//		 1.0f,  1.0f, -1.0f,
//		 1.0f,  1.0f,  1.0f,
//		 1.0f,  1.0f,  1.0f,
//		-1.0f,  1.0f,  1.0f,
//		-1.0f,  1.0f, -1.0f,
//
//		-1.0f, -1.0f, -1.0f,
//		-1.0f, -1.0f,  1.0f,
//		 1.0f, -1.0f, -1.0f,
//		 1.0f, -1.0f, -1.0f,
//		-1.0f, -1.0f,  1.0f,
//		 1.0f, -1.0f,  1.0f
//	};
//
//	unsigned int skyboxVAO, skyboxVBO;
//	glGenVertexArrays(1, &skyboxVAO);
//	glGenBuffers(1, &skyboxVBO);
//	glBindVertexArray(skyboxVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	//Ìì¿ÕºÐÉèÖÃ-------------------------------------------------------------------------------------
//
//
//
//
//	//µØÃæÉèÖÃ-------------------------------------------------------------------------------------
//	unsigned int floorTexture = loadTexture("../Assets/floor/floor.jpg");
//
//	// ´´½¨µØÃæ Shader
//	Shader floorShader("../NewOpenGL/Shaders/floor.vs", "../NewOpenGL/Shaders/floor.fs");
//	floorShader.use();
//	floorShader.setInt("texture1", 0);
//
//
//	float planeVertices[] = {
//		// Î»ÖÃ               // ÎÆÀí×ø±ê
//		5.0f,  0.0f,  5.0f,   5.0f, 0.0f,
//	   -5.0f,  0.0f,  5.0f,   0.0f, 0.0f,
//	   -5.0f,  0.0f, -5.0f,   0.0f, 5.0f,
//
//		5.0f,  0.0f,  5.0f,   5.0f, 0.0f,
//	   -5.0f,  0.0f, -5.0f,   0.0f, 5.0f,
//		5.0f,  0.0f, -5.0f,   5.0f, 5.0f
//	};
//
//	unsigned int planeVAO, planeVBO;
//	glGenVertexArrays(1, &planeVAO);
//	glGenBuffers(1, &planeVBO);
//	glBindVertexArray(planeVAO);
//
//	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
//
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
//
//	glEnableVertexAttribArray(1);
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
//
//	glBindVertexArray(0);
//	//µØÃæÉèÖÃ-------------------------------------------------------------------------------------
//
//
//
//
//	//Ö±Éä¹âÉèÖÃ-------------------------------------------------------------------------------------
//	glm::vec3 lightDirection = glm::normalize(glm::vec3(0.5f, -1.0f, 0.5f));
//	glm::vec3 lightColor = glm::vec3(1.0f, 0.95f, 0.9f); // ÇáÎ¢Æ«Å¯É«
//	glm::vec3 lightPos(-2.0f, 50.0f, -1.0f);
//	//Ö±Éä¹âÉèÖÃ-------------------------------------------------------------------------------------
//
//
//
//
//	//ÒõÓ°ÉèÖÃ-------------------------------------------------------------------------------------
//	Shader shadowShader("../NewOpenGL/Shaders/shadow_depth_Instance.vs", "../NewOpenGL/Shaders/shadow_depth.fs");
//	// ¶¨ÒåÒõÓ°ÌùÍ¼³ß´ç£¨¿É¸ù¾ÝÐèÒªµ÷Õû£©
//	const unsigned int SHADOW_WIDTH = 400, SHADOW_HEIGHT = 400;
//	unsigned int depthMapFBO;
//	glGenFramebuffers(1, &depthMapFBO);
//
//	// ´´½¨Éî¶ÈÎÆÀí
//	unsigned int depthMap;
//	glGenTextures(1, &depthMap);
//	glBindTexture(GL_TEXTURE_2D, depthMap);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
//		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	// ÉèÖÃ±ßÔµ²ÉÑùÎª°×É«£¨±íÊ¾²»ÔÚÒõÓ°ÖÐ£©
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
//
//	// ½«Éî¶ÈÎÆÀí¸½¼Óµ½Ö¡»º³åµÄÉî¶È¸½¼þÉÏ
//	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
//	glDrawBuffer(GL_NONE); // ²»ÐèÒªÑÕÉ«»º³å
//	glReadBuffer(GL_NONE);
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//	//ÒõÓ°ÉèÖÃ-------------------------------------------------------------------------------------
//
//
//
//
//
//
//	while (!glfwWindowShouldClose(window))
//	{
//		float currentFrame = glfwGetTime();
//		deltaTime = currentFrame - lastFrame;
//		lastFrame = currentFrame;
//		processInput(window);
//
//
//		animators[1].UpdateAnimation(deltaTime);
//
//
//		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//
//		glEnable(GL_CULL_FACE); // ÆôÓÃÃæÌÞ³ý
//
//
//
//        //ÒõÓ°-------------------------------------------------------------------------------------
//		// ÏÈ»æÖÆÒõÓ°ÌùÍ¼
//		glm::mat4 lightProjection, lightView;
//		glm::mat4 lightSpaceMatrix;
//		float near_plane = 1.0f, far_plane = 60.5f;
//		lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
//		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
//		lightSpaceMatrix = lightProjection * lightView;
//
//		// °ó¶¨FBO & ÉèÖÃÊÓ¿Ú
//		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
//		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
//		glClear(GL_DEPTH_BUFFER_BIT);
//
//		// ÅäÖÃÒõÓ°×ÅÉ«Æ÷
//		{
//			shadowShader.use();
//			shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
//			auto transforms = animators[1].GetFinalBoneMatrices();
//			for (int j = 0; j < transforms.size(); ++j)
//				shadowShader.setMat4("finalBonesMatrices[" + std::to_string(j) + "]", transforms[j]);
//
//			instanceRenderer.Draw(shadowShader);
//
//			glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//
//		}
//
//		// »Ö¸´Ä¬ÈÏFBO & ÊÓ¿Ú
//		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//		//ÒõÓ°-------------------------------------------------------------------------------------
//
//
//
//
//
//		//Ä£ÐÍ-------------------------------------------------------------------------------------
//		modelShader.use();
//		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//		glm::mat4 view = camera.GetViewMatrix();
//		modelShader.setMat4("projection", projection);
//		modelShader.setMat4("view", view);
//		modelShader.setVec3("lightDirection", lightDirection);
//		modelShader.setVec3("lightColor", lightColor);
//		modelShader.setVec3("viewPos", camera.Position);
//		auto transforms = animators[1].GetFinalBoneMatrices();
//		for (int j = 0; j < transforms.size(); ++j)
//			modelShader.setMat4("finalBonesMatrices[" + std::to_string(j) + "]", transforms[j]);
//
//		instanceRenderer.Draw(modelShader);
//
//		//Ä£ÐÍ-------------------------------------------------------------------------------------
//
//
//
//		glDisable(GL_CULL_FACE);
//
//
//
//		//µØÃæ-------------------------------------------------------------------------------------
//		floorShader.use();
//		glm::mat4 model = glm::mat4(1.0f);
//		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // ÏÈÎ»ÒÆ
//		model = glm::scale(model, glm::vec3(10.1f, 10.1f, 10.1f)); // ÔÙ·Å´ó
//		floorShader.setMat4("model", model);
//		floorShader.setMat4("view", camera.GetViewMatrix());
//		floorShader.setMat4("projection", projection);
//		floorShader.setVec3("lightDirection", lightDirection);
//		floorShader.setVec3("lightColor", lightColor);
//		floorShader.setVec3("viewPos", camera.Position);
//		floorShader.setInt("texture_diffuse1", 0);
//		floorShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
//
//		//ÒõÓ°£º¸æËß²ÉÑùÆ÷£¬shadowMap ÔÚÎÆÀíµ¥Ôª1
//		floorShader.setInt("shadowMap", 1);
//		glActiveTexture(GL_TEXTURE1);
//		glBindTexture(GL_TEXTURE_2D, depthMap);
//
//		// »æÖÆµØÃæ
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, floorTexture);
//		glBindVertexArray(planeVAO);
//		glDrawArrays(GL_TRIANGLES, 0, 6);
//		glBindVertexArray(0);
//		//µØÃæ-------------------------------------------------------------------------------------
//
//
//
//
//		//Ìì¿ÕºÐ-------------------------------------------------------------------------------------
//		skyboxShader.use();
//		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // ÒÆ³ýÆ½ÒÆ²¿·Ö
//		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // ÏÈÈ¥µôÎ»ÒÆ
//		view[0][2] *= -1; // ·­×ª X ÖáµÄ Z ·ÖÁ¿
//		view[1][2] *= -1; // ·­×ª Y ÖáµÄ Z ·ÖÁ¿
//		view[2][2] *= -1; // ·­×ª Z ÖáµÄ Z ·ÖÁ¿
//		projection = glm::perspective(-glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//		skyboxShader.setMat4("view", view);
//		skyboxShader.setMat4("projection", projection);
//
//		// äÖÈ¾Ìì¿ÕºÐ
//		glDepthFunc(GL_LEQUAL);
//		glBindVertexArray(skyboxVAO);
//		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
//		glDrawArrays(GL_TRIANGLES, 0, 36);
//		glBindVertexArray(0);
//		glDepthFunc(GL_LESS);
//		//Ìì¿ÕºÐ-------------------------------------------------------------------------------------
//
//
//
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//	}
//
//	// glfw: terminate, clearing all previously allocated GLFW resources.
//	// ------------------------------------------------------------------
//	glfwTerminate();
//	return 0;
//}
//
//// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
//// ---------------------------------------------------------------------------------------------------------
//void processInput(GLFWwindow* window)
//{
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, true);
//
//	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//		camera.ProcessKeyboard(FORWARD, deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//		camera.ProcessKeyboard(BACKWARD, deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//		camera.ProcessKeyboard(LEFT, deltaTime);
//	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//		camera.ProcessKeyboard(RIGHT, deltaTime);
//}
//
//// glfw: whenever the window size changed (by OS or user resize) this callback function executes
//// ---------------------------------------------------------------------------------------------
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	// make sure the viewport matches the new window dimensions; note that width and 
//	// height will be significantly larger than specified on retina displays.
//	glViewport(0, 0, width, height);
//}
//
//// glfw: whenever the mouse moves, this callback is called
//// -------------------------------------------------------
//void mouse_callback(GLFWwindow* window, double xpos, double ypos)
//{
//	if (firstMouse)
//	{
//		lastX = xpos;
//		lastY = ypos;
//		firstMouse = false;
//	}
//
//	float xoffset = xpos - lastX;
//	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
//
//	lastX = xpos;
//	lastY = ypos;
//
//	camera.ProcessMouseMovement(xoffset, yoffset);
//}
//
//// glfw: whenever the mouse scroll wheel scrolls, this callback is called
//// ----------------------------------------------------------------------
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
//{
//	camera.ProcessMouseScroll(yoffset);
//}
//
//
//// Ìì¿ÕºÐ
//unsigned int loadCubemap(std::vector<std::string> faces) {
//	unsigned int textureID;
//	glGenTextures(1, &textureID);
//	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
//
//	int width, height, nrChannels;
//	for (unsigned int i = 0; i < faces.size(); i++) {
//		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
//		if (data) {
//			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
//				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
//			);
//			stbi_image_free(data);
//		}
//		else {
//			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
//			stbi_image_free(data);
//		}
//	}
//
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//	return textureID;
//}
//
//// ¼ÓÔØµØÃæ
//unsigned int loadTexture(const char* path)
//{
//	unsigned int textureID;
//	glGenTextures(1, &textureID);
//
//	int width, height, nrComponents;
//	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
//	if (data)
//	{
//		GLenum format = GL_RGB;
//		if (nrComponents == 1)
//			format = GL_RED;
//		else if (nrComponents == 3)
//			format = GL_RGB;
//		else if (nrComponents == 4)
//			format = GL_RGBA;
//
//		glBindTexture(GL_TEXTURE_2D, textureID);
//		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//		glGenerateMipmap(GL_TEXTURE_2D);
//
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//		stbi_image_free(data);
//	}
//	else
//	{
//		std::cout << "Failed to load texture at path: " << path << std::endl;
//		stbi_image_free(data);
//	}
//
//	return textureID;
//}
