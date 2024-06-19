#ifndef SNDSCIFIHIT_H
#define SNDSCIFIHIT_H 1

#include "SndlhcHit.h"
#include "ScifiPoint.h"
#include "TObject.h"
#include "TVector3.h"

class sndScifiHit : public SndlhcHit
{
  public:

    /** Default constructor **/
    sndScifiHit();
    /** Copy constructor **/
    sndScifiHit(const sndScifiHit& hit) = default;
    sndScifiHit& operator=(const sndScifiHit& hit) = default;
    explicit sndScifiHit(Int_t detID);
    //  Constructor from ScifiPoint
    sndScifiHit(int detID,std::vector<ScifiPoint*>,std::vector<Float_t>);

 /** Destructor **/
    virtual ~sndScifiHit();

    /** Output to screen **/
	void Print() ;
	Float_t GetEnergy();

	void setInvalid() {flag = false;}
	bool isValid() const {return flag;}
	Int_t GetStation(){return floor(fDetectorID/1000000);}
	bool isVertical(){
		if ( int(fDetectorID/100000)%10 == 1){return true;}
		else{return (false);}
	}
	Int_t  GetMat(){return ( int(fDetectorID/10000)%10);}
	Int_t  GetSiPM(){return ( int(fDetectorID/1000)%10);}
	Int_t GetSiPMChan(){return ( fDetectorID%1000);}
	Int_t GetChannelID(){return fDetectorID;}
/*  
	from Guido (22.9.2021): A threshold of 3.5pe should be used, which corresponds to 0.031MeV.
	1 SiPM channel has 104 pixels, pixel can only see 0 or >0 photons.
*/
  private:
    Float_t ly_loss(Float_t distance);
    Float_t sipm_saturation(Float_t ly, Float_t nphe_max);
    Float_t npix_to_qdc(Float_t npix);
    Float_t flag;   ///< flag

    ClassDef(sndScifiHit, 4);

};

#endif
