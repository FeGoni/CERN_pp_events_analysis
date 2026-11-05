// creating plots from analysis.root
// Usage -> root -l -b -q 'draw_plots.C("[file_with_analysis.root]", "[catalog_for_plots]")'
// if the above names are not given, default are analysis.root, plots

#include <algorithm>
#include <iostream>
#include "TCanvas.h"
#include "TError.h"
#include "TFile.h"
#include "TH1.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TParameter.h"
#include "TString.h"
#include "TStyle.h"
#include "TSystem.h"

void draw_plots(const char* inFile = "analysis.root", const char* outDir = "plots") {
  TFile* f = TFile::Open(inFile, "READ");
  if (!f) { // nullpointer or invalid file
    std::cerr << "cannot read file " << inFile << "\n";
    return;
  }
  gSystem->mkdir(outDir, kTRUE); //creating the folder for plots

  int nEvents = 0;
  TParameter<int>* param = f->Get<TParameter<int>>("nEvents");
  if (param != nullptr) {
    nEvents = param->GetVal();
  } //reading the number of events from analysis.root, and assigning 0 in case of an error


  //settings of all histograms drawn from this moment on
  gStyle->SetOptStat("emrou"); //box on each plot showing: mean, std, number of events, overflow and underflow
  gStyle->SetStatX(0.94);
  gStyle->SetStatY(0.86); 
  gStyle->SetPadLeftMargin(0.13);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetPadTopMargin(0.13); // space above the plot for the text with information about the sample
  gStyle->SetTitleOffset(1.3, "Y");
  gErrorIgnoreLevel = kWarning;

  const int nCat = 8;
  const char* names[nCat]  = {"all", "ch", "pip", "pim", "Kp", "Km", "p", "pbar"};
  const char* labels[nCat] = {"all (final particles)", "all charged", "#pi^{+}", "#pi^{-}", "K^{+}", "K^{-}", "p", "#bar{p}"};
  const int colors[nCat]   = {kOrange + 7, kAzure + 2, kRed + 1, kBlue + 1, kGreen + 2, kViolet + 1, kBlack, kGray + 2,};

  const int nVar = 4;
  const char* prefix[nVar] = {"hN", "hPt", "hEta", "hPhi"};
  const char* titles[nVar] = {"Multiplicity", "Transverse momentum", "Pseudorapidity",
                              "Azimuthal angle"};
  const char* xTitle[nVar] = {"Multiplicity N", "p_{T} [GeV]", "#eta", "#phi [rad]"};
  const char* yTitle[nVar] = {"Events", "Particles / 0.1 GeV", "Particles / 0.1",
                              "Particles / (#pi/64 rad)"};
  const bool logY[nVar]    = {true, true, false, false};

  // info about generator
  const TString info = Form("PYTHIA 8, pp #sqrt{s} = 13.6 TeV, %d events", nEvents);
  TLatex text;
  text.SetNDC();
  text.SetTextFont(42);
  text.SetTextSize(0.03);
  text.SetTextAlign(31); 
  const double infoX = 0.95; 
  const double infoY = 0.885; 

  // common style of the histogram of category c for variable v
  auto style = [&](TH1* h, int c, int v) {
    h->GetXaxis()->SetTitle(xTitle[v]);
    h->GetYaxis()->SetTitle(yTitle[v]);
    h->SetLineColor(colors[c]);
    h->SetLineStyle(kSolid);
    h->SetLineWidth(2);
    h->SetMarkerColor(colors[c]);
    h->SetMarkerStyle(kFullCircle);
    h->SetMarkerSize(0.6);
  };

  // cutting the X axis on the plot to the data range
  // so that e.g. the kaon plots are not mostly empty. The histogram in the file stays 0-400
  auto multXMax = [](TH1* h1, TH1* h2) {
    int last = h1->FindLastBinAbove(0);
    if (h2) last = std::max(last, h2->FindLastBinAbove(0));
    const int bin = std::min(last + std::max(2, last / 5), h1->GetNbinsX());
    return h1->GetXaxis()->GetBinUpEdge(bin);
  };

  // one big PDF with all plots: "[" opens the file, "]" closes it, each plot in between is one page
  const TString allPdf = Form("%s/all_plots.pdf", outDir);
  TCanvas cvAll("cvAll", "", 800, 600);
  cvAll.Print(allPdf + "[");

  for (int v = 0; v < nVar; ++v) { //we go through all categories and variables
    for (int c = 0; c < nCat; ++c) {
      TH1* h = f->Get<TH1>(Form("%s_%s", prefix[v], names[c])); // getting the histogram and creating its proper name
      if (!h) continue;

      // variables with a logarithmic scale are drawn twice: log, then linear (file name ending with _lin)
      const int nScales = logY[v] ? 2 : 1;
      for (int s = 0; s < nScales; ++s) {
        const bool useLog = logY[v] && s == 0;
        const bool extraLinear = logY[v] && s == 1;
        const TString plotName = extraLinear ? Form("%s_lin", h->GetName()) : h->GetName();

        TCanvas cv("cv", "", 800, 600);
        cv.SetLogy(useLog); // logarithmic scale where it makes sense (set above)
        h->SetTitle(Form("%s: %s%s", titles[v], labels[c], extraLinear ? " (linear scale)" : ""));
        style(h, c, v);
        h->SetStats(true);

        const double ymax = h->GetBinContent(h->GetMaximumBin());
        if (useLog) {
          h->SetMinimum(0.3);
          h->SetMaximum(50 * ymax);
        }
        else {
          h->SetMinimum(0);
          h->SetMaximum(1.4 * ymax);
        }

        if (v == 0) { // For multiplicities
          TH1F* frame = cv.DrawFrame(0, h->GetMinimum(), multXMax(h, nullptr), h->GetMaximum(), Form("%s;%s;%s",
            h->GetTitle(), xTitle[v], yTitle[v]));
          if (extraLinear) frame->GetYaxis()->SetTitleOffset(1.75); // 5-digit axis labels on the linear scale
          h->Draw("HIST SAMES"); // draw on the DrawFrame frame, keep the stats box
        } else {
          h->Draw("HIST");
        }

        text.DrawLatex(infoX, infoY, info);

        cv.Print(Form("%s/%s.png", outDir, plotName.Data()));
        cv.Print(Form("%s/%s.pdf", outDir, plotName.Data()));
        cv.Print(allPdf, Form("Title:%s", plotName.Data()));
      }
    }
  }

  // 
  // Addition: comparing particles and antiparticles
  //
  const char* pairNames[4] = {"all_charged", "pions", "kaons", "protons"};

  for (int v = 0; v < nVar; ++v) {
    for (int k = 0; k < 4; ++k) {
      const int c1 = 2 * k;
      const int c2 = 2 * k + 1;
      TH1* h1 = f->Get<TH1>(Form("%s_%s", prefix[v], names[c1]));
      TH1* h2 = f->Get<TH1>(Form("%s_%s", prefix[v], names[c2]));
      if (!h1 || !h2) continue;

      TCanvas cv("cv", "", 800, 600);
      cv.SetLogy(logY[v]);

      h1->SetTitle(Form("%s: %s vs %s", titles[v], labels[c1], labels[c2]));
      style(h1, c1, v);
      style(h2, c2, v);
      h1->SetStats(false);
      h2->SetStats(false);
      h2->SetLineStyle(2);
      h2->SetMarkerStyle(kOpenCircle);

      const double ymax = std::max(h1->GetBinContent(h1->GetMaximumBin()), h2->GetBinContent(h2->GetMaximumBin()));
      if (logY[v]) { 
        h1->SetMinimum(0.3); h1->SetMaximum(50 * ymax);
        h2->SetMinimum(0.3); h2->SetMaximum(50 * ymax); // h2 keeps its axis limits from the linear plot otherwise 
      } else { 
        h1->SetMinimum(0); h1->SetMaximum(1.4 * ymax); 
      }

      // all vs charged: points, because N_ch has every other bin empty and the line would be unreadable
      const bool points = (v == 0 && k == 0);

      if (v == 0) {
        cv.DrawFrame(0, h1->GetMinimum(), multXMax(h1, h2), h1->GetMaximum(),
                     Form("%s;%s;%s", h1->GetTitle(), xTitle[v], yTitle[v]));
        h1->Draw(points ? "P SAME" : "HIST SAME");
      } else {
        h1->Draw("HIST");
      }
      h2->Draw(points ? "P SAME" : "HIST SAME");

      TLegend leg(0.60, 0.70, 0.92, 0.82);
      leg.SetBorderSize(0);
      leg.SetFillStyle(0);
      leg.SetTextSize(0.04);
      leg.AddEntry(h1, labels[c1], points ? "p" : "l");
      leg.AddEntry(h2, labels[c2], points ? "p" : "l");
      leg.Draw();

      text.DrawLatex(infoX, infoY, info);
      cv.Print(Form("%s/%s_%s.png", outDir, prefix[v], pairNames[k]));
      cv.Print(Form("%s/%s_%s.pdf", outDir, prefix[v], pairNames[k]));
      cv.Print(allPdf, Form("Title:%s_%s", prefix[v], pairNames[k]));
    }
  }

  cvAll.Print(allPdf + "]");

  delete f;
}
