// Read the previously produced N-Tuple and print on screen
// its content

void read_ntuple_from_file(){

    // Open a file, save the ntuple and close the file
    TFile in_file("conductivity_experiment.root");//打开加载文件
    TNtuple* my_tuple;in_file.GetObject("cond_data",my_tuple);//定义数组指针my_tuple
        //在文件指针in_file中取得数组cond_data，存在定义好的数组地址my_tuple中
    float pot,cur,temp,pres; float* row_content;//定义四个浮点类型；定义row_content浮点指针

    cout << "Potential\tCurrent\tTemperature\tPressure\n";//输出数据表头
    for (int irow=0;irow<my_tuple->GetEntries();++irow){//取得在数组my_tuple中的数据数量，并且进入循环
        my_tuple->GetEntry(irow);//取第irow个数组
        row_content = my_tuple->GetArgs();//取出数组到row_content中
        pot = row_content[0];
        cur = row_content[1];
        temp = row_content[2];
        pres = row_content[3];
        cout << pot << "\t" << cur << "\t" << temp
             << "\t" << pres << endl;
        }

    }