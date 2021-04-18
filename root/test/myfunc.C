// Cacro myfunc.C
Double_t myfunction(Double_t *x, Double_t *par)
{
	Float_t xx =x[0];
	Double_t f = TMath::Abs(par[0]*sin(par[1]*xx)/xx);
	return f;
}
void myfunc()
{
	TF1*f1 = new TF1("myfunc",myfunction,0,10,2);//指针为f1，内存名为myfunc
	f1->SetParameters(2,1);
	f1->SetParNames("constant","coefficient");
	f1->Draw();
}
void myfit()
{
	TH1F *h1 = new TH1F("h1","test",100,0,10);
	h1->FillRandom("myfunc",20000);
	TF1 *f1=(TF1 *)gROOT->GetFunction("myfunc");//这里直接处理指针
	f1->SetParameters(800,1);
	h1->Fit("myfunc");//TH1F可以直接在TF1的基础上直接作图？
}
