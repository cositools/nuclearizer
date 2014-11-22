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
CXXFLAGS += -Werror -Wno-unused-variable

# Python
ifeq ($(ARCH),macosx)
CXXFLAGS += -I/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
PYTHONLIBS  += -L/System/Library/Frameworks/Python.framework/Versions/2.7/lib/ -lpython2.7
else
CXXFLAGS += `python-config --includes`
PYTHONLIBS += `python-config --libs`
endif

# Names of the program
NUCLEARIZERPRG = $(BN)/nuclearizer
NUCLEARIZERCXX = src/MAssembly.cxx

# The nuclearizer library
NUCLEARIZERLIBS = \
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
$(LB)/MNCTModuleDepthCalibration3rdPolyPixel.o \
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
# Command rules
#

all: 
	@$(MAKE) $(NUCLEARIZERPRG)
	@$(MAKE) apps

n: nuclearizer
nuclearizer:
	@$(MAKE) $(NUCLEARIZERPRG)
	@$(NUCLEARIZERPRG) $(CMD)

apps: 
	@$(MAKE) $(NUCLEARIZERSHAREDLIB)
	@$(MAKE) -C apps

clean:
	@$(MAKE) clean_fretalonframework -C $(MEGALIB)/src
	@-rm -f $(NUCLEARIZERSHAREDLIB) $(NUCLEARIZERLIBS)
	@-rm -f $(NUCLEARIZERPRG)
	@-rm -f *~ include/*~ src/*~
	@$(MAKE) clean -C apps

#----------------------------------------------------------------
# Explicit rules & dependencies:
#

$(FRETALONLIBS): $(LB)/%.o: $(FRETALONDIR)/src/%.cxx $(FRETALONDIR)/inc/%.h
	@echo "Compiling $(subst $(FRETALONDIR)/src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NUCLEARIZERLIBS): $(LB)/%.o: src/%.cxx include/%.h
	@echo "Compiling $(subst src/,,$<) ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@
 
$(NUCLEARIZERSHAREDLIB): $(FRETALONLIBS) $(NUCLEARIZERLIBS)
	@echo "Linking $(subst $(LB)/,,$@) ..."
	@$(LD) $(LDFLAGS) $(SOFLAGS) $(NUCLEARIZERLIBS) $(FRETALONLIBS) $(GLIBS) $(LIBS) $(PYTHONLIBS) -o $(NUCLEARIZERSHAREDLIB)

$(NUCLEARIZERPRG): $(NUCLEARIZERSHAREDLIB) $(NUCLEARIZERCXX)
	@echo "Linking and compiling $(subst $(BN)/,,$(NUCLEARIZERPRG)) ... Please stand by ... "
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(NUCLEARIZERCXX) $(NUCLEARIZERSHAREDLIB) $(ALLLIBS) $(GLIBS) $(LIBS) $(PYTHONLIBS) -o $(NUCLEARIZERPRG)


#
#----------------------------------------------------------------
