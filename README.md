# keyPrefix

A Windows application that adds customizable prefix and suffix text to keyboard input from a selected keyboard device. Perfect for multi-keyboard setups, barcode scanners, or any scenario where you need to automatically wrap input from a specific keyboard with predefined text.

<img src="keyprefixLogo.png" alt="keyPrefix logo">

## Features

- **Selective Keyboard Hooking**: Choose which keyboard device to monitor from a list of connected keyboards
- **Prefix & Suffix Text**: Automatically add custom text before and after input from the selected keyboard
- **Low-Level Input Control**: Uses the Interception driver for reliable, system-wide keyboard hooking
- **ImGui Interface**: Clean, responsive GUI built with ImGui and DirectX 11
- **System Tray Integration**: Minimize to tray, start/stop hooking, and quick access to controls
- **Run on Startup**: Optional Windows Task Scheduler integration to launch on system boot
- **Device Persistence**: Remembers your selected keyboard by VID/PID across sessions
- **Configurable Timing**: Adjust idle timeout and timer tick intervals for optimal performance
- **DPI Aware**: Crisp rendering on high-DPI displays

## Use Cases

- **Barcode Scanner Integration**: Automatically format barcode scanner input with prefixes/suffixes
- **Multi-Keyboard Workflows**: Differentiate input from multiple keyboards for data entry tasks
- **Automated Data Entry**: Add formatting or delimiters to specific keyboard input
- **Testing & Development**: Simulate formatted input for application testing

## System Requirements

- **OS**: Windows 10/11 (64-bit)
- **Privileges**: Administrator rights (required for Interception driver)
- **Dependencies**: Interception driver (included in installer)

## Installation

### Using the Installer (Recommended)

