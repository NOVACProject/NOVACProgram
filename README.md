# NOVACProgram
This program works together with the NOVAC Scanning DOAS system (Scanning Dualbeam miniature Differential Optical Absorption Spectroscopy system).


## SpectralEvaluation
Building the NovacProgram also requires the repository SpectralEvaluation, which contains shared components between the NovacProgram, NovacPPP and MobileDOAS. This repo is added using the feature _submodule_ in git which pins the code to a specific commit in SpectralEvaluation.

When cloning NOVACProgram, also run:

git submodule init

git submodule update

to checkout the correct commit of SpectralEvaluation to the working directory.

## Building

The solution is named _NovacMasterProgram.sln_ and is built with **Visual Studio 2026** (Community Edition works), using the **v145** platform toolset. Install the "Desktop development with C++" workload together with the optional **C++ MFC** component for the latest build tools, since the program links MFC statically.

From a Visual Studio 2026 Developer Command Prompt:

```
msbuild NovacMasterProgram.sln /m /p:Configuration=Release /p:Platform=x86
```

### Build status

| Configuration | Status |
|---|---|
| Release, x86 (32-bit) | Builds |
| Debug, x86 (32-bit) | Builds |
| Release, x64 (64-bit) | Does not build: fails at link time |
| Debug, x64 (64-bit) | Does not build: fails at link time |

The program is built and released as **32-bit (x86)**.

**Why x64 does not build:** the bundled curl library in `External/curl-7.74.0-win32-mingw` is a 32-bit build only, so the 64-bit link fails with unresolved `curl_*` symbols. Everything else, including NetCDF (which ships both `x86` and `x64` libraries in `External/NetCdf_4_7_2`), already compiles for x64. Building 64-bit requires adding a 64-bit curl build to `External/` and pointing the x64 configurations at it.
