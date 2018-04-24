///////////////////////////////////////////////////////////////////////////
// This code implementation is the intellectual property of the
// ton-scale 0vbb in Germanium collaboration. It is based on Geant4, an
// intellectual property of the RD44 GEANT4 collaboration.
//
// *********************
//
// Neither the authors of this software system, nor their employing
// institutes, nor the agencies providing financial support for this
// work make any representation or warranty, express or implied,
// regarding this software system or assume any liability for its use.
// By copying, distributing or modifying the Program (or any work based
// on the Program) you indicate your acceptance of this statement,
// and all its terms.
//////////////////////////////////////////////////////////////////////////////
/// \file ShowerGenerator.cc
/// \brief Implementation of the ShowerGenerator class

#include "ShowerGenerator.hh"


#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"
#include <math.h>

#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4Neutron.hh"
#include "G4Alpha.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"

#include "G4NeutrinoE.hh"
#include "G4NeutrinoMu.hh"
#include "G4AntiNeutrinoE.hh"
#include "G4AntiNeutrinoMu.hh"
#include "G4KaonZeroLong.hh"
#include "G4KaonMinus.hh"
#include "G4KaonPlus.hh"
#include "G4PionMinus.hh"
#include "G4PionPlus.hh"
#include "G4AntiNeutron.hh"
#include "G4Deuteron.hh"

