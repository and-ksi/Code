// Fill an n-tuple and write it to a file simulating measurement of
// conductivity of a material in different conditions of pressure
// and temperature using branches.

void write_ntuple_to_file_advanced(
   const std::string& outputFileName="conductivity_experiment.root"
   ,unsigned int numDataPoints=1000000){//定义字符串，取地址符为outputFileName,内容为conductivity_experiment.root;定义无符号整形numDataPoints=1000000

   TFile ofile(outputFileName.c_str(),"RECREATE");//定义文件指针ofile，打开outputFileName字符串中名为conductivity_experiment.root的文件，如果不存在该文件就创建一个；将outputFileName地址中的字符串conductivity_experiment.root取出。
   //c_str()：生成一个const char*指针，指向以空字符终止的数组。这个数组应该是string类内部的数组

   // Initialise the TNtuple
   TTree cond_data("cond_data", "Example N-Tuple");//TTree函数定义名为cond_data的tree，地址名相同，title为Example N-Tuple
   //

   // define the variables and book them for the ntuple
   float pot,cur,temp,pres;
   cond_data.Branch("Potential", &pot, "Potential/F");//在Tree.cond_data下创建Branch，分别命名，将各个地址符赋予Branch，但是后面/F不明
   cond_data.Branch("Current", &cur, "Current/F");
   cond_data.Branch("Temperature", &temp, "Temperature/F");
   cond_data.Branch("Pressure", &pres, "Pressure/F");

   for (int i=0;i<numDataPoints;++i){
      // Fill it randomly to fake the acquired data
      pot=gRandom->Uniform(0.,10.)*gRandom->Gaus(1.,0.01);
      temp=gRandom->Uniform(250.,350.)+gRandom->Gaus(0.,0.3);
      pres=gRandom->Uniform(0.5,1.5)*gRandom->Gaus(1.,0.02);
      cur=pot/(10.+0.05*(temp-300.)-0.2*(pres-1.))*
                    gRandom->Gaus(1.,0.01);
      // write to ntuple
      cond_data.Fill();}

   // Save the ntuple and close the file
   cond_data.Write();
   ofile.Close();
}