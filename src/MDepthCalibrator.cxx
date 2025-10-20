#include "MDepthCalibrator.h"

#include "MModule.h"

//TFile* RootF;

MDepthCalibrator::MDepthCalibrator()
{
	//need unordered_map m_Coeffs
	m_CoeffsFileIsLoaded = false;
	m_SplinesFileIsLoaded = false;
	//m_ThicknessFileIsLoaded = false;
	m_Thicknesses.reserve(12);

	/*
	if(Year == 2014){

		m_Thicknesses[0] = 1.49;
		m_Thicknesses[1] = 1.45;
		m_Thicknesses[2] = 1.50;
		m_Thicknesses[3] = 1.51;
		m_Thicknesses[4] = 1.50;
		m_Thicknesses[5] = 1.47;
		m_Thicknesses[6] = 1.48;
		m_Thicknesses[7] = 1.47;
		m_Thicknesses[8] = 1.49;
		m_Thicknesses[9] = 1.47;
		m_Thicknesses[10] = 1.42;
		m_Thicknesses[11] = 1.45;

	} else if(Year == 2016){

		m_Thicknesses[0] = 1.49;
		m_Thicknesses[1] = 1.45;
		m_Thicknesses[2] = 1.50;
		m_Thicknesses[3] = 1.45;
		m_Thicknesses[4] = 1.51;
		m_Thicknesses[5] = 1.50;
		m_Thicknesses[6] = 1.48;
		m_Thicknesses[7] = 1.47;
		m_Thicknesses[8] = 1.49;
		m_Thicknesses[9] = 1.47;
		m_Thicknesses[10] = 1.42;
		m_Thicknesses[11] = 1.45;

	}
	*/




	//RootF = new TFile("$NUCLEARIZER/resource/rootfiles/timing_splines.root","recreate");

}

/*
bool MDepthCalibrator::LoadThicknessFile(MString FName)
{
	MFile F;
	if(!F.Open(FName)){
		cout << "MDepthCalibrator: failed to open detector thickness file..." << endl;
		return false;
	} else {
		MString line;
		while(F.ReadLine(line)){
			vector<MString> Tokens = line.Tokenize(" ");
			if(Tokens.size() == 2){
				int DetID = Tokens[0].ToInt();
				if(DetID < 12){
					m_Thicknesses[DetID] = Tokens[1].ToDouble();
				} else {
					cout << "MDepthCalibrator: bad detector ID when parsing detector thickness file... got: " << DetID << endl;
					F.Close();
					return false;
				}
			}
		}
		F.Close();
		return true;
	}
}
*/

bool MDepthCalibrator::LoadCoeffsFile(MString FName)
{
	MFile F;
	if( F.Open(FName) == false ){
		cout << "MDepthCalibrator: failed to open coefficicents file..." << endl;
		return false;
	} else {
		MString Line;
		while( F.ReadLine( Line ) ){
			if ( Line.BeginsWith('#') ){
				std::vector<MString> Tokens = Line.Tokenize(" ");
				m_Coeffs_Energy = Tokens[5].ToDouble();
				cout << "The stretch and offset were calculated for " << m_Coeffs_Energy << " keV." << endl;
			}
			else {
				std::vector<MString> Tokens = Line.Tokenize(",");
				if( Tokens.size() == 5 ){
					int pixel_code = Tokens[0].ToInt();
					double Stretch = Tokens[1].ToDouble();
					double Offset = Tokens[2].ToDouble();
					double CTD_FWHM = Tokens[3].ToDouble() * 2.355;
					double Chi2 = Tokens[4].ToDouble();
					// Previous iteration of depth calibration read in "Scale" instead of ctd resolution.
					vector<double> coeffs;
					coeffs.push_back(Stretch); coeffs.push_back(Offset); coeffs.push_back(CTD_FWHM); coeffs.push_back(Chi2);
					m_Coeffs[pixel_code] = coeffs;
				}
			}
		}
		F.Close();
	}

	m_CoeffsFileIsLoaded = true;

	return true;

}

