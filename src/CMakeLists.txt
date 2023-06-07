# SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
# SPDX-FileCopyrightText: 2020 Alexander Stippich <a.stippich@gmx.net>
#
# SPDX-License-Identifier: BSD-2-Clause

include_directories(${skanpage_BINARY_DIR})

configure_file(qml/GlobalMenu.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/GlobalMenu.qml)
configure_file(qml/SettingsWindow.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/SettingsWindow.qml)
configure_file(qml/ExportWindow.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/ExportWindow.qml)
configure_file(qml/MainWindow.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/MainWindow.qml)
configure_file(qml/ShareWindow.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/ShareWindow.qml)
configure_file(qml.qrc.in ${CMAKE_CURRENT_SOURCE_DIR}/qml.generated.qrc)

add_executable(skanpage)

target_sources(skanpage PRIVATE
    DevicesModel.cpp
    DevicesModel.h
    DocumentModel.cpp
    DocumentModel.h
    DocumentPrinter.cpp
    DocumentPrinter.h
    DocumentSaver.cpp
    DocumentSaver.h
    InProgressPainter.cpp
    InProgressPainter.h
    main.cpp
    OptionsModel.cpp
    OptionsModel.h
    FilteredOptionsModel.cpp
    FilteredOptionsModel.cpp
    Skanpage.cpp
    Skanpage.h
    SkanpageUtils.cpp
    SkanpageUtils.h
    FormatModel.cpp
    FormatModel.h
    OCREngine.cpp
    OCREngine.h
    OCRLanguageModel.cpp
    OCRLanguageModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qml.generated.qrc
    )

ecm_qt_declare_logging_category(skanpage
    HEADER skanpage_debug.h
    IDENTIFIER SKANPAGE_LOG
    CATEGORY_NAME org.kde.skanpage
    DESCRIPTION "Skanpage"
    EXPORT SKANPAGE
)

ecm_qt_install_logging_categories(
    EXPORT SKANPAGE
    FILE skanpage.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

kconfig_add_kcfg_files(skanpage skanpage_state.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(skanpage skanpage_config.kcfgc GENERATE_MOC)

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
    KSane::Core
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Kirigami2
    KF${KF_MAJOR_VERSION}::Crash
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::ConfigGui
    KF${KF_MAJOR_VERSION}::Purpose
    KF${KF_MAJOR_VERSION}::XmlGui
)

if (TESSERACT_FOUND AND LEPTONICA_FOUND)
    target_link_libraries(skanpage
    PRIVATE
        Tesseract::Tesseract
        Leptonica::Leptonica
    )
endif()

install(TARGETS skanpage ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
