/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-07-07
 * @brief  country selector combo-box.
 *
 * @author Copyright (C) 2009-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef COUNTRY_SELECTOR_H
#define COUNTRY_SELECTOR_H

// Qt includes

#include <QtCore/QString>
#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT CountrySelector : public QComboBox
{

public:

    CountrySelector(QWidget* const parent);
    ~CountrySelector();

    void setCountry(const QString& countryCode);
    bool country(QString& countryCode, QString& countryName) const;

    static QString countryForCode(const QString& countryCode);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // COUNTRY_SELECTOR_H
