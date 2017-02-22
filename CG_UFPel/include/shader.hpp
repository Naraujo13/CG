#ifndef SHADER_HPP
#define SHADER_HPP

class Shader {
public:
	GLuint programID;
	Shader(const GLchar* vertex_file_path, const GLchar* fragment_file_path);
	Shader(const GLchar* vertex_file_path, const GLchar* fragment_file_path, const GLchar* geometry_file_path);
	void use();
};

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path, const char * geometry_file_path);

#endif
