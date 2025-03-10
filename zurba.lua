
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

   postbuildcommands { '{DELETE} %{cfg.targetdir}\\data & {COPYDIR} data %{cfg.targetdir}\\data' }

   includedirs { "external" }

   links {"d3d12.lib", "dxgi.lib", "d3dcompiler.lib"}

   files { "src/**.h", "src/**.cpp", "external/**.h" }
   

   filter { "configurations:Debug" }
      defines { "_DEBUG" }
      symbols "On"

   filter { "configurations:Release" }
      defines { "_RELEASE" }
      optimize "On"
