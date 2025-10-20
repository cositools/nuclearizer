#----------------------------------------------------------------
#
#  Makefile for Nuclearizer
#
#  Top level makefile: definitions, paths and command switch
#
#  Author: Andreas Zoglauer
#
#----------------------------------------------------------------


#-------------------------------------------------------------------------------
# Definitions

CXX_FILES := $(wildcard *.cxx)
CXX_FILES := $(filter-out MAspectTester.cxx,$(CXX_FILES))
PRGS := $(CXX_FILES:%.cxx=%)
PRGS := $(addprefix $(BN)/,$(PRGS))


#-------------------------------------------------------------------------------
# Directives

all: $(PRGS)

clean:
	@rm -f $(PRGS)

#-------------------------------------------------------------------------------
# Rules:

$(PRGS): $(BN)/%: %.cxx
	@echo "Linking and compiling $(subst $(BN)/,,$@) ... Please stand by ... "
	@$(CXX) $(CXXFLAGS) -Wno-vla-cxx-extension $(LDFLAGS) $(subst $(BN)/,,$@).cxx $(NUCLEARIZER_SHARED_LIB) $(ALLLIBS) $(GLIBS) $(LIBS) $(PYTHONLIBS) -o $(@)


#
#-------------------------------------------------------------------------------


