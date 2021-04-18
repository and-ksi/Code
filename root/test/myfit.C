/// \file
/// \ingroup tutorial_fit
/// \notebook -js
/// Get in memory an histogram from a root file and fit a user defined function.
/// Note that a user defined function must always be defined
/// as in this example:
///  - first parameter: array of variables (in this example only 1-dimension)
///  - second parameter: array of parameters
/// Note also that in case of user defined functions, one must set
/// an initial value for each parameter.
///
/// \macro_image
/// \macro_output
/// \macro_code
///
/// \author Rene Brun

Double_t fitf(Double_t *x, Double_t *par)
{
   Double_t arg = 0;
   if (par[2] != 0) arg = (x[0] - par[1])/par[2];

   Double_t fitval = par[0]*TMath::Exp(-0.5*arg*arg);
   return fitval;
}
void myfit()
{
   /*TString dir = gROOT->GetTutorialDir();//显然起到打开TutorialDir目录的作用
   dir.Append("/hsimple.C");
   dir.ReplaceAll("/./","/");
   if (!gInterpreter->IsLoaded(dir.Data())) gInterpreter->LoadMacro(dir.Data());
   TFile *hsimpleFile = (TFile*)gROOT->ProcessLineFast("hsimple(1)");*/
   //上述代码取消，已经在文件中存在hsimple.root
   TFile*hsimpleFile=TFile::Open("hsimple.root");//另一种在文件名前加../，失败
   if (!hsimpleFile) return;

   TCanvas *c1 = new TCanvas("c1","the fit canvas",500,400);

   TH1F *hpx = (TH1F*)hsimpleFile->Get("hpx");//Get("what");

// Creates a Root function based on function fitf above
   TF1 *func = new TF1("fitf",fitf,-2,2,3);

// Sets initial values and parameter names
   func->SetParameters(100,0,1);
   func->SetParNames("Constant","Mean_value","Sigma");

// Fit histogram in range defined by function
   hpx->Fit(func,"r");//仅在函数取值范围内进行拟合

// Gets integral of function between fit limits
   printf("Integral of function = %g\n",func->Integral(-2,2));//%g   表示bai以%f%e中较短的输出宽du度输出单、双zhi精度实数，在指数小于-4或者dao大于等于精度时使用%e格式。
}
