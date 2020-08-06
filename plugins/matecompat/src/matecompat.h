/*
 * Copyright © 2009 Danny Baumann
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Danny Baumann not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Danny Baumann makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DANNY BAUMANN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Danny Baumann <dannybaumann@web.de>
 */

#ifndef _COMPIZ_MATECOMPAT_SRC_MATECOMPAT_H
#define _COMPIZ_MATECOMPAT_SRC_MATECOMPAT_H

#include <core/screen.h>
#include <core/pluginclasshandler.h>

#include "matecompat_options.h"

class MateCompatScreen :
    public PluginClassHandler<MateCompatScreen, CompScreen>,
    public MatecompatOptions
{
    public:
	MateCompatScreen (CompScreen *s);

	void panelAction (CompOption::Vector& options, Atom action);

	Atom panelActionAtom;
	Atom panelMainMenuAtom;
	Atom panelRunDialogAtom;
};

#define MATE_SCREEN(s)                                \
    MateCompatScreen *gs = MateCompatScreen::get (s)

class MateCompatPluginVTable :
    public CompPlugin::VTableForScreen<MateCompatScreen>
{
    public:
	bool init ();
};

#endif /* _COMPIZ_MATECOMPAT_SRC_MATECOMPAT_H */
