#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <tuple>                // to return multiple values
#include <chrono>               // for time estimations
#include <ctime>
#include <string>
#include <stdint.h>
#include <stdlib.h>             // exit
#include <vector>
#include <algorithm>            // std::sort
#include <sys/stat.h>
#include "TKey.h"
#include "FairEventHeader.h"    // for FairEventHeader
#include "SNDLHCEventHeader.h"  // for EventHeader
#include "FairRunAna.h"         // for FairRunAna
#include "FairRootManager.h"    // for FairRootManager
#include "FairParamList.h"
#include "FairLogger.h"
#include "ConvRawData.h"
#include "TClonesArray.h"
#include "TObjString.h"
#include "TROOT.h"
#include "TList.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TH1F.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "SndlhcHit.h"
#include "Scifi.h"	        // for SciFi detector
#include "sndScifiHit.h"	// for SciFi Hit
#include "MuFilterHit.h"	// for Muon Filter Hit
#include "TString.h"
#include "nlohmann/json.hpp"     // library to operate with json files
#include "boardMappingParser.h"  // for board mapping
#include "XrdCl/XrdClFile.hh"

using namespace std;
using namespace std::chrono;
using namespace XrdCl;


ConvRawData::ConvRawData()
    : FairTask("ConvRawData")
    , fEventTree(nullptr)
    , boards{}
    , fEventHeader(nullptr)
    , fSNDLHCEventHeader(nullptr)
    , fDigiSciFi(nullptr)
    , fDigiMuFilter(nullptr)
{}

ConvRawData::~ConvRawData() {}

InitStatus ConvRawData::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman) {
        LOG (error) << "-E- ConvRawData::Init: "
                    << "RootManager not instantiated!";
        return kFATAL;
    }
        
    // Reading input - initiating parameters
    TObjString* runN_obj = dynamic_cast<TObjString*>(ioman->GetObject("runN"));
    TObjString* pathCalib_obj = dynamic_cast<TObjString*>(ioman->GetObject("pathCalib"));
    TObjString* pathJSON_obj = dynamic_cast<TObjString*>(ioman->GetObject("pathJSON"));
    TObjString* nEvents_obj = dynamic_cast<TObjString*>(ioman->GetObject("nEvents"));
    TObjString* nStart_obj = dynamic_cast<TObjString*>(ioman->GetObject("nStart"));
    TObjString* debug_obj = dynamic_cast<TObjString*>(ioman->GetObject("debug"));
    TObjString* stop_obj = dynamic_cast<TObjString*>(ioman->GetObject("stop"));
    TObjString* heartBeat_obj = dynamic_cast<TObjString*>(ioman->GetObject("heartBeat"));
    TObjString* makeCalibration_obj = dynamic_cast<TObjString*>(ioman->GetObject("makeCalibration"));
    TObjString* chi2Max_obj = dynamic_cast<TObjString*>(ioman->GetObject("chi2Max"));
    TObjString* saturationLimit_obj = dynamic_cast<TObjString*>(ioman->GetObject("saturationLimit"));
    TObjString* newFormat_obj = dynamic_cast<TObjString*>(ioman->GetObject("newFormat"));
    TObjString* local_obj = dynamic_cast<TObjString*>(ioman->GetObject("local"));
    // TMap containing the filling scheme per given run
    FSmap = static_cast<TMap*>(ioman->GetObject("FSmap"));    
    // Input raw data file is read from the FairRootManager
    // This allows to have it in custom format, e.g. have arbitary names of TTrees
    TFile* f0 = dynamic_cast<TFile*>(ioman->GetObject("rawData"));
    // Set run parameters
    std::istringstream(runN_obj->GetString().Data()) >> frunNumber;
    fpathCalib = pathCalib_obj->GetString().Data();
    fpathJSON = pathJSON_obj->GetString().Data();
    std::istringstream(nEvents_obj->GetString().Data()) >> fnEvents;
    std::istringstream(nStart_obj->GetString().Data()) >> fnStart;
    std::istringstream(debug_obj->GetString().Data()) >> debug;
    std::istringstream(stop_obj->GetString().Data()) >> stop;
    std::istringstream(heartBeat_obj->GetString().Data()) >> fheartBeat;
    std::istringstream(makeCalibration_obj->GetString().Data()) >> makeCalibration;
    std::istringstream(chi2Max_obj->GetString().Data()) >> chi2Max;
    std::istringstream(saturationLimit_obj->GetString().Data()) >> saturationLimit;
    std::istringstream(newFormat_obj->GetString().Data()) >> newFormat;
    std::istringstream(local_obj->GetString().Data()) >> local;
    
    if (!newFormat)
    { 
       fEventTree = (TTree*)f0->Get("event"); 
       // Get board_x data
       TIter next(f0->GetListOfKeys());
       TKey *b = new TKey();
       string name;
       while ((b = (TKey*)next()))
       {
          name = b->GetName();
          // string.find func: If no matches were found, the function returns string::npos.
          if ( name.find("board") == string::npos ) continue;
          boards[name] = (TTree*)f0->Get(name.c_str());
        }
        // use FairRoot eventHeader class
        fEventHeader = new FairEventHeader();
        ioman->Register("EventHeader", "sndEventHeader", fEventHeader, kTRUE);
    }
    else
    {
        fEventTree = (TTree*)f0->Get("data");
        // use sndlhc eventHeader class
        fSNDLHCEventHeader = new SNDLHCEventHeader();
        ioman->Register("EventHeader.", "sndEventHeader", fSNDLHCEventHeader, kTRUE);        
    }
     
    fDigiSciFi    = new TClonesArray("sndScifiHit");
    ioman->Register("Digi_ScifiHits", "DigiScifiHit_det", fDigiSciFi, kTRUE);
    fDigiMuFilter = new TClonesArray("MuFilterHit");
    ioman->Register("Digi_MuFilterHits", "DigiMuFilterHit_det", fDigiMuFilter, kTRUE);
    ScifiDet = dynamic_cast<Scifi*> (gROOT->GetListOfGlobals()->FindObject("Scifi") );
    TStopwatch timerCSV;
    timerCSV.Start();
    read_csv(fpathCalib);
    timerCSV.Stop();
    LOG (info) << "Time to read CSV " << timerCSV.RealTime();
    
    //calibrationReport();
    TStopwatch timerBMap;
    timerBMap.Start();
    DetMapping(fpathJSON);
    if (newFormat) StartTimeofRun(fpathJSON);
    timerBMap.Stop();
    LOG (info) << "Time to set the board mapping " << timerBMap.RealTime();
    
    // Get the FairLogger
    FairLogger *logger = FairLogger::GetLogger();
    
    eventNumber = fnStart;
    
    return kSUCCESS;
}

