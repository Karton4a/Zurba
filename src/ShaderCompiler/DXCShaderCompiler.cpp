module;
#include <wrl.h>
#include <dxcapi.h>
#include "../DXHelpers.h"
#include <fstream>
#include <vector>
module DXCShaderCompiler;

DXCShaderCompiler::DXCShaderCompiler()
{
	HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_DxcCompiler));
	ThrowIfFailed(hr);

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_DxcUtils));
	ThrowIfFailed(hr);

	hr = m_DxcUtils->CreateDefaultIncludeHandler(&m_IncludeHandler);
	ThrowIfFailed(hr);

}

DXCShaderCompiler::CompilationResult DXCShaderCompiler::Compile(std::filesystem::path aFilePath,
	const DXCArgs& aDxcArgs)
{
	std::ifstream shaderFile(aFilePath, std::ios_base::in);
	std::string utf8Path = aFilePath.generic_string();
	if (!shaderFile.is_open())
	{
		return std::make_shared<DXCShaderCompilerResult>(std::format("Cannot open file {}\n", utf8Path));
	}

	std::string buffer(std::istreambuf_iterator<char>(shaderFile), {});

	DxcBuffer sourceBuffer {
		.Ptr = buffer.data(),
		.Size = buffer.size()
	};

	std::wstring shaderTypeVersion = std::format(L"{}_{}_{}", ShaderTypeToString(aDxcArgs.Type), aDxcArgs.MajorVersion, aDxcArgs.MinorVersion);
	std::vector<LPCWSTR> args;
	args.push_back(L"-E");
	args.push_back(aDxcArgs.EntryPoint.c_str());
	args.push_back(L"-T");
	args.push_back(shaderTypeVersion.c_str());
	if (aDxcArgs.Debug)
	{
		args.push_back(L"-Od");
		args.push_back(L"-Zi");
	}
	else
	{
		args.push_back(L"-O3");
	}

	Microsoft::WRL::ComPtr<IDxcResult> result;
	HRESULT hr = m_DxcCompiler->Compile(&sourceBuffer, args.data(), static_cast<UINT32>(args.size()), m_IncludeHandler.Get(), IID_PPV_ARGS(&result));

	if (FAILED(hr))
	{
		return std::make_shared<DXCShaderCompilerResult>(std::format("Failed to compile shader \n"));
	}
	result->GetStatus(&hr);
	if (FAILED(hr))
	{
		Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
		if (errors && errors->GetStringLength() > 0)
		{
			return std::make_shared<DXCShaderCompilerResult>(std::format("Failed to compile shader {}\n {}\n", utf8Path, errors->GetStringPointer()));
		}
		else
		{
			return std::make_shared<DXCShaderCompilerResult>(std::format("Failed to compile shader \n", utf8Path));
		}
	}

	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
	result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);


	if (aDxcArgs.Debug)
	{
		Microsoft::WRL::ComPtr<IDxcBlob> pPDB;
		Microsoft::WRL::ComPtr<IDxcBlobUtf16> pPDBName;
		result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPDB), &pPDBName);
		return std::make_shared<DXCShaderCompilerResult>(shaderBlob, pPDBName->GetStringPointer(), pPDB);
	}

	return std::make_shared<DXCShaderCompilerResult>(shaderBlob);
}

const wchar_t* DXCShaderCompiler::ShaderTypeToString(DX12Shader::ShaderType aType)
{
	switch (aType)
	{
	case DX12Shader::Vertex:
		return L"vs";
	case DX12Shader::Pixel:
		return L"ps";
	case DX12Shader::Compute:
		return L"cs";
	case DX12Shader::Hull:
		return L"hs";
	case DX12Shader::Tesselation:
		return L"ts";
	case DX12Shader::Geometry:
		return L"gs";
	case DX12Shader::Unknown:
	default:
		return L"unknown";
	}
}
