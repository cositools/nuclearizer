

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>

using namespace std;

/* ROOT */
#include <TROOT.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TLine.h>
#include <getopt.h>

/* YAML */
#include <yaml-cpp/yaml.h>

/* MEGAlib */
#include "MGlobal.h"
#include "MSupervisor.h"
#include "MModuleLoaderMeasurementsHDF.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MString.h"


/* ------------------------------------------------------------- */
/* Strip identifier structure                                     */
/* ------------------------------------------------------------- */

struct StripKey
{
  int det;
  char side;
  int strip;

  bool operator<(const StripKey& o) const
  {
    if(det!=o.det) return det<o.det;
    if(side!=o.side) return side<o.side;
    return strip<o.strip;
  }
};


/* -------------------------------------------------------------   */
/* Simple calibration helper                                       */
/* Converts ADC → Energy using polynomial calibration coefficients */
/* -------------------------------------------------------------   */

class EnergyCalHelper
{
public:

  struct Coeff
  {
    double c0=0;
    double c1=0;
    double c2=0;
    double c3=0;
  };

  bool Load(const string& fileName,int nDet=64,int nStrip=65)
  {
    m_NDet=nDet;
    m_NStrip=nStrip;

    m_Coeffs.resize(m_NDet);

    for(int d=0;d<m_NDet;d++)
    {
      m_Coeffs[d].resize(2);

      for(int s=0;s<2;s++)
        m_Coeffs[d][s].resize(m_NStrip);
    }

    ifstream in(fileName);

    if(!in)
    {
      cout<<"Unable to open calibration file "<<fileName<<endl;
      return false;
    }

    string line;

    while(getline(in,line))
    {
      if(line.empty()) continue;

      stringstream ss(line);

      string tag;
      ss>>tag;

      if(tag!="CM") continue;

      string unused;
      int det=-1;
      int strip=-1;
      string side;
      string order;

      ss>>unused>>det>>strip>>side>>order;

      if(det<0||det>=m_NDet) continue;
      if(strip<0||strip>=m_NStrip) continue;

      int sideInt=(side=="l")?0:1;

      Coeff c;

      if(order=="poly1zero")
        ss>>c.c1;
      else if(order=="poly1")
        ss>>c.c0>>c.c1;
      else if(order=="poly2")
        ss>>c.c0>>c.c1>>c.c2;
      else
        ss>>c.c0>>c.c1>>c.c2>>c.c3;

      m_Coeffs[det][sideInt][strip]=c;
    }

    return true;
  }

  double ADCToEnergy(int det,char side,int strip,double adc) const
  {
    int sideInt=(side=='l')?0:1;
    const Coeff& c=m_Coeffs[det][sideInt][strip];

    return c.c3*pow(adc,3)+c.c2*pow(adc,2)+c.c1*adc+c.c0;
  }

private:

  int m_NDet;
  int m_NStrip;

  vector<vector<vector<Coeff>>> m_Coeffs;
};

/* ------------------------------------------------------------- */
/* TAC calibration helper                                        */
/* ------------------------------------------------------------- */

class TACCalHelper
{
public:

  struct Coeff
  {
    double slope = 0;
    double offset = 0;
  };

  bool Load(const string& fileName)
  {
    ifstream in(fileName);
    if(!in)
    {
      cout<<"Failed to open TAC calibration file: "<<fileName<<endl;
      return false;
    }

    string line;
    getline(in,line); // skip header

    while(getline(in,line))
    {
      if(line.empty()) continue;

      stringstream ss(line);

      int strip_id, det, side, strip;
      double slope, slope_err, offset, offset_err;

      ss >> strip_id >> det >> side >> strip
         >> slope >> slope_err >> offset >> offset_err;

      char sideChar = (side==0) ? 'l' : 'h';

      StripKey key{det, sideChar, strip};

      m_Coeffs[key] = {slope, offset};
    }

    return true;
  }

  double TACToEnergy(int det, char side, int strip, double tac) const
  {
    StripKey key{det, side, strip};

    auto it = m_Coeffs.find(key);
    if(it == m_Coeffs.end()) return 0;

    return it->second.slope * tac + it->second.offset;
  }

private:
  map<StripKey, Coeff> m_Coeffs;
};


/* ------------------------------------------------------------- */
/* Hit selection                                                 */
/* ------------------------------------------------------------- */

bool PassHitSelection(MStripHit* SH)
{
  if(SH==nullptr) return false;

  if(SH->IsGuardRing()) return true;
  if(SH->IsNearestNeighbor()) return false;

  return true;
}


/* ------------------------------------------------------------- */
/* Main program                                                   */
/* ------------------------------------------------------------- */

