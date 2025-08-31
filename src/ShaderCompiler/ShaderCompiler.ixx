module;
#include <cstdint>
#include <string>
export module ShaderCompiler;

export class ShaderCompilerResult
{
public:
	ShaderCompilerResult() = default;
	ShaderCompilerResult(const std::string&& aError)
		:m_ErrorMessage(aError), m_HasError(true){ }

	bool HasError() const { return m_HasError; }
	const std::string& GetError() const { return m_ErrorMessage; }

	virtual void* GetBuffer() = 0;
	virtual size_t GetBufferSize() = 0;
	virtual void* GetPdbBuffer() = 0;
	virtual size_t GetPdbBufferSize() = 0;
	virtual const std::wstring& GetPdbName() const = 0;
protected:
	std::string m_ErrorMessage = "";
	bool m_HasError = false;
};