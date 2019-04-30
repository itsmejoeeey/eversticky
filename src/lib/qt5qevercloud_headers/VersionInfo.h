/**
 * Copyright (c) 2017 Dmitry Ivanov
 *
 * This file is a part of QEverCloud project and is distributed under the terms of MIT license:
 * https://opensource.org/licenses/MIT
 */

#ifndef QEVERCLOUD_VERSION_INFO_H
#define QEVERCLOUD_VERSION_INFO_H

/**
 * This file contains configuration-specific macro definitions
 * which can then be tested in compile time by library users
 */

/**
 * This macro tells whether QEverCloud library was built with OAuth support
 */
#define QEVERCLOUD_HAS_OAUTH 1

/**
 * This macro tells whether QEverCloud library uses QtWebEngine backend for OAuth support
 * (if it was built with OAuth support)
 */
#define QEVERCLOUD_USE_QT_WEB_ENGINE 1

#endif // QEVERCLOUD_VERSION_INFO_H
