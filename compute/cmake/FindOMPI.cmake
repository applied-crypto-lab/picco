include(FindPackageHandleStandardArgs)

if(OMPI_INCLUDES AND OMPI_LIBRARIES)
  set(OMPI_FIND_QUIETLY TRUE)
endif(OMPI_INCLUDES AND OMPI_LIBRARIES)

find_path(OMPI_INCLUDES
  NAMES
  ompi/omp.h
  HINTS
  "${OMPDIR}/include"
  PATHS
  $ENV{OMPDIR}
  ${INCLUDE_INSTALL_DIR}
)

find_library(OMPI_LIBRARIES
  NAMES
  ompi/default/libort.a
  HINTS
  "${OMPDIR}/lib"
  PATHS
  $ENV{OMPDIR}
  ${LIB_INSTALL_DIR}
)

set(OMPI_FOUND NO)

if(OMPI_LIBRARIES AND OMPI_INCLUDES)
  set(OMPI_FOUND YES)
endif()

find_package_handle_standard_args(OMP DEFAULT_MSG
  OMPI_INCLUDES OMPI_LIBRARIES
)

if(OMPI_FOUND)
  mark_as_advanced(OMPI_INCLUDES)
  mark_as_advanced(OMPI_LIBRARIES)
endif()
