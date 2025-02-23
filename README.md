<!--
 SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>

 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
-->


# Skanpage

Skanpage is a multi-page scanning application built using the KSaneCore library and a QML interface.
It supports saving to image and PDF files. It uses tesseract as an OCR engine.

# Development and contributions

Please report bugs at KDE's Bugzilla at https://bugs.kde.org/enter_bug.cgi?product=skanpage.
Please always include the installed SANE version, your scanner device model and the backend if known.

Skanpage's source code can be found at https://invent.kde.org/utilities/skanpage/.

To submit a patch to Skanpage, use https://invent.kde.org/utilities/skanpage/-/merge_requests.

## Build and run from source

To build Skanpage from source, also see https://community.kde.org/Get_Involved/development.

In order to build the master branch of Skanpage, you need the same branch of KSaneCore, at minimum a version of KDE Frameworks 6.9 and Qt 6.5.

## Command line options

-d, --device _device_ Specify a SANE scanner device name that shall be opened upon launch of Skanpage.

-i, --import _file/path/to/import/file.pdf_ Specify a file path to import an image or PDF file when opening Skanpage.

-o, --options _file/path/to/output.txt_ Specify a file path where all option parameters of a scanner device a written in a JSON format. Useful for debugging.

## Debugging

Through KSaneCore, Skanpage is relying on the SANE (Scanner Access Now Easy) backends for interfacing with the scanner hardware.

As there are many different hardware with different backends, finding the root cause of issues or testing new features may be difficult as users may use hardware and backends developers has no access to. However, there are some possibilities that help with debugging, especially for the postprocessing after a scan has been made (e.g. saving, OCR etc.). 

SANE provides the "test" backend that is not visible by default on most distros. You can start Skanpage with the device command line option and specify test, e.g.

```skanpage -d test```

The test backend simulates a scanner and can be used to generate images, almost interacting with the graphical user interface etc. as with a normal scanner.

Via the import option on the command line, Skanpage also provides some help for faster debugging for the postprocessing steps.
In the source repository, various test images are available in the autotest folder /autotests/images/.

```skanpage -i /path/to/test/pages1-8.pdf```

In case of specific issue with a hardware and backend, the output of the option parameters may be helpful, also for bug reports.

```-o /path/to/options.txt```

For a enabling debugging categories:

```QT_LOGGING_RULES="org.kde.skanpage=true;org.kde.ksanecore=true" skanpage```

If you want to debug Skanpage and have images already imported on launch, you can a useful combination of the above, e.g.:

```QT_LOGGING_RULES="org.kde.skanpage=true;org.kde.ksanecore=true" skanpage -d test -i /path/to/test/pages1-8.pdf```
