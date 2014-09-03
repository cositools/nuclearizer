/*
 * MNCTModuleStripPairingGreedy.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModuleStripPairingGreedy
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleStripPairingGreedy.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTMath.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleStripPairingGreedy)
#endif


////////////////////////////////////////////////////////////////////////////////


  MNCTModuleStripPairingGreedy::MNCTModuleStripPairingGreedy() : MModule()
{
  // Construct an instance of MNCTModuleStripPairingGreedy

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Strip pairing greedy";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "StripPairingGreedy";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_StripPairing);


  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options

  // Add member variables here
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleStripPairingGreedy::~MNCTModuleStripPairingGreedy()
{
  // Delete this instance of MNCTModuleStripPairingGreedy
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleStripPairingGreedy::Initialize()
{
  // Initialize the module 

  // Add all initializations which are global to all events
  // and have member variables here

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleStripPairingGreedy::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  // Attention: Make sure too loop over all detectors
  
  // Check number of detectors & store detector ID in vector

  // for all detectors

  // Original input:
  //int x[4] = {1,2,5,7}; 
  //int y[3] = {5,6,15};
  //float x_e[4] = {56.,126.,481.,952.}; //These are the input variables.
  //float y_e[3] = {180.,483.,900.};    //Their dimension varies and the
  //float x_sig[4] = {0.9,0.9,0.9,0.9}; //program should accept and recongnize
  //float y_sig[3] = {0.9,0.9,0.9}; //Vectors of variable length.

  int n_x = 0;
  int n_y = 0;

  // Find the number of hits per side for this detector
  for (unsigned int s = 0; s < Event->GetNStripHits(); ++s) {
    // if detector ....
    if (Event->GetStripHit(s)->IsXStrip() == true) {
      ++n_x;
    } else {
      ++n_y;
    }
  }

  // Initialize variables
  int x[n_x];
  int y[n_y];
  float x_e[n_x];
  float y_e[n_y];
  float x_sig[n_x];
  float y_sig[n_y];

  int ix = 0;
  int iy = 0;
  // Initialize data:
  for (unsigned int s = 0; s < Event->GetNStripHits(); ++s) {
    // if detector ....
    if (Event->GetStripHit(s)->IsXStrip() == true) {
      x[ix] = Event->GetStripHit(s)->GetStripID();
      x_e[ix] = Event->GetStripHit(s)->GetEnergy();
      x_sig[ix] = 0.9;
      ++ix;
    } else {
      y[iy] = Event->GetStripHit(s)->GetStripID();
      y_e[iy] = Event->GetStripHit(s)->GetEnergy();
      y_sig[iy] = 0.9;
      ++iy;
    }
  }

  // Continue with original code

  float obesity; 
  int i,j,k,m,p,q;
  int i_kill, j_kill, intob;
  int n_x1,n_y1;
  int extra_n_x=0;
  int extra_n_y=0;
  //n_x=sizeof(x)/sizeof(int);
  //n_y=sizeof(y)/sizeof(int);
  int flag_x[n_x];
  int flag_y[n_y];
  for(i=0;i<n_x;i++) flag_x[i]=500;
  for(i=0;i<n_y;i++) flag_y[i]=500;

  // 

  //Set Flags if two neighbouring strips are fired 
 
  for(i=0;i<n_x-1;i++){
    if(x[i+1]-x[i]==1){   
      extra_n_x++;  //counts the number of extra dimensions needed for x1
      flag_x[i]=i;
    }}
 
  for(i=0;i<n_y-1;i++){
    if(y[i+1]-y[i]==1){ 
      extra_n_y++;
      flag_y[i]=i;
    }}
  //X1 and Y1 INCLUDE THE POSIBILITY OF STRIP SHARING     
  /* Create arrays with the extra dimensions needed for shared strips
     and fill with the approrpiate vaules */ 
 
  //for x
  n_x1=n_x+extra_n_x;  //dimension of x1
  int x1[n_x1];
  float x1_e[n_x1];   //creation of x1 variables
  float x1_sig[n_x1];
  int kill_x1[n_x1][n_x];
  for(i=0;i<n_x1;i++){
    for(j=0;j<n_x;j++){
      kill_x1[i][j]=0;
    }}
 
  for(i=0;i<n_x;i++){
    x1[i]=x[i];
    x1_e[i]=x_e[i];       // I asign the old x values to the first x1 elements
    x1_sig[i]=x_sig[i];
    kill_x1[i][i]=1;
  }

  if(extra_n_x != 0){
    m=0;
    for(k=0;k<n_x-1;k++){     // Assign the strip sharing values to x1
      if(flag_x[k]!=500){
        x1[n_x+m]=x[k]+50;  // Shared strips have a number greater than 50
        x1_e[n_x+m]=x_e[k]+x_e[k+1];
        x1_sig[n_x+m]=sqrt(x_sig[k]*x_sig[k]+x_sig[k+1]*x_sig[k+1]);
        for(j=0;j<n_x;j++){
          kill_x1[n_x+m][j]=kill_x1[k][j]+kill_x1[k+1][j];
        } 
        m++;
      }}}
  
 
  //for y  (Same I did before, but for the y side of the detector)

  n_y1=n_y+extra_n_y;
  int y1[n_y1];
  float y1_e[n_y1];
  float y1_sig[n_y1];
  int kill_y1[n_y1][n_y];
  for(i=0;i<n_y1;i++){
    for(j=0;j<n_y;j++){
      kill_y1[i][j]=0;
    }}
  
  for(i=0;i<n_y;i++){
    y1[i]=y[i];
    y1_e[i]=y_e[i];
    y1_sig[i]=y_sig[i];
    kill_y1[i][i]=1;
  }

  if(extra_n_y != 0){
    m=0;
    for(k=0;k<n_y-1;k++){
      if(flag_y[k]!=500){
        y1[n_y+m]=y[k]+50;
        y1_e[n_y+m]=y_e[k]+y_e[k+1];
        y1_sig[n_y+m]=sqrt(y_sig[k]*y_sig[k]+y_sig[k+1]*y_sig[k+1]);
        for(j=0;j<n_y;j++){
          kill_y1[n_y+m][j]=kill_y1[k][j]+kill_y1[k+1][j];
        } 
        m++;
      }}}
 
  
       
  for(i=0;i<n_x1;i++){
    printf("x1[%d] is %d \n",i,x1[i]);
    printf("x1_e[%d] is %f \n",i,x1_e[i]);    // print some values, for debugging
    printf("x1_sig[%d] is %f \n",i,x1_sig[i]); 
    printf("Licended to kill: %d, %d, %d, %d \n",kill_x1[i][0],kill_x1[i][1],kill_x1[i][2],kill_x1[i][3]);
  }
  /*
    for(i=0;i<n_y1;i++){
    printf("y1[%d] is %d \n",i,y1[i]);
    printf("y1_e[%d] is %f \n",i,y1_e[i]); 
    printf("y1_sig[%d] is %f \n",i,y1_sig[i]);
    }
  */

  //X2 and Y2 include the posiblity of the same strip (or two strips sharing charnge) firing twice. 

  //for x
 
  int n_x2;
  int extra_n_x2; 
  extra_n_x2=MNCTMath::comb(n_x1,2); // the extra dimensions needed for X2 are the combinations
  n_x2=n_x1+extra_n_x2;   // of the previous strips. 
  printf("n_x2= %d \n",n_x2);

  int x2[n_x2];
  float x2_e[n_x2];
  float x2_sig[n_x2];      //create X2
  int kill_x2[n_x2][n_x];
 
  for(i=0;i<n_x1;i++){
    x2[i]=x1[i]; // fill out the first elements of X2 with the elements from X1
    x2_e[i]=x1_e[i];      
    x2_sig[i]=x1_sig[i];
    for(j=0;j<n_x;j++) kill_x2[i][j]=kill_x1[i][j];
  }

  if(n_x2 > n_x1){
    m=0;
    for(k=0;k<n_x1-1;k++){
      for(p=n_x1-1;p>k;p--){
        x2[n_x1+m]=x1[k]*100+x1[p]; // asign the new values of X2 
        x2_e[n_x1+m]=x1_e[k]+x1_e[p];
        x2_sig[n_x1+m]=sqrt(x1_sig[k]*x1_sig[k]+x1_sig[p]*x1_sig[p]);
        for(j=0;j<n_x;j++){
          kill_x2[n_x1+m][j]=kill_x2[k][j]+kill_x2[p][j];
        }
        m++;
      }}}
   
  //for y  (Same as above, but for Y-side of detector)
 
  int n_y2;
  int extra_n_y2; 
  extra_n_y2=MNCTMath::comb(n_y1,2);
  n_y2=n_y1+extra_n_y2;
  printf("n_y2= %d \n",n_y2);

  int y2[n_y2];
  float y2_e[n_y2];
  float y2_sig[n_y2];
  int kill_y2[n_y2][n_y];
 
  for(i=0;i<n_y1;i++){
    y2[i]=y1[i];
    y2_e[i]=y1_e[i];
    y2_sig[i]=y1_sig[i];
    for(j=0;j<n_y;j++) kill_y2[i][j]=kill_y1[i][j];
  }

  if(n_y2 > n_y1){
    m=0;
    for(k=0;k<n_y1-1;k++){
      for(p=n_y1-1;p>k;p--){
        y2[n_y1+m]=y1[k]*100+y1[p];
        y2_e[n_y1+m]=y1_e[k]+y1_e[p];
        y2_sig[n_y1+m]=sqrt(y1_sig[k]*y1_sig[k]+y1_sig[p]*y1_sig[p]);
        for(j=0;j<n_y;j++){
          kill_y2[n_y1+m][j]=kill_y2[k][j]+kill_y2[p][j];
        }
        m++;
      }}}
  // COMPUTE NUMBER OF STRIPS NOT BEING ACCOUNTED FOR. AND INVALID STRIP COMBINATIONS (ONE STRIP ACCOUNTED FOR MORE THAN ONCE)   
  int x2_n_kills[n_x2], y2_n_kills[n_y2];
  int x2_bad[n_x2], y2_bad[n_y2];

  for(i=0;i<n_x2;i++) x2_n_kills[i]=0;
  for(i=0;i<n_y2;i++) y2_n_kills[i]=0; 
  for(i=0;i<n_x2;i++) x2_bad[i]=0;
  for(i=0;i<n_y2;i++) y2_bad[i]=0; 

  for(i=0;i<n_x2;i++){
    for(p=0;p<n_x;p++){
      if(kill_x2[i][p] != 0) x2_n_kills[i]++;
      if(kill_x2[i][p] > 1) x2_bad[i]++;   // if a strip is licenced to kill another
    }}                            // stirp more than once, it is a bad combination

  for(i=0;i<n_y2;i++){
    for(p=0;p<n_y;p++){
      if(kill_y2[i][p] != 0) y2_n_kills[i]++;
      if(kill_y2[i][p] > 1) y2_bad[i]++;
    }}
  
  printf("n_x, n_x1, n_x2, n_y, n_y1, n_y2 are %d, %d, %d, %d, %d, %d \n",n_x, n_x1,n_x2,n_y,n_y1,n_y2);
  for(i=0;i<n_x2;i++){
    printf("x2[%d] is %d \n",i,x2[i]);
    printf("x2_e[%d] is %f \n",i,x2_e[i]); 
    printf("x2_sig[%d] is %f \n",i,x2_sig[i]);
    printf("Licended to kill: %d, %d, %d, %d, total strips: %d, bad combination: %d \n",kill_x2[i][0],kill_x2[i][1],kill_x2[i][2],kill_x2[i][3],x2_n_kills[i],x2_bad[i]);
  } //print stuff out, to see if bad combination algorithm works



  //*********************WEIGHT MATIRIX**************************
  
      // create and evaluate weight matrix   
      int xrem=n_x;
  int yrem=n_y;
  float weights[n_x2][n_y2];
  float min_weight=5000.;             
  float fmax_pairs, fn_x2=n_x2, fn_y2=n_y2;
  fmax_pairs=(0.5)*abs(fn_x2+fn_y2)-(0.5)*abs(fn_x2-fn_y2);
  int max_pairs=floor(fmax_pairs);   //this is to know the maximum number of pairs
  printf("max_pairs= %d \n",max_pairs); //is possible from the input paramenters
  int pair [max_pairs][2];
 
                                                                                                                                     
  printf("size of x= %d \n",n_x2);

  //Calculate the weights between all X and Y strips
  for(i=0;i<n_x2;i++){
    for(j=0;j<n_y2;j++){
      if(x2_bad[i]==0 && y2_bad[j]==0){ //Exclude bad maches (for example 151= strip 1 and shared 1,2)
        if(i<n_x1 || j<n_y1){   //Exclude double/double interactions (for example 1 and 5 with 7 and 9)
          weights[i][j]=(x2_e[i]-y2_e[j])*(x2_e[i]-y2_e[j])/(x2_sig[i]*x2_sig[i]+y2_sig[j]*y2_sig[j]);
          //  printf("%d %d %f \n",i,j, weights[i][j]); 
          // printf("x1[i] is %d \n",x1[i]); 
        }else{
          printf("Excluding double/double interaction %d %d \n",x2[i],y2[j]);
          weights[i][j]=10000000.;
        }
      }else{
        printf("Excluding bad pair %d and %d \n",x2[i],y2[j]);
        weights[i][j]=10000000.;
      }
    }
  }
 
  k=0;
  do{

    for(i=n_x2-1;i>=0;i--){          //go through all matrix elements and find 
      for(j=n_y2-1;j>=0;j--){         // the minimum
        // printf("x[i] is %d \n",x[i]); 
        if (weights[i][j] <= min_weight){
          min_weight=weights[i][j];
          pair[k][0]=x2[i];     //pair[k] contains the mached elements
          pair[k][1]=y2[j];
          i_kill=i;
          j_kill=j; 
        }
      }
    } 

    for(i=0;i<n_x2;i++){
      printf("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f \n" ,weights[i][0]/10000000.,weights[i][1]/10000000.,weights[i][2]/10000000.,weights[i][3]/10000000.,weights[i][4]/10000000.,weights[i][5]/10000000.,weights[i][6]/10000000.,weights[i][7]/10000000.,weights[i][8]/10000000.,weights[i][9]/10000000.); // print out the matrix to see how things progress
	  } 
    xrem=xrem-x2_n_kills[i_kill]; //these variables tell me if all strips have been
    yrem=yrem-y2_n_kills[j_kill]; //matched at the end of the process

    min_weight=10000000.-1.;

 
    for(q=0;q<n_x;q++){ 
      if(kill_x2[i_kill][q] != 0){   
        for(p=0;p<n_x2;p++){     // kill all the matrix elements that the X-strip was 
          if(kill_x2[p][q] !=0){ // entitled to.
            for(j=0;j<n_y2;j++) weights[p][j]=10000000.;
            printf("X: %d chosen, therefore killing %d \n",x2[i_kill],x2[p]); 
          }}}}
    for(q=0;q<n_y;q++){ 
      if(kill_y2[j_kill][q] != 0){   
        for(p=0;p<n_y2;p++){  //kill all the matrix elements that the X-strip was
          if(kill_y2[p][q] !=0){ //entitled to.
            for(i=0;i<n_x2;i++) weights[i][p]=10000000.;
            printf("Y: %d chosen, therefore killing %d \n",y2[j_kill],y2[p]);
          }}}}
   
    obesity=0.;   // the "obesity" is just the percentage of matrix elements killed.
    for(i=0;i<n_x2;i++){ //when everyone is dead, the matrix is 1.00 obese
      for(j=0;j<n_y2;j++){
        obesity=obesity+weights[i][j]/10000000.;
      }} 
 
    intob=floor(obesity);
    printf("obesity: %f \n",obesity/(n_x2*n_y2));
    k++; 
  }while(intob < n_x2*n_y2 && k<10); // repeat search process until matrix is completeley obese



  for(i=0;i<k;i++){
    //for(i=0;i<2;i++){
    //printf("pair[0][%1d] = %d\n", i, pair[0][i]);
    printf("pair[%1d]= %d with %d \n", i, pair[i][0], pair[i][1]);  
  }  // print final values. 
 
  printf("xrem= %d, yrem= %d \n", xrem, yrem);
 
  /* to include charge sharing, create matrix with x2 that has x1 and all its combinations of 2 strips. Three interactions per strip will not be taken into account, which according to zong kai are less than 0.1% of the events, which will be very hard to recover data from anywhay. (less than 2min of data in a 36h flight) Rename charge sharing strips. There are 37 strips. The idea is that charge sharing between 1 and 2 becomes 51 , 2 and 3 52, etc. */

  // Create Hits and add them to 
  for(i=0;i<k;i++){
    MNCTHit* Hit = new MNCTHit();
   
    // Add all strip number..
    //for ( all strips ) {
    // Loop over strips in event
    // If it is identical with this information add the object to the hit
    //Hit->AddStripHit(Event->GetStripHit(xxx));
    //}

    //Hit->SetEnergy(XX);
    //Hit->SetEnergyResolution(XX);
    //Hit->SetPosition(YYY);
    //Hit->SetPositionResolution(YYY);

    Event->AddHit(Hit);
  }

  // end for all detector loop

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleStripPairingGreedy::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MNCTModuleStripPairingGreedy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
