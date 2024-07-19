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
	struct {
		alignas(16) glm::vec3 v;
	} lightDir[2];
	struct {
		alignas(16) glm::vec3 v;
	} lightPos[2];
	alignas(16) glm::vec4 lightColor[2];
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

enum RocketState { MOVING, RESTING };


class ConfigManager : public BaseProject {
protected:
	// Current aspect ratio (used by the callback that resized the window)
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSL;

	SceneManager<Vertex> SC;

	// Vertex formats
	VertexDescriptor VD;

	// Pipelines [Shader couples]
	/// Lambert diffuse + Cook-Torrance specular
	Pipeline PCookTorrance;
	/// self-emissive objects (e.g. lamps)
	Pipeline PEmission;
	/// cartoon Shader for the rocket
	Pipeline PCartoon;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex
	// structure Models
	Model<Vertex> MRocket;

	// Descriptor sets
	DescriptorSet DSRocket;

	// Textures
	Texture TFurniture;
	Texture TCoin;
	Texture TStack;
	Texture TRocket;

	// C++ storage for uniform variables
	UniformBufferObject RocketUbo;

	// Here you set the main application parameters
	void setWindowParameters() override {
		// window size, titile and initial background
		windowWidth = 720;
		windowHeight = 480;
		windowTitle = "Project";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.5f, 0.5f, 0.6f, 1.0f};

