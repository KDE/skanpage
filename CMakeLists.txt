cmake_minimum_required(VERSION 3.10)

project(skanpage
        VERSION "0.0.1"
        LANGUAGES CXX)

set(REQUIRED_QT_VERSION "5.12.8")
set(REQUIRED_KF5_VERSION "5.72.0")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Qml QuickControls2 Widgets Quick PrintSupport)
find_package(ECM ${REQUIRED_KF5_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(ECMSetupVersion) # yiels ecm_setup_version
include(KDEInstallDirs) # yields ${XDG_APPS_INSTALL_DIR}
include(KDEFrameworkCompilerSettings)
    
find_package(KF5 ${REQUIRED_KF5_VERSION} REQUIRED
    COMPONENTS # alias to find_package(KFGuiAddons ${KF5_VERSION} REQUIRED)
        CoreAddons  # KAboutData
        I18n        # I18n
        XmlGui      # KAboutApplicationDialog
        KIO         # contains the KIOWidgets which we use in target_link_libraries
        #DocTools   # yields kdoctools_create_handbook
)

find_package(KF5 "5.55" REQUIRED
    COMPONENTS # alias to find_package(KFGuiAddons ${KF5_VERSION} REQUIRED)
        Sane        # KF5Sane
)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX SKANPAGE
    VERSION_HEADER skanpage-version.h)

set(CMAKE_AUTOMOC ON)

add_subdirectory(src)

