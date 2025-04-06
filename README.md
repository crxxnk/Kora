# Kora Text Editor

A minimalist and efficient text editor built in C. It is designed for users who prefer simplicity and speed, providing basic functionality without unnecessary bloat.

## Features

- **Minimalist design**: Focuses on text editing with no distractions.
- **Raw input mode**: Allows for direct text manipulation in the terminal.
- **Cursor management**: Customizable cursor visibility for a clean interface.
- **Keyboard shortcuts**: Quick navigation and editing using common keybindings.

## Installation

### Prerequisites

Ensure that you have the folloing installed:
- **GCC** or any other C compiler
- **Make** (optional, for easy compilation)

### Clone the repository
To get started, clone the repository:

```bash
git clone https://github.com/crxxnk/kora.git
cd kora
```

### Build the project

To compile the text editor, run the following command:
```bash
make
```

To remove the compiled object files and the executable, run the following command:
```bash
make clean
```

Alternatively, you can compile it manually with *gcc*:
```bash
gcc kora.c -o editor
```

### Usage

Once compiled, you can run the editor:
```bash
./editor
```

# TODO

This is a list of features and tasks for building the text editor in C.

## Core Functionality

- [x] Enable and disable raw mode (Completed).
- [ ] Handle user input and display text on the screen.
- [ ] Implement the quit functionality
- [ ] Add support for basic text navigation
- [ ] Implement text input and editing

## File Operations

- [ ] Add file opening functionality (open a file and load its contents).
- [ ] Implement file saving

## UI and User Interaction

- [ ] Implement cursor movement.
- [ ] Add language-based syntax highlighting.
- [ ] Implement keybindings for other operations such as copy, paste, undo, etc.
- [ ] Add a status bar (show line number, file size, etc.).

## Testing and Debugging
- [ ] Test on different terminal environments.
- [ ] Fix any bugs with text rendering or input handling.
