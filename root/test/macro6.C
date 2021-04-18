// Divide and add 1D Histograms

void format_h(TH1F* h, int linecolor){//对直方图进行美化
    h->SetLineWidth(3);
    h->SetLineColor(linecolor);
    }

void macro6(){

    auto sig_h=new TH1F("sig_h","Signal Histo",50,0,10);//定义四个直方图
    auto gaus_h1=new TH1F("gaus_h1","Gauss Histo 1",30,0,10);
    auto gaus_h2=new TH1F("gaus_h2","Gauss Histo 2",30,0,10);
    auto bkg_h=new TH1F("exp_h","Exponential Histo",50,0,10);

    // simulate the measurements
    TRandom3 rndgen;
    for (int imeas=0;imeas<4000;imeas++){
        bkg_h->Fill(rndgen.Exp(4));//采用指数分布选取随机数，填充入背景本底
        if (imeas%4==0) gaus_h1->Fill(rndgen.Gaus(5,2));
        if (imeas%4==0) gaus_h2->Fill(rndgen.Gaus(5,2));
        if (imeas%10==0)sig_h->Fill(rndgen.Gaus(5,.5));}

    // Format Histograms
    int i=0;
    for (auto hist : {sig_h,bkg_h,gaus_h1,gaus_h2})//创建一个hist数组，包括上述四个直方图，并且使用for的特殊用法，遍历hist数组格式化
        format_h(hist,1+i++);//将四个直方图格式化，美化

    // Sum
    auto sum_h= new TH1F(*bkg_h);
    sum_h->Add(sig_h,1.);//将信号函数加入背景本底，1代表sig_h乘以1以后加入sum_h直方图
    sum_h->SetTitle("Exponential + Gaussian;X variable;Y variable");
    format_h(sum_h,kBlue);//将sum_h格式化

    auto c_sum= new TCanvas();
    sum_h->Draw("hist");
    bkg_h->Draw("Samehist");//hist是否为上一行内容？
    sig_h->Draw("Samehist");
    

    // Divide
    auto dividend=new TH1F(*gaus_h1);//设置新的histo 名为dividend
    dividend->Divide(gaus_h2);//使用Divide()函数将直方图相除

    // Graphical Maquillage
    dividend->SetTitle(";X axis;Gaus Histo 1 / Gaus Histo 2");//总标题无内容
    format_h(dividend,kOrange);//将相除的直方图格式化
    gaus_h1->SetTitle(";;Gaus Histo 1 and Gaus Histo 2");//总标题、x轴无内容
    gStyle->SetOptStat(0);//禁止打印总标题

    TCanvas* c_divide= new TCanvas();
    c_divide->Divide(1,2,0,0);//将c_divide直方图分割，x方向分为1份，y方向分为2份，0,0意味着如果分割，两者距离均为0
    c_divide->cd(1);//设置分割后的第一块内容
    c_divide->GetPad(1)->SetRightMargin(.01);//在第一块内容中，设置右边距离为0.01是为了让图标右边框显示出来
    gaus_h1->SetMinimum(0.001);
    gaus_h1->DrawNormalized("Hist");//绘制归一化的高斯函数1
    gaus_h2->DrawNormalized("HistSame");//

    c_divide->cd(2);
    dividend->GetYaxis()->SetRangeUser(0,2.49);//为了防止显示出来2.5
    c_divide->GetPad(2)->SetGridy();//绘制y方向grid
    c_divide->GetPad(2)->SetRightMargin(.01);
    gPad->SetTickx(1);
    dividend->Draw();
}