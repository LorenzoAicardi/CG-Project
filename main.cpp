// This has been adapted from the Vulkan tutorial

#include "modules/Starter.hpp"
#include <glm/gtx/string_cast.hpp>

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
	alignas(16) glm::vec3 lightPos;
	alignas(16) glm::vec3 eyePos;
	alignas(16) glm::vec4 eyeDir;
};

// The vertices data structures
struct Vertex {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

struct SphereCollider {
	glm::vec3 center;
	float radius;
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
	Model<Vertex> MRoof;
	Model<Vertex> MBed;
	Model<Vertex> MCloset;
	Model<Vertex> MDesk;
	Model<Vertex> MGamingDesk;
	Model<Vertex> MRedColumn;
	Model<Vertex> MClock;
	Model<Vertex> MCoinSack;
	Model<Vertex> MCoinStack;
	Model<Vertex> MDoor;
	Model<Vertex> MGamingPouf;
	Model<Vertex> MLoungeChair;
	Model<Vertex> MRecordTable;
	Model<Vertex> MRoofLamp;
	// Model<Vertex> MCoinTata;

	// Descriptor sets
	DescriptorSet DSRocket;
	DescriptorSet DSWallN;
	DescriptorSet DSWallE;
	DescriptorSet DSWallS;
	DescriptorSet DSWallW;
	DescriptorSet DSWindow1;
	DescriptorSet DSWindow2;
	DescriptorSet DSFloor;
	DescriptorSet DSRoof;
	DescriptorSet DSBed;
	DescriptorSet DSCloset;
	DescriptorSet DSDesk;
	DescriptorSet DSGamingDesk;
	DescriptorSet DSDoor;
	DescriptorSet DSRedColumn;
	DescriptorSet DSClock;
	DescriptorSet DSCoinSack;
	DescriptorSet DSCoinStack;
	DescriptorSet DSGamingPouf;
	DescriptorSet DSLoungeChair;
	DescriptorSet DSRecordTable;
	DescriptorSet DSRoofLamp;
	// DescriptorSet DSCoinTata;

	// Textures
	Texture TFurniture;
	Texture TSack;
	Texture TStack;

	// C++ storage for uniform variables
	UniformBufferObject RocketUbo;
	UniformBufferObject WallNUbo;
	UniformBufferObject WallEUbo;
	UniformBufferObject WallSUbo;
	UniformBufferObject WallWUbo;
	UniformBufferObject Window1Ubo;
	UniformBufferObject Window2Ubo;
	UniformBufferObject FloorUbo;
	UniformBufferObject RoofUbo;
	UniformBufferObject BedUbo;
	UniformBufferObject ClosetUbo;
	UniformBufferObject DeskUbo;
	UniformBufferObject GamingDeskUbo;
	UniformBufferObject RedColumnUbo;
	UniformBufferObject ClockUbo;
	UniformBufferObject CoinSackUbo;
	UniformBufferObject CoinStackUbo;
	UniformBufferObject DoorUbo;
	UniformBufferObject GamingPoufUbo;
	UniformBufferObject LoungeChairUbo;
	UniformBufferObject RecordTableUbo;
	UniformBufferObject RoofLampUbo;
	// UniformBufferObject CoinTataUbo;

	// Here you set the main application parameters
	void setWindowParameters() override {
		// window size, titile and initial background
		windowWidth = 1920;
		windowHeight = 1080;
		windowTitle = "Project";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.5f, 0.5f, 0.6f, 1.0f};

