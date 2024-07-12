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
// Example:
struct UniformBlock {
	alignas(16) glm::mat4 mvpMat;
};

struct GlobalUniformBlock {
	alignas(16) glm::vec3 lightDir;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
	alignas(16) glm::vec4 eyeDir;
};

// The vertices data structures
// Example
struct Vertex {
	glm::vec3 pos;
	glm::vec2 UV;
};


// MAIN !
class ConfigManager : public BaseProject {
protected:
	// Current aspect ratio (used by the callback that resized the window
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSL;

	// Vertex formats
	VertexDescriptor VD;

	// Pipelines [Shader couples]
	Pipeline P;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex
	// structure Models
	Model<Vertex> MRocket;
	Model<Vertex> MWallN;
	Model<Vertex> MWallE;
	Model<Vertex> MWallS;
	Model<Vertex> MWallW;

	// Descriptor sets
	DescriptorSet DSRocket;
	DescriptorSet DSWallN;
	DescriptorSet DSWallE;
	DescriptorSet DSWallS;
	DescriptorSet DSWallW;

	// Textures
	Texture TFurniture1;
	Texture TFurniture2;
	Texture TFurniture3;
	Texture TFurniture4;
	Texture TFurniture5;

	// C++ storage for uniform variables
	UniformBlock RocketUbo;
	UniformBlock WallNUbo;
	UniformBlock WallEUbo;
	UniformBlock WallSUbo;
	UniformBlock WallWUbo;

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
		uniformBlocksInPool = 5;
		texturesInPool = 5;
		setsInPool = 5;

		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) override { Ar = (float)w / (float)h; }

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() override {
		// Descriptor Layouts [what will be passed to the shaders]
		DSL.init(this,
		         {// this array contains the bindings:
		          // first  element : the binding number
		          // second element : the type of element (buffer or texture)
		          // using the corresponding Vulkan constant
		          // third  element : the pipeline stage where it will be used
		          // using the corresponding Vulkan constant
		          {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
		          {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		           VK_SHADER_STAGE_FRAGMENT_BIT}});

		// Vertex descriptors
		VD.init(this,
		        {// this array contains the bindings
		         // first  element : the binding number
		         // second element : the stride of this binging
		         // third  element : whether this parameter change per vertex or
		         // per instance using the corresponding Vulkan constant
		         {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}},
		        {// this array contains the location
		         // first  element : the binding number
		         // second element : the location number
		         // third  element : the offset of this element in the memory
		         // record fourth element : the data type of the element using
		         // the corresponding Vulkan constant fifth  elmenet : the size
		         // in byte of the element sixth  element : a constant defining
		         // the element usage POSITION - a vec3 with the position NORMAL
		         // - a vec3 with the normal vector UV       - a vec2 with a UV
		         // coordinate COLOR    - a vec4 with a RGBA color TANGENT  - a
		         // vec4 with the tangent vector OTHER    - anything else
		         //
		         // ***************** DOUBLE CHECK ********************
		         // That the Vertex data structure you use in the "offsetof" and
		         //	in the "sizeof" in the previous array, refers to the correct
		         // one, 	if you have more than one vertex format!
		         // ***************************************************
		         {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
		          sizeof(glm::vec3), POSITION},
		         {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
		          sizeof(glm::vec2), UV}});

		// Pipelines [Shader couples]
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P.init(this, &VD, "shaders/ShaderVert.spv", "shaders/ShaderFrag.spv", {&DSL});

		// Models, textures and Descriptors (values assigned to the uniforms)
		// Create models
		// The second parameter is the pointer to the vertex definition for this model
		// The third parameter is the file name
		// The last is a constant specifying the file type: currently only OBJ or GLTF
		MRocket.init(this, &VD, "models/rotrocketypositive.obj", OBJ);
		MWallN.init(this, &VD, "models/gray_wall.mgcg", MGCG);
		MWallE.init(this, &VD, "models/gray_wall.mgcg", MGCG);
		MWallS.init(this, &VD, "models/gray_wall.mgcg", MGCG);
		MWallW.init(this, &VD, "models/gray_wall.mgcg", MGCG);

		// Create the textures
		// The second parameter is the file name
		//TFurniture1.init(this, "textures/Textures_Forniture.png");
		TFurniture2.init(this, "textures/Textures_Forniture.png");
		TFurniture3.init(this, "textures/Textures_Forniture.png");
		TFurniture4.init(this, "textures/Textures_Forniture.png");
		TFurniture5.init(this, "textures/Textures_Forniture.png");

		// Init local variables
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() override {
		// This creates a new pipeline (with the current surface), using its shaders
		P.create();

		// Here you define the data set
		DSRocket.init(this, &DSL,
		              {
		                  // the second parameter, is a pointer to the Uniform Set Layout of this set
		                  // the last parameter is an array, with one element per binding of the set.
		                  // first  elmenet : the binding number
		                  // second element : UNIFORM or TEXTURE (an enum) depending on the type
		                  // third  element : only for UNIFORMs, the size of the corresponding C++ object. For texture, just put 0
		                  // fourth element : only for TEXTUREs, the pointer to the corresponding texture object. For uniforms, use nullptr
		                  {0, UNIFORM, sizeof(UniformBlock), nullptr},
		                  //{1, TEXTURE, 0, &TFurniture1},
		              });
		DSWallN.init(this, &DSL,
		             {
		                 {0, UNIFORM, sizeof(UniformBlock), nullptr},
		                 {1, TEXTURE, 0, &TFurniture2},
		             });
		DSWallE.init(this, &DSL,
		             {
		                 {0, UNIFORM, sizeof(UniformBlock), nullptr},
		                 {1, TEXTURE, 0, &TFurniture3},
		             });
		DSWallS.init(this, &DSL,
		             {{0, UNIFORM, sizeof(UniformBlock), nullptr},
		              {1, TEXTURE, 0, &TFurniture4}});
		DSWallW.init(this, &DSL,
		             {{0, UNIFORM, sizeof(UniformBlock), nullptr},
		              {1, TEXTURE, 0, &TFurniture5}});
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() override {
		// Cleanup pipelines
		P.cleanup();

		// Cleanup descriptor sets
		DSRocket.cleanup();
		DSWallN.cleanup();
		DSWallE.cleanup();
		DSWallS.cleanup();
		DSWallW.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() override {
		// Cleanup textures
		//TFurniture1.cleanup();
		TFurniture2.cleanup();
		TFurniture3.cleanup();
		TFurniture4.cleanup();
		TFurniture5.cleanup();

		// Cleanup models
		MRocket.cleanup();
		MWallN.cleanup();
		MWallE.cleanup();
		MWallS.cleanup();
		MWallW.cleanup();

		// Cleanup descriptor set layouts
		DSL.cleanup();

		// Destroys the pipelines
		P.destroy();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) override {
		// binds the pipeline
		P.bind(commandBuffer);
		// For a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter

		// binds the data set
		DSRocket.bind(commandBuffer, P, 0, currentImage);
		MRocket.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MRocket.indices.size()), 1, 0, 0, 0);

		DSWallN.bind(commandBuffer, P, 0, currentImage);
		MWallN.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MWallN.indices.size()), 1, 0, 0, 0);

		DSWallE.bind(commandBuffer, P, 0, currentImage);
		MWallE.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MWallE.indices.size()), 1, 0, 0, 0);

		DSWallS.bind(commandBuffer, P, 0, currentImage);
		MWallS.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MWallS.indices.size()), 1, 0, 0, 0);

		DSWallW.bind(commandBuffer, P, 0, currentImage);
		MWallW.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
		                 static_cast<uint32_t>(MWallW.indices.size()), 1, 0, 0, 0);
	}

	glm::vec3 rocketPosition = glm::vec3(0.0f, 0.0f, 10.0f);
    glm::vec3 rocketDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 camPos = rocketPosition + glm::vec3(6, 3, 10) / 2.0f;
	glm::mat4 View = glm::lookAt(camPos, rocketPosition, glm::vec3(0, 1, 0));

    glm::vec3 rocketRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rocketCameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);

    const float ROT_SPEED = 50.0f;
    const float MOVE_SPEED = 5.0f;
    glm::vec3 CamPos = glm::vec3(0.0, 0.1, 5.0);
    glm::mat4 Scale = glm::scale(glm::mat4(1.0), glm::vec3(1, 1, 1));
    glm::mat4 Rotate = glm::rotate(glm::mat4(1.0), 0.0f, glm::vec3(0,0,1));
    float verticalSpeed = 0.0f;
    // Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) override {
		// Standard procedure to quit when the ESC key is pressed
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		// Integration with the timers and the controllers

		/*
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire);
		 */

		// getSixAxis() is defined in Starter.hpp in the base class.
		// It fills the float point variable passed in its first parameter with the time
		// since the last call to the procedure.
		// It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
		// to motion (with left stick of the gamepad, or WASD + RF keys on the keyboard)
		// It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
		// to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
		// If fills the last boolean variable with true if fire has been pressed:
		//          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button

		// Parameters
		// Camera FOV-y, Near Plane and Far Plane
		const float FOVy = glm::radians(90.0f);
		const float nearPlane = 0.1f;
		const float farPlane = 100.0f;

		glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;

		glm::mat4 World;

        // Rocket
		World = glm::translate(glm::mat4(1.0f), rocketPosition);
        World *= glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));
		RocketUbo.mvpMat = Prj * View * World;
		DSRocket.map(currentImage, &RocketUbo, sizeof(RocketUbo), 0);
		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block

        // Walls
		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(2.0f, 1.0f, 1.0f));
		WallNUbo.mvpMat = Prj * View * World;
		DSWallN.map(currentImage, &WallNUbo, sizeof(WallNUbo), 0);

		World = glm::translate(glm::mat4(1), glm::vec3(4.0f, 0.0f, 4.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(90.0f),
		                     glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(2.0f, 1.0f, 1.0f));
		WallEUbo.mvpMat = Prj * View * World;
		DSWallE.map(currentImage, &WallEUbo, sizeof(WallEUbo), 0);

		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 8.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(2.0f, 1.0f, 1.0f));
		WallSUbo.mvpMat = Prj * View * World;
		DSWallS.map(currentImage, &WallSUbo, sizeof(WallSUbo), 0);

		World = glm::translate(glm::mat4(1), glm::vec3(-4.0f, 0.0f, 4.0f));
		World *= glm::rotate(glm::mat4(1), glm::radians(-90.0f),
		                     glm::vec3(0.0f, 1.0f, 0.0f));
		World *= glm::scale(glm::mat4(1), glm::vec3(2.0f, 1.0f, 1.0f));
		WallWUbo.mvpMat = Prj * View * World;
		DSWallW.map(currentImage, &WallWUbo, sizeof(WallWUbo), 0);

        // Rocket Logic
		float deltaT = 0.016f;

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
		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { // Space only moves me forward
			rocketDirection.z -= 1.0f;
            float rocketSpeed = 0.0f;

            glm::mat4 rocketRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rocketRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            rocketRotationMatrix = glm::rotate(rocketRotationMatrix, glm::radians(rocketRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));

            glm::vec3 newRocketDirection = glm::vec3(rocketRotationMatrix * glm::vec4(rocketDirection, 0.0f));

            rocketSpeed += MOVE_SPEED * deltaT;
            rocketPosition += newRocketDirection * MOVE_SPEED * deltaT;

            rocketDirection = {0,0,0};
		}
		if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            rocketCameraRotation.x -= 1.0f;
		}
		if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            rocketCameraRotation.x += 1.0f;
		}
		if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            rocketCameraRotation.y -= 1.0f;
		}
		if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            rocketCameraRotation.y += 1.0f;
		}

        if (rocketCameraRotation.y > 89.0f)
            rocketCameraRotation.y = 89.0f;
        if (rocketCameraRotation.y < -89.0f)
            rocketCameraRotation.y = -89.0f;

        // Updating object position relative to its axis
        // Gravity
        if(!glfwGetKey(window, GLFW_KEY_SPACE)) {
            verticalSpeed += 9.18 * deltaT;
            rocketPosition.y -= verticalSpeed * deltaT;
            if(rocketPosition.y < 0.0f)
                rocketPosition.y = 0.0f;
        } else {
            verticalSpeed = 0.0f;
        }


        // Update rocket world matrix
        World = glm::translate(glm::mat4(1.0f), rocketPosition);
        World *= glm::rotate(glm::mat4(1.0f),glm::radians(rocketRotation.y), glm::vec3(0.0f,1.0f,0.0f));
        World *= glm::rotate(glm::mat4(1.0f), glm::radians(rocketRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        World *= glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

        float radius = 3.0f;
        float camx = sin(glm::radians(rocketRotation.y)) * radius;
        float camz = cos(glm::radians(rocketRotation.y)) * radius;
        float camy = - sin (glm::radians(rocketRotation.x)) * radius; // + 3?
        View = glm::lookAt(glm::vec3(camx, camy, camz) + rocketPosition,
                           rocketPosition,
                           glm::vec3(0,1,0));


		RocketUbo.mvpMat = Prj * View * World;
		DSRocket.map(currentImage, &RocketUbo, sizeof(RocketUbo), 0);

        rocketDirection = glm::vec3(0.0f,0.0f,0.0f);
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