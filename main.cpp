// This has been adapted from the Vulkan tutorial

#include "modules/Starter.hpp"

// The uniform buffer objects data structures
// Remember to use the correct alignas(...) value
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)
struct UniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
};

struct GlobalUniformBufferObject {
	alignas(16) glm::vec3 lightDir;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
	alignas(16) glm::vec4 eyeDir;
};

// The vertices data structures
struct Vertex {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};


class ConfigManager : public BaseProject {
protected:
	// Current aspect ratio (used by the callback that resized the window
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSL;

	// Vertex formats
	VertexDescriptor VD;

	// Pipelines [Shader couples]
	Pipeline PBlinn;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex
	// structure Models
	Model<Vertex> MRocket;
	Model<Vertex> MWallN;
	Model<Vertex> MWallE;
	Model<Vertex> MWallS;
	Model<Vertex> MWallW;
	Model<Vertex> MWindow1;
	Model<Vertex> MWindow2;
	Model<Vertex> MFloor;
	Model<Vertex> MBed;
	Model<Vertex> MCloset;
	Model<Vertex> MDesk;
	Model<Vertex> MGamingDesk;
	Model<Vertex> MRedColumn;
	Model<Vertex> MWhiteColumn;
	Model<Vertex> MCoinSack;
	Model<Vertex> MCoinStack;
	Model<Vertex> MDoor;

	// Descriptor sets
	DescriptorSet DSRocket;
	DescriptorSet DSWallN;
	DescriptorSet DSWallE;
	DescriptorSet DSWallS;
	DescriptorSet DSWallW;
	DescriptorSet DSWindow1;
	DescriptorSet DSWindow2;
	DescriptorSet DSFloor;
	DescriptorSet DSBed;
	DescriptorSet DSCloset;
	DescriptorSet DSDesk;
	DescriptorSet DSGamingDesk;
	DescriptorSet DSDoor;
	DescriptorSet DSRedColumn;
	DescriptorSet DSWhiteColumn;
	DescriptorSet DSCoinSack;
	DescriptorSet DSCoinStack;

	// Textures
	Texture TFurniture;

	// C++ storage for uniform variables
	UniformBufferObject RocketUbo;
	UniformBufferObject WallNUbo;
	UniformBufferObject WallEUbo;
	UniformBufferObject WallSUbo;
	UniformBufferObject WallWUbo;
	UniformBufferObject Window1Ubo;
	UniformBufferObject Window2Ubo;
	UniformBufferObject FloorUbo;
	UniformBufferObject BedUbo;
	UniformBufferObject ClosetUbo;
	UniformBufferObject DeskUbo;
	UniformBufferObject GamingDeskUbo;
	UniformBufferObject RedColumnUbo;
	UniformBufferObject WhiteColumnUbo;
	UniformBufferObject CoinSackUbo;
	UniformBufferObject CoinStackUbo;
	UniformBufferObject DoorUbo;
	// Other application parameters

	// Here you set the main application parameters
	void setWindowParameters() override {
		// window size, titile and initial background
		windowWidth = 1920;
		windowHeight = 1080;
		windowTitle = "Project";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.5f, 0.5f, 0.6f, 1.0f};

