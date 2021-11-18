# SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
# SPDX-FileCopyrightText: 2020 Alexander Stippich <a.stippich@gmx.net>
#
# SPDX-License-Identifier: BSD-2-Clause

include_directories(${skanpage_BINARY_DIR})

set(skanpage_SRCS
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
    SingleOption.cpp
    SingleOption.h
    Skanpage.cpp
    Skanpage.h
    SkanpageUtils.cpp
    SkanpageUtils.h
    )

qt_add_resources(skanpage_SRCS qml.qrc)

ecm_qt_declare_logging_category(skanpage_SRCS
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
    KF5::ConfigCore
)

install(TARGETS skanpage ${INSTALL_TARGETS_DEFAULT_ARGS})
