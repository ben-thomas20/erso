#pragma once

namespace engine {

// Engine-local key codes.  GLFW constants must never appear above platform/.
enum class Key {
    // Alphabet
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // Digits
    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    // Navigation / modifiers
    Escape, Enter, Tab, Backspace, Delete, Insert,
    Left, Right, Up, Down,
    PageUp, PageDown, Home, End,
    LeftShift, RightShift,
    LeftCtrl, RightCtrl,
    LeftAlt, RightAlt,
    Space,
    Unknown
};

enum class MouseButton {
    Left,
    Right,
    Middle,
    Button4,
    Button5,
    Unknown
};

} // namespace engine
