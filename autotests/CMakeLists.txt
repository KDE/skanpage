#
# SPDX-FileCopyrightText: 2020 by Alexander Stippich <a.stippich@gmx.net>
#
# SPDX-License-Identifier: BSD-2-Clause
#

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include(ECMAddTests)

set(skanpageTest_SRCS)
ecm_qt_declare_logging_category(skanpageTest_SRCS
    HEADER skanpage_debug.h
    IDENTIFIER SKANPAGE_LOG
    CATEGORY_NAME org.kde.skanpage
)

ecm_add_test(documentSaverTest.cpp ../src/DocumentSaver.cpp  ../src/OCREngine.cpp  ../src/OCRLanguageModel.cpp ../src/SkanpageUtils.cpp ${skanpageTest_SRCS}
    TEST_NAME "documentsavertest"
    LINK_LIBRARIES 
        Qt::Test
        Qt::Gui
        Qt::Concurrent
        KF${KF_MAJOR_VERSION}::I18n
)

if (TESSERACT_FOUND AND LEPTONICA_FOUND)
    target_link_libraries(documentsavertest
        Tesseract::Tesseract
        Leptonica::Leptonica
    )
endif()
