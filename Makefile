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

CMD=""
CXXFLAGS += -I$(IN) -I$(MEGALIB)/include -I/opt/local/include
# Python

ifeq ($(ARCH),macosx)
CXXFLAGS += -I/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
PYTHONLIBS  += -L/System/Library/Frameworks/Python.framework/Versions/2.7/lib/ -lpython2.7
else
CXXFLAGS += `python-config --includes`
PYTHONLIBS += `python-config --libs`
endif

# Names of the programs
NUCLEARIZERPRG = $(BN)/nuclearizer
NUCLEARIZERCXX = src/MAssembly.cxx

DEEPRG = $(BN)/deecosi
DEECXX = src/MNCTDetectorEffectsEngineCOSI.cxx

ALLPROGRAMS = $(NUCLEARIZERPRG) $(DEEPRG)

# The nuclearizer library
NUCLEARIZERLIB = \
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
$(LB)/MNCTFile.o \
$(LB)/MNCTFileEventsDat.o \
$(LB)/MNCTEventBuffer.o \
$(LB)/MNCTModuleSimulationLoader.o \
$(LB)/MGUIOptionsSimulationLoader.o \
$(LB)/MNCTModuleMeasurementLoader.o \
$(LB)/MNCTModuleMeasurementLoaderROA.o \
$(LB)/MNCTModuleMeasurementLoaderGRIPS2013.o \
$(LB)/MNCTModuleMeasurementLoaderNCT2009.o \
$(LB)/MGUIOptionsMeasurementLoader.o \
$(LB)/MNCTBinaryFlightDataParser.o \
$(LB)/MNCTModuleReceiverCOSI2014.o \
$(LB)/MGUIOptionsReceiverCOSI2014.o \
$(LB)/MNCTModuleMeasurementLoaderBinary.o \
$(LB)/MGUIOptionsMeasurementLoaderBinary.o \
$(LB)/MGUIExpoEnergyCalibration.o \
$(LB)/MNCTModuleEnergyCalibration.o \
$(LB)/MNCTModuleEnergyCalibrationUniversal.o \
$(LB)/MGUIOptionsEnergyCalibrationUniversal.o \
$(LB)/MNCTModuleEnergyCalibrationLinear.o \
$(LB)/MNCTModuleEnergyCalibrationNonlinear.o \
$(LB)/MNCTInverseCrosstalkCorrection.o \
$(LB)/MNCTModuleCrosstalkCorrection.o \
$(LB)/MNCTModuleChargeSharingCorrection.o \
$(LB)/MGUIExpoDepthCalibration.o \
$(LB)/MNCTModuleDepthAndStripCalibration.o \
$(LB)/MNCTModuleDepthCalibration.o \
$(LB)/MNCTModuleDepthCalibrationLinearStrip.o \
$(LB)/MNCTModuleDepthCalibrationLinearPixel.o \
$(LB)/MNCTModuleDepthCalibration3rdPolyPixel.o \
$(LB)/MGUIExpoStripPairing.o \
$(LB)/MNCTModuleStripPairing.o \
$(LB)/MNCTModuleStripPairingGreedy.o \
$(LB)/MNCTModuleStripPairingGreedy_a.o \
$(LB)/MNCTModuleStripPairingGreedy_b.o \
$(LB)/MNCTModuleAspect.o \
$(LB)/MNCTModuleEventFilter.o \
$(LB)/MNCTModuleDumpEvent.o \
$(LB)/MGUIOptionsEventSaver.o \
$(LB)/MNCTModuleEventSaver.o \
$(LB)/MGUIOptionsAspect.o \
$(LB)/MGUIOptionsEventFilter.o \
$(LB)/MNCTPreprocessor.o \
$(LB)/MCalibratorEnergy.o \
$(LB)/MCalibratorEnergyPointwiseLinear.o \

