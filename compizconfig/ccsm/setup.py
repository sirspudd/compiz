#!/usr/bin/env python3

import sys, os, glob
import subprocess
from stat import *
from distutils.core import setup
from distutils.command.install import install as _install
from distutils.command.build import build as _build
from distutils.command.install_data import install_data as _install_data

INSTALLED_FILES = "installed_files"

class build (_build):

    user_options = _build.user_options[:]
    user_options.extend ([('version=', None, "Version of the package")])

    def initialize_options(self):
        self.version = None
        _build.initialize_options (self)

    def finalize_options(self):
        _build.finalize_options (self)

class install (_install):

    user_options = _install.user_options[:]
    user_options.extend ([('version=', None, "Version of the package")])

    def initialize_options(self):
        self.version = None
        _install.initialize_options (self)

    def finalize_options(self):
        _install.finalize_options (self)

    def run (self):
        _install.run (self)

        icon_prefix = '%s/share/ccsm/icons/hicolor' % prefix
        icon_theme_cache = '%s/icon-theme.cache' % icon_prefix

        print ("icon_prefix ", icon_prefix)
        print ("icon_theme_cache ", icon_theme_cache)

        outputs = self.get_outputs ()

        # Update icon cache
        gtk_update_icon_cache = '''gtk-update-icon-cache -f -t \
%s''' % icon_prefix
        root_specified = len (list (filter (lambda s: s.startswith ("--root"),
                                            sys.argv))) > 0
        if not root_specified:
            print ("Updating Gtk icon cache.")
            os.system (gtk_update_icon_cache)
            outputs.append (icon_theme_cache)
        else:
            print ('''*** Icon cache not updated. After install, run this:
***     %s''' % gtk_update_icon_cache)

        length = 0
        if self.root:
            length += len (self.root)
        if self.prefix:
            length += len (self.prefix)
        if length:
            for counter in range (len (outputs)):
                outputs[counter] = outputs[counter][length:]
        data = "\n".join (outputs)
        try:
            file = open (INSTALLED_FILES, "w")
        except:
            self.warn ("Could not write installed files list %s" % \
                       INSTALLED_FILES)
            return 
        file.write (data)
        file.close ()

class install_data (_install_data):

    def run (self):
        def chmod_data_file (file):
            try:
                os.chmod (file, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
            except:
                self.warn ("Could not chmod data file %s" % file)
        _install_data.run (self)
        map (chmod_data_file, self.get_outputs ())

class uninstall (_install):

    user_options = _install.user_options[:]
    user_options.extend ([('version=', None, "Version of the package")])

    def initialize_options(self):
        self.version = None
        _install.initialize_options (self)

    def finalize_options(self):
        _install.finalize_options (self)

    def run (self):
        try:
            file = open (INSTALLED_FILES, "r")
        except:
            self.warn ("Could not read installed files list %s" % \
                       INSTALLED_FILES)
            return 
        files = file.readlines ()
        file.close ()
        prepend = ""
        if self.root:
            prepend += self.root
        if self.prefix:
            prepend += self.prefix
        if len (prepend):
            for counter in range (len (files)):
                files[counter] = prepend + files[counter].rstrip ()
        for file in files:
            print ("Uninstalling %s" % file)
            try:
                os.unlink (file)
            except:
                self.warn ("Could not remove file %s" % file)

ops = ("install", "build", "sdist", "uninstall", "clean")

if len (sys.argv) < 2 or sys.argv[1] not in ops:
    print ("Please specify operation : %s" % " | ".join (ops))
    raise SystemExit

prefix = None
if len (sys.argv) > 2:
    i = 0
    for o in sys.argv:
        if o.startswith ("--prefix"):
            if o == "--prefix":
                if len (sys.argv) >= i:
                    prefix = sys.argv[i + 1]
                sys.argv.remove (prefix)
            elif o.startswith ("--prefix=") and len (o[9:]):
                prefix = o[9:]
            sys.argv.remove (o)
            break
        i += 1
if not prefix and "PREFIX" in os.environ:
    prefix = os.environ["PREFIX"]
if not prefix or not len (prefix):
    prefix = "/usr/local"

if sys.argv[1] in ("install", "uninstall") and len (prefix):
    sys.argv += ["--prefix", prefix]

for arg in sys.argv:
    if "--version" in arg:
        VERSION = arg.split ("=")[1]

f = open (os.path.join ("ccm/Constants.py.in"), "rt")
data = f.read ()
f.close ()
data = data.replace ("@prefix@", prefix)
data = data.replace ("@version@", VERSION)
f = open (os.path.join ("ccm/Constants.py"), "wt")
f.write (data)
f.close ()

cmd = "intltool-merge -d -u po/ ccsm.desktop.in ccsm.desktop".split(" ")
proc = subprocess.Popen(cmd)
proc.wait()

custom_images = []

data_files = [
                ("share/applications", ["ccsm.desktop"]),
             ]

global_icon_path = "share/icons/hicolor/"
local_icon_path = "share/ccsm/icons/hicolor/"

for dir, subdirs, files in os.walk("images/"):
    if dir == "images/":
        for file in files:
            custom_images.append(dir + file)
    else:
        images = []
        global_images = []

        for file in files:
            if file.find(".svg") or file.find(".png"):
                file_path = "%s/%s" % (dir, file)
                # global image
                if file[:-4] == "ccsm":
                    global_images.append(file_path)
                # local image
                else:
                    images.append(file_path)
        # local
        if len(images) > 0:
            data_files.append((local_icon_path + dir[7:], images))
        # global
        if len(global_images) > 0:
            data_files.append((global_icon_path + dir[7:], global_images))

data_files.append(("share/ccsm/images", custom_images))

podir = os.path.join (os.path.realpath ("."), "po")
if os.path.isdir (podir):
    buildcmd = "msgfmt -o build/locale/%s/ccsm.mo po/%s.po"
    mopath = "build/locale/%s/ccsm.mo"
    destpath = "share/locale/%s/LC_MESSAGES"
    for name in os.listdir (podir):
        if name[-2:] == "po":
            name = name[:-3]
            if sys.argv[1] == "build" \
               or (sys.argv[1] == "install" and \
                   not os.path.exists (mopath % name)):
                if not os.path.isdir ("build/locale/" + name):
                    os.makedirs ("build/locale/" + name)
                os.system (buildcmd % (name, name))
            data_files.append ((destpath % name, [mopath % name]))

setup (
        name             = "ccsm",
        version          = VERSION,
        description      = "CompizConfig Settings Manager",
        author           = "Patrick Niklaus",
        author_email     = "marex@opencompositing.org",
        url              = "https://launchpad.net/compiz",
        license          = "GPL",
        data_files       = data_files,
        packages         = ["ccm"],
        scripts          = ["ccsm"],
        cmdclass         = {"build"     : build,
                            "uninstall" : uninstall,
                            "install" : install,
                            "install_data" : install_data}
     )

os.remove ("ccm/Constants.py")
