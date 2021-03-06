/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2008-09-14
 * Description : a presentation tool.
 *
 * Copyright (C) 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "presentationaudiowidget.h"

// Qt includes

#include <QTime>
#include <QUrl>
#include <QKeyEvent>
#include <QIcon>

// QtAV includes

#include <QtAV/AVError.h>
#include <QtAV/AVPlayer.h>

// Local includes

#include "presentationcontainer.h"
#include "digikam_debug.h"

using namespace QtAV;

namespace Digikam
{

class PresentationAudioWidget::Private
{

public:

    Private()
    {
        sharedData  = 0;
        currIndex   = 0;
        mediaObject = 0;
        stopCalled  = false;
        canHide     = true;
        isZeroTime  = false;
    }

    PresentationContainer* sharedData;
    QList<QUrl>            urlList;
    int                    currIndex;
    bool                   stopCalled;
    bool                   isZeroTime;
    bool                   canHide;

    AVPlayer*              mediaObject;
};

PresentationAudioWidget::PresentationAudioWidget(QWidget* const parent, const QList<QUrl>& urls, PresentationContainer* const sharedData)
    : QWidget(parent),
      d(new Private)
{
    setupUi(this);

    d->sharedData = sharedData;

    m_soundLabel->setPixmap(QIcon::fromTheme(QString::fromLatin1("speaker")).pixmap(64, 64));

    m_prevButton->setText(QString::fromLatin1(""));
    m_nextButton->setText(QString::fromLatin1(""));
    m_playButton->setText(QString::fromLatin1(""));
    m_stopButton->setText(QString::fromLatin1(""));

    m_prevButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-skip-backward")));
    m_nextButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-skip-forward")));
    m_playButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-playback-start")));
    m_stopButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-playback-stop")));

    connect(m_prevButton, SIGNAL(clicked()),
            this, SLOT(slotPrev()));

    connect(m_nextButton, SIGNAL(clicked()),
            this, SLOT(slotNext()));

    connect(m_playButton, SIGNAL(clicked()),
            this, SLOT(slotPlay()));

    connect(m_stopButton, SIGNAL(clicked()),
            this, SLOT(slotStop()));

    if (urls.empty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Tracks list is empty...";
        setEnabled(false);
        return;
    }

    // Waiting for files to be enqueued.
    m_playButton->setEnabled(false);
    m_prevButton->setEnabled(false);

    d->mediaObject = new AVPlayer(this);
    d->mediaObject->setNotifyInterval(500);

    connect(d->mediaObject, SIGNAL(mediaStatusChanged(QtAV::MediaStatus)),
            this, SLOT(slotMediaStateChanged(QtAV::MediaStatus)));

    connect(d->mediaObject, SIGNAL(stateChanged(QtAV::AVPlayer::State)),
            this, SLOT(slotPlayerStateChanged(QtAV::AVPlayer::State)));

    connect(d->mediaObject, SIGNAL(error(QtAV::AVError)),
            this, SLOT(slotPlayerError(QtAV::AVError)));

    connect(d->mediaObject, SIGNAL(positionChanged(qint64)),
            this, SLOT(slotTimeUpdaterTimeout()));

    connect(m_volumeWidget, SIGNAL(valueChanged(int)),
            this, SLOT(slotSetVolume(int)));

    enqueue(urls);

    setZeroTime();
}

PresentationAudioWidget::~PresentationAudioWidget()
{
    if (!d->urlList.empty())
    {
        d->mediaObject->stop();
    }

    delete d;
}

void PresentationAudioWidget::slotSetVolume(int v)
{
    if (d->mediaObject->audio())
    {
        d->mediaObject->audio()->setVolume(v);
    }
}

bool PresentationAudioWidget::canHide() const
{
    return d->canHide;
}

bool PresentationAudioWidget::isPaused() const
{
    return d->mediaObject->isPaused();
}

void PresentationAudioWidget::checkSkip()
{
    m_prevButton->setEnabled(true);
    m_nextButton->setEnabled(true);

    if (!d->sharedData->soundtrackLoop)
    {
        if (d->currIndex == 0)
            m_prevButton->setEnabled(false);

        if (d->currIndex == d->urlList.count() - 1)
            m_nextButton->setEnabled(false);
    }
}

void PresentationAudioWidget::setGUIPlay(bool isPlaying)
{
    m_playButton->setIcon(QIcon::fromTheme(isPlaying ? QString::fromLatin1("media-playback-start")
                                                     : QString::fromLatin1("media-playback-pause")));
}

void PresentationAudioWidget::setZeroTime()
{
    QTime zeroTime(0, 0, 0);
    m_elapsedTimeLabel->setText(zeroTime.toString(QString::fromLatin1("H:mm:ss")));
    m_totalTimeLabel->setText(zeroTime.toString(QString::fromLatin1("H:mm:ss")));
    d->isZeroTime = true;
}

void PresentationAudioWidget::enqueue(const QList<QUrl>& urls)
{
    d->urlList   = urls;
    d->currIndex = 0;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Tracks : " << d->urlList;

    if (d->urlList.isEmpty())
        return;

    d->mediaObject->setFile(d->urlList[d->currIndex].toLocalFile());

    m_playButton->setEnabled(true);
}

void PresentationAudioWidget::setPaused(bool val)
{
    if (val == isPaused())
        return;

    slotPlay();
}

void PresentationAudioWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case(Qt::Key_Space):
        {
            m_playButton->animateClick();
            break;
        }

