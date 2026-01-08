# keyPrefix Installer Documentation

This document provides detailed information about the keyPrefix installer, including how to build it, customize it, and understand its components.

## Overview

The keyPrefix installer is built using [Inno Setup](https://jrsoftware.org/isinfo.php), a free installer for Windows programs. The installer handles:

- Application installation to Program Files
- Interception driver installation
- Start Menu and Desktop shortcut creation
- System restart for driver loading
- Clean uninstallation with driver removal

## Installer Features

### What the Installer Does

1. **Extracts Application Files**
   - Copies `keyPrefix.exe` to `C:\Program Files\keyPrefix\`
   - Copies `install-interception.exe` to the same directory

2. **Installs the Interception Driver**
   - Runs `install-interception.exe /install` silently
   - Requires administrator privileges
   - Driver is installed to Windows system directories

3. **Creates Shortcuts**
   - Start Menu: `All Programs\keyPrefix`
   - Desktop icon (optional, unchecked by default)

4. **Forces System Restart**
   - Required for the kernel-mode driver to load properly
   - User is prompted before restart
   - Option to launch the application is available (though restart will occur)

### Uninstaller Features

1. **Removes Application Files**
   - Deletes all files from the installation directory
   - Removes shortcuts

2. **Uninstalls the Interception Driver**
   - Runs `install-interception.exe /uninstall` silently
   - Cleans up driver files from system directories

3. **Preserves Configuration**
   - User configuration in `%LOCALAPPDATA%\keyPrefix\` is NOT removed
   - Allows clean reinstallation without losing settings

## Building the Installer

### Prerequisites

1. **Install Inno Setup**
   - Download from [https://jrsoftware.org/isdl.php](https://jrsoftware.org/isdl.php)
   - Version 6.0 or higher recommended
   - Install with default options

2. **Build the Application**
   ```bash
   cd keyPrefix
   mkdir cmake-build-release
   cd cmake-build-release
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build . --config Release
   ```

3. **Verify Files**
   Ensure these files exist:
   - `cmake-build-release\keyPrefix.exe`
   - `installer\dependencies\install-interception.exe`

### Build Steps

#### Using Inno Setup GUI

1. Open Inno Setup Compiler
2. Click "File" > "Open"
3. Navigate to `installer\installer.iss`
4. Click "Build" > "Compile"
5. The output will be in `installer\Output\keyPrefix_Setup.exe`

#### Using Command Line

```cmd
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" "installer\installer.iss"
```

The compiled installer will be created at:
```
installer\Output\keyPrefix_Setup.exe
```

## Installer Script Structure

### Script File: `installer.iss`

The script is divided into several sections:

#### 1. Application Information

```pascal
#define MyAppName "keyPrefix"
#define MyAppVersion "1.5"
#define MyAppPublisher "Key"
#define MyAppExeName "keyPrefix.exe"
```

**To update the version**: Change the `MyAppVersion` value before building.

#### 2. Setup Section

```pascal
[Setup]
AppId={{8555E0A7-2102-41B4-B41A-FB086A61D972}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
AlwaysRestart=yes
WizardStyle=modern dark
```

**Key settings**:
- `PrivilegesRequired=admin`: Required for driver installation
- `ArchitecturesAllowed/InstallIn64BitMode=x64compatible`: 64-bit only
- `AlwaysRestart=yes`: Forces restart after installation
- `WizardStyle=modern dark`: Modern dark-themed installer UI

#### 3. Files Section

```pascal
[Files]
Source: "..\cmake-build-release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "dependencies\install-interception.exe"; DestDir: "{app}"; Flags: ignoreversion
```

**Paths are relative to the installer.iss location**.

#### 4. Icons Section

```pascal
[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
```

Creates Start Menu and optional Desktop shortcuts.

#### 5. Run Section

```pascal
[Run]
Filename: "{app}\install-interception.exe"; Parameters: "/install"; StatusMsg: "Installing Interception Driver..."; Flags: waituntilterminated runhidden
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
```

Executes during installation:
1. Installs the Interception driver silently
2. Optionally launches the application (user can uncheck)

#### 6. UninstallRun Section

```pascal
[UninstallRun]
Filename: "{app}\install-interception.exe"; Parameters: "/uninstall"; RunOnceId: "DelInterception"; Flags: waituntilterminated runhidden
```

Uninstalls the driver during uninstallation.

## Customization Guide

### Changing the Version Number

1. Open `installer\installer.iss`
2. Locate: `#define MyAppVersion "1.5"`
3. Change to your new version: `#define MyAppVersion "1.6"`
4. Rebuild the installer

### Changing the Application Name

1. Update `#define MyAppName "keyPrefix"` in `installer.iss`
2. Update `AppId` GUID if creating a different product
3. Update references in README and other documentation

### Changing the Publisher

1. Update `#define MyAppPublisher "Key"` in `installer.iss`
2. Update `AppPublisher` in the `[Setup]` section if needed

### Changing the Installation Directory

Edit the `DefaultDirName` in the `[Setup]` section:
```pascal
DefaultDirName={autopf}\MyCustomFolder
```

### Removing the Forced Restart

**Warning**: The driver will not work until restart!

Change in `[Setup]`:
```pascal
AlwaysRestart=no
```

Add to `[Run]` section instead:
```pascal
Filename: "{sys}\shutdown.exe"; Parameters: "/r /t 0"; Description: "Restart now"; Flags: postinstall nowait skipifsilent runhidden
```

### Adding Additional Files

Add to the `[Files]` section:
```pascal
Source: "path\to\additional\file.dll"; DestDir: "{app}"; Flags: ignoreversion
```

### Changing Installer Theme

In `[Setup]` section:
```pascal
WizardStyle=modern        ; Modern light theme
WizardStyle=classic       ; Classic Windows theme
WizardStyle=modern dark   ; Modern dark theme (current)
```

## Advanced Configuration

### Silent Installation

Users can run the installer silently:
```cmd
keyPrefix_Setup.exe /SILENT /NORESTART
```

- `/SILENT`: No UI, only progress window
- `/VERYSILENT`: No UI at all
- `/NORESTART`: Suppress restart (not recommended)
- `/DIR="C:\Custom\Path"`: Custom installation directory

### Logging

Enable installation logging:
```cmd
keyPrefix_Setup.exe /LOG="C:\install.log"
```

### Unattended Installation

Create a response file for automated deployments:
```cmd
keyPrefix_Setup.exe /SILENT /SUPPRESSMSGBOXES /NORESTART /LOG="install.log"
```

## Interception Driver Details

### Driver Files

The `install-interception.exe` included in the installer:
- Version: Check with vendor documentation
- License: Non-commercial use only
- Source: [https://github.com/oblitum/Interception](https://github.com/oblitum/Interception)

### Driver Installation Process

1. Copies driver files to `C:\Windows\System32\drivers\`
2. Registers the driver with Windows
3. Creates a Windows service for the driver
4. Requires restart for the service to start

### Manual Driver Management

Install manually:
```cmd
install-interception.exe /install
```

Uninstall manually:
```cmd
install-interception.exe /uninstall
```

Check driver status:
```cmd
sc query keyboard
```

## Testing the Installer

### Pre-Release Checklist

- [ ] Build the application in Release mode
- [ ] Update version number in `installer.iss`
- [ ] Verify all source files exist
- [ ] Compile the installer without errors
- [ ] Test installation on a clean Windows VM
- [ ] Verify driver installation works
- [ ] Test application launches correctly after restart
- [ ] Verify all shortcuts are created
- [ ] Test uninstallation removes all files
- [ ] Verify driver is removed after uninstall
- [ ] Check that configuration files are preserved

### Testing in a VM

1. Create a Windows 10/11 VM snapshot
2. Run the installer
3. Allow the restart
4. Launch the application and test functionality
5. Uninstall the application
6. Verify complete removal
7. Restore VM snapshot for next test

## Troubleshooting

### Installer Won't Build

**Error**: "Cannot find file: cmake-build-release\keyPrefix.exe"
- **Solution**: Build the Release version of the application first

**Error**: "Cannot find file: dependencies\install-interception.exe"
- **Solution**: Ensure the Interception driver installer is in the correct location

### Installation Fails

**Error**: "Installation failed" or "Access denied"
- **Solution**: Run the installer as Administrator
- Right-click `keyPrefix_Setup.exe` > "Run as administrator"

**Error**: Driver installation fails
- **Solution**: Check Windows Event Viewer for driver signing issues
- Ensure Test Signing is enabled if using unsigned drivers (not recommended for production)

### Application Won't Start After Install

- Verify the restart was completed
- Check that the driver service is running: `sc query keyboard`
- Reinstall with logging enabled: `keyPrefix_Setup.exe /LOG="install.log"`
- Check the log file for errors

## Distribution

### Preparing for Release

1. **Version Control**
   - Tag the release in Git: `git tag v1.5`
   - Push tags: `git push --tags`

2. **Build Checklist**
   - [ ] Update version in `installer.iss`
   - [ ] Update version in `CMakeLists.txt` if tracked there
   - [ ] Update CHANGELOG.md
   - [ ] Build Release version
   - [ ] Build installer
   - [ ] Test installer on clean system

3. **GitHub Release**
   - Create a new release on GitHub
   - Upload `keyPrefix_Setup.exe`
   - Add release notes
   - Link to documentation

### Installer Naming Convention

Consider including version in filename:
```pascal
OutputBaseFilename=keyPrefix_Setup_v{#MyAppVersion}
```

Result: `keyPrefix_Setup_v1.5.exe`

## File Structure for Building

```
keyPrefix/
├── cmake-build-release/
│   └── keyPrefix.exe           ← Must exist before building installer
├── installer/
│   ├── installer.iss           ← Inno Setup script
│   ├── dependencies/
│   │   └── install-interception.exe  ← Driver installer
│   └── Output/
│       └── keyPrefix_Setup.exe  ← Generated installer
```

## License Considerations

### keyPrefix Application

- Licensed under the **Unlicense** (Public Domain)
- Completely free and open for any use, commercial or non-commercial
- No restrictions, attribution, or obligations
- You can do anything you want with this code

### Interception Driver

- Source: [https://github.com/oblitum/Interception](https://github.com/oblitum/Interception)
- Licensed for non-commercial use only
- Commercial use requires licensing from the author
- Include license terms in distribution

### Inno Setup

- Free for commercial and non-commercial use
- No attribution required
- Consider donating if using commercially

## Support

For installer-related issues:
1. Check this documentation
2. Review Inno Setup documentation: [https://jrsoftware.org/ishelp/](https://jrsoftware.org/ishelp/)
3. Open an issue on the GitHub repository

---

**Last Updated**: January 2026
**Installer Script Version**: 1.5
**Inno Setup Version**: 6.0+
