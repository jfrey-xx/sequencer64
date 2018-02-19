#ifndef SEQ64_QCOLORING_HPP
#define SEQ64_QCOLORING_HPP

/*
 *  This file is part of seq24/sequencer64.
 *
 *  seq24 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  seq24 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with seq24; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * \file          qcoloring.hpp
 *
 *  This module declares/defines items for an abstract representation of the
 *  color of a sequence or panel item.  Colors are, of course, part of using a
 *  GUI, but here we are not tied to a GUI.
 *
 * \library       sequencer64 application
 * \author        Chris Ahlstrom
 * \date          2018-02-18
 * \updates       2018-02-18
 * \license       GNU GPLv2 or above
 *
 *  This module is inspired by MidiPerformance::getSequenceColor() in
 *  Kepler34.
 */

#include <QColor>                       /* Qt 5 QColor class                */
#include <map>                          /* std::map instead of QMap         */

#include "coloring.hpp"                 /* basic color palette enumeration  */

/*
 *  Do not document a namespace; it breaks Doxygen.
 */

namespace seq64
{

/**
 *  Defines a color map.  This map associates the color names with the actual
 *  colors used in the Qt version of the palette.
 */

class qcoloring : public coloring
{

private:

    /**
     *  Provides a typedef for the color map, so that we can more easily shift
     *  between GUI frameworks.
     */

    typedef std::map<thumb_colors_t, QColor> ColorMap;

    /**
     *  Provides the Qt 5 implementation of the color mapping.
     */

    ColorMap m_color_map;

public:

    qcoloring ();

    virtual ~qcoloring ()
    {
        // no code needed as yet
    }

    virtual void add_color (thumb_colors_t c, int r, int g, int b);
    virtual void get_color (thumb_colors_t c, int & r, int & g, int & b);

};          // class qcoloring

}           // namespace seq64

#endif      // SEQ64_QCOLORING_HPP

/*
 * Qcoloring.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

