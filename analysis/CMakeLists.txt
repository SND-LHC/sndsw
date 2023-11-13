set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}
${VMC_INCLUDE_DIRS}
${CMAKE_SOURCE_DIR}/shipdata
${CMAKE_SOURCE_DIR}/shipLHC
${CMAKE_SOURCE_DIR}/analysis/cuts
${CMAKE_SOURCE_DIR}/analysis/tools
)

include_directories( ${INCLUDE_DIRECTORIES} )

add_subdirectory(cuts)
add_subdirectory(tools)

set(LINK_DIRECTORIES
${FAIRROOT_LIBRARY_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})
link_directories(${genfit2_LIBDIR})
link_directories(${Rave_LIBDIR})

add_executable(neutrinoFilterGoldenSample neutrinoFilterGoldenSample.cxx)
target_link_libraries(neutrinoFilterGoldenSample ${ROOT_LIBRARIES} sndCuts)
