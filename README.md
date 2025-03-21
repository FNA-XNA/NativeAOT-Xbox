# NativeAOT Xbox

This is the FNA project's NativeAOT bootstrapping program for FNA games. While many of the instructions and the demo are FNA-specific, the actual NativeAOT project can be used with any other .NET application, and may also be useful as a foundation for embedding .NET into an Xbox-compatible engine like Godot.

## Project Status

The repo is currently based on the .NET 8.0.1 tag, and will be updated for compatibility with future .NET versions.

Note that this does _NOT_ work unless you have the Xbox GDK, which requires signing the private GDK agreement with Microsoft. See [this](https://developer.microsoft.com/en-US/games/publish/) page for more details.

For the most part this is largely the Windows version but with GDK project files, some Windows API stubs, and a few GDKX-specific changes to existing files. We're opening this up to everybody since Xbox is a unique situation where FOSS projects can reference the GDK as long as we don't ship stuff that's _in_ the GDK or is precompiled with the GDK toolset. Our long-term goal is to make Xbox support an upstream .NET SDK feature, or at the very least an officially streamlined Xbox GDK feature.

## Known issues:

* Console.WriteLine output will not appear in any debug console (VS or XBWatson). This is an Xbox OS limitation. You have to use System.Diagnostics.Debug.WriteLine instead for any debug logging.

## Requirements

1. Microsoft Game Development Kit (with Xbox Extensions) version 220600 or later
2. Windows 10 SDK version 10.0.20348.0 or later

## Initial Setup

1. Clone this repo recursively: `git clone --recursive https://github.com/FNA-XNA/NativeAOT-Xbox`
2. `git clone https://github.com/dotnet/runtime -b v8.0.1` into a neighboring directory.
3. `git clone https://github.com/libsdl-org/SDL` into a neighboring directory.
4. `git clone --recursive https://github.com/libsdl-org/SDL_shadercross` into a neighboring directory.
5. `git clone --recursive https://github.com/FNA-XNA/FNA` into a neighboring directory.

## Setting up your game

1. Add your game project to the "managed" section of the NativeAOT-GDKX solution.
2. In your game project, add a project reference to FNA.Core.
3. Select the Bootstrap project, then select Project > Project Dependencies from the top menu. Add your game project as a dependency. This guarantees it will build when you build the whole solution.
4. Copy-paste the following into your .csproj:
```xml
<Import Project="$(SolutionDir)NativeAOT_Console.targets" Condition="Exists('$(SolutionDir)NativeAOT_Console.targets')" />
```
This does a number of things:
  * Installs the latest ILCompiler for Windows via NuGet.
  * Installs the latest win-x64 managed runtime libraries.
  * Generates path properties for the ILC package, so it can be used in the post-build step.
  * Adds a post-build step that automatically AOT-compiles your project for win-x64 and spits the output .o file into the solution's `aot-output` folder to be consumed by the Bootstrap project.
5. Add your game assets folder to the `Bootstrap/Gaming.Xbox.{PLATFORM}.x64/Layout/Image/Loose` directory.

An example of all these steps is included in the sln automatically in the form of DemoGame. Feel free to explore the implementation and experiment with it, and then rip it out and replace it with your own game.

## Running your game

Connect to your devkit, target your desired VS configuration, set the Startup project to Bootstrap, and click the big build button at the top. That's it!

Note: Your devkit may need to be hard-lined to your router.

If anything goes wrong, let us know in the FNA discord in the #xbox channel (again, requires GDK Xbox access)!

## About rd.xml

Inside the DemoGame is an XML file that contains information about the managed assemblies that may be important for AOT compilation - for example, you may need to specify that types are used primarily via reflection (XNA ContentReaders are a common example). UWP users will be familiar with this file, and BRUTE users will likely remember .brute config files; this is a similar idea.

You are encouraged to copy this file next to your game's project file and reference it in the project XML; you can see an example of this in DemoGame.csproj.

Inside the current rd.xml is the FNA ContentReaders, which should act as a good example. If you find that your game is crashing due to missing types, this is likely the issue!

For UWP users, there is a converter available [here](https://github.com/tomcashman/rd.xml-UWP-to-NativeAOT).