std::vector<double>* MDepthCalibrator::GetPixelCoeffs(int pixel_code)
{
	if( m_CoeffsFileIsLoaded ){
		if( m_Coeffs.count(pixel_code) > 0 ){
			return &m_Coeffs[pixel_code];
		} else {
			return NULL;
		}
	} else {
		cout << "MDepthCalibrator::GetPixelCoeffs: cannot get coeffs, coeff file has not yet been loaded" << endl;
		return NULL;
	}

}

bool MDepthCalibrator::LoadTACCalFile(MString FName)
{
  // Read in the TAC Calibration file, which should contain for each strip:
  // DetID, h or l for high or low voltage, TAC cal, TAC cal error, TAC cal offset, TAC offset error
  MFile F;
  if (!F.Open(FName)) {
    cout << "MDepthCalibrator: failed to open TAC Calibration file." << endl;
    m_TACCalFileIsLoaded = false;
    return false;
  } 

  MString Line;
  while (F.ReadLine(Line)) {
    if (!Line.BeginsWith("#")) {
      vector<MString> Tokens = Line.Tokenize(",");
      if (Tokens.size() == 7) {
        int DetID = Tokens[0].ToInt();
        int StripID = Tokens[2].ToInt();
        double taccal = Tokens[3].ToDouble();
        double taccal_err = Tokens[4].ToDouble();
        double offset = Tokens[5].ToDouble();
        double offset_err = Tokens[6].ToDouble();

        // Create a vector with calibration values
        vector<double> cal_vals = {taccal, offset, taccal_err, offset_err};

        // Ensure the unordered_map exists for this DetID (automatically creates if missing)
        if (Tokens[1] == "l") {
          m_LVTACCal[DetID][StripID] = cal_vals;  
        } else if (Tokens[1] == "h") {
          m_HVTACCal[DetID][StripID] = cal_vals;
        }
      }
    }
  }

  F.Close();
  m_TACCalFileIsLoaded = true;
  return true;
}

std::vector<double>* MDepthCalibrator::GetLVTACCal(int DetID, int StripID)
{
	if( m_TACCalFileIsLoaded ){
		if( m_LVTACCal.count(DetID) && m_LVTACCal[DetID].count(StripID) > 0 ){
			return &m_LVTACCal[DetID][StripID];
		} else {
			return NULL;
		}
	} else {
		cout << "MDepthCalibrator::GetLVTACCal: cannot get coeffs, coeff file has not yet been loaded" << endl;
		return NULL;
	}

}

std::vector<double>* MDepthCalibrator::GetHVTACCal(int DetID, int StripID)
{
	if( m_TACCalFileIsLoaded ){
		if( m_HVTACCal.count(DetID) && m_HVTACCal[DetID].count(StripID) > 0 ){
			return &m_HVTACCal[DetID][StripID];
		} else {
			return NULL;
		}
	} else {
		cout << "MDepthCalibrator::GetHVTACCal: cannot get coeffs, coeff file has not yet been loaded" << endl;
		return NULL;
	}

}

bool MDepthCalibrator::LoadSplinesFile(MString FName)
{

  //when invert flag is set to true, the splines returned are CTD->Depth
	MFile F; 
	if( F.Open(FName) == false ){
		return false;
	}
	vector<double> depthvec, ctdvec, anovec, catvec;
	MString line;
	int DetID, NewDetID;
	while( F.ReadLine(line) ){
		if( line.Length() != 0 ){
			if( line.BeginsWith("#") ){
				vector<MString> tokens = line.Tokenize(" ");
				NewDetID = tokens[1].ToInt();
				if( depthvec.size() > 0 ) {
					AddSpline(depthvec, ctdvec, DetID, m_SplineMap_Depth2CTD, false);
					AddSpline(depthvec, ctdvec, DetID, m_SplineMap_CTD2Depth, true);
					AddSpline(depthvec, anovec, DetID, m_SplineMap_Depth2AnoTiming, false);
					AddSpline(depthvec, catvec, DetID, m_SplineMap_Depth2CatTiming, false);
				}

				depthvec.clear(); ctdvec.clear(); anovec.clear(); catvec.clear();
				DetID = NewDetID;
			} else {
				vector<MString> tokens = line.Tokenize(",");
				depthvec.push_back(tokens[0].ToDouble()); ctdvec.push_back(tokens[1].ToDouble());
				anovec.push_back(tokens[2].ToDouble()); catvec.push_back(tokens[3].ToDouble());
			}
		}
	}
	//make last spline
	if( depthvec.size() > 0 ){
		AddSpline(depthvec, ctdvec, DetID, m_SplineMap_Depth2CTD, false);
		AddSpline(depthvec, ctdvec, DetID, m_SplineMap_CTD2Depth, true);
		AddSpline(depthvec, anovec, DetID, m_SplineMap_Depth2AnoTiming, false);
		AddSpline(depthvec, catvec, DetID, m_SplineMap_Depth2CatTiming, false);
	}

	m_SplinesFileIsLoaded = true;
	return true;


}

