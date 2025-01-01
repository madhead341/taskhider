# Task Hider Injector Tool

A command-line Python tool for injecting a custom DLL into `taskmgr.exe` (or any process), enabling you to manipulate processes or hide them from Task Manager.

## Features
- Injects a DLL into `taskmgr.exe` or any process.
- Uses hardcoded DLL and injector byte arrays (no external files).
- Temporary files are stored in the system’s `TEMP` folder and deleted after use.

## Requirements
- Python 3.x

## Setup

1. Clone the repo:
    ```bash
    git clone https://github.com/madhead341/taskhider.git
    cd taskhider
    ```

2. Modify the script to include your DLL and injector byte arrays.

3. Run the script:
    ```bash
    python loader.py
    ```

## Usage
- The script waits for `taskmgr.exe` to appear, injects the DLL, and exits. This is just a launcher.

## Notes
- For educational use only.
- Ensure compliance with applicable laws and terms of service.
- The DLL should be customized for the target process.

## License
MIT License. See the LICENSE file for details.
