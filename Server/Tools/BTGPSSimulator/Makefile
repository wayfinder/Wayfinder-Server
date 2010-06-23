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
# INFO:	Compiles, Builds JAR-file, Creates installation file, zips

JAVASOURCE  = SerialPortConnection.java SenderManager.java CloseableFrame.java ErrorDialog.java CancelDialog.java
JAVASOURCE += HelpDialog.java LogWindow.java MainSimulatorWindow.java 
JAVASOURCE += IsabRouteElement.java IsabRouteList.java Simulator.java
JAVASOURCE += NMEAMessage.java PortDialog.java RepeatDialog.java ImageMercatorMap.java
JAVASOURCE += SlideDialog.java SocketDialog.java SpeedDialog.java SPMercatorMap.java
JAVASOURCE += NMEALoader.java Util.java GPSSender.java Queue.java
JAVASOURCE += GPSSenderListener.java SocketGPSSender.java ParamFile.java FileReadException.java
JAVASOURCE += MapComponent.java OldMapComponent.java FilterDialog.java MercatorMapComponent.java SPDesc.java
JAVASOURCE += ServerComm.java XMLServerComm.java ThreadedServerComm.java
JAVASOURCE += ServerEventQueue.java AWTServerEventQueue.java
JAVASOURCE += ServerRequest.java ServerMercatorRequest.java
JAVASOURCE += ServerEventQueue.java
JAVASOURCE += MercatorMap.java ImageMercatorMap.java
JAVASOURCE += SPMercatorMap.java MercatorMapParams.java

.PHONY: all zip setup clean realclean
all: unpackjar zip
zip: files/Output/BTGPS.zip
setup: files/Output/setup.exe

unpackjar: ./files/comm.jar
	jar xf ./files/comm.jar

#this rule usually just run once due to java dependency checking
%.class: %.java
   # Uncomment to compile using < 1.5
	#javac   -sourcepath . -classpath ./files/comm.jar $^
	javac  -source 1.4 -target 1.4 -sourcepath . -classpath ./files/comm.jar $^

BTGPS.jar: manifest.txt $(subst .java,.class,$(JAVASOURCE))
	jar cmf $< $@ *.class com javax 

files/BTGPS.jar: BTGPS.jar
	cp $< $@

files/param.txt: param.txt
	cp $< $@

files/Readme.txt: Readme.txt
	cp $< $@

files/Output/setup.exe: files/BTGPS.iss files/param.txt files/Readme.txt \
                        files/BTGPS.jar files/BTGPS.bat
	cd files; wine ISCC $(notdir $<)

files/Output/BTGPS.zip: files/Output/setup.exe
	zip -9 $@ $<

clean: 
	rm -vrf *.class files/BTGPS.jar files/param.txt files/Readme.txt
realclean: clean
	rm -vrf files/Output BTGPS.jar

