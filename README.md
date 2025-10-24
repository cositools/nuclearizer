# COSI's calibration tool nuclearizer

## What is nuclearizer?

Nuclearizer is the detectior calibration tool of the gamma-ray telescope COSI, the Compton Spectrometer and Imager.
It handles (at least partly) the detector calibration parameter generation, the appliation of the calibration to measured data, and it as a detector effects engine to make simulation look like real data.
Nuclearizer is written in C++ and based on MEGAlib's detector calibration framework Fretalon.

## Short history

The ancestor of Nuclearizer is the calibration tool of the gamma-ray telescope MEGA, which was called MEGAlyze.
It inherited the data flow, the class organization, and the fact that it was based on CERN's ROOT library.
The development of nuclearizer started the year before COSI's second Fort Sumner balloon flight in 2008.
The code base was significanly rewritten and reorganized for version 2 of nuclearizer which was developped for the 2014 Antarctic balloon flight opd COSI. This is the first version based on MEGAlib's Fretalon calibration framework.
The current version is written for the COSI SMEX mission, which resulted in more code cleanups and further code reorganizations

## Installing nuclearizer

1. Nuclearizer is based on MEGAlib. Thus the first step is to install MEGAlib: https://github.com/zoglauer/MEGAlib
2. Then set the nuclearizer environment variable. For example in bash/zsh do:
   ```
   export NUCLEARIZER=/path/to/nuclearziers/main/directory
   ```
3. Then switch to the nuclearzier main directory and do
   ```
   make -j10
   ```
   Where 10 is the number of cores you want to use for compilation
4. Launch nuclearzier via
   ```
   nuclearizer
   ```

## Contributing

Everyboidy is welcome to contribute.
We follow the standard fork-pull-request workflow.

Please take a look at the file CodingConventions.md for the coding conventions and other programming tips.

### Debugging 

In case you need to debug nuclearizer do the following:
1. First you have to compile MEGALib and nuclearizer in debug mode:
   ```
   cd $MEGALIB
   bash configure --deb=on --opt=off
   make -j10
   cd $NUCLEARIZER
   make clean
   make -j 10
   ```
2. Then start nuclearizer in debug mode
   ```
   debug nuclearizer [you can add command line options here - it automatically runs]
   ```
   This works both on macOS and Linux
3. Then when nuclearizer crashes, use the standard gdb / lldb syntax to figure out what went wrong:
   ```
   bt (for backtrace)
   up (to move to the crash in nuclearizer/megalib code)
   p (fro priny the content of a variable)
   ```

### Issues

Feel free to report all issues in GitHub's issue tracker.



