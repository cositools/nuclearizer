#----------------------------------------------------------------
#
#  Makefile for Nuclearizer
#
#  Top level makefile: definitions, paths and command switch
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------


SHELL = /bin/sh




#----------------------------------------------------------------
# Directories:
#

# Basic directories 
TOPLEVEL = $(shell pwd)
IN = $(TOPLEVEL)/include
LB = $(MEGALIB)/lib
BN = $(MEGALIB)/bin


#----------------------------------------------------------------
# Include path/location macros (result of "sh configure")
#

include $(MEGALIB)/config/Makefile.options
include $(MEGALIB)/config/Makefile.config


#----------------------------------------------------------------
# Definitions based on architecture and user options
#

CXXFLAGS += -I$(IN) -I$(MEGALIB)/include -I/opt/local/include

# Names of the programs
NUCLEARIZERPRG = $(BN)/nuclearizer
NUCLEARIZERCXX = src/MNuclearizerMain.cxx

DEEPRG = $(BN)/deecosi
DEECXX = src/MNCTDetectorEffectsEngineCOSI.cxx

ALLPROGRAMS = $(NUCLEARIZERPRG) $(DEEPRG)

# The nuclearizer library
NUCLEARIZERLIB = \
$(LB)/MInterfaceNuclearizer.o \
$(LB)/MGUIMainNuclearizer.o \
$(LB)/MGUIEModule.o \
$(LB)/MGUIOptions.o \
$(LB)/MGUIModuleSelector.o \
$(LB)/MNCTArray.o \
$(LB)/MNCTCoincidenceVolume.o \
$(LB)/MNCTDetectorArray.o \
$(LB)/MNCTEvent.o \
$(LB)/MNCTHit.o \
$(LB)/MNCTHitInVoxel.o \
$(LB)/MNCTMath.o \
$(LB)/MNCTTimeAndCoordinate.o \
$(LB)/MNCTStrip.o \
$(LB)/MNCTStripEnergyDepth.o \
$(LB)/MNCTStripHit.o \
$(LB)/MNCTDetectorResponse.o \
$(LB)/MNCTGuardringHit.o \
$(LB)/MNCTFile.o \
$(LB)/MNCTFileEventsDat.o \
$(LB)/MNCTEventBuffer.o \
$(LB)/MNCTData.o \
$(LB)/MNCTModule.o \
$(LB)/MNCTModuleTemplate.o \
$(LB)/MGUIOptionsTemplate.o \
$(LB)/MNCTModuleMeasurementLoader.o \
$(LB)/MNCTModuleMeasurementLoaderROA.o \
$(LB)/MNCTModuleMeasurementLoaderGRIPS2013.o \
$(LB)/MNCTModuleMeasurementLoaderNCT2009.o \
$(LB)/MGUIOptionsMeasurementLoader.o \
$(LB)/MNCTModuleDetectorEffectsEngine.o \
$(LB)/MNCTModuleEnergyCalibration.o \
$(LB)/MNCTModuleEnergyCalibrationUniversal.o \
$(LB)/MGUIOptionsEnergyCalibrationUniversal.o \
$(LB)/MNCTModuleEnergyCalibrationLinear.o \
$(LB)/MNCTModuleEnergyCalibrationNonlinear.o \
$(LB)/MNCTInverseCrosstalkCorrection.o \
$(LB)/MNCTModuleCrosstalkCorrection.o \
$(LB)/MNCTModuleChargeSharingCorrection.o \
$(LB)/MNCTModuleDepthAndStripCalibration.o \
$(LB)/MNCTModuleDepthCalibration.o \
$(LB)/MNCTModuleDepthCalibrationLinearStrip.o \
$(LB)/MNCTModuleDepthCalibrationLinearPixel.o \
$(LB)/MNCTModuleDepthCalibration3rdPolyPixel.o \
$(LB)/MNCTModuleStripPairing.o \
$(LB)/MNCTModuleStripPairingGreedy.o \
$(LB)/MNCTModuleStripPairingGreedy_a.o \
$(LB)/MNCTModuleAspect.o \
$(LB)/MNCTModuleEventFilter.o \
$(LB)/MNCTModuleDumpEvent.o \
$(LB)/MGUIOptionsEventSaver.o \
$(LB)/MNCTModuleEventSaver.o \
$(LB)/MGUIOptionsDetectorEffectsEngine.o \
$(LB)/MGUIOptionsAspect.o \
$(LB)/MGUIOptionsEventFilter.o \
$(LB)/MNCTPreprocessor.o \
$(LB)/MCalibratorEnergy.o \
$(LB)/MCalibratorEnergyPointwiseLinear.o \

# The shared library
NUCLEARIZERSHAREDLIB = $(LB)/libNuclearizer.$(DLL)

# External libraries
# MEGAlib
ALLLIBS = -lCommonMisc -lCommonGui -lGeomega -lSivan -lRevan -lRevanGui -lSpectralyzeGui -lSpectralyze -lFretalonBase -L$(MEGALIB)/lib -L$(LB)
# ROOT
ALLLIBS += -lMathCore

#----------------------------------------------------------------
# Built-in targets
#

.EXPORT_ALL_VARIABLES: all header sources doc clean
.Phony:                all header sources doc clean


#----------------------------------------------------------------
# Command rules
#

# Compile all libraries and programs
all: $(ALLPROGRAMS)

# Clean-up
clean:
	@-rm -f $(NUCLEARIZERO) $(DEEO) $(NUCLEARIZERSHAREDLIB) $(NUCLEARIZERLIB)
	@-rm -f $(NUCLEARIZERPRG) $(DEEPRG)
	@-rm -f *~ include/*~ src/*~


#----------------------------------------------------------------
# Explicit rules & dependencies:
#

$(NUCLEARIZERLIB): $(LB)/%.o: src/%.cxx include/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NUCLEARIZERSHAREDLIB): $(NUCLEARIZERLIB)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(NUCLEARIZERLIB) $(GLIBS) $(LIBS) -o $(NUCLEARIZERSHAREDLIB)

$(NUCLEARIZERPRG): $(NUCLEARIZERSHAREDLIB) $(NUCLEARIZERCXX)
	@echo "Linking and compiling $(subst $(BN)/,,$(NUCLEARIZERPRG)) ... Please stand by ... "
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(NUCLEARIZERCXX) $(NUCLEARIZERSHAREDLIB) $(ALLLIBS) $(GLIBS) $(LIBS) -o $(NUCLEARIZERPRG)

$(DEEPRG): $(NUCLEARIZERSHAREDLIB) $(DEECXX)
	@echo "Linking and compiling $(subst $(BN)/,,$(DEEPRG)) ... Please stand by ... "
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEECXX) $(NUCLEARIZERSHAREDLIB) $(ALLLIBS) $(GLIBS) $(LIBS) -o $(DEEPRG)


#
#----------------------------------------------------------------



