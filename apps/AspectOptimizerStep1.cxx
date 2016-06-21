/*
	For now, use MNCTTimeAndCoordinate, but should also consider using python/pyephem as a cross check on the ephemeris
 */
#include "MGlobal.h"
#include "MFile.h"
#include "MString.h"
#include "MNCTTimeAndCoordinate.h"
#include <iostream>
#include <vector>
#include "MTime.h"
using namespace std;

double sine(double sine_input){
	double sine_output = sin((sine_input * 3.14159265359)/180);
	return sine_output;
}
double arcsine(double arcsine_input){
	double arcsine_output = ((asin(arcsine_input))*180)/3.14159265359;
	return arcsine_output;
}
double cosine(double cosine_input){
	double cosine_output = cos((cosine_input * 3.14159265359)/180);
	return cosine_output;
}
double arccosine(double arccosine_input){
	double arccosine_output = ((acos(arccosine_input))*180)/3.14159265359;
	return arccosine_output;
}
double tangent(double tangent_input){
	double tangent_output = tan((tangent_input * 3.14159265359)/180);
	return tangent_output;
}
double arctangent(double arctangent_input){
	double angle_in_degrees = ((atan(arctangent_input))*180)/3.14159265359;
	return angle_in_degrees;
}	
double arctangent2(double y, double x){
	double angle_in_degrees = ((atan2(y,x))*180)/3.14159265359;
	return angle_in_degrees;
}

class Event
{
	public:
		vector<MString> Lines;
		MTime Time;
		MTime AspectTime;
		double Heading;
		double Pitch;
		double Roll;
		double Lat;
		double Lon;
		vector<double> HX;
		vector<double> HZ;
		vector<double> GX;
		vector<double> GZ;
		Event(): HX(2),HZ(2),GX(2),GZ(2)
	{
		Heading = 0;
		Pitch = 0;
		Roll = 0;
		Lat = 0;
		Lon = 0;
	}
};

