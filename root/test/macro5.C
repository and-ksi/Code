// Create, Fill and draw an Histogram which reproduces the
// counts of a scaler linked to a Geiger counter.

void macro5(){
    auto cnt_r_h=new TH1F("count_rate",//定义一个histogram，cnt_r_h作为一个指针，count...作为标题，x轴title，y轴title
                "Count Rate;N_{Counts};# occurencies",
                100, // Number of Bins
                -0.5, // Lower X Boundary
                15.5); // Upper X Boundary

    auto mean_count=3.6f;//声明一个常量3.6，定义为float
    TRandom3 rndgen;
    // simulate the measurements
    for (int imeas=0;imeas<400;imeas++)
        cnt_r_h->Fill(rndgen.Poisson(mean_count));//根据泊松分布，用Random3，以3.6为μ值填充histogram

    auto c= new TCanvas();
    cnt_r_h->Draw();//声明一个TCanvas，并且开始作图

    auto c_norm= new TCanvas();
    cnt_r_h->DrawNormalized();//做一个新图，归一化分布

    // Print summary
    cout << "Moments of Distribution:\n"
         << " - Mean     = " << cnt_r_h->GetMean() << " +- "
                             << cnt_r_h->GetMeanError() << "\n"
         << " - Std Dev  = " << cnt_r_h->GetStdDev() << " +- "
                             << cnt_r_h->GetStdDevError() << "\n"
         << " - Skewness = " << cnt_r_h->GetSkewness() << "\n"
         << " - Kurtosis = " << cnt_r_h->GetKurtosis() << "\n";
}