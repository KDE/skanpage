# SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
# SPDX-FileCopyrightText: 2020 Alexander Stippich <a.stippich@gmx.net>
#
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Gear version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "24")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "70")

set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(skanpage
        VERSION ${RELEASE_SERVICE_VERSION}
        LANGUAGES CXX)

set(QT_REQUIRED_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH})
include(FeatureSummary)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDEClangFormat)
include(KDECMakeSettings)
include(ECMAddAppIcon)
include(ECMInstallIcons)

find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Core Qml QuickControls2 Concurrent Widgets Quick PrintSupport Pdf)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED
    COMPONENTS
        CoreAddons
        I18n
        Kirigami2
        Crash
        Config
        Purpose
        XmlGui
)

find_package(KSaneCore6 "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}" REQUIRED)

find_package(KQuickImageEditor REQUIRED)

find_package(Tesseract 5)
find_package(Leptonica)

if (LEPTONICA_FOUND AND TESSERACT_FOUND)
   set(OCR_AVAILABLE 1)
   set(CMAKE_CXX_FLAGS "-fopenmp")
else()
   message(STATUS "Skanpage will be built without OCR as libraries are missing.")
endif()

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX SKANPAGE
    VERSION_HEADER skanpage_version.h)

install(PROGRAMS org.kde.skanpage.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.skanpage.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(config-skanpage.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-skanpage.h)

add_subdirectory(src)
add_subdirectory(icons)
#add_subdirectory(autotests)

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