FRETALONDIR         := $(MEGALIB)/src/fretalon/framework
FRETALON_CXX_MAIN   := $(FRETALONDIR)/src/MAssembly.cxx $(FRETALONDIR)/src/MReadOutAssembly.cxx
FRETALON_CXX_FILES  := $(wildcard $(FRETALONDIR)/src/*.cxx)
FRETALON_CXX_FILES  := $(filter-out $(FRETALON_CXX_MAIN),$(FRETALON_CXX_FILES))
FRETALON_H_FILES    := $(wildcard $(FRETALONDIR)/inc/*.h)
FRETALON_H_FILES    := $(filter-out $(FRETALONDIR)/inc/MAssembly.h $(FRETALONDIR)/inc/MReadOutAssembly.h,$(FRETALON_H_FILES))
FRETALONLIBS        := $(addprefix $(LB)/,$(notdir $(FRETALON_CXX_FILES:.cxx=.o)))


# The shared library
NUCLEARIZERSHAREDLIB = $(LB)/libNuclearizer.$(DLL)

# External libraries
# MEGAlib
ALLLIBS = -lCommonMisc -lCommonGui -lGeomega -lSivan -lRevan -lRevanGui -lSpectralyzeGui -lSpectralyze -lFretalonBase -L$(MEGALIB)/lib -L$(LB) 
# ROOT
ALLLIBS += -lMathCore

#----------------------------------------------------------------
# Built-in targets$(PYTHONLIBS)
#

.EXPORT_ALL_VARIABLES: all header sources doc clean
.Phony:                all header sources doc clean


#----------------------------------------------------------------
# Command rules
#

# Compile all libraries and programs
all: $(ALLPROGRAMS)

# Compile all libraries and programs
n: $(ALLPROGRAMS)
	@$(NUCLEARIZERPRG) $(CMD)
	@$(NUCLEARIZERPRG) $(CMD)

nuclearizer: $(ALLPROGRAMS)
	@$(NUCLEARIZERPRG) $(CMD)

# Clean-up
clean:
	@-rm -f $(NUCLEARIZERO) $(DEEO) $(NUCLEARIZERSHAREDLIB) $(NUCLEARIZERLIB)
	@-rm -f $(NUCLEARIZERPRG) $(DEEPRG)
	@-rm -f *~ include/*~ src/*~

clean_framework:
	@$(MAKE) clean_fre -C $(MEGALIB)/src
	
#----------------------------------------------------------------
# Explicit rules & dependencies:
#

$(FRETALONLIBS): $(LB)/%.o: $(FRETALONDIR)/src/%.cxx $(FRETALONDIR)/inc/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NUCLEARIZERLIB): $(LB)/%.o: src/%.cxx include/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NUCLEARIZERSHAREDLIB): $(FRETALONLIBS) $(NUCLEARIZERLIB) 
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(NUCLEARIZERLIB) $(FRETALONLIBS) $(GLIBS) $(LIBS) $(PYTHONLIBS) -o $(NUCLEARIZERSHAREDLIB)

$(NUCLEARIZERPRG): $(NUCLEARIZERSHAREDLIB) $(NUCLEARIZERCXX)
	@echo $(FRETALONLIB)
	@echo "Linking and compiling $(subst $(BN)/,,$(NUCLEARIZERPRG)) ... Please stand by ... "
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(NUCLEARIZERCXX) $(NUCLEARIZERSHAREDLIB) $(ALLLIBS) $(GLIBS) $(LIBS) $(PYTHONLIBS) -o $(NUCLEARIZERPRG)

$(DEEPRG): $(NUCLEARIZERSHAREDLIB) $(DEECXX)
	@echo "Linking and compiling $(subst $(BN)/,,$(DEEPRG)) ... Please stand by ... "
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(DEECXX) $(NUCLEARIZERSHAREDLIB) $(ALLLIBS) $(GLIBS) $(LIBS) $(PYTHONLIBS) -o $(DEEPRG)


#
#----------------------------------------------------------------



