cmake_minimum_required(VERSION 3.10)

project(skanpage
        VERSION "0.0.1"
        LANGUAGES CXX)

set(REQUIRED_QT_VERSION "5.14.2")
set(REQUIRED_KF5_VERSION "5.74.0")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Qml QuickControls2 Widgets Quick PrintSupport)
find_package(ECM ${REQUIRED_KF5_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(ECMSetupVersion) 
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

find_package(KF5 ${REQUIRED_KF5_VERSION} REQUIRED
    COMPONENTS # alias to find_package(KFGuiAddons ${KF5_VERSION} REQUIRED)
        CoreAddons  # KAboutData
        I18n        # I18n
        XmlGui      # KAboutApplicationDialog
        KIO         # contains the KIOWidgets which we use in target_link_libraries
        Kirigami2
        #DocTools   # yields kdoctools_create_handbook
)

find_package(KF5 "5.55" REQUIRED
    COMPONENTS # alias to find_package(KFGuiAddons ${KF5_VERSION} REQUIRED)
        Sane        # KF5Sane
)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX SKANPAGE
    VERSION_HEADER skanpage_version.h)
    
install(FILES skanpage.categories DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
install(PROGRAMS org.kde.skanpage.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES org.kde.skanpage.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )

set(CMAKE_AUTOMOC ON)

add_subdirectory(src)

ki18n_install(po)
