#!/bin/sh

# SPDX-FileCopyrightText: 2021 Yuri Chornoivan <yurchor@ukr.net>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL


$XGETTEXT `find . \( -name "*.qml" -or -name "*.cpp" -or -name "*.h" \)` -o $podir/skanpage.pot