void ConvRawData::Exec(Option_t* /*opt*/)
{     
     fDigiSciFi->Delete();
     fDigiMuFilter->Delete();
     
     if (!newFormat) Process0();
     else Process1();

     // Manually change event number as input file is not set as source for this task
     eventNumber++;

}

void ConvRawData::UpdateInput(int NewStart)
{ 
   fEventTree->Refresh();
   if (!newFormat)
      for (auto it : boards) boards[it.first]->Refresh();
   eventNumber = NewStart; 
}

void ConvRawData::Process0()
{   
  int indexSciFi{}, indexMuFilter{};
  bool scifi = false, mask = false;
  int board_id{};
  TTree* bt;
  int tofpet_id{}, tofpet_channel{}, tac{}, mat{};
  string station;
  double TDC{}, QDC{}, Chi2ndof{}, satur{};
  int A{};
  double B{};
  high_resolution_clock::time_point tE{}, t0{}, t1{}, t4{}, t5{}, t6{}, tt{};
  int system{}, key{}, sipmChannel{};
  string tmp;
  int nSiPMs{}, nSides{}, direction{}, detID{}, sipm_number{}, chan{}, orientation{}, sipmLocal{};
  int sipmID{};
  double test{};
  //TStopwatch timer;     
  
  tE = high_resolution_clock::now();
  //timer.Start();
  fEventTree->GetEvent(eventNumber);
  if ( eventNumber%fheartBeat == 0 )
  {
     tt = high_resolution_clock::now();
     time_t ttp = high_resolution_clock::to_time_t(tt);
     LOG (info) << "run " << frunNumber << " event " << eventNumber
                << " local time " << ctime(&ttp);
  }
     
  fEventHeader->SetRunId(frunNumber);
  fEventHeader->SetEventTime(fEventTree->GetLeaf("evtTimestamp")->GetValue());
  LOG (info) << "event: " << eventNumber << " timestamp: "
              << fEventTree->GetLeaf("evtTimestamp")->GetValue();
  // Delete pointer map elements
  for (auto it : digiSciFiStore)
  {
      delete it.second;
  }
  digiSciFiStore.clear();
  for (auto it : digiMuFilterStore)
  {
      delete it.second;
  }
  digiMuFilterStore.clear();
     
  // Loop over boards
  for ( auto board : boards )// loop over TTrees
  { 
       board_id = stoi(board.first.substr(board.first.find("_")+1));
       scifi = true;
       if (boardMaps["Scifi"].count(board.first)!=0) 
       {
         for (auto it : boardMaps["Scifi"][board.first])
         {
           station = it.first;
           mat = it.second;
         }         
       }
       else if (boardMaps["MuFilter"].count(board.first)!=0) scifi = false;
       else
       {
         LOG (error) << board.first << " not known. Serious error, stop!";
         break;
       }
       bt = boards[board.first];
       bt->GetEvent(eventNumber);
       // Loop over hits in event
       for ( int n = 0; n <  bt->GetLeaf("nHits")->GetValue(); n++ )
       {
         mask = false;
         LOG (info) << "In scifi? " << scifi 
                    << " " << board_id << " " << bt->GetLeaf("tofpetId")->GetValue(n)
                    << " " << bt->GetLeaf("tofpetChannel")->GetValue(n)
                    << " " << bt->GetLeaf("tac")->GetValue(n)
                    << " " << bt->GetLeaf("tCoarse")->GetValue(n)
                    << " " << bt->GetLeaf("tFine")->GetValue(n)
                    << " " << bt->GetLeaf("vCoarse")->GetValue(n)
                    << " " << bt->GetLeaf("vFine")->GetValue(n);
         t0 = high_resolution_clock::now();
         tofpet_id = bt->GetLeaf("tofpetId")->GetValue(n);
         tofpet_channel = bt->GetLeaf("tofpetChannel")->GetValue(n);
         tac = bt->GetLeaf("tac")->GetValue(n);
         /* Since run 39 calibration is done online and 
         calib data stored in root raw data file */
         if (makeCalibration)
           tie(TDC,QDC,Chi2ndof,satur) = comb_calibration(board_id, tofpet_id, tofpet_channel, tac,
                                                     bt->GetLeaf("vCoarse")->GetValue(n),
                                                     bt->GetLeaf("vFine")->GetValue(n),
                                                     bt->GetLeaf("tCoarse")->GetValue(n),
                                                     bt->GetLeaf("tFine")->GetValue(n),
                                                     1.0, 0);
         else
         {
           TDC = bt->GetLeaf("timestamp")->GetValue(n);
           QDC = bt->GetLeaf("value")->GetValue(n);
           Chi2ndof = max(bt->GetLeaf("timestampCalChi2")->GetValue(n)/bt->GetLeaf("timestampCalDof")->GetValue(n),
                          	bt->GetLeaf("valueCalChi2")->GetValue(n)/bt->GetLeaf("valueCalDof")->GetValue(n));
           satur = bt->GetLeaf("valueSaturation")->GetValue(n);
         }
         
         // Print a warning if TDC or QDC is nan.        
         if ( TDC != TDC || QDC!=QDC) {
         LOG (error) << "NAN tdc/qdc detected! Check maps!"
                       << " " << board_id << " " << bt->GetLeaf("tofpetId")->GetValue(n)
                       << " " << bt->GetLeaf("tofpetChannel")->GetValue(n)
                       << " " << bt->GetLeaf("tac")->GetValue(n)
                       << " " << bt->GetLeaf("tCoarse")->GetValue(n)
                       << " " << bt->GetLeaf("tFine")->GetValue(n)
                       << " " << bt->GetLeaf("vCoarse")->GetValue(n)
                       << " " << bt->GetLeaf("vFine")->GetValue(n);
         }
         
         t1 = high_resolution_clock::now();
         if ( Chi2ndof > chi2Max )
         {
           if (QDC>1E20) QDC = 997.; // checking for inf
           if (QDC != QDC) QDC = 998.; // checking for nan
           if (QDC>0) QDC = -QDC;
           mask = true;
         }
         else if (satur > saturationLimit || QDC>1E20 || QDC != QDC)
         {
           if (QDC>1E20) QDC = 987.; // checking for inf
           LOG (info) << "inf " << board_id << " " << bt->GetLeaf("tofpetId")->GetValue(n)    
                      << " " << bt->GetLeaf("tofpetChannel")->GetValue(n)
                      << " " << bt->GetLeaf("tac")->GetValue(n) 
                      << " " << bt->GetLeaf("vCoarse")->GetValue(n)
                      << " " << bt->GetLeaf("vFine")->GetValue(n)
                      << " " << TDC-bt->GetLeaf("tCoarse")->GetValue(n) 
                      << " " << eventNumber << " " << Chi2ndof;
           if (QDC != QDC) QDC = 988.; // checking for nan
           LOG (info) << "nan " << board_id << " " << bt->GetLeaf("tofpetId")->GetValue(n)
                       << " " << bt->GetLeaf("tofpetChannel")->GetValue(n)
                       << " " << bt->GetLeaf("tac")->GetValue(n)
                       << " " << bt->GetLeaf("vCoarse")->GetValue(n)
                       << " " << bt->GetLeaf("vFine")->GetValue(n)
                       << " " << TDC-bt->GetLeaf("tCoarse")->GetValue(n)
                       << " " << TDC << " " << bt->GetLeaf("tCoarse")->GetValue(n)
                       << " " << eventNumber << " " << Chi2ndof;
           A = int(min(QDC,double(1000.)));
           B = min(satur,double(999.))/1000.;
           QDC = A+B;
           mask = true;
         }
         else if ( Chi2ndof > chi2Max )
         {
            if (QDC>0) QDC = -QDC;
            mask = true;
         }         
         LOG (info) << "calibrated: tdc = " << TDC << ", qdc = " << QDC;//TDC clock cycle = 160 MHz or 6.25ns
         t4 = high_resolution_clock::now();
         // Set the unit of the execution time measurement to ns
         counters["qdc"]+= duration_cast<nanoseconds>(t1 - t0).count();
         counters["make"]+= duration_cast<nanoseconds>(t4-t0).count();
         
         // MuFilter encoding
         if (!scifi)
         {
           system = MufiSystem[board_id][tofpet_id];
           key = (tofpet_id%2)*1000 + tofpet_channel;
           tmp = boardMapsMu["MuFilter"][board.first][slots[tofpet_id]];
           if (debug || !tmp.find("not") == string::npos )
           {
             LOG (info) << system << " " << key << " " << board.first << " " << tofpet_id
                         << " " << tofpet_id%2 << " " << tofpet_channel;
           }
           sipmChannel = 99;
           if ( TofpetMap[system].count(key) == 0)
           {
                        cout << "key " << key << " does not exist. " << endl
                             << board.first << " Tofpet id " << tofpet_id
                             << " System " << system << " has Tofpet map elements: {";
                        for ( auto it : TofpetMap[system])
                        {
                          cout << it.first << " : " << it.second << ", ";
                        }
                        cout << "}\n";
           }
           else sipmChannel = TofpetMap[system][key]-1;
           
           nSiPMs = abs(offMap[tmp][1]);
           nSides = abs(offMap[tmp][2]);
           direction = int(offMap[tmp][1]/nSiPMs);
           detID = offMap[tmp][0] + direction*int(sipmChannel/nSiPMs);
           sipm_number = sipmChannel%(nSiPMs);
           if ( tmp.find("Right") != string::npos ) sipm_number+= nSiPMs;
           if (digiMuFilterStore.count(detID)==0) 
           {
 	     digiMuFilterStore[detID] = new MuFilterHit(detID,nSiPMs,nSides);
           }
           test = digiMuFilterStore[detID]->GetSignal(sipm_number);
           digiMuFilterStore[detID]->SetDigi(QDC,TDC,sipm_number);
           digiMuFilterStore[detID]->SetDaqID(sipm_number,n, board_id, tofpet_id, tofpet_channel);
           if (mask) digiMuFilterStore[detID]->SetMasked(sipm_number);
           
           LOG (info) << "create mu hit: " << detID << " " << tmp << " " << system
                       << " " << tofpet_id << " " << offMap[tmp][0] << " " << offMap[tmp][1]
                       << " " << offMap[tmp][2] << " " << sipmChannel << " " << nSiPMs
                       << " " << nSides << " " << test << endl
                       << detID << " " << sipm_number << " " << QDC << " " << TDC;
                       
           if (test>0 || detID%1000>200 || sipm_number>15)
           {
             cout << "what goes wrong? " << detID << " SiPM " << sipm_number << " system " << system
                  << " key " << key << " board " << board.first << " tofperID " << tofpet_id
                  << " tofperChannel " << tofpet_channel << " test " << test << endl;
           }
           t5 = high_resolution_clock::now();
           counters["createMufi"]+= duration_cast<nanoseconds>(t5 - t4).count();
         } // end MuFilter encoding
         
         else // now Scifi encoding
         {
           chan = channel_func(tofpet_id, tofpet_channel, mat);
           orientation = 1;
           if (station[2]=='Y') orientation = 0;
           sipmLocal = (chan - mat*512);
           sipmID = 1000000*int(station[1]-'0') + 100000*orientation + 10000*mat
                                            + 1000*(int(sipmLocal/128)) + chan%128;
           if (digiSciFiStore.count(sipmID)==0)
           {
             digiSciFiStore[sipmID] =  new sndScifiHit(sipmID);             
           }
           digiSciFiStore[sipmID]->SetDigi(QDC,TDC);
           digiSciFiStore[sipmID]->SetDaqID(0,n, board_id, tofpet_id, tofpet_channel);
           if (mask) digiSciFiStore[sipmID]->setInvalid();
           LOG (info) << "create scifi hit: tdc = " << board.first << " " << sipmID
                       << " " << QDC << " " << TDC <<endl
                       << "tofpet:" << " " << tofpet_id << " " << tofpet_channel << " " << mat
                       << " " << chan << endl
                       << station[1] << " " << station[2] << " " << mat << " " << chan
                       << " " << int(chan/128)%4 << " " << chan%128;
           t5 = high_resolution_clock::now();
           counters["createScifi"]+= duration_cast<nanoseconds>(t5 - t4).count();
         } // end Scifi encoding
       } // end loop over hits in event
  } // end loop over boards (TTrees in data file)

  counters["N"]+= 1;
  t6 = high_resolution_clock::now();
  for (auto it_sipmID : digiSciFiStore)
  {
    (*fDigiSciFi)[indexSciFi]=digiSciFiStore[it_sipmID.first];
    indexSciFi+= 1;
  }
  for (auto it_detID : digiMuFilterStore)
  {
    (*fDigiMuFilter)[indexMuFilter]=digiMuFilterStore[it_detID.first];
    indexMuFilter+= 1;
  }
  counters["storage"]+= duration_cast<nanoseconds>(high_resolution_clock::now() - t6).count();
  counters["event"]+= duration_cast<nanoseconds>(high_resolution_clock::now() - tE).count();
  //timer.Stop();
  //cout<<timer.RealTime()<<endl;
    
  LOG (info) << fnStart+1 << " events processed out of "
             << fEventTree->GetEntries() << " number of events in file.";
  /*
  cout << "Monitor computing time:" << endl;
  for (auto it: counters)
  {
     if( it.first=="N" )
     {
       cout << "Processed " << it.first<< " = " << it.second << " events." << endl;
     }
     else
     {
       // Print execution time in seconds
       cout << "stage " << it.first<< " took " << it.second*1e-9 << " [s]" << endl;
     }
  }
  */ 
}

