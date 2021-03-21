# Skanpage

Skanpage is a multi-page scanning application built using the libksane library and a QML interface.
It supports saving to image and PDF files.

Skanpage is currently in development and has ALPHA quality.

## Build and run from source

In order to build Skanpage, you need Qt, KDE Frameworks and usually the latest, unreleased version of libksane.

## Known limitations

The additional options interface is currently provided directly by libksane and still uses a QWidget-based interface.
The options provided directly in Skanpage may not be synced between the two different interfaces.
This is a known limitation and is actively being worked on.
