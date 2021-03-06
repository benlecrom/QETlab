// *******************************************************************
// * License and Disclaimer                                          *
// *                                                                 *
// * This software is copyright of Geant4 Associates International   *
// * Ltd (hereafter 'G4AI'). It is provided under the terms and      *
// * conditions described in the file 'LICENSE' included in the      *
// * software system.                                                *
// * Neither the authors of this software system nor G4AI make any   *
// * representation or warranty, express or implied, regarding this  *
// * software system or assume any liability for its use.            *
// * Please see the file 'LICENSE' for full disclaimer and the       *
// * limitation of liability.                                        *
// *******************************************************************
// $Id$
// John Allison  22nd May 2017

#include "Tangle2SteppingAction.hh"

#include "Tangle2RunAction.hh"
#include "Tangle2Data.hh"

#include "G4Step.hh"
#include "G4VProcess.hh"
#include "G4MTRunManager.hh"
#include "G4EventManager.hh"
#include "G4TrackingManager.hh"
#include "G4SteppingManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4Gamma.hh"
#include "G4Threading.hh"

Tangle2SteppingAction::Tangle2SteppingAction
(Tangle2RunAction* runAction)
{}

void Tangle2SteppingAction::BeginOfEventAction()
{
}

void Tangle2SteppingAction::EndOfEventAction()
{
}

void Tangle2SteppingAction::UserSteppingAction(const G4Step* step)
{
   G4StepPoint* preStepPoint = step->GetPreStepPoint();
  G4StepPoint* postStepPoint = step->GetPostStepPoint();

  G4double eDep = step->GetTotalEnergyDeposit();

  G4VPhysicalVolume* prePV = preStepPoint->GetPhysicalVolume();
  G4VPhysicalVolume* postPV = postStepPoint->GetPhysicalVolume();

  G4ThreeVector prePos = preStepPoint->GetPosition();
  G4ThreeVector postPos = postStepPoint->GetPosition();

  G4ThreeVector postMomentumDir = postStepPoint->GetMomentumDirection();

  G4Track* track = step->GetTrack();
  G4ParticleDefinition* particleDefinition = track->GetDefinition();
  const G4VProcess* creatorProcess = track->GetCreatorProcess();

  const G4VProcess* processDefinedStep = postStepPoint->GetProcessDefinedStep();
  
  // Always use a lock when writing a file in MT mode
  G4AutoLock lock(&Tangle2::outFileMutex);
  static G4bool first = true;
  if (first) {
    first = false;
    // Write header
    Tangle2::outFile
    << "#,TrackID,name,prePV,copyNo,prePos_X,prePos_Y,prePos_Z,postPV,copyNo"
    ",postPos_X,postPos_Y,postPos_Z,creatorProcess,processDefinedStep,Mom_X,Mom_Y,Mom_Z,eDep/MeV"
    << std::endl;
  }
  Tangle2::outFile
  << ',' << track->GetTrackID()
  << ',' << particleDefinition->GetParticleName()
  << ',' << (prePV? prePV->GetName(): "none")
  << ',' << (prePV? prePV->GetCopyNo(): 0)
  << ',' << prePos[0]
  << ',' << prePos[1]
  << ',' << prePos[2]
  << ',' << (postPV? postPV->GetName(): "none")
  << ',' << (postPV? postPV->GetCopyNo(): 0)
  << ',' << postPos[0]
  << ',' << postPos[1]
  << ',' << postPos[2]
  << ',' << (creatorProcess? creatorProcess->GetProcessName(): "none")
  << ',' << processDefinedStep->GetProcessName()
  << ',' << postMomentumDir[0]
  << ',' << postMomentumDir[1]
  << ',' << postMomentumDir[2]
  << ',' << eDep/MeV
  << std::endl;

  if (eDep > 0){
    (postPV? Tangle2::eDepCryst[postPV->GetCopyNo()] += eDep : eDep = 0);}

  if ((prePos == Tangle2::posA) && (track->GetTrackID() == 1)){
    Tangle2::posA = postPos - prePos;
    if (processDefinedStep->GetProcessName() == "compt"){
      Tangle2::posA = postMomentumDir;}
  }

  if ((prePos == Tangle2::posB) && (track->GetTrackID() == 2)){
    Tangle2::posB = postPos - prePos;
    if (processDefinedStep->GetProcessName() == "compt"){
      Tangle2::posB = postMomentumDir;}
  }

  if (processDefinedStep->GetProcessName() == "compt"){
    Tangle2::nb_Compt += 1;}

  return;
}
