// generation of pp events @ 13.6 TeV using Pythia 8
// Usage -  ./generate [file.cmnd] [output_file.root]
// if the above names are not given, settings are read from generate.cmnd and saved to events.root

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Pythia8/Pythia.h"
#include "TFile.h"
#include "TNamed.h"
#include "TTree.h"

namespace {
// function for reading Pythia settings for generation from generate.cmnd
std::string readWholeFile(const std::string& path) {
  std::ifstream in(path);
  if (!in) return {}; // empty when opening fails
  std::stringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}
// help in case of wrong usage
void printUsage(const char* prog) {
  std::cerr << "Syntax error, correct usage - \n" << prog
            << " [pythia_settings.cmnd] [output_file.root]\n"
            << "default: generate.cmnd, events.root\n";
}

}

int main(int argc, char* argv[]) {
  if (argc > 3) { // too many parameters given
    printUsage(argv[0]);
    return 1;
  }
  // if not given - default files
  const std::string cmndFile = (argc > 1) ? argv[1] : "generate.cmnd";
  const std::string outFile  = (argc > 2) ? argv[2] : "events.root";

  // reading pythia settings from .cmnd
  const std::string cmndText = readWholeFile(cmndFile);
  if (cmndText.empty()) {
    std::cerr << "error reading generator settings " << cmndFile << "\n";
    return 1;
  }

  //
  //
  //

  Pythia8::Pythia pythia;
  //Loading generator settings
  if (!pythia.readFile(cmndFile)) {
    std::cerr << "invalid generator settings " << cmndFile << "\n";
    return 1;
  }
  const int nEvents = pythia.mode("Main:numberOfEvents");
  const int nAbortMax = pythia.mode("Main:timesAllowErrors");


  // Pythia initialization
  if (!pythia.init()) {
    std::cerr << "error during initialization\n";
    return 1;
  }

  // creating the output file
  TFile out(outFile.c_str(), "RECREATE");
  if (out.IsZombie()) {
    std::cerr << "cannot create file " << outFile << "\n";
    return 1;
  }

  // we create a tree, to which we will later add final particles and their properties
  auto* tree = new TTree("events", "Final particles, pp @ 13.6 TeV, minimum bias (SoftQCD)");

  std::vector<int> vPdg, vCharge;
  std::vector<float> vPt, vEta, vPhi;

  tree->Branch("pdg", &vPdg);
  tree->Branch("charge", &vCharge);
  tree->Branch("pt", &vPt);
  tree->Branch("eta", &vEta);
  tree->Branch("phi", &vPhi);

  //
  //
  //

  int nAccepted = 0;
  int nAborted  = 0;

  // we save events until reaching the number set in the generator
  // or exceeding the set error limit
  while (nAccepted < nEvents) {
    if (!pythia.next()) { // at this point one event is already generated, many particles are created (if it returned True)
      ++nAborted; //error tracking, break when the limit is exceeded
      if (nAborted > nAbortMax) {
        std::cerr << "allowed error limit exceeded\n";
        break;
      }
      continue;
    }

    vPdg.clear();
    vPt.clear();
    vPhi.clear();
    vEta.clear();
    vCharge.clear();


    const Pythia8::Event& event = pythia.event;  // particles created after pythia.next
    for (int i = 0; i < event.size(); ++i) {
      const Pythia8::Particle& p = event[i];
      if (!p.isFinal()) continue;  // only final particles

      const int q = static_cast<int>(std::lround(p.charge())); //changing double charge to int

      vPdg.push_back(p.id());
      vCharge.push_back(q);
      vPt.push_back(static_cast<float>(p.pT()));
      vEta.push_back(static_cast<float>(p.eta()));
      vPhi.push_back(static_cast<float>(p.phi()));
      // the (-pi,pi] state is saved here, this will need to be corrected later
    }

    tree->Fill(); //saving particles from 1 event to the tree
    ++nAccepted;
  }

  //
  //
  //
  pythia.stat();

  TNamed config("config", cmndText.c_str());

  tree->Write("", TObject::kOverwrite);
  config.Write();
  out.Close();

  return (nAccepted == nEvents) ? 0 : 2;
}