void ConvRawData::Process1()
{    
  int indexSciFi{}, indexMuFilter{};
  bool scifi = false, mask = false;
  int board_id{};
  string board_name;
  int tofpet_id{}, tofpet_channel{}, tac{}, mat{};
  string station;
  double TDC{}, QDC{}, Chi2ndof{}, satur{};
  int A{};
  double B{};
  high_resolution_clock::time_point tE{}, t0{}, t1{}, t4{}, t5{}, t6{}, tt{};
  int system{}, key{}, sipmChannel{};
  string tmp;
  int nSiPMs{}, nSides{}, direction{}, detID{}, sipm_number{}, chan{}, orientation{}, sipmLocal{};
  int sipmID{};
  double test{};
  int64_t eventTime{};
  //TStopwatch timer;
  
  tE = high_resolution_clock::now();
  //timer.Start();
  fEventTree->GetEvent(eventNumber);
  if ( eventNumber%fheartBeat == 0 )
  {
     tt = high_resolution_clock::now();
     time_t ttp = high_resolution_clock::to_time_t(tt);
     LOG (info) << "run " << frunNumber << " event " << eventNumber
                << " local time " << ctime(&ttp);
  }
  
  fSNDLHCEventHeader->SetFlags(fEventTree->GetLeaf("evtFlags")->GetValue());
  fSNDLHCEventHeader->SetRunId(frunNumber);
  eventTime = fEventTree->GetLeaf("evtTimestamp")->GetValue();
  fSNDLHCEventHeader->SetEventTime(eventTime);
  fSNDLHCEventHeader->SetUTCtimestamp(eventTime*6.23768*1e-9 + runStartUTC);
  fSNDLHCEventHeader->SetEventNumber(fEventTree->GetLeaf("evtNumber")->GetValue());
  // Fill filling scheme data into the event header
  if (FSmap->GetEntries()>1){
      // ion runs
      if (fSNDLHCEventHeader->GetAccMode()== 12){
         fSNDLHCEventHeader->SetBunchType(stoi(((TObjString*)FSmap->GetValue(Form("%d",
                                             int((eventTime%(4*3564))/8+0.5))))->GetString().Data()));
      }
      // proton runs
      else{
         fSNDLHCEventHeader->SetBunchType(stoi(((TObjString*)FSmap->GetValue(Form("%d",
                                             int((eventTime%(4*3564))/4))))->GetString().Data()));
      }
  }    
  else
      fSNDLHCEventHeader->SetBunchType(stoi(((TObjString*)FSmap->GetValue("0"))->GetString().Data())); 
  
  LOG (info) << "evtNumber per run "
             << fEventTree->GetLeaf("evtNumber")->GetValue()
             << " evtNumber per partition: " << eventNumber
             << " timestamp: " << eventTime;
  // Delete pointer map elements
  for (auto it : digiSciFiStore)
  {
      delete it.second;
  }
  digiSciFiStore.clear();
  for (auto it : digiMuFilterStore)
  {
      delete it.second;
  }
  digiMuFilterStore.clear();
     
  // Loop over hits per event!
  for ( int n =0; n < fEventTree->GetLeaf("nHits")->GetValue(); n++ )
  { 
       board_id = fEventTree->GetLeaf("boardId")->GetValue(n);
       board_name = "board_"+to_string(board_id);
       scifi = true;
       if (boardMaps["Scifi"].count(board_name)!=0) 
       {
         for (auto it : boardMaps["Scifi"][board_name])
         {
           station = it.first;
           mat = it.second;
         }         
       }
       else if (boardMaps["MuFilter"].count(board_name)!=0) scifi = false;
       else
       {
         LOG (error) << board_name << " not known. Serious error, stop!";
         break;
       }
       mask = false;
       LOG (info) << "In scifi? " << scifi 
                  << " " << board_id << " " << fEventTree->GetLeaf("tofpetId")->GetValue(n)
                  << " " << fEventTree->GetLeaf("tofpetChannel")->GetValue(n)
                  << " " << fEventTree->GetLeaf("tac")->GetValue(n)
                  << " " << fEventTree->GetLeaf("tCoarse")->GetValue(n)
                  << " " << fEventTree->GetLeaf("tFine")->GetValue(n)
                  << " " << fEventTree->GetLeaf("vCoarse")->GetValue(n)
                  << " " << fEventTree->GetLeaf("vFine")->GetValue(n);
       t0 = high_resolution_clock::now();
       tofpet_id = fEventTree->GetLeaf("tofpetId")->GetValue(n);
       tofpet_channel = fEventTree->GetLeaf("tofpetChannel")->GetValue(n);
       tac = fEventTree->GetLeaf("tac")->GetValue(n);
       /* Since run 39 calibration is done online and 
       calib data stored in root raw data file */
       if (makeCalibration)
         tie(TDC,QDC,Chi2ndof,satur) = comb_calibration(board_id, tofpet_id, tofpet_channel, tac,
                                                   fEventTree->GetLeaf("vCoarse")->GetValue(n),
                                                   fEventTree->GetLeaf("vFine")->GetValue(n),
                                                   fEventTree->GetLeaf("tCoarse")->GetValue(n),
                                                   fEventTree->GetLeaf("tFine")->GetValue(n),
                                                   1.0, 0);
       else
       {
         TDC = fEventTree->GetLeaf("timestamp")->GetValue(n);
         QDC = fEventTree->GetLeaf("value")->GetValue(n);
         Chi2ndof = max(fEventTree->GetLeaf("timestampCalChi2")->GetValue(n)/fEventTree->GetLeaf("timestampCalDof")->GetValue(n),
                        fEventTree->GetLeaf("valueCalChi2")->GetValue(n)/fEventTree->GetLeaf("valueCalDof")->GetValue(n));
         satur = fEventTree->GetLeaf("valueSaturation")->GetValue(n);
       }
  
       // Print a warning if TDC or QDC is nan.        
       if ( TDC != TDC || QDC!=QDC) {
       LOG (error) << "NAN tdc/qdc detected! Check maps!"
                      << " " << board_id << " " << fEventTree->GetLeaf("tofpetId")->GetValue(n)
                     << " " << fEventTree->GetLeaf("tofpetChannel")->GetValue(n)
                     << " " << fEventTree->GetLeaf("tac")->GetValue(n)
                     << " " << fEventTree->GetLeaf("tCoarse")->GetValue(n)
                     << " " << fEventTree->GetLeaf("tFine")->GetValue(n)
                     << " " << fEventTree->GetLeaf("vCoarse")->GetValue(n)
                     << " " << fEventTree->GetLeaf("vFine")->GetValue(n);
       }
         
       t1 = high_resolution_clock::now();
       if ( Chi2ndof > chi2Max )
       {
         if (QDC>1E20) QDC = 997.; // checking for inf
         if (QDC != QDC) QDC = 998.; // checking for nan
         if (QDC>0) QDC = -QDC;
         mask = true;
       }
       else if (satur > saturationLimit || QDC>1E20 || QDC != QDC)
       {
         if (QDC>1E20) QDC = 987.; // checking for inf
         LOG (info) << "inf " << board_id << " " << fEventTree->GetLeaf("tofpetId")->GetValue(n)    
                    << " " << fEventTree->GetLeaf("tofpetChannel")->GetValue(n)
                    << " " << fEventTree->GetLeaf("tac")->GetValue(n) 
                    << " " << fEventTree->GetLeaf("vCoarse")->GetValue(n)
                    << " " << fEventTree->GetLeaf("vFine")->GetValue(n)
                    << " " << TDC-fEventTree->GetLeaf("tCoarse")->GetValue(n) 
                    << " " << eventNumber << " " << Chi2ndof;
         if (QDC != QDC) QDC = 988.; // checking for nan
         LOG (info) << "nan " << board_id << " " << fEventTree->GetLeaf("tofpetId")->GetValue(n)
                     << " " << fEventTree->GetLeaf("tofpetChannel")->GetValue(n)
                     << " " << fEventTree->GetLeaf("tac")->GetValue(n)
                     << " " << fEventTree->GetLeaf("vCoarse")->GetValue(n)
                     << " " << fEventTree->GetLeaf("vFine")->GetValue(n)
                     << " " << TDC-fEventTree->GetLeaf("tCoarse")->GetValue(n)
                     << " " << TDC << " " << fEventTree->GetLeaf("tCoarse")->GetValue(n)
                     << " " << eventNumber << " " << Chi2ndof;
         A = int(min(QDC,double(1000.)));
         B = min(satur,double(999.))/1000.;
         QDC = A+B;
         mask = true;
       }
       else if ( Chi2ndof > chi2Max )
       {
          if (QDC>0) QDC = -QDC;
          mask = true;
       }         
         LOG (info) << "calibrated: tdc = " << TDC << ", qdc = " << QDC;//TDC clock cycle = 160 MHz or 6.25ns
       t4 = high_resolution_clock::now();
       // Set the unit of the execution time measurement to ns
       counters["qdc"]+= duration_cast<nanoseconds>(t1 - t0).count();
       counters["make"]+= duration_cast<nanoseconds>(t4-t0).count();
       
       // MuFilter encoding
       if (!scifi)
       {
           system = MufiSystem[board_id][tofpet_id];
           key = (tofpet_id%2)*1000 + tofpet_channel;
           tmp = boardMapsMu["MuFilter"][board_name][slots[tofpet_id]];
           if (debug || !tmp.find("not") == string::npos )
           {
             LOG (info) << system << " " << key << " " << board_name << " " << tofpet_id
                         << " " << tofpet_id%2 << " " << tofpet_channel;
           }
           sipmChannel = 99;
           if ( TofpetMap[system].count(key) == 0)
           {
                        cout << "key " << key << " does not exist. " << endl
                             << board_name << " Tofpet id " << tofpet_id
                             << " System " << system << " has Tofpet map elements: {";
                        for ( auto it : TofpetMap[system])
                        {
                          cout << it.first << " : " << it.second << ", ";
                        }
                        cout << "}\n";
           }
           else sipmChannel = TofpetMap[system][key]-1;
           
           nSiPMs = abs(offMap[tmp][1]);
           nSides = abs(offMap[tmp][2]);
           direction = int(offMap[tmp][1]/nSiPMs);
           detID = offMap[tmp][0] + direction*int(sipmChannel/nSiPMs);
           sipm_number = sipmChannel%(nSiPMs);
           if ( tmp.find("Right") != string::npos ) sipm_number+= nSiPMs;
           if (digiMuFilterStore.count(detID)==0) 
           {
 	     digiMuFilterStore[detID] = new MuFilterHit(detID,nSiPMs,nSides);
           }
           test = digiMuFilterStore[detID]->GetSignal(sipm_number);
           digiMuFilterStore[detID]->SetDigi(QDC,TDC,sipm_number);
           digiMuFilterStore[detID]->SetDaqID(sipm_number,n, board_id, tofpet_id, tofpet_channel);
           if (mask) digiMuFilterStore[detID]->SetMasked(sipm_number);
           
           LOG (info) << "create mu hit: " << detID << " " << tmp << " " << system
                       << " " << tofpet_id << " " << offMap[tmp][0] << " " << offMap[tmp][1]
                       << " " << offMap[tmp][2] << " " << sipmChannel << " " << nSiPMs
                       << " " << nSides << " " << test << endl
                       << detID << " " << sipm_number << " " << QDC << " " << TDC;
                       
           if (test>0 || detID%1000>200 || sipm_number>15)
           {
             cout << "what goes wrong? " << detID << " SiPM " << sipm_number << " system " << system
                  << " key " << key << " board " << board_name << " tofperID " << tofpet_id
                  << " tofperChannel " << tofpet_channel << " test " << test << endl;
           }
           t5 = high_resolution_clock::now();
           counters["createMufi"]+= duration_cast<nanoseconds>(t5 - t4).count();
       } // end MuFilter encoding
       
       else // now Scifi encoding
       {
           chan = channel_func(tofpet_id, tofpet_channel, mat);
           orientation = 1;
           if (station[2]=='Y') orientation = 0;
           sipmLocal = (chan - mat*512);
           sipmID = 1000000*int(station[1]-'0') + 100000*orientation + 10000*mat
                                            + 1000*(int(sipmLocal/128)) + chan%128;
           if (digiSciFiStore.count(sipmID)==0)
           {
             digiSciFiStore[sipmID] =  new sndScifiHit(sipmID);             
           }
           digiSciFiStore[sipmID]->SetDigi(QDC,TDC);
           digiSciFiStore[sipmID]->SetDaqID(0,n,board_id, tofpet_id, tofpet_channel);
           if (mask) digiSciFiStore[sipmID]->setInvalid();
           LOG (info) << "create scifi hit: tdc = " << board_name << " " << sipmID
                       << " " << QDC << " " << TDC <<endl
                       << "tofpet:" << " " << tofpet_id << " " << tofpet_channel << " " << mat
                       << " " << chan << endl
                       << station[1] << " " << station[2] << " " << mat << " " << chan
                       << " " << int(chan/128)%4 << " " << chan%128;
           t5 = high_resolution_clock::now();
           counters["createScifi"]+= duration_cast<nanoseconds>(t5 - t4).count();
       } // end Scifi encoding
  } // end loop over hits in event

  counters["N"]+= 1;
  t6 = high_resolution_clock::now();
  for (auto it_sipmID : digiSciFiStore)
  {
    (*fDigiSciFi)[indexSciFi]=digiSciFiStore[it_sipmID.first];
    indexSciFi+= 1;
  }
  for (auto it_detID : digiMuFilterStore)
  {
    (*fDigiMuFilter)[indexMuFilter]=digiMuFilterStore[it_detID.first];
    indexMuFilter+= 1;
  }
  counters["storage"]+= duration_cast<nanoseconds>(high_resolution_clock::now() - t6).count();
  counters["event"]+= duration_cast<nanoseconds>(high_resolution_clock::now() - tE).count();
  //timer.Stop();
  //cout<<timer.RealTime()<<endl;
    
  LOG (info) << fnStart+1 << " events processed out of "
             << fEventTree->GetEntries() << " number of events in file.";
  /*
  cout << "Monitor computing time:" << endl;
  for (auto it: counters)
  {
     if( it.first=="N" )
     {
       cout << "Processed " << it.first<< " = " << it.second << " events." << endl;
     }
     else
     {
       // Print execution time in seconds
       cout << "stage " << it.first<< " took " << it.second*1e-9 << " [s]" << endl;
     }
  }
  */
}
  
