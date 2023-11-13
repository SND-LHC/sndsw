set(SND_CUTS_SRCS
sndSciFiBaseCut.cxx
sndMuFilterBaseCut.cxx
sndMinSciFiHitsCut.cxx
sndSciFiStationCut.cxx
sndVetoCut.cxx
sndMinSciFiConsecutivePlanes.cxx
sndDSActivityCut.cxx
sndUSQDCCut.cxx
sndEventHeaderBaseCut.cxx
sndEventDeltat.cxx
sndAvgSciFiFiducialCut.cxx
sndAvgDSFiducialCut.cxx
sndDSVetoCut.cxx
)

set(LINK_DIRECTORIES
${FAIRROOT_LIBRARY_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})
link_directories(${genfit2_LIBDIR})
link_directories(${Rave_LIBDIR})

add_library(sndCuts SHARED ${SND_CUTS_SRCS})
target_link_libraries(sndCuts ${ROOT_LIBRARIES} sndTools shipLHC veto)
