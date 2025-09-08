module;
#include <wrl.h>
#include <dxcapi.h>
#include <string>
#include <filesystem>
export module DXCShaderCompiler;
import DX12Shader;
import ShaderCompiler;

export struct DXCArgs
{
	std::wstring EntryPoint = L"main";
	DX12Shader::ShaderType Type = DX12Shader::ShaderType::Unknown;
	uint8_t MajorVersion = 6;
	uint8_t MinorVersion = 0;
	bool Debug = false;
};

export class DXCShaderCompiler
{
public:
	DXCShaderCompiler();
	using CompilationResult = std::shared_ptr<ShaderCompilerResult>;

	CompilationResult Compile(std::filesystem::path aFilePath, const DXCArgs& aDxcArgs);
private:
	static const wchar_t* ShaderTypeToString(DX12Shader::ShaderType aType);

	Microsoft::WRL::ComPtr<IDxcUtils> m_DxcUtils;
	Microsoft::WRL::ComPtr<IDxcCompiler3> m_DxcCompiler;
	Microsoft::WRL::ComPtr <IDxcIncludeHandler> m_IncludeHandler;
};

class DXCShaderCompilerResult : public ShaderCompilerResult
{
public:
	DXCShaderCompilerResult(Microsoft::WRL::ComPtr<IDxcBlob>& aBlob)
		:ShaderCompilerResult(),
		m_Blob(aBlob),
		m_PdbName(),
		m_PdbBlob(nullptr)
	{}

	DXCShaderCompilerResult(Microsoft::WRL::ComPtr<IDxcBlob>& aBlob,
		LPCWSTR aPdbName,
		Microsoft::WRL::ComPtr<IDxcBlob>& aPdbBlob)
		:ShaderCompilerResult(),
		m_Blob(aBlob),
		m_PdbName(aPdbName),
		m_PdbBlob(aPdbBlob)
	{
	}

	DXCShaderCompilerResult(std::string&& aError)
		:ShaderCompilerResult(std::move(aError)),
		m_Blob(nullptr)
	{}

	void* GetBuffer() override
	{
		return m_Blob->GetBufferPointer();
	}
	size_t GetBufferSize() override
	{
		return m_Blob->GetBufferSize();
	}

	void* GetPdbBuffer() override
	{
		return m_PdbBlob->GetBufferPointer();
	}
	size_t GetPdbBufferSize() override
	{
		return m_PdbBlob->GetBufferSize();
	}
	const std::wstring& GetPdbName() const override
	{
		return m_PdbName;
	}

private:
	Microsoft::WRL::ComPtr<IDxcBlob> m_Blob;
	std::wstring m_PdbName;
	Microsoft::WRL::ComPtr<IDxcBlob> m_PdbBlob;
};