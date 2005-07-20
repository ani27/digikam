/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-03-09
 * Description :
 *
 * Copyright 2003 by Renchi Raju
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */


#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ktextedit.h>
#include <kdeversion.h>
#include <kconfig.h>
#include <kfilemetainfo.h>

#include <qframe.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qlistview.h>
#include <qguardedptr.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qpushbutton.h>

#include <libkexif/kexifwidget.h>
#include <libkexif/kexifutils.h>
#include <libkexif/kexifdata.h>

#include "albumiconview.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "albumsettings.h"
#include "imagedescedit.h"
#include "tagcreatedlg.h"
#include "syncjob.h"
#include "thumbnailjob.h"

class TAlbumCheckListItem : public QCheckListItem
{
public:

    TAlbumCheckListItem(QListView* parent, TAlbum* album)
        : QCheckListItem(parent, album->title()),
          m_album(album)
        {}

    TAlbumCheckListItem(QCheckListItem* parent, TAlbum* album)
        : QCheckListItem(parent, album->title(), QCheckListItem::CheckBox),
          m_album(album)
        {}

    TAlbum* m_album;
};

ImageDescEdit::ImageDescEdit(AlbumIconView* view, AlbumIconItem* currItem,
                             QWidget *parent, bool singleMode)
             : KDialogBase(Plain, i18n("Image Comments/Tags"),
                           singleMode ? Help|Stretch|Ok|Apply|Cancel :
                           Help|User1|User2|Stretch|Ok|Apply|Cancel,
                           Ok, parent, 0, true, true,
                           KStdGuiItem::guiItem(KStdGuiItem::Forward),
                           KStdGuiItem::guiItem(KStdGuiItem::Back))
{
    setHelp("tagscommentsedit.anchor", "digikam");
    m_view     = view;
    m_currItem = currItem;

    QGridLayout *topLayout = new QGridLayout(plainPage(), 3, 2, 5, spacingHint());

    QGroupBox  *thumbBox = new QGroupBox(plainPage());
    QVBoxLayout *thumbBoxLayout = new QVBoxLayout(thumbBox, marginHint(),
                                                  spacingHint());
    m_thumbLabel = new QLabel(thumbBox);
    m_thumbLabel->setFixedSize(256, 256);
    m_thumbLabel->setScaledContents(false);
    m_thumbLabel->setAlignment(Qt::AlignCenter);
    thumbBoxLayout->addWidget(m_thumbLabel, 0, Qt::AlignCenter);
    m_nameLabel = new QLabel(thumbBox);
    thumbBoxLayout->addWidget(m_nameLabel, 0, Qt::AlignCenter);
    topLayout->addWidget(thumbBox, 0, 0);

    QVGroupBox* commentsBox = new QVGroupBox(i18n("Comments"), plainPage());
    m_commentsEdit = new KTextEdit(commentsBox);
    m_commentsEdit->setTextFormat(QTextEdit::PlainText);
#if KDE_IS_VERSION(3,2,0)
    m_commentsEdit->setCheckSpellingEnabled(true);
#endif
    topLayout->addWidget(commentsBox, 1, 0);

    QVGroupBox* tagsBox = new QVGroupBox(i18n("Tags"), plainPage());
    m_tagsView = new QListView(tagsBox);
    m_recentTagsBtn = new QPushButton(i18n("Recent Tags"), tagsBox);
    topLayout->addMultiCellWidget(tagsBox, 0, 1, 1, 1);

    m_tagsView->addColumn(i18n( "Tags" ));
    m_tagsView->header()->hide();
    m_tagsView->setSelectionMode(QListView::Single);
    m_tagsView->setResizeMode(QListView::LastColumn);
    populateTags();

    connect(m_commentsEdit, SIGNAL(textChanged()),
            SLOT(slotModified()));

    connect(m_tagsView, SIGNAL(rightButtonClicked(QListViewItem*,
            const QPoint &, int)), this,
            SLOT(slotRightButtonClicked(QListViewItem*,
            const QPoint&, int)));

    connect(m_recentTagsBtn, SIGNAL(clicked()),
            SLOT(slotRecentTags()));

    slotItemChanged();

    resize(configDialogSize("Image Description Dialog"));

    m_commentsEdit->installEventFilter(this);
    m_tagsView->installEventFilter(this);

    m_commentsEdit->setFocus();

    AlbumManager* man = AlbumManager::instance();
    connect(man, SIGNAL(signalAlbumAdded(Album*)),
            SLOT(slotAlbumAdded(Album*)));
    connect(man, SIGNAL(signalAlbumDeleted(Album*)),
            SLOT(slotAlbumDeleted(Album*)));
    connect(man, SIGNAL(signalAlbumRenamed(Album*)),
            SLOT(slotAlbumRenamed(Album*)));
    connect(man, SIGNAL(signalAlbumIconChanged(Album*)),
            SLOT(slotAlbumIconChanged(Album*)));

    connect(m_view, SIGNAL(signalItemDeleted(AlbumIconItem*)),
            SLOT(slotItemDeleted(AlbumIconItem*)));
    connect(m_view, SIGNAL(signalCleared()),
            SLOT(slotCleared()));
}

