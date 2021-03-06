# -*- coding: UTF-8 -*-

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# Authors: Quinn Storm (quinn@beryl-project.org)
#          Patrick Niklaus (marex@opencompositing.org)
#          Guillaume Seguin (guillaume@segu.in)
#          Christopher Williams (christopherw@verizon.net)
# Copyright (C) 2007 Quinn Storm

from gi.repository import Gtk

from ccm.Pages import PluginPage, MainPage
from ccm.Constants import DataDir

import locale
import gettext
locale.setlocale(locale.LC_ALL, "")
gettext.bindtextdomain("ccsm", DataDir + "/locale")
gettext.textdomain("ccsm")
_ = gettext.gettext

class MainWin(Gtk.Window):

    currentCategory = None

    def __init__(self, Context, pluginPage=None, categoryName=None):
        Gtk.Window.__init__(self)
        self.ShowingPlugin = None
        self.Context = Context
        self.connect("destroy", self.Quit)
        self.set_default_size(990, 580)
        self.set_title(_("CompizConfig Settings Manager"))
        
        # Build the panes
        self.MainBox = Gtk.HBox()
        self.add(self.MainBox)
        self.LeftPane = Gtk.VBox()
        self.RightPane = Gtk.VBox()
        self.RightPane.set_border_width(5)
        self.MainBox.pack_start(self.LeftPane, False, False, 0)
        self.MainBox.pack_start(self.RightPane, True, True, 0)
        self.MainPage = MainPage(self, self.Context)
        self.CurrentPage = None
        self.SetPage(self.MainPage)

        self.LeftPane.set_size_request(self.LeftPane.size_request().width, -1)
        self.show_all()

        if pluginPage in self.Context.Plugins:
            self.MainPage.ShowPlugin(None, self.Context.Plugins[pluginPage])
        if categoryName in self.Context.Categories:
            self.MainPage.ToggleCategory(None, categoryName)

    def Quit(self, *args):
        Gtk.main_quit()

    def SetPage(self, page):
        if page == self.CurrentPage:
            return

        if page != self.MainPage:
            page.connect('go-back', self.BackToMain)
        
        if self.CurrentPage:
            leftWidget = self.CurrentPage.LeftWidget
            rightWidget = self.CurrentPage.RightWidget
            leftWidget.hide()
            rightWidget.hide()
            self.LeftPane.remove(leftWidget)
            self.RightPane.remove(rightWidget)
            if self.CurrentPage != self.MainPage:
                leftWidget.destroy()
                rightWidget.destroy()

        self.LeftPane.pack_start(page.LeftWidget, True, True, 0)
        self.RightPane.pack_start(page.RightWidget, True, True, 0)
        self.CurrentPage = page
        self.show_all()

    def BackToMain(self, widget):
        self.SetPage(self.MainPage)
        self.MainPage.filterEntry.grab_focus()

    def RefreshPage(self, updatedPlugin):
        currentPage = self.CurrentPage

        if isinstance(currentPage, PluginPage) and currentPage.Plugin:
            for basePlugin in updatedPlugin.GetExtensionBasePlugins ():
                # If updatedPlugin is an extension plugin and a base plugin
                # is currently being displayed, then update its current page
                if currentPage.Plugin.Name == basePlugin.Name:
                    if currentPage.CheckDialogs(basePlugin, self):
                        currentPage.RefreshPage(basePlugin, self)
                    break

Gtk.Window.set_default_icon_name('ccsm')
