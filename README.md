# Task Hider - DLL Injector

A C++ tool that hides specific processes from the system process list by hooking into `NtQuerySystemInformation`. This allows processes to be hidden from the user and surveillance programs.

## Features

- **Hide Processes**: Hide up to 5 processes at a time.
- **Dynamic Hooking**: Hooks into `NtQuerySystemInformation` to modify process enumeration.
- **Unhooking**: Reverts changes and unhooks the function when exiting.

## Requirements

- **Visual Studio or MinGW**: For compiling the code.
- **C++17**: The project uses C++17.

## Setup Instructions

1. **Clone the repository**:
    ```bash
    git clone https://github.com/madhead341/taskhider.git
    cd taskhider
    ```

2. **Compile the DLL**:
    - Use Visual Studio or MinGW to compile the code and generate `TaskManagerHack.dll`.

3. **Inject the DLL**:
    - Inject the `TaskManagerHack.dll` into the target process (`taskmgr.exe`) using a DLL injection tool or the python launcher.

## How It Works

- The tool hooks into `NtQuerySystemInformation` and modifies the system's process enumeration to hide selected processes.
- Processes to hide are stored in a list and can be added or removed via the interactive console menu.

## Usage

After injection, use the console menu to:

- **[1] Hide Process**: Add a process to the hidden list (max 5).
- **[2] View Hidden Processes**: View and unhide processes.
- **[H] Hide Console**: Hide the console window.
- **[X] Close**: Unhook the function and close the application.

## Important Notes

- **Ethical Use**: This tool is intended for educational purposes. Use responsibly.

## License

MIT License - see the [LICENSE](LICENSE) file for details.
