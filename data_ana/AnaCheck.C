#define AnaCheck_cxx
#include "AnaCheck.h"
#include <TAxis.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TH1F.h>
#include <TH2.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TLatex.h>
#include <TMath.h>
#include <TStyle.h>
#include <TSystem.h>
#include <iostream>
using namespace std;

void AnaCheck::Loop()
{
    if (fChain == 0)
        return;

    Long64_t nentries = fChain->GetEntries();
    gStyle->SetOptStat(0);
    gStyle->SetPadTopMargin(0.05);
    gStyle->SetPadRightMargin(0.05);
    gStyle->SetPadLeftMargin(0.13);
    gStyle->SetPadBottomMargin(0.13);
    gStyle->SetTitleSize(0.05, "XYZ");
    gStyle->SetTitleFont(62, "XYZ");
    gStyle->SetTitleOffset(1.1, "XYZ");
    gStyle->SetNdivisions(505, "XYZ");

    Float_t CFDdata[1024];
    Float_t CFDmean;
    Float_t CFDfactor = 1.05;
    Float_t CFDthresh1 = -60;
    Int_t CFDoffset = 50;
    Float_t Ch2ns = 0.4; //5G==>0.2, 2.5G==>0.4

    TCanvas *canv0 = new TCanvas("canv0", "canv0", 1500, 700);
    canv0->Divide(2, 1);
    canv0->cd(1);
    TH1F *hf = gPad->DrawFrame(0, 0, 1100, 3500);
    hf->GetXaxis()->SetTitle("channel");
    hf->GetXaxis()->CenterTitle();
    TGraph *fgr[9];
    for (Int_t i = 0; i < 9; i++)
    {
        fgr[i] = new TGraph(1024);
        for (Int_t j = 0; j < 1024; j++)
            fgr[i]->SetPoint(j, j + 1, 3000);
        fgr[i]->SetLineWidth(2);
        fgr[i]->SetLineColor(i + 1);
        fgr[i]->Draw("L");
    }
    canv0->cd(2);
    TH3F *hxyz = new TH3F("hxyz", ";x (mm);y (mm);z(mm)", 100, -60, 60, 100, -60, 60, 100, -150, 150);
    hxyz->GetXaxis()->CenterTitle();
    hxyz->GetYaxis()->CenterTitle();
    hxyz->GetZaxis()->CenterTitle();
    hxyz->Draw();
    TGraph2D *fgr2d = new TGraph2D(2);
    fgr2d->SetPoint(0, 0, 0, 500);
    fgr2d->SetPoint(1, 0, 0, -500);
    fgr2d->SetMarkerStyle(20);
    fgr2d->SetMarkerColor(2);
    fgr2d->SetLineStyle(1);
    fgr2d->SetLineColor(2);
    fgr2d->SetLineWidth(2);
    fgr2d->Draw("LINEPsame");

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0)
            break;
        //if (ientry > 1e4) continue;
        fChain->GetEntry(jentry);
        Float_t Tch[9] = {0};
        Int_t iflag = 0;
        for (Int_t ich = 0; ich < 9; ich++)
        {
            CFDmean = TMath::Mean(100, TrackData[ich]);
            for (Int_t i = 0; i < 1024; i++)
            {
                CFDdata[i] = TrackData[ich][i] - CFDmean;
                if (i >= CFDoffset)
                    CFDdata[i] += CFDfactor * (CFDmean - TrackData[ich][i - CFDoffset]);
            }
            Int_t T0 = TMath::LocMin(900, CFDdata);
            if (CFDdata[T0] > CFDthresh1)
            {
                Tch[ich] = 0;
                iflag = 1;
                continue;
            }
            for (Int_t i = T0; i < 1023; i++)
            {
                if (CFDdata[i] == 0)
                {
                    Tch[ich] = i;
                    break;
                }
                else if (CFDdata[i] * CFDdata[i + 1] < 0)
                {
                    Tch[ich] = float(i) + TMath::Abs(CFDdata[i] / (CFDdata[i] - CFDdata[i + 1]));
                    break;
                }
            }
            if (Tch[ich] <= 100)
            {
                iflag = 1;
                break;
            }
        }
        //for(Int_t ich=2;ich<4;ich++) if(Tch[ich]<=50){ iflag = 1; break;}
        //for(Int_t ich=6;ich<8;ich++) if(Tch[ich]<=50){ iflag = 1; break;}

        if (iflag == 1)
            continue;
        Int_t icount = 0;
        for (Int_t i = 0; i < 160; i++)
        {
            if (TMath::MaxElement(5, &TrackData[8][150 + i * 5]) -
                    TMath::MinElement(5, &TrackData[8][150 + i * 5]) >
                200)
                icount++;
        }

        if (icount > 7)
            continue;
        cout << fCurrent << " " << ientry << " " << icount << endl;

        for (Int_t i = 0; i < 9; i++)
        {
            for (Int_t j = 0; j < 1024; j++)
            {
                fgr[i]->SetPoint(j, j + 1, TrackData[i][j] - i * 200 + 600);
            }
        }

        Float_t XX1 = (Tch[0] - Tch[1]) * 3. / 8. * Ch2ns;
        Float_t YY1 = (Tch[2] - Tch[3]) * 3. / 8. * Ch2ns;
        fgr2d->SetPoint(0, XX1, YY1, 100.);
        Float_t XX2 = (Tch[4] - Tch[5]) * 3. / 8. * Ch2ns;
        Float_t YY2 = (Tch[6] - Tch[7]) * 3. / 8. * Ch2ns;
        fgr2d->SetPoint(1, XX2, YY2, -100.);
        for (Int_t i = 0; i < 9; i++)
            cout << Tch[i] << " ";
        cout << endl;
        canv0->cd(1)->Modified();
        canv0->cd(1)->Update();
        canv0->cd(2)->Modified();
        canv0->cd(2)->Update();
        gPad->WaitPrimitive();
    }
}