/* https://gitlab.cern.ch/snd-scifi/software/-/wikis/Raw-data-format 
      tac: 0-3, identifies the Time-To-Analogue converter used for this hit (each channel has four of them and they require separate calibration).
      t_coarse: Coarse timestamp of the hit, running on a 4 times the LHC clock
      t_fine: 0-1023, fine timestamp of the hit. It contains the raw value of the charge digitized by the TDC and requires calibration.
      v_coarse: 0-1023, QDC mode: it represents the number of clock cycles the charge integration lasted.
      v_fine = 0-1023, QDC mode: represents the charge measured. Requires calibration.
*/
/** Start time of run **/
void ConvRawData::StartTimeofRun(string Path)
{
  nlohmann::json j;
  // reading file with xrootd protocol
  File file;
  XRootDStatus status;
  StatInfo *info;
  uint64_t offset = 0;
  uint32_t size;
  uint32_t bytesRead = 0;
  if (local)
  {
    ifstream jsonfile(Form("%s/run_timestamps.json", Path.c_str()));
    jsonfile >> j;
  }
  else
  {
    status = file.Open(Form("%s/run_timestamps.json", Path.c_str()), OpenFlags::Read);
    if( !status.IsOK() )
    {
      LOG (error) << "Error opening file";
      exit(0);
    }
    file.Stat(false, info);
    size = info->GetSize();
    char *buff = new char[size];
    status = file.Read(offset, size, buff, bytesRead);
    vector<char> vec;
    for (int i = 0; i < size; i++){vec.push_back(buff[i]);}
    j = json::parse(vec);
    status = file.Close();
    delete info;
    delete [] buff;
    }
  // Read the start-time string
  struct tm tm;
  for (auto& el : j.items())
  {
     if (el.key() == "start_time")
     {
       string timeStr = el.value().get<string>();
       strptime(timeStr.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm);
     }
  }
  runStartUTC = timegm(&tm);
}
/** Board mapping for Scifi and MuFilter **/
void ConvRawData::DetMapping(string Path)
{
  nlohmann::json j;
  // reading file with xrootd protocol
  File file;
  XRootDStatus status;
  StatInfo *info;
  uint64_t offset = 0;
  uint32_t size;
  uint32_t bytesRead = 0;
  // Call boardMappingParser
  if (local)
  {
    ifstream jsonfile(Form("%s/board_mapping.json", Path.c_str()));      
    jsonfile >> j;
  }
  else
  {
    status = file.Open(Form("%s/board_mapping.json", Path.c_str()), OpenFlags::Read);
    if( !status.IsOK() )
    {
      LOG (error) << "Error opening file";
      exit(0);
    }
    file.Stat(false, info);
    size = info->GetSize();
    char *buff = new char[size];
    status = file.Read(offset, size, buff, bytesRead);
    vector<char> vec;
    for (int i = 0; i < size; i++){vec.push_back(buff[i]);}
    j = json::parse(vec);
    status = file.Close();
    delete info;
    delete [] buff;    
  }
  // Pass the read string to getBoardMapping()
  tie(boardMaps, boardMapsMu) = getBoardMapping(j);
  
  int board_id_mu{}, s{};
  string tmp;
  for ( auto it : boardMapsMu["MuFilter"] )
  {
     board_id_mu = stoi(it.first.substr(it.first.find("_") + 1));
     for ( auto x : boardMapsMu["MuFilter"][it.first] )
     {
       for ( auto slot : slots )
       {
         if ( Path.find("testbeam_23") == string::npos ) s = 0;
         else s = 3;
         tmp = x.second.substr(0, x.second.find("_"));
         if ( tmp =="US" ) s = 1;
         if ( tmp=="DS" )
         {
           s = 2;
           if ( Path.find("testbeam_24") != string::npos && x.second.substr(x.second.find("_")+1,1)==2) s = 3;
         }
         if ( slots[slot.first] == x.first )
         {
            MufiSystem[board_id_mu][slot.first] = s;
            boardMaps["MuFilter"][it.first][slot.second] = slot.first;
         }
       }
     }
  }
  
  //string o;
  for (int i = 1 ; i < 6; i++ )
  {
    if (i<3)
    {
      offMap[Form("Veto_%iLeft",i)]  = {10000 + (i-1)*1000+ 0, 8, 2};//first channel, nSiPMs, nSides
      offMap[Form("Veto_%iRight",i)] = {10000 + (i-1)*1000+ 0, 8, 2};
    }
    if (i==3) offMap[Form("Veto_%iVert",i)] = {10000 + (i-1)*1000+ 6, -8, 1};// 3rd Veto plane
    if (i<4)
    {
      // DS
      offMap[Form("DS_%iLeft",i)]  = {30000 + (i-1)*1000+ 59, -1, 2};// direction not known
      offMap[Form("DS_%iRight",i)] = {30000 + (i-1)*1000+ 59, -1, 2};// direction not known
    }
    if (i<5)
    {
      offMap[Form("DS_%iVert",i)] = {30000 + (i-1)*1000+ 119, -1, 1};// direction not known
    }
    offMap[Form("US_%iLeft",i)]  = {20000 + (i-1)*1000+ 9, -8, 2}; // from top to bottom
    offMap[Form("US_%iRight",i)] = {20000 + (i-1)*1000+ 9, -8, 2};
  }  
}
/* Define calibration functions **/
double ConvRawData::qdc_calibration( int board_id, int tofpet_id, int channel,
                       int tac, uint16_t v_coarse, uint16_t v_fine, uint16_t tf )
  {
    double GQDC = 1.0; // or 3.6
    map<string, double> par= X_qdc[{board_id,tofpet_id,channel,tac}];
    double x = v_coarse - tf;
    double fqdc = -par["c"]*log(1+exp( par["a"]*pow((x-par["e"]),2)
                 -par["b"]*(x-par["e"]) )) + par["d"];
    double value = (v_fine-fqdc)/GQDC;
    return value;
}
double ConvRawData::qdc_chi2( int board_id, int tofpet_id, int channel, int tac, int TDC=0 )
{
    map<string, double> par = X_qdc[{board_id,tofpet_id,channel,tac}];
    map<string, double> parT = X_tdc[{board_id,tofpet_id,channel,tac, TDC}];
    return max(par["chi2Ndof"],parT["chi2Ndof"]);
}
double ConvRawData::qdc_sat( int board_id, int tofpet_id, int channel, int tac, uint16_t v_fine )
{
    map<string, double> par = X_qdc[{board_id,tofpet_id,channel,tac}];
    return v_fine/par["d"];  
}
double ConvRawData::time_calibration( int board_id, int tofpet_id, int channel,
                        int tac, int64_t t_coarse, uint16_t t_fine, int TDC=0 )
{
    map<string, double> parT = X_tdc[{board_id,tofpet_id,channel,tac, TDC}];
    double x = t_fine;
    double ftdc = (-parT["b"]-sqrt(pow(parT["b"],2)
                  -4*parT["a"]*(parT["c"]-x)))/(2*parT["a"]);
    double timestamp = t_coarse+ftdc;
    return timestamp;
}
tuple<double, double, double, double> ConvRawData::comb_calibration( int board_id, int tofpet_id, int channel,int tac, uint16_t v_coarse, uint16_t v_fine,int64_t t_coarse, uint16_t t_fine, double GQDC = 1.0, int TDC=0 )// max gain QDC = 3.6
{
    map<string, double> par= X_qdc[{board_id,tofpet_id,channel,tac}];
    map<string, double> parT = X_tdc[{board_id,tofpet_id,channel,tac,TDC}];
    double x    = t_fine;
    double ftdc = (-parT["b"]-sqrt(pow(parT["b"],2)
                  -4*parT["a"]*(parT["c"]-x)))/(2*parT["a"]); // Ettore 28/01/2022 +par['d']
    double timestamp = t_coarse + ftdc;
    double tf = timestamp - t_coarse;
    x = v_coarse - tf;
    double fqdc = - par["c"]*log(1+exp( par["a"]*pow((x-par["e"]),2)-par["b"]*(x-par["e"]) ))
                 + par["d"];
    double value = (v_fine-fqdc)/GQDC;
    return make_tuple(timestamp,value,max(par["chi2Ndof"],parT["chi2Ndof"]),v_fine/par["d"]);
}
map<double, pair<double, double> > ConvRawData::calibrationReport()
{
    TH1F* h = new TH1F("chi2","chi2", 1000, 0., 10000);
    map<double, pair<double, double> > report{};
    int TDC = 0;
    double chi2{}, chi2T{}, key{};
    // loop over entries of X_qdc and get map keys
    for (auto it : X_qdc)
    {
      map<string, double> par= X_qdc[{it.first[0], it.first[1], it.first[2], it.first[3]}];
      if (par["chi2Ndof"]) chi2 = par["chi2Ndof"];
      else chi2 = -1;
      map<string, double> parT = X_tdc[{it.first[0], it.first[1], it.first[2], it.first[3], TDC}];
      if (parT["chi2Ndof"]) chi2T = parT["chi2Ndof"];
      else chi2T = -1;
      key = it.first[3] +10*it.first[2] + it.first[1]*10*100 + it.first[0]*10*100*100;
      if (report.count(key)==0) report[key] = make_pair(chi2,chi2T);
    }
    for (auto it : report)
    {
         h->Fill(report[it.first].first);
         h->Fill(report[it.first].second);
    }
    h->Draw();
    return report;
}
/** Define some other functions **/
int ConvRawData::channel_func( int tofpet_id, int tofpet_channel, int position)
{
    return (64*tofpet_id + 63 - tofpet_channel + 512*position);// 512 channels per mat, 1536 channels per plane. One channel covers all 6 layers of fibres.
}
/** Read csv data files **/
void ConvRawData::read_csv(string Path)
{
  ifstream infile; // ifstream is the stream class to only read! from files.
  stringstream X;
  // reading file with xrootd protocol
  File file;
  XRootDStatus status;
  StatInfo *info;
  uint64_t offset = 0;
  uint32_t size;
  uint32_t bytesRead = 0;
  string line, element;
  double chi2_Ndof{};
  // counter of number of elements
  vector<int> key_vector{};
  
  // Always read the SiPM mapping
  int SiPM{};
  vector<int> data_vector{};
  map<string, map<int, vector<int>> > SiPMmap{};
  vector<int> row{};
  map<string, int> key { {"BM",3}, {"DS",2}, {"US",1}, {"Veto",0} };
  struct stat buffer;
  TString sndRoot = gSystem->Getenv("SNDSW_ROOT");
  string sndswPath = sndRoot.Data();
  string path_SiPMmap = Form("%s/geometry", sndswPath.c_str());
  if (stat(path_SiPMmap.c_str(), &buffer) != 0)
  {
    LOG (error) << "Path "<< path_SiPMmap.c_str() << " does not exist!";
    exit (0); 
  }
  for (auto sys : key)
  {
    infile.open(Form("%s/%s_SiPM_mapping.csv", path_SiPMmap.c_str(), sys.first.c_str()));
    X << infile.rdbuf();
    infile.close();
          
    // Skip 1st line
    getline(X,line);
    LOG (info) << "In " << sys.first << " SiPM map file: " << line;
    // Read all other lines
    while (getline(X,line))
    {
        data_vector.clear();
        stringstream items(line);
        // first element in line is SiPM
        getline(items, element, ',');
        SiPM = stoi(element);
        // only taking first few elements of line
        while (data_vector.size()<4 && getline(items, element, ',')) 
        {
           data_vector.push_back(stoi(element));
        }
        SiPMmap[sys.first][SiPM] = data_vector;
        if (X.peek() == EOF) break;
    }
    X.str(string()); X.clear(); line.clear();
    size = 0; offset = 0; bytesRead = 0;
    for (auto channel : SiPMmap[sys.first])
    {
      row = channel.second;
      TofpetMap[sys.second][row.at(2)*1000+row.at(3)] = channel.first;
    }
  } // end filling SiPMmap and TofpetMap

  // Get QDC/TDC calibration data if flag is set
  if (!makeCalibration) return;

  // Get QDC calibration data
  if (local)
  {
   infile.open(Form("%s/qdc_cal.csv", Path.c_str()));
   X << infile.rdbuf();
   infile.close();
  }
  else
  {
    status = file.Open(Form("%s/qdc_cal.csv", Path.c_str()), OpenFlags::Read);
    if( !status.IsOK() )
    {
      LOG (error) << "Error opening file";
      exit(0);
    }
    file.Stat(false, info);
    size = info->GetSize();
    char *buff = new char[size];
    status = file.Read(offset, size, buff, bytesRead);
    X << buff;
    status = file.Close();
    delete info;
    delete [] buff;  
  }
  LOG (info) << "Read_csv "<<Path.c_str();
  // Skip 1st line
  getline(X, line);
  LOG (info) << "In QDC cal file: " << line;
  vector<double> qdcData{};
  // Read all other lines
  while (getline(X, line))
  {    
    qdcData.clear();
    stringstream items(line);
    while (getline(items, element, ','))
    {
      if(iscntrl(element[0])) break;
      qdcData.push_back(stof(element));
    }
    if (qdcData.size()<10) continue;
    key_vector.clear();
    key_vector = {int(qdcData[0]), int(qdcData[1]), int(qdcData[2]), int(qdcData[3])};
    chi2_Ndof = (qdcData[9] < 2) ? 999999. : qdcData[7]/qdcData[9]; 
    X_qdc[key_vector] = { {"a",qdcData[4]}, {"b",qdcData[5]}, {"c",qdcData[6]},
                          {"d",qdcData[8]}, {"e",qdcData[10]}, {"chi2Ndof",chi2_Ndof} };
    if (X.peek() == EOF) break;
  }
  X.str(string()); X.clear(); line.clear();
  size = 0; offset = 0; bytesRead = 0;
  // Get TDC calibration data
  if (local)
  {
   infile.open(Form("%s/tdc_cal.csv", Path.c_str()));
   X << infile.rdbuf();
   infile.close();
  }
  else
  {
    status = file.Open(Form("%s/tdc_cal.csv", Path.c_str()), OpenFlags::Read);
    if( !status.IsOK() )
    {
      LOG (error) << "Error opening file";
      exit(0);
    }
    file.Stat(false, info);
    size = info->GetSize();
    char *buff = new char[size];
    status = file.Read(offset, size, buff, bytesRead);
    X << buff;
    status = file.Close();
    delete info;
    delete [] buff;
  }
  // Skip 1st line
  getline(X, line);
  LOG (info) << "In TDC cal file: " << line;
  vector<double> tdcData{};
  // Read all other lines
  while (getline(X, line))
  {
    tdcData.clear();
    stringstream items(line);
    if (line.length()<5){continue;}
    while (getline(items, element, ','))
    {
      if(iscntrl(element[0])) break;
      tdcData.push_back(stof(element));
    }
    if (tdcData.size()<9) continue;
    key_vector.clear();
    key_vector = {int(tdcData[0]), int(tdcData[1]), int(tdcData[2]), int(tdcData[3]), int(tdcData[4])};
    chi2_Ndof = (tdcData[10] < 2) ? 999999. : tdcData[8]/tdcData[10]; 
    X_tdc[key_vector] = { {"a",tdcData[5]}, {"b",tdcData[6]}, {"c",tdcData[7]},
                          {"d",tdcData[9]}, {"chi2Ndof",chi2_Ndof} };
    if (X.peek() == EOF) break;
  }
  X.str(string()); X.clear(); line.clear();
  size = 0; offset = 0; bytesRead = 0;
}
void ConvRawData::debugMapping(string brd, int tofpetID, int tofpetChannel)
{
  int Key{}, SiPMChannel{}, n_SiPMs{}, n_Sides{}, Direction{}, DetID{}, SiPM_number{};
  string Tmp;
  int brdID{}, System{};
  Key = (tofpetID%2)*1000 + tofpetChannel;
  Tmp = boardMapsMu["MuFilter"][brd][slots[tofpetID]];
  brdID = stoi(brd.substr(brd.find("_")+1));
  System = MufiSystem[brdID][tofpetID];
  SiPMChannel = TofpetMap[System][Key]-1;
  n_SiPMs = abs(offMap[Tmp][1]);
  n_Sides = abs(offMap[Tmp][2]);
  Direction = int(offMap[Tmp][1]/n_SiPMs);
  DetID = offMap[Tmp][0] + Direction*int(SiPMChannel/n_SiPMs);
  SiPM_number = SiPMChannel%(n_SiPMs);
  cout << "SiPM channel " << SiPMChannel << " nSiPM " << n_SiPMs << " nSides " << n_Sides
       << " detID " << DetID << " SiPM number " << SiPM_number << endl;
}

ClassImp(ConvRawData);
