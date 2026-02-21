# Arduino Simulator

A desktop Arduino simulator with visual wiring, code editor, and compilation output, built with Qt 6 and C++.

## Requirements and Installation

### Required tools

- Windows 10/11 or macOS
- Visual Studio 2026/2022/2019 (MSVC) on Windows
- CMake 3.21+
- Ninja
- Qt 6.x and Qt Creator
- GitHub access (for cloning the repository)

### Step-by-step to build and run

1. **[Install Qt 6](docs/INSTALL_QT.md)** — Follow the linked guide to install Qt 6 and Qt Creator.
2. **Clone the repository** — Clone the repo (private)
3. **Open the project in Qt Creator** — In Qt Creator: **File → Open File or Project** → select this project’s `CMakeLists.txt`.
4. **Build and run** — Follow the setup steps, then build and run the application in Qt Creator.

## Project Folder Structure
```
.
├── src/                    # Application source code
│   ├── app/                # UI and main window
│   ├── compiler/           # Build system integration
│   ├── core/               # Logger, ThemeManager, shared utilities
│   ├── scene/              # Graphics items: Arduino board, LED, button, resistor, wires
│   ├── sim/                # Simulation engine (board model, parser, runtime, pins)
│   └── ui/                 # Panels: editor, output, simulator
├── resources/              # Images (board, components) and Qt resources
├── docs/                   # Documentation (INSTALL_QT.md, SIMULATOR.md)   
├── CMakeLists.txt
└── README.md
```

## Simulator

The simulator panel provides a visual Arduino Uno–style breadboard where components can be placed, connected to board pins via wires, and interacted with (move, delete, zoom). It integrates with the rest of the application so that code can be edited, built, and run while viewing the circuit layout. **For detailed documentation (features, UI elements, technical design, and folder structure), see [Simulator documentation](docs/SIMULATOR.md).**

### Features

- **Arduino Uno board** — Draggable board with labeled digital and power pins; wires stay connected when the board is moved.
- **Add components** — The add-component menu (LED, pushbutton, resistor) provides icons for each type; new components are placed at the center-top of the view.
- **Pin mapping and wiring** — Right-click a component to assign Arduino pins (LED: anode/cathode; resistor: leg 1/2; button: pins 1–4). Wires are drawn from board pins to component terminals with distinct colors per component type.
- **Toolbar** — Add component, delete selected items, and zoom in/out; toolbar is overlaid on the simulator view.
- **Layout** — The view fits the scene and re-fits when the board or components are moved or added.

### How to use

1. **Add a component** — Click the add-component button (+), choose LED, Pushbutton, or Resistor from the menu.
2. **Connect to pins** — Right-click the component → **Set pin (anode)** / **Set pin (cathode)** (LED), **Set pin (lead1)** / **Set pin (lead2)** (resistor), or **Set pin (pin1–4)** (button) → select the Arduino pin (e.g. D0–D13, GND).
3. **Move** — Drag the board or any component; wires update automatically.
4. **Remove** — Select one or more components (or the board), then click the delete (trash) button.
5. **Zoom** — Use the zoom menu (⋮) to zoom in or out.


