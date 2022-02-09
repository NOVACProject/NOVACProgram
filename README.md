# NOVACProgram
This program works together with the NOVAC Scanning DOAS system (Scanning Dualbeam miniature Differential Optical Absorption Spectroscopy system).


## SpectralEvaluation
Building the NovacProgram also requires the repository SpectralEvaluation, which contains shared components between the NovacProgram, NovacPPP and MobileDOAS. This repo is added using the feature _submodule_ in git which pins the code to a specific commit in SpectralEvaluation.

When cloning NOVACProgram, also run:

git submodule init

git submodule update

to checkout the correct commit of SpectralEvaluation to the working directory.

The solution is named _NovacMasterProgram.sln_ and is regularly built using Visual Studio Community Edition 2022. Currently only in 32-bit mode.

'
