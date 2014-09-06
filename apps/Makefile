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
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) $(subst $(BN)/,,$@).cxx $(NUCLEARIZERSHAREDLIB) $(ALLLIBS) $(GLIBS) $(LIBS) $(PYTHONLIBS) -o $(@)


#
#-------------------------------------------------------------------------------


