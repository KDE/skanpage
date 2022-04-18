# SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
#
# SPDX-License-Identifier: BSD-2-Clause
#
#  Tesseract_FOUND - system has Tesseract
#  Tesseract_INCLUDE_DIR - the Tesseract include directory
#  Tesseract_LIBRARIES - The libraries needed to use Tesseract

find_path(Tesseract_INCLUDE_DIRS tesseract/baseapi.h
   /usr/include
   /usr/local/include
   /opt/local/include
)

find_library(Tesseract_LIBRARIES NAMES tesseract libtesseract
   PATHS
   /usr/lib
   /usr/local/lib
   /opt/local/lib
)

if(NOT Tesseract_VERSION AND DEFINED Tesseract_INCLUDE_DIRS)
    if(EXISTS "${Tesseract_INCLUDE_DIRS}/tesseract/version.h")
        file(READ "${Tesseract_INCLUDE_DIRS}/tesseract/version.h" TESSERACT_H)

        string(REGEX MATCH "#define TESSERACT_MAJOR_VERSION[ ]+[0-9]+" TESSERACT_MAJOR_VERSION_MATCH ${TESSERACT_H})
        string(REGEX MATCH "#define TESSERACT_MINOR_VERSION[ ]+[0-9]+" TESSERACT_MINOR_VERSION_MATCH ${TESSERACT_H})
        string(REGEX MATCH "#define TESSERACT_PATCH_VERSION[ ]+[0-9]+" TESSERACT_PATCH_VERSION_MATCH ${TESSERACT_H})

        string(REGEX REPLACE ".*_MAJOR_VERSION[ ]+(.*)" "\\1" TESSERACT_MAJOR_VERSION "${TESSERACT_MAJOR_VERSION_MATCH}")
        string(REGEX REPLACE ".*_MINOR_VERSION[ ]+(.*)" "\\1" TESSERACT_MINOR_VERSION "${TESSERACT_MINOR_VERSION_MATCH}")
        string(REGEX REPLACE ".*_PATCH_VERSION[ ]+(.*)" "\\1" TESSERACT_PATCH_VERSION "${TESSERACT_PATCH_VERSION_MATCH}")

        set(Tesseract_VERSION "${TESSERACT_MAJOR_VERSION}.${TESSERACT_MINOR_VERSION}.${TESSERACT_PATCH_VERSION}")
    elseif(EXISTS "${Tesseract_INCLUDE_DIRS}/tesseract/tess_version.h")
        file(READ "${Tesseract_INCLUDE_DIRS}/tesseract/tess_version.h" TESSERACT_H)

        string(REGEX MATCH "#define TESSERACT_MAJOR_VERSION[ ]+[0-9]+" TESSERACT_MAJOR_VERSION_MATCH ${TESSERACT_H})
        string(REGEX MATCH "#define TESSERACT_MINOR_VERSION[ ]+[0-9]+" TESSERACT_MINOR_VERSION_MATCH ${TESSERACT_H})
        string(REGEX MATCH "#define TESSERACT_PATCH_VERSION[ ]+[0-9]+" TESSERACT_PATCH_VERSION_MATCH ${TESSERACT_H})

        string(REGEX REPLACE ".*_MAJOR_VERSION[ ]+(.*)" "\\1" TESSERACT_MAJOR_VERSION "${TESSERACT_MAJOR_VERSION_MATCH}")
        string(REGEX REPLACE ".*_MINOR_VERSION[ ]+(.*)" "\\1" TESSERACT_MINOR_VERSION "${TESSERACT_MINOR_VERSION_MATCH}")
        string(REGEX REPLACE ".*_PATCH_VERSION[ ]+(.*)" "\\1" TESSERACT_PATCH_VERSION "${TESSERACT_PATCH_VERSION_MATCH}")

        set(Tesseract_VERSION "${TESSERACT_MAJOR_VERSION}.${TESSERACT_MINOR_VERSION}.${TESSERACT_PATCH_VERSION}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Tesseract
    FOUND_VAR
        Tesseract_FOUND
    REQUIRED_VARS
        Tesseract_LIBRARIES
        Tesseract_INCLUDE_DIRS
    VERSION_VAR
        Tesseract_VERSION
)

if(Tesseract_FOUND AND NOT TARGET Tesseract::Tesseract)
    add_library(Tesseract::Tesseract UNKNOWN IMPORTED)
    set_target_properties(Tesseract::Tesseract PROPERTIES
        IMPORTED_LOCATION "${Tesseract_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${Tesseract_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(Tesseract_LIBRARIES Tesseract_INCLUDE_DIRS Tesseract_VERSION)

include(FeatureSummary)
set_package_properties(Tesseract PROPERTIES
    DESCRIPTION "Optical Character Recognition (OCR) library"
    URL "https://github.com/tesseract-ocr/tesseract"
)
