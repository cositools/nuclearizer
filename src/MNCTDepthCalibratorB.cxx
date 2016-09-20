#include "MNCTDepthCalibratorB.h"
#include "MFile.h"
#include "TGraph.h"

MNCTDepthCalibratorB::MNCTDepthCalibratorB()
{
	m_LookupTablesLoaded = false;
}

/*
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
*/

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
		vector<double> t;
		vector<double> z;
		while( F.ReadLine( Line ) ){
			if(Line.BeginsWith("PX")){
				if(t.size() > 1){
					//make the interpolator, store i 
					TGraph* g = new TGraph(t.size(), &t[0], &z[0]);
					m_PixelTables[CurrentPixel] = g;
					t.clear();
					z.clear();
				}
				vector<MString> Tokens = Line.Tokenize(" ");
				if(Tokens.size() == 2){
					CurrentPixel = Tokens[1].ToInt();
				}
			} else {
				vector<MString> Tokens = Line.Tokenize(" ");
				if(Tokens.size() == 2){
					t.push_back(Tokens[0].ToDouble());
					z.push_back(Tokens[1].ToDouble());
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

/*
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
*/

bool MNCTDepthCalibratorB::GetZ(int Pixel, double t, double& z, double r){ 
	if(m_PixelTables.count(Pixel)){
		TGraph* g = m_PixelTables[Pixel];
		double t_ = t + m_Rand.Uniform(0.0,r);
		if((t_ >= g->GetX()[0]) && (t_ <= g->GetX()[g->GetN() - 1])){
			z = g->Eval(t_);
			return true;
		} else {
			return false;
		}   
	} else {
		return false;
	}   
}

