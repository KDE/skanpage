
include_directories(${skanpage_BINARY_DIR})

set(skanpage_SRCS main.cpp SaveFileDialog.cpp DocumentModel.cpp Skanpage.cpp)

qt5_add_resources(skanpage_SRCS qml.qrc)

add_executable(skanpage ${skanpage_SRCS})

target_link_libraries(skanpage
  PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Quick
    Qt5::PrintSupport
    Qt5::Qml
    Qt5::QuickControls2
  PRIVATE
    KF5::CoreAddons
    KF5::Sane
    KF5::I18n
    KF5::XmlGui
    KF5::KIOWidgets
)


install(TARGETS skanpage ${INSTALL_TARGETS_DEFAULT_ARGS})