// bool MDepthCalibrator::AddDepthCTD(vector<double> depthvec, vector<vector<double>> ctdarr, int DetID, unordered_map<int, vector<double>>& DepthGrid, unordered_map<int,vector<vector<double>>>& CTDMap){

//   // Saves a CTD array, basically allowing for multiple CTDs as a function of depth 
//   // depthvec: list of simulated depth values
//   // ctdarr: vector of vectors of simulated CTD values. Each vector of CTDs must be the same length as depthvec
//   // DetID: An integer which specifies which detector.
//   // CTDMap: unordered map into which the array of CTDs should be placed

//   // TODO: Possible energy dependence of CTD?
//   // TODO: Depth values need to be evenly spaced. Check this when reading the files in.

//   // Check to make sure things look right.
//   // First check that the CTDs all have the right length.
//   for( unsigned int i = 0; i < ctdarr.size(); ++i ){
//     if( (ctdarr[i].size() != depthvec.size()) && (ctdarr[i].size() > 0) ){
//       cout << "MDepthCalibrator::AddDepthCTD: The number of values in the CTD list is not equal to the number of depth values." << endl;
//       return false;
//     }
//   }

//   double maxdepth = * std::max_element(depthvec.begin(), depthvec.end());
//   double mindepth = * std::min_element(depthvec.begin(), depthvec.end());
//   m_Thicknesses[DetID] = maxdepth-mindepth;
//   cout << "MDepthCalibrator::AddDepthCTD: The thickness of detector " << DetID << " is " << m_Thicknesses[DetID] << endl;
  
//   //Now make sure the values for the depth start with 0.0.
//   if( mindepth != 0.0){
//       cout << "MDepthCalibrator::AddDepthCTD: The minimum depth is not zero. Editing the depth vector." << endl;
//       for( unsigned int i = 0; i < depthvec.size(); ++i ){
//         depthvec[i] -= mindepth;
//       }
//   }

//   CTDMap[DetID] = ctdarr;
//   DepthGrid[DetID] = depthvec;
//   return true;
// }

