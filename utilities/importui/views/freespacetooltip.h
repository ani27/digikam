/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-01-19
 * Description : free space widget tool tip
 *
 * Copyright (C) 2009-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FREESPACETOOLTIP_H
#define FREESPACETOOLTIP_H

// Local includes

#include "ditemtooltip.h"

namespace Digikam
{

class FreeSpaceToolTip : public DItemToolTip
{
public:

    explicit FreeSpaceToolTip(QWidget* const parent);
    virtual ~FreeSpaceToolTip();

    void setToolTip(const QString& tip);

    void show();

protected:

    virtual QRect   repositionRect();
    virtual QString tipContents();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* FREESPACETOOLTIP_H */
