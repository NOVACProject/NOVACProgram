# NOVACProgram
This program works together with the NOVAC Scanning DOAS system (Scanning Dualbeam miniature Differential Optical Absorption Spectroscopy system).


## SpectralEvaluation
Building the NovacProgram also requires the repository SpectralEvaluation, which contains shared components between the NovacProgram, NovacPPP and MobileDOAS. This repo is added using the feature _submodule_ in git which pins the code to a specific commit in SpectralEvaluation.

When cloning NOVACProgram, also run:

git submodule init

git submodule update

to checkout the correct commit of SpectralEvaluation to the working directory.

## Version 3.2
Version 3.2 of the NovacProgram is, during development, tied to the branch 'dynamic_spectrometer_model' of SpectralEvaluation.
To build NovacProgram in the branch 3.2, make sure the folder 'SpectralEvaluation' is set to branch 'dynamic_spectrometer_model'.

'