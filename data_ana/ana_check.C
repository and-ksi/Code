1. #define AnaCheck_cxx 
2. #include "AnaCheck.h" 
3. #include<TSystem.h> 
4. #include<TH2.h> 
5. #include<TStyle.h> 
6. #include<TCanvas.h> 
7. #include<TMath.h> 
8. #include<TH1F.h> 
9. #include<TH2F.h> 10. #include<TH3F.h> 11. #include<TAxis.h> 12. #include<TLatex.h> 13. #include<iostream> 14. #include<TGraph2D.h> 15. #include<TGraph.h> 16. using namespace std;
17. 18. void AnaCheck::Loop() 19.
{
    20. if (fChain == 0) return;
    21. 22. Long64_t nentries = fChain->GetEntries();
    23. gStyle->SetOptStat(0);
    24. gStyle->SetPadTopMargin(0.05);
    25. gStyle->SetPadRightMargin(0.05);
    26. gStyle->SetPadLeftMargin(0.13);
    27. gStyle->SetPadBottomMargin(0.13);
    28. gStyle->SetTitleSize(0.05, "XYZ");
    29. gStyle->SetTitleFont(62, "XYZ");
    30. gStyle->SetTitleOffset(1.1, "XYZ");
    31. gStyle->SetNdivisions(505, "XYZ");
    32. 33. Float_t CFDdata[1024];
    34. Float_t CFDmean;
    35. Float_t CFDfactor = 1.05;
    36. Float_t CFDthresh1 = -60;
    37. Int_t CFDoffset = 50;
    38. Float_t Ch2ns = 0.4; //5G==>0.2, 2.5G==>0.4
    39. 40. TCanvas *canv0 = new TCanvas("canv0", "canv0", 1500, 700);
    41. canv0->Divide(2, 1);
    42. canv0->cd(1);
    43. TH1F *hf = gPad->DrawFrame(0, 0, 1100, 3500);
    44. hf->GetXaxis()->SetTitle("channel");
    45. hf->GetXaxis()->CenterTitle();
    46. TGraph *fgr[9];
    47. for (Int_t i = 0; i < 9; i++)
    {
        48. fgr[i] = new TGraph(1024);
        49. for (Int_t j = 0; j < 1024; j++) fgr[i]->SetPoint(j, j + 1, 3000);
        50. fgr[i]->SetLineWidth(2);
        51. fgr[i]->SetLineColor(i + 1);
        52. fgr[i]->Draw("L");
        53.
    }
    54. canv0->cd(2);
    55. TH3F *hxyz = new TH3F("hxyz", ";x (mm);y (mm);z(mm)", 100, -60, 60, 100, -60, 60, 100, -150, 150);
    56. hxyz->GetXaxis()->CenterTitle();
    57. hxyz->GetYaxis()->CenterTitle();
    58. hxyz->GetZaxis()->CenterTitle();
    59. hxyz->Draw();
    60. TGraph2D *fgr2d = new TGraph2D(2);
    61. fgr2d->SetPoint(0, 0, 0, 500);
    62. fgr2d->SetPoint(1, 0, 0, -500);
    63. fgr2d->SetMarkerStyle(20);
    64. fgr2d->SetMarkerColor(2);
    65. fgr2d->SetLineStyle(1);
    66. fgr2d->SetLineColor(2);
    67. fgr2d->SetLineWidth(2);
    68. fgr2d->Draw("LINEPsame");
    69. 70. for (Long64_t jentry = 0; jentry < nentries; jentry++)
    {
        71. Long64_t ientry = LoadTree(jentry);
        72. if (ientry < 0) break;
        73. //if (ientry > 1e4) continue;
            74. fChain->GetEntry(jentry);
        75. Float_t Tch[9] = {0};
        76. Int_t iflag = 0;
        77. for (Int_t ich = 0; ich < 9; ich++)
        {
            78. CFDmean = TMath::Mean(100, TrackData[ich]);
            79. for (Int_t i = 0; i < 1024; i++)
            {
                80. CFDdata[i] = TrackData[ich][i] - CFDmean;
                81. if (i >= CFDoffset) CFDdata[i] += CFDfactor * (CFDmean - TrackData[ich][iCFDoffset]);
                82.
            }
            83. Int_t T0 = TMath::LocMin(900, CFDdata);
            84. if (CFDdata[T0] > CFDthresh1)
            {
                Tch[ich] = 0;
                iflag = 1;
                continue;
            }
            85. for (Int_t i = T0; i < 1023; i++)
            {
                86. if (CFDdata[i] == 0)
                {
                    87. Tch[ich] = i;
                    88. break;
                    89.
                }
                90. else if (CFDdata[i] * CFDdata[i + 1] < 0)
                {
                    91. Tch[ich] = float(i) + TMath::Abs(CFDdata[i] / (CFDdata[i] - CFDdata[i + 1]));
                    92. break;
                    93.
                }
                94.
            }
            95. if (Tch[ich] <= 100)
            {
                iflag = 1;
                break;
            }
            96.
        }
        97.     //for(Int_t ich=2;ich<4;ich++) if(Tch[ich]<=50){ iflag = 1; break;}
            98. //for(Int_t ich=6;ich<8;ich++) if(Tch[ich]<=50){ iflag = 1; break;}
            99. 100. if (iflag == 1) continue;
        101. Int_t icount = 0;
        102. for (Int_t i = 0; i < 160; i++)
        {
            103. if (TMath::MaxElement(5, &TrackData[8][150 + i * 5]) -
                         TMath::MinElement(5, &TrackData[8][150 + i * 5]) >
                     200) icount++;
            104.
        }
        105. 106. if (icount > 7) continue;
        107. cout << fCurrent << " " << ientry << " " << icount << endl;
        108. 109. for (Int_t i = 0; i < 9; i++)
        {
            110. for (Int_t j = 0; j < 1024; j++)
            {
                111. fgr[i]->SetPoint(j, j + 1, TrackData[i][j] - i * 200 + 600);
                112.
            }
            113.
        }
        114. 115. Float_t XX1 = (Tch[0] - Tch[1]) * 3. / 8. * Ch2ns;
        116. Float_t YY1 = (Tch[2] - Tch[3]) * 3. / 8. * Ch2ns;
        117. fgr2d->SetPoint(0, XX1, YY1, 100.);
        118. Float_t XX2 = (Tch[4] - Tch[5]) * 3. / 8. * Ch2ns;
        119. Float_t YY2 = (Tch[6] - Tch[7]) * 3. / 8. * Ch2ns;
        120. fgr2d->SetPoint(1, XX2, YY2, -100.);
        121. for (Int_t i = 0; i < 9; i++) cout << Tch[i] << " ";
        122. cout << endl;
        123. canv0->cd(1)->Modified();
        124. canv0->cd(1)->Update();
        125. canv0->cd(2)->Modified();
        126. canv0->cd(2)->Update();
        127. gPad->WaitPrimitive();
        128. 129.
    }
    130.
}