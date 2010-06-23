#-------------------------------------------------------------------------------
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
#-------------------------------------------------------------------------------


To install BTGPSSimulator
==========================
Check-out the latest version and run the make file:

[Server/Tools/BTGPSSimulator]$make

This will compile the simulator and create a setup file, 
which will appear in:

Server/Tools/BTGPSSimulator/files/Output/setup.exe

Run the setup (in windows).


To use the simulator
========================

Selecting Bluetooth Serial Port
---------------------------------
Select a free COM port for the Simulator.
In the main simulator window ["BTGPSSimulator"] choose :

Connect->Select Port from List

and choose among the available ports. The value is saved in a parameter file
for automatic use the next time the program is started.

To find a suitable COM port goto "My Bluetooth Places"
Right Click "My Device" and choose "Properties".
Under the folder "Local Services" you can find the "Bluetooth Serial Port".

Make sure that the port is not used by an other application (like PC suite).



Parameter file.
----------------

In the parameter file, param.txt, a key word is followed by a " " 
and then a value.

Keywords.
---------
DEFAULT_SEND_TIME_INTERVAL - how often to send positions.
LOADDIR - where to find route files.
LOADFILE - a route file.
LOG_DIR -
LOG_FIL -
NMEADIR -
NMEAFILE -
PORT - the serial port that is used for sending positions to a listener, a "null" value means no auto start.
READPORT -
REPEAT_MODE - if a route should stop at end or repeat.
SERVER - where to get maps and routes.
SOCKETHOST - a host to use for a socket connection (connect to)
SOCKETPORT - a port to use for a socket connection (connect to/incoming connection)
STARTX - the longitude of the first map center point. (mc2 coord)
STARTY - the latgitude of the first map center point. (mc2 coord)
XML_LOGIN - The user login for XML API access to the host in SERVER parameter.
XML_PASSWORD - The user password for XML API access to the host in SERVER parameter.

No characters are allowed before the keyword, which can be used to 
out comment a value:

xPORT COM5
PORT COM4

If the parameter file is missing a new one will be created with default values.


For run time help see the help menu in the simulator.
