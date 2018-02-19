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
 *  This module declares/defines items for the Qt representation of the
 *  color of a sequence or panel item.
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

#include "qcoloring.hpp"                /* Qt 5 color palette support       */

/*
 *  Do not document a namespace; it breaks Doxygen.
 */

namespace seq64
{

/**
 *  Defines a color map.  This map associates the color names with the actual
 *  colors used in the Qt version of the palette.
 */

qcoloring::qcoloring ()
 :
    coloring    (),
    m_color_map ()
{
    add_color(BLACK, 0, 0, 0);
    add_color(RED, 173, 33, 33);
    add_color(GREEN, 26, 139, 26);
    add_color(BLUE, 28, 72, 112);
    add_color(YELLOW, 169, 173, 33);
    add_color(MAGENTA, 87, 26, 115);
    add_color(CYAN, 0, 96, 96);
    add_color(WHITE, 196, 196, 196);

    // add_color(Pink, QColor(152, 29, 72));
    // add_color(Orange, QColor(173, 115, 33));
}

/**
 *
 */

void
qcoloring::add_color (thumb_colors_t c, int r, int g, int b)
{
    if (c >= BLACK && c < NONE)
        m_color_map[c] = QColor(r, g, b);
}

/**
 *
 */

void
qcoloring::get_color (thumb_colors_t c, int & r, int & g, int & b)
{
    if (c >= BLACK && c < NONE)
    {
    }
    else
    {
        r = g = b = 0;
    }
}

}           // namespace seq64

/*
 * qcoloring.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