void MDepthCalibrator::AddSpline(vector<double> xvec, vector<double> yvec, int DetID, std::unordered_map<int,TSpline3*>& SplineMap, bool invert){
	//add one more point to the start and end, corresponding to the detector edges so that the spline covers the
	//entire detector. just use a linear interpolation to get the edge values.

	// the sparsify flag should be set to true if we X->CTD and Y->Depth... in this case, the X values near the detector edges are too close together and the 
	// the spline will come out wrong.  When sparsify is set, it will remove data points that neighbor other data points too closely

	//first extrapolate the lower side
	double dx, dy, m, b, newx, newy;
	dx = xvec[1] - xvec[0];
	dy = yvec[1] - yvec[0];
	m = dy / dx;
	b = yvec[0] - m*xvec[0];
	newx = xvec[0] - (dx/2.0);
	newy = m*newx + b;
	//	xvec.push_front(newx); yvec.push_front(newy);
	xvec.insert(xvec.begin(), newx); yvec.insert(yvec.begin(), newy);


	//next extrapolate the upper side
	size_t N = xvec.size();
	dx = xvec[N-1] - xvec[N-2];
	dy = yvec[N-1] - yvec[N-2];
	m = dy / dx;
	b = yvec[N-1] - m*xvec[N-1];
	newx = xvec[N-1] + (dx/2.0);
	newy = m*newx + b;
	xvec.push_back(newx); yvec.push_back(newy);

//	double* x = &xvec[0]; double* y = &yvec[0];
	double *x, *y;
	if( invert ){
		//need to filter the data here so that there aren't knots that are too close together
		
		bool Done = false;
		while( !Done ){
			Done = true;
			for( unsigned int i = 1; i < (xvec.size()-1); ++i ){
				if( (fabs(yvec[i] - yvec[i-1]) < 0.1) || (fabs(yvec[i] - yvec[i+1]) < 0.1) ){
					xvec.erase(xvec.begin() + i);
					yvec.erase(yvec.begin() + i);
					Done = false;
					break;
				}
			}
		}
		
		x = &yvec[0]; y = &xvec[0];
	} else {
		x = &xvec[0]; y = &yvec[0];
	}

	TSpline3* Sp = new TSpline3("",x,y,xvec.size());
	SplineMap[DetID] = Sp;
	double min = Sp->GetXmin();
	double max = Sp->GetXmax();
	int sN = 100;
	double sdx = fabs(max-min)/(sN-1);
	std::vector<double> sx, sy;
	for( int i = 0; i < sN; ++i ){
		double sx_ = (min + i*sdx); 
		sx.push_back(sx_);
		double sy_ = Sp->Eval(sx_);
		sy.push_back(sy_);
	}
	/*
	TGraph* Gsp = new TGraph(sx.size(), (double*) &sx[0], (double*) &sy[0]);
	Gsp->SetLineColor(2);
	Gsp->SetLineWidth(3);
	TGraph* Gd = new TGraph(xvec.size(), x, y);
	Gd->SetLineStyle(2);
	Gd->SetLineWidth(0);
	Gd->SetMarkerStyle(3);
	TMultiGraph* MG = new TMultiGraph();
	MG->Add(Gd);
	MG->Add(Gsp);
	RootF->WriteTObject(MG);
  */
	return;
}

TSpline3* MDepthCalibrator::GetSpline(int Det, bool Depth2CTD)
{
	/*
	if( Depth2CTD ){
		if( !m_SplinesFileIsLoaded_Depth2CTD ){
			cout << "MDepthCalibrator::GetSpline: cannot return spline, spline for Depth2CTD has not been loaded" << endl;
			return NULL;
		}
	} else {
		if( !m_SplinesFileIsLoaded_CTD2Depth ){
			cout << "MDepthCalibrator::GetSpline: cannot return spline, spline for CTD2Depth has not been loaded" << endl;
			return NULL;
		}
	}*/

	if( !m_SplinesFileIsLoaded ){
		cout << "MDepthCalibrator::GetSpline: cannot return spline because spline file was not loaded." << endl;
		return NULL;
	}

	std::unordered_map<int,TSpline3*>* MapPtr;
	if( Depth2CTD ) MapPtr = &m_SplineMap_Depth2CTD; else MapPtr = &m_SplineMap_CTD2Depth;
	if( MapPtr->count(Det) > 0 ){
		return (*MapPtr)[Det];
	} else {
		return NULL;
	}

}

TSpline3* MDepthCalibrator::GetAnodeSpline(int Det){
	if( !m_SplinesFileIsLoaded ){
		cout << "MDepthCalibrator::GetAnodeSpline: cannot return spline because spline file was not loaded." << endl;
		return NULL;
	}

	return m_SplineMap_Depth2AnoTiming[Det];
}

TSpline3* MDepthCalibrator::GetCathodeSpline(int Det){
	if( !m_SplinesFileIsLoaded ){
		cout << "MDepthCalibrator::GetCathodeSpline: cannot return spline because spline file was not loaded." << endl;
		return NULL;
	}

	return m_SplineMap_Depth2CatTiming[Det];
}


double MDepthCalibrator::GetThickness(int DetID){
	return m_Thicknesses[DetID];
}
	


