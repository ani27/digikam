/* ============================================================
 * File  : albumitemhandler.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-09-26
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#ifndef ALBUMITEMHANDLER_H
#define ALBUMITEMHANDLER_H

#include <qstringlist.h>

/*! 
  AlbumItemHandler
  An abstract class which returns the selected items and all items in
  the current album. All views which handle album items should derive
  from this.
*/

class AlbumItemHandler
{
public:

    AlbumItemHandler();
    virtual ~AlbumItemHandler();
    
    virtual QStringList allItems() = 0;
    virtual QStringList selectedItems() = 0;
    virtual QStringList allItemsPath() = 0;
    virtual QStringList selectedItemsPath() = 0;
    virtual void refresh() = 0;
    virtual void refreshItems(const QStringList& items) = 0;

protected:

    void emitItemsSelected(bool val);
};


#endif /* ALBUMITEMHANDLER_H */
