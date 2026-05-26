#ifndef SHIPLHC_DRIFTTUBEHIT_H_
#define SHIPLHC_DRIFTTUBEHIT_H_

#include "DriftTubePoint.h"
#include "TObject.h"
#include "TVector3.h"

inline const double VDRIFT {0.0055 * 1e9};
inline const double TPED {190.0 * 1e-9}; 

class DriftTubeHit : public TObject {
public:
   /** Default constructor **/
   DriftTubeHit();
   /** Copy constructor **/
   DriftTubeHit(const DriftTubeHit &hit) = default;
   DriftTubeHit &operator=(const DriftTubeHit &hit) = default;
   explicit DriftTubeHit(Int_t detID);
   //  Constructor from MiniDT raw data
   explicit DriftTubeHit(Int_t detID, const Double_t& m_timestamp);
   //  Constructor from DriftTubePoint
   DriftTubeHit(int detID, std::vector<DriftTubePoint *>, std::vector<Float_t>);

   /** Destructor **/
   virtual ~DriftTubeHit();

   /** Output to screen **/
   void Print();

   void setInvalid() { flag = false; }
   bool isValid() const { return flag; }
   Double_t GetTimestamp() {return timestamp; }
   Int_t GetSystem() { return floor(fDetectorID / 10000); }
   Int_t GetStation() { return 1; } // do we need such a method? FIXME
   Int_t GetPlane() { return int(fDetectorID / 1000) % 10; }
   Int_t GetLayer() { return int(fDetectorID % 1000) / 100; }
   Int_t GetCell() { return int(fDetectorID % 100); }
   bool isVertical() { return GetPlane() == 1; }      
   void setLaterality(const int& lat) { laterality = lat; }
   int GetLaterality() {return laterality;};

   int GetClusterID() {return clusterID;}
   void setClusterID(const int& ID) { clusterID = ID;}

   int GetDetectorID() {return fDetectorID;}

private:
   Float_t flag; ///< flag

   Int_t   fDetectorID;     ///< Detector unique identifier

   Double_t timestamp; 

   Int_t    laterality = 0;

   Int_t    clusterID = -1;

   ClassDef(DriftTubeHit, 1);
};

#endif // SHIPLHC_DRIFTTUBEHIT_H_