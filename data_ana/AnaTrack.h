#ifndef AnaTrack_h
#define AnaTrack_h

#include <TChain.h>
#include <TFile.h>
#include <TROOT.h>

// Header file for the classes stored in the TTree if any.

class AnaTrack
{
public:
    TTree *fChain;  //!pointer to the analyzed TTree or TChain
    Int_t fCurrent; //!current Tree number in a TChain

    // Fixed size dimensions of array or collections stored in the TTree if any.

    // Declaration of leaf types
    Int_t EventId;
    Float_t TrackData[9][1024];
    Int_t SampleId[1024];

    // List of branches
    TBranch *b_EventId;   //!
    TBranch *b_TrackData; //!
    TBranch *b_SampleId;  //!

    AnaTrack(TTree *tree = 0);
    virtual ~AnaTrack();
    virtual Int_t Cut(Long64_t entry);
    virtual Int_t GetEntry(Long64_t entry);
    virtual Long64_t LoadTree(Long64_t entry);
    virtual void Init(TTree *tree);
    virtual void Loop();
    virtual Bool_t Notify();
    virtual void Show(Long64_t entry = -1);
};

#endif

#ifdef AnaTrack_cxx
AnaTrack::AnaTrack(TTree *tree) : fChain(0)
{
    // if parameter tree is not specified (or zero), connect the file
    // used to generate this class and read the Tree.
    if (tree == 0)
    {

#ifdef SINGLE_TREE
        // The following code should be used if you want this class to access
        // a single tree instead of a chain
        TFile *f = (TFile *)gROOT->GetListOfFiles()->FindObject("Memory Directory");
        if (!f || !f->IsOpen())
        {
            f = new TFile("Memory Directory");
        }
        f->GetObject("track", tree);

#else  // SINGLE_TREE

        // The following code should be used if you want this class to access a chain
        // of trees.
        TChain *chain = new TChain("track", "");
        chain->Add("data_cosmic_ray_track_run0000_det1_1700V.root/track");
        chain->Add("data_cosmic_ray_track_run0001_det1_1700V.root/track");
        chain->Add("data_cosmic_ray_track_run0002_det1_1700V.root/track");
        tree = chain;
#endif // SINGLE_TREE
    }
    Init(tree);
}

AnaTrack::~AnaTrack()
{
    if (!fChain)
        return;
    delete fChain->GetCurrentFile();
}

Int_t AnaTrack::GetEntry(Long64_t entry)
{
    // Read contents of entry.
    if (!fChain)
        return 0;
    return fChain->GetEntry(entry);
}
Long64_t AnaTrack::LoadTree(Long64_t entry)
{
    // Set the environment to read one entry
    if (!fChain)
        return -5;
    Long64_t centry = fChain->LoadTree(entry);
    if (centry < 0)
        return centry;
    if (fChain->GetTreeNumber() != fCurrent)
    {
        fCurrent = fChain->GetTreeNumber();
        Notify();
    }
    return centry;
}

void AnaTrack::Init(TTree *tree)
{
    // The Init() function is called when the selector needs to initialize
    // a new tree or chain. Typically here the branch addresses and branch
    // pointers of the tree will be set.
    // It is normally not necessary to make changes to the generated
    // code, but the routine can be extended by the user if needed.
    // Init() will be called many times when running on PROOF
    // (once per file to be processed).

    // Set branch addresses and branch pointers
    if (!tree)
        return;
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass(1);

    fChain->SetBranchAddress("EventId", &EventId, &b_EventId);
    fChain->SetBranchAddress("TrackData", TrackData, &b_TrackData);
    fChain->SetBranchAddress("SampleId", SampleId, &b_SampleId);
    Notify();
}

Bool_t AnaTrack::Notify()
{
    // The Notify() function is called when a new file is opened. This
    // can be either for a new TTree in a TChain or when when a new TTree
    // is started when using PROOF. It is normally not necessary to make changes
    // to the generated code, but the routine can be extended by the
    // user if needed. The return value is currently not used.

    return kTRUE;
}

void AnaTrack::Show(Long64_t entry)
{
    // Print contents of entry.
    // If entry is not specified, print current entry
    if (!fChain)
        return;
    fChain->Show(entry);
}
Int_t AnaTrack::Cut(Long64_t entry)
{
    // This function may be called from Loop.
    // returns 1 if entry is accepted.
    // returns -1 otherwise.
    return 1;
}
#endif // #ifdef AnaTrack_cxx