1. Download the latest `keyPrefix_Setup.exe` from the [Releases](https://github.com/Amrkurdi202/keyPrefix/releases) page
2. Run the installer as Administrator
3. The installer will:
   - Install the application to `Program Files\keyPrefix`
   - Install the Interception driver
   - Create Start Menu shortcuts
   - Optionally create a desktop icon
4. **Important**: The installer requires a system restart to load the driver

### Building from Source

#### Prerequisites

- CMake 3.20 or higher
- Visual Studio 2019 or newer (with C++ desktop development workload)
- [Dear ImGui](https://github.com/ocornut/imgui) library
- Interception driver SDK

#### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/Amrkurdi202/keyPrefix.git
   cd keyPrefix
   ```

2. Download [Dear ImGui](https://github.com/ocornut/imgui) and extract it to `C:\dev\imgui` (or update `CMakeLists.txt` with your path)

3. Configure and build:
   ```bash
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build . --config Release
   ```

4. The executable will be in `build\Release\keyPrefix.exe`

5. Install the Interception driver manually:
   ```bash
   installer\dependencies\install-interception.exe /install
   ```

## Usage

### First Launch

1. Run `keyPrefix.exe` as Administrator (required for driver access)
2. The main window displays a list of detected keyboards
3. Select your target keyboard from the list (e.g., a barcode scanner)
4. Configure prefix and suffix text in the text boxes
5. Enable "Hook enabled" to start monitoring

### Configuration Options

- **Hook enabled**: Enable/disable keyboard monitoring
- **Idle timeout ms**: Time to wait for inactivity before processing input (10-2000ms)
- **Timer tick ms**: Polling interval for the hook thread (1-50ms)
- **Run on startup**: Creates a Windows Task Scheduler task to auto-start minimized
- **Prefix/Suffix**: Multi-line text fields supporting Unicode input

### System Tray

Right-click the tray icon for quick actions:
- Show/Hide main window
- Start/Stop hook
- Exit application

### Command Line

- `/minimized` - Start the application minimized to tray

Example:
```cmd
keyPrefix.exe /minimized
```

## Configuration File

Settings are saved to `%LOCALAPPDATA%\keyPrefix\config.txt` including:
- Selected keyboard VID/PID
- Prefix and suffix text
- Timing parameters
- Hook enable state

## How It Works

1. **Interception Driver**: Installs a kernel-mode driver that intercepts keyboard input at the lowest level
2. **Device Selection**: Enumerates connected keyboards by hardware ID and allows selection
3. **Input Monitoring**: Hooks keyboard events from the selected device only
4. **Text Injection**: When input is detected:
   - Sends prefix text (if configured)
   - Allows original input to pass through
   - Sends suffix text (if configured)
5. **Idle Detection**: Uses configurable timeout to determine when input sequence is complete

## Project Structure

```
keyPrefix/
├── main.cpp                 # Application entry point and main loop
├── app_logic.cpp/h          # Core application logic and hook thread
├── interception_logic.cpp/h # Interception driver interface and key sending
├── config.cpp/h             # Configuration file management
├── window_manager.cpp/h     # Window creation and message handling
├── directx_renderer.cpp/h   # DirectX 11 rendering setup
├── imgui_helpers.cpp/h      # ImGui utility functions
├── startup_manager.cpp/h    # Windows Task Scheduler integration
├── string_utils.cpp/h       # UTF-8/UTF-16 string conversion
├── globals.cpp/h            # Global state and shared variables
├── keyboard_info.h          # Keyboard device info structures
├── interception.c/h         # Interception driver API
├── resource.rc/h            # Application icon and resources
├── CMakeLists.txt           # Build configuration
└── installer/
    ├── installer.iss        # Inno Setup installer script
    └── dependencies/
        └── install-interception.exe  # Interception driver installer
```

## Building the Installer

1. Install [Inno Setup](https://jrsoftware.org/isinfo.php)
2. Build the Release version of keyPrefix
3. Open `installer\installer.iss` in Inno Setup
4. Click "Compile" to generate `installer\Output\keyPrefix_Setup.exe`

## Troubleshooting

### Driver Not Working
- Ensure you've restarted your computer after installation
- Check that the Interception driver is installed: Run `install-interception.exe /install` as Administrator
- Verify the service is running: `sc query keyboard` in Command Prompt

### Keyboard Not Detected
- Try unplugging and replugging the keyboard
- Check Device Manager for any driver issues
- Some USB keyboards may need a USB hub to be properly detected

### Application Won't Start
- Run as Administrator
- Check Windows Event Viewer for error messages
- Ensure DirectX 11 is available on your system

### Hook Not Working
- Make sure "Hook enabled" is checked
- Verify you've selected a keyboard from the list
- Try adjusting the idle timeout value
- Check that no other keyboard hook software is running

## Uninstallation

1. Open "Add or Remove Programs" in Windows Settings
2. Find "keyPrefix" and click Uninstall
3. The uninstaller will automatically remove the Interception driver
4. Restart your computer to complete driver removal

## Security & Privacy

- This application requires Administrator privileges due to the kernel-mode Interception driver
- It only monitors the selected keyboard device, not all input
- No data is transmitted over the network
- Configuration is stored locally only
- The Interception driver is open-source and widely used in the community

## License

This project is released under the **Unlicense** - it is completely free and open for any use, commercial or non-commercial, with no restrictions whatsoever. You can do anything you want with this code.

This project uses the following components:
- **Interception**: [Interception Driver](https://github.com/oblitum/Interception) (Non-commercial use) - Low-level keyboard input driver
- **Dear ImGui**: MIT License
- **Project Code**: Unlicense (Public Domain)

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

### Development Setup

1. Follow the "Building from Source" instructions
2. Open the project in CLion or Visual Studio
3. Make your changes
4. Test thoroughly with different keyboard configurations
5. Submit a pull request

## Credits

- [Interception](https://github.com/oblitum/Interception) by Francisco Lopes - Low-level input driver used for keyboard hooking
- [Dear ImGui](https://github.com/ocornut/imgui) by Omar Cornut
- DirectX 11 rendering implementation

## Changelog

### Version 1.5 (Current)
- Enhanced keyboard device selection
- Improved prefix/suffix text handling
- Configurable timing parameters
- System tray integration
- Startup task support
- DPI awareness improvements

## Support

For issues, questions, or feature requests, please open an issue on the [GitHub Issues](https://github.com/Amrkurdi202/keyPrefix/issues) page.

---

**Note**: This software is provided as-is. The Interception driver requires administrator privileges and operates at the kernel level. Use at your own risk.
