# Panic loop: Morse on Caps Lock

`kernel_halt()` is the kernel's terminal state. It never returns. It sends this message,
repeating:

    HELP ME I HAVE CRASHED AND I CANNOT GET UP

by blinking the Caps Lock LED.

## Timing
- One time unit is 60 ms (20 words per minute). A dit is 60 ms on, a dah 180 ms on.
- Dit 1 unit on, dah 3 units on. 1 unit off between symbols, 3 between letters, 7 between words.
- One repetition is 330 units, about 20 s, with 84 signal symbols. A 28-unit gap (about 1.7 s)
  separates repetitions.

## Hardware
- Caps Lock is set through the PS/2 keyboard controller (port 0x60, command 0xED, then a bit
  mask; bit 2 is Caps Lock). Each write waits for the 0xFA acknowledge, with a spin limit.
- If there is no PS/2 keyboard (USB-only machines), `kbd_led_set` returns -1 and the loop
  keeps its timing silently. The message is then not visible.
- Delays come from PIT channel 2, polled through port 0x61. No interrupts are used, so the
  panic loop works with interrupts disabled.

## Units
- `kernel/morse_encode`: text to on/off events (host tested)
- `kernel/morse_play`: plays events forever
- `kernel/delay_pit`: millisecond delay
- `console/kbd_led`: Caps Lock LED
- `kernel/port_io`: inb and outb
- `kernel/halt`: the panic loop itself

## Limits
- The Morse output is only as good as the LED path. It is untested on hardware. The encoder
  is host tested. The LED and PIT paths are written but not run.
- Caps Lock toggles on some keyboards are immediate and some ignore the LED command.
- Lowercase folds to capitals. Digits and a few letters are supported; punctuation is skipped.

## Restart policy (panic)
`kernel_panic()` sends the message for `restart_s` seconds, then resets the machine:
- `restart_s = -1` (default): repeat forever, never restart.
- `restart_s = 0`: no message, restart at once (the Ctrl+Alt+Shift+Enter combo, with Caps Lock on).
- otherwise: `restart_s / 20` messages (each about 20 s), then reset.

    if n == -1: repeat forever
    else:       for i in range(n // 20): morse()
    reset

The reset writes 0xFE to the keyboard controller (port 0x64), which pulses the CPU reset line.
If the reset does nothing, the CPU halts.

`sys_debug_crash(principal, restart_s)` stores the restart policy and then crashes. Once the
IDT exists, the fault handler calls `kernel_panic()`, so the policy applies. Until then, the
fault resets the machine directly and no message is sent.