		// Descriptor pool sizes
		uniformBlocksInPool = 50;
		texturesInPool = 50;
		setsInPool = 50;

		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) override { Ar = (float)w / (float)h; }

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() override {
		// init descriptor layouts [what will be passed to the shaders]
		DSL.init(this,
		         {// this array contains the bindings:
		          // first  element : the binding number
		          // second element : the type of element (buffer or texture)
		          // using the corresponding Vulkan constant
		          // third  element : the pipeline stage where it will be used
		          // using the corresponding Vulkan constant
		          {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
		          {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
		          {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT}});

		// init vertex descriptors
		VD.init(this, {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}},
		        {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
		          sizeof(glm::vec3), POSITION},
		         {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
		          sizeof(glm::vec3), NORMAL},
		         {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
		          sizeof(glm::vec2), UV}});

		// init pipelines
		PBlinn.init(this, &VD, "shaders/LambertBlinnShaderVert.spv",
		            "shaders/LambertBlinnShaderFrag.spv", {&DSL});

		// init models
		MRocket.init(this, &VD, "models/desk_lamp.mgcg", MGCG);
		MWallN.init(this, &VD, "models/gray_wall.mgcg", MGCG);
		MWallE.init(this, &VD, "models/gray_wall.mgcg", MGCG);
		MWallS.init(this, &VD, "models/gray_wall.mgcg", MGCG);
		MWallW.init(this, &VD, "models/gray_wall.mgcg", MGCG);
		MWindow1.init(this, &VD, "models/window.mgcg", MGCG);
		MWindow2.init(this, &VD, "models/window.mgcg", MGCG);
		MFloor.init(this, &VD, "models/parquet.mgcg", MGCG);
		MBed.init(this, &VD, "models/tower_bed.mgcg", MGCG);
		MGamingDesk.init(this, &VD, "models/gaming_desk.mgcg", MGCG);
		MCloset.init(this, &VD, "models/closet.mgcg", MGCG);
		MDoor.init(this, &VD, "models/door.mgcg", MGCG);
		MDesk.init(this, &VD, "models/study_desk.mgcg", MGCG);
		MRedColumn.init(this, &VD, "models/red_column.mgcg", MGCG);
		MWhiteColumn.init(this, &VD, "models/white_column.mgcg", MGCG);
		MCoinSack.init(this, &VD, "models/coin_sack.mgcg", MGCG);
		MCoinStack.init(this, &VD, "models/coin_stack.mgcg", MGCG);
		// Create the textures
		// The second parameter is the file name
		TFurniture.init(this, "textures/Textures_Forniture.png");

		// Init local variables
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() override {
		// This creates a new pipeline (with the current surface), using its shaders
		PBlinn.create();

		// Here you define the data set
		DSRocket.init(this, &DSL,
		              {// the second parameter, is a pointer to the Uniform Set Layout of this set
		               // the last parameter is an array, with one element per binding of the set.
		               // first  elmenet : the binding number
		               // second element : UNIFORM or TEXTURE (an enum) depending on the type
		               // third  element : only for UNIFORMs, the size of the corresponding C++ object. For texture, just put 0
		               // fourth element : only for TEXTUREs, the pointer to the corresponding texture object. For uniforms, use nullptr
		               {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		               {1, TEXTURE, 0, &TFurniture},
		               {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSWallN.init(this, &DSL,
		             {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		              {1, TEXTURE, 0, &TFurniture},
		              {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSWallE.init(this, &DSL,
		             {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		              {1, TEXTURE, 0, &TFurniture},
		              {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSWallS.init(this, &DSL,
		             {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		              {1, TEXTURE, 0, &TFurniture},
		              {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSWallW.init(this, &DSL,
		             {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		              {1, TEXTURE, 0, &TFurniture},
		              {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSWindow1.init(this, &DSL,
		               {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		                {1, TEXTURE, 0, &TFurniture},
		                {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSWindow2.init(this, &DSL,
		               {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		                {1, TEXTURE, 0, &TFurniture},
		                {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSFloor.init(this, &DSL,
		             {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		              {1, TEXTURE, 0, &TFurniture},
		              {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSBed.init(this, &DSL,
		           {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		            {1, TEXTURE, 0, &TFurniture},
		            {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSGamingDesk.init(this, &DSL,
		           {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		            {1, TEXTURE, 0, &TFurniture},
		            {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSCloset.init(this, &DSL,
		                  {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		                   {1, TEXTURE, 0, &TFurniture},
		                   {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSDoor.init(this, &DSL,
		              {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		               {1, TEXTURE, 0, &TFurniture},
		               {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSDesk.init(this, &DSL,
		            {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		             {1, TEXTURE, 0, &TFurniture},
		             {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSRedColumn.init(this, &DSL,
		                 {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		                  {1, TEXTURE, 0, &TFurniture},
		                  {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSWhiteColumn.init(this, &DSL,
		                 {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		                  {1, TEXTURE, 0, &TFurniture},
		                  {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSCoinSack.init(this, &DSL,
		                   {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		                    {1, TEXTURE, 0, &TFurniture},
		                    {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSCoinStack.init(this, &DSL,
		                {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		                 {1, TEXTURE, 0, &TFurniture},
		                 {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});

	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() override {
		// Cleanup pipelines
		PBlinn.cleanup();

		// Cleanup descriptor sets
		DSRocket.cleanup();
		DSWallN.cleanup();
		DSWallE.cleanup();
		DSWallS.cleanup();
		DSWallW.cleanup();
		DSWindow1.cleanup();
		DSWindow2.cleanup();
		DSFloor.cleanup();
		DSBed.cleanup();
		DSGamingDesk.cleanup();
		DSCloset.cleanup();
		DSDoor.cleanup();
		DSDesk.cleanup();
		DSRedColumn.cleanup();
		DSWhiteColumn.cleanup();
		DSCoinSack.cleanup();
		DSCoinStack.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() override {
		// Cleanup textures
		TFurniture.cleanup();

		// Cleanup models
		MRocket.cleanup();
		MWallN.cleanup();
		MWallE.cleanup();
		MWallS.cleanup();
		MWallW.cleanup();
		MWindow1.cleanup();
		MWindow2.cleanup();
		MFloor.cleanup();
		MBed.cleanup();
		MGamingDesk.cleanup();
		MCloset.cleanup();
		MDoor.cleanup();
		MDesk.cleanup();
		MRedColumn.cleanup();
		MWhiteColumn.cleanup();
		MCoinSack.cleanup();
		MCoinStack.cleanup();

		// Cleanup descriptor set layouts
		DSL.cleanup();

		// Destroys the pipelines
		PBlinn.destroy();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
		// binds the pipeline
		PBlinn.bind(commandBuffer);
		// For a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter

		// binds the data sets
		DSRocket.bind(commandBuffer, PBlinn, 0, currentImage);
		MRocket.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MRocket.indices.size()), 1, 0, 0, 0);

		DSWallN.bind(commandBuffer, PBlinn, 0, currentImage);
		MWallN.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MWallN.indices.size()), 1, 0, 0, 0);

		DSWallE.bind(commandBuffer, PBlinn, 0, currentImage);
		MWallE.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MWallE.indices.size()), 1, 0, 0, 0);

		DSWallS.bind(commandBuffer, PBlinn, 0, currentImage);
		MWallS.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MWallS.indices.size()), 1, 0, 0, 0);

		DSWallW.bind(commandBuffer, PBlinn, 0, currentImage);
		MWallW.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MWallW.indices.size()), 1, 0, 0, 0);

		DSWindow1.bind(commandBuffer, PBlinn, 0, currentImage);
		MWindow1.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MWindow1.indices.size()), 1, 0, 0, 0);

		DSWindow2.bind(commandBuffer, PBlinn, 0, currentImage);
		MWindow2.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MWindow2.indices.size()), 1, 0, 0, 0);

		DSFloor.bind(commandBuffer, PBlinn, 0, currentImage);
		MFloor.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MFloor.indices.size()), 1, 0, 0, 0);

		DSBed.bind(commandBuffer, PBlinn, 0, currentImage);
		MBed.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MBed.indices.size()), 1, 0, 0, 0);
		DSGamingDesk.bind(commandBuffer, PBlinn, 0, currentImage);
		MGamingDesk.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MGamingDesk.indices.size()), 1, 0, 0, 0);

		DSCloset.bind(commandBuffer, PBlinn, 0, currentImage);
		MCloset.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MCloset.indices.size()), 1, 0, 0, 0);

		DSDoor.bind(commandBuffer, PBlinn, 0, currentImage);
		MDoor.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MDoor.indices.size()), 1, 0, 0, 0);

		DSDesk.bind(commandBuffer, PBlinn, 0, currentImage);
		MDesk.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MDesk.indices.size()), 1, 0, 0, 0);

		DSRedColumn.bind(commandBuffer, PBlinn, 0, currentImage);
		MRedColumn.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MRedColumn.indices.size()), 1, 0, 0, 0);

		DSWhiteColumn.bind(commandBuffer, PBlinn, 0, currentImage);
		MWhiteColumn.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MWhiteColumn.indices.size()), 1, 0, 0, 0);

		DSCoinSack.bind(commandBuffer, PBlinn, 0, currentImage);
		MCoinSack.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MCoinSack.indices.size()), 1, 0, 0, 0);

		DSCoinStack.bind(commandBuffer, PBlinn, 0, currentImage);
		MCoinStack.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MCoinStack.indices.size()), 1, 0, 0, 0);


	}

	glm::vec3 rocketPosition = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 camPos = rocketPosition + glm::vec3(6, 3, 10) / 2.0f;
	glm::mat4 View = glm::lookAt(camPos, rocketPosition, glm::vec3(0, 1, 0));

	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	const float ROT_SPEED = 50.0f;
	const float MOVE_SPEED = 2.5f;
	glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
	glm::mat4 Scale = glm::scale(glm::mat4(1.0), glm::vec3(1, 1, 1));
	glm::mat4 Rotate = glm::rotate(glm::mat4(1.0), 0.0f, glm::vec3(0, 0, 1));

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) override {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		// Integration with the timers and controllers
		static float CamPitch = glm::radians(20.0f);
		static float CamYaw = M_PI;
		static float CamDist = 10.0f;
		static float CamRoll = 0.0f;
		const glm::vec3 CamTargetDelta = glm::vec3(0, 2, 0);
		const glm::vec3 Cam1stPos = glm::vec3(0, 0, 10);

		float deltaT = 0.016f;
		static float cTime = 0.0;
		const float turnTime = 36.0f;
		const float angTurnTimeFact = 2.0f * M_PI / turnTime;
		cTime = cTime + deltaT;
		cTime = (cTime > turnTime) ? (cTime - turnTime) : cTime;

		static glm::vec3 dampedCamPos = CamPos;

		// Parameters for the projection
		// Camera FOV-y, Near Plane and Far Plane
		const float FOVy = glm::radians(90.0f);
		const float nearPlane = 0.1f;
		const float farPlane = 100.0f;

		glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;

		glm::mat4 World;
		glm::mat4 ViewPrj = Prj * View;

		// update global uniforms
		GlobalUniformBufferObject gubo{};
		gubo.lightDir =
		    glm::vec3(cos(glm::radians(135.0f)) * cos(cTime * angTurnTimeFact),
		              sin(glm::radians(135.0f)),
		              cos(glm::radians(135.0f)) * sin(cTime * angTurnTimeFact));
		gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.eyePos = CamPos;

		// north wall
		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(3.0f, 1.0f, 1.0f));
		WallNUbo.mvpMat = ViewPrj * World;
		DSWallN.map(currentImage, &WallNUbo, sizeof(WallNUbo), 0);
		DSWallN.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// east wall
		World = glm::translate(glm::mat4(1), glm::vec3(6.0f, 0.0f, 4.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(90.0f),
		                     glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(2.0f, 1.0f, 1.0f));
		WallEUbo.mvpMat = ViewPrj * World;
		DSWallE.map(currentImage, &WallEUbo, sizeof(WallEUbo), 0);
		DSWallE.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// south wall
		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 8.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(3.0f, 1.0f, 1.0f));
		WallSUbo.mvpMat = ViewPrj * World;
		DSWallS.map(currentImage, &WallSUbo, sizeof(WallSUbo), 0);
		DSWallS.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// west wall
		World = glm::translate(glm::mat4(1), glm::vec3(-6.0f, 0.0f, 4.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-90.0f),
		                     glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(2.0f, 1.0f, 1.0f));
		WallWUbo.mvpMat = ViewPrj * World;
		DSWallW.map(currentImage, &WallWUbo, sizeof(WallWUbo), 0);
		DSWallW.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// windows
		World = glm::translate(glm::mat4(1), glm::vec3(5.8f, 2.5f, 2.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-90.0f),
		                     glm::vec3(0.0f, 1.0f, 0.0f));
		Window1Ubo.mvpMat = ViewPrj * World;
		DSWindow1.map(currentImage, &Window1Ubo, sizeof(Window1Ubo), 0);
		DSWindow1.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		World = glm::translate(glm::mat4(1), glm::vec3(5.8f, 2.5f, 6.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-90.0f),
		                     glm::vec3(0.0f, 1.0f, 0.0f));
		Window2Ubo.mvpMat = ViewPrj * World;
		DSWindow2.map(currentImage, &Window2Ubo, sizeof(Window2Ubo), 0);
		DSWindow2.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// floor
		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 4.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(3.0f, 1.0f, 2.0f));
		FloorUbo.mvpMat = ViewPrj * World;
		DSFloor.map(currentImage, &FloorUbo, sizeof(FloorUbo), 0);
		DSFloor.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// bed
		World = glm::translate(glm::mat4(1), glm::vec3(-4.0f, 0.0f, 1.0f));
		BedUbo.mvpMat = ViewPrj * World;
		DSBed.map(currentImage, &BedUbo, sizeof(BedUbo), 0);
		DSBed.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// closet
		World = glm::translate(glm::mat4(1), glm::vec3(-5.75f, 0.0f, 4.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(90.0f),
		                     glm::vec3(0.0f, 1.0f, 0.0f));
		ClosetUbo.mvpMat = ViewPrj * World;
		DSCloset.map(currentImage, &ClosetUbo, sizeof(ClosetUbo), 0);
		DSCloset.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// desk
		World = glm::translate(glm::mat4(1), glm::vec3(5.5f, 0.0f, 2.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-90.0f),
		                     glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(2.0f, 2.0f, 2.0f));
		DeskUbo.mvpMat = ViewPrj * World;
		DSDesk.map(currentImage, &DeskUbo, sizeof(DeskUbo), 0);
		DSDesk.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// gaming desk
		World = glm::translate(glm::mat4(1), glm::vec3(5.3f, 0.0f, 6.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-90.0f),
		                     glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(1.0f, 1.0f, 1.0f));
		GamingDeskUbo.mvpMat = ViewPrj * World;
		DSGamingDesk.map(currentImage, &GamingDeskUbo, sizeof(GamingDeskUbo), 0);
		DSGamingDesk.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// door
		World = glm::translate(glm::mat4(1), glm::vec3(-0.5f, 0.0f, 7.9f));
		World *= glm::scale(glm::mat4(1), glm::vec3(1.0f, 1.0f, 1.0f));
		DoorUbo.mvpMat = ViewPrj * World;
		DSDoor.map(currentImage, &DoorUbo, sizeof(DoorUbo),0);
		DSDoor.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// red column
		World = glm::translate(glm::mat4(1), glm::vec3(-3.5f, 2.0f, 7.5f));
		RedColumnUbo.mvpMat = ViewPrj * World;
		DSRedColumn.map(currentImage, &RedColumnUbo, sizeof(RedColumnUbo),0);
		DSRedColumn.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// white column
		World = glm::translate(glm::mat4(1), glm::vec3(-3.9f, 2.0f, 3.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(90.0f),glm::vec3(0.0f, 1.0f, 0.0f));
		WhiteColumnUbo.mvpMat = ViewPrj * World;
		DSWhiteColumn.map(currentImage, &WhiteColumnUbo, sizeof(WhiteColumnUbo),0);
		DSWhiteColumn.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// coin sack
		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 1.0f, 2.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(0.003f, 0.003f, 0.003f));
		CoinSackUbo.mvpMat = ViewPrj * World;
		DSCoinSack.map(currentImage, &CoinSackUbo, sizeof(CoinSackUbo),0);
		DSCoinSack.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// coin stack
		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 1.0f, 2.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(0.007f, 0.007f, 0.007f));
		CoinStackUbo.mvpMat = ViewPrj * World;
		DSCoinStack.map(currentImage, &CoinStackUbo, sizeof(CoinStackUbo),0);
		DSCoinStack.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			rocketPosition.z -= MOVE_SPEED * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			rocketPosition.z += MOVE_SPEED * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			rocketPosition.x -= MOVE_SPEED * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			rocketPosition.x += MOVE_SPEED * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			rocketPosition.y += MOVE_SPEED * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			rocketPosition.y -= MOVE_SPEED * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			rotation.x += ROT_SPEED * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			rotation.x -= ROT_SPEED * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			rotation.y += ROT_SPEED * deltaT;
		}
		if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			rotation.y -= ROT_SPEED * deltaT;
		}

		if(rotation.y > 89.0f) rotation.y = 89.0f;
		if(rotation.y < -89.0f) rotation.y = -89.0f;

		// if(rotation.x > 89.0f) rotation.x = 89.0f;
		// if(rotation.x < -89.0f) rotation.x = -89.0f;

		float radius = 3.0f;
		float camx = sin(glm::radians(rotation.x)) * radius;
		float camz = cos(glm::radians(rotation.x)) * radius;
		float camy = sin(glm::radians(rotation.y)) * radius;  // +3?

		View = glm::lookAt(glm::vec3(camx, camy, camz) + rocketPosition,
		                   rocketPosition, glm::vec3(0, 1, 0));
		World = glm::translate(glm::mat4(1.0f), rocketPosition);
		World *= glm::scale(glm::mat4(1), glm::vec3(1.0f, 1.0f, 1.0f));

		RocketUbo.mvpMat = ViewPrj * World;
		DSRocket.map(currentImage, &RocketUbo, sizeof(RocketUbo), 0);
		DSRocket.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);
	}
};


// This is the main: probably you do not need to touch this!
int main() {
	ConfigManager app;

	try {
		app.run();
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}