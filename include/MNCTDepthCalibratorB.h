#include <unordered_map>
#include "MString.h"

class MNCTDepthCalibratorB
{
	public:
		//! Default constructor
		MNCTDepthCalibratorB();
		//! Load calibration data
		bool LoadLookupTables(MString Fname);
		//! Check if lookup table exists for a pixel
		bool PixelTableExists(int Pixel);
		//! Get Z from Pixel ID and measured CTD value
		bool GetZ(int Pixel, int CTD, double& Z);

	private:
		bool m_LookupTablesLoaded;
		unordered_map<int,unordered_map<int,double>*> m_PixelTables;

};
