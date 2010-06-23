#
# Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# 
#     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#     * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Set the doumentname for the html-generation

DOCUMENTNAME = design
COMMON = ../Misc/mc2include_defaultvalues.tex ../Misc/mc2include_packages.tex \
../Misc/mc2include_defines.tex ../Misc/mc2_xmldochelp.tex

include ../Makefile.config


# Build all documents (except the html) as standard
all:              xml.pdf xmlsummary.pdf public_xml.pdf \
                  navserver_prot.pdf navserver_route_spec.pdf


# The "rules" to build the xml-api
xml.dvi:          xml.tex externalapi.tex $(COMMON) \
                  ../../Server/bin/isab-mc2.dtd \
                  exampleuser.xml exampleuserstatus.xml \
                  examplesearchandrouterequest.xml \
                  examplesearchandroutereply.xml simpleroute.xml \
                  simplesearch.xml xml_api_favorite.tex \
                  xml_wgs_position_system.tex category_tree_bin_format.tex 
xml.ps:           xml.dvi
xml.pdf:          xml.ps
xml.tex:	  

# The "rules" to build the xml summary
xmlsummary.dvi:   xmlsummary.tex $(COMMON)
xmlsummary.ps:    xmlsummary.dvi
xmlsummary.pdf:   xmlsummary.ps

# The "rules" to build the public xml-api
public_xml.dvi:   public_xml.tex public_api.tex $(COMMON) \
                  ../../Server/bin/public.dtd \
                  xml_api_favorite.tex xml_wgs_position_system.tex
public_xml.ps:           public_xml.dvi
public_xml.pdf:          public_xml.ps
public_xml.tex:

# Rules for navserver prot
navserver_prot.dvi:	navserver_prot.tex category_tree_bin_format.tex \
			serverdataparams.tex serverparams.tex
navserver_prot.ps:	navserver_prot.dvi
navserver_prot.pdf:	navserver_prot.ps

# Rules for navserver route specification.
navserver_route_spec.dvi:  navserver_route_spec.tex
navserver_route_spec.ps:   navserver_route_spec.dvi
navserver_route_spec.pdf:  navserver_route_spec.ps
