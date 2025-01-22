/*
 * MModuleStripPairingGreedy.h
 *
 * Copyright (C) by Clio Sleator & Daniel Perez-Becker
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


/*
************ STRIP PARING MODULE GREEDY b ******************
Purpose: Correctly pair strips when multiple interactions occur in a COSI detector, taking into account the possibility of charge sharing between two strips and multiple hits on one strip. It also flags dead strips, noise hits, charge loss, and a hit near the threshold energy.
 
This module is based off of Daniel Perez-Becker's strip pairing code. It has been re-written completely, but is the exact same algorithm and follows Daniel's general idea.

Daniel based the idea is based on M.Bandstra’s greedy algorithm (see his slides if possible), but the code was written from scratch (not a translation of his idl code).

The heart of the greedy algorithm is a matrix that contains the pairing weights of all possible strip combinations. The purpose of the program is to choose the paring of the strips on the X side of the detector with the ones on the Y side that minimizes the total weight of the interactions.

In the greedy approach, this is done by the following steps:

a) First, it selects the matrix element with the lowest weight M(i,j), this matches X-strip-i with Y-strip-j

b) The program subsequently ignores all other matrix elements that are associated either with X-strip-i or Y-strip-j, by reassigning their weight to -1. Normally, these are just the elements that are in the same row as i (M(i,*)) or in the same column as j (M(*,j)). However, it becomes more complicated when charge sharing and multiple hits on a strip are included. (This will be described in more detail down below).

c) The algorithm then selects the next lowest weight of the remaining matrix elements, and repeats the process described in b) until all the strips are matched.

*******

INPUT:
The program takes an MReadOutAssembly* as input. The first function, GetStripHits, fills the following member variables:

vector<vector<int> > stripHits: a list of all the strip numbers that were hit
vector<vector<float> > energy: a list of the energy corresponding to the strip hits
vector<vector<float> > sig: a list of the energy error corresponding to the strip hits

for all of these vectors, vector.at(0) is a vector of x quantities and vector.at(1) is a vector of y quantities. For example, stripHits.at(0) is a vector containing all the x strip hits. energy.at(1) is a vector containing all the y energies.

****************
Strip Sharing:
The program accounts for the possibility of strip sharing by searching stripsHit to see if any adjacent strips were hit (done in function CheckForAdjacentStrips()). If so, a "new strip" number is added to the end of stripsHit. The "new strip" is a combination of the two adjacent strips, and it is indexed by adding 50 to the strip number of the lower strip. For example, if strips 2 and 3 on the x axis are both hit, the strip ID 52 will get added to the end of stripsHit.at(0).

The energy of the combination is the sum of the energies of each adjacent strip. The error can be calculated using Gaussian error propagation: sig.at(axis).at(50+n) = sqrt(sig.at(axis).at(n)^2+sig.at(axis).at(n+1)^2), where axis refers x or y and n refers to the lower adjacent strip number.

*****
Multiple Hits on One Strip:
The program first checks if the number of strip hits on the x strip is the same as that on the y strip. If the number of strip hits on x and y are not equal, the program considers the possibility of multiple hits on whichever side has less strip hits (done in CheckMultipleHits()).

To consider this possibility, the program makes a list of all unordered pairs from stripsHit.at(axis) (including the strip sharing hits). These pairs are labeled by multiplying the smaller strip number by 100 and adding the result to the larger strip number (done in AddMultipleHits(axis)). For example, strip 112 is strip 1 combined with strip 12. Strip 1751 is strip 17 combined with charge sharing between strips 1 and 2.

The energy and error of these combinations is calculated the same way as the strip sharing combinations.

*****
Choosing Final Pairs:
To select the final pairs of x and y strips (done in FindFinalPairs()), the weight matrix must be calculated (done in CalculateWeightMatrix()). The weight matrix is just a matrix with the weights of each combination of x and y strips as elements. The weights are calculated using weight_ij = (xE-yE)*(xE-yE)/((xS*xS)+(yS*yS)), where xE and yE are the x and y energies and xS and yS are the x and y error, or sigma (done in CalculateWeight(i,j).

Once a pair has been chosen, any other x or y strips that conflict with the x or y strip chosen must be eliminated, so that they are not also chosen. For example, imagine three initial strip hits on the x axis on strips 5, 6, and 15. Because there are adjacent strips, stripHits.at(0) would have the elements 5, 6, 15, 55. If there are less x strip hits than y strip hits, then stripsHit.at(0) would have the elements 5, 6, 15, 55, 506, 515, 555, 615, 655, and 1555. If strip 5 is chosen as the x strip in a final pair, "strips" 55, 515, and 1555 should be eliminated, as they can't be chosen.

This example also indicates another problem that must be resolved: strip numbers like 555 and 655 are not valid, because they mean the combination of strip 5 and strip sharing between 5 and 6, and the combination of strip 6 and strip sharing between 5 and 6, respectively. These "bad" strip combinations should be eliminated.

In order to prevent conflicting strips from being chosen, matrices called the kill matrices are created to keep track of which strips "kill", or eliminate, other strips. There is one kill matrix for x and one for y. The kill matrix elements, indexed by strip numbers i and j, are 0 if strip i and strip j can both be chosen and non-zero if strip i and j cannot both be chosen. Each element of the kill matrices is the vector sum of the constituent strips (done in InitializeKillMatrices(), ExpandKillMatrix(axis), CheckForAdjacentStrips(), and AddMultipleHits(axis)). Note that the diagnoal elements of the kill matrices are always set to 1 when the kill matrices are initialized.

After each pair is chosen, the program checks whether the kill matrix element (xStripChosen, anyOtherXStrip) is zero or non-zero. If it's non-zero, all weight matrix elements containing the not chosen strip are set to -1 (done in ConflictingStrips(xIndex, yIndex)).

The kill matrices are also used to get rid of "bad" strip combinations, such as 555 and 655 in the example above. Due to how the kill matrices are made, if the kill matrix element i,j is greater than 1, then strip i is a bad combination. The bad combination information is stored in a vector, badCombinations. badCombinations.at(i) is 0 if stripsHit.at(axis).at(i) is a good combination, and 1 if stripsHit.at(axis).at(i) is a bad combination (done in CheckForBadCombinations()). When the weight matrix is calculated, it checks the badCombinations vector for each weight. If the x and y strips in question are both good combinations, the weight is calculated as normal, but if either are bad combinations, the weight is set to -1 (done in CalculateWeightMatrix()).

With the mechanisms in place to get rid of bad combinations and eliminate conflicting strips, the list of final pairs can be calculated. The program searches for the lowest weight that is still a positive number and selects those strips as the first pair (done in FindMinWeight(), FindFinalPairs()). It then sets the weights of the selected strips to -1 and eliminates any conflicting strips (done in ConflictingStrips(xIndex,yIndex), FindFinalPairs()). This process is repeated until all the weights are -1.

The program checks whether all strips were paired (done in CheckAllStripsWerePaired()). The function returns true if all strips were paired, and false otherwise. Situations where not all of the strips were paired can occur; for example, a noise hit, a dead strip, or a hit with energy close to the threshold energy. NOTE: It should probably add a flag or something if all strips weren't paired, but currently DOES NOT DO THIS!!

 
*****
Output:
The final pairs are then decoded: for example, if strip 55 is chosen, then the program recognizes this as being a combination of strips 5 and 6 (done in DecodeFinalPairs()). Each final pair is then added to an MHit (done in WriteHits(Event, detector number)). In addition, the hit quality (weight), energy (formula from Andreas), and energy resolution (Gaussian error analysis on energy) are added to the MHit.

 
*****
Other things:
Event Quality:
The detector quality factor is the average of all the hit quality factors. The event quality factor is the average of all the detector quality factors. The event quality factor is added to the input MReadOutAssembly (done in CalculateDetectorQuality(), CalculateEventQuality()).


CheckInitialEnergyDifference: STILL BEING WORKED OUT!!
 

*********


Clio Sleator, 2014
 */

