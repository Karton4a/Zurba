require "external/premake-ecc/ecc"

PROJECT_NAME = "Zurba"

workspace(PROJECT_NAME)
   configurations { "Debug", "Release" }

   platforms { "Win64" }
   filter { "platforms:Win64" }
   system "Windows"
   architecture "x86_64"

project(PROJECT_NAME)
   kind "WindowedApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "C++20"

   postbuildcommands { '{DELETE} %{cfg.targetdir}\\data & {COPYDIR} data %{cfg.targetdir}\\data & {COPYFILE} external\\dxc\\bin\\x64\\dxcompiler.dll %{cfg.targetdir}' }

   includedirs { "external", "external/dxc/inc" }

   links {"d3d12.lib", "dxgi.lib", "d3dcompiler.lib", "external/dxc/lib/x64/dxcompiler.lib"}

   files { "src/**.h", "src/**.cpp", "src/**.ixx", "external/**.h" }
   

   filter { "configurations:Debug" }
      defines { "_DEBUG" }
      symbols "On"

   filter { "configurations:Release" }
      defines { "_RELEASE" }
      optimize "On"