		// Descriptor pool sizes
		SC.countResources("models/scene.json");
		uniformBlocksInPool = SC.resCtr.uboInPool + 50;
		texturesInPool = SC.resCtr.textureInPool + 50;
		setsInPool = SC.resCtr.dsInPool + 50;

		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) override { Ar = (float)w / (float)h; }

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	glm::vec3 rocketPosition = glm::vec3(-1.0f, 2.0f, 4.0f);
	glm::vec3 rocketDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rocketRotation = glm::vec3(0.0f, 0.0f, 0.0f);
	SphereCollider rocketCollider;
	RocketState rocketState = MOVING;
	glm::vec3 restingPosition;
	const float MOVE_SPEED = 5.0f;
	glm::mat4 Scale = glm::scale(glm::mat4(1.0), glm::vec3(1, 1, 1));
	glm::mat4 Rotate = glm::rotate(glm::mat4(1.0), 0.0f, glm::vec3(0, 0, 1));
	float verticalSpeed = 0.0f;
	glm::vec3 rocketSpeed = glm::vec3(0.0f, 0.0f, 0.0f);
	float GRAVITY_CONSTANT = 0.1f;

	glm::mat4 View = glm::lookAt(camPos, rocketPosition, glm::vec3(0, 1, 0));
	glm::vec3 camPos = rocketPosition + glm::vec3(6, 3, 10) / 2.0f;
	glm::vec3 rocketCameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 DEFAULT_POSITION = glm::vec3(0.0f, 1.0f, 4.0f);
	glm::vec3 BETWEEN_BED_AND_CLOSET = glm::vec3(-2.0f, 0.5f, 1.0f);
	glm::vec3 ABOVE_CLOSET = glm::vec3(-1.0f, 3.0f, 0.4f);
	glm::vec3 ABOVE_RECORD_TABLE = glm::vec3(-3.0f, 2.0f, 7.5f);
	glm::vec3 BEHIND_RED_COLUMN = glm::vec3(5.0f, 2.0f, 7.0f);
	std::vector<glm::vec3> coinLocations = {DEFAULT_POSITION,
											BETWEEN_BED_AND_CLOSET, ABOVE_CLOSET,
											ABOVE_RECORD_TABLE, BEHIND_RED_COLUMN};

	void localInit() override {
		// Init descriptor layouts [what will be passed to the shaders]
		SC.initLayouts(this, "models/scene.json");


		// init vertex descriptors
		VD.init(this, {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}},
				{{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
				  sizeof(glm::vec3), POSITION},
				 {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
				  sizeof(glm::vec3), NORMAL},
				 {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
				  sizeof(glm::vec2), UV}});

		// Init pipelines
		PCookTorrance.init(this, &VD, "shaders/CookTorranceShaderVert.spv",
						   "shaders/CookTorranceShaderFrag.spv",
						   {SC.DSL[SC.LayoutIds["DSLGlobal"]]});
		PEmission.init(this, &VD, "shaders/LambertBlinnShaderVert.spv",
					   "shaders/LambertBlinnSEShaderFrag.spv",
					   {SC.DSL[SC.LayoutIds["DSLGlobal"]]});
		PCartoon.init(this, &VD, "shaders/ToonShaderVert.spv",
					  "shaders/ToonShaderFrag.spv",
					  {SC.DSL[SC.LayoutIds["DSLGlobal"]]});

		// Init scene (models & textures)
		SC.init(this, &VD, PCookTorrance, "models/scene.json");
		MRocket.init(this, &VD, "models/rocket.obj", OBJ, "rocket", SC.vecMap);

		// Init local variables
		rocketCollider.radius = 0.05f;
	}

	void pipelinesAndDescriptorSetsInit() override {
		PCookTorrance.create();
		PEmission.create();
		PCartoon.create();

		std::unordered_map<std::string, std::vector<DescriptorSetElement>> bindings;
		bindings["default"] = {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
							   {1, TEXTURE, 0, SC.T[0]},
							   {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}};

		bindings["abstractPainting"] = {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
										{1, TEXTURE, 0, SC.T[1]},
										{2, UNIFORM,
										 sizeof(GlobalUniformBufferObject), nullptr}};

		bindings["rocket"] = {{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
							  {1, TEXTURE, 0, SC.T[2]},
							  {2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}};

		SC.pipelinesAndDescriptorSetsInit(bindings);
		DSRocket.init(this, {SC.DSL[SC.LayoutIds["DSLGlobal"]]}, bindings["rocket"]);
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() override {
		// cleanup pipelines
		PCookTorrance.cleanup();
		PEmission.cleanup();
		PCartoon.cleanup();

		// cleanup descriptor sets
		SC.pipelinesAndDescriptorSetsCleanup();

		DSRocket.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() override {
		// cleanup textures & models
		SC.localCleanup();
		MRocket.cleanup();
		// Destroys the pipelines
		PCookTorrance.destroy();
		PEmission.destroy();
		PCartoon.destroy();
	}

	// Here is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
		// binds the pipeline
		PCookTorrance.bind(commandBuffer);
		// for a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter

		// binds the data sets
		SC.populateCommandBuffer(commandBuffer, currentImage, PCookTorrance);

		PCartoon.bind(commandBuffer);
		MRocket.bind(commandBuffer);
		DSRocket.bind(commandBuffer, PCartoon, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(MRocket.indices.size()), 1, 0, 0, 0);
	}

	// Rocket
	// restingPosition = glm::vec3(0.0f);

	int coinLocation = 0;

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

	void placeObject(std::string mId, std::string iId, glm::mat4& World,
					 std::unordered_map<std::string, BoundingBox>& bbMap) {
		if(bbMap.find(iId) == bbMap.end()) {
			BoundingBox bbox;
			glm::vec4 homogeneousPoint;

			bbox.min = glm::vec3(std::numeric_limits<float>::max());
			bbox.max = glm::vec3(std::numeric_limits<float>::lowest());
			for(int j = 0; j < SC.vecMap[mId].size(); j++) {
				glm::vec3 vertex = SC.vecMap[mId][j];
				homogeneousPoint = glm::vec4(vertex, 1.0f);
				glm::vec4 newVertex = World * homogeneousPoint;
				vertex = glm::vec3(newVertex);

				bbox.min = glm::min(bbox.min, vertex);
				bbox.max = glm::max(bbox.max, vertex);
			}
			bbox.max = glm::round(bbox.max * 100.0f) / 100.0f;
			bbox.min = glm::round(bbox.min * 100.0f) / 100.0f;
			(mId.substr(0, 4) == "coin") ? bbox.cType = COLLECTIBLE
										 : bbox.cType = OBJECT;

			bbMap[iId] = bbox;
		}
	}

	void getDirection() {
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
	}

	void getCameraControls() {
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

		// Update global uniforms
		GlobalUniformBufferObject gubo{};
		// Direct light
		gubo.lightDir[0].v =
			glm::vec3(cos(glm::radians(135.0f)),  // * cos(cTime * angTurnTimeFact)),
					  sin(glm::radians(135.0f)),
					  cos(glm::radians(135.0f)));  // * sin(cTime * angTurnTimeFact));
		gubo.lightPos[0].v = glm::vec3(7.0f, 6.0f, 0.0f);
		gubo.lightColor[0] = glm::vec4(1.0f);

		// Point light (roof lamp)
		gubo.lightDir[1].v = glm::vec3(0.0f);
		gubo.lightPos[1].v = glm::vec3(0.0f, 4.0f, 4.0f);
		gubo.lightColor[1] = glm::vec4(1.0f, 1.0f, 1.0f, 2.0f);
		gubo.eyeDir = glm::vec4(0.0f);
		gubo.eyeDir.w = 1.0f;
		gubo.eyePos = camPos;

		// Place static objects on scene
		UniformBufferObject ubo{};
		int i;
		std::string instanceId, modelId;
		for(auto instance : SC.InstanceIds) {
			i = instance.second;
			instanceId = instance.first;
			modelId = *SC.I[i].BBid;
			ubo.mvpMat = ViewPrj * SC.I[i].Wm;
			placeObject(modelId, instanceId, SC.I[i].Wm, SC.bbMap);
			SC.DS[i]->map(currentImage, &ubo, sizeof(ubo), 0);
			SC.DS[i]->map(currentImage, &gubo, sizeof(gubo), 2);
		}

		// Need to check collisions first
		bool isCollision = false;
		std::string collisionId;
		for(auto bb : SC.bbMap) {
			if(checkCollision(rocketCollider, bb.second)) {
				isCollision = true;
				// Grab key of colliding object
				collisionId = bb.first;
				break;
			}
		}

		getDirection();

		// Gravity (gravity constant can be lowered)
		if(rocketState == MOVING) {	 // If the rocket is falling apply gravity
			verticalSpeed += GRAVITY_CONSTANT * deltaT;
			// Set terminal fall speed
			verticalSpeed = glm::max(verticalSpeed, 0.1f);
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
			rocketState = MOVING;
			// Reset direction to avoid permanently going in the same direction
			rocketDirection = {0, 0, 0};
			// "Cancel" gravity while accelerating
			verticalSpeed = 0.0f;
		}
		/* else {


			if(!isCollision) {
				if(rocketState == RESTING) {
					rocketPosition = restingPosition;
				} else {
					// rocketPosition.y -= verticalSpeed * deltaT;
					// Deceleration towards minimum speed (0)
					if(glm::length(rocketSpeed) > 0.0f) {
						float speed = glm::length(rocketSpeed);
						speed -= MOVE_SPEED * deltaT;
						speed = glm::max(speed, 0.0f);
						rocketSpeed = glm::normalize(rocketSpeed) * speed;
					}
				}
			} else {
				switch(SC.bbMap[collisionId].cType) {
					case OBJECT: {
						// Compute the closest point on the AABB to the sphere center
						glm::vec3 closestPoint =
							glm::clamp(rocketPosition, SC.bbMap[collisionId].min,
									   SC.bbMap[collisionId].max);

						// Calculate the normal of the collision surface
						glm::vec3 difference = rocketPosition - closestPoint;
						float distance = glm::length(difference);

						glm::vec3 normal = glm::normalize(difference);
						// Move the sphere out of collision along the normal
						rocketPosition = closestPoint + normal * rocketCollider.radius;

						// Adjust the sphere's velocity to slide along the AABB surface
						float dotProduct = glm::dot(rocketSpeed, normal);
						glm::vec3 correction = normal * dotProduct;
						rocketSpeed -= correction;

						if(rocketPosition.y <= SC.bbMap[collisionId].max.y +
												   rocketCollider.radius &&	 // If the collision is coming from above
						   !(std::abs(normal.x) > 0.5f ||
							 std::abs(normal.z) > 0.5f) &&	// Not from the side
						   normal.y != -1.0f) {				// Not from below
							rocketSpeed = glm::vec3(0.0f);
							rocketState = RESTING;
							restingPosition.x = rocketPosition.x;
							restingPosition.y = SC.bbMap[collisionId].max.y +
												rocketCollider.radius + 0.01f;
							restingPosition.z = rocketPosition.z;
						}
						break;
					}
					case COLLECTIBLE: {
						coinLocation = (std::rand() % (4 - 0 + 1));
						SC.bbMap.erase(collisionId);
						break;
					}
				}
			}
		}*/

		if(isCollision) {
			switch(SC.bbMap[collisionId].cType) {
				case OBJECT: {
					// Compute the closest point on the AABB to the sphere center
					glm::vec3 closestPoint =
						glm::clamp(rocketPosition, SC.bbMap[collisionId].min,
								   SC.bbMap[collisionId].max);

					// Calculate the normal of the collision surface
					glm::vec3 difference = rocketPosition - closestPoint;
					float distance = glm::length(difference);

					glm::vec3 normal = glm::normalize(difference);

					// Move the sphere out of collision along the normal
					rocketPosition = closestPoint + normal * rocketCollider.radius;
					// Adjust the sphere's velocity to slide along the AABB surface
					float dotProduct = glm::dot(rocketSpeed, normal);
					glm::vec3 correction = normal * dotProduct;
					rocketSpeed -= correction;
					if(rocketPosition.y <=
						   SC.bbMap[collisionId].max.y + rocketCollider.radius &&  // If the collision is coming from above
					   !(std::abs(normal.x) > 0.5f || std::abs(normal.z) > 0.5f) &&	 // Not from the side
					   normal.y != -1.0f) {	 // Not from below
						rocketState = RESTING;
						restingPosition.x = rocketPosition.x;
						restingPosition.y = rocketPosition.y + 0.01f;
						restingPosition.z = rocketPosition.z;
						rocketSpeed = glm::vec3(0.0f);
					}
					break;
				}
				case COLLECTIBLE: {
					coinLocation = (std::rand() % (4 - 0 + 1));
					SC.bbMap.erase(collisionId);
					break;
				}
			}
		}

		if(rocketState == RESTING) {
			rocketPosition = restingPosition;
		} else {
			rocketSpeed.y -= verticalSpeed;
			rocketSpeed.y = glm::max(rocketSpeed.y, -1.75f);
			rocketPosition += rocketSpeed * deltaT;
		}

		// Camera controls
		getCameraControls();

		if(rocketCameraRotation.y > 89.0f) rocketCameraRotation.y = 89.0f;
		if(rocketCameraRotation.y < -89.0f) rocketCameraRotation.y = -89.0f;
		if(rocketCameraRotation.x < -89.0f) rocketCameraRotation.x = -89.0f;
		if(rocketCameraRotation.x > 89.0f) rocketCameraRotation.x = 89.0f;


		// Update rocket world matrix
		World = glm::translate(glm::mat4(1.0f), rocketPosition);
		World *= glm::rotate(glm::mat4(1.0f), glm::radians(rocketRotation.y),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::rotate(glm::mat4(1.0f), glm::radians(rocketRotation.x),
							 glm::vec3(1.0f, 0.0f, 0.0f));
		World *= glm::scale(glm::mat4(1.0f), glm::vec3(0.02f, 0.02f, 0.02f));

		// Update view matrix
		float radius = 0.5f;
		float camx =
			sin(glm::radians(rocketRotation.y + rocketCameraRotation.y)) * radius;
		float camz =
			cos(glm::radians(rocketRotation.y + rocketCameraRotation.y)) * radius;
		float camy =
			-sin(glm::radians(rocketRotation.x + rocketCameraRotation.x)) * radius;
		camPos = glm::vec3(camx, camy, camz) + rocketPosition;

		View = glm::lookAt(camPos, rocketPosition, glm::vec3(0, 1, 0));
		// Update mvpMat and map the rocket
		RocketUbo.mvpMat = Prj * View * World;
		rocketCollider.center = rocketPosition;

		DSRocket.map(currentImage, &RocketUbo, sizeof(RocketUbo), 0);
		DSRocket.map(currentImage, &gubo, sizeof(GlobalUniformBufferObject), 2);
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