		// Descriptor pool sizes
		uniformBlocksInPool = 70;
		texturesInPool = 70;
		setsInPool = 70;

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
		MRocket.init(this, &VD, "models/rotrocketypositive.obj", OBJ, bbList);
		MWallN.init(this, &VD, "models/blue_wall.mgcg", MGCG, bbList);
		MWallE.init(this, &VD, "models/blue_wall.mgcg", MGCG, bbList);
		MWallS.init(this, &VD, "models/blue_wall.mgcg", MGCG, bbList);
		MWallW.init(this, &VD, "models/blue_wall.mgcg", MGCG, bbList);
		MWindow1.init(this, &VD, "models/window.mgcg", MGCG, bbList);
		MWindow2.init(this, &VD, "models/window.mgcg", MGCG, bbList);
		MFloor.init(this, &VD, "models/parquet.mgcg", MGCG, bbList);
		MRoof.init(this, &VD, "models/blue_wall.mgcg", MGCG, bbList);
		MBed.init(this, &VD, "models/tower_bed.mgcg", MGCG, bbList);
		MGamingDesk.init(this, &VD, "models/gaming_desk.mgcg", MGCG, bbList);
		MCloset.init(this, &VD, "models/big_closet.mgcg", MGCG, bbList);
		MDoor.init(this, &VD, "models/door.mgcg", MGCG, bbList);
		MDesk.init(this, &VD, "models/study_desk.mgcg", MGCG, bbList);
		MRedColumn.init(this, &VD, "models/red_column.mgcg", MGCG, bbList);
		MClock.init(this, &VD, "models/clock.mgcg", MGCG, bbList);
		MCoinSack.init(this, &VD, "models/coin_sack.mgcg", MGCG, bbList);
		MCoinStack.init(this, &VD, "models/coin_stack.mgcg", MGCG, bbList);
		MGamingPouf.init(this, &VD, "models/gaming_pouf.mgcg", MGCG, bbList);
		MLoungeChair.init(this, &VD, "models/lounge_chair.mgcg", MGCG, bbList);
		MRecordTable.init(this, &VD, "models/record_table.mgcg", MGCG, bbList);
		MRoofLamp.init(this, &VD, "models/roof_lamp.mgcg", MGCG, bbList);
		// MCoinTata.init(this, &VD, "models/Coin.obj", OBJ);
		for(int i = 0; i < bbList.size(); i++)
			std::cout << "BOUNDING BOX " << i << ": " << bbList[i].min[0] << ","
					  << bbList[i].min[1] << "," << bbList[i].min[2] << ","
					  << bbList[i].max[0] << "," << bbList[i].max[1] << ","
					  << bbList[i].max[2] << " OLLEEEEEEEEEEEEEEEEE \n";
		// Create the textures
		// The second parameter is the file name
		TFurniture.init(this, "textures/Textures_Forniture.png");
		TSack.init(this, "textures/MoneySack_Albedo.png");
		TStack.init(this, "textures/CoinStack_Albedo.png");

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
		DSRoof.init(this, &DSL,
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
		DSClock.init(this, &DSL,
					 {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					  {1, TEXTURE, 0, &TFurniture},
					  {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSCoinSack.init(this, &DSL,
						{{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						 {1, TEXTURE, 0, &TSack},
						 {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSCoinStack.init(this, &DSL,
						 {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						  {1, TEXTURE, 0, &TStack},
						  {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSGamingPouf.init(this, &DSL,
						  {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						   {1, TEXTURE, 0, &TFurniture},
						   {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSLoungeChair.init(this, &DSL,
						   {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
							{1, TEXTURE, 0, &TFurniture},
							{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSRecordTable.init(this, &DSL,
						   {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
							{1, TEXTURE, 0, &TFurniture},
							{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		DSRoofLamp.init(this, &DSL,
						{{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						 {1, TEXTURE, 0, &TFurniture},
						 {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
		/*
		DSCoinTata.init(this, &DSL,
						{{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						 {1, TEXTURE, 0, &TStack},
						 {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}});
						 */
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
		DSRoof.cleanup();
		DSBed.cleanup();
		DSGamingDesk.cleanup();
		DSCloset.cleanup();
		DSDoor.cleanup();
		DSDesk.cleanup();
		DSRedColumn.cleanup();
		DSClock.cleanup();
		DSCoinSack.cleanup();
		DSCoinStack.cleanup();
		DSGamingPouf.cleanup();
		DSLoungeChair.cleanup();
		DSRecordTable.cleanup();
		DSRoofLamp.cleanup();
		// DSCoinTata.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() override {
		// Cleanup textures
		TFurniture.cleanup();
		TSack.cleanup();
		TStack.cleanup();

		// Cleanup models
		MRocket.cleanup();
		MWallN.cleanup();
		MWallE.cleanup();
		MWallS.cleanup();
		MWallW.cleanup();
		MWindow1.cleanup();
		MWindow2.cleanup();
		MFloor.cleanup();
		MRoof.cleanup();
		MBed.cleanup();
		MGamingDesk.cleanup();
		MCloset.cleanup();
		MDoor.cleanup();
		MDesk.cleanup();
		MRedColumn.cleanup();
		MClock.cleanup();
		MCoinSack.cleanup();
		MCoinStack.cleanup();
		MGamingPouf.cleanup();
		MLoungeChair.cleanup();
		MRecordTable.cleanup();
		MRoofLamp.cleanup();
		// MCoinTata.cleanup();

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

		DSRoof.bind(commandBuffer, PBlinn, 0, currentImage);
		MRoof.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MRoof.indices.size()), 1, 0, 0, 0);

		DSBed.bind(commandBuffer, PBlinn, 0, currentImage);
		MBed.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MBed.indices.size()), 1, 0, 0, 0);

		DSGamingDesk.bind(commandBuffer, PBlinn, 0, currentImage);
		MGamingDesk.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MGamingDesk.indices.size()), 1,
						 0, 0, 0);

		DSCloset.bind(commandBuffer, PBlinn, 0, currentImage);
		MCloset.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MCloset.indices.size()), 1, 0, 0, 0);

		DSDoor.bind(commandBuffer, PBlinn, 0, currentImage);
		MDoor.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MDoor.indices.size()), 1, 0, 0, 0);

		DSDesk.bind(commandBuffer, PBlinn, 0, currentImage);
		MDesk.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MDesk.indices.size()), 1, 0, 0, 0);

		DSRedColumn.bind(commandBuffer, PBlinn, 0, currentImage);
		MRedColumn.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MRedColumn.indices.size()), 1, 0,
						 0, 0);

		DSClock.bind(commandBuffer, PBlinn, 0, currentImage);
		MClock.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MClock.indices.size()), 1, 0, 0, 0);

		DSCoinSack.bind(commandBuffer, PBlinn, 0, currentImage);
		MCoinSack.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MCoinSack.indices.size()), 1, 0, 0, 0);

		DSCoinStack.bind(commandBuffer, PBlinn, 0, currentImage);
		MCoinStack.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MCoinStack.indices.size()), 1, 0,
						 0, 0);

		DSGamingPouf.bind(commandBuffer, PBlinn, 0, currentImage);
		MGamingPouf.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MGamingPouf.indices.size()), 1,
						 0, 0, 0);

		DSLoungeChair.bind(commandBuffer, PBlinn, 0, currentImage);
		MLoungeChair.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MLoungeChair.indices.size()), 1,
						 0, 0, 0);