bool ComputeRotation(Event* E, double dh, double dp, double dr){

	double heading = E->Heading;
	double pitch = E->Pitch;
	double roll = E->Roll;
	double Z[3][3], Y[3][3], X[3][3], YX[3][3], ZYX[3][3];
	double Z_[3][3], Y_[3][3], X_[3][3], YX_[3][3], ZYX_[3][3];
	MNCTTimeAndCoordinate TC;

	////ZYX is the rotation applied once the croystat is aligned with the GPS
	/////////////////////////////////////////////////////////////////////////
	Z[0][0] = cosine(heading);
	Z[0][1] = 0.0 - sine(heading);
	Z[0][2] = 0.0;

	Z[1][0] = sine(heading);
	Z[1][1] = cosine(heading);
	Z[1][2] = 0.0;

	Z[2][0] = 0.0;
	Z[2][1] = 0.0;
	Z[2][2] = 1.0;

	Y[0][0] = cosine(roll);
	Y[0][1] = 0.0;
	Y[0][2] = sine(roll);

	Y[1][0] = 0.0;
	Y[1][1] = 1.0;
	Y[1][2] = 0.0;

	Y[2][0] = 0.0 - sine(roll);
	Y[2][1] = 0.0;
	Y[2][2] = cosine(roll);

	X[0][0] = 1.0;
	X[0][1] = 0.0;
	X[0][2] = 0.0;

	X[1][0] = 0.0;
	X[1][1] = cosine(pitch);
	X[1][2] = 0.0 - sine(pitch);

	X[2][0] = 0.0;
	X[2][1] = sine(pitch);
	X[2][2] = cosine(pitch);

	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){   
			YX[i][j]=0;
			for(int k=0;k<3;k++){
				//ares' X and Y are backwards, changing this from YX to XY
				//so the full rotation is ZXY
				//YX[i][j]=YX[i][j]+Y[i][k]*X[k][j];
				YX[i][j]=YX[i][j]+X[i][k]*Y[k][j];
			}
		}
	}

	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){   
			ZYX[i][j]=0;
			for(int k=0;k<3;k++){
				ZYX[i][j]=ZYX[i][j]+Z[i][k]*YX[k][j];
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////



	////underscored matrices are for finding the cryostat vectors in the GPS frame
	/////////////////////////////////////////////////////////////////
	Z_[0][0] = cosine(-90 + dh);
	Z_[0][1] = 0.0 - sine(-90 + dh);
	Z_[0][2] = 0.0;

	Z_[1][0] = sine(-90 + dh);
	Z_[1][1] = cosine(-90 + dh);
	Z_[1][2] = 0.0;

	Z_[2][0] = 0.0;
	Z_[2][1] = 0.0;
	Z_[2][2] = 1.0;

	Y_[0][0] = cosine(dr);
	Y_[0][1] = 0.0;
	Y_[0][2] = sine(dr);

	Y_[1][0] = 0.0;
	Y_[1][1] = 1.0;
	Y_[1][2] = 0.0;

	Y_[2][0] = 0.0 - sine(dr);
	Y_[2][1] = 0.0;
	Y_[2][2] = cosine(dr);

	X_[0][0] = 1.0;
	X_[0][1] = 0.0;
	X_[0][2] = 0.0;

	X_[1][0] = 0.0;
	X_[1][1] = cosine(dp);
	X_[1][2] = 0.0 - sine(dp);

	X_[2][0] = 0.0;
	X_[2][1] = sine(dp);
	X_[2][2] = cosine(dp);

	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){   
			YX_[i][j]=0;
			for(int k=0;k<3;k++){
				//ares' X and Y are backwards, changing this from YX to XY
				//so the full rotation is ZXY
				//YX[i][j]=YX[i][j]+Y[i][k]*X[k][j];
				YX_[i][j]=YX_[i][j]+X_[i][k]*Y_[k][j];
			}
		}
	}

	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){   
			ZYX_[i][j]=0;
			for(int k=0;k<3;k++){
				ZYX_[i][j]=ZYX_[i][j]+Z_[i][k]*YX_[k][j];
			}
		}
	}
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////


	double Zhat[3];
	double Xhat[3];
	double Yhat[3];
	double temp[3];

	Zhat[0] = 0.0;
	Zhat[1] = 0.0;
	Zhat[2] = 1.0;

	Xhat[0] = 1.0;
	Xhat[1] = 0.0;
	Xhat[2] = 0.0;

	Yhat[0] = 0.0;
	Yhat[1] = 1.0;
	Yhat[2] = 0.0;

	//apply underscore rotation to align cryostat
	/////////////////////////////////////////////
	//transform Yhat

	for(int i=0;i<3;i++){
		double Q = 0;
		for(int k=0;k<3;k++){
			Q += ZYX_[i][k] * Yhat[k];
		}
		temp[i] = Q;
	}
	Yhat[0] = temp[0]; Yhat[1] = temp[1]; Yhat[2] = temp[2];

	//transform Xhat
	for(int i=0;i<3;i++){
		double Q = 0;
		for(int k=0;k<3;k++){
			Q += ZYX_[i][k] * Xhat[k];
		}
		temp[i] = Q;
	}
	Xhat[0] = temp[0]; Xhat[1] = temp[1]; Xhat[2] = temp[2];

	//transform Zhat
	for(int i=0;i<3;i++){
		double Q = 0;
		for(int k=0;k<3;k++){
			Q += ZYX_[i][k] * Zhat[k];
		}
		temp[i] = Q;
	}
	Zhat[0] = temp[0]; Zhat[1] = temp[1]; Zhat[2] = temp[2];

	/*
	for(int i=0;i<3;i++){
		double Q = 0;
		for(int k=0;k<3;k++){
			Q += Z_[i][k] * Yhat[k];
		}
		temp[i] = Q;
	}
	Yhat[0] = temp[0]; Yhat[1] = temp[1]; Yhat[2] = temp[2];

	//transform Xhat
	for(int i=0;i<3;i++){
		double Q = 0;
		for(int k=0;k<3;k++){
			Q += Z_[i][k] * Xhat[k];
		}
		temp[i] = Q;
	}
	Xhat[0] = temp[0]; Xhat[1] = temp[1]; Xhat[2] = temp[2];
	*/

	/////////////////////////////////////////////
	/////////////////////////////////////////////


	//now apply full heading/pitch/roll rotation
	////////////////////////////////////////////
	for(int i=0;i<3;i++){
		double Q = 0;
		for(int k=0;k<3;k++){
			Q += ZYX[i][k] * Yhat[k];
		}
		temp[i] = Q;
	}
	Yhat[0] = temp[0]; Yhat[1] = temp[1]; Yhat[2] = temp[2];

	//transform Xhat
	for(int i=0;i<3;i++){
		double Q = 0;
		for(int k=0;k<3;k++){
			Q += ZYX[i][k] * Xhat[k];
		}
		temp[i] = Q;
	}
	Xhat[0] = temp[0]; Xhat[1] = temp[1]; Xhat[2] = temp[2];

	//transform Zhat
	for(int i=0;i<3;i++){
		double Q = 0;
		for(int k=0;k<3;k++){
			Q += ZYX[i][k] * Zhat[k];
		}
		temp[i] = Q;
	}
	Zhat[0] = temp[0]; Zhat[1] = temp[1]; Zhat[2] = temp[2];
	////////////////////////////////////////////
	////////////////////////////////////////////

	//dot Zhat/Xhat into (0,0,1) and take the arcos to get the zenith angle.  then convert this to elevation angle
	double Z_Elevation = 90.0 - arccosine( Zhat[2] );
	double X_Elevation = 90.0 - arccosine( Xhat[2] );
	double Y_Elevation = 90.0 - arccosine( Yhat[2] );

	//get azimuth of Zhat/Xhat by projecting into X-Y plane, re-normalizing, and dotting into (0,1,0)
	double Z_proj[3]; Z_proj[0] = Zhat[0]; Z_proj[1] = Zhat[1]; Z_proj[2] = 0.0;
	double X_proj[3]; X_proj[0] = Xhat[0]; X_proj[1] = Xhat[1]; X_proj[2] = 0.0;
	double Y_proj[3]; Y_proj[0] = Yhat[0]; Y_proj[1] = Yhat[1]; Y_proj[2] = 0.0;

	double Znorm = sqrt( pow(Z_proj[0],2.0) + pow(Z_proj[1],2.0) );
	double Xnorm = sqrt( pow(X_proj[0],2.0) + pow(X_proj[1],2.0) );
	double Ynorm = sqrt( pow(Y_proj[0],2.0) + pow(Y_proj[1],2.0) );

	double Z_Azimuth = 0;
	if( Znorm > 1.0E-9 ){
		Z_Azimuth = arccosine( Z_proj[1]/Znorm ); if( Z_proj[0] < 0.0 ) Z_Azimuth = 360.0 - Z_Azimuth;
	} else {
		Z_Azimuth = 0.0;
	}

	double X_Azimuth = 0;
	if( Xnorm > 1.0E-9 ){
		X_Azimuth = arccosine( X_proj[1]/Xnorm ); if( X_proj[0] < 0.0 ) X_Azimuth = 360.0 - X_Azimuth;
	} else {
		X_Azimuth = 0.0;
	}

	double Y_Azimuth  = 0;
	if( Ynorm > 1.0E-9 ){
		Y_Azimuth = arccosine( Y_proj[1]/Ynorm ); if( Y_proj[0] < 0.0 ) Y_Azimuth = 360.0 - Y_Azimuth;
	} else {
		Y_Azimuth = 0.0;
	}

	E->HX[0] = X_Azimuth;
	E->HX[1] = X_Elevation;
	E->HZ[0] = Z_Azimuth;
	E->HZ[1] = Z_Elevation;

	TC.SetLocation(E->Lat, E->Lon);
	TC.SetUnixTime(E->AspectTime.GetAsSystemSeconds());

	vector<double> ZGalactic; 
	vector<double> ZEquatorial;
	ZEquatorial = TC.Horizon2Equatorial(Z_Azimuth, Z_Elevation);
	ZGalactic = TC.Equatorial2Galactic(ZEquatorial);
	double Zgalat = ZGalactic[1];
	double Zgalon = ZGalactic[0];

	vector<double> XGalactic; 
	vector<double> XEquatorial;
	XEquatorial= TC.Horizon2Equatorial(X_Azimuth, X_Elevation);
	XGalactic = TC.Equatorial2Galactic(XEquatorial);	
	double Xgalat = XGalactic[1];
	double Xgalon = XGalactic[0];

	vector<double> YGalactic; 
	vector<double> YEquatorial;
	YEquatorial= TC.Horizon2Equatorial(Y_Azimuth, Y_Elevation);
	YGalactic = TC.Equatorial2Galactic(YEquatorial);	
	double Ygalat = YGalactic[1];
	double Ygalon = YGalactic[0];

	E->GX[0] = Xgalon;
	E->GX[1] = Xgalat;
	E->GZ[0] = Zgalon;
	E->GZ[1] = Zgalat;

	return true;
}



