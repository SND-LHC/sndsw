cmake_minimum_required(VERSION 3.12)
project(G4Processes)

# Find GEANT4 and ROOT
find_package(Geant4 REQUIRED)
find_package(ROOT REQUIRED)

# Create library
add_library(G4Processes SHARED G4MuonDISProcess.cc G4MuonDISProcess.hh ${CMAKE_SOURCE_DIR}/shipgen/MuDISGenerator.cxx)

# Include headers, also from GEANT4 and ROOT
target_include_directories(G4Processes PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/shipgen
  ${GEANT4_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIR}
)

target_link_libraries(G4Processes PUBLIC 
  ${GEANT4_LIBRARIES}     # Link GEANT4 libraries
  ${ROOT_LIBRARIES}       # Link ROOT libraries
  ${ROOT_LIBRARY_DIR}/libEGPythia6.so
  ${CMAKE_SOURCE_DIR}/shipgen # Link shipgen
)

# Install the library
install(TARGETS G4Processes DESTINATION lib)
