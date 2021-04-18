// Builds a graph with errors, displays it and saves it as
// image. First, include some header files
// (not necessary for Cling)

#include "TCanvas.h"
#include "TROOT.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TLegend.h"
#include "TArrow.h"
#include "TLatex.h"

void macro1(){
    // The values and the errors on the Y axis
    const int n_points=10;
    double x_vals[n_points]=
            {1,2,3,4,5,6,7,8,9,10};
    double y_vals[n_points]=
            {6,12,14,20,22,24,35,45,44,53};
    double y_errs[n_points]=
            {5,5,4.7,4.5,4.2,5.1,2.9,4.1,4.8,5.43};

    // Instance of the graph
    TGraphErrors graph(n_points,x_vals,y_vals,nullptr,y_errs);//TGraphErrors name(采用点数,x值,y值,x error,y error);并且空指针为nullptr,注意以上均为指针
    graph.SetTitle("Measurement XYZ;lenght [cm];Arb.Units");//定义标题内容和X,Y轴标题

    // Make the plot estetically better更详细的设置参见TMarker和TColor类
    graph.SetMarkerStyle(kOpenCircle);
    graph.SetMarkerColor(kBlue);
    graph.SetLineColor(kBlue);

    // The canvas on which we'll draw the graph
    auto  mycanvas = new TCanvas();

    // Draw the graph !
    graph.DrawClone("APE");//A加入图轴；P绘制Point标记；E绘制errors误差；复制函数绘图

    // Define a linear function
    TF1 f("Linear_law","[0]+x*[1]",.5,10.5);//注意此处仅指定了范围，没有指定[0]和[1]的值
    // Let's make the function line nicer
    f.SetLineColor(kRed); f.SetLineStyle(2);
    // Fit it to the graph and draw it
    graph.Fit(&f);//这是一个拟合函数，将h(直线)和gaph(点图，有误差)拟合；并且自动赋予h线参量值
    f.DrawClone("Same");

    // Build and Draw a legend
    TLegend leg(.1,.7,.3,.9,"Lab. Lesson 1");//lab位置(xl,yd,xr,yu,"name");
    leg.SetFillColor(0);
    graph.SetFillColor(0);
    leg.AddEntry(&graph,"Exp. Points");
    leg.AddEntry(&f,"Th. Law");
    leg.DrawClone("Same");

    // Draw an arrow on the canvas
    TArrow arrow(8,8,6.2,23,0.02,"|>");
    arrow.SetLineWidth(2);
    arrow.DrawClone();

    // Add some text to the plot
    TLatex text(8.2,7.5,"#splitline{Maximum}{Deviation}");//#splitline{}{}允许存储多行内容
    text.DrawClone();

    mycanvas->Print("graph_with_law.pdf");//输出为pdf文件
}

int main(){
    macro1();
    }
