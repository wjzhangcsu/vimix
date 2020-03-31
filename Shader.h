#ifndef __SHADER_H_
#define __SHADER_H_

#include <string>
#include <vector>
#include <glm/glm.hpp>

class ShadingProgram
{
public:
    ShadingProgram(const std::string& vertex_file, const std::string& fragment_file);
    void init();
    bool initialized();
    void use();
	template<typename T> void setUniform(const std::string& name, T val);
	template<typename T> void setUniform(const std::string& name, T val1, T val2);
	template<typename T> void setUniform(const std::string& name, T val1, T val2, T val3);

	static void enduse();

private:
	void checkCompileErr();
	void checkLinkingErr();
	void compile();
	void link();
	unsigned int vertex_id_, fragment_id_, id_;
	std::string vertex_code_;
	std::string fragment_code_;
    std::string vertex_file_;
    std::string fragment_file_;

    static ShadingProgram *currentProgram_;
};

class Shader
{
public:
    Shader();
    virtual ~Shader() {}

    virtual void use();
    virtual void reset();

    glm::mat4 projection;
    glm::mat4 modelview;
    glm::vec4 color;

    void setModelview(float x, float y, float angle, float scale, float aspect_ratio);

protected:
    ShadingProgram *program_;
};

#endif /* __SHADER_H_ */