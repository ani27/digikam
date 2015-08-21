/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-12
 * Description : DMetadata Settings Tests for getImageComment and setImageComment.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "commentreadwritetest.h"

// Qt includes

#include <QTest>
#include <QStringList>
#include <QString>

// Local includes

#include "dmetadata.h"

QTEST_MAIN(CommentReadWriteTest)

using namespace Digikam;


void CommentReadWriteTest::initTestCase()
{
    KExiv2::AltLangMap authorsMap, authorsMap2;
    KExiv2::AltLangMap datesMap, datesMap2;
    KExiv2::AltLangMap commentsMap, commentsMap2;
    QString commonAuthor, commonAuthor2;

    authorsMap.insert(QLatin1String("x-default"), QLatin1String("Veaceslav"));
    commentsMap.insert(QLatin1String("x-default"), QLatin1String("Veaceslav's comment"));
    commonAuthor = QLatin1String("Veaceslav");

    commentSet1.setData(commentsMap, authorsMap, commonAuthor, datesMap);

    authorsMap2.insert(QLatin1String("x-default"), QLatin1String("Munteanu"));
    commentsMap2.insert(QLatin1String("x-default"), QLatin1String("Munteanu's comment"));
    commonAuthor2 = QLatin1String("Munteanu");

    commentSet2.setData(commentsMap2, authorsMap2, commonAuthor2, datesMap2);
}


void CommentReadWriteTest::testSimpleReadAfterWrite()
{
    DMetadata dmeta;
    CaptionsMap result;

    // Trick dmetadata, so it will think that we have a file path
    dmeta.setFilePath(QLatin1String("random.org"));

    dmeta.setImageComments(commentSet1);

    result = dmeta.getImageComments();

    QString rezAuthor = result.value(QLatin1String("x-default")).author;
    QString rezComment = result.value(QLatin1String("x-default")).caption;

    QCOMPARE(rezAuthor, commentSet1.value(QLatin1String("x-default")).author);
    QCOMPARE(rezComment, commentSet1.value(QLatin1String("x-default")).caption);

}


void CommentReadWriteTest::testWriteToDisabledNamespaces()
{
    DMetadata dmeta;
    dmeta.setFilePath(QLatin1String("random.org"));

    KExiv2::AltLangMap commentsMap;
    QString commentString;

    DMetadataSettingsContainer dmsettings;

    NamespaceEntry commNs1 (QLatin1String("Xmp.dc.description"),
                            NamespaceEntry::COMMENT,
                            NamespaceEntry::COMMENT_ATLLANGLIST,
                            0);
    commNs1.subspace = NamespaceEntry::XMP;
    commNs1.isDisabled = true;

    NamespaceEntry commNs2 (QLatin1String("Xmp.exif.UserComment"),
                            NamespaceEntry::COMMENT,
                            NamespaceEntry::COMMENT_ALTLANG,
                            1);
    commNs2.subspace = NamespaceEntry::XMP;



    dmsettings.unifyReadWrite = false;
    dmsettings.writeCommentNamespaces.clear();
    dmsettings.writeCommentNamespaces.append(commNs1);
    dmsettings.writeCommentNamespaces.append(commNs2);

    bool rez = dmeta.setImageComments(commentSet1, dmsettings);

    QVERIFY(rez);

    commentsMap = dmeta.getXmpTagStringListLangAlt("Xmp.dc.description", false);

    QCOMPARE(commentsMap.value(QLatin1String("x-default")), QString());

    commentString = dmeta.getXmpTagStringLangAlt("Xmp.exif.UserComment", QString(), false);

    QCOMPARE(commentString, commentSet1.value(QLatin1String("x-default")).caption);


}


void CommentReadWriteTest::testReadFromDisabledNamespaces()
{
    DMetadata dmeta;
    dmeta.setFilePath(QLatin1String("random.org"));

    CaptionsMap rez;

    DMetadataSettingsContainer dmsettings;

    NamespaceEntry commNs1 (QLatin1String("Xmp.dc.description"),
                            NamespaceEntry::COMMENT,
                            NamespaceEntry::COMMENT_ATLLANGLIST,
                            0);
    commNs1.subspace = NamespaceEntry::XMP;
    commNs1.isDisabled = true;

    NamespaceEntry commNs2 (QLatin1String("Xmp.exif.UserComment"),
                            NamespaceEntry::COMMENT,
                            NamespaceEntry::COMMENT_ALTLANG,
                            1);
    commNs2.subspace = NamespaceEntry::XMP;



    dmsettings.unifyReadWrite = false;
    dmsettings.readCommentNamespaces.clear();
    dmsettings.readCommentNamespaces.append(commNs1);
    dmsettings.readCommentNamespaces.append(commNs2);

    dmeta.setXmpTagStringListLangAlt("Xmp.dc.description", commentSet1.toAltLangMap(),false);
    dmeta.setXmpTagStringLangAlt("Xmp.exif.UserComment", commentSet2.value(QLatin1String("x-default")).caption, QString(),false);

    rez = dmeta.getImageComments(dmsettings);
    QCOMPARE(rez.value(QLatin1String("x-default")).caption, commentSet2.value(QLatin1String("x-default")).caption);
}