int main(int argc,char** argv)
{
  
  /* ------------------------------------------------------------- */
  /* Allow help menu without requiring config.yaml                 */
  /* ------------------------------------------------------------- */

  for(int i=1;i<argc;i++)
  {
    string arg = argv[i];

    if(arg == "--help" || arg == "-h")
    {
      cout<<endl;
      cout<<"StripEnergyThresholdFinder"<<endl;
      cout<<endl;
      cout<<"Usage:"<<endl;
      cout<<"  ./StripEnergyThresholdFinder config.yaml [options]"<<endl;
      cout<<endl;
      cout<<"Options:"<<endl;
	  cout<<"Input/Output overrides:"<<endl;
      cout<<"  --data_file FILE            Add data file (can be used multiple times)"<<endl;
      cout<<"  --calibration_file FILE"<<endl;
      cout<<"  --strip_map FILE"<<endl;
      cout<<"  --output_prefix NAME"<<endl;
      cout<<endl;
      cout<<"  --min_entries N              Minimum entries required in strip histogram"<<endl;
      cout<<"  --noise_search_max_adc N     Maximum ADC value used when locating noise peak"<<endl;
      cout<<"  --fallback_threshold_keV N   Threshold assigned if algorithm fails"<<endl;
      cout<<"  --help                       Show this help menu"<<endl;
      cout<<endl;
      cout<<"Example:"<<endl;
      cout<<"  ./StripEnergyThresholdFinder config.yaml --noise_search_max_adc 1800"<<endl;
      cout<<endl;
      return 0;
    }
  }
   
  TH1::AddDirectory(false);
  MGlobal::Initialize("Standalone","ThresholdFinder");

  if(argc<2)
  {
    cout<<"Usage: ./StripEnergyThresholdFinder config.yaml"<<endl;
    return -1;
  }

  string configFile=argv[1];
  YAML::Node config=YAML::LoadFile(configFile);

  int minEntries=10;
  double fallbackThreshold=20;
  int histogramBins=2048;
  double histogramMaxADC=4096;
  double noise_search_max_adc=2200;
  
  /* ------------------------------------------------------------- */
  /* Command line override parameters (optional)                   */
  /* ------------------------------------------------------------- */

  int cmd_min_entries = -1;
  double cmd_noise_search_max_adc = -1;
  double cmd_fallback_threshold_keV = -1;
  
  string cmd_data_file = "";
  string cmd_calibration_file = "";
  string cmd_strip_map = "";
  string cmd_output_prefix = "";
  long max_events = -1;

  /* ------------------------------------------------------------- */
  /* YAML file input loading                                       */
  /* ------------------------------------------------------------- */

  if(config["analysis"])
  {
    if(config["analysis"]["min_entries"])
      minEntries=config["analysis"]["min_entries"].as<int>();

    if(config["analysis"]["fallback_threshold_keV"])
      fallbackThreshold=config["analysis"]["fallback_threshold_keV"].as<double>();

    if(config["analysis"]["histogram_bins"])
      histogramBins=config["analysis"]["histogram_bins"].as<int>();

    if(config["analysis"]["histogram_max_adc"])
      histogramMaxADC=config["analysis"]["histogram_max_adc"].as<double>();

    if(config["analysis"]["noise_search_max_adc"])
      noise_search_max_adc=config["analysis"]["noise_search_max_adc"].as<double>();
  }

  //vector<string> inputFiles=config["input"]["data_files"].as<vector<string>>();
  //MString stripMapFile=config["input"]["strip_map"].as<string>().c_str();
  //string outfile=config["output"]["prefix"].as<string>();

  vector<string> inputFiles=config["input"]["data_files"].as<vector<string>>();
  string calibrationFile=config["input"]["calibration_file"].as<string>();
  string stripMapFileStr=config["input"]["strip_map"].as<string>();
  string outfile=config["output"]["prefix"].as<string>();


  EnergyCalHelper helperCal;
  TACCalHelper helperCal_TAC;
  
  if(!helperCal.Load(calibrationFile))
  {
    cout<<"Failed to load calibration file: "<<calibrationFile<<endl;
    return -1;
  }
  
  string tacCalibrationFile = config["input"]["tac_calibration_file"].as<string>();


  // NOTE: TAC calibration currently not used in threshold finding
  //if(!helperCal_TAC.Load(tacCalibrationFile))
    //return -1;

  /* ------------------------------------------------------------- */
  /* Parse optional command line overrides                         */
  /* ------------------------------------------------------------- */

  //for(int i=2;i<argc;i++)
  //{
   // string arg = argv[i];

    //if(arg=="--min_entries" && i+1<argc)
    //{
      //cmd_min_entries = atoi(argv[++i]);
    //}
    //else if(arg=="--noise_search_max_adc" && i+1<argc)
    //{
      //cmd_noise_search_max_adc = atof(argv[++i]);
    //}
    //else if(arg=="--fallback_threshold_keV" && i+1<argc)
    //{
      //cmd_fallback_threshold_keV = atof(argv[++i]);
    //}
  //}
  
  
  /* ------------------------------------------------------------- */
  /* Modern command line parser using getopt_long                  */
  /* ------------------------------------------------------------- */

  static struct option long_options[] =
  {
    {"min_entries", required_argument, 0, 'm'},
    {"noise_search_max_adc", required_argument, 0, 'n'},
    {"fallback_threshold_keV", required_argument, 0, 'f'},

    /* NEW overrides */
    {"data_file", required_argument, 0, 'd'},
    {"calibration_file", required_argument, 0, 'c'},
    {"strip_map", required_argument, 0, 's'},
    {"output_prefix", required_argument, 0, 'o'},
	{"max_events", required_argument, 0, 'e'},

    {"help", no_argument, 0, 'h'},
    {0,0,0,0}
  };

  optind = 2;   // skip program name and YAML file

  int opt;
  while((opt = getopt_long(argc, argv, "", long_options, NULL)) != -1)
  {
    switch(opt)
    {
      case 'm':
        cmd_min_entries = atoi(optarg);
        break;

      case 'n':
        cmd_noise_search_max_adc = atof(optarg);
        break;

      case 'f':
        cmd_fallback_threshold_keV = atof(optarg);
        break;
		
	  //case 'd':
        //cmd_data_file = optarg;
        //break;
      case 'd':
        inputFiles.push_back(optarg);
        break;
		
	  case 'e':
        max_events = atol(optarg);
        break; 
		
      case 'c':
        cmd_calibration_file = optarg;
        break;

      case 's':
        cmd_strip_map = optarg;
        break;

      case 'o':
        cmd_output_prefix = optarg;
        break;

      case 'h':
        cout<<endl;
        cout<<"Usage:"<<endl;
        cout<<"  ./StripEnergyThresholdFinder config.yaml [options]"<<endl;
        cout<<endl;
        cout<<"Options:"<<endl;
		cout<<"Input/Output overrides:"<<endl;
        cout<<"  --data_file FILE            Override YAML data file"<<endl;
        cout<<"  --calibration_file FILE     Override calibration file"<<endl;
        cout<<"  --strip_map FILE            Override strip map"<<endl;
        cout<<"  --output_prefix NAME        Override output file prefix"<<endl;
        cout<<endl;
        cout<<"  --min_entries N              Minimum histogram entries"<<endl;
        cout<<"  --noise_search_max_adc N     Max ADC for noise search"<<endl;
        cout<<"  --fallback_threshold_keV N   Default threshold if fit fails"<<endl;
        cout<<"  --help                       Show this message"<<endl;
        cout<<endl;
        exit(0);

      default:
        break;
    }
  }
  
  /* ------------------------------------------------------------- */
  /* Apply command line overrides for input/output                  */
  /* ------------------------------------------------------------- */

  //if(cmd_data_file != "")
  //{
    //inputFiles.clear();
    //inputFiles.push_back(cmd_data_file);
  //}

  if(cmd_calibration_file != "")
  {
    calibrationFile = cmd_calibration_file;
  }

  if(cmd_strip_map != "")
  {
    stripMapFileStr = cmd_strip_map;
  }

  if(cmd_output_prefix != "")
  {
    outfile = cmd_output_prefix;
  } 
  
  MString stripMapFile = stripMapFileStr.c_str();

  /* ------------------------------------------------------------- */
  /* Apply command line overrides if provided                      */
  /* ------------------------------------------------------------- */

  if(cmd_min_entries >= 0)
    minEntries = cmd_min_entries;

  if(cmd_noise_search_max_adc > 0)
    noise_search_max_adc = cmd_noise_search_max_adc;

  if(cmd_fallback_threshold_keV > 0)
    fallbackThreshold = cmd_fallback_threshold_keV;


  MSupervisor* S=MSupervisor::GetSupervisor();

  map<StripKey,TH1D*> histograms;
  map<StripKey,TH1D*> histograms_TAC;
  
  map<StripKey, map<int, pair<int,int>>> timingCounts;
  
  /* ------------------------------------------------------------- */
  /* Save configuration to log file                                */
  /* ------------------------------------------------------------- */
  
  cout<<endl;
  cout<<"  calibration_file:        "<<calibrationFile<<endl;
  cout<<"  strip_map:               "<<stripMapFileStr<<endl;
  cout<<"  output_prefix:           "<<outfile<<endl;

  cout<<"  data_files:"<<endl;
  for(auto &f : inputFiles)
      cout<<"    "<<f<<endl;

  cout<<endl;
  cout<<"Active analysis configuration:"<<endl;
  cout<<"  min_entries:            "<<minEntries<<endl;
  cout<<"  fallback_threshold_keV: "<<fallbackThreshold<<endl;
  cout<<"  noise_search_max_adc:   "<<noise_search_max_adc<<endl;
  cout<<"  histogram_bins:         "<<histogramBins<<endl;
  cout<<"  histogram_max_adc:      "<<histogramMaxADC<<endl;
  cout<<endl;


  /* ------------------------------------------------------------- */
  /* Build ADC histograms from data                                */
  /* ------------------------------------------------------------- */

  for(string inputFile:inputFiles)
  {

    MModuleLoaderMeasurementsHDF* Loader=new MModuleLoaderMeasurementsHDF();
    Loader->SetFileName(inputFile.c_str());
    Loader->SetFileNameStripMap(stripMapFile);

    S->SetModule(Loader,0);

    if(!Loader->Initialize()) return -1;

    MReadOutAssembly* Event=new MReadOutAssembly();
    
	long event_counter = 0;
    while(Loader->IsFinished()==false)
    {
      if(max_events > 0 && event_counter >= max_events)
        break;
	  Event->Clear();

      if(Loader->IsReady())
      {
        Loader->AnalyzeEvent(Event);
		event_counter++;

        for(unsigned int sh=0;sh<Event->GetNStripHits();sh++)
        {
          MStripHit* SH=Event->GetStripHit(sh);

          if(!PassHitSelection(SH)) continue;

          double adc=SH->GetADCUnits();
		  
		  int det=SH->GetDetectorID();
          int strip=SH->GetStripID();
          char side=SH->IsLowVoltageStrip()?'l':'h';
		  
		  StripKey key{det,side,strip};
		  
		  /* ------------------------------------------------------------- */
          /* Fast threshold data accumulation (dt0 vs dt1)                 */
          /* ------------------------------------------------------------- */

          int adc_int = (int)adc;

          // Define timing type
          int timing_type = SH->HasFastTiming() ? 1 : 0;

          // Accumulate counts
          auto& entry = timingCounts[key][adc_int];

          if(timing_type == 0)
            entry.first++;
          else
            entry.second++;
		  
		  /* ------------------------------------------------------------- */
          /* TAC (fast shaper) histogram filling                           */
          /* ------------------------------------------------------------- */

          if(SH->HasFastTiming() && SH->GetTAC() > 0)
          {
            double tac = SH->GetTAC();

            //StripKey key{det,side,strip};

            if(histograms_TAC[key]==nullptr)
            {
              string name="h_TAC_"+to_string(det)+"_"+side+"_"+to_string(strip);

              histograms_TAC[key]=new TH1D(
                name.c_str(),
                name.c_str(),
                histogramBins,0,histogramMaxADC);

              histograms_TAC[key]->GetXaxis()->SetTitle("TAC ADC");
              histograms_TAC[key]->GetYaxis()->SetTitle("Counts");
            }

            histograms_TAC[key]->Fill(tac);
          }
		  
		  
		  
		  static int debugCounter = 0;

          //if(debugCounter < 20)
          //{
          //  cout << "Slow ADC: " << adc << endl;
          //  cout << "TAC:   " << (SH->HasFastTiming() ? SH->GetTAC() : -1) << endl;
          //  cout << "------------------------" << endl;
          //  debugCounter++;
          //}  

          //int det=SH->GetDetectorID();
          //int strip=SH->GetStripID();
          //char side=SH->IsLowVoltageStrip()?'l':'h';

          //StripKey key{det,side,strip};

          if(histograms[key]==nullptr)
          {
            string name="h_"+to_string(det)+"_"+side+"_"+to_string(strip);

            histograms[key]=new TH1D(name.c_str(),name.c_str(),
                                     histogramBins,0,histogramMaxADC);

            histograms[key]->GetXaxis()->SetTitle("ADC");
            histograms[key]->GetYaxis()->SetTitle("Counts");
          }

          histograms[key]->Fill(adc);
        }
      }
    }

    delete Event;
  }


  /* ------------------------------------------------------------- */
  /* Threshold finding algorithm                                   */
  /* ------------------------------------------------------------- */

  map<StripKey,double> thresholds;
  map<StripKey,double> thresholdsADC;

  map<int,double> thresholdLV;
  map<int,double> thresholdHV;

  map<int,double> thresholdLV_ADC;
  map<int,double> thresholdHV_ADC;
  
  /* ------------------------------------------------------------- */
  /* Fast shaper (TAC) histograms                                  */
  /* ------------------------------------------------------------- */

  //map<StripKey,TH1D*> histograms_TAC;
  
  /* ------------------------------------------------------------- */
  /* TAC threshold storage                                         */
  /* ------------------------------------------------------------- */

  map<StripKey,double> thresholds_TAC;
  map<StripKey,double> thresholds_TAC_ADC;
  //map<StripKey, map<int, pair<int,int>>> timingCounts;
  
  
  /* HV/LV split */

  map<int,double> thresholdLV_TAC;
  map<int,double> thresholdHV_TAC;

  map<int,double> thresholdLV_TAC_ADC;
  map<int,double> thresholdHV_TAC_ADC;
  
  
  
  /* ------------------------------------------------------------- */
  /* Diagnostic storage vectors                                     */
  /* These vectors allow us to build ROOT diagnostic plots later    */
  /* ------------------------------------------------------------- */

  vector<double> stripIndex;
  vector<double> thresholdValues;
  vector<double> noisePeakADC;
  
  /* ------------------------------------------------------------- */
  /* Separate vectors for HV and LV strip diagnostics               */
  /* ------------------------------------------------------------- */

  vector<double> stripIndex_LV;
  vector<double> stripIndex_HV;

  vector<double> thresholdValues_LV;
  vector<double> thresholdValues_HV;

  vector<double> noisePeakADC_LV;
  vector<double> noisePeakADC_HV;
  
  /* ------------------------------------------------------------- */
  /* TAC diagnostic vectors                                        */
  /* ------------------------------------------------------------- */

  vector<double> stripIndex_TAC_LV;
  vector<double> stripIndex_TAC_HV;

  vector<double> thresholdValues_TAC_LV;
  vector<double> thresholdValues_TAC_HV;

  vector<double> tacPeakADC_LV;
  vector<double> tacPeakADC_HV;
  
  vector<double> slowEnergyVec;
  vector<double> tacEnergyVec;

  for(auto& kv:histograms)
  {

    StripKey key=kv.first;
    TH1D* hist=kv.second;

    if(hist->GetEntries()<minEntries)
    {
      thresholds[key]=fallbackThreshold;
      continue;
    }

    hist->Smooth(3);

    int maxSearchBin=hist->FindBin(noise_search_max_adc);

    int startBin=-1;

    for(int b=1;b<=maxSearchBin;b++)
      if(hist->GetBinContent(b)>5){ startBin=b; break; }

    if(startBin<0)
    {
      thresholds[key]=fallbackThreshold;
      continue;
    }

    int peakBin=startBin;
    double peakCounts=hist->GetBinContent(startBin);

    for(int b=startBin+1;b<=maxSearchBin;b++)
    {
      double c=hist->GetBinContent(b);

      if(c>peakCounts){ peakCounts=c; peakBin=b; }
      else if(b>peakBin && c<peakCounts*0.9) break;
    }

    int thresholdBin=peakBin;

    if(key.strip==64)
    {
      for(int b=peakBin+1;b<=maxSearchBin;b++)
        if(hist->GetBinContent(b)<=peakCounts*0.5)
        { thresholdBin=b; break; }
    }
    else
    {
      for(int b=peakBin+1;b<=maxSearchBin;b++)
      {
        double c=hist->GetBinContent(b);

        if(c<hist->GetBinContent(thresholdBin))
          thresholdBin=b;

        if(b>peakBin && c>peakCounts*0.5)
          break;
      }
    }
	
	/* ------------------------------------------------------------- */
    /* Build Slow vs TAC energy comparison                           */
    /* ------------------------------------------------------------- */

   /* for(const auto& kv : thresholds)
    {
      StripKey key = kv.first;

      // Check if TAC threshold exists for same strip
      if(thresholds_TAC.find(key) == thresholds_TAC.end())
        continue;

      double slowE = kv.second;
      double tacE  = thresholds_TAC[key];

      // Optional: skip unphysical values
      if(tacE <= 0 || slowE <= 0)
        continue;

      slowEnergyVec.push_back(slowE);
      tacEnergyVec.push_back(tacE);
    }
	*/
	

    /* ------------------------------------------------------------- */
    /* Shift threshold slightly to the right of the noise trough     */
    /* This prevents thresholds from sitting inside the noise tail   */
    /* ------------------------------------------------------------- */

    int shiftBins = 2;   // move threshold slightly right
    thresholdBin = min(thresholdBin + shiftBins, hist->GetNbinsX());

    double thresholdADC = hist->GetBinCenter(thresholdBin);

    /* Convert ADC → keV using SLOW calibration */
    double thresholdKeV =
      helperCal.ADCToEnergy(key.det,key.side,key.strip,thresholdADC);

    /* Store SLOW thresholds */
    thresholds[key] = thresholdKeV;
    thresholdsADC[key] = thresholdADC;

    /* ------------------------------------------------------------- */
    /* Store values for diagnostic plots                             */
    /* ------------------------------------------------------------- */

    stripIndex.push_back(key.strip);
    thresholdValues.push_back(thresholdKeV);
    noisePeakADC.push_back(hist->GetBinCenter(peakBin));
	
	/* ------------------------------------------------------------- */
    /* Store HV and LV diagnostics separately so both appear in plots */
    /* ------------------------------------------------------------- */

    if(key.side=='l')
    {
        stripIndex_LV.push_back(key.strip);
        thresholdValues_LV.push_back(thresholdKeV);
        noisePeakADC_LV.push_back(hist->GetBinCenter(peakBin));
    }
    else
    {
        stripIndex_HV.push_back(key.strip);
        thresholdValues_HV.push_back(thresholdKeV);
        noisePeakADC_HV.push_back(hist->GetBinCenter(peakBin));
    }

    if(key.side=='l')
    {
	  thresholdLV[key.strip]=thresholdKeV;
      thresholdLV_ADC[key.strip]=thresholdADC;
    }
    else
    {
      thresholdHV[key.strip]=thresholdKeV;
      thresholdHV_ADC[key.strip]=thresholdADC;
    }
  }


  
  /* ------------------------------------------------------------- */
  /* Write CSV threshold tables (HV and LV)                        */
  /* ------------------------------------------------------------- */

  ofstream csv_HV(outfile + "_Slow_HV_thresholds.csv");
  ofstream csv_LV(outfile + "_Slow_LV_thresholds.csv");

  /* CSV headers */

  csv_HV << "detector_side,strip,threshold_adc,threshold_keV\n";
  csv_LV << "detector_side,strip,threshold_adc,threshold_keV\n";

  /* Write rows */

  for(const auto& kv : thresholds)
  {
    char side = kv.first.side;
    int strip = kv.first.strip;

    double thr_keV = kv.second;
    //double thr_adc = thresholdsADC[kv.first];
    auto it_adc = thresholds_TAC_ADC.find(kv.first);
    if(it_adc == thresholds_TAC_ADC.end()) continue;

    double thr_adc = it_adc->second;
    
	
	
	if(side == 'h')
    {
      csv_HV << "h,"
             << strip << ","
             << thr_adc << ","
             << thr_keV << "\n";
    }
    else if(side == 'l')
    {
      csv_LV << "l,"
             << strip << ","
             << thr_adc << ","
             << thr_keV << "\n";
    }
  }

  csv_HV.close();
  csv_LV.close();
  
  ofstream csv_TAC_HV(outfile + "_Fast_HV_thresholds.csv");
  ofstream csv_TAC_LV(outfile + "_Fast_LV_thresholds.csv");

  csv_TAC_HV << "detector_side,strip,threshold_adc\n";
  csv_TAC_LV << "detector_side,strip,threshold_adc\n";
  

  for(const auto& kv : thresholds_TAC)
  {
    char side = kv.first.side;
    int strip = kv.first.strip;
    //double thr_adc = kv.second;
	double thr_adc = thresholds_TAC_ADC[kv.first];
    double thr_keV = kv.second;

    if(side == 'h')
    {
      csv_TAC_HV << "h," << strip << "," << thr_adc << "\n";
    }
    else if(side == 'l')
    {
      csv_TAC_LV << "l," << strip << "," << thr_adc << "\n";
    }
  }

  csv_TAC_HV.close();
  csv_TAC_LV.close();
  
  /* ------------------------------------------------------------- */
  /* Pixel threshold maps                                          */
  /* ------------------------------------------------------------- */

  TH2D pixelThresholdMap(
    "PixelThresholdMap",
    "Pixel Threshold Map (keV);LV Strip;HV Strip",
    64,0,64,
    64,64,0);

  TH2D pixelThresholdADCMap(
    "PixelThresholdADCMap",
    "Pixel Threshold Map (ADC);LV Strip;HV Strip",
    64,0,64,
    64,64,0);

  /* FIX: force ROOT to display full strip axes */

  pixelThresholdMap.GetXaxis()->SetNdivisions(64,false);
  pixelThresholdMap.GetYaxis()->SetNdivisions(64,false);

  pixelThresholdADCMap.GetXaxis()->SetNdivisions(64,false);
  pixelThresholdADCMap.GetYaxis()->SetNdivisions(64,false);


  for(int lv=0;lv<64;lv++)
  {
    if(thresholdLV.find(lv)==thresholdLV.end()) continue;

    for(int hv=0;hv<64;hv++)
    {
      if(thresholdHV.find(hv)==thresholdHV.end()) continue;

      double pixelThr=max(thresholdLV[lv],thresholdHV[hv]);
      pixelThresholdMap.Fill(lv,hv,pixelThr);

      double pixelADC=max(thresholdLV_ADC[lv],thresholdHV_ADC[hv]);
      pixelThresholdADCMap.Fill(lv,hv,pixelADC);
    }
  }


  /* ------------------------------------------------------------- */
  /* ROOT output                                                   */
  /* ------------------------------------------------------------- */

  TFile f((outfile+"_diagnostics.root").c_str(),"RECREATE");
  
  /* ------------------------------------------------------------- */
  /* Slow vs TAC Energy Correlation                                */
  /* ------------------------------------------------------------- */

  TGraph Slow_vs_TAC(
    slowEnergyVec.size(),
    slowEnergyVec.data(),
    tacEnergyVec.data());

  Slow_vs_TAC.SetName("Slow_vs_TAC");
  Slow_vs_TAC.SetTitle("Slow vs TAC Energy;Slow Energy (keV);TAC Energy (keV)");

  Slow_vs_TAC.SetMarkerStyle(20);
  Slow_vs_TAC.SetMarkerSize(1);

  Slow_vs_TAC.Write();

  /* ------------------------------------------------------------- */
  /*  Slow and Fast Threshold diagnostic plots                                     */
  /* ------------------------------------------------------------- */

  /* Histogram showing distribution of threshold values */

  TH1D ThresholdDistribution(
    "ThresholdDistribution",
    "Threshold Distribution;Threshold (keV);Counts",
    200,0,30);

  for(auto &kv : thresholds)
  {
    ThresholdDistribution.Fill(kv.second);
  }

  ThresholdDistribution.Write();


  /* Threshold vs strip number */

  //TGraph Threshold_vs_Strip(
    //stripIndex.size(),
    //stripIndex.data(),
    //thresholdValues.data());

  //Threshold_vs_Strip.SetName("Threshold_vs_Strip");
  //Threshold_vs_Strip.SetTitle("Threshold vs Strip;Strip;Threshold (keV)");
  //Threshold_vs_Strip.Write();
  //Threshold_vs_Strip.SetMarkerStyle(20);   // solid circle
  //Threshold_vs_Strip.SetMarkerSize(1);   // increase point size
  //Threshold_vs_Strip.SetMarkerColor(kBlue);
  //Threshold_vs_Strip.Write();
  
  TH1D FastThresholdDistribution(
    "FastThresholdDistribution",
    "Fast Threshold Distribution;Threshold (keV);Counts",
    200,0,50);

  for(auto &kv : thresholds_TAC)
  {
    FastThresholdDistribution.Fill(kv.second);
  }

FastThresholdDistribution.Write();
  
  
  
  /* ------------------------------------------------------------- */
  /* Threshold vs strip for HV and LV sides                        */
  /* ------------------------------------------------------------- */

  TGraph Threshold_vs_Strip_LV(
    stripIndex_LV.size(),
    stripIndex_LV.data(),
    thresholdValues_LV.data());

  Threshold_vs_Strip_LV.SetName("Threshold_vs_Strip_LV");
  Threshold_vs_Strip_LV.SetTitle("Threshold vs Strip;Strip;Threshold (keV)");

  Threshold_vs_Strip_LV.SetMarkerStyle(20);
  Threshold_vs_Strip_LV.SetMarkerColor(kBlue);
  Threshold_vs_Strip_LV.SetMarkerSize(1);

  Threshold_vs_Strip_LV.Write();


  TGraph Threshold_vs_Strip_HV(
    stripIndex_HV.size(),
    stripIndex_HV.data(),
    thresholdValues_HV.data());

  Threshold_vs_Strip_HV.SetName("Threshold_vs_Strip_HV");

  Threshold_vs_Strip_HV.SetMarkerStyle(20);
  Threshold_vs_Strip_HV.SetMarkerColor(kRed);
  Threshold_vs_Strip_HV.SetMarkerSize(1);

  Threshold_vs_Strip_HV.Write();




  /* Noise peak ADC position vs strip */

  //TGraph NoisePeakADC_vs_Strip(
   // stripIndex.size(),
    //stripIndex.data(),
    //noisePeakADC.data());

  //NoisePeakADC_vs_Strip.SetName("NoisePeakADC_vs_Strip");
  //NoisePeakADC_vs_Strip.SetTitle("Noise Peak ADC vs Strip;Strip;Noise Peak (ADC)");
  //NoisePeakADC_vs_Strip.SetMarkerStyle(20);
  //NoisePeakADC_vs_Strip.SetMarkerSize(1);
  //NoisePeakADC_vs_Strip.SetMarkerColor(kRed);
  //NoisePeakADC_vs_Strip.Write();

  //pixelThresholdMap.Write();
  //pixelThresholdADCMap.Write();

  /* ------------------------------------------------------------- */
  /* Noise peak ADC vs strip for HV and LV sides                   */
  /* ------------------------------------------------------------- */

  TGraph NoisePeakADC_vs_Strip_LV(
    stripIndex_LV.size(),
    stripIndex_LV.data(),
    noisePeakADC_LV.data());

  NoisePeakADC_vs_Strip_LV.SetName("NoisePeakADC_vs_Strip_LV");
  NoisePeakADC_vs_Strip_LV.SetTitle("Noise Peak ADC vs Strip;Strip;Noise Peak (ADC)");

  NoisePeakADC_vs_Strip_LV.SetMarkerStyle(20);
  NoisePeakADC_vs_Strip_LV.SetMarkerColor(kBlue);
  NoisePeakADC_vs_Strip_LV.SetMarkerSize(1);

  NoisePeakADC_vs_Strip_LV.Write();


  TGraph NoisePeakADC_vs_Strip_HV(
    stripIndex_HV.size(),
    stripIndex_HV.data(),
    noisePeakADC_HV.data());

  NoisePeakADC_vs_Strip_HV.SetName("NoisePeakADC_vs_Strip_HV");

  NoisePeakADC_vs_Strip_HV.SetMarkerStyle(20);
  NoisePeakADC_vs_Strip_HV.SetMarkerColor(kRed);
  NoisePeakADC_vs_Strip_HV.SetMarkerSize(1);

  NoisePeakADC_vs_Strip_HV.Write();

 

  
  /* ------------------------------------------------------------- */
  /* TAC Threshold vs Strip                                        */
  /* ------------------------------------------------------------- */

  TGraph Threshold_TAC_vs_Strip_LV(
    stripIndex_TAC_LV.size(),
    stripIndex_TAC_LV.data(),
    thresholdValues_TAC_LV.data());

  Threshold_TAC_vs_Strip_LV.SetName("Threshold_TAC_vs_Strip_LV");
  Threshold_TAC_vs_Strip_LV.SetTitle("TAC Threshold vs Strip;Strip;Threshold (keV)");
  Threshold_TAC_vs_Strip_LV.SetMarkerStyle(20);
  Threshold_TAC_vs_Strip_LV.SetMarkerColor(kBlue);
  Threshold_TAC_vs_Strip_LV.SetMarkerSize(1);

  Threshold_TAC_vs_Strip_LV.Write();


  TGraph Threshold_TAC_vs_Strip_HV(
    stripIndex_TAC_HV.size(),
    stripIndex_TAC_HV.data(),
    thresholdValues_TAC_HV.data());

  Threshold_TAC_vs_Strip_HV.SetName("Threshold_TAC_vs_Strip_HV");
  Threshold_TAC_vs_Strip_HV.SetMarkerStyle(20);
  Threshold_TAC_vs_Strip_HV.SetMarkerColor(kRed);
  Threshold_TAC_vs_Strip_HV.SetMarkerSize(1);

  Threshold_TAC_vs_Strip_HV.Write();
  
  
  
  
  
  /* ------------------------------------------------------------- */
  /* Write ADC spectra and create energy spectra with thresholds   */
  /* ------------------------------------------------------------- */

  for(auto& kv:histograms)
  {
    StripKey key=kv.first;
    TH1D* adcHist=kv.second;

    adcHist->Write();

    string name="Energy_"+to_string(key.det)+"_"+key.side+"_"+to_string(key.strip);

    TH1D* energyHist=new TH1D(
        name.c_str(),
        name.c_str(),
        adcHist->GetNbinsX(),
        0,
        helperCal.ADCToEnergy(key.det,key.side,key.strip,histogramMaxADC)
    );

    for(int b=1;b<=adcHist->GetNbinsX();b++)
    {
      double adc=adcHist->GetBinCenter(b);
      double energy=helperCal.ADCToEnergy(key.det,key.side,key.strip,adc);
      double counts=adcHist->GetBinContent(b);

      int ebin=energyHist->FindBin(energy);
      energyHist->AddBinContent(ebin,counts);
    }

    double thr=thresholds[key];

    TLine* line=new TLine(thr,0,thr,energyHist->GetMaximum());
    line->SetLineColor(kRed);
    line->SetLineWidth(2);

    energyHist->GetListOfFunctions()->Add(line);

    energyHist->Write();
  }
  
  /* ------------------------------------------------------------- */
  /* Fast threshold finder (dt0 vs dt1 crossover)                  */
  /* ------------------------------------------------------------- */

  for(auto& kv : timingCounts)
  {
    StripKey key = kv.first;
    auto& adcMap = kv.second;

    /*if(adcMap.size() < minEntries)
    {
      thresholds_TAC[key] = fallbackThreshold;
      continue;
    }*/
	
	int totalCounts = 0;
    for(auto& a : adcMap)
      totalCounts += a.second.first + a.second.second;

    //if(totalCounts < minEntries)
		
	// >>> NEW: Proper minEntries guard
    if(totalCounts < minEntries)
    {
      thresholds_TAC[key] = fallbackThreshold;
      continue;
    }

    // Find first nonzero ADC
    int first_nonzero = -1;

    for(auto& a : adcMap)
    {
      if(a.second.first + a.second.second > 0)
      {
        first_nonzero = a.first + 10;
        break;
      }
    }

    if(first_nonzero < 0)
    {
      thresholds_TAC[key] = fallbackThreshold;
      continue;
    }

    int bestADC = -1;
    int minDiff = 1e9;

    int nbins = 21;

    // Extend search window for stability
    int searchMax = first_nonzero + 800;

    for(int adc = first_nonzero; adc < searchMax; adc++)
    {
      int n_dt0 = 0;
      int n_dt1 = 0;

      for(int a = adc; a < adc + nbins; a++)
      {
        if(adcMap.find(a) == adcMap.end()) continue;

        n_dt0 += adcMap[a].first;
        n_dt1 += adcMap[a].second;
      }

      int diff;

      if(n_dt0 == 0 && n_dt1 == 0)
        diff = 1000000;
      else
        diff = abs(n_dt1 - n_dt0);

      if(diff < minDiff)
      {
        minDiff = diff;
        bestADC = adc;
      }
    }

    if(bestADC < 0)
    {
      thresholds_TAC[key] = fallbackThreshold;
      continue;
    }

    int fast_thresh_adc = bestADC + nbins/2;

    thresholds_TAC_ADC[key] = fast_thresh_adc;

    double fast_thresh_keV =
      helperCal.ADCToEnergy(key.det,key.side,key.strip,fast_thresh_adc);

    thresholds_TAC[key] = fast_thresh_keV;

    /* Store diagnostics */

    if(key.side=='l')
    {
      stripIndex_TAC_LV.push_back(key.strip);
      thresholdValues_TAC_LV.push_back(fast_thresh_keV);
    }
    else
    {
      stripIndex_TAC_HV.push_back(key.strip);
      thresholdValues_TAC_HV.push_back(fast_thresh_keV);
    }

    /* Debug print */

    cout << "[FAST] DET " << key.det
         << " " << key.side
         << " strip " << key.strip
         << " ADC: " << fast_thresh_adc
         << " keV: " << fast_thresh_keV
         << endl;
    
  } 
  
  /* ------------------------------------------------------------- */
  /* Write FAST threshold CSVs (MOVED HERE - AFTER computation)    */
  /* ------------------------------------------------------------- */

  //ofstream csv_TAC_HV(outfile + "_Fast_HV_thresholds.csv");
  //ofstream csv_TAC_LV(outfile + "_Fast_LV_thresholds.csv");

  //csv_TAC_HV << "detector_side,strip,threshold_adc\n";
  //csv_TAC_LV << "detector_side,strip,threshold_adc\n";

  /*for(const auto& kv : thresholds_TAC)
  {
    char side = kv.first.side;
    int strip = kv.first.strip;
    double thr_adc = thresholds_TAC_ADC[kv.first];

    if(side == 'h')
      csv_TAC_HV << "h," << strip << "," << thr_adc << "\n";
    else if(side == 'l')
      csv_TAC_LV << "l," << strip << "," << thr_adc << "\n";
  }

  csv_TAC_HV.close();
  csv_TAC_LV.close();
  */
  
  
  
  /* ------------------------------------------------------------- */
  /* Build Slow vs TAC comparison                                  */
  /* ------------------------------------------------------------- */

  for(const auto& kv : thresholds)
  {
    StripKey key = kv.first;

    if(thresholds_TAC.find(key) == thresholds_TAC.end())
      continue;

    double slowE = kv.second;
    double tacE  = thresholds_TAC[key];

    if(slowE > 0 && tacE > 0)
    {
      slowEnergyVec.push_back(slowE);
      tacEnergyVec.push_back(tacE);
    }
  }

  /* ------------------------------------------------------------- */
  /* dt0 vs dt1 diagnostic histogram                               */
  /* ------------------------------------------------------------- */
 
  
  for(auto& kv : timingCounts)
  {
    StripKey key = kv.first;
    auto& adcMap = kv.second;

    string name0 = "dt0_" + to_string(key.det) + "_" + key.side + "_" + to_string(key.strip);
    string name1 = "dt1_" + to_string(key.det) + "_" + key.side + "_" + to_string(key.strip);

    TH1D* h0 = new TH1D(name0.c_str(),name0.c_str(),4096,0,4096);
    TH1D* h1 = new TH1D(name1.c_str(),name1.c_str(),4096,0,4096);

    for(auto& a : adcMap)
    {
      int adc = a.first;
      int n0 = a.second.first;
      int n1 = a.second.second;

      h0->SetBinContent(adc+1,n0);
      h1->SetBinContent(adc+1,n1);
    }

    h0->SetLineColor(kRed);
    h1->SetLineColor(kBlue);

    h0->Write();
    h1->Write();
  }
  
  /* write TAC histograms */

  /*for(auto& kv:histograms_TAC)
  {
    kv.second->Write();
  }
  */

  f.Close();
  /* ------------------------------------------------------------- */
  /* Helpful ROOT instructions                                     */
  /* ------------------------------------------------------------- */

  cout<<endl;
  cout<<"Diagnostics written to "<<outfile<<"_diagnostics.root"<<endl;
  cout<<endl;

  cout<<"Example ROOT commands for diagnostics:"<<endl;
  cout<<"---------------------------------------"<<endl;

  cout<<"Open file:"<<endl;
  cout<<"  root -l "<<outfile<<"_diagnostics.root"<<endl;
  cout<<endl;

  cout<<"Pixel threshold heat maps:"<<endl;
  cout<<"  PixelThresholdMap->Draw(\"COLZ\")"<<endl;
  cout<<"  PixelThresholdADCMap->Draw(\"COLZ\")"<<endl;
  cout<<endl;

  cout<<"Threshold distribution:"<<endl;
  cout<<"  ThresholdDistribution->Draw()"<<endl;
  cout<<endl;

  cout<<endl;
  cout<<"Threshold vs strip (both detector sides):"<<endl;
  cout<<"  Threshold_vs_Strip_LV->Draw(\"AP\")"<<endl;
  cout<<"  Threshold_vs_Strip_HV->Draw(\"P SAME\")"<<endl;

  cout<<endl;
  cout<<"Noise peak vs strip:"<<endl;
  cout<<"  NoisePeakADC_vs_Strip_LV->Draw(\"AP\")"<<endl;
  cout<<"  NoisePeakADC_vs_Strip_HV->Draw(\"P SAME\")"<<endl;

  cout<<"Example energy spectrum with threshold:"<<endl;
  cout<<"  Energy_0_h_10->Draw()"<<endl;
  cout<<endl;

  cout<<"Zoom in on low-energy region:"<<endl;
  cout<<"  Energy_0_h_10->GetXaxis()->SetRangeUser(0,30)"<<endl;
  cout<<endl;

  cout<<"Example ADC spectrum:"<<endl;
  cout<<"  h_0_h_10->Draw()"<<endl;

  cout<<"Threshold calibration complete."<<endl;
  
  
  /*
  cout<<endl;
  cout<<"TAC (fast shaper) diagnostics:"<<endl;
  cout<<"---------------------------------------"<<endl;

  cout<<"Example TAC histogram:"<<endl;
  cout<<"  h_TAC_0_l_10->Draw()"<<endl;
  cout<<endl;

  cout<<"Zoom into pedestal region:"<<endl;
  cout<<"  h_TAC_0_l_10->GetXaxis()->SetRangeUser(0,200)"<<endl;
  cout<<endl;

  cout<<"Log scale (useful for tails):"<<endl;
  cout<<"  gPad->SetLogy(); h_TAC_0_l_10->Draw()"<<endl;
  cout<<endl;

  cout<<"Overlay slow vs TAC (same strip):"<<endl;
  cout<<"  h_0_l_10->Draw()"<<endl;
  cout<<"  h_TAC_0_l_10->SetLineColor(kRed)"<<endl;
  cout<<"  h_TAC_0_l_10->Draw(\"SAME\")"<<endl;
  cout<<endl;

  cout<<"Compare HV vs LV TAC:"<<endl;
  cout<<"  h_TAC_0_l_10->Draw()"<<endl;
  cout<<"  h_TAC_0_h_10->SetLineColor(kRed)"<<endl;
  cout<<"  h_TAC_0_h_10->Draw(\"SAME\")"<<endl;
  cout<<endl;

  cout<<"Draw TAC thresholds:"<<endl;
  cout<<"  Threshold_TAC_vs_Strip_LV->Draw(\"AP\")"<<endl;
  cout<<"  Threshold_TAC_vs_Strip_HV->Draw(\"P SAME\")"<<endl;
  cout<<endl;

  cout<<"Inspect a problematic strip (example strip 63):"<<endl;
  cout<<"  h_TAC_0_l_63->Draw()"<<endl;
  cout<<endl;

  cout<<"Loop over all TAC histograms (quick scan):"<<endl;
  cout<<"  TIter iter(gDirectory->GetList());"<<endl;
  cout<<"  TObject* obj = nullptr;"<<endl;
  cout<<"  while((obj = iter()) != nullptr){"<<endl;
  cout<<"    TString name = obj->GetName();"<<endl;
  cout<<"    if(name.BeginsWith(\"h_TAC\")){"<<endl;
  cout<<"      obj->Draw();"<<endl;
  cout<<"      gPad->WaitPrimitive();"<<endl;
  cout<<"    }"<<endl;
  cout<<"  }"<<endl;
  cout<<endl;
  
  cout<<endl;
  cout<<"Slow vs TAC energy correlation:"<<endl;
  cout<<"  Slow_vs_TAC->Draw(\"AP\")"<<endl;
  cout<<endl;
  */
  
  
  cout<<"Fast threshold diagnostic (dt0 vs dt1):"<<endl;
  cout<<"  dt0_0_l_10->Draw()"<<endl;
  cout<<"  dt1_0_l_10->SetLineColor(kBlue)"<<endl;
  cout<<"  dt1_0_l_10->Draw(\"SAME\")"<<endl;
  cout<<endl;
  
  
  return 0;
}