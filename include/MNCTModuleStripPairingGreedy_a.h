/*
 * MNCTModuleStripPairingGreedy_a.h
 *
 * Copyright (C) by Daniel Perez-Becker
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


/*
************ STRIP PARING MODULE - GREEDY 4C ******************

Purpose: Correctly pair strips when multiple interactions occur in a NCT detector, taking into account the possibility of charge sharing between two strips and multiple hits on one strip.

The idea is based on M.Bandstra’s greedy algorithm (see his slides if possible), but the code was written from scratch (not a translation of his idl code). 

The heart of the greedy algorithm is a matrix that contains the paring weighs of all possible strip combinations. The purpose of the program is to choose the paring of the strips on the X side of the detector with the ones on the Y side that minimizes the total weight of the interactions. 

In the greedy approach, this is done by the following steps:

a) First, it selects the matrix element with the lowest weight M(i,j), this matches X-strip-i with Y-strip-j

b) The program subsequently ignores all other matrix elements that are associated either with X-strip-i or Y-strip-j, by reassigning their weight to a very large number (I call this process killing). Normally, these are just the elements that are in the same row as i (M(i,*)) or in the same column as j (M(*,j)), but as we include charge sharing and multiple hits on a strip, things become more complicated. To keep track of this, each strip keeps track of all the strips it is “licensed to kill” once it has been selected. 

c) The algorithm then selects the next lowest weight of the remaining matrix elements, and repeats the process described in b) until all the strips are matched.

*******

INPUT:  The program has the following input variables: 
int x[n]  n-dimensional vector containing the fired strips on the X-side of the detector. 	Obviously n varies for different interactions
int y[m] m-dimensional vector for the Y-side.
float x_e[n], x_sig[n] measured energy and sigma for each X side strip, needed to compute the weight. 
float y_e[m], y_sig[m] same for Y side strips.

****************
The most basic program would simply generate the weight matrix M(n,m) between the elements above and go through steps a) to c). This would not consider the possibility of charge sharing or multiple hits on one strip. Charge sharing and multiple hits on one strip are handled on two separate steps in the program (X1 and X2). 

First, to account for the possibility of charge sharing, a new set of variables is created, labeled x1, x1_e, x1_sig, etc. The dimensions of these variables is computed by counting the number of times two consecutive strips are fired (i.e. (x[i+1]-x[i]==1) and adding this number to the original dimension of these variables (n for our example).

The first n elements of x1 are the same same as for x, and the new ones contain the information of charge sharing. The shared strips are assigned numbers grater than 50. They are determined by adding 50 to the strip number of the lower strip that is sharing charge. For example, strip 51 corresponds to strips 1 and 2 sharing charge, strip 52 stands for strip 2 and 3 sharing charge, all the way up to 86 which stands for strip 36 and 37 sharing charge.       

Continuing with our example, the charge of strip 51 is just the sum of the charge of strip 1 and strip 2, and the sigmas are added x1_sig=sqrt(x_sig[k]^2 +x_sig[k+1]^2)

At this point I create a new matrix called kill_x1, which contains information on which strips a given strip is licensed to kill if chosen by the program as a pair element. This is easiest explained in an example: Imagine you had 3 firings on the X side with strips 5,6, and 15 firing. X would be a 3 dimensional vector with entries X=[5,6,15]. As described above, X1 would be 4 dimensional containing the possibility of 5 and 6 charing charge, labeled strip 55. X1=[5,6,15,55]. All strips are entitled to “commit suicide”, but in addition, strip 5 is allowed to kill strip 55 (so is strip 6) and strip 55 is entitled to kill 5 and 6. Note as a general rule that if strip A is licensed to kill strip B, and strip C is licensed to kill strip B, then strip A is licensed to kill strip C and vise-versa. Therefore, we only have to keep track of which of the original strips a given strip is licensed to kill. 

X2 is built on X1, but also considers the possibility of multiple hits on one strip. It calculates number of unordered pairs one can make from the elements of X1. The dimension of the X2 family of vectors is that of X1 plus all the combinations. The combinations are labeled by multiplying the number of the lower strips being combined by 100 and adding it to the strip number of the second strip in the combination. Then, strip 112 is strip 1 combined with strip 12. Strip 1751 is the combination of strip 17 with the charge sharing between strip 1 and 2.  

The license to kill from the new strips is the vector sum of its constituent strips. Doing this has also the purpose of identifying “bad combinations”, that take a strip into account more than once.  An example for a bad combination is strip 251, which is made up of strip 2 and the charge sharing between 1 and 2. In this example we are counting the the charge in strip 2 twice, which is wrong. The program spots these errors, by noting that strip 2 is licensed to kill itself but so is strip 51. By doing a vector addition of the killing licenses, bad combinations are identified by allowing to kill one of the original strips more than once.  

Having identified the bad combinations from X2 and Y2, we are finally in a position to construct the weight matrix. I ignore one last type of interaction, which I call the “double/double” interaction. These are matrix elements from unphysical interactions where four strips are fired, and arise from combining two strips with numbers >100. For example comparing strip 105 with 715.    
The program now finally constructs the matrix containing the weights and is able to run through the steps a) to c) above. 

*********

OUTPUT: The output is a matrix containing the paired strip numbers. pair[0,0] is the number of the first x strip paired with the first y strip (pair[0,1]). Pair[1,*] contains the strip numbers of the second pairing, and so forth. 

Note: The program does not have a check to make sure that all the strips were paired in the selection. This will be enforced in a newer version i have been writing that does a recursive search in the weight matrix above.


Daniel Perez-Becker, 2008
 */

#ifndef __MNCTModuleStripPairingGreedy_a__
#define __MNCTModuleStripPairingGreedy_a__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MGUIExpoStripPairing.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleStripPairingGreedy_a : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleStripPairingGreedy_a();
  //! Default destructor
  virtual ~MNCTModuleStripPairingGreedy_a();
  
  //! Create a new object of this class 
  virtual MNCTModuleStripPairingGreedy_a* Clone() { return new MNCTModuleStripPairingGreedy_a(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! The display of debugging data
  MGUIExpoStripPairing* m_ExpoStripPairing;

  //! Event Counters
  int m_TotalMatches;
  
  //! Variable Match counter, used to events with a specific numbers of
  //! strips involved 
  int m_NMatches;
  
  //! Counts the number of badly matched events
  int m_NBadMatches;

  // private members:
 private:




#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleStripPairingGreedy_a, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