#include "TFile.h"
#include "TStyle.h"
#include "TTree.h"
#include "TChain.h"
#include "TRandom3.h"
#include <vector>
#include <string>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ShowerGenerator::ShowerGenerator()
{
  const char* muon_path = "/home/CJ.Barton/Muon_GUORE/Truon_GUORE/mac";
  inputfile = new TFile(Form("%s/SmallMuonFile.root", muon_path) ,"READ");
  datatree = (TTree*)inputfile->Get("muontree");

  //  TRandom3 *ran3 = new TRandom3(50000);
  //ran3->SetSeed(time(NULL));

  
  max_entries = datatree->GetEntries();

  start_energy = 0;    //muon start energy in surface GeV
  start_costheta = 0;    //muon start angle
  particle_energy = 0;
  particle_momentumX = 0;  //GeV
  particle_momentumY = 0;  //GeV
  particle_momentumZ = 0;  //GeV
  weight = 0;
  
   datatree->SetBranchAddress("startenergy", &start_energy);
   datatree->SetBranchAddress("startcostheta", &start_costheta);
   datatree->SetBranchAddress("energy", &start_energy);
   datatree->SetBranchAddress("px", &particle_momentumX);
   datatree->SetBranchAddress("py", &particle_momentumY);
   datatree->SetBranchAddress("pz", &particle_momentumZ);
   datatree->SetBranchAddress("x", &x);
   datatree->SetBranchAddress("y", &y);
   datatree->SetBranchAddress("z", &z);
   datatree->SetBranchAddress("weight", &weight);
	 G4cout << max_entries << " " << inputfile->GetName() << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ShowerGenerator::~ShowerGenerator()
{
  delete inputfile;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ShowerGenerator::GeneratePrimaryVertex(G4Event* anEvent)
{
  // Select random entry within file
  //Flat version (stable:) int ev_ID = (int)(G4UniformRand()*max_entries);
  double comparator = G4UniformRand(); //should be between 0 and 1
  double comparee = 0;
  int counter = 0;
  while(comparee<comparator)
    {
      datatree->GetEntry(counter);
      comparee += weight; //weight is normalized so that total sum is 1.0
      if(counter>=max_entries)
	G4cout << G4endl << "Sampling method error" << G4endl; //should never happen
      counter++;
    }

  datatree->GetEntry(counter);

  //for all shower particles


  particle_time = 0.0*s;

  double xx = x;
  double yy = y;
  double zz = z;
  
  particle_position.setX(xx*m);
  particle_position.setY(yy*m);
  particle_position.setZ(zz*m);

	G4PrimaryVertex* vertex = new G4PrimaryVertex(particle_position,particle_time);

	G4ParticleDefinition* particle_definition = 0;
       particle_definition = G4MuonMinus::MuonMinusDefinition();

  // Set momenta, no rotations done here (unlike in MaGe)
       double px_MJD = particle_momentumX;
       double py_MJD = particle_momentumY;
       double pz_MJD = particle_momentumZ;
       //       double pEnergy = particle_energy;


  G4ThreeVector momentum(px_MJD*GeV,py_MJD*GeV,pz_MJD*GeV);

	G4PrimaryParticle* thePrimaryParticle =
    new G4PrimaryParticle(particle_definition,
						  px_MJD*GeV,
						  py_MJD*GeV,
						  pz_MJD*GeV);
	vertex->SetPrimary(thePrimaryParticle);
  // vertex->SetWeight(Distribution(start_energy,start_costheta));  // Totally useless for now

  anEvent->AddPrimaryVertex(vertex);


  // Output information, maybe not so useful


}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//weighting with incident muon distribution
//https://escholarship.org/uc/item/6jm8g76d#page-3
double ShowerGenerator::Distribution(double Energy, double CosTheta)
{
  double CosThetaStar = sqrt(
	(pow(CosTheta,2) + pow(0.102573,2) -0.068287*pow(CosTheta,0.958633)+0.0407253*pow(CosTheta,0.817285) )/
	(1+pow(0.102573,2)-0.068287+0.0407253));

  double I;
  I = 0.14*pow(Energy*(1+3.64/(Energy*pow(CosThetaStar,1.29))),-2.7)
	  *((1./(1+(1.1*Energy*CosThetaStar)/115.))+(0.054/(1+(1.1*Energy*CosThetaStar)/850.)));

  return (I);
}


/*

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

WeightedGenerator::WeightedGenerator()
{
  const char* muon_path = "/home/CJ.Barton/Muon_GUORE/Truon_GUORE/mac";
  inputfile = new TFile(Form("%s/SmallMuonFile.root", muon_path) ,"READ");
  datatree = (TTree*)inputfile->Get("muontree");

  max_entries = datatree->GetEntries();

  startenergy = 0;    //muon start energy in surface GeV
  startcostheta = 0;    //muon start angle
  energy = 0;
  px = 0;  //GeV
  py = 0;  //GeV
  pz = 0;  //GeV
  x = 0;
  y = 0;
  z = 0;

   datatree->SetBranchAddress("startenergy", &startenergy);
   datatree->SetBranchAddress("startcostheta", &startcostheta);
   datatree->SetBranchAddress("energy", &energy);
   datatree->SetBranchAddress("px", &px);
   datatree->SetBranchAddress("py", &py);
   datatree->SetBranchAddress("pz", &pz);
   datatree->SetBranchAddress("x", &x);
   datatree->SetBranchAddress("y", &y);
   datatree->SetBranchAddress("z", &z);

	 G4cout << max_entries << " " << inputfile->GetName() << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

WeightedGenerator::~WeightedGenerator()
{
  delete inputfile;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void WeightedGenerator::GeneratePrimaryVertex(G4Event* anEvent)
{
  // Select random entry within file
  int ev_ID = (int)(G4UniformRand()*max_entries);
  datatree->GetEntry(ev_ID);

  particletime = 0.0*s;
  double xx = x[ev_ID];
  double yy = y[ev_ID];
  double zz = z[ev_ID];
  double pxx = px[ev_ID];
  double pyy = py[ev_ID];
  double pzz = pz[ev_ID];
  particleposition.setX(xx*m);
  particleposition.setY(yy*m);
  particleposition.setZ(zz*m);

	G4PrimaryVertex* vertex = new G4PrimaryVertex(particleposition,particletime);
	
 
	G4ThreeVector momentum(pxx*GeV,pyy*GeV,pzz*GeV);
G4ParticleDefinition* particledefinition = G4MuonMinus::MuonMinusDefinition();
	G4PrimaryParticle* thePrimaryParticle =
    new G4PrimaryParticle(particledefinition,
			  pxx*GeV,
			  pyy*GeV,
			  pzz*GeV);
	vertex->SetPrimary(thePrimaryParticle);
  // vertex->SetWeight(Distribution(start_energy,start_costheta));  // Totally useless for now

  anEvent->AddPrimaryVertex(vertex);


  // Output information, maybe not so useful
   G4cout << "Entry #: " << ev_ID << G4endl;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//weighting with incident muon distribution
//https://escholarship.org/uc/item/6jm8g76d#page-3
double WeightedGenerator::Distribution(double Energy, double CosTheta)
{
  double CosThetaStar = sqrt(
	(pow(CosTheta,2) + pow(0.102573,2) -0.068287*pow(CosTheta,0.958633)+0.0407253*pow(CosTheta,0.817285) )/
	(1+pow(0.102573,2)-0.068287+0.0407253));

  double I;
  I = 0.14*pow(Energy*(1+3.64/(Energy*pow(CosThetaStar,1.29))),-2.7)
	  *((1./(1+(1.1*Energy*CosThetaStar)/115.))+(0.054/(1+(1.1*Energy*CosThetaStar)/850.)));

  return (I);
}
*/
