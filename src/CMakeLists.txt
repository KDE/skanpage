
include_directories(${skanpage_BINARY_DIR})

set(skanpage_SRCS
    main.cpp 
    Skanpage.cpp
    DocumentModel.cpp
    DocumentSaver.cpp
    DocumentPrinter.cpp
    DevicesModel.cpp
    OptionsModel.cpp
    SingleOption.cpp
    )

qt_add_resources(skanpage_SRCS qml.qrc)

ecm_qt_declare_logging_category(skanpage_SRCS
    HEADER skanpage_debug.h
    IDENTIFIER SKANPAGE_LOG
    CATEGORY_NAME org.kde.skanpage
)

add_executable(skanpage ${skanpage_SRCS})

target_link_libraries(skanpage
  PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Quick
    Qt::PrintSupport
    Qt::Qml
    Qt::QuickControls2
    Qt::Concurrent
  PRIVATE
    KF5::Sane
    KF5::CoreAddons
    KF5::I18n
    KF5::Kirigami2
    KF5::Crash
    KF5::XmlGui
)


install(TARGETS skanpage ${INSTALL_TARGETS_DEFAULT_ARGS})