#ifndef __MModuleStripPairingGreedy__
#define __MModuleStripPairingGreedy__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<iostream>
#include<vector>
#include<cmath>
#include<algorithm>
#include<map>
#include<limits>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MModule.h"
#include "MGUIExpoStripPairing.h"

// Forward declarations:

////////////////////////////////////////////////////////////////////////////////

using namespace std;

class MModuleStripPairingGreedy : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleStripPairingGreedy();
  //! Default destructor
  virtual ~MModuleStripPairingGreedy();

  //! Create a new object of this class 
  virtual MModuleStripPairingGreedy* Clone() { return new MModuleStripPairingGreedy(); }  
  
  //! Create the expos
  virtual void CreateExpos();
  
  //! Initialize the module
  virtual bool Initialize();

  //! Show the options GUI
  virtual void ShowOptionsGUI();

	//! Read the configuration data from an XML node
	virtual bool ReadXmlConfiguration(MXmlNode* Node);
	//! Create an XML node tree from the configuration
	virtual MXmlNode* CreateXmlConfiguration();

  //!Main data analysis routine, which updates the event to a new level
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

//other functions
  int GetEventInfo(MReadOutAssembly*, int);
  void WriteHits(MReadOutAssembly*, int);
  void ClearMembers();  //clears member vectors in between detectors
  bool CheckInitialEnergyDifference();
  bool CheckForAdjacentStrips(); //checks if any adjacent strips were hit and if so, adds the possibility of charge sharing
  bool CheckMultipleHits(); //checks to see if more x or more y strips were hit, if so, calls AddMuoltipleHits(int)
  void AddMultipleHits(int); //adds possibility of multiple hits on one x (or y) strip by adding all possible combinations of y (or x) strips to vector of y (or x) hits
	void ChargeSharingThreeStrips(int);
	void AddThreeHits(int);
  void InitializeBadCombinations(); //fill badCombinations vector with 0's
  void CheckForBadCombinations(); //fills out badCombinations vector using information from kill matrix
  void PrintXYStripsHit(); //prints out current list x and y strips that were hit, including combinations
	void PrintXYStripsHitOrig();	//prints out original list of x and y strips that were hit, without combinations
  float CalculateWeight(int, int); //calculate weight of one possible combination
  void CalculateWeightMatrix(); //calculate the weight matrix
  vector<int> FindMinWeight(); //find the minimum weight in the weight matrix
  void ConflictingStrips(int, int); //once a pair is chosen, prevents the program from choosing the same strip again
  float FindFinalPairs(); //finds the most likely pairs (or combinations) by creating and analyzing the weight matrix
	void PrintFinalPairs();	//print final pairs
  void InitializeWeightMatrix(); //fill weight matrix with 0's
  void PrintWeightMatrix(); //print out all elements in the weight matrix
  void InitializeKillMatrices(); //fill kill matrices with 0's
  void ExpandKillMatrix(int); //add column and row to the kill matrices
	void PrintKillMatrix(); //print out all elements in the kill matrix
  int CountNegativeElements(); //count number of elements in the weight matrix that are negative (which means they are no longer valid to be chosen as the minimum weight)
  vector<vector<vector<int> > > DecodeFinalPairs(); //put lists of final strip pairings in terms of numbers between 1 and 37
  bool CheckAllStripsWerePaired(); //check that all strips are in the list of final pairs
  float CalculateSigma();
  void DetermineOption(bool);


	int GetStripIndex(int, int);	//for a given strip ID, returns index in stripsHit vector
  float GetEth(); //returns threshold energy
  void SetStripsHit(vector<vector<int> >);
  vector<vector<int> > GetStripsHit();
  void SetFinalPairs(vector<vector<int> >);
  void SetEnergy(vector<vector<float> >);
  vector<vector<float> > GetEnergy();
  void SetSigma(vector<vector<float> >);
  vector<vector<float> > GetSigma();
  vector<vector<float> > GetWeightMatrix();
  vector<vector<int> > GetBadCombinations();
  void SetBadCombinations(vector<vector<int> >);
	int GetNBadCombinations(int);

	//get the mode
	unsigned int GetMode() const { return m_Mode; }
	//set the mode
	void SetMode(unsigned int Mode) { m_Mode = Mode; }

  void CalculateDetectorQuality();
  void CalculateEventQuality(MReadOutAssembly*, int);

	void dummy_func();

  
  // protected methods:
 protected:

   
  // private methods:
 private:
  //! No Copy constructor
  MModuleStripPairingGreedy(const MModuleStripPairingGreedy&) = delete;
  //! No copying itself
  MModuleStripPairingGreedy& operator=(const MModuleStripPairingGreedy&) = delete;



  // protected members:
 protected:
  //! The display of debugging data
  MGUIExpoStripPairing* m_ExpoStripPairing;
 
  //! Need a global strip pairing failed flag
  MString m_StripPairingFailed;

  int m_TotalMatches; //Event Counters 
  int m_NMatches; //Variable Match counter, used to events with a specific numbers of strips involved 
  int m_NBadMatches; //Counts the number of badly matched events

  //for all of these vectors, vector.at(0) = vector of x information
  //and vector.at(1) = vector of y information
  //for example, stripsHit.at(0) = vector containing all of the x strips hit
  //and energy.at(1) = vector containing all of the energies of the y strips hit
  vector<vector<int> > stripsHit; //strip numbers that got a signal
  vector<vector<float> > energy; //energy of the signal, corresponds to stripsHit
  vector<vector<float> > sig; //error of energy measurement, corresponds to stripsHit

  vector<int> nHits; //number of hits on x and y sides, changes to incorporate adjacent strips and multiple hits per strip
  vector<int> nHitsOrig; //number of hits on x and y sides, stays at original number throughout the entire program
	vector<int> nHitsAdj;	//number of pairs of adjacent strips hit
	vector<int> nThreeHitsAdj;	//number of triples of three adj strips hit

  vector<vector<int> > badCombinations; //badCombinations.at(axis).at(i) = 0 when i is a good combination, = 1 when i is a bad combination
                    //a bad combination is one like 151, which is strip 1 combined with strip sharing between strips 1 and 2
  vector<vector<vector<int> > > killMatrix; //is used to indicate which strip numbers cannot be chosen once a particular strip is selected
  vector<vector<float> > weightMatrix; //matrix of all weights of all x and y combinations
  vector<vector<int> > finalPairs; //list of final pairs that are chosen
	vector<vector<float> > finalPairEnergy;
	vector<vector<float> > finalPairRes;

  vector<float> hitQualityFactor; //list of hit quality factors, which are the weights of each hit
  vector<float> energyResolution; //energy resolution of the pairs
  vector<float> hitEnergy;  //energy of hit

	//keep track of hits where charge sharing occurred or
	// strips were hit multiple times
	vector<int> xStripHitMultipleTimes;
	vector<int> yStripHitMultipleTimes;
	vector<int> chargeSharing;

  vector<float> detectorQualityFactors;
  vector<bool> noHits;

	vector<vector<int> > xCombos;
	vector<vector<int> > yCombos;

  float Eth; //threshold energy = 30 keV for now

  // private members:
 private:
	//operation mode
	unsigned int m_Mode;


  unsigned int m_MagicNumberA;
  unsigned int m_MagicNumberB;
  unsigned int m_MagicNumberC;
  unsigned int m_MagicNumberD;
  unsigned int m_MagicNumberE;



#ifdef ___CLING___
 public:
  ClassDef(MModuleStripPairingGreedy, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
