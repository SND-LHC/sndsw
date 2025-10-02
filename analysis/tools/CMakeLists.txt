set(INCLUDE_DIRECTORIES
${SYSTEM_INCLUDE_DIRECTORIES}
${VMC_INCLUDE_DIRS}
${CMAKE_SOURCE_DIR}/shipdata
${CMAKE_SOURCE_DIR}/shipLHC
${CMAKE_SOURCE_DIR}/analysis/tools
${FMT_INCLUDE_DIR}
)

include_directories( ${INCLUDE_DIRECTORIES} )

set(SRCS
sndSciFiTools.cxx
sndTchainGetter.cxx
sndGeometryGetter.cxx
sndConfiguration.cxx
sndScifiPlane.cxx
sndUSPlane.cxx
sndPlaneTools.cxx
sndShowerTools.cxx
)

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
)

link_directories( ${LINK_DIRECTORIES})

Set(HEADERS )
Set(LINKDEF AnalysisToolsLinkDef.h)
Set(LIBRARY_NAME snd_analysis_tools)
Set(DEPENDENCIES shipLHC)

GENERATE_LIBRARY()

target_link_libraries(snd_analysis_tools -lROOTTPython -lMatrix -lMathCore -lCore)