        case(Qt::Key_A):
        {
            if (m_prevButton->isEnabled())
                m_prevButton->animateClick();

            break;
        }

        case(Qt::Key_S):
        {
            if (m_nextButton->isEnabled())
                m_nextButton->animateClick();

            break;
        }

        case(Qt::Key_Escape):
        {
            if (m_stopButton->isEnabled())
                m_stopButton->animateClick();

            break;
        }

        default:
            break;
    }

    event->accept();
}

void PresentationAudioWidget::slotPlay()
{
    if (!d->mediaObject->isPlaying() || d->mediaObject->isPaused())
    {
        if (!d->mediaObject->isPlaying())
        {
            d->mediaObject->play();
        }
        else
        {
            d->mediaObject->pause(false);
        }

        setGUIPlay(false);
        d->canHide = true;
        emit signalPlay();
        return;
    }
    else
    {
        d->mediaObject->pause();
        setGUIPlay(true);
        d->canHide = false;
        emit signalPause();
        return;
    }
}

void PresentationAudioWidget::slotStop()
{
    d->mediaObject->stop();
    d->stopCalled = true;
    d->currIndex  = 0;
    d->mediaObject->setFile(d->urlList[d->currIndex].toLocalFile());
    checkSkip();
    setGUIPlay(false);
    setZeroTime();
}

void PresentationAudioWidget::slotPrev()
{
    d->currIndex--;

    if (d->currIndex < 0)
    {
        if (d->sharedData->soundtrackLoop)
        {
            d->currIndex = d->urlList.count() - 1;
        }
        else
        {
            d->currIndex = 0;
            return;
        }
    }

    d->mediaObject->stop();
    d->mediaObject->setFile(d->urlList[d->currIndex].toLocalFile());
    d->mediaObject->play();
    setZeroTime();
}

void PresentationAudioWidget::slotNext()
{
    d->currIndex++;

    if (d->currIndex >= d->urlList.count())
    {
        if (d->sharedData->soundtrackLoop)
        {
            d->currIndex = 0;
        }
        else
        {
            d->currIndex = d->urlList.count() - 1;
            return;
        }
    }

    d->mediaObject->stop();
    d->mediaObject->setFile(d->urlList[d->currIndex].toLocalFile());
    d->mediaObject->play();
    setZeroTime();
}

void PresentationAudioWidget::slotTimeUpdaterTimeout()
{
    if (d->mediaObject->mediaStatus() == QtAV::UnknownMediaStatus ||
        d->mediaObject->mediaStatus() == QtAV::NoMedia            ||
        d->mediaObject->mediaStatus() == QtAV::StalledMedia       ||
        d->mediaObject->mediaStatus() == QtAV::InvalidMedia)
    {
        slotError();
        return;
    }

    qint64 current = d->mediaObject->position();
    int hours      = (int)(current  / (qint64)(60 * 60 * 1000));
    int mins       = (int)((current / (qint64)(60 * 1000)) - (qint64)(hours * 60));
    int secs       = (int)((current / (qint64)1000) - (qint64)(hours * 60 + mins * 60));
    QTime elapsedTime(hours, mins, secs);

    if (d->isZeroTime && d->mediaObject->duration() > 0)
    {
        d->isZeroTime = false;
        qint64 total  = d->mediaObject->duration();
        hours         = (int)(total  / (qint64)(60 * 60 * 1000));
        mins          = (int)((total / (qint64)(60 * 1000)) - (qint64)(hours * 60));
        secs          = (int)((total / (qint64)1000) - (qint64)(hours * 60 + mins * 60));
        QTime totalTime(hours, mins, secs);
        m_totalTimeLabel->setText(totalTime.toString(QString::fromLatin1("H:mm:ss")));
    }

    m_elapsedTimeLabel->setText(elapsedTime.toString(QString::fromLatin1("H:mm:ss")));
}

void PresentationAudioWidget::slotMediaStateChanged(QtAV::MediaStatus status)
{
    if (status == QtAV::EndOfMedia)
    {
        slotNext();
    }
}

void PresentationAudioWidget::slotPlayerError(const QtAV::AVError& err)
{
    if (err.error() != AVError::NoError)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "An error as occured while playing (" << err.string() << ")";
        slotError();
    }
}

void PresentationAudioWidget::slotPlayerStateChanged(QtAV::AVPlayer::State state)
{
    switch (state)
    {
        case QtAV::AVPlayer::StoppedState:
            m_playButton->setEnabled(true);
            setGUIPlay(true);

            if (d->mediaObject->mediaStatus() == QtAV::LoadingMedia)
            {
                if (d->stopCalled)
                {
                    d->stopCalled = false;
                }
                else
                {
                    slotPlay();
                    checkSkip();
                }
            }
            break;

        case QtAV::AVPlayer::PlayingState:
            setGUIPlay(false);
            checkSkip();
            break;

        default:
            break;
    }
}

void PresentationAudioWidget::slotError()
{
    /* TODO :
     * Display error on slideshow.
     * A QWidget pop-up could help
     */

    slotNext();
}

} // namespace Digikam
