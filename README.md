# HKL

### Introduction

This app was created to easily open applications using simple key combinations.

## Config

The config file is named `hkl.json` and should be placed in the root directory. It uses the following format:

```json
{
  "quit": "ctrl+alt+q",
  "items": [
    {
      "exe": "C:\\example.exe",
      "bind": "ctrl+=",
      "args": "",
      "dir": "",
      "mutex": ""
    }
  ]
}
```

### Parameters Description

* **`items`** — A list of programs bound to shortcuts for quick launching.
* **`exe`** — Path to the executable file (`.exe` or any other executable format).
* **`bind`** — Key combination used to trigger the app.
* **`args`** — Optional command-line arguments to run the program with.
* **`dir`** — Working directory (HOMEDIR) for the running program.
* **`mutex`** — Mutex name of the program (used to handle single-instance applications).
* **`quit`** — Global hotkey to exit the HKL utility; you can change this at any time.
