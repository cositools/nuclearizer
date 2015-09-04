#include "MNCTDepthCalibrator.h"

MNCTDepthCalibrator::MNCTDepthCalibrator()
{
	//need unordered_map m_Coeffs
	m_CoeffsFileIsLoaded = false;
	m_SplinesFileIsLoaded_Depth2CTD = false;
	m_SplinesFileIsLoaded_CTD2Depth = false;
	m_Thicknesses.reserve(12);
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


}

bool MNCTDepthCalibrator::LoadCoeffsFile(MString FName)
{
	MFile F;
	if( F.Open(FName) == false ){
		cout << "MNCTDepthCalibrator: failed to open coefficicents file..." << endl;
		return false;
	} else {
		MString Line;
		while( F.ReadLine( Line ) ){
			if( !Line.BeginsWith("#") ){
				std::vector<MString> Tokens = Line.Tokenize(" ");
				if( Tokens.size() == 5 ){
					int pixel_code = Tokens[0].ToInt();
					double Stretch = Tokens[1].ToDouble();
					double Offset = Tokens[2].ToDouble();
					double Scale = Tokens[3].ToDouble();
					double Chi2 = Tokens[4].ToDouble();
					//last two tokens are amplitude and chi2, not really needed here
					std::vector<double>* coeffs = new std::vector<double>();
					coeffs->push_back(Stretch); coeffs->push_back(Offset); coeffs->push_back(Scale); coeffs->push_back(Chi2);
					m_Coeffs[pixel_code] = coeffs;
				}
			}
		}
		F.Close();
	}

	m_CoeffsFileIsLoaded = true;

	return true;

}

std::vector<double>* MNCTDepthCalibrator::GetPixelCoeffs(int pixel_code)
{
	if( m_CoeffsFileIsLoaded ){
		if( m_Coeffs.count(pixel_code) > 0 ){
			return m_Coeffs[pixel_code];
		} else {
			return NULL;
		}
	} else {
		cout << "MNCTDepthCalibrator::GetPixelCoeffs: cannot get coeffs, coeff file has not yet been loaded" << endl;
		return NULL;
	}

}


bool MNCTDepthCalibrator::LoadSplinesFile(MString FName, bool invert)
{
	//when invert flag is set to true, the splines returned are CTD->Depth
	MFile F; 
	if( F.Open(FName) == false ){
		return false;
	}
	vector<double> xvec, yvec;
	MString line;
	int DetID, NewDetID;
	while( F.ReadLine(line) ){
		if( line.Length() != 0 ){
			if( line.BeginsWith("#") ){
				vector<MString> tokens = line.Tokenize(" ");
				NewDetID = tokens[1].ToInt();
				if( xvec.size() > 0 ) AddSpline(xvec, yvec, DetID, invert);
				DetID = NewDetID;
			} else {
				vector<MString> tokens = line.Tokenize(" ");
				xvec.push_back(tokens[0].ToDouble()); yvec.push_back(tokens[1].ToDouble());
			}
		}
	}
	//make last spline
	if( xvec.size() > 0 ) AddSpline(xvec, yvec, DetID, invert);

	if( invert ) m_SplinesFileIsLoaded_CTD2Depth = true; else m_SplinesFileIsLoaded_Depth2CTD = true;
	return true;

}

void MNCTDepthCalibrator::AddSpline(vector<double>& xvec, vector<double>& yvec, int DetID, bool invert){
	//add one more point to the start and end, corresponding to the detector edges so that the spline covers the
	//entire detector. just use a linear interpolation to get the edge values.

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

	//double* x = &xvec[0]; double* y = &yvec[0];
	if( invert ){
		//need to filter the data here so that there aren't knots that are too close together
		bool Done = false;
		while( !Done ){
			Done = true;
			for( unsigned int i = 1; i < (xvec.size()-1); ++i ){
				if( (fabs(yvec[i] - yvec[i-1]) < 1.5) || (fabs(yvec[i] - yvec[i+1]) < 1.5) ){
					xvec.erase(xvec.begin() + i);
					yvec.erase(yvec.begin() + i);
					Done = false;
					break;
				}
			}
		}

		m_SplineMap_CTD2Depth[DetID] = new TSpline3("",(double*) &yvec[0],(double*) &xvec[0],xvec.size());

	} else {
		m_SplineMap_Depth2CTD[DetID] = new TSpline3("",(double*) &xvec[0],(double*) &yvec[0],xvec.size());
	}
	xvec.clear(); yvec.clear();
	return;
}

TSpline3* MNCTDepthCalibrator::GetSpline(int Det, bool Depth2CTD)
{
	if( Depth2CTD ){
		if( !m_SplinesFileIsLoaded_Depth2CTD ){
			cout << "MNCTDepthCalibrator::GetSpline: cannot return spline, spline for Depth2CTD has not been loaded" << endl;
			return NULL;
		}
	} else {
		if( !m_SplinesFileIsLoaded_CTD2Depth ){
			cout << "MNCTDepthCalibrator::GetSpline: cannot return spline, spline for CTD2Depth has not been loaded" << endl;
			return NULL;
		}
	}

	std::unordered_map<int,TSpline3*>* MapPtr;
	if( Depth2CTD ) MapPtr = &m_SplineMap_Depth2CTD; else MapPtr = &m_SplineMap_CTD2Depth;
	if( MapPtr->count(Det) > 0 ){
		return (*MapPtr)[Det];
	} else {
		return NULL;
	}

}

double MNCTDepthCalibrator::GetThickness(int DetID){
	return m_Thicknesses[DetID];
}
	


