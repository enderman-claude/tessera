#include "tessera/kernel/morse_encode.h"

/* Each code is a string of '.' and '-'. */
static const char *code_for(char c)
{
    switch (c) {
    case 'A': return ".-";     case 'B': return "-...";   case 'C': return "-.-.";
    case 'D': return "-..";    case 'E': return ".";      case 'F': return "..-.";
    case 'G': return "--.";    case 'H': return "....";   case 'I': return "..";
    case 'J': return ".---";   case 'K': return "-.-";    case 'L': return ".-..";
    case 'M': return "--";     case 'N': return "-.";     case 'O': return "---";
    case 'P': return ".--.";   case 'Q': return "--.-";   case 'R': return ".-.";
    case 'S': return "...";    case 'T': return "-";      case 'U': return "..-";
    case 'V': return "...-";   case 'W': return ".--";    case 'X': return "-..-";
    case 'Y': return "-.--";   case 'Z': return "--..";
    case '0': return "-----";  case '1': return ".----";  case '2': return "..---";
    case '3': return "...--";  case '4': return "....-";  case '5': return ".....";
    case '6': return "-....";  case '7': return "--...";  case '8': return "---..";
    case '9': return "----.";
    default: return 0;
    }
}

static int push(struct morse_event *out, size_t cap, size_t *n, uint8_t on, uint8_t units)
{
    if (*n >= cap) return -1;
    out[*n].on = on;
    out[*n].units = units;
    (*n)++;
    return 0;
}

int morse_encode(const char *text, struct morse_event *out, size_t cap)
{
    size_t n = 0;
    for (const char *p = text; *p; p++) {
        char c = *p;
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
        if (c == ' ') {
            /* Word gap is 7 units. The letter gap of 3 already follows the last letter, so add 4. */
            if (n == 0 || out[n - 1].on) return -1;
            if (push(out, cap, &n, 0, 4) != 0) return -1;
            continue;
        }
        const char *code = code_for(c);
        if (!code) continue;
        for (const char *s = code; *s; s++) {
            uint8_t on_units = (*s == '-') ? 3 : 1;
            if (push(out, cap, &n, 1, on_units) != 0) return -1;
            if (s[1]) {
                if (push(out, cap, &n, 0, 1) != 0) return -1;
            }
        }
        if (push(out, cap, &n, 0, 3) != 0) return -1;
    }
    return (int)n;
}
