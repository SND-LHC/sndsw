set(INCLUDE_DIRECTORIES
${SYSTEM_INCLUDE_DIRECTORIES}
${VMC_INCLUDE_DIRS}
${CMAKE_SOURCE_DIR}/shipdata
${CMAKE_SOURCE_DIR}/shipLHC
${CMAKE_SOURCE_DIR}/analysis/cuts
${FMT_INCLUDE_DIR}
)

include_directories( ${INCLUDE_DIRECTORIES} )

set(SRCS
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

Set(HEADERS )
Set(LINKDEF AnalysisCutsLinkDef.h)
Set(LIBRARY_NAME snd_analysis_cuts)
Set(DEPENDENCIES snd_analysis_tools shipLHC veto)

GENERATE_LIBRARY()
