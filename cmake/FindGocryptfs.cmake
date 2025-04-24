# SPDX-FileCopyrightText: 2025 Nate Graham <nate@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause


find_program(GOCRYPTFS_PROGRAM gocryptfs)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gocryptfs
    FOUND_VAR
        Gocryptfs_FOUND
    REQUIRED_VARS
        GOCRYPTFS_PROGRAM
)
