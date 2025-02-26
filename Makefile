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

# Names of the program
NUCLEARIZER_PRG = $(BN)/nuclearizer
NUCLEARIZER_CXX_MAIN = src/MNuclearizerMain.cxx

# The nuclearizer library
NUCLEARIZER_LIBS = \
$(LB)/magfld.o \
$(LB)/MAssembly.o \
$(LB)/MReadOutAssembly.o \
$(LB)/MAspect.o \
$(LB)/MAspectPacket.o \
$(LB)/MAspectReconstruction.o \
$(LB)/MHit.o \
$(LB)/MTimeAndCoordinate.o \
$(LB)/MStripHit.o \
$(LB)/MGuardringHit.o \
$(LB)/MDetectorEffectsEngineBalloon.o \
$(LB)/MModuleLoaderSimulationsBalloon.o \
$(LB)/MDetectorEffectsEngineSMEX.o \
$(LB)/MModuleLoaderSimulationsSMEX.o \
$(LB)/MGUIOptionsLoaderSimulations.o \
$(LB)/MModuleLoaderMeasurements.o \
$(LB)/MModuleLoaderMeasurementsROA.o \
$(LB)/MGUIOptionsLoaderMeasurements.o \
$(LB)/MBinaryFlightDataParser.o \
$(LB)/MModuleReceiverBalloon.o \
$(LB)/MGUIOptionsReceiverBalloon.o \
$(LB)/MGUIExpoReceiver.o \
$(LB)/MModuleLoaderMeasurementsBinary.o \
$(LB)/MGUIOptionsLoaderMeasurementsBinary.o \
$(LB)/MGUIExpoAspectViewer.o \
$(LB)/MGUIExpoEnergyCalibration.o \
$(LB)/MModuleEnergyCalibration.o \
$(LB)/MModuleEnergyCalibrationUniversal.o \
$(LB)/MGUIOptionsEnergyCalibrationUniversal.o \
$(LB)/MInverseCrosstalkCorrection.o \
$(LB)/MModuleCrosstalkCorrection.o \
$(LB)/MGUIOptionsCrosstalkCorrection.o \
$(LB)/MModuleChargeSharingCorrection.o \
$(LB)/MGUIExpoDepthCalibration.o \
$(LB)/MGUIExpoDepthCalibration2024.o \
$(LB)/MModuleDepthCalibration.o \
$(LB)/MGUIOptionsDepthCalibration.o \
$(LB)/MModuleDepthCalibration2024.o \
$(LB)/MGUIOptionsDepthCalibration2024.o \
$(LB)/MGUIExpoStripPairing.o \
$(LB)/MGUIExpoStripPairingHits.o \
$(LB)/MGUIExpoStripPairingStripHits.o \
$(LB)/MModuleStripPairingGreedy.o \
$(LB)/MGUIOptionsStripPairing.o \
$(LB)/MModuleStripPairingChiSquare.o \
$(LB)/MGUIOptionsEventSaver.o \
$(LB)/MModuleEventSaver.o \
$(LB)/MGUIOptionsEventFilter.o \
$(LB)/MModuleEventFilter.o \
$(LB)/MCalibratorEnergy.o \
$(LB)/MCalibratorEnergyPointwiseLinear.o \
$(LB)/MDepthCalibrator.o \
$(LB)/GCUSettingsParser.o \
$(LB)/MTIRecord.o \
$(LB)/GCUHousekeepingParser.o \
$(LB)/LivetimeParser.o \
$(LB)/MDepthCalibratorB.o \
$(LB)/MModuleDepthCalibrationB.o \
$(LB)/MGUIOptionsDepthCalibrationB.o \
$(LB)/MGUIOptionsResponseGenerator.o \
$(LB)/MModuleResponseGenerator.o \
$(LB)/MModuleDiagnostics.o \
$(LB)/MModuleDiagnosticsEnergyPerStrip.o \
$(LB)/MGUIExpoDiagnosticsEnergyPerStrip.o \
$(LB)/MGUIExpoDiagnostics.o \
$(LB)/MModuleTACcut.o \
$(LB)/MGUIExpoTACcut.o \
$(LB)/MGUIOptionsTACcut.o \



