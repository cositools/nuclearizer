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
TOPLEVEL  = $(shell pwd)
IN				= $(TOPLEVEL)/include
LB				= $(MEGALIB)/lib
BN				= $(MEGALIB)/bin


#----------------------------------------------------------------
# Include path/location macros (result of "sh configure")
#

include $(MEGALIB)/config/Makefile.options
include $(MEGALIB)/config/Makefile.config


#----------------------------------------------------------------
# Definitions based on architecture and user options
#

CXXFLAGS += -I$(IN) -I$(MEGALIB)/include -I/opt/local/include

# Name of the program
PROGRAM		= $(BN)/Nuclearizer

# All files:
OBJS			= $(LB)/MNuclearizerMain.o \
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
						$(LB)/MNCTModuleDetectorEffectsEngine.o \
						$(LB)/MNCTModuleEnergyCalibration.o \
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
						$(LB)/MGUIOptionsDetectorEffectsEngine.o \
						$(LB)/MGUIOptionsAspect.o \
						$(LB)/MGUIOptionsEventFilter.o \
						$(LB)/MNCTPreprocessor.o \
#						$(LB)/MNCTModuleEventReconstruction.o \
#						$(LB)/MGUIOptionsEventReconstruction.o \

ALLLIBS 	= -lCommonMisc -lCommonGui -lGeomega -lSivan -lRevan -lRevanGui -L$(MEGALIB)/lib -L$(LB)

# ROOT Mathematical Libraries
#ALLLIBS		+= -lMathCore -lMathMore
ALLLIBS		+= -lMathCore

#----------------------------------------------------------------
# Built-in targets
#

.EXPORT_ALL_VARIABLES: all header sources doc clean
.Phony:                all header sources doc clean


#----------------------------------------------------------------
# Command rules
#

all: $(PROGRAM)
	@$(PROGRAM)

# Only compile, do not start the program
only: $(PROGRAM)

# Clean-up
clean:
	@-rm -f $(OBJS) $(SOBJ)
	@-rm -f $(PROGRAM)
	@-rm -f *~ include/*~ src/*~


#----------------------------------------------------------------
# Explicit rules & dependencies:
#

$(OBJS): $(LB)/%.o: src/%.cxx include/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(PROGRAM): $(OBJS)
	@echo "Linking $(subst $(BN)/,,$(PROGRAM)) ... Please stand by ... $(ALLLIBS)"
	$(LD) $(LDFLAGS) $(OBJS) $(ALLLIBS) $(GLIBS) $(LIBS) -o $(PROGRAM)
	@echo "$(subst $(BN)/,,$(PROGRAM)) created!"


#
#----------------------------------------------------------------



