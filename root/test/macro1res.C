void macro1res()
{
//=========Macro generated from canvas: c1/c1
//=========  (Thu Jan 28 20:02:29 2021) by ROOT version 6.22/06
   TCanvas *c1 = new TCanvas("c1", "c1",507,86,700,500);
   c1->Range(-1.242755,-6.988366,12.24275,72.06137);
   c1->SetFillColor(0);
   c1->SetBorderMode(0);
   c1->SetBorderSize(2);
   c1->SetFrameBorderMode(0);
   c1->SetFrameBorderMode(0);
   
   Double_t Graph0_fx1001[10] = {
   1,
   2,
   3,
   4,
   5,
   6,
   7,
   8,
   9,
   10};
   Double_t Graph0_fy1001[10] = {
   6,
   12,
   14,
   20,
   22,
   24,
   35,
   45,
   44,
   53};
   Double_t Graph0_fex1001[10] = {
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0};
   Double_t Graph0_fey1001[10] = {
   5,
   5,
   4.7,
   4.5,
   4.2,
   5.1,
   2.9,
   4.1,
   4.8,
   5.43};
   TGraphErrors *gre = new TGraphErrors(10,Graph0_fx1001,Graph0_fy1001,Graph0_fex1001,Graph0_fey1001);
   gre->SetName("Graph0");
   gre->SetTitle("Measurement XYZ");
   gre->SetFillStyle(1000);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#0000ff");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#0000ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(24);
   
   TH1F *Graph_Graph01001 = new TH1F("Graph_Graph01001","Measurement XYZ",100,0.1,10.9);
   Graph_Graph01001->SetMinimum(0.9);
   Graph_Graph01001->SetMaximum(64.173);
   Graph_Graph01001->SetDirectory(0);
   Graph_Graph01001->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph01001->SetLineColor(ci);
   Graph_Graph01001->GetXaxis()->SetTitle("lenght [cm]");
   Graph_Graph01001->GetXaxis()->SetLabelFont(42);
   Graph_Graph01001->GetXaxis()->SetTitleOffset(1);
   Graph_Graph01001->GetXaxis()->SetTitleFont(42);
   Graph_Graph01001->GetYaxis()->SetTitle("Arb.Units");
   Graph_Graph01001->GetYaxis()->SetLabelFont(42);
   Graph_Graph01001->GetYaxis()->SetTitleFont(42);
   Graph_Graph01001->GetZaxis()->SetLabelFont(42);
   Graph_Graph01001->GetZaxis()->SetTitleOffset(1);
   Graph_Graph01001->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph01001);
   
   gre->Draw("ape");
   
   TF1 *Linear law1 = new TF1("Linear law","[0]+x*[1]",0.5,10.5, TF1::EAddToList::kDefault);
   Linear law1->SetFillColor(19);
   Linear law1->SetFillStyle(0);

   ci = TColor::GetColor("#ff0000");
   Linear law1->SetLineColor(ci);
   Linear law1->SetLineWidth(2);
   Linear law1->SetLineStyle(2);
   Linear law1->SetChisquare(3.848831);
   Linear law1->SetNDF(8);
   Linear law1->GetXaxis()->SetLabelFont(42);
   Linear law1->GetXaxis()->SetTitleOffset(1);
   Linear law1->GetXaxis()->SetTitleFont(42);
   Linear law1->GetYaxis()->SetLabelFont(42);
   Linear law1->GetYaxis()->SetTitleFont(42);
   Linear law1->SetParameter(0,-1.016044);
   Linear law1->SetParError(0,3.334091);
   Linear law1->SetParLimits(0,0,0);
   Linear law1->SetParameter(1,5.187557);
   Linear law1->SetParError(1,0.530717);
   Linear law1->SetParLimits(1,0,0);
   Linear law1->Draw("Same");
   
   TLegend *leg = new TLegend(0.1,0.7,0.3,0.9,NULL,"brNDC");
   leg->SetBorderSize(1);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   TLegendEntry *entry=leg->AddEntry("NULL","Lab. Lesson 1","h");
   entry->SetLineColor(1);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(21);
   entry->SetMarkerSize(1);
   entry->SetTextFont(42);
   entry=leg->AddEntry("Graph","Exp. Points","lpf");
   entry->SetFillStyle(1000);

   ci = TColor::GetColor("#0000ff");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#0000ff");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(24);
   entry->SetMarkerSize(1);
   entry->SetTextFont(42);
   entry=leg->AddEntry("Linear law","Th. Law","lpf");
   entry->SetFillColor(19);

   ci = TColor::GetColor("#ff0000");
   entry->SetLineColor(ci);
   entry->SetLineStyle(2);
   entry->SetLineWidth(2);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(1);
   entry->SetMarkerSize(1);
   entry->SetTextFont(42);
   leg->Draw();
   TArrow *arrow = new TArrow(8,8,6.2,23,0.02,"|>");
   arrow->SetFillColor(1);
   arrow->SetFillStyle(1001);
   arrow->SetLineWidth(2);
   arrow->Draw();
   TLatex *   tex = new TLatex(8.2,7.5,"#splitline{Maximum}{Deviation}");
   tex->SetLineWidth(2);
   tex->Draw();
   
   TPaveText *pt = new TPaveText(0.3410029,0.94,0.6589971,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   TText *pt_LaTex = pt->AddText("Measurement XYZ");
   pt->Draw();
   c1->Modified();
   c1->cd();
   c1->SetSelected(c1);
}
