# SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
#
# SPDX-License-Identifier: BSD-2-Clause
#
#  Leptonica_FOUND - system has Leptonica
#  Leptonica_INCLUDE_DIR - the Leptonica include directory
#  Leptonica_LIBRARIES - The libraries needed to use Leptonica

find_path(Leptonica_INCLUDE_DIRS leptonica/allheaders.h
   /usr/include
   /usr/local/include
   /opt/local/include
)

find_library(Leptonica_LIBRARIES NAMES leptonica lept libleptonica liblept
   PATHS
   /usr/lib
   /usr/local/lib
   /opt/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Leptonica
    FOUND_VAR
        Leptonica_FOUND
    REQUIRED_VARS
        Leptonica_LIBRARIES
        Leptonica_INCLUDE_DIRS
)

if(Leptonica_FOUND AND NOT TARGET Leptonica::Leptonica)
    add_library(Leptonica::Leptonica UNKNOWN IMPORTED)
    set_target_properties(Leptonica::Leptonica PROPERTIES
        IMPORTED_LOCATION "${Leptonica_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${Leptonica_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(Leptonica_LIBRARIES Leptonica_INCLUDE_DIRS Leptonica_VERSION)

include(FeatureSummary)
set_package_properties(Leptonica PROPERTIES
    DESCRIPTION "Image processing library"
    URL "http://leptonica.org/"
)
