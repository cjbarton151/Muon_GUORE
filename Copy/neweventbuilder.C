#include <iostream>
#include <algorithm>
#include <iterator>
#include<math.h>
#include <stdlib.h>
#include <stdio.h>

void neweventbuilder(int startnumber)
{

  //Take a time-sorted output file (with G4events together also
  //and bundle hits within 20us of one another.
  //Input is in ns, so what we're really interested in is
  //events within time of 20,000 of one another.
  //Once bundled, generate a tree and shove the bundled
  //RealEvent into it and name the tree in a way that's
  //consistent with the timing window it covers.

  //I considered also working the G4event# into the naming
  //convention of the RealEvent trees, but then realized a
  //similar way to look at them individually could be accom-
  //plished by creative usage of TChains and the eventnumber
  //variable.


  //Clean mode means that entries with time>10^14 ns
  //(a.k.a 10^5 s a.k.a. 3 hours) will be assigned an artificial
  //'same' time. ALSO, entries with energy depositions <5 keV
  //will be stripped from the file. All these discriminators
  //are modifiable right here with these constant values.

  bool cleanmode = true;
  double energycutoff = 0.0001; //keV
  double timecutoff = 100000000000000; //ns


  //BLOCK 1: ITERATING ON EACH FILE
  //1111111111111111111111111111111111111111111111
  const char* allfilesbeginningwith = Form("output%i", startnumber);
  const char* indirectory = ".";

  char* directory = gSystem->ExpandPathName(indirectory);
  void* directoryP = gSystem->OpenDirectory(directory);

  const char* checkedname;
  const char* acceptedname[100];
  int wan = 0;
  TString checkedstring;

  while((checkedname = (char*)gSystem->GetDirEntry(directoryP)))
    {
      checkedstring = checkedname;
      cout << "Checked string: " << checkedstring << endl;
      if(checkedstring.BeginsWith(allfilesbeginningwith))
	{
	  acceptedname[wan] = checkedname;
	  wan++;
	}
    }

  cout << wan << endl << wan << endl;

  
  for(int iwan = 0;iwan < wan;iwan++)
    {
    cout << acceptedname[iwan] << endl;
    
    //1111111111111111111111111111111111111111111111111

    //TFile *fileo = new TFile(Form("bundled%s",acceptedname[iwan]), "read")
    //  if(fileo) continue;  

  //changeo
    TFile input(Form("%s",acceptedname[iwan]),"read");
    TFile output(Form("bundled%s",acceptedname[iwan]),"recreate");
  input.cd();
  
  double time = 0;
  int eventnumber = 0;
  int detectornumber = 0;
  double energy = 0;

  fTree->SetBranchStatus("*",0);
  fTree->SetBranchStatus("time",1);
  fTree->SetBranchStatus("eventnumber",1);
  fTree->SetBranchStatus("detectornumber",1);
  fTree->SetBranchStatus("energy",1);

  
  fTree->SetBranchAddress("time", &time);
  fTree->SetBranchAddress("eventnumber", &eventnumber);
  fTree->SetBranchAddress("detectornumber", &detectornumber);
  fTree->SetBranchAddress("energy", &energy);


  
  int entries = fTree->GetEntries();
  
  //Okay, here's how I'm going to handle the detectornumber matching.
  //Create an empty array of size == number of steps in current
  //G4event, just like I did before.This time though, the array will
  //be a boolean which prevents double counting.

  //Then, for each step in current G4event, check if its entry in the
  //array has been flipped. If not, start looking for 'partner' steps.
  //These will be steps with same detectornumber (and G4event obviously)
  //within 20us of the step in question (divided into even increments,
  //i.e. a step at 294 us would fall into the 280-300 category, and even
  //though a step at 305us is technically within 20us of it, it falls out-
  //side the 280-300 us window.

  //Make a list of array sizes, like in timesorter.C

  fTree->GetEntry(0);
  cout << "Entries in current input tree: " << entries << endl;

  int checkevent = eventnumber; //initializer for array sizer
  int numberofstepsinevent = 0;
  int indexsize[100] = {0}; //each G4event needs its own index size
                            //Maximum of 100 events with hits possible
                            //but should be much less than this
  int indexsizemover = 0;
  int masterincrementer = 0;
  int tempincrementer = 0; //These are used to control place in the tree

  
  int i = 0;
  while(i<entries) //determines number of hits in each G4event
    {
      fTree->GetEntry(i);

      if(eventnumber==checkevent)
        numberofstepsinevent++;

      else
        {
          indexsize[indexsizemover] = numberofstepsinevent;
          numberofstepsinevent = 1;
          checkevent = eventnumber;
          indexsizemover++;
        }

        i++;

    }

  indexsize[indexsizemover] = numberofstepsinevent; //to catch the last one

  cout << "Number of entries determined" << endl;
  //to check indexsize is working properly
   for(int j = 0;j<100;j++)
    {
      if(indexsize[j])
        cout << indexsize[j] << endl;

    }

   //GOOD
   for (int k = 0; k<indexsizemover+1;k++)
     {
   
   //Define boolean array to make sure bundled events aren't double counted
       std::vector<int>counted;
       counted.clear();
       counted.resize(indexsize[k],0);
       //A lot counts on this array so I'm going to explain it well
       //A value of zero means the below loops haven't decided that
       //the step entry equal to (tempincrementer+L) exists yet
       //Once they decide it exists, it needs to be compared to every
       //entry below it. So it's marked with a '2' to indicate 'hey,
       //this entry is the active one'. Once an entry with the same
       //detectornumber and same time window is found, it is also
       //given a '2'. Once all entries have been cycled through,
       //the index is looped over again, any entry with a '2' is added
       //to a NEW event tree, the '2' is changed to a '1' to signify
       //that it's been added, and at the end of this one time index loop
       //the new tree is written to the output file.
       
       
       tempincrementer = masterincrementer;

       //make a 2D vector of stepnumbers and times
       //use this vector in the choosing loop  in 20 or so lines

       std::vector<double> timechooser;
       timechooser.clear();
       timechooser.resize(indexsize[k],0);

       std::vector<int> detchooser;
       detchooser.clear();
       detchooser.resize(indexsize[k],0);
       
       for(int onid = 0; onid < indexsize[k]; onid++)
	 {

	   fTree->GetEntry(onid + tempincrementer);
	     timechooser[onid] = time;
	     detchooser[onid] = detectornumber;
	     
	 }


       cout << "Processing event of size " << indexsize[k] << endl;
       while(std::accumulate(counted.begin(), counted.end(), 0)!=indexsize[k])
	 {
	   // cout << "INDEX SIZE " << indexsize[k] << endl;
	   if(std::accumulate(counted.begin(), counted.end(), 0)%2==0)
	     cout << "Number filled: " << std::accumulate(counted.begin(), counted.end(), 0) << endl;


	   
	   //Find lowest time value not written
	   double lowesttime = 0;
	   int loweststep = 0;
	   double timeoriginal = 0;
	   int detectororiginal = 0;
	   
	   for(int Q = 0;Q<indexsize[k];Q++)
	     {//time grabbing loop

	       if(counted[Q]) continue;//No need to compare filled values
	       //fTree->GetEntry(tempincrementer+Q);		 

	       if(!lowesttime || timechooser[Q]<lowesttime)
		 {
		   lowesttime = timechooser[Q];
		   loweststep = Q;
		 }
	     }

	   counted[loweststep]=2;
	 
	   //Grab comparands
	   //fTree->GetEntry(tempincrementer+loweststep);
	   timeoriginal = lowesttime;
	   detectororiginal = detchooser[loweststep];

	   //clean mode
	   if(cleanmode && timeoriginal>timecutoff)
	     timeoriginal=timecutoff;

	   //Compare all non-written values
	   for(Q=0;Q<indexsize[k];Q++)
	     {

	        if(cleanmode && timechooser[Q]>timecutoff)
		 {
		   timechooser[Q] = timecutoff+1;
		 }
	       
	       if(counted[Q]||TMath::Abs(timechooser[Q]-timeoriginal)>20000) continue;//dont compare written values or values definitely outside the time range

	       if(detectororiginal!=detchooser[Q]) continue;
	       
	       //	       fTree->GetEntry(tempincrementer+Q);
	      	  
	       if(detchooser[Q]==detectororiginal && timeoriginal<=timechooser[Q]<=timeoriginal+20000 && Q!=loweststep)
		 {//We have a match!

		   if(counted[Q])
		     cout << "SERIOUS DOUBLE COUNT ERROR !!!" << endl;
		   
		   counted[Q]=2;
		 }
	       
	     }

	   //Now to go through and write all counted matches



	   
	   //CONVERTS TIMEORIGINAL TO A STRING
	   std::ostringstream timestream;
	   if(timeoriginal>=1000000000)
	     timestream.precision(15);
	   double splo2 = timeoriginal;
	   timestream << splo2;
	   std::string timestring = timestream.str();
	   
	   //CONVERTS EVENTNUMBER TO A STRING
	   std::ostringstream eventstream;
	   eventstream << eventnumber;
	   std::string eventstring = eventstream.str();
	   
	   //CONVERTS DETECTORNUMBER TO A STRING
	   std::ostringstream detectorstream;
	   detectorstream << detectororiginal;
	   std::string detectorstring = detectorstream.str();
	   
	   
	   string prefix = "bundledtree";
	   string hyphenator = "__";
	   string suffix = ".root";
	   
	   /*
	   // CONVERTS TIMESTRING TO A FIXED LENGTH CHAR ARRAY
	   //CURRENTLY DEPRECATED AND WILL HOPEFULLY REMAIN SO
	   cout <<"DOUBLE TO STRING: " << timestring << endl;
	   
	   int len = timestring.length(); 
	   
	   // declaring character array
	   char time_array[31]; 
	   
	   // copying the contents of the 
	   // string to char array
	   strcpy(time_array, str.c_str()); 
	   
	   for (int q=0; q<len; q++)
	   cout << time_array[q];    
	   cout << endl;*/
	   
	   string treename = prefix + eventstring + hyphenator + timestring + hyphenator + detectorstring + suffix;
	   
	   int len = treename.length(); 
	   
	   // declaring character array
	   char tree_array[60]; 
	   
	   // copying the contents of the 
	   // string to char array
	   strcpy(tree_array, treename.c_str()); 
	   
	   // for (int q=0; q<len; q++)
	   //   cout << tree_array[q];    
	   //   cout << endl;
	   
	   
	   
	   // cout << "TREE NAME: " << treename << endl;
	   
	   TTree *bundledtree = (TTree*)fTree->CloneTree(0);
	   bundledtree->SetName(Form("%s",tree_array));

	   for(int m=0;m<indexsize[k];m++)
	     {
	       if(counted[m]==2)
		 {
		   fTree->GetEntry(tempincrementer+m);
		   if(!cleanmode)
		     bundledtree->Fill();
		   else if(cleanmode && energy>=energycutoff)
		     bundledtree->Fill();
		   //Won't fill in cleanmode if energy<cutoff
		   //Not the most efficient place to implement...
		   //But the safest
		   
		   counted[m]=1;
		 }
	     }//Writing loop obviously
		   
	   output.cd();	   

	   if(bundledtree->GetEntries())
	     bundledtree->Write();
	   //The above may seem a weird inclusion, but
	   //with cleanmode on, it's possible to create
	   //a tree but have no entries that pass the
	   //cutoffs to be written to it. So this
	   //prevents empty trees from being added.
	   delete bundledtree;
	   
	   input.cd();
	   	      
	   
	 }//Goes through current G4event until done
       
 masterincrementer = masterincrementer+indexsize[k];
       //master should be changed down here somewhere

     }//Switches G4events/for(int k
   
    }//iterates on each file
  
}//EOF

