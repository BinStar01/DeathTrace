# DeathTrace

**Ever wondered how quickly you reacted after dying in Geometry Dash?**

DeathTrace measures the time between your death and your first keyboard input, displaying the result in milliseconds.

### Features

- Measures the first keyboard state change after death.
- Displays input delay in milliseconds (`ms`).
- Supports both key presses and releases.
- Detects input within a 50ms window after death.
- Works with Practice Mode, Start Positions, and Auto Restart.
- Supports Pause and Resume.

### How It Works

When you die, DeathTrace starts monitoring keyboard input. The first detected key state change within 50ms is recorded and displayed on the screen.

The result remains visible until the next attempt begins.

**Note:** Mouse and controller inputs are not currently supported.

### Feedback

Found a bug or have a suggestion? Feel free to report it!