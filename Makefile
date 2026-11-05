# =============================================================================
#
#    make task -> entire pipeline
#
#    make -> compile programs
#    make events -> generate 100 000 pp events
#    make hist -> make histograms based on events.root
#    make plots -> create .png and .pdf plots based on analysis.root
#    make clean -> removal of compiled programs
# =============================================================================

CXX := g++
ROOT_CFLAGS := $(shell root-config --cflags)
ROOT_LIBS := $(shell root-config --libs)
PYTHIA_INC := -I$(shell pythia8-config --includedir)
PYTHIA_LIBS := $(shell pythia8-config --libs)

CXXFLAGS := -O2 -Wall -Wextra $(ROOT_CFLAGS)

PROGRAMS := generate analysis

all: $(PROGRAMS)

generate: generate.cc
	$(CXX) $(CXXFLAGS) $(PYTHIA_INC) $< -o $@ $(PYTHIA_LIBS) $(ROOT_LIBS)

analysis: analysis.cc
	$(CXX) $(CXXFLAGS) $< -o $@ $(ROOT_LIBS)

events: generate generate.cmnd
	./generate generate.cmnd events.root

hist: analysis
	./analysis events.root analysis.root

plots: draw_plots.C
	root -l -b -q 'draw_plots.C("analysis.root", "plots")'

task: events hist plots

clean:
	rm -f $(PROGRAMS)

.PHONY: all events hist plots task clean
