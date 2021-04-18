void format_line(TAttLine* line,int col,int sty){
    line->SetLineWidth(5); line->SetLineColor(col);
    line->SetLineStyle(sty);}

double the_gausppar(double* vars, double* pars){//定义复合信号复合函数样式  vars定义  pars参数
    return pars[0]*TMath::Gaus(vars[0],pars[1],pars[2])+//参数乘以一个高斯函数
        pars[3]+pars[4]*vars[0]+pars[5]*vars[0]*vars[0];}

int macro8(){
    gStyle->SetOptTitle(0); gStyle->SetOptStat(0);//禁止自动打印title 禁止打印统计信息（mean之类）
    gStyle->SetOptFit(1111); gStyle->SetStatBorderSize(0);//将一些控制信息打开，数字越少，控制越少   图标框为0
    gStyle->SetStatX(.89); gStyle->SetStatY(.89);//控制信息右上角坐标

    TF1 parabola("parabola","[0]+[1]*x+[2]*x**2",0,20);//声明抛物线函数
    format_line(&parabola,kBlue,2);//对抛物线样式定义   &抛物线指针

    TF1 gaussian("gaussian","[0]*TMath::Gaus(x,[1],[2])",0,20);
    format_line(&gaussian,kRed,2);

    TF1 gausppar("gausppar",the_gausppar,-0,20,6);//定义复合信号函数   6为参数个数
    double a=15; double b=-1.2; double c=.03;
    double norm=4; double mean=7; double sigma=1;
    gausppar.SetParameters(norm,mean,sigma,a,b,c);//设置参数值
    gausppar.SetParNames("Norm","Mean","Sigma","a","b","c");//设置参数名
    format_line(&gausppar,kBlue,1);

    TH1F histo("histo","Signal plus background;X vals;Y Vals",50,0,20);//定义了一个historygrame
    histo.SetMarkerStyle(8);//设定标注符号8   TH1F name("name","title;X name;Y name",bin number,start,end)//bin可以自动调整数量

    // Fake the data
    for (int i=1;i<=5000;++i) histo.Fill(gausppar.GetRandom());//填充histo函数，使用复合信号函数  Fill(gausppar.GetRandom());    ?

    // Reset the parameters before the fit and set
    // by eye a peak at 6 with an area of more or less 50
    gausppar.SetParameter(0,50);
    gausppar.SetParameter(1,6);
    int npar=gausppar.GetNpar();//get number of par
    for (int ipar=2;ipar<npar;++ipar) gausppar.SetParameter(ipar,1);//将其余参数均设置为1

    // perform fit ...
    auto fitResPtr = histo.Fit(&gausppar, "S");//进行拟合
    // ... and retrieve fit results
    fitResPtr->Print(); // print fit results
    // get covariance Matrix an print it
    TMatrixDSym covMatrix (fitResPtr->GetCovarianceMatrix());//调用协方差计算函数
    covMatrix.Print();

    // Set the values of the gaussian and parabola
    for (int ipar=0;ipar<3;ipar++){
        gaussian.SetParameter(ipar,gausppar.GetParameter(ipar));
        parabola.SetParameter(ipar,gausppar.GetParameter(ipar+3));//分别打印出高斯函数和二次函数图像
    }

    histo.GetYaxis()->SetRangeUser(0,250);
    histo.DrawClone("PE");//画出point 和error
    parabola.DrawClone("Same"); gaussian.DrawClone("Same");
    TLatex latex(2,220,"#splitline{Signal Peak over}{background}");//自动分两行
    latex.DrawClone("Same");//画在同一张图中
    return 0;
}
