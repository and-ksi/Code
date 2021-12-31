#define AnaTrack_cxx
#include "AnaTrack.h"
#include <TAxis.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2.h>
#include <TH2F.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TMath.h>
#include <TStyle.h>
#include <iostream>
using namespace std;

void AnaTrack::Loop()
{
    if (fChain == 0)
        return;

    Long64_t nentries = fChain->GetEntries();
    gStyle->SetPadTopMargin(0.05);
    gStyle->SetPadRightMargin(0.05);
    gStyle->SetPadLeftMargin(0.13);
    gStyle->SetPadBottomMargin(0.13);
    gStyle->SetTitleSize(0.05, "XY");
    gStyle->SetTitleFont(62, "XY");
    gStyle->SetTitleOffset(1.1, "XY");

    Float_t CFDdata[1024];
    Float_t CFDmean;
    Float_t CFDfactor = 1.05;
    Float_t CFDthresh = -60;
    Int_t CFDoffset = 60;
    Float_t ch2ns = 0.4; // 5GS/s==>0.2
    // 2.5Gs/s==>0.4
    // 1GS/s==>1
    TH1F *hSX1, *hSY1;
    TH1F *hDX1, *hDY1;
    TH2F *hSXY1, *hDXY1;
    TH1F *hSX2, *hSY2;
    TH1F *hDX2, *hDY2;
    TH2F *hSXY2, *hDXY2;
    hSX1 = new TH1F("hSX1", ";X (mm);Counts", 200, -100, 100);
    hSY1 = new TH1F("hSY1", ";Y (mm);Counts", 200, -100, 100);
    hDX1 = new TH1F("hDX1", ";X (mm);Counts", 200, -100, 100);
    hDY1 = new TH1F("hDY1", ";Y (mm);Counts", 200, -100, 100);
    hSXY1 = new TH2F("hSXY1", ";X (mm);Y (mm)", 200, -100, 100, 200, -100, 100);
    hDXY1 = new TH2F("hDXY1", ";X (mm);Y (mm)", 200, -100, 100, 200, -100, 100);
    hSX2 = new TH1F("hSX2", ";X (mm);Counts", 200, -100, 100);
    hSY2 = new TH1F("hSY2", ";Y (mm);Counts", 200, -100, 100);
    hDX2 = new TH1F("hDX2", ";X (mm);Counts", 200, -100, 100);
    hDY2 = new TH1F("hDY2", ";Y (mm);Counts", 200, -100, 100);
    hSXY2 = new TH2F("hSXY2", ";X (mm);Y (mm)", 200, -100, 100, 200, -100, 100);
    hDXY2 = new TH2F("hDXY2", ";X (mm);Y (mm)", 200, -100, 100, 200, -100, 100);

    for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0)
            break;
        fChain->GetEntry(jentry);
        Float_t Tch[8] = {0};
        for (Int_t ich = 0; ich < 8; ich++)
        {
            CFDmean = TMath::Mean(100, TrackData[ich]);
            for (Int_t i = 0; i < 1024; i++)
            {
                CFDdata[i] = TrackData[ich][i] - CFDmean;
                if (i >= CFDoffset)
                    CFDdata[i] += CFDfactor * (CFDmean - TrackData[ich][i - CFDoffset]);
            }
            Int_t T0 = TMath::LocMin(800, CFDdata);
            if (CFDdata[T0] > CFDthresh)
            {
                Tch[ich] = 0;
                continue;
            }
            for (Int_t i = T0; i < 1000; i++)
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
        }

        Int_t icount = 0;
        for (Int_t i = 0; i < 160; i++)
        {
            if (TMath::MaxElement(5, &TrackData[8][200 + i * 5]) -
                    TMath::MinElement(5, &TrackData[8][200 + i * 5]) >
                100)
                icount++;
        }
        if (icount > 7)
            continue;
        //cout<<icount<<endl;

        Int_t iflag1 = 0;
        for (Int_t ich = 0; ich < 4; ich++)
            if (Tch[ich] <= 200)
            {
                iflag1 = 1;
                break;
            }
        Int_t iflag2 = 0;
        for (Int_t ich = 4; ich < 8; ich++)
            if (Tch[ich] <= 200)
            {
                iflag2 = 1;
                break;
            }

        Float_t XX1 = (Tch[0] - Tch[1]) * 3. / 8. * ch2ns;
        Float_t YY1 = (Tch[2] - Tch[3]) * 3. / 8. * ch2ns;
        Float_t XX2 = (Tch[4] - Tch[5]) * 3. / 8. * ch2ns;
        Float_t YY2 = (Tch[6] - Tch[7]) * 3. / 8. * ch2ns;
        if (iflag1 == 0 && iflag2 == 0)
        {
            hDX1->Fill(XX1);
            hDY1->Fill(YY1);
            hDX2->Fill(XX2);
            hDY2->Fill(YY2);
            hDXY1->Fill(XX1, YY1);
            hDXY2->Fill(XX2, YY2);
            //cout<<((Tch[0]+Tch[1])/2.-(Tch[4]+Tch[5])/2.)*ch2ns<<"ns ";
            //cout<<((Tch[2]+Tch[3])/2.-(Tch[6]+Tch[7])/2.)*ch2ns<<"ns ";
            //cout<<((Tch[0]+Tch[1]+Tch[2]+Tch[3])/4.-(Tch[4]+Tch[5]+Tch[6]+Tch[7])/4.)*ch2ns<<"ns"<<endl;
        }
        else if (iflag1 == 0)
        {
            hSX1->Fill(XX1);
            hSY1->Fill(YY1);
            hSXY1->Fill(XX1, YY1);
        }
        else if (iflag2 == 0)
        {
            hSX2->Fill(XX2);
            hSY2->Fill(YY2);
            hSXY2->Fill(XX2, YY2);
        }
    }
    TLatex ltx;
    TLegend *lgd = new TLegend(0.4, 0.7, 0.9, 0.9);
    lgd->SetBorderSize(0);
    lgd->SetFillColor(10);
    lgd->SetTextFont(62);
    lgd->SetTextSize(0.07);

    TCanvas *canv1 = new TCanvas("canv1", "canv1", 1300, 800);
    canv1->Divide(3, 2);
    canv1->cd(1);
    TH1F *hf1 = gPad->DrawFrame(-100, 0.7, 100, 1e4);
    hf1->GetXaxis()->SetTitle("X (mm)");
    hf1->GetXaxis()->CenterTitle();
    gPad->SetLogy();
    ltx.SetTextFont(62);
    ltx.SetTextSize(0.08);
    ltx.DrawLatex(-90, 3e3, "MWPC1");
    ltx.DrawLatex(70, 3e3, "1^{#}");
    hSX1->SetLineColor(1);
    hSX1->SetLineWidth(2);
    hSX1->Draw("same");
    hDX1->SetLineColor(2);
    hDX1->SetLineWidth(2);
    hDX1->Draw("same");
    canv1->cd(2);
    TH1F *hf2 = gPad->DrawFrame(-100, 0.7, 100, 1e4);
    hf2->GetXaxis()->SetTitle("Y (mm)");
    hf2->GetXaxis()->CenterTitle();
    gPad->SetLogy();
    hSY1->SetLineColor(1);
    hSY1->SetLineWidth(2);
    hSY1->Draw("same");
    hDY1->SetLineColor(2);
    hDY1->SetLineWidth(2);
    hDY1->Draw("same");
    lgd->AddEntry(hSY1, "Single fired", "L");
    lgd->AddEntry(hDY1, "Double fired", "L");
    lgd->Draw();
    canv1->cd(3);
    TH1F *hf3 = gPad->DrawFrame(-100, -100, 100, 100);
    hf3->GetXaxis()->SetTitle("X (mm)");
    hf3->GetXaxis()->CenterTitle();
    hf3->GetYaxis()->SetTitle("Y (mm)");
    hf3->GetYaxis()->CenterTitle();
    hSXY1->Draw("same");
    hDXY1->SetMarkerColor(2);
    hDXY1->Draw("same");
    canv1->cd(4);
    TH1F *hf4 = gPad->DrawFrame(-100, 0.7, 100, 1e4);
    hf4->GetXaxis()->SetTitle("X (mm)");
    hf4->GetXaxis()->CenterTitle();
    gPad->SetLogy();
    ltx.SetTextFont(62);
    ltx.SetTextSize(0.08);
    ltx.DrawLatex(-90, 3e3, "MWPC2");
    hSX2->SetLineColor(1);
    hSX2->SetLineWidth(2);
    hSX2->Draw("same");
    hDX2->SetLineColor(2);
    hDX2->SetLineWidth(2);
    hDX2->Draw("same");
    canv1->cd(5);
    TH1F *hf5 = gPad->DrawFrame(-100, 0.7, 100, 1e4);
    hf5->GetXaxis()->SetTitle("Y (mm)");
    hf5->GetXaxis()->CenterTitle();
    gPad->SetLogy();
    hSY2->SetLineColor(1);
    hSY2->SetLineWidth(2);
    hSY2->Draw("same");
    hDY2->SetLineColor(2);
    hDY2->SetLineWidth(2);
    hDY2->Draw("same");
    canv1->cd(6);
    TH1F *hf6 = gPad->DrawFrame(-100, -100, 100, 100);
    hf6->GetXaxis()->SetTitle("X (mm)");
    hf6->GetXaxis()->CenterTitle();
    hf6->GetYaxis()->SetTitle("Y (mm)");
    hf6->GetYaxis()->CenterTitle();
    hSXY2->SetMarkerSize(2);
    hSXY2->Draw("same");
    hDXY2->SetMarkerSize(2);
    hDXY2->SetMarkerColor(2);
    hDXY2->Draw("same");

    TFile *rootfile = new TFile("Histograms_cosmic_ray_track_det1_1700V.root", "recreate");
    hSX1->Write();
    hSY1->Write();
    hSXY1->Write();
    hSX2->Write();
    hSY2->Write();
    hSXY2->Write();
    hDX1->Write();
    hDY1->Write();
    hDXY1->Write();
    hDX2->Write();
    hDY2->Write();
    hDXY2->Write();
    rootfile->Write();
}