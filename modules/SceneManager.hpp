

#include "Starter.hpp"
#include <json.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

typedef struct {
	std::string *id;
	int Mid;
	int Tid;
	glm::mat4 Wm;
} Instance;

class TransformInterpreter {
public:
	/**
	 * Compute world matrix from a list of transformations
	 * @param transforms array of transformation objects (assumed to be valid)
	 * @return the world matrix
	 */
	static glm::mat4 computeWm(nlohmann::json transforms) {
		glm::mat4 Wm = glm::mat4(1.0f);
		int nTrans = transforms.size();

		for(int i = 0; i < nTrans; i++) {
			nlohmann::json trans = transforms[i];

			if(trans["type"] == "translate") {
				float tx = trans["vec"][0];
				float ty = trans["vec"][1];
				float tz = trans["vec"][2];

				Wm *= glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz));
			}

			if(trans["type"] == "rotate") {
				float rx = trans["vec"][0];
				float ry = trans["vec"][1];
				float rz = trans["vec"][2];

				if(rx != 0.0f)
					Wm *= glm::rotate(glm::mat4(1.0f), glm::radians(rx),
									  glm::vec3(1.0f, 0.0f, 0.0f));
				if(ry != 0.0f)
					Wm *= glm::rotate(glm::mat4(1.0f), glm::radians(ry),
									  glm::vec3(0.0f, 1.0f, 0.0f));
				if(rz != 0.0f)
					Wm *= glm::rotate(glm::mat4(1.0f), glm::radians(rz),
									  glm::vec3(0.0f, 0.0f, 1.0f));
			}

			if(trans["type"] == "scale") {
				float sx = trans["vec"][0];
				float sy = trans["vec"][1];
				float sz = trans["vec"][2];

				Wm *= glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz));
			}
		}

		return Wm;
	}
};

/**
 * Handle scene initialization and
 * object placement
 */
template<class Vert>
class SceneManager {
public:
	VertexDescriptor *VD;

	BaseProject *BP;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex
	// structure Models
	int ModelCount = 0;
	Model<Vert> **M;
	std::unordered_map<std::string, int> MeshIds;
	std::vector<std::vector<glm::vec3>> vecList;

	// Textures
	int TextureCount = 0;
	Texture **T;
	std::unordered_map<std::string, int> TextureIds;

	// Descriptor sets and instances
	int InstanceCount = 0;
	DescriptorSet **DS;
	Instance *I;
	std::unordered_map<std::string, int> InstanceIds;

	void init(BaseProject *_BP, VertexDescriptor *VD, DescriptorSetLayout &DSL,
			  Pipeline &P, std::string file) {
		BP = _BP;

		nlohmann::json js;
		std::ifstream ifs(file);

		if(!ifs.is_open()) {
			std::cout << "Error! Scene file not found!";
			exit(-1);
		}
		try {
			std::cout << "Parsing JSON\n";
			ifs >> js;
			ifs.close();
			std::cout << "\n\n\nScene contains " << js.size()
					  << " definitions sections\n\n\n";

			// MODELS
			nlohmann::json ms = js["models"];
			ModelCount = ms.size();
			std::cout << "Models count: " << ModelCount << "\n";

			M = (Model<Vert> **)calloc(ModelCount, sizeof(Model<Vert> *));
			for(int k = 0; k < ModelCount; k++) {
				MeshIds[ms[k]["id"]] = k;
				std::string MT = ms[k]["format"].template get<std::string>();
				M[k] = new Model<Vert>();

				M[k]->init(BP, VD, ms[k]["model"].template get<std::string>(),
						   (MT[0] == 'O') ? OBJ : ((MT[0] == 'G') ? GLTF : MGCG),
						   vecList);
			}

			// TEXTURES
			nlohmann::json ts = js["textures"];
			TextureCount = ts.size();
			std::cout << "Textures count: " << TextureCount << "\n";

			T = (Texture **)calloc(ModelCount, sizeof(Texture *));
			for(int k = 0; k < TextureCount; k++) {
				TextureIds[ts[k]["id"]] = k;
				T[k] = new Texture();

				T[k]->init(BP, ts[k]["texture"].template get<std::string>().c_str());
			}

			// INSTANCES TextureCount
			nlohmann::json is = js["instances"];
			InstanceCount = is.size();
			std::cout << "Instances count: " << InstanceCount << "\n";

			DS = (DescriptorSet **)calloc(InstanceCount, sizeof(DescriptorSet *));
			I = (Instance *)calloc(InstanceCount, sizeof(Instance));
			for(int k = 0; k < InstanceCount; k++) {
				std::cout << k << "\t" << is[k]["id"] << ", " << is[k]["model"]
						  << "(" << MeshIds[is[k]["model"]] << "), "
						  << is[k]["texture"] << "("
						  << TextureIds[is[k]["texture"]] << ")\n";
				InstanceIds[is[k]["id"]] = k;
				I[k].id = new std::string(is[k]["id"]);
				I[k].Mid = MeshIds[is[k]["model"]];
				I[k].Tid = TextureIds[is[k]["texture"]];
				// nlohmann::json TMjson = is[k]["transform"];
				// float TMj[16];
				// for(int l = 0; l < 16; l++) {
				// 	TMj[l] = TMjson[l];
				// }
				// I[k].Wm = glm::mat4(TMj[0], TMj[4], TMj[8], TMj[12], TMj[1], TMj[5],
				// 					TMj[9], TMj[13], TMj[2], TMj[6], TMj[10],
				// 					TMj[14], TMj[3], TMj[7], TMj[11], TMj[15]);
				nlohmann::json trans = is[k]["transforms"];
				I[k].Wm = TransformInterpreter::computeWm(trans);
			}
		} catch(const nlohmann::json::exception &e) {
			std::cout << e.what() << '\n';
		}
	}


	void pipelinesAndDescriptorSetsInit(DescriptorSetLayout &DSL,
										std::vector<DescriptorSetElement> dsInst) {
		for(int i = 0; i < InstanceCount; i++) {
			DS[i] = new DescriptorSet();
			DS[i]->init(BP, &DSL, dsInst);
		}
	}

	void pipelinesAndDescriptorSetsCleanup() {
		for(int i = 0; i < InstanceCount; i++) {
			DS[i]->cleanup();
			delete DS[i];
		}
	}

	void localCleanup() {
		// Cleanup textures
		for(int i = 0; i < TextureCount; i++) {
			T[i]->cleanup();
			delete T[i];
		}
		free(T);

		// Cleanup models
		for(int i = 0; i < ModelCount; i++) {
			M[i]->cleanup();
			delete M[i];
		}
		free(M);

		free(DS);
		for(int i = 0; i < InstanceCount; i++) {
			delete I[i].id;
		}
		free(I);
	}

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage,
							   Pipeline &P) {
		for(int i = 0; i < InstanceCount; i++) {
			M[I[i].Mid]->bind(commandBuffer);
			DS[i]->bind(commandBuffer, P, 0, currentImage);

			vkCmdDrawIndexed(commandBuffer,
							 static_cast<uint32_t>(M[I[i].Mid]->indices.size()),
							 1, 0, 0, 0);
		}
	}
};