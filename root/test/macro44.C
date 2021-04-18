void macro44(){
	gStyle->SetPalette(1);
	const double e =0.3;
	const int nd = 500;

	TRandom3 my_random_generator;
	TF2 *f2 = new TF2("f2","1000*(([0]*sin(x)/x)*([1]*sin(y)/y))+200",-6,6,-6,6);
	f2->SetParameters(1,1);
	TGraph2DErrors *dte = new TGraph2DErrors(nd);
	double rnd,x,y,z,ex,ey,ez;
	for(Int_t i=0; i<nd; i++){
		f2->GetRandom2(x,y);
		rnd = my_random_generator.Uniform(-e,e);
		z=f2->Eval(x,y)*(1+rnd);
		dte->SetPoint(i,x,y,z);
		ex=0.05*my_random_generator.Uniform();
		ey=0.05*my_random_generator.Uniform();
		ez=TMath::Abs(z*rnd);
		dte->SetPointError(i,ex,ey,ez);
	}
	f2->SetParameters(0.7,1.5);
	f2->SetTitle("Fitted 2D function");
	dte->Fit(f2);
	TCanvas *c1 = new TCanvas();
	f2->Draw("Surf1");
	dte->Draw("P0 Same");
	TCanvas* c_p=new TCanvas("ProjCan","The Projections",1000,400);
	c_p->Divide(2,1);
	c_p->cd(1);
	dte->Project("x")->Draw();
	c_p->cd(2);
	dte->Project("y")->Draw();
}