ImageDescEdit::~ImageDescEdit()
{
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        (*it)->removeExtraData(this);
    }

    saveDialogSize("Image Description Dialog");
}

bool ImageDescEdit::eventFilter(QObject *, QEvent *e)
{
    if ( e->type() == QEvent::KeyPress )
    {
        QKeyEvent *k = (QKeyEvent *)e;
        if (k->state() == Qt::ControlButton &&
            (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            slotApply();
            AlbumIconItem* item =
                dynamic_cast<AlbumIconItem*>(m_currItem->nextItem());
            if (item)
            {
                slotUser1();
            }
            return true;
        }
        else if (k->state() == Qt::ShiftButton &&
                 (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            slotApply();
            AlbumIconItem* item =
                dynamic_cast<AlbumIconItem*>(m_currItem->prevItem());
            if (item)
            {
                slotUser2();
            }
            return true;
        }

        return false;
    }
    return false;
}

void ImageDescEdit::populateTags()
{
    m_tagsView->clear();

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* tag  = (TAlbum*)(*it);

        TAlbumCheckListItem* viewItem = 0;
        
        if (tag->isRoot())
        {
            viewItem = new TAlbumCheckListItem(m_tagsView, tag);
        }
        else
        {
            QCheckListItem* parentItem  =
                (QCheckListItem*)(tag->parent()->extraData(this));
            if (!parentItem)
            {
                kdWarning() << "Failed to find parent for Tag " << tag->title()
                            << endl;
                continue;
            }

            viewItem = new TAlbumCheckListItem(parentItem, tag);
        }

        if (viewItem)
        {
            viewItem->setOpen(true);
            viewItem->setPixmap(0, tagThumbnail(tag));
            tag->setExtraData(this, viewItem);
        }
    }
}

void ImageDescEdit::slotModified()
{
    m_modified = true;
}

void ImageDescEdit::slotUser1()
{
    if (!m_currItem)
        return;

    // Save the data
    slotApply();

    m_currItem = dynamic_cast<AlbumIconItem*>(m_currItem->nextItem());
    m_currItem->setSelected(true);
    m_view->ensureItemVisible(m_currItem);

    slotItemChanged();
}

void ImageDescEdit::slotUser2()
{
    if (!m_currItem)
        return;

    // Save the data
    slotApply();

    m_currItem = dynamic_cast<AlbumIconItem*>(m_currItem->prevItem());
    m_currItem->setSelected(true);
    m_view->ensureItemVisible(m_currItem);

    slotItemChanged();
}

void ImageDescEdit::slotApply()
{
    if (!m_currItem)
        return;

    ImageInfo* info = m_currItem->imageInfo();

    if (m_modified)
    {
        info->setCaption(m_commentsEdit->text());

        if (AlbumSettings::instance() &&
            AlbumSettings::instance()->getSaveExifComments())
        {
            // store as JPEG Exif comment
            KFileMetaInfo  metaInfo(info->filePath(), "image/jpeg", KFileMetaInfo::Fastest);

            // set Jpeg comment
            if (metaInfo.isValid () && metaInfo.mimeType() == "image/jpeg"
                && metaInfo.containsGroup("Jpeg EXIF Data"))
            {
                kdDebug() << k_funcinfo << "Contains JPEG Exif data, setting comment"
                          << endl;
                metaInfo["Jpeg EXIF Data"].item("Comment")
                    .setValue(m_commentsEdit->text());
                metaInfo.applyChanges();
            }
        }

        m_modified = false;
    }

    info->removeAllTags();
    QListViewItemIterator it(m_tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem = dynamic_cast<TAlbumCheckListItem*>(it.current());
        if (tItem && tItem->isOn())
        {
            info->setTag(tItem->m_album->id());
        }
        ++it;
    }
}

void ImageDescEdit::slotOk()
{
    slotApply();
    KDialogBase::slotOk();
}

void ImageDescEdit::slotItemChanged()
{
    if (!m_currItem)
        return;

    m_modified = false;

    if (!m_thumbJob.isNull())
    {
        m_thumbJob->kill();
    }

    if (!m_thumbJob.isNull())
    {
        delete m_thumbJob;
    }

    ImageInfo* info = m_currItem->imageInfo();
    KURL fileURL;
    fileURL.setPath(info->filePath());

    m_thumbJob = new ThumbnailJob(fileURL, 256);

    connect(m_thumbJob,
            SIGNAL(signalThumbnail(const KURL&,
                                   const QPixmap&)),
            SLOT(slotGotThumbnail(const KURL&,
                                  const QPixmap&)));

    connect(m_thumbJob,
            SIGNAL(signalFailed(const KURL&)),
            SLOT(slotFailedThumbnail(const KURL&)));

    PAlbum *album = m_currItem->imageInfo()->album();
    if (!album)
    {
        kdWarning() << "Failed to find parent album for"
                    << fileURL << endl;
        return;
    }

    m_nameLabel->setText(info->name());
    m_thumbLabel->setPixmap(QPixmap());
    m_commentsEdit->setText(info->caption());

    QValueList<int> tagIDs = info->tagIDs();

    QListViewItemIterator it( m_tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem =
            dynamic_cast<TAlbumCheckListItem*>(it.current());

        if (tItem)
        {
            if (tagIDs.contains(tItem->m_album->id()))
                tItem->setOn(true);
            else
                tItem->setOn(false);
        }
        ++it;
    }

    enableButton(User1, m_currItem->nextItem() != 0);
    enableButton(User2, m_currItem->prevItem() != 0);
}

void ImageDescEdit::slotGotThumbnail(const KURL&, const QPixmap& pix)
{
    m_thumbLabel->setPixmap(pix);
}

void ImageDescEdit::slotFailedThumbnail(const KURL&)
{
    m_thumbLabel->clear();
    m_thumbLabel->setText(i18n("Thumbnail unavailable"));
}

void ImageDescEdit::slotRightButtonClicked(QListViewItem *item,
                                           const QPoint &, int )
{
    TAlbum              *album;

    if (!item)
    {
        album = AlbumManager::instance()->findTAlbum(0);
    }
    else
    {
        TAlbumCheckListItem* viewItem = dynamic_cast<TAlbumCheckListItem*>(item);

        if(!viewItem)
            album = AlbumManager::instance()->findTAlbum(0);
        else
            album = viewItem->m_album;
    }

    if(!album)
        return;

    QPopupMenu popmenu(this);

    popmenu.insertItem(SmallIcon("tag"),
                       i18n("New Tag..."), 10);
    if (!album->isRoot())
    {
        popmenu.insertItem(SmallIcon("pencil"),
                           i18n("Edit Tag Properties..."), 11);
        popmenu.insertItem(SmallIcon("edittrash"),
                           i18n("Delete Tag"), 12);
    }

    switch (popmenu.exec(QCursor::pos()))
    {
    case 10:
    {
        tagNew(album);
        break;
    }
    case 11:
    {
        if (!album->isRoot())
            tagEdit(album);
        break;
    }
    case 12:
    {
        if (!album->isRoot())
            tagDelete(album);
        break;
    }
    default:
        break;
    }
}

void ImageDescEdit::tagNew(TAlbum* parAlbum)
{
    if (!parAlbum)
        return;

    QString title, icon;
    AlbumManager *albumMan_ = AlbumManager::instance();

    if (!TagCreateDlg::tagCreate(parAlbum, title, icon))
        return;

    QString errMsg;
    TAlbum* album = albumMan_->createTAlbum(parAlbum, title, icon, errMsg);
    
    if (!album)
    {
        KMessageBox::error(this, errMsg);
    }
    else
    {
        TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)album->extraData(this);
        if (viewItem)
        {
            viewItem->setOn(true);
            m_tagsView->setSelected(viewItem, true);
            m_tagsView->ensureItemVisible(viewItem);
        }
    }
}

void ImageDescEdit::tagDelete(TAlbum *album)
{
    if (!album || album->isRoot())
        return;

    AlbumManager *albumMan = AlbumManager::instance();

    if (album == albumMan->currentAlbum() ||
        album->isAncestorOf(albumMan->currentAlbum()))
    {
        KMessageBox::error(this, i18n("You are currently viewing items in the "
                                      "tag '%1' that you are about to delete? "
                                      "You will need to close this dialog first "
                                      "if you want to delete the tag" )
                           .arg(album->title()));
        return;
    }

    int result =
        KMessageBox::questionYesNo(this, i18n("Delete '%1' tag?")
                                   .arg(album->title()));

    if (result == KMessageBox::Yes)
    {
        QString errMsg;
        if (!albumMan->deleteTAlbum(album, errMsg))
            KMessageBox::error(this, errMsg);
    }
}


void ImageDescEdit::tagEdit(TAlbum* album)
{
    if (!album || album->isRoot())
        return;

    QString title;
    QString icon;
    
    if (!TagEditDlg::tagEdit(album, title, icon))
        return;

    
    AlbumManager *albumMan = AlbumManager::instance();
    if (album->title() != title)
    {
        QString errMsg;
        if (!albumMan->renameTAlbum(album, title, errMsg))
        {
            KMessageBox::error(this, errMsg);
            return;
        }
    }

    if (album->icon() != icon)
    {
        QString errMsg;
        if (!albumMan->updateTAlbumIcon(album, icon, 0, errMsg));
        {
            KMessageBox::error(this, errMsg);
        }
    }
}

void ImageDescEdit::slotAlbumAdded(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;
    
    QCheckListItem* parentItem  =
        (QCheckListItem*)(album->parent()->extraData(this));

    if (!parentItem)
    {
        kdWarning() << "Failed to find parent for Tag " << album->title()
                    << endl;
        return;
    }
        
    TAlbumCheckListItem* viewItem = new TAlbumCheckListItem(parentItem, album);
    viewItem->setOpen(true);
    viewItem->setPixmap(0, tagThumbnail(album));
    album->setExtraData(this, viewItem);
}

void ImageDescEdit::slotAlbumDeleted(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;

    QCheckListItem* viewItem = (QCheckListItem*)(album->extraData(this));
    delete viewItem;
    album->removeExtraData(this);
}

void ImageDescEdit::slotAlbumIconChanged(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;
    
    QCheckListItem* viewItem = (QCheckListItem*)(album->extraData(this));
    if (!viewItem)
    {
        kdWarning() << "Failed to find view item for Tag "
                    << album->title() << endl;
        return;
    }

    viewItem->setPixmap(0, tagThumbnail(album));
}

void ImageDescEdit::slotAlbumRenamed(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;
    
    TAlbum* album = (TAlbum*)a;

    QCheckListItem* viewItem = (QCheckListItem*)(album->extraData(this));
    if (!viewItem)
    {
        kdWarning() << "Failed to find view item for Tag "
                    << album->title() << endl;
        return;
    }

    viewItem->setText(0, album->title());
}

QPixmap ImageDescEdit::tagThumbnail(TAlbum* album) const
{
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();

    QPixmap pix;
    
    if (!album->isRoot())
        pix = SyncJob::getTagThumbnail(album->icon(), 20);
    else
        pix = iconLoader->loadIcon("tag-folder", KIcon::NoGroup, 20,
                                   KIcon::DefaultState, 0, true);

    return pix;
}

void ImageDescEdit::slotItemDeleted(AlbumIconItem* iconItem)
{
    if (m_currItem != iconItem)
        return;

    // uh oh. our current item got deleted. close
    m_currItem = 0;
    close();
}

void ImageDescEdit::slotCleared()
{
    // if the iconview has been cleared, bail out and close
    close();
}

void ImageDescEdit::slotRecentTags()
{
    QPopupMenu menu(this);

    AlbumManager* albumMan = AlbumManager::instance();
    IntList recentTags = albumMan->albumDB()->getRecentlyAssignedTags();

    if (recentTags.isEmpty())
    {
        menu.insertItem(i18n("No recently assigned tags"), 0);
        menu.setItemEnabled(0, false);
    }
    else
    {
        for (IntList::const_iterator it = recentTags.begin();
             it != recentTags.end(); ++it)
        {
            TAlbum* album = albumMan->findTAlbum(*it);
            if (album)
            {
                QPixmap pix = SyncJob::getTagThumbnail(album->icon(),
                                                       KIcon::SizeSmall);
                menu.insertItem(pix, album->url().remove(0,1), album->id());
            }
        }
    }

    int id = menu.exec(QCursor::pos());

    if (id > 0)
    {
        TAlbum* album = albumMan->findTAlbum(id);
        if (album)
        {
            TAlbumCheckListItem* viewItem =
                (TAlbumCheckListItem*)album->extraData(this);
            if (viewItem)
            {
                viewItem->setOn(true);
                m_tagsView->setSelected(viewItem, true);
                m_tagsView->ensureItemVisible(viewItem);
            }
        }
    }
}

#include "imagedescedit.moc"
