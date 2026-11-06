// histograms of final particles
// Usage - ./analysis [input_file.root] [output_file.root]
// if the above names are not given, data is read from events.root and analysis.root is created

#include <iostream>
#include <string>
#include "TFile.h"
#include "TH1D.h"
#include "TMath.h"
#include "TParameter.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

namespace {
// help in case of wrong usage
void printUsage(const char* prog) {
  std::cerr << "Syntax error, correct usage - \n" << prog
            << " [input_file.root] [output_file.root]\n"
            << "default: events.root, analysis.root\n";
}
}

int main(int argc, char* argv[]) {
  if (argc > 3) { // too many parameters given
    printUsage(argv[0]);
    return 1;
  }
  // if not given - default files (e.g. when a different name was given in generate.cc)
  const std::string inFile  = (argc > 1) ? argv[1] : "events.root";
  const std::string outFile = (argc > 2) ? argv[2] : "analysis.root";

  //
  //
  //

  TFile in(inFile.c_str(), "READ"); //reading the events.root file
  if (in.IsZombie()) {
    std::cerr << "cannot read file " << inFile << "\n";
    return 1;
  }
  TTree* tree = in.Get<TTree>("events"); //loading the tree from the events.root file
  if (!tree) {
    std::cerr << "tree not found in " << inFile << "\n";
    return 1;
  }

  TFile out(outFile.c_str(), "RECREATE"); //creating the analysis.root file
  if (out.IsZombie()) {
    std::cerr << "cannot create file " << outFile << "\n";
    return 1;
  }

  const int nCat = 8;
  const char* names[nCat] = {"all", "ch", "pip", "pim", "Kp", "Km", "p", "pbar"};
  const char* labels[nCat] = {"all (final particles)", "charged", "#pi^{+}", "#pi^{-}", "K^{+}", "K^{-}", "p", "#bar{p}"};
  const int pdgCode[nCat] = {0, 0, 211, -211, 321, -321, 2212, -2212};

  // arrays which I will fill with histograms
  TH1D* hN[nCat];
  TH1D* hPt[nCat];
  TH1D* hEta[nCat];
  TH1D* hPhi[nCat];

  // bins and range set in the task
  for (int c = 0; c < nCat; ++c) {
    hN[c]   = new TH1D(Form("hN_%s", names[c]),
                       Form("Multiplicity: %s;Multiplicity N;Events", labels[c]),
                       400, 0., 400.);
    hPt[c]  = new TH1D(Form("hPt_%s", names[c]),
                       Form("Transverse momentum: %s;p_{T} [GeV];Particles", labels[c]),
                       100, 0., 10.);
    hEta[c] = new TH1D(Form("hEta_%s", names[c]),
                       Form("Pseudorapidity: %s;#eta;Particles", labels[c]),
                       100, -5., 5.);
    hPhi[c] = new TH1D(Form("hPhi_%s", names[c]),
                       Form("Azimuthal angle: %s;#phi [rad];Particles", labels[c]),
                       64, -TMath::Pi(), TMath::Pi());
  }

  TTreeReader reader(tree);
  TTreeReaderArray<int> pdg(reader, "pdg");
  TTreeReaderArray<int> charge(reader, "charge");
  TTreeReaderArray<float> pt(reader, "pt");
  TTreeReaderArray<float> eta(reader, "eta");
  TTreeReaderArray<float> phi(reader, "phi");

  int nRead = 0;  // number of events actually read
  while (reader.Next()) {
    ++nRead;
    int count[nCat] = {0};

    for (size_t i = 0; i < pdg.GetSize(); ++i) {
      auto fill = [&](int c) {
        ++count[c];
        hPt[c]->Fill(pt[i]);
        hEta[c]->Fill(eta[i]);
        // Pythia gives phi in (-pi, pi], the same range as the histogram
        hPhi[c]->Fill(phi[i]);
      };

      fill(0); // all - every particle goes here
      if (charge[i] != 0) fill(1); // charged
      for (int c = 2; c < nCat; ++c) {  // pip, pim, Kp, Km, p, pbar
        if (pdg[i] == pdgCode[c]) {
          fill(c);
          break;
        }
      }
    }

    for (int c = 0; c < nCat; ++c) {
      hN[c]->Fill(count[c]);
    }
  }

  if (nRead != tree->GetEntries()) { // not all events were read
    std::cerr << "error while loading, not all entries were loaded\n";
    return 1;
  }

  out.cd();
  TParameter<int> nEvents("nEvents", nRead);
  nEvents.Write();
  out.Write();
  out.Close();

  return 0;
}
