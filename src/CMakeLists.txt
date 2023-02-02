# SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
# SPDX-FileCopyrightText: 2020 Alexander Stippich <a.stippich@gmx.net>
#
# SPDX-License-Identifier: BSD-2-Clause

include_directories(${skanpage_BINARY_DIR})

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
    qml.qrc
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
)

if (TESSERACT_FOUND AND LEPTONICA_FOUND)
    target_link_libraries(skanpage
    PRIVATE
        Tesseract::Tesseract
        Leptonica::Leptonica
    )
endif()

install(TARGETS skanpage ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
