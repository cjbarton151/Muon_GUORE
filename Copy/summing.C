#include "TSystem.h"

/*
class StoreOne{
public:
  float time;
  int multiplicity;
};

class StoreMore{
public:
  float energysum;
  int detectornum;
  };*/

void summing(void)
{



  //BLOCK 1: ITERATING ON EACH FILE
  //1111111111111111111111111111111111111111111111
  const char* allfilesbeginningwith = "bundled";
  const char* indirectory = ".";

  char* directory = gSystem->ExpandPathName(indirectory);
  void* directoryP = gSystem->OpenDirectory(directory);

  const char* checkedname;
  const char* acceptedname[100];
  int n = 0;
  TString checkedstring;

  while((checkedname = (char*)gSystem->GetDirEntry(directoryP))) {
    checkedstring = checkedname;
    cout << "Checked string: " << checkedstring << endl;
    if(checkedstring.BeginsWith(allfilesbeginningwith))
      acceptedname[n++] = checkedname;
  }

  for(int i = 0;i < n;i++)
    {
    cout << acceptedname[i] << endl;
    //1111111111111111111111111111111111111111111111111



    //Parse number from input file name

    string name = acceptedname[i];
    string numstring = "";

    for(std::string::size_type placer = 0;
	placer < name.size(); ++placer)
      {
	if(isdigit(name[placer]))
	  numstring += name[placer];
      }

    int truenum = 0;

    stringstream preem(numstring);

    preem >> truenum;


    
    
    //BLOCK 2: ITERATING ON EACH TREE
    //2222222222222222222222222222222222222222222222222

    //Variables for iterating on trees
    const char *keyname;
    TFile currentfile(Form("%s",acceptedname[i]));
    currentfile.Print();
    TIter nexto(currentfile.GetListOfKeys());
    TKey *key;
    TTree *T;
    //Comparands 
    bool first = true;
    double eventoriginal = -2;
    double timeoriginal = 0;
    int hitcounter = 0;
    //Input tree info
    int entries = 0;
    int detectornumber = 0;
    int eventnumber = 0;
    double time = 0;
    double energy = 0;//for setting branch address

    //Output tree info
    double energysum = 0;
    vector<double> energyvector;
    vector<int> detectorvector;
    int hit = 0;
    TTree *sumtree = new TTree("sumtree","sumtree");
    sumtree->Branch("multiplicity",&hitcounter);
    sumtree->Branch("hit",&hit);
    sumtree->Branch("starttime",&timeoriginal);
    sumtree->Branch("energysum",&energyvector);
    sumtree->Branch("detector",&detectorvector);
    //Deprecated
    //StoreOne header;
    //StoreMore body;
    
    while ((key=(TKey*)nexto()))
      {
	if (strcmp(key->GetClassName(),"TTree")) continue; //do not use keys that are not trees
	currenttree = (TTree*)currentfile.Get(key->GetName()); //fetch the Tree header in memory
	keyname = key->GetName();



	
	//TREE BY TREE PROCESSING STARTS HERE
	//Each tree is already a bundled tree, ergo, they all share
	//G4eventnumber, time block, and detectornumber
	//Each tree would be a 'hit' in MAJORANA
	//I need to sum each tree's energies to get hit energy
	//To get multiplicity... I need all trees with same g4event#, time
	//To correlate these, I need to make an ntuple tree in the right way
	//This will require pre-filled vectors to do right.
	
	entries = currenttree->GetEntries();
	//cout << entries << endl;

	currenttree->SetBranchAddress("time", &time);
	currenttree->SetBranchAddress("energy", &energy);
	currenttree->SetBranchAddress("detectornumber", &detectornumber);
	currenttree->SetBranchAddress("eventnumber", &eventnumber);
	
	currenttree->GetEntry(0);
	

	//If [hit] is changing, write to the output tree,
	//flush the vectors and multiplicity, grab new value for time
	//[hit] can change if eventnumber OR time block changes
	//If [hit] is NOT changing, fill the vectors and move on
	//after incrementing multiplicity, of course

	if(first==true)//Very first tree comparison, initialize
	  {
	    timeoriginal = time;
	    eventoriginal = eventnumber;
	    hitcounter = 1;
	    hit = 1;
	    first=false;

	    for(int j = 0;j<entries;j++)
	      {
		currenttree->GetEntry(j);
		energysum+=energy;
	      }
	    energyvector.push_back(energysum);
	    detectorvector.push_back(detectornumber);
	    energysum = 0;
	  }//if(first

	else if(time>(timeoriginal+20000)||eventnumber!=eventoriginal)
	  {//Hit change, write and flush and grab
	    //write
	    sumtree->Fill();
	    //flush
	    energyvector.clear();
	    detectorvector.clear();
	    //grab
	    hitcounter=1;//The current tree is being added, so it is 1
	    timeoriginal=time;
	    eventoriginal = eventnumber;
	    hit++;
	    for(int j = 0;j<entries;j++)
	      {
		currenttree->GetEntry(j);
		energysum+=energy;
	      }
	    energyvector.push_back(energysum);
	    detectorvector.push_back(detectornumber);
	    energysum = 0;
	  }//Hit change


	
	else//same hit, add to the vectors and multiplicity only
	  {
	    for(int j = 0;j<entries;j++)
	      {
		currenttree->GetEntry(j);
		energysum+=energy;
	      }
	    energyvector.push_back(energysum);
	    detectorvector.push_back(detectornumber);
	    energysum = 0;
	    hitcounter++;
	  }//Same hit
	
	
	//TREE BY TREE PROCESSING ENDS HERE
	delete T;
      }//while key

    sumtree->Fill();//Not 100% sure this is necessary

    TFile *output = new TFile(Form("sumtree%i.root",truenum),"recreate");
    sumtree->Write();
    output->Close();
    
    }//for int i = 0
  //222222222222222222222222222222222222222222222222222222222

  //One final fill for the very last hit

}//EOF


//Store garbage methods down here
  /*  TChain *chain1 = new TChain();
  chain1->Add("/home/CJ.Barton/Muon_GUORE/Truon_GUORE/simfiles/fakefiles/output*.root");
  cout << chain1->GetEntries() << endl;
  while(chain1->GetFile())
    {

      TFile *file1 = chain1->GetFile();

      cout << fTree->GetEntries() << endl;
      
    }
  

  TObjArray *allfiles=chain1->GetListOfFiles();
TIter nextfile(allfiles);
TChainElement *link1=0;
while ((link1=(TChainElement*)nextfile()))
  {
    TFile f(link1->GetTitle());
    //    ... do something with f ...
    cout << fTree->GetEntries() << endl;
  }*/



    //    TFile currentfile(Form("%s",acceptedname[i]),"READ");

    //TChain *chain1 = new TChain("fTre*");
    //chain1->Add(Form("%s",acceptedname[i]));
    //cout << chain1->GetEntries() << endl;
