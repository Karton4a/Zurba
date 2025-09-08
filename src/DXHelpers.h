#pragma once
#include <winerror.h>
#include <exception>
#include <format>
#include <iostream>
#include "Windows.h"
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
}

template<typename... Args>
inline void DLog(const std::wformat_string<Args...> fmt, Args&&... args)
{
    OutputDebugString(std::vformat(fmt.get(), std::make_wformat_args(args...)).c_str());
}

template<typename... Args>
inline void DLog(const std::wstring& aPrint)
{
    OutputDebugString(aPrint.c_str());
}

template<typename... Args>
inline void DLog(const wchar_t* aPrint)
{
    OutputDebugString(aPrint.c_str());
}


template<typename... Args>
inline void DLog(const std::format_string<Args...> fmt, Args&&... args)
{
    OutputDebugStringA(std::vformat(fmt.get(), std::make_format_args(args...)).c_str());
}

template<typename... Args>
inline void DLog(const char* aPrint)
{
    OutputDebugStringA(aPrint);
}