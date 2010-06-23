
[Setup]
AppName=BTGPSSimulator
AppVerName=Bluetooth GPS Simulator 1.0
AppPublisher=Publisher
AppPublisherURL=http://publisher.co
AppSupportURL=http://appsupport.co
AppUpdatesURL=http://appupdate.co
DefaultDirName={pf}\BTGPSSimulator
DefaultGroupName=BTGPSSimulator
AllowNoIcons=yes

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Files]
Source: "BTGPS.jar"; DestDir: "{app}"; Flags: ignoreversion
Source: "Readme.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "param.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "win32com.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "javax.comm.properties"; DestDir: "{app}"; Flags: ignoreversion
Source: "BTGPS.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "wflogo.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "routes\*.r"; DestDir: "{app}\routes"; Flags: ignoreversion
Source: "images\*.gif"; DestDir: "{app}\images"; Flags: ignoreversion

[Icons]
Name: "{group}\BTGPSSimulator"; Filename: "{app}\BTGPS.bat"; WorkingDir: {app}; IconFilename: "{app}\wflogo.ico"
Name: "{userdesktop}\BTGPSSimulator"; Filename: "{app}\BTGPS.bat"; WorkingDir: {app}; Tasks: desktopicon; IconFilename: "{app}\wflogo.ico"

[Code]
var
  jreVersion: String;
  jdkVersion: String;
  jreArray:   TArrayOfString;
  jdkArray:   TArrayOfString;
  theLength:  Integer;
  I:          Integer;
  instPath:   String;
  myPath:     String;
  regPath:    String;

function InitializeSetup(): Boolean;
begin
  // Get the current version of JRE and SDK
  RegQueryStringValue(HKLM, 'SOFTWARE\JavaSoft\Java Runtime Environment', 'CurrentVersion', jreVersion);
  RegQueryStringValue(HKLM, 'SOFTWARE\JavaSoft\Java Development Kit', 'CurrentVersion', jdkVersion);

  // Abort if unable to locate JRE nor SDK
  if (jreVersion = '') and (jdkVersion = '') then begin
       MsgBox('Could not locate Java RE or SDK', mbCriticalError, MB_OK);
       Result := False;
  end else begin
    // Collect available install directories
    RegGetSubkeyNames(HKLM, 'SOFTWARE\JavaSoft\Java Runtime Environment', jreArray);
    RegGetSubkeyNames(HKLM, 'SOFTWARE\JavaSoft\Java Development Kit', jdkArray);

    theLength := GetArrayLength(jreArray) - 1;
    for I := 0 to theLength do begin
      regPath := 'SOFTWARE\JavaSoft\Java Runtime Environment\';
      regPath := regPath + jreArray[I];
      RegQueryStringValue(HKLM, regPath, 'JavaHome', myPath);
      jreArray[I] := myPath;
    end;
    
    // We have to add a instance of the path with the JRE subdirectory
    theLength := GetArrayLength(jdkArray);
    SetArrayLength(jdkArray,(theLength*2));
    
    for I := 0 to (theLength - 1) do begin
      regPath := 'SOFTWARE\JavaSoft\Java Development Kit\';
      regPath := regPath + jdkArray[I];
      RegQueryStringValue(HKLM, regPath, 'JavaHome', myPath);
      jdkArray[I] := myPath;
      jdkArray[I + theLength] := myPath + '\jre';
    end;

    Result := True;
  end;

end;

procedure CurStepChanged(CurStep: Integer);
begin

  if (CurStep = csFinished) then begin // Now copy the java comm files to all JRE and SDK directories
    instPath := WizardDirValue();

    theLength := GetArrayLength(jreArray) - 1;
    for I := 0 to theLength do begin
      FileCopy(instPath + '\win32com.dll', jreArray[I] + '\bin\win32com.dll', False)
      FileCopy(instPath + '\javax.comm.properties', jreArray[I] + '\lib\javax.comm.properties', False)
    end;

    theLength := GetArrayLength(jdkArray) - 1;
    for I := 0 to theLength do begin
      FileCopy(instPath + '\win32com.dll', jdkArray[I] + '\bin\win32com.dll', False)
      FileCopy(instPath + '\javax.comm.properties', jdkArray[I] + '\lib\javax.comm.properties', False)
    end;

  end;

end;
