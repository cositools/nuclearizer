#include "TSpline.h"
#include <unordered_map>
#include <vector>
#include "MString.h"
#include "MFile.h"

class MNCTDepthCalibrator
{
	public:
		//! Default constructor
		MNCTDepthCalibrator();
		//! Load the coefficients file (i.e. fit parameters for each pixel)
		bool LoadCoeffsFile(MString FName);
		//! Return the coefficients for a pixel
		std::vector<double>* GetPixelCoeffs(int pixel_code);
		//! Load the splines file
		bool LoadSplinesFile(MString FName, bool invert);
		//! Return a pointer to a spline
		TSpline3* GetSpline(int Det, bool Depth2CTD);
		//! Return detector thickness
		double GetThickness(int DetID);


	private:
		//! Generate the spline from the data and add it to the internal spline map
		void AddSpline(vector<double>& xvec, vector<double>& yvec, int DetID, bool invert);


	private:
		std::unordered_map<int,std::vector<double>*> m_Coeffs;
		std::unordered_map<int,TSpline3*> m_SplineMap_Depth2CTD;
		std::unordered_map<int,TSpline3*> m_SplineMap_CTD2Depth;
		bool m_SplinesFileIsLoaded_CTD2Depth;
		bool m_SplinesFileIsLoaded_Depth2CTD;
		bool m_CoeffsFileIsLoaded;
		std::vector<double> m_Thicknesses;

};