int main(int argc, char ** argv){
	if(argc != 5){
		cout << "need four arguments: fname, heading, pitch, roll" << endl;
		return -1;
	} 

	double heading = MString(argv[2]).ToDouble();
	double pitch = MString(argv[3]).ToDouble();
	double roll = MString(argv[4]).ToDouble();

	MFile fin;
	fin.Open(MString(argv[1]));

	MFile fout;
	fout.Open(MString(argv[1]) + ".out",MFile::c_Write);

	MString line;
	Event* E = new Event();
	while(fin.ReadLine(line)){
		if(line.BeginsWith("SE")){
			//do rotations
			ComputeRotation(E, heading, pitch, roll);
			//write lines
			for(auto m: E->Lines){
				char line_[128];
				if(m.BeginsWith("GX")){
					snprintf(line_,sizeof(line_),"GX %6.3f %6.3f\n",E->GX[0],E->GX[1]);
					fout.Write(line_);
				} else if(m.BeginsWith("GZ")){
					snprintf(line_,sizeof(line_),"GZ %6.3f %6.3f\n",E->GZ[0],E->GZ[1]);
					fout.Write(line_);
				} else if(m.BeginsWith("HX")){
					snprintf(line_,sizeof(line_),"HX %6.3f %6.3f\n",E->HX[0],E->HX[1]);
					fout.Write(line_);
				} else if(m.BeginsWith("HZ")){
					snprintf(line_,sizeof(line_),"HZ %6.3f %6.3f\n",E->HZ[0],E->HZ[1]);
					fout.Write(line_);
				} else {
					fout.Write(m + "\n");
				}
			}
			delete E;
			E = new Event();
			E->Lines.push_back(line);
		} else if(line.BeginsWith("CC AS")){
			vector<MString> Tokens = line.Tokenize(" ");
			if(Tokens.size() == 8){
				E->Lat = Tokens[2].ToDouble();
				E->Lon = Tokens[3].ToDouble();
				E->Heading = Tokens[4].ToDouble();
				E->Pitch = Tokens[5].ToDouble();
				E->Roll = Tokens[6].ToDouble();
				MTime t; t.Set(Tokens[7]);
				E->AspectTime = t;

			}
			E->Lines.push_back(line);
		} else if(line.BeginsWith("TI")){
			MTime t;
			t.Set(line);
			E->Time = t;
			E->Lines.push_back(line);
		} else {
			E->Lines.push_back(line);
		}

	}
	fout.Close();
	fin.Close();

	return 0;

}





