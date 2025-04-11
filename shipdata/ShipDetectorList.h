// -------------------------------------------------------------------------A
// -----                  ShipDetectorList header file                  -----
// -------------------------------------------------------------------------


/** Defines unique identifier for all SND@LHC detector systems **/

#ifndef ShipDetectorList_H
#define ShipDetectorList_H 1

// kEndOfList is needed for iteration over the enum. All detectors have to be put before.
 enum DetectorId {kEmulsionDet,kLHCScifi,kMuFilter,kDriftTube, kVETO, kTimeDet, ktauRpc, ktauHpt, ktauTT, ktauTarget, kStraw, kecal, khcal, kMuon , kPreshower, kTRSTATION, kSplitCal, kBox1, kSpectrometer\
			  , kPixelModules, kSciFi, kScintillator, kMufluxSpectrometer, kMuonTagger, kUpstreamTagger, kEndOfList};
// First four for SND@LHC, others legacy from SHiP
#endif
