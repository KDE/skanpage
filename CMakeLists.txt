# SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
# SPDX-FileCopyrightText: 2020 Alexander Stippich <a.stippich@gmx.net>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

cmake_minimum_required(VERSION 3.16)

project(skanpage
        VERSION "0.0.1"
        LANGUAGES CXX)

set(REQUIRED_QT_VERSION "5.15.2")
set(REQUIRED_KF5_VERSION "5.83")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Qml QuickControls2 Concurrent Widgets Quick PrintSupport)
find_package(ECM ${REQUIRED_KF5_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

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

find_package(KF5 ${REQUIRED_KF5_VERSION} REQUIRED
    COMPONENTS
        CoreAddons
        I18n
        Kirigami2
        Crash
        Config
        XmlGui
        #DocTools
)

find_package(KF5 "21.07.80" REQUIRED
    COMPONENTS
        Sane
)

add_definitions(
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_KEYWORDS
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    -DQT_STRICT_ITERATORS
)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX SKANPAGE
    VERSION_HEADER skanpage_version.h)

install(PROGRAMS org.kde.skanpage.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES org.kde.skanpage.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )

add_subdirectory(src)
add_subdirectory(icons)
#add_subdirectory(autotests)

ki18n_install(po)
