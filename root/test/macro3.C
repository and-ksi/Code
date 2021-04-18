// Builds a polar graph in a square Canvas.

void macro3(){
	TCanvas* c = new TCanvas("myCanvas","myCanvas",600,600);
	double theta_min = 0.;
	double theta_max = TMath::Pi()*6.;
	const int npoints = 1000;
	Double_t r[npoints];
	Double_t theta[npoints];
	for (Int_t ipt = 0; ipt < npoints; ipt++) {
		theta[ipt] = ipt*(theta_max-theta_min)/npoints+theta_min;
		r[ipt] = TMath::Sin(theta[ipt]);
	}
	auto grP1= new TGraphPolar (npoints,theta,r);
	grP1->SetTitle("A           Fan");
	grP1->SetLineWidth(3);
	grP1->SetLineColor(2);
	grP1->DrawClone("L");
	gPad->Update();
	grP1->GetPolargram()->SetToRadian();
}