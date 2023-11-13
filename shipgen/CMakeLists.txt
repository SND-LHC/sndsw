# Create a library called "libShipGen" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${CMAKE_SOURCE_DIR}/shipgen
${CMAKE_SOURCE_DIR}/generators
${CMAKE_SOURCE_DIR}/shipdata
${CMAKE_SOURCE_DIR}/veto
)

set(SYSTEM_INCLUDE_DIRECTORIES
${SYSTEM_INCLUDE_DIRECTORIES}
${PYTHIA8_INCLUDE_DIR}
${PYTHIA8_INCLUDE_DIR}/..
${HEPMC_INCLUDE_DIR}
${EVTGEN_INCLUDE_DIR}
${EVTGEN_INCLUDE_DIR}/..
$ENV{GENIE_ROOT}/genie/src 
)

include_directories(${INCLUDE_DIRECTORIES} ${VMC_INCLUDE_DIRS} ${FAIRROOT_INCLUDE_DIR} ${FairLogger_INCDIR} ${FMT_INCLUDE_DIR})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${PYTHIA8_LIBRARY_DIR}
${EVTGEN_LIBRARY_DIR}
${SIMPATH}/lib
)

link_directories(${LINK_DIRECTORIES})
link_directories(${genfit2_LIBDIR})
link_directories(${Rave_LIBDIR})

set(SRCS
Pythia8Generator.cxx
Pythia6Generator.cxx
tPythia6Generator.cxx
HNLPythia8Generator.cxx
DPPythia8Generator.cxx
GenieGenerator.cxx
NuageGenerator.cxx
NtupleGenerator.cxx
NtupleGenerator_FLUKA.cxx
NeutronGenerator_FLUKA.cxx
MuonBackGenerator.cxx
CosmicsGenerator.cxx
MuDISGenerator.cxx
FixedTargetGenerator.cxx
ALPACAGenerator.cxx
)

set(LINKDEF GenLinkDef.h)
set(LIBRARY_NAME ShipGen)

set(DEPENDENCIES Base ShipData EvtGen EvtGenExternal)

GENERATE_LIBRARY()

add_executable(sndlhcGSimpleNtpConverter sndlhcGSimpleNtpConverter.cxx)
add_executable(addAuxiliaryToGST addAuxiliaryToGST.cxx)

target_link_libraries(sndlhcGSimpleNtpConverter -Wl,--allow-shlib-undefined -L$ENV{GENIE_ROOT}/lib/ -lGFwMsg -lGFwReg -lGFwAlg -lGFwInt -lGFwGHEP -lGFwNum -lGFwUtl -lGFwParDat -lGFwEG -lGFwNtp -lGTlFlx ${ROOT_LIBRARIES} -lEGPythia6 -L$ENV{LIBXML2_ROOT}/lib -lxml2 -L$ENV{LOG4_ROOT}/lib -llog4cpp -L$ENV{LHAPDF_ROOT}/lib -lLHAPDF)
target_link_libraries(addAuxiliaryToGST -Wl,--allow-shlib-undefined -L$ENV{GENIE_ROOT}/lib/ -lGFwMsg -lGFwReg -lGFwAlg -lGFwInt -lGFwGHEP -lGFwNum -lGFwUtl -lGFwParDat -lGFwEG -lGFwNtp -lGTlFlx ${ROOT_LIBRARIES} -lEGPythia6 -L$ENV{LIBXML2_ROOT}/lib -lxml2 -L$ENV{LOG4_ROOT}/lib -llog4cpp -L$ENV{LHAPDF_ROOT}/lib -lLHAPDF)
