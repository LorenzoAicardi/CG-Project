#include "Starter.hpp"
#include <json.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

typedef struct {
	std::string *id;
	int Mid;
	int Tid;
	int DSLid;
	int Pid;
	std::string *BBid;	// equal to model id
	glm::mat4 Wm;
} Instance;

class TransformInterpreter {
public:
	/**
	 * Compute world matrix from a list of transformations
	 * @param transforms array of transformation objects (assumed to be valid)
	 * @return the world matrix
	 */
	static glm::mat4 computeWorld(nlohmann::json transforms) {
		glm::mat4 Wm = glm::mat4(1.0f);
		int numTrans = transforms.size();

		for(int i = 0; i < numTrans; i++) {
			nlohmann::json trans = transforms[i];

			if(trans["type"] == "translate") {
				float tx = trans["vec"][0];
				float ty = trans["vec"][1];
				float tz = trans["vec"][2];

				Wm *= glm::translate(glm::mat4(1.0f), glm::vec3(tx, ty, tz));
			} else if(trans["type"] == "rotate") {
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
			} else if(trans["type"] == "scale") {
				float sx = trans["vec"][0];
				float sy = trans["vec"][1];
				float sz = trans["vec"][2];

				Wm *= glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz));
			}
		}

		return Wm;
	}
};

class LayoutInterpreter {
public:
	/**
	 * Build DSLs from recipe directives
	 * @param bindings json array describing bindings
	 * @return array of DSLs
	 */
	static std::vector<DescriptorSetLayoutBinding> getBindings(nlohmann::json bindings) {
		int numBindings = bindings.size();
		std::vector<DescriptorSetLayoutBinding> res(numBindings);

		for(int i = 0; i < numBindings; i++) {
			nlohmann::json binding = bindings[i];

			res[i].binding = i;

			if(binding["type"] == "ubo") {
				res[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			} else if(binding["type"] == "img") {
				res[i].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			}

			if(binding["stage"] == "vert") {
				res[i].flags = VK_SHADER_STAGE_VERTEX_BIT;
			} else if(binding["stage"] == "frag") {
				res[i].flags = VK_SHADER_STAGE_FRAGMENT_BIT;
			} else {
				res[i].flags = VK_SHADER_STAGE_ALL_GRAPHICS;
			}
		}

		return res;
	}
};

/**
 * Number of buffer objects, textures and descriptors
 * to allocate
 */
struct ResourceAmount {
	int uboInPool;
	int textureInPool;
	int dsInPool;

	ResourceAmount() : uboInPool(0), textureInPool(0), dsInPool(0) {}
};

/**
 * Handle scene initialization and
 * object placement.
 * In particular, the following elements' lifecycle
 * is handled:
 * Pipelines
 * DSLs
 * DSs
 * Textures
 * Models
 */
template<class Vert>
class SceneManager {
public:
	BaseProject *BP;

	VertexDescriptor *VD;

	/// Layouts
	int LayoutCount = 0;
	DescriptorSetLayout **DSL;
	std::unordered_map<std::string, int> LayoutIds;

	/// Models
	int ModelCount = 0;
	Model<Vert> **M;
	std::unordered_map<std::string, int> MeshIds;
	std::unordered_map<std::string, std::vector<glm::vec3>> vecMap;
	std::unordered_map<std::string, BoundingBox> bbMap;

	/// Textures
	int TextureCount = 0;
	Texture **T;
	std::unordered_map<std::string, int> TextureIds;

	/// Descriptor sets and instances
	int InstanceCount = 0;
	DescriptorSet **DS;
	Instance *I;
	std::unordered_map<std::string, int> InstanceIds;

	/// Pipelines
	Pipeline **P;
	int PipelineCount = 0;
	std::unordered_map<std::string, int> PipelineIds;

	/// Resource counter
	ResourceAmount resCtr;

	void countResources(std::string file) {
		nlohmann::json js;
		std::ifstream ifs(file);

		if(!ifs.is_open()) {
			std::cout << "Error! Scene file not found!";
			exit(-1);
		}

		try {
			ifs >> js;
			ifs.close();

			resCtr.textureInPool = js["textures"].size();
			resCtr.uboInPool = js["instances"].size();
			resCtr.dsInPool = js["instances"].size();
		} catch(const nlohmann::json::exception &e) {
			std::cout << e.what() << '\n';
		}
	}