NUCLEARIZER_DEP_FILES := $(NUCLEARIZER_LIBS:.o=.d)
NUCLEARIZER_H_FILES := $(addprefix $(NUCLEARIZER)/include/,$(notdir $(NUCLEARIZER_LIBS:.o=.h)))


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

# The main program
NUCLEARIZER_CXX_MAIN := $(NUCLEARIZER)/src/MNuclearizerMain.cxx

# External libraries
# MEGAlib
ALLLIBS = -L$(LB) -lResponseCreator -lFretalonBase -lSivan -lRevanGui -lRevan -lMimrec -lGeomega -lSpectralyzeGui -lSpectralyze -lCommonMisc -lCommonGui -L$(MEGALIB)/lib -L$(LB) 
# ROOT
ALLLIBS += -lMathCore



NUCLEARIZER_DICT_NAME=Nuclearizer_Dictionary
NUCLEARIZER_DICT=$(LB)/$(NUCLEARIZER_DICT_NAME).cxx
NUCLEARIZER_DICT_LIB=$(LB)/$(NUCLEARIZER_DICT_NAME).o
NUCLEARIZER_LINKDEF=$(LB)/$(NUCLEARIZER_DICT_NAME)_LinkDef.h
NUCLEARIZER_ROOTMAP=$(LB)/libNuclearizer.rootmap
NUCLEARIZER_ROOTPCM=libNuclearizer_rdict.pcm

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
	@-rm -f $(NUCLEARIZER_DICT) $(NUCLEARIZER_LINKDEF) $(NUCLEARIZER_ROOTMAP) $(NUCLEARIZER_ROOTPCM)
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

$(NUCLEARIZER_DICT): $(FRETALON_H_FILES) $(NUCLEARIZER_H_FILES)
	@echo "Generating LinkDef ..."
	@$(BN)/generatelinkdef -o $(NUCLEARIZER_LINKDEF) -i $(NUCLEARIZER_H_FILES) $(FRETALON_H_FILES)
	@echo "Generating dictionary ..."
	@rootcling -f $@ -I$(IN) -I$(MEGALIB)/include -D___CLING___ -rmf $(NUCLEARIZER_ROOTMAP) -s libNuclearizer -c  $(NUCLEARIZER_H_FILES) $(FRETALON_H_FILES) $(NUCLEARIZER_LINKDEF)
	@mv $(NUCLEARIZER_ROOTPCM) $(LB)

$(NUCLEARIZER_DICT_LIB): $(NUCLEARIZER_DICT)
	@echo "Compiling dictionary ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NUCLEARIZER_SHARED_LIB): $(FRETALON_LIBS) $(NUCLEARIZER_LIBS) $(NUCLEARIZER_DICT_LIB)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(NUCLEARIZER_DICT_LIB) $(NUCLEARIZER_LIBS) $(FRETALON_LIBS) $(GLIBS) $(LIBS) -o $(NUCLEARIZER_SHARED_LIB)

$(NUCLEARIZER_PRG): $(NUCLEARIZER_SHARED_LIB) $(NUCLEARIZER_CXX_MAIN)
	@echo "Linking and compiling $(subst $(BN)/,,$(NUCLEARIZER_PRG)) ... Please stand by ... "
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(NUCLEARIZER_CXX_MAIN) $(NUCLEARIZER_SHARED_LIB) $(ALLLIBS) $(GLIBS) $(LIBS) -o $(NUCLEARIZER_PRG)

ifneq ($(MAKECMDGOALS),clean)
-include $(NUCLEARIZER_DEP_FILES)
-include $(FRETALON_DEP_FILES)
endif

#
#----------------------------------------------------------------
