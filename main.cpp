// This has been adapted from the Vulkan tutorial

#include "modules/SceneManager.hpp"

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

	SceneManager<Vertex> SC;

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

	// Descriptor sets
	DescriptorSet DSRocket;

	// Textures
	Texture TFurniture;
	Texture TSack;
	Texture TStack;

	// C++ storage for uniform variables
	UniformBufferObject RocketUbo;

	// Here you set the main application parameters
	void setWindowParameters() override {
		// window size, titile and initial background
		windowWidth = 1920;
		windowHeight = 1080;
		windowTitle = "Project";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.5f, 0.5f, 0.6f, 1.0f};

		// Descriptor pool sizes
		SC.countResources("models/scene.json");
		uniformBlocksInPool = SC.resCtr.uboInPool;
		texturesInPool = SC.resCtr.textureInPool;
		setsInPool = SC.resCtr.dsInPool;

		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) override { Ar = (float)w / (float)h; }

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() override {
		// init descriptor layouts [what will be passed to the shaders]
		DSL.init(this,
				 {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
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

		// init scene (models & textures)
		SC.init(this, &VD, DSL, PBlinn, "models/scene.json");

		// init local variables
	}

	void pipelinesAndDescriptorSetsInit() override {
		PBlinn.create();

		std::vector<DescriptorSetElement> bindings =
			{{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
			 {1, TEXTURE, 0, SC.T[0]},
			 {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}};

		SC.pipelinesAndDescriptorSetsInit(DSL, bindings);
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() override {
		// cleanup pipelines
		PBlinn.cleanup();

		// cleanup descriptor sets
		SC.pipelinesAndDescriptorSetsCleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() override {
		// cleanup textures & models
		SC.localCleanup();

		// Cleanup descriptor set layouts
		DSL.cleanup();

		// Destroys the pipelines
		PBlinn.destroy();
	}

	// Here is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
		// binds the pipeline
		PBlinn.bind(commandBuffer);
		// for a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter

		// binds the data sets
		SC.populateCommandBuffer(commandBuffer, currentImage, PBlinn);
	}

	glm::vec3 rocketPosition = glm::vec3(0.0f, 1.0f, 4.0f) + glm::vec3(10.0f);
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

		float distance = glm::sqrt((x - sphere.center.x) * (x - sphere.center.x) +
								   (y - sphere.center.y) * (y - sphere.center.y) +
								   (z - sphere.center.z) * (z - sphere.center.z));

		return distance < sphere.radius;
	}


	void placeObject(int index, bool (&placed)[22], glm::mat4& World,
					 std::vector<BoundingBox>& bbList) {
		if(!placed[index]) {
			BoundingBox bbox;
			glm::vec4 homogeneousPoint;

			bbox.min = glm::vec3(std::numeric_limits<float>::max());
			bbox.max = glm::vec3(std::numeric_limits<float>::lowest());
			for(int j = 0; j < vecList[index].size(); j++) {
				glm::vec3 vertex = vecList[index][j];
				homogeneousPoint = glm::vec4(vertex, 1.0f);
				glm::vec4 newVertex = World * homogeneousPoint;
				vertex = glm::vec3(newVertex);

				bbox.min = glm::min(bbox.min, vertex);
				bbox.max = glm::max(bbox.max, vertex);
			}
			bbox.max = glm::round(bbox.max * 100.0f) / 100.0f;
			bbox.min = glm::round(bbox.min * 100.0f) / 100.0f;
			/*
			if(index == 16 || index == 17) {
				bbox.max = glm::vec3(0.0f);
				bbox.min = glm::vec3(0.0f);
			}*/
			bbList.push_back(bbox);

			/*
			std::cout << "Bounding box " << index << " : " <<
				bbList[index].min.x << ", " << bbList[index].min.y << ", " << bbList[index].min.z << ", " <<
				bbList[index].max.x << ", " << bbList[index].max.y << ", " << bbList[index].max.z << "\n";
			*/

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
		// World = glm::translate(glm::mat4(1.0f), rocketPosition);
		// World *= glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));
		// RocketUbo.mvpMat = Prj * View * World;
		// if(!isPlaced[0]) {
		// 	placeObject(0, isPlaced, World, bbList);
		//
		// 	rocketCollider.center = rocketPosition;
		// 	rocketCollider.radius = 0.1f;
		// 	isPlaced[0] = true;
		// }
		// DSRocket.map(currentImage, &RocketUbo, sizeof(RocketUbo), 0);
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

		// place objects on scene
		UniformBufferObject ubo{};
		for(auto instance : SC.InstanceIds) {
			int i = instance.second;
			ubo.mvpMat = ViewPrj * SC.I[i].Wm;
			SC.DS[i]->map(currentImage, &ubo, sizeof(ubo), 0);
			SC.DS[i]->map(currentImage, &gubo, sizeof(gubo), 2);
		}

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
			// Set terminal falling speed
			verticalSpeed = glm::max(verticalSpeed, 2.0f);
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
		int collisionIndex = -1;
		for(int i = 1; i < bbList.size(); i++) {
			if(checkCollision(rocketCollider, bbList[i])) {
				isCollision = true;
				collisionIndex = i;
				std::cout << i << "\n";
				break;
			}
		}

		// Update the rocket's position based on collision or not
		if(!isCollision)
			rocketPosition += rocketSpeed * deltaT;
		else {
			/*
			glm::vec3 closestPoint;
			closestPoint.x = glm::max(rocketCollider.center.x, glm::min(bbList[collisionIndex].max.x, bbList[collisionIndex].min.x));
			closestPoint.y = glm::max(rocketCollider.center.y, glm::min(bbList[collisionIndex].max.y, bbList[collisionIndex].min.y));
			closestPoint.z = glm::max(rocketCollider.center.z, glm::min(bbList[collisionIndex].max.z, bbList[collisionIndex].min.z));

			glm::vec3 difference = closestPoint - rocketCollider.center;
			float distanceSquared = glm::dot(difference, difference);
			float distance = glm::sqrt(distanceSquared);

			if (distance < rocketCollider.radius) {
				glm::vec3 penetrationVector = difference * ((rocketCollider.radius - distance) / distance);
				rocketPosition -= penetrationVector;
			}*/
			rocketPosition = glm::vec3(-1.0f, 2.0f, 4.0f) + glm::vec3(10.0f);
		}

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

		rocketCollider.center = rocketPosition;

		// DSRocket.map(currentImage, &RocketUbo, sizeof(RocketUbo), 0);
		// DSRocket.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);
		rocketDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	}
};

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