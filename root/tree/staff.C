#include <TStyle.h>
#include <TTree.h>
#include <stdio.h>
#include <TFile.h>


void staff()
{
    // Create the structure to hold the variables for the branch.
    struct staff_t
    {
        Int_t cat;
        Int_t division;
        Int_t flag;
        Int_t age;
        Int_t service;
        Int_t children;
        Int_t grade;
        Int_t step;
        Int_t nation;
        Int_t hrweek;
        Int_t cost;
    };
    staff_t staff;

    // Open the ASCII file.
    FILE *fp = fopen("staff.dat", "r");
    char line[81];

    // Create a new ROOT file.
    TFile *f = new TFile("staff.root", "RECREATE");

    // Create a TTree.
    TTree *tree = new TTree("T", "Staff data from ASCII file");

    // Create one branch with all information from the structure.
   tree->Branch("staff",&staff.cat,"cat/I:division:flag:age:service:
   children:grade:step:nation:hrweek:cost");

// Fill the tree from the values in ASCII file.
   while (fgets(&line,80,fp)) {
        sscanf(&line[0], "%d%d%d%d", &staff.cat, &staff.division,
               &staff.flag, &staff.age);
        sscanf(&line[13], "%d%d%d%d", &staff.service, &staff.children,
               &staff.grade, &staff.step);
        sscanf(&line[24], "%d%d%d", &staff.nation, &staff.hrweek,
               &staff.cost);
        tree->Fill();
   }

// Check what the tree looks like.
   tree->Print();
   fclose(fp);
   f->Write();
}