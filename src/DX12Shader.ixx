module;
#include <string>
export module DX12Shader;

export class DX12Shader
{
public:
	DX12Shader(std::string path);
	enum ShaderType
	{
		Vertex,
		Pixel,
		Compute,
		Hull,
		Tesselation,
		Geometry,
		Unknown,
		//etc
	};
private:
	void CompileShader(FILE* fd);
	void MakeReflection();
private:
	ShaderType m_Type;
	
};