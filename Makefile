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

TOPLEVEL = $(shell pwd)
IN = $(TOPLEVEL)/include
LB = $(MEGALIB)/lib
BN = $(MEGALIB)/bin


#----------------------------------------------------------------
# Setting up make itself
#

include $(MEGALIB)/config/Makefile.options
include $(MEGALIB)/config/Makefile.config

MAKEFLAGS += --no-builtin-rules

.SUFFIXES:
#.SUFFIXES: .cxx .h .o .so
.PHONY: all n nuclearizer megalib apps clean
.EXPORT_ALL_VARIABLES:
#.NOTPARALLEL: megalib
.SILENT:

#----------------------------------------------------------------
# Definitions based on architecture and user options
#

CMD=""
CXXFLAGS += -I$(IN) -I$(MEGALIB)/include -I/opt/local/include
# Comment this line out if you want to accept warnings
#CXXFLAGS += -Werror -Wno-unused-variable

# Python
ifeq ($(ARCH),macosx)
CXXFLAGS += -I/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
PYTHONLIBS  += -L/System/Library/Frameworks/Python.framework/Versions/2.7/lib/ -lpython2.7
else
CXXFLAGS += `python-config --includes`
PYTHONLIBS += `python-config --libs`
endif

# Names of the program
NUCLEARIZER_PRG = $(BN)/nuclearizer
NUCLEARIZER_CXX_MAIN = src/MAssembly.cxx

# The nuclearizer library
NUCLEARIZER_LIBS = \
$(LB)/magfld.o \
$(LB)/MReadOutAssembly.o \
$(LB)/MNCTArray.o \
$(LB)/MNCTCoincidenceVolume.o \
$(LB)/MNCTDetectorArray.o \
$(LB)/MNCTAspect.o \
$(LB)/MNCTAspectReconstruction.o \
$(LB)/MNCTHit.o \
$(LB)/MNCTHitInVoxel.o \
$(LB)/MNCTMath.o \
$(LB)/MNCTTimeAndCoordinate.o \
$(LB)/MNCTStrip.o \
$(LB)/MNCTStripEnergyDepth.o \
$(LB)/MNCTStripHit.o \
$(LB)/MNCTDetectorResponse.o \
$(LB)/MNCTGuardringHit.o \
$(LB)/MNCTModuleSimulationLoader.o \
$(LB)/MGUIOptionsSimulationLoader.o \
$(LB)/MNCTModuleMeasurementLoader.o \
$(LB)/MNCTModuleMeasurementLoaderROA.o \
$(LB)/MNCTModuleMeasurementLoaderGRIPS2013.o \
$(LB)/MGUIOptionsMeasurementLoader.o \
$(LB)/MNCTBinaryFlightDataParser.o \
$(LB)/MNCTModuleReceiverCOSI2014.o \
$(LB)/MGUIOptionsReceiverCOSI2014.o \
$(LB)/MNCTModuleMeasurementLoaderBinary.o \
$(LB)/MGUIOptionsMeasurementLoaderBinary.o \
$(LB)/MGUIExpoAspectViewer.o \
$(LB)/MGUIExpoEnergyCalibration.o \
$(LB)/MNCTModuleEnergyCalibration.o \
$(LB)/MNCTModuleEnergyCalibrationUniversal.o \
$(LB)/MGUIOptionsEnergyCalibrationUniversal.o \
$(LB)/MNCTModuleEnergyCalibrationLinear.o \
$(LB)/MNCTModuleEnergyCalibrationNonlinear.o \
$(LB)/MNCTInverseCrosstalkCorrection.o \
$(LB)/MNCTModuleCrosstalkCorrection.o \
$(LB)/MGUIOptionsCrosstalkCorrection.o \
$(LB)/MNCTModuleChargeSharingCorrection.o \
$(LB)/MGUIExpoDepthCalibration.o \
$(LB)/MNCTModuleDepthAndStripCalibration.o \
$(LB)/MNCTModuleDepthCalibration.o \
$(LB)/MNCTModuleDepthCalibrationLinearPixel.o \
$(LB)/MGUIOptionsDepthCalibrationLinearPixel.o \
$(LB)/MNCTModuleDepthCalibration3rdPolyPixel.o \
$(LB)/MGUIOptionsDepthCalibration3rdPolyPixel.o \
$(LB)/MGUIExpoStripPairing.o \
$(LB)/MNCTModuleStripPairingGreedy_a.o \
$(LB)/MGUIOptionsStripPairing.o \
$(LB)/MNCTModuleStripPairingGreedy_b.o \
$(LB)/MNCTModuleFlagHits.o \
$(LB)/MNCTModuleAspect.o \
$(LB)/MNCTModuleEventFilter.o \
$(LB)/MGUIOptionsEventSaver.o \
$(LB)/MNCTModuleEventSaver.o \
$(LB)/MGUIOptionsAspect.o \
$(LB)/MGUIOptionsEventFilter.o \
$(LB)/MCalibratorEnergy.o \
$(LB)/MCalibratorEnergyPointwiseLinear.o \

NUCLEARIZER_DEP_FILES := $(NUCLEARIZER_LIBS:.o=.d)
NUCLEARIZER_H_FILES := $(addprefix inc/,$(notdir $(NUCLEARIZER_LIBS:.o=.h)))

FRETALON_DIR          := $(MEGALIB)/src/fretalon/framework
FRETALON_CXX_MAIN     := $(FRETALON_DIR)/src/MAssembly.cxx $(FRETALON_DIR)/src/MReadOutAssembly.cxx
FRETALON_CXX_FILES    := $(wildcard $(FRETALON_DIR)/src/*.cxx)
FRETALON_CXX_FILES    := $(filter-out $(FRETALON_CXX_MAIN),$(FRETALON_CXX_FILES))
FRETALON_H_FILES      := $(wildcard $(FRETALON_DIR)/inc/*.h)
FRETALON_H_FILES      := $(filter-out $(FRETALON_DIR)/inc/MAssembly.h $(FRETALON_DIR)/inc/MReadOutAssembly.h,$(FRETALON_H_FILES))
FRETALON_LIBS         := $(addprefix $(LB)/,$(notdir $(FRETALON_CXX_FILES:.cxx=.o)))
FRETALON_DEP_FILES    := $(FRETALON_LIBS:.o=.d)

# The shared library
NUCLEARIZER_SHARED_LIB = $(LB)/libNuclearizer.$(DLL)

# External libraries
# MEGAlib
ALLLIBS = -lCommonMisc -lCommonGui -lGeomega -lSivan -lRevan -lRevanGui -lSpectralyzeGui -lSpectralyze -lFretalonBase -L$(MEGALIB)/lib -L$(LB) 
# ROOT
ALLLIBS += -lMathCore


#----------------------------------------------------------------
# Command rules
#

all:
	@$(MAKE) $(NUCLEARIZER_PRG)
	@$(MAKE) apps

n: nuclearizer
nuclearizer:
	@$(MAKE) $(NUCLEARIZER_PRG)
	@$(NUCLEARIZER_PRG) $(CMD)

apps:
	@$(MAKE) $(NUCLEARIZER_SHARED_LIB)
	@$(MAKE) -C apps

clean:
	@-rm -f $(MEGALIB)/include/MAssembly.h $(MEGALIB)/include/MReadOutAssembly.h
	@-rm -f $(FRETALON_LIBS) $(FRETALON_DEP_FILES)
	@-rm -f $(NUCLEARIZER_SHARED_LIB) $(NUCLEARIZER_LIBS) $(NUCLEARIZER_DEP_FILES)
	@-rm -f $(NUCLEARIZER_PRG)
	@-rm -f *~ include/*~ src/*~
	@$(MAKE) clean -C apps

#----------------------------------------------------------------
# Explicit rules & dependencies:
#

$(FRETALON_DEP_FILES): $(LB)/%.d: $(FRETALON_DIR)/src/%.cxx
	@echo "Creating dependencies for $(subst $(FRETALON_DIR)/src/,,$<) ..."
	@set -e; rm -f $@; $(CXX) $(DEPFLAGS) $(CXXFLAGS) $< > $@.$$$$; sed -e 's|.*:|$(LB)/$*.o:|' -e 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

$(FRETALON_LIBS): $(LB)/%.o: $(FRETALON_DIR)/src/%.cxx $(FRETALON_DIR)/inc/%.h $(LB)/%.d
	@echo "Compiling $(subst $(FRETALON_DIR)/src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NUCLEARIZER_DEP_FILES): $(LB)/%.d: src/%.cxx
	@echo "Creating dependencies for $(subst src/,,$<) ..."
	@set -e; rm -f $@; $(CXX) $(DEPFLAGS) $(CXXFLAGS) $< > $@.$$$$; sed -e 's|.*:|$(LB)/$*.o:|' -e 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

$(NUCLEARIZER_LIBS): $(LB)/%.o: src/%.cxx include/%.h $(LB)/%.d
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NUCLEARIZER_SHARED_LIB): $(FRETALON_LIBS) $(NUCLEARIZER_LIBS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(NUCLEARIZER_LIBS) $(FRETALON_LIBS) $(GLIBS) $(LIBS) $(PYTHONLIBS) -o $(NUCLEARIZER_SHARED_LIB)

$(NUCLEARIZER_PRG): $(NUCLEARIZER_SHARED_LIB) $(NUCLEARIZER_CXX)
	@echo "Linking and compiling $(subst $(BN)/,,$(NUCLEARIZER_PRG)) ... Please stand by ... "
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(NUCLEARIZER_CXX_MAIN) $(NUCLEARIZER_SHARED_LIB) $(ALLLIBS) $(GLIBS) $(LIBS) $(PYTHONLIBS) -o $(NUCLEARIZER_PRG)

ifneq ($(MAKECMDGOALS),clean)
-include $(NUCLEARIZER_DEP_FILES)
-include $(FRETALON_DEP_FILES)
endif

#
#----------------------------------------------------------------
