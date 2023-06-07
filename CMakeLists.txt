# SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
# SPDX-FileCopyrightText: 2020 Alexander Stippich <a.stippich@gmx.net>
#
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Gear version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "70")

set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(skanpage
        VERSION ${RELEASE_SERVICE_VERSION}
        LANGUAGES CXX)

set(REQUIRED_QT_VERSION "5.15.2")
set(REQUIRED_KF5_VERSION "5.94")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(ECM ${REQUIRED_KF5_VERSION} REQUIRED NO_MODULE)
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
include(ECMDeprecationSettings)
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()


if(QT_MAJOR_VERSION STREQUAL "6")
    set(KIRIGAMI_ICON_NAME "icon.name")
    set(FILEDIALOG_CURRENT_FOLDER "currentFolder")
    set(FILEDIALOG_SELECTED_NAME_FILTER "selectedNameFilter.index")
    set(FILEDIALOG_SELECTED_FILE "selectedFile")
    set(FILEDIALOG_MODE_SAVE "fileMode: FileDialog.SaveFile")
    set(FILEDIALOG_MODE_LOAD_FOLDER "FolderDialog")
    set(FILEDIALOG_MODE_LOAD_FOLDER_SELECTEXISTING "")
    set(FILEDIALOG_MODE_LOAD_FOLDER_SELECTFOLDER "")
    set(FILEDIALOG_MODE_LOAD_FOLDER_NOTSELECTFOLDER "")
    set(QTQUICK_DIALOGS_IMPORT "QtQuick.Dialogs")
    set(BASICLISTITEM_ICON "icon.name")
else()
    set(KIRIGAMI_ICON_NAME "iconName")
    set(FILEDIALOG_CURRENT_FOLDER "folder")
    set(FILEDIALOG_SELECTED_NAME_FILTER "selectedNameFilter")
    set(FILEDIALOG_MODE_SAVE "selectExisting: false")
    set(FILEDIALOG_SELECTED_FILE "fileUrl")
    set(FILEDIALOG_MODE_LOAD_FOLDER "FileDialog")
    set(FILEDIALOG_MODE_LOAD_FOLDER_SELECTEXISTING "selectExisting: true")
    set(FILEDIALOG_MODE_LOAD_FOLDER_SELECTFOLDER "selectFolder: true")
    set(FILEDIALOG_MODE_LOAD_FOLDER_NOTSELECTFOLDER "selectExisting: false")
    set(QTQUICK_DIALOGS_IMPORT "QtQuick.Dialogs 1.3")
    set(BASICLISTITEM_ICON "icon")
endif()


find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Qml QuickControls2 Concurrent Widgets Quick PrintSupport)

find_package(KF${KF_MAJOR_VERSION} ${REQUIRED_KF5_VERSION} REQUIRED
    COMPONENTS
        CoreAddons
        I18n
        Kirigami2
        Crash
        Config
        Purpose
        XmlGui
)

find_package(KSaneCore "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}" REQUIRED)

find_package(KQuickImageEditor REQUIRED)

find_package(Tesseract 4)
find_package(Leptonica)

if (LEPTONICA_FOUND AND TESSERACT_FOUND)
   set(OCR_AVAILABLE 1)
   set(CMAKE_CXX_FLAGS "-fopenmp")
else()
   message(STATUS "Skanpage will be built without OCR as libraries are missing.")
endif()

ecm_set_disabled_deprecation_versions(QT 6.4
    KF 5.101
)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX SKANPAGE
    VERSION_HEADER skanpage_version.h)

install(PROGRAMS org.kde.skanpage.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.skanpage.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(config-skanpage.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-skanpage.h)

add_subdirectory(src)
add_subdirectory(icons)
#add_subdirectory(autotests)

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
