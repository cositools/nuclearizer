/*
 * MNCTModuleStripPairingGreedy_a.cxx
 *
 *
 * Copyright (C) 2008-2008 by Daniel Perez-Becker.
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
// MNCTModuleStripPairingGreedy_a
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleStripPairingGreedy_a.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTModule.h"
#include "MNCTMath.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleStripPairingGreedy_a)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleStripPairingGreedy_a::MNCTModuleStripPairingGreedy_a() : MNCTModule()
{
  // Construct an instance of MNCTModuleStripPairingGreedy_a
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Strip pairing - Daniel's \"Greedy\" version";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "StripPairingGreedy_a";
  
  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(c_EventLoader);
  AddPreceedingModuleType(c_EnergyCalibration);
  
  // Set all types this modules handles
  AddModuleType(c_StripPairing);
  
  // Set all modules, which can follow this module
  AddSucceedingModuleType(c_NoRestriction);
  
  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
  
  // Add member variables here
  
  m_NBadMatches =0;
  m_NMatches =0;
  m_TotalMatches=0;
  
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleStripPairingGreedy_a::~MNCTModuleStripPairingGreedy_a()
{
  // Delete this instance of MNCTModuleStripPairingGreedy_a
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleStripPairingGreedy_a::Initialize()
{
  // Initialize the module 
  
  // Add all initializations which are global to all events
  // and have member variables here
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleStripPairingGreedy_a::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level 
  
  // Check if the event has all the appropriate flogs
  if (Event->IsEnergyCalibrated() == false) {
    mout<<"Error: Energy not calibrated!"<<endl;
    return false;
  }
  
  
  
  
  float EventQualityFactor=0.; //Gives average quality of all hits in an event
  float EventQualityCounter=0.;
  
  
  int n_detectors=12; //Number of detectors (labeled 0 to n_detectors-1) Make the program read this variable automatically 
  // Attention: Make sure too loop over all detectors
  
  int detector=0; 
  
  for (detector=0; detector < n_detectors; detector++){
    
    // Check number of detectors & store detector ID in vector, WHERE IS THIS VARIABLE STORED? 
    
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
      if (detector == Event->GetStripHit(s)->GetDetectorID()){
        if (Event->GetStripHit(s)->IsXStrip() == true) {
          ++n_x;
        } else {
          ++n_y;
        }
        
      }
    }
    
    //mout << "EventID: " << Event->GetID() << endl;
    
    float DetectorQualityFactor=0.;
    float DetectorQualityCounter=0.;
    
    //#printf("n_x= %d, n_y= %d \n", n_x, n_y); 
    
    //IMPORTANT: THE IF STATEMENT BELOW SETS CAPS ON THE KIND OF EVENTS ANALIZED BY THE CODE!!!!!!!!
    
    if( (n_x > 0) && (n_y > 0) && (fabs(n_x - n_y) < 5) && (n_x < 8) && (n_y < 8)) {
      
      // Initialize variables
      int x0[n_x], x[n_x];
      int y0[n_y], y[n_y];
      float x0_e[n_x], x_e[n_x];
      float y0_e[n_y], y_e[n_y];
      float x0_sig[n_x], x_sig[n_x];
      float y0_sig[n_y], y_sig[n_y];
      
      int ix = 0;
      int iy = 0;
      int i,j,k,m,p,q;
      
      // Initialize data:
      for (unsigned int s = 0; s < Event->GetNStripHits(); ++s) {
        if (detector == Event->GetStripHit(s)->GetDetectorID()){
          if (Event->GetStripHit(s)->IsXStrip() == true) {
            x0[ix] = Event->GetStripHit(s)->GetStripID();
            x0_e[ix] = Event->GetStripHit(s)->GetEnergy();
            x0_sig[ix] = Event->GetStripHit(s)->GetEnergyResolution();
            ++ix;
          } else {
            y0[iy] = Event->GetStripHit(s)->GetStripID();
            y0_e[iy] = Event->GetStripHit(s)->GetEnergy();
            y0_sig[iy] = Event->GetStripHit(s)->GetEnergyResolution();
            ++iy;
          }
        }
      }
      
      // transform x0 into x. x contains all the hit strips in order
      q=0;
      for (i=1; i<38;i++){
        for (p=0; p<n_x; p++){
          if (x0[p]==i){
            x[q]=x0[p];
            x_e[q]=x0_e[p];
            x_sig[q]=x0_sig[p];
            q++;
          }
        }
        
      }
      
      q=0;
      for (i=1; i<38;i++){
        for (p=0; p<n_y; p++){
          if (y0[p]==i){
            y[q]=y0[p];
            y_e[q]=y0_e[p];
            y_sig[q]=y0_sig[p];
            q++;
          }
        }
        
      }
      
      
      // Continue with original code
      
      float obesity; 
      int i_kill=0, j_kill=0, intob=0;
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
        }
        
      }
      
      for(i=0;i<n_y-1;i++){
        if(y[i+1]-y[i]==1){ 
          extra_n_y++;
          flag_y[i]=i;
        }
        
      }
      //X1 and Y1 INCLUDE THE POSIBILITY OF STRIP SHARING     
      /* Create arrays with the extra dimensions needed for shared strips
       *     and fill with the approrpiate vaules */ 
      
      //for x
      n_x1=n_x+extra_n_x;  //dimension of x1
      int x1[n_x1];
      float x1_e[n_x1];   //creation of x1 variables
      float x1_sig[n_x1];
      int kill_x1[n_x1][n_x];
      for(i=0;i<n_x1;i++){
        for(j=0;j<n_x;j++){
          kill_x1[i][j]=0;
        }
        
      }
      
      for(i=0;i<n_x;i++){
        x1[i]=x[i];
        x1_e[i]=x_e[i];       // I asign the old x values to the first x1 elements
        x1_sig[i]=x_sig[i];
        kill_x1[i][i]=1;
      }
      
      if(extra_n_x != 0) {
        m=0;
        for(k=0;k<n_x-1;k++) {     // Assign the strip sharing values to x1
      if(flag_x[k]!=500) {
        x1[n_x+m]=x[k]+50;  // Shared strips have a number greater than 50
        x1_e[n_x+m]=x_e[k]+x_e[k+1];
        x1_sig[n_x+m]=sqrt(x_sig[k]*x_sig[k]+x_sig[k+1]*x_sig[k+1]);
        for(j=0;j<n_x;j++){
          kill_x1[n_x+m][j]=kill_x1[k][j]+kill_x1[k+1][j];
        } 
        m++;
      }
        } 
      }
      
      
      //for y  (Same I did before, but for the y side of the detector)
      
      n_y1=n_y+extra_n_y;
      int y1[n_y1];
      float y1_e[n_y1];
      float y1_sig[n_y1];
      int kill_y1[n_y1][n_y];
      for(i=0;i<n_y1;i++) {
        for(j=0;j<n_y;j++) {
          kill_y1[i][j]=0;
        }
      }
      
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
          }
          
        }
        
      }
      
      
      
      //  for(i=0;i<n_x1;i++){
        //    printf("x1[%d] is %d \n",i,x1[i]);
        //    printf("x1_e[%d] is %f \n",i,x1_e[i]);    // print some values, for debugging
        //   printf("x1_sig[%d] is %f \n",i,x1_sig[i]); 
        //   printf("Licended to kill: %d, %d, %d, %d \n",kill_x1[i][0],kill_x1[i][1],kill_x1[i][2],kill_x1[i][3]);
        //  }
        /*
         *    for(i=0;i<n_y1;i++){
         *    printf("y1[%d] is %d \n",i,y1[i]);
         *    printf("y1_e[%d] is %f \n",i,y1_e[i]); 
         *    printf("y1_sig[%d] is %f \n",i,y1_sig[i]);
    }
    */
        
        //X2 and Y2 include the posiblity of the same strip (or two strips sharing charnge) firing twice. 
        
        //for x
        
        int n_x2;
        int extra_n_x2; 
        extra_n_x2=MNCTMath::comb(n_x1,2); // the extra dimensions needed for X2 are the combinations
        n_x2=n_x1+extra_n_x2;   // of the previous strips. 
        //  printf("n_x2= %d \n",n_x2);
        
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
            }
            
          }
          
        }
        
        //for y  (Same as above, but for Y-side of detector)
        
        int n_y2;
        int extra_n_y2; 
        extra_n_y2=MNCTMath::comb(n_y1,2);
        n_y2=n_y1+extra_n_y2;
        //  printf("n_y2= %d \n",n_y2);
        
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
            }
            
          }
          
        }
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
          }
          
        }                            // stirp more than once, it is a bad combination
        
        for(i=0;i<n_y2;i++){
          for(p=0;p<n_y;p++){
            if(kill_y2[i][p] != 0) y2_n_kills[i]++;
            if(kill_y2[i][p] > 1) y2_bad[i]++;
          }
          
        }
        
        
        // START PRINTING TO FIND ERROR
        
        /*
         *  for(j=0;j<n_x;j++){
         *    if(x2[j]==0){
         *      printf("x2[%d]= %d, x[%d]= %d \n",j,x2[j],j,x[j]);
    }}
    
    */
        /*
         *  for(j=0;j<n_x;j++){
         *  if( (x[j]> 40000) || (x[j] < 1)){
         *  printf("n_x, n_x1, n_x2, n_y, n_y1, n_y2 are %d, %d, %d, %d, %d, %d \n",n_x, n_x1,n_x2,n_y,n_y1,n_y2);
         *  for(i=0;i<n_x2;i++){
         *    printf("x[%d] is %d \n",i,x[i]);
         *    printf("x_e[%d] is %f \n",i,x_e[i]); 
         *    printf("x_sig[%d] is %f \n",i,x_sig[i]);
         *    printf("Licended to kill: %d, %d, %d, %d, total strips: %d, bad combination: %d \n",kill_x2[i][0],kill_x2[i][1],kill_x2[i][2],kill_x2[i][3],x2_n_kills[i],x2_bad[i]);
    } //print stuff out, to see if bad combination algorithm works
    
    for(i=0;i<n_y2;i++){
      printf("y2[%d] is %d \n",i,y2[i]);
      printf("y2_e[%d] is %f \n",i,y2_e[i]); 
      printf("y2_sig[%d] is %f \n",i,y2_sig[i]);
      printf("Licended to kill: %d, %d, %d, %d, total strips: %d, bad combination: %d \n",kill_y2[i][0],kill_y2[i][1],kill_y2[i][2],kill_y2[i][3],y2_n_kills[i],y2_bad[i]);
    } //print stuff out, to see if bad combination algorithm works
    printf("______DETECTOR: %d_______________ \n", detector);		  
    }} */
        
        //END PRINTING TO FIND ERROR
        
        
        
        //*********************WEIGHT MATIRIX**************************
        
        // create and evaluate weight matrix   
        int xrem=n_x;
        int yrem=n_y;
        float weights[n_x2][n_y2];
        float min_weight=5000.;             
        int max_pairs=0;
        max_pairs=(int)floor((0.5)*fabs((float)n_x2+(float)n_y2)-(0.5)*fabs((float)n_x2-(float)n_y2));
        //  printf("max_pairs= %d \n",max_pairs); //is possible from the input paramenters
        int pair [max_pairs][2];
        float energy [max_pairs];
        float e_resolution [max_pairs];
        
        float HitQualityFactor [max_pairs];
        for(i=0;i<max_pairs;i++){
          HitQualityFactor[i]=0.;
          energy[i]=0.;
          e_resolution[i]=0.;
        }
        
        //  printf("size of x= %d \n",n_x2);
        
        //Calculate the weights between all X and Y strips
        for (i=0;i<n_x2;i++) {
          for (j=0;j<n_y2;j++) {
            if (x2_bad[i]==0 && y2_bad[j]==0) { //Exclude bad maches (for example 151= strip 1 and shared 1,2)
        if (i<n_x1 || j<n_y1) {   //Exclude double/double interactions (for example 1 and 5 with 7 and 9)
        weights[i][j]=(x2_e[i]-y2_e[j])*(x2_e[i]-y2_e[j])/(x2_sig[i]*x2_sig[i]+y2_sig[j]*y2_sig[j]);
        //  printf("%d %d %f \n",i,j, weights[i][j]); 
        // printf("x1[i] is %d \n",x1[i]); 
        } else {
          //        printf("Excluding double/double interaction %d %d \n",x2[i],y2[j]);
          weights[i][j]=10000000.;
        }
            } else {
              //      printf("Excluding bad pair %d and %d \n",x2[i],y2[j]);
              weights[i][j]=10000000.;
            }
          }
        }
        
        k=0;
        do {
          
          for (i=n_x2-1;i>=0;i--) {          //go through all matrix elements and find 
      for (j=n_y2-1;j>=0;j--) {         // the minimum
        // printf("x[i] is %d \n",x[i]); 
        if (weights[i][j] <= min_weight){
          min_weight=weights[i][j];
          pair[k][0]=x2[i];     //pair[k] contains the mached elements
          pair[k][1]=y2[j];
          //energy[k]=(x2_e[i]+y2_e[j])/2.; //set energy for matched pair (just average for now)
          //e_resolution[k]=(x2_sig[i]+y2_sig[j])/2.; //just average for now
          energy[k]=y2_e[j]; //set energy for matched pair (use negative side)
          e_resolution[k]=y2_sig[j];
          i_kill=i;
          j_kill=j; 
          
          HitQualityFactor[k]=weights[i][j];
          
        }
      }
          } 
          DetectorQualityFactor=DetectorQualityFactor+HitQualityFactor[k];
          DetectorQualityCounter=DetectorQualityCounter+1.;
          
          
          // for(i=0;i<n_x2;i++){
            //   printf("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f \n" ,weights[i][0]/10000000.,weights[i][1]/10000000.,weights[i][2]/10000000.,weights[i][3]/10000000.,weights[i][4]/10000000.,weights[i][5]/10000000.,weights[i][6]/10000000.,weights[i][7]/10000000.,weights[i][8]/10000000.,weights[i][9]/10000000.); // print out the matrix to see how things progress
          //	  } 
          xrem=xrem-x2_n_kills[i_kill]; //these variables tell me if all strips have been
          yrem=yrem-y2_n_kills[j_kill]; //matched at the end of the process
          
          //  printf("yrem= %d, y2_n_kills= %d,j_kill= %d \n",yrem, y2_n_kills[j_kill], j_kill);  
          
          min_weight=10000000.-1.;
          
          
          for(q=0;q<n_x;q++){ 
            if(kill_x2[i_kill][q] != 0){   
              for(p=0;p<n_x2;p++){     // kill all the matrix elements that the X-strip was 
          if(kill_x2[p][q] !=0){ // entitled to.
            for(j=0;j<n_y2;j++) weights[p][j]=10000000.;
            //        printf("X: %d chosen, therefore killing %d \n",x2[i_kill],x2[p]); 
          }    
              }   
            }
          }
          for(q=0;q<n_y;q++){ 
            if(kill_y2[j_kill][q] != 0){   
              for(p=0;p<n_y2;p++){  //kill all the matrix elements that the X-strip was
          if(kill_y2[p][q] !=0){ //entitled to.
            for(i=0;i<n_x2;i++) weights[i][p]=10000000.;
            //       printf("Y: %d chosen, therefore killing %d \n",y2[j_kill],y2[p]);
          }
          
              }
              
            }
            
          }
          
          obesity=0.;   // the "obesity" is just the percentage of matrix elements killed.
          for(i=0;i<n_x2;i++){ //when everyone is dead, the matrix is 1.00 obese
      for(j=0;j<n_y2;j++){
        obesity=obesity+weights[i][j]/10000000.;
      }
      
          } 
          
          intob=(int)floor(obesity);
          //  printf("obesity: %f \n",obesity/(n_x2*n_y2));
          k++; 
        } while(intob < n_x2*n_y2 && k<10); // repeat search process until matrix is completeley obese
        
        DetectorQualityFactor=DetectorQualityFactor/DetectorQualityCounter;
        
        if((detector==8)&&(n_x > 0 || n_y > 0)){
          
          //  if ((n_x==0 && n_y==1)||(n_x==1 && n_y==0)){
            
            m_TotalMatches++;
            
            //  printf("n_x, n_x1, n_x2, n_y, n_y1, n_y2 are %d, %d, %d, %d, %d, %d \n",n_x, n_x1,n_x2,n_y,n_y1,n_y2);
            //#printf("n_x, n_y, are %d, %d \n",n_x,n_y);
            for(i=0;i<n_x2;i++){
              //#printf("x2[%d] is %d \n",i,x2[i]);
              //#printf("x2_e[%d] is %f \n",i,x2_e[i]); 
              // printf("x2_sig[%d] is %f \n",i,x2_sig[i]);
              // printf("Licended to kill: %d, %d, %d, %d, total strips: %d, bad combination: %d\n",kill_x2[i][0],kill_x2[i][1],kill_x2[i][2],kill_x2[i][3],x2_n_kills[i],x2_bad[i]);
            } //print stuff out, to see if bad combination algorithm works
            
            for(i=0;i<n_y2;i++){
              //# printf("y2[%d] is %d \n",i,y2[i]);
              //#printf("y2_e[%d] is %f \n",i,y2_e[i]); 
              //  printf("y2_sig[%d] is %f \n",i,y2_sig[i]);
              //  printf("Licended to kill: %d, %d, %d, %d, total strips: %d, bad combination: %d\n",kill_y2[i][0],kill_y2[i][1],kill_y2[i][2],kill_y2[i][3],y2_n_kills[i],y2_bad[i]);
            } //print stuff out, to see if bad combination algorithm works
            
            
            
            for(i=0;i<k;i++){
              //for(i=0;i<2;i++){
                //printf("pair[0][%1d] = %d\n", i, pair[0][i]);
                //# printf("pair[%1d]= %d with %d \n", i, pair[i][0], pair[i][1]);  
            }  // print final values. 
            
            //#printf("xrem= %d, yrem= %d \n", xrem, yrem);
            
            if ((n_x==5 && n_y==5)||(n_x==5 && n_y==5)){ 
              m_NMatches++;
              if (xrem+yrem > 0){
                m_NBadMatches++;
              }
              
            }
            //#  float temp_nmatches=m_NMatches;
            //#  float temp_badmatches=m_NBadMatches;
            //#  float temp_totalmatches=m_TotalMatches;
            //#  printf("Occurance: %f %, Missmatch: %f %, Of total Missmatch: %f % \n", temp_nmatches/temp_totalmatches*100., temp_badmatches/temp_nmatches*100., temp_badmatches/(0.057*temp_totalmatches)*100.);
        }
        //}
        
        
        /* to include charge sharing, create matrix with x2 that has x1 and all its combinations of 2 strips. Three interactions per strip will not be taken into account, which according to zong kai are less than 0.1% of the events, which will be very hard to recover data from anywhay. (less than 2min of data in a 36h flight) Rename charge sharing strips. There are 37 strips. The idea is that charge sharing between 1 and 2 becomes 51 , 2 and 3 52, etc. */
        
        int px_a=0;
        int px_b=0;
        int px1=0;
        int px2=0;
        int px3=0;
        int px4=0;
        
        int py_a=0;
        int py_b=0;
        int py1=0;
        int py2=0;
        int py3=0;
        int py4=0;
        
        // Create Hits and add them to 
        for(i=0;i<k;i++){
          
          px_a=0;
          px_b=0;
          
          px1=0;
          px2=0;
          px3=0;
          px4=0;
          
          
          
          // Decode matched strips from the combined number
          if (pair[i][0] < 38){ 
            
            px1=pair[i][0];
            
            
          }else if ((pair[i][0] > 38) && (pair[i][0] < 100)){
            
            px1=pair[i][0]-50;
            px2=px1+1;
            
            
          }else if (pair[i][0] > 100){
            
            px_a=pair[i][0]/100;
            px_b=pair[i][0]-px_a*100;
            
            if (px_a < 38){ 
              
              px1=px_a;
              
              
            }else if ((px_a > 38) && (px_a < 100)){
              
              px1=px_a-50;
              px2=px1+1;
            }
            
            if (px_b < 38){      
              
              px3=px_b;
              
            }else if ((px_b > 38) && (px_b < 100)){
              
              px3=px_b-50;
              px4=px3+1;
            }
            
          }
          
          py_a=0;
          py_b=0;
          
          py1=0;
          py2=0;
          py3=0;
          py4=0;
          
          if (pair[i][1] < 38){
            
            py1=pair[i][1];
            
            
          }else if ((pair[i][1] > 38) && (pair[i][1] < 100)){
            
            py1=pair[i][1]-50;
            py2=py1+1;
            
            
          }else if (pair[i][1] > 100){
            
            py_a=pair[i][1]/100;
            py_b=pair[i][1]-py_a*100;
            
            if (py_a < 38){
              
              py1=py_a;
              
              
            }else if ((py_a > 38) && (py_a < 100)){
              
              py1=py_a-50;
              py2=py1+1;
            }
            
            if (py_b < 38){
              
              py3=py_b;
              
            }else if ((py_b	> 38) && (py_b < 100)){
              
              py3=py_b-50;
              py4=py3+1;												       }
              
              
          }
          
          
          
          
          if((px1 > 0) && (px1 < 38)){
            //#printf("px1: %d, px2: %d, px3: %d, px4: %d \n", px1, px2, px3, px4);
            //#printf("py1: %d, py2: %d, py3: %d, py4: %d \n", py1, py2, py3, py4);
            //#printf("energy: %f \n", energy[i]);
            //#printf("HitQuality: %f \n", HitQualityFactor[i]);
            
            MNCTHit* Hit = new MNCTHit();
            
            // Add all strip number..
            for (unsigned int s=0; s < Event->GetNStripHits() ; ++s){
              if (detector == Event->GetStripHit(s)->GetDetectorID()){
                if (Event->GetStripHit(s)->IsXStrip() == true){
                  if (px1==Event->GetStripHit(s)->GetStripID()){
                    Hit->AddStripHit(Event->GetStripHit(s));
                    Hit->SetHitQuality(HitQualityFactor[i]);
                    Hit->SetEnergyResolution(e_resolution[i]);
                    
                  }else if (px2==Event->GetStripHit(s)->GetStripID()){
                    Hit->AddStripHit(Event->GetStripHit(s));
                  }else if (px3==Event->GetStripHit(s)->GetStripID()){
                    Hit->AddStripHit(Event->GetStripHit(s));
                  }else if (px4==Event->GetStripHit(s)->GetStripID()){
                    Hit->AddStripHit(Event->GetStripHit(s));
                  }
                }else{ 
                  if (py1==Event->GetStripHit(s)->GetStripID()){
                    Hit->AddStripHit(Event->GetStripHit(s));
                  }else if (py2==Event->GetStripHit(s)->GetStripID()){
                    Hit->AddStripHit(Event->GetStripHit(s));
                  }else if (py3==Event->GetStripHit(s)->GetStripID()){
                    Hit->AddStripHit(Event->GetStripHit(s));
                  }else if (py4==Event->GetStripHit(s)->GetStripID()){
                    Hit->AddStripHit(Event->GetStripHit(s));
                  }
                }
                
              }
              
            }
            // Loop over strips in event
            
            // If it is identical with this information add the object to the hit
            //Hit->AddStripHit(Event->GetStripHit(xxx));
            //}
            
            Hit->SetEnergy(energy[i]);
            //Hit->SetEnergyResolution(2.0);
            // Hit->SetPosition(YYY);
            // Hit->SetPositionResolution(YYY);
            
            Event->AddHit(Hit);
            //  }
            
          }/*else{ //end if px1>0 && px1< 38...
          
          printf("ERROR! px1 outside of allowed range. Tell Daniel about this! \n");
          printf("EventID= %d \n",Event->GetID());
          printf("px1= %d \n", px1);
          printf("n_x= %d, n_y= %d \n", n_x, n_y);
          printf("pair[%1d]= %d with %d \n", i, pair[i][0], pair[i][1]);  
          } */
          
          
          } //end for loop for strip match decoding and output (i=0;i<k;i++)
          
          // printf("DetectorQualityCounter= %f, EventQualityCounter= %f \n",DetectorQualityCounter, EventQualityCounter);
          
          if(DetectorQualityCounter > 0.5){
            EventQualityFactor=EventQualityFactor+DetectorQualityFactor;
            EventQualityCounter=EventQualityCounter+1.;
          }
          
        }  // end bi if fabs(nx-ny)<5 loop
        
        // printf("Detector= %d, DetectorQualityFactor= %f, EventQualityCounter= %f \n", detector, DetectorQualityFactor, EventQualityCounter);
        
    } // end detector loop 
    
    EventQualityFactor=EventQualityFactor/EventQualityCounter;
    
    // printf("EventQuality= %f \n", EventQualityFactor);
    Event->SetEventQuality(EventQualityFactor);
    
    
    // printf("__________________________________________\n");



    //Carolyn's edit to get rid of poorly matched strips...
    for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
      //mout<<"Daniel's Hit "<<h<<endl;
      //for (unsigned int s = 0; s < Event->GetHit(h)->GetNStripHits(); ++s) {
      //  mout<<"Energy: "<<Event->GetHit(h)->GetStripHit(s)->GetEnergy()<<endl;
      //}
      if (Event->GetHit(h)->GetNStripHits() == 2) {
      double deviation = 2.0*((Event->GetHit(h)->GetStripHit(0)->GetEnergy()) - (Event->GetHit(h)->GetStripHit(1)->GetEnergy()))/((Event->GetHit(h)->GetStripHit(0)->GetEnergy()) + (Event->GetHit(h)->GetStripHit(1)->GetEnergy()));
      //mout<<"deviation: "<<deviation<<endl;
      if (abs(deviation) > 0.5) {
        Event->SetStripPairingIncomplete(true);
      }
    }
  }


    
    return true;
  }
  
  
  ////////////////////////////////////////////////////////////////////////////////
  
  
  void MNCTModuleStripPairingGreedy_a::ShowOptionsGUI()
  {
    // Show the options GUI - or do nothing
  }
  
  
  // MNCTModuleStripPairingGreedy_a.cxx: the end...
  ////////////////////////////////////////////////////////////////////////////////
  