		DSRecordTable.bind(commandBuffer, PBlinn, 0, currentImage);
		MRecordTable.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MRecordTable.indices.size()), 1,
						 0, 0, 0);
		DSRoofLamp.bind(commandBuffer, PBlinn, 0, currentImage);
		MRoofLamp.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MRoofLamp.indices.size()), 1, 0, 0, 0);
		/*
		DSCoinTata.bind(commandBuffer, PBlinn, 0, currentImage);
		MCoinTata.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MCoinTata.indices.size()), 1,
						 0, 0, 0);
		*/
	}

	glm::vec3 rocketPosition = glm::vec3(-1.0f, 2.0f, 4.0f) + glm::vec3(10.0f);
	glm::vec3 rocketDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 camPos = rocketPosition + glm::vec3(6, 3, 10) / 2.0f;
	glm::mat4 View = glm::lookAt(camPos, rocketPosition, glm::vec3(0, 1, 0));

	glm::vec3 rocketRotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rocketCameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	const float ROT_SPEED = 50.0f;
	const float MOVE_SPEED = 0.5f;
	glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
	glm::mat4 Scale = glm::scale(glm::mat4(1.0), glm::vec3(1, 1, 1));
	glm::mat4 Rotate = glm::rotate(glm::mat4(1.0), 0.0f, glm::vec3(0, 0, 1));
	float verticalSpeed = 0.0f;
	glm::vec3 rocketSpeed = glm::vec3(0.0f, 0.0f, 0.0f);
	float GRAVITY_CONSTANT = 2.0f;

	bool isPlaced[22] = {0};
	glm::vec3 ogRocketMin;
	glm::vec3 ogRocketMax;

	SphereCollider rocketCollider;

	// Helper function for checking collisions
	bool checkCollision(const SphereCollider& sphere, const BoundingBox& box) {
		float x = glm::max(box.min.x, glm::min(sphere.center.x, box.max.x));
		float y = glm::max(box.min.y, glm::min(sphere.center.y, box.max.y));
		float z = glm::max(box.min.z, glm::min(sphere.center.z, box.max.z));

		// this is the same as isPointInsideSphere
		float distance = glm::sqrt((x - sphere.center.x) * (x - sphere.center.x) +
								   (y - sphere.center.y) * (y - sphere.center.y) +
								   (z - sphere.center.z) * (z - sphere.center.z));

		return distance < sphere.radius;

		/*
		return (
				box1.min.x <= box2.max.x &&
				box1.max.x >= box2.min.x &&
				box1.min.y <= box2.max.y &&
				box1.max.y >= box2.min.y &&
				box1.min.z <= box2.max.y &&
				box1.max.z >= box2.min.z
		);
		 */
	}

	void placeObject(int index, bool (&placed)[22], glm::mat4& World,
					 std::vector<BoundingBox>& bbList) {
		glm::vec4 homogeneousPoint;
		if(!placed[index]) {
			std::cout << index << " Pre min: " << bbList[index].min[0] << ", "
					  << bbList[index].min[1] << ", " << bbList[index].min[2] << "\n";
			std::cout << index << " Pre max: " << bbList[index].max[0] << ", "
					  << bbList[index].max[1] << ", " << bbList[index].max[2] << "\n";

			homogeneousPoint = glm::vec4(bbList[index].min, 1.0f);
			glm::vec4 newPoint = World * homogeneousPoint;
			bbList[index].min = glm::vec3(newPoint);

			homogeneousPoint = glm::vec4(bbList[index].max, 1.0f);
			newPoint = World * homogeneousPoint;
			bbList[index].max = glm::vec3(newPoint);

			std::cout << index << " Post min: " << bbList[index].min[0] << ", "
					  << bbList[index].min[1] << ", " << bbList[index].min[2] << "\n";
			std::cout << index << " Post max: " << bbList[index].max[0] << ", "
					  << bbList[index].max[1] << ", " << bbList[index].max[2] << "\n";
			placed[index] = true;
		}
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) override {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		// Integration with the timers and controllers
		float deltaT = 0.016f;
		static float cTime = 0.0;
		const float turnTime = 36.0f;
		const float angTurnTimeFact = 2.0f * M_PI / turnTime;
		cTime = cTime + deltaT;
		cTime = (cTime > turnTime) ? (cTime - turnTime) : cTime;

		// Parameters for the projection
		// Camera FOV-y, Near Plane and Far Plane
		const float FOVy = glm::radians(90.0f);
		const float nearPlane = 0.1f;
		const float farPlane = 100.0f;

		glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;

		glm::mat4 World;
		glm::mat4 ViewPrj = Prj * View;

		glm::vec4 homogeneousPoint;
		// Rocket
		World = glm::translate(glm::mat4(1.0f), rocketPosition);
		World *= glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));
		RocketUbo.mvpMat = Prj * View * World;
		if(!isPlaced[0]) {
			placeObject(0, isPlaced, World, bbList);

			ogRocketMin = bbList[0].min;
			ogRocketMax = bbList[0].max;

			rocketCollider.center = rocketPosition;
			rocketCollider.radius = 0.5f;
			isPlaced[0] = true;
		}
		DSRocket.map(currentImage, &RocketUbo, sizeof(RocketUbo), 0);
		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block

		// Walls
		// update global uniforms
		GlobalUniformBufferObject gubo{};
		gubo.lightDir =
			glm::vec3(cos(glm::radians(135.0f)), /** cos(cTime * angTurnTimeFact)*/
					  sin(glm::radians(135.0f)),
					  cos(glm::radians(135.0f))); /** sin(cTime * angTurnTimeFact));*/
		gubo.lightPos = glm::vec3(0.0f, 3.6f, 4.0f);
		gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.eyePos = CamPos;

		// north wall
		World = glm::translate(glm::mat4(1),
							   glm::vec3(0.0f, 0.0f, 0.0f) + glm::vec3(10.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(3.0f, 1.0f, 1.0f));
		WallNUbo.mvpMat = ViewPrj * World;
		placeObject(1, isPlaced, World, bbList);
		DSWallN.map(currentImage, &WallNUbo, sizeof(WallNUbo), 0);
		DSWallN.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// east wall
		World = glm::translate(glm::mat4(1),
							   glm::vec3(6.0f, 0.0f, 4.0f) + glm::vec3(10.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(90.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(-2.0f, 1.0f, -1.0f));
		WallEUbo.mvpMat = ViewPrj * World;
		placeObject(2, isPlaced, World, bbList);
		DSWallE.map(currentImage, &WallEUbo, sizeof(WallEUbo), 0);
		DSWallE.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// south wall
		World = glm::translate(glm::mat4(1),
							   glm::vec3(0.0f, 0.0f, 8.0f) + glm::vec3(10.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(-3.0f, 1.0f, -1.0f));
		WallSUbo.mvpMat = ViewPrj * World;
		placeObject(3, isPlaced, World, bbList);
		DSWallS.map(currentImage, &WallSUbo, sizeof(WallSUbo), 0);
		DSWallS.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// west wall
		World = glm::translate(glm::mat4(1),
							   glm::vec3(-6.0f, 0.0f, 4.0f) + glm::vec3(10.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-90.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(-2.0f, 1.0f, -1.0f));
		WallWUbo.mvpMat = ViewPrj * World;
		placeObject(4, isPlaced, World, bbList);
		DSWallW.map(currentImage, &WallWUbo, sizeof(WallWUbo), 0);
		DSWallW.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// windows
		World = glm::translate(glm::mat4(1),
							   glm::vec3(5.8f, 2.5f, 2.0f) + glm::vec3(10.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-90.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		Window1Ubo.mvpMat = ViewPrj * World;
		placeObject(5, isPlaced, World, bbList);
		DSWindow1.map(currentImage, &Window1Ubo, sizeof(Window1Ubo), 0);
		DSWindow1.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		World = glm::translate(glm::mat4(1),
							   glm::vec3(5.8f, 2.5f, 6.0f) + glm::vec3(10.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-90.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		Window2Ubo.mvpMat = ViewPrj * World;
		placeObject(6, isPlaced, World, bbList);
		DSWindow2.map(currentImage, &Window2Ubo, sizeof(Window2Ubo), 0);
		DSWindow2.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// floor
		World = glm::translate(glm::mat4(1),
							   glm::vec3(0.0f, 0.0f, 4.0f) + glm::vec3(10.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(3.0f, 1.0f, 2.0f));
		FloorUbo.mvpMat = ViewPrj * World;
		placeObject(7, isPlaced, World, bbList);
		DSFloor.map(currentImage, &FloorUbo, sizeof(FloorUbo), 0);
		DSFloor.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// roof
		World = glm::translate(glm::mat4(1),
							   glm::vec3(0.0f, 4.0f, 0.0f) + glm::vec3(10.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(90.0f),
							 glm::vec3(1.0f, 0.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(3.0f, 2.0f, 2.0f));
		RoofUbo.mvpMat = ViewPrj * World;
		placeObject(8, isPlaced, World, bbList);
		DSRoof.map(currentImage, &RoofUbo, sizeof(RoofUbo), 0);
		DSRoof.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// bed
		World = glm::translate(glm::mat4(1),
							   glm::vec3(-4.0f, 0.0f, 1.0f) + glm::vec3(10.0f));
		BedUbo.mvpMat = ViewPrj * World;
		placeObject(9, isPlaced, World, bbList);
		DSBed.map(currentImage, &BedUbo, sizeof(BedUbo), 0);
		DSBed.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// closet
		World = glm::translate(glm::mat4(1),
							   glm::vec3(-1.0f, 0.0f, 0.4f) + glm::vec3(10.0f));
		ClosetUbo.mvpMat = ViewPrj * World;
		placeObject(10, isPlaced, World, bbList);
		DSCloset.map(currentImage, &ClosetUbo, sizeof(ClosetUbo), 0);
		DSCloset.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// desk
		World = glm::translate(glm::mat4(1),
							   glm::vec3(5.5f, 0.0f, 2.0f) + glm::vec3(10.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-90.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(2.0f, 2.0f, 2.0f));
		DeskUbo.mvpMat = ViewPrj * World;
		placeObject(11, isPlaced, World, bbList);
		DSDesk.map(currentImage, &DeskUbo, sizeof(DeskUbo), 0);
		DSDesk.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// gaming desk
		World = glm::translate(glm::mat4(1),
							   glm::vec3(3.0f, 0.0f, 0.7f) + glm::vec3(10.0f));
		GamingDeskUbo.mvpMat = ViewPrj * World;
		placeObject(12, isPlaced, World, bbList);
		DSGamingDesk.map(currentImage, &GamingDeskUbo, sizeof(GamingDeskUbo), 0);
		DSGamingDesk.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// gaming pouf
		World = glm::translate(glm::mat4(1),
							   glm::vec3(3.0f, 0.0f, 1.0f) + glm::vec3(10.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-180.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		GamingPoufUbo.mvpMat = ViewPrj * World;
		placeObject(13, isPlaced, World, bbList);
		DSGamingPouf.map(currentImage, &GamingPoufUbo, sizeof(GamingPoufUbo), 0);
		DSGamingPouf.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// door
		World = glm::translate(glm::mat4(1),
							   glm::vec3(-0.5f, 0.0f, 7.9f) + glm::vec3(10.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(-1.0f, 1.0f, -1.0f));
		DoorUbo.mvpMat = ViewPrj * World;
		placeObject(14, isPlaced, World, bbList);
		DSDoor.map(currentImage, &DoorUbo, sizeof(DoorUbo), 0);
		DSDoor.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// red column
		World = glm::translate(glm::mat4(1),
							   glm::vec3(3.5f, 2.0f, 6.0f) + glm::vec3(10.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(-1.0f, 1.0f, -1.0f));
		RedColumnUbo.mvpMat = ViewPrj * World;
		placeObject(15, isPlaced, World, bbList);
		DSRedColumn.map(currentImage, &RedColumnUbo, sizeof(RedColumnUbo), 0);
		DSRedColumn.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// clock
		World = glm::translate(glm::mat4(1),
							   glm::vec3(-5.9f, 2.0f, 3.0f) + glm::vec3(10.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(90.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		ClockUbo.mvpMat = ViewPrj * World;
		placeObject(16, isPlaced, World, bbList);
		DSClock.map(currentImage, &ClockUbo, sizeof(ClockUbo), 0);
		DSClock.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// lounge chair
		World = glm::translate(glm::mat4(1),
							   glm::vec3(-5.0f, 0.0f, 7.0f) + glm::vec3(10.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(135.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		LoungeChairUbo.mvpMat = ViewPrj * World;
		placeObject(17, isPlaced, World, bbList);
		DSLoungeChair.map(currentImage, &LoungeChairUbo, sizeof(LoungeChairUbo), 0);
		DSLoungeChair.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// record & TV table
		World = glm::translate(glm::mat4(1),
							   glm::vec3(-3.0f, 0.0f, 7.5f) + glm::vec3(10.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(180.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(1.5f, 1.5f, 1.5f));
		RecordTableUbo.mvpMat = ViewPrj * World;
		placeObject(18, isPlaced, World, bbList);
		DSRecordTable.map(currentImage, &RecordTableUbo, sizeof(RecordTableUbo), 0);
		DSRecordTable.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// roof lamp
		World = glm::translate(glm::mat4(1),
							   glm::vec3(0.0f, 3.8f, 4.0f) + glm::vec3(10.0f));
		RoofLampUbo.mvpMat = ViewPrj * World;
		placeObject(19, isPlaced, World, bbList);
		DSRoofLamp.map(currentImage, &RoofLampUbo, sizeof(RoofLampUbo), 0);
		DSRoofLamp.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// coin sack
		World = glm::translate(glm::mat4(1),
							   glm::vec3(0.0f, 1.0f, 4.0f) + glm::vec3(10.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(0.003f, 0.003f, 0.003f));
		CoinSackUbo.mvpMat = ViewPrj * World;
		placeObject(20, isPlaced, World, bbList);
		DSCoinSack.map(currentImage, &CoinSackUbo, sizeof(CoinSackUbo), 0);
		DSCoinSack.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		// coin stack
		World = glm::translate(glm::mat4(1),
							   glm::vec3(0.0f, 1.0f, 2.0f) + glm::vec3(10.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(0.007f, 0.007f, 0.007f));
		CoinStackUbo.mvpMat = ViewPrj * World;
		placeObject(21, isPlaced, World, bbList);
		DSCoinStack.map(currentImage, &CoinStackUbo, sizeof(CoinStackUbo), 0);
		DSCoinStack.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);

		/*
		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 1.0f, 6.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(0.01f, 0.01f, 0.01f));
		CoinTataUbo.mvpMat = ViewPrj * World;
		DSCoinTata.map(currentImage, &CoinTataUbo, sizeof(CoinTataUbo), 0);
		DSCoinTata.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);
		*/

		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			rocketRotation.x -= 1.0f;
		}
		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			rocketRotation.x += 1.0f;
		}
		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			rocketRotation.y += 1.0f;
		}
		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			rocketRotation.y -= 1.0f;
		}
		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {	// Space only moves me forward
			rocketDirection.z -= 1.0f;

			glm::mat4 rocketRotationMatrix =
				glm::rotate(glm::mat4(1.0f), glm::radians(rocketRotation.y),
							glm::vec3(0.0f, 1.0f, 0.0f));
			rocketRotationMatrix =
				glm::rotate(rocketRotationMatrix, glm::radians(rocketRotation.x),
							glm::vec3(1.0f, 0.0f, 0.0f));
			glm::vec3 newRocketDirection =
				glm::vec3(rocketRotationMatrix * glm::vec4(rocketDirection, 0.0f));

			rocketSpeed += newRocketDirection * MOVE_SPEED * deltaT;
			// Cap maximum speed

			if(glm::length(rocketSpeed) > 1.0f)
				rocketSpeed = glm::normalize(rocketSpeed) * 1.0f;
			// Acceleration towards maximum speed

			// Reset direction to avoid permanently going in the same direction
			rocketDirection = {0, 0, 0};
			// "Cancel" gravity while accelerating
			verticalSpeed = 0.0f;
		} else {
			// Gravity (gravity constant can be lowered)
			verticalSpeed += GRAVITY_CONSTANT * deltaT;
			rocketPosition.y -= verticalSpeed * deltaT;
			// Ground
			if(rocketPosition.y < 10.0f) rocketPosition.y = 10.0f;

			// Deceleration towards minimum speed (0)
			if(glm::length(rocketSpeed) > 0.0f) {
				float speed = glm::length(rocketSpeed);
				speed -= MOVE_SPEED * deltaT;
				speed = glm::max(speed, 0.0f);
				rocketSpeed = glm::normalize(rocketSpeed) * speed;
			}
			rocketDirection = {0, 0, 0};
		}

		// Camera controls
		if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			rocketCameraRotation.y -= 1.0f;
		}
		if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			rocketCameraRotation.y += 1.0f;
		}
		if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			rocketCameraRotation.x -= 1.0f;
		}
		if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			rocketCameraRotation.x += 1.0f;
		}

		if(rocketCameraRotation.y > 89.0f) rocketCameraRotation.y = 89.0f;
		if(rocketCameraRotation.y < -89.0f) rocketCameraRotation.y = -89.0f;
		if(rocketCameraRotation.x < -89.0f) rocketCameraRotation.x = -89.0f;
		if(rocketCameraRotation.x > 89.0f) rocketCameraRotation.x = 89.0f;

		// Need to check collisions first
		bool isCollision = false;
		for(int i = 1; i < bbList.size(); i++) {
			if(rocketPosition.x > 6.8 && rocketPosition.x < 7.2) {
				/*
				std::cout << "Rocket bounding box: " << bbList[0].min[0] << " " << bbList[0].min[1] << " " << bbList[0].min[2] << " " << bbList[0].max[0]
						<< " " << bbList[0].max[1] << " " << bbList[0].max[2] << "\n";
				std::cout << "East wall bounding box: " << bbList[i].min[0] << " " << bbList[i].min[1] << " " << bbList[i].min[2] << " " << bbList[i].max[0]
						<< " " << bbList[i].max[1] << " " << bbList[i].max[2] << "\n";

				std::cout << (bbList[0].min.x <= bbList[i].max.x) <<
						(bbList[0].max.x >= bbList[i].min.x) <<
						(bbList[0].min.y <= bbList[i].max.y) <<
						(bbList[0].max.y >= bbList[i].min.y) <<
						(bbList[0].min.z <= bbList[i].max.y) <<
						(bbList[0].max.z >= bbList[i].min.z) << "\n";
				*/

				std::cout << "Rocket bounding box: "
						  << "\n\tcenter: " << glm::to_string(rocketCollider.center)
						  << "\n\tradius: " << rocketCollider.radius << "\n";
				std::cout << "West wall bounding box: " << bbList[i].min[0]
						  << " " << bbList[i].min[1] << " " << bbList[i].min[2]
						  << " " << bbList[i].max[0] << " " << bbList[i].max[1]
						  << " " << bbList[i].max[2] << "\n";

				float x =
					glm::max(bbList[i].min.x,
							 glm::min(rocketCollider.center.x, bbList[i].max.x));
				float y =
					glm::max(bbList[i].min.y,
							 glm::min(rocketCollider.center.y, bbList[i].max.y));
				float z =
					glm::max(bbList[i].min.z,
							 glm::min(rocketCollider.center.z, bbList[i].max.z));

				std::cout << "closest point: " << x << ", " << y << ", " << z
						  << std::endl;
				std::cout << "distance = "
						  << glm::sqrt((x - rocketCollider.center.x) *
										   (x - rocketCollider.center.x) +
									   (y - rocketCollider.center.y) *
										   (y - rocketCollider.center.y) +
									   (z - rocketCollider.center.z) *
										   (z - rocketCollider.center.z))
						  << std::endl;
			}
			if(checkCollision(rocketCollider, bbList[i])) {
				isCollision = true;
				std::cout << i << "\n";
				break;
			}
		}

		// Update the rocket's position based on collision or not
		if(!isCollision)
			rocketPosition += rocketSpeed * deltaT;
		else
			rocketPosition = glm::vec3(-1.0f, 2.0f, 4.0f) + glm::vec3(10.0f);

		// Update rocket world matrix
		World = glm::translate(glm::mat4(1.0f), rocketPosition);
		World *= glm::rotate(glm::mat4(1.0f), glm::radians(rocketRotation.y),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::rotate(glm::mat4(1.0f), glm::radians(rocketRotation.x),
							 glm::vec3(1.0f, 0.0f, 0.0f));
		World *= glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));

		// Update view matrix
		float radius = 0.5f;
		float camx =
			sin(glm::radians(rocketRotation.y + rocketCameraRotation.y)) * radius;
		float camz =
			cos(glm::radians(rocketRotation.y + rocketCameraRotation.y)) * radius;
		float camy =
			-sin(glm::radians(rocketRotation.x + rocketCameraRotation.x)) * radius;
		View = glm::lookAt(glm::vec3(camx, camy, camz) + rocketPosition,
						   rocketPosition, glm::vec3(0, 1, 0));

		// Update mvpMat and map the rocket
		RocketUbo.mvpMat = Prj * View * World;

		/*
		homogeneousPoint = glm::vec4(ogRocketMin, 1.0f);
		homogeneousPoint = World * homogeneousPoint;
		bbList[0].min = glm::vec3(homogeneousPoint);
		homogeneousPoint = glm::vec4(ogRocketMax, 1.0f);
		homogeneousPoint = World * homogeneousPoint;
		bbList[0].max = glm::vec3(homogeneousPoint);
		*/
		rocketCollider.center = rocketPosition;

		DSRocket.map(currentImage, &RocketUbo, sizeof(RocketUbo), 0);
		DSRocket.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);
		rocketDirection = glm::vec3(0.0f, 0.0f, 0.0f);
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