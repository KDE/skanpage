cmake_minimum_required(VERSION 3.10)

project(skanpage)

find_package(ECM 1.2.0 REQUIRED NO_MODULE)
# ECM setup2 and include find same cmake module (copied from /usr/share/kde4/apps/cmake/modules/FindSane.cmake)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_SOURCE_DIR}/cmake-modules)

include(FeatureSummary)
include(ECMSetupVersion) # yiels ecm_setup_version
include(KDEInstallDirs) # yields ${XDG_APPS_INSTALL_DIR}
include(KDEFrameworkCompilerSettings)


# Dependencies
set(REQUIRED_QT_VERSION "5.4.1")
set(SKANPAGE_VERSION_STRING "1.0.0")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Widgets Quick PrintSupport)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

find_package(KF5 ${KF5_VERSION} REQUIRED
    COMPONENTS # alias to find_package(KFGuiAddons ${KF5_VERSION} REQUIRED)
        CoreAddons  # KAboutData
        I18n        # I18n
        XmlGui      # KAboutApplicationDialog
        KIO         # contains the KIOWidgets which we use in target_link_libraries
        #DocTools   # yields kdoctools_create_handbook
        Sane        # KF5Sane
)

set(CMAKE_AUTOMOC ON)


set(skanpage_SRCS main.cpp SaveFileDialog.cpp DocumentModel.cpp Skanpage.cpp)

qt5_add_resources(skanpage_SRCS qml.qrc)

add_executable(skanpage ${skanpage_SRCS})

target_link_libraries(skanpage
  PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Quick
    Qt5::PrintSupport
  PRIVATE
    KF5::CoreAddons
    KF5::Sane
    KF5::I18n
    KF5::XmlGui
    KF5::KIOWidgets
)

# This target does some QML sanity checks as a by-product ;)
add_custom_target(lupdate ALL
    lupdate ${CMAKE_SOURCE_DIR} -ts ${CMAKE_CURRENT_BINARY_DIR}/project.ts
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "running lupdate"
)

install(TARGETS skanpage ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(PROGRAMS org.kde.skanlite.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