	void initLayouts(BaseProject *_BP, std::string file) {
		BP = _BP;

		nlohmann::json js;
		std::ifstream ifs(file);

		if(!ifs.is_open()) {
			std::cout << "Error! Scene file not found!";
			exit(-1);
		}

		try {
			ifs >> js;
			ifs.close();

			// Layouts
			nlohmann::json ly = js["layouts"];
			LayoutCount = ly.size();
			std::cout << "Layout count: " << LayoutCount << "\n";
			DSL = (DescriptorSetLayout **)calloc(LayoutCount,
												 sizeof(DescriptorSetLayout *));
			for(int k = 0; k < LayoutCount; k++) {
				LayoutIds[ly[k]["name"]] = k;
				DSL[k] = new DescriptorSetLayout();

				auto bindings =
					LayoutInterpreter::getBindings(ly[k]["bindings"]);
				DSL[k]->init(BP, bindings);
			}
		} catch(const nlohmann::json::exception &e) {
			std::cout << e.what() << '\n';
		}
	}

	void initPipelines(BaseProject *_BP, VertexDescriptor *VD, std::string file) {
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

			// Pipelines
			nlohmann::json ppl = js["pipelines"];
			PipelineCount = ppl.size();
			P = (Pipeline **)calloc(PipelineCount, sizeof(Pipeline *));
			for(int k = 0; k < PipelineCount; k++) {
				PipelineIds[ppl[k]["name"]] = k;
				std::string frag = ppl[k]["frag"];
				std::string vert = ppl[k]["vert"];
				std::string layout = ppl[k]["layout"];

				P[k] = new Pipeline();
				P[k]->init(BP, VD, vert, frag, {DSL[LayoutIds[layout]]});
			}
		} catch(const nlohmann::json::exception &e) {
			std::cout << e.what() << '\n';
		}
	}

	void init(BaseProject *_BP, VertexDescriptor *VD, std::string file) {
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

			// Models
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
						   ms[k]["id"], vecMap);
			}

			// Textures
			nlohmann::json ts = js["textures"];
			TextureCount = ts.size();
			std::cout << "Textures count: " << TextureCount << "\n";

			T = (Texture **)calloc(ModelCount, sizeof(Texture *));
			for(int k = 0; k < TextureCount; k++) {
				TextureIds[ts[k]["id"]] = k;
				T[k] = new Texture();

				T[k]->init(BP, ts[k]["texture"].template get<std::string>().c_str());
			}

			// Instances
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
				I[k].DSLid = LayoutIds[is[k]["layout"]];
				I[k].Pid = PipelineIds[is[k]["pipeline"]];
				I[k].BBid = new std::string(is[k]["model"]);

				I[k].Wm =
					TransformInterpreter::computeWorld(is[k]["transforms"]);
			}
		} catch(const nlohmann::json::exception &e) {
			std::cout << e.what() << '\n';
		}
	}

	void createPipelines() {
		for(int i = 0; i < PipelineCount; i++) {
			P[i]->create();
		}
	}

	void pipelinesAndDescriptorSetsInit(
		std::unordered_map<std::string, std::vector<DescriptorSetElement>> dsInst) {
		// Assumed to always exist
		auto defaultBinding = dsInst["default"];

		for(auto inst : InstanceIds) {
			int i = inst.second;
			DS[i] = new DescriptorSet();
			if(dsInst.find(inst.first) != dsInst.end())
				DS[i]->init(BP, DSL[I[i].DSLid], dsInst[inst.first]);
			else
				DS[i]->init(BP, DSL[I[i].DSLid], defaultBinding);
		}
	}

	void pipelinesAndDescriptorSetsCleanup() {
		for(int i = 0; i < PipelineCount; i++) {
			P[i]->cleanup();
		}
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

		// Cleanup layouts
		for(int i = 0; i < LayoutCount; i++) {
			DSL[i]->cleanup();
			delete DSL[i];
		}
		free(DSL);

		free(DS);
		for(int i = 0; i < InstanceCount; i++) {
			delete I[i].id;
		}
		free(I);

		// Destroy pipelines
		for(int i = 0; i < PipelineCount; i++) {
			P[i]->destroy();
			delete P[i];
		}
		free(P);
	}

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		for(int i = 0; i < InstanceCount; i++) {
			P[I[i].Pid]->bind(commandBuffer);
			M[I[i].Mid]->bind(commandBuffer);
			DS[i]->bind(commandBuffer, *P[I[i].Pid], 0, currentImage);

			vkCmdDrawIndexed(commandBuffer,
							 static_cast<uint32_t>(M[I[i].Mid]->indices.size()),
							 1, 0, 0, 0);
		}
	}
};