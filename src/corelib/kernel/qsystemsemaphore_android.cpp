/****************************************************************************
**
** Copyright (C) 2012 BogDan Vatra <bogdan@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsystemsemaphore.h"
#include "qsystemsemaphore_p.h"

#include <qdebug.h>
#include <qfile.h>
#include <qcoreapplication.h>

#ifndef QT_NO_SYSTEMSEMAPHORE

#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <errno.h>


QT_BEGIN_NAMESPACE

QSystemSemaphorePrivate::QSystemSemaphorePrivate() :
        android_key(NULL), error(QSystemSemaphore::NoError)
{
}

void QSystemSemaphorePrivate::setErrorString(const QString &function)
{
    // EINVAL is handled in functions so they can give better error strings
    switch (errno) {
    case EPERM:
    case EACCES:
        errorString = QCoreApplication::translate("QSystemSemaphore", "%1: permission denied").arg(function);
        error = QSystemSemaphore::PermissionDenied;
        break;
    case EEXIST:
        errorString = QCoreApplication::translate("QSystemSemaphore", "%1: already exists").arg(function);
        error = QSystemSemaphore::AlreadyExists;
        break;
    case ENOENT:
        errorString = QCoreApplication::translate("QSystemSemaphore", "%1: does not exist").arg(function);
        error = QSystemSemaphore::NotFound;
        break;
    case ERANGE:
    case ENOSPC:
        errorString = QCoreApplication::translate("QSystemSemaphore", "%1: out of resources").arg(function);
        error = QSystemSemaphore::OutOfResources;
        break;
    default:
        errorString = QCoreApplication::translate("QSystemSemaphore", "%1: unknown error %2").arg(function).arg(errno);
        error = QSystemSemaphore::UnknownError;
#if defined QSYSTEMSEMAPHORE_DEBUG
        qDebug() << errorString << "key" << key << "errno" << errno << EINVAL;
#endif
    }
}

/*!
    \internal

    Setup android_key
 */
void QSystemSemaphorePrivate::handle(QSystemSemaphore::AccessMode mode)
{

    android_key=sem_open(makeKeyFileName().toLatin1(), (mode == QSystemSemaphore::Open) ? O_CREAT : O_CREAT | O_EXCL);
    if (SEM_FAILED == android_key)
        setErrorString(QLatin1String("QSystemSemaphore::handle"));
}

/*!
    \internal

    Cleanup the android_key
 */
void QSystemSemaphorePrivate::cleanHandle()
{
    if (android_key && SEM_FAILED != android_key)
        sem_close(android_key);
}

/*!
    \internal
 */
bool QSystemSemaphorePrivate::modifySemaphore(int count)
{
    if (!android_key || SEM_FAILED == android_key)
        return false;

    if (count > 0)
        while(count--)
            sem_post(android_key);
    else
        sem_wait(android_key);
    return true;
}

QT_END_NAMESPACE

#endif // QT_NO_SYSTEMSEMAPHORE
