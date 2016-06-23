#include "MNCTDepthCalibratorB.h"
#include "MFile.h"

MNCTDepthCalibratorB::MNCTDepthCalibratorB()
{
	m_LookupTablesLoaded = false;
}

bool MNCTDepthCalibratorB::LoadLookupTables(MString FName)
{
	MFile F;
	if( F.Open(FName) == false ){
		cout << "MNCTDepthCalibratorB: failed to open lookup table file..." << endl;
		m_LookupTablesLoaded = false;
		return false;
	} else {
		MString Line;
		int CurrentPixel = -1;
		while( F.ReadLine( Line ) ){
			if(Line.BeginsWith("PX")){
				vector<MString> Tokens = Line.Tokenize(" ");
				if(Tokens.size() == 2){
					CurrentPixel = Tokens[1].ToInt();
					m_PixelTables[CurrentPixel] = new unordered_map<int,double>();
				}
			} else {
				vector<MString> Tokens = Line.Tokenize(" ");
				if(Tokens.size() == 2){
					int CTD = Tokens[0].ToInt();
					double Z = Tokens[1].ToDouble();
					(*m_PixelTables[CurrentPixel])[CTD] = Z;
				}
			}
		}
	}
	m_LookupTablesLoaded = true;
	F.Close();
	return true;
}

bool MNCTDepthCalibratorB::PixelTableExists(int Pixel){
	if(m_PixelTables.count(Pixel)){
		return true;
	} else {
		return false;
	}
}

bool MNCTDepthCalibratorB::GetZ(int Pixel, int CTD, double& Z){
	if(m_PixelTables.count(Pixel)){
		if(m_PixelTables[Pixel]->count(CTD)){
			Z = (*m_PixelTables[Pixel])[CTD];
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}
