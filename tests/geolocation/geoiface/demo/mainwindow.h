/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  main-window of the demo application
 *
 * @author Copyright (C) 2009-2010, 2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2014 by Justus Schwartz
 *         <a href="mailto:justus at gmx dot li">justus at gmx dot li</a>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes

#include <QItemSelection>
#include <QMainWindow>

// geoiface includes

#include "geoiface_types.h"
#include "modelhelper.h"
#include "tracks.h"

class QCommandLineParser;

class MarkerModelHelper : public GeoIface::ModelHelper
{
    Q_OBJECT

public:

    MarkerModelHelper(QAbstractItemModel* const itemModel, QItemSelectionModel* const itemSelectionModel);
    ~MarkerModelHelper();

    virtual QAbstractItemModel*  model()          const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual Flags                modelFlags()     const;
    virtual bool itemCoordinates(const QModelIndex& index, GeoIface::GeoCoordinates* const coordinates) const;
    virtual void onIndicesMoved(const QList<QPersistentModelIndex>& movedIndices, const GeoIface::GeoCoordinates& targetCoordinates, const QPersistentModelIndex& targetSnapIndex);

private:

    QAbstractItemModel* const  m_itemModel;
    QItemSelectionModel* const m_itemSelectionModel;

Q_SIGNALS:

    void signalMarkersMoved(const QList<QPersistentModelIndex>& movedIndices);
};

// ------------------------------------------------------------------------------------------------

class MyTrackModelHelper : public QObject
{
    Q_OBJECT

public:

    MyTrackModelHelper(QAbstractItemModel* const imageItemsModel);

    virtual GeoIface::TrackManager::Track::List getTracks() const;

Q_SIGNALS:

    void signalModelChanged();

public Q_SLOTS:

    void slotTrackModelChanged();

private:

    GeoIface::TrackManager::Track::List m_tracks;
    QAbstractItemModel*                m_itemModel;
};

// ------------------------------------------------------------------------------------------------

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QCommandLineParser* const cmdLineArgs, QWidget* const parent = 0);
    ~MainWindow();

public Q_SLOTS:

    void slotScheduleImagesForLoading(const QList<QUrl> imagesToSchedule);

protected:

    void readSettings();
    void saveSettings();
    void closeEvent(QCloseEvent* e);
    void createMenus();

private Q_SLOTS:

    void slotFutureResultsReadyAt(int startIndex, int endIndex);
    void slotImageLoadingBunchReady();
    void slotMarkersMoved(const QList<QPersistentModelIndex>& markerIndices);
    void slotAltitudeRequestsReady(const QList<int>& readyRequests);
    void slotAltitudeLookupDone();
    void slotAddImages();

private:

    class Private;
    Private* const d;
};

#endif /* MAINWINDOW_H */
