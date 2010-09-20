#!/bin/sh
#
# Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#    * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#    * Neither the name of the Vodafone Group Services Ltd nor the names of its
#    contributors may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

if [ "$MC2VER" = "" ]; then
   echo Need MC2VER
   exit
fi

#INSTALL_ROOT=/tmp/mc2-install
BASEDIR=${BASEDIR:-`pwd`}
DESTDIR=${DESTDIR:-/mc2}
if [[ "$BIN" = "" ]]; then
   if [[ -d $BASEDIR/Server/bin-centos4-i386 ]]; then
      BIN=bin-centos4-i386
   fi
   if [[ -d $BASEDIR/Server/bin-centos-i386-el5 ]]; then
      BIN=bin-centos-i386-el5
   fi
   if [[ -d $BASEDIR/Server/bin-centos4-x86_64 ]]; then
      BIN=bin-centos4-x86_64
   fi
fi
echo BASEDIR: $BASEDIR
echo BIN: $BIN
# binaries
MC2BINDIR=$BASEDIR/bin-$MC2VER
MC2BINNONSTRIPDIR=$BASEDIR/bin-non-stripped-$MC2VER

MC2SRCDIR=`pwd`

echo "Installing MC2 in $MC2BINDIR, using $MC2SRCDIR as the source"
echo

echo "  - Creating directories"
for i in $MC2BINDIR $MC2BINNONSTRIPDIR
do
   mkdir -p ${INSTALL_ROOT}/$i
   chmod 755 ${INSTALL_ROOT}/$i
done

echo "  - Installing modules"
for module in EmailModule InfoModule MapModule RouteModule SearchModule SMSModule UserModule GfxModule TileModule ExtServiceModule CommunicationModule
do
   install -s -m755 $MC2SRCDIR/Server/$BIN/$module ${INSTALL_ROOT}/$MC2BINDIR
   install -m755 $MC2SRCDIR/Server/$BIN/$module ${INSTALL_ROOT}/$MC2BINNONSTRIPDIR
done

echo "  - Installing servers"
for server in ModuleTestServer NavigatorServer XMLServer TrafficServer
do
   install -s -m755 $MC2SRCDIR/Server/$BIN/$server ${INSTALL_ROOT}/$MC2BINDIR
   install -m755 $MC2SRCDIR/Server/$BIN/$server ${INSTALL_ROOT}/$MC2BINNONSTRIPDIR
done

echo "  - Installing utilities"
for util in runBin Supervisor
do
   if [[ -x $MC2SRCDIR/Server/bin/$util ]]; then
      install -m755 $MC2SRCDIR/Server/bin/$util ${INSTALL_ROOT}/$MC2BINDIR
   fi
   if [[ -x $MC2SRCDIR/Server/$BIN/$util ]]; then
      install -m755 $MC2SRCDIR/Server/$BIN/$util ${INSTALL_ROOT}/$MC2BINDIR
   fi
done

if [ -x $MC2SRCDIR/Server/Tools/httpdwrap/httpdwrap ]
then
   install -m755 $MC2SRCDIR/Server/Tools/httpdwrap/httpdwrap ${INSTALL_ROOT}/$MC2BINDIR
fi
install -m644 $MC2SRCDIR/Server/Tools/httpdwrap/httpdwrap.c ${INSTALL_ROOT}/$MC2BINDIR

echo "  - Installing scripts"
for script in mc2control mc2controlscreenstarter httpfile
do
   install -m755 $MC2SRCDIR/Server/bin/Scripts/$script ${INSTALL_ROOT}/$MC2BINDIR
done

echo "  - Copying configuration files, etc except mc2.prop"
for f in httpd.pem navclientsettings.txt namedservers.txt poi_category_tree.xml tile_map_colors_v2.xml tile_map_night_colors_v2.xml tile_map_colors.xml tile_map_night_colors.xml poi_scale_ranges.xml
do
   install -m 644  $MC2SRCDIR/Server/bin/$f ${INSTALL_ROOT}/$MC2BINDIR
done
ln -s $DESTDIR/etc/mc2.prop ${INSTALL_ROOT}/$MC2BINDIR/mc2.prop

echo "  - Copying html files etc"
cp -r $MC2SRCDIR/Server/bin/HtmlFiles ${INSTALL_ROOT}/$MC2BINDIR/HtmlFiles
cp -r $MC2SRCDIR/Server/bin/wfcat ${INSTALL_ROOT}/$MC2BINDIR/wfcat

echo "  - Copying files for XMLServer"
mkdir ${INSTALL_ROOT}/$MC2BINDIR/XML
for dtd in isab-mc2.dtd public.dtd
do
   cp $MC2SRCDIR/Server/bin/$dtd ${INSTALL_ROOT}/$MC2BINDIR/XML
   ln -s XML/$dtd ${INSTALL_ROOT}/$MC2BINDIR/$dtd
done

echo "  - Copying other files"
cp -r $MC2SRCDIR/Server/bin/Images ${INSTALL_ROOT}/$MC2BINDIR/Images
cp -r $MC2SRCDIR/Server/bin/Fonts ${INSTALL_ROOT}/$MC2BINDIR/Fonts
cp -r $MC2SRCDIR/Server/bin/MessageTemplate ${INSTALL_ROOT}/$MC2BINDIR/MessageTemplate
cp $MC2SRCDIR/Server/bin/map_generation-mc2.dtd ${INSTALL_ROOT}/$MC2BINDIR
cp $MC2SRCDIR/Server/bin/Scripts/region_ids.xml ${INSTALL_ROOT}/$MC2BINDIR
cp -r $MC2SRCDIR/Server/bin/ConfigFiles ${INSTALL_ROOT}/$MC2BINDIR/ConfigFiles
cp -r $MC2SRCDIR/Server/bin/KeyedData ${INSTALL_ROOT}/$MC2BINDIR/KeyedData

echo "  - Making symlinks, patching scripts, etc"
ln -s /maps/wcimg ${INSTALL_ROOT}/${MC2BINDIR}/HtmlFiles/wcimg
pushd ${INSTALL_ROOT}/${MC2BINDIR}
perl -pi -e "s#^[ ]*configdir=.*#configdir=$DESTDIR/etc#" mc2control
# mc2control symlinks, change and add more depending on your needs
for i in mc2control-modules mc2control-servers
do
   ln -s mc2control $i
done
popd
