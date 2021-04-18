// Create, Draw and fit a TGraph2DErrors//奇怪的是x,y的投影图并不正确，不知原因,可能是默认设置的一些原因
void macro4(){
   gStyle->SetPalette(kBird);//设置调色板
   const double e = 0.3;
   const int nd = 500;

   TRandom3 my_random_generator;//引用TRandom3
   TF2 f2("f2",
          "1000*(([0]*sin(x)/x)*([1]*sin(y)/y))+200",
          -6,6,-6,6);//设置x,y的区间范围
   f2.SetParameters(1,1);//分别赋值[0][1]
   TGraph2DErrors dte(nd);//绘制TGraph2DErrors dte并且包含500个点
   // Fill the 2D graph
   double rnd, x, y, z, ex, ey, ez;
   for (Int_t i=0; i<nd; i++) {
      f2.GetRandom2(x,y);//随机获取f2函数中的x,y点
      // A random number in [-e,e]
      rnd = my_random_generator.Uniform(-e,e);//使用TRandom3在-e到e范围内取随机数赋值给rnd
      z = f2.Eval(x,y)*(1+rnd);//计算f2函数在x,y处的值，并且在值后乘以模拟误差，Eval计算f2函数值
      dte.SetPoint(i,x,y,z);//在dte中设置点位，即为16行中取出的x,y值，以及19行计算出的z值
      ex = 0.05*my_random_generator.Uniform();//使用TRandom3设置ex和ey，
      ey = 0.05*my_random_generator.Uniform();
      ez = fabs(z*rnd);//取绝对值
      dte.SetPointError(i,ex,ey,ez);//在dte中设置error点位
   }
   // Fit function to generated data
   f2.SetParameters(0.7,1.5);  // set initial values for fit，改变参量，测试程序能否找回原值
   f2.SetTitle("Fitted 2D function");
   dte.Fit(&f2);//根据dte中点位，拟合f2
   // Plot the result
   auto c1 = new TCanvas();
   f2.SetLineWidth(1);
   f2.SetLineColor(kBlue-5);
   TF2   *f2c = (TF2*)f2.DrawClone("Surf1");//DrawClone详细设置，设置图画形式为带颜色标着的面，Surf1选项将TF2对象（但也包括二维直方图）绘制为彩色表面，并在三维画布上使用线框
   TAxis *Xaxis = f2c->GetXaxis();//不太懂
   TAxis *Yaxis = f2c->GetYaxis();
   TAxis *Zaxis = f2c->GetZaxis();
   Xaxis->SetTitle("X Title"); Xaxis->SetTitleOffset(1.5);//设置轴标题偏移距离
   Yaxis->SetTitle("Y Title"); Yaxis->SetTitleOffset(1.5);
   Zaxis->SetTitle("Z Title"); Zaxis->SetTitleOffset(1.5);
   dte.DrawClone("P0 Same");
   // Make the x and y projections
   auto c_p= new TCanvas("ProjCan",
                         "The Projections",1000,400);//这里主要是为了绘制x和y方向上的投影
   c_p->Divide(2,1);
   c_p->cd(1);
   dte.Project("x")->Draw();
   c_p->cd(2);
   dte.Project("y")->Draw();
}