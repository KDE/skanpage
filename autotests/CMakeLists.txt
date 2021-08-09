#
# SPDX-FileCopyrightText: 2020 by Alexander Stippich <a.stippich@gmx.net>
#
# SPDX-License-Identifier: BSD-2-Clause
#

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include(ECMAddTests)

set(skanpageTest_SRCS)
ecm_qt_declare_logging_category(skanpageTest_SRCS
    HEADER skanpage_debug.h
    IDENTIFIER SKANPAGE_LOG
    CATEGORY_NAME org.kde.skanpage
)

ecm_add_test(documentSaverTest.cpp ../src/DocumentSaver.cpp ../src/SkanpageUtils.cpp ${skanpageTest_SRCS}
    TEST_NAME "documentsavertest"
    LINK_LIBRARIES 
        Qt::Test
        Qt::Gui
        Qt::Concurrent
        KF5::I18n
)

