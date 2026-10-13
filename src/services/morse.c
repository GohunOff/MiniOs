#include "morse.h"

#include <ctype.h>

#include "pico/stdlib.h"

static unsigned int morse_led_pin;

static const char *morse_table[36] =
{
    ".-",    // A
    "-...",  // B
    "-.-.",  // C
    "-..",   // D
    ".",     // E
    "..-.",  // F
    "--.",   // G
    "....",  // H
    "..",    // I
    ".---",  // J
    "-.-",   // K
    ".-..",  // L
    "--",    // M
    "-.",    // N
    "---",   // O
    ".--.",  // P
    "--.-",  // Q
    ".-.",   // R
    "...",   // S
    "-",     // T
    "..-",   // U
    "...-",  // V
    ".--",   // W
    "-..-",  // X
    "-.--",  // Y
    "--..",  // Z

    "-----", // 0
    ".----", // 1
    "..---", // 2
    "...--", // 3
    "....-", // 4
    ".....", // 5
    "-....", // 6
    "--...", // 7
    "---..", // 8
    "----."  // 9
};

static void morse_dot(void)
{
    gpio_put(morse_led_pin, 1);
    sleep_ms(150);

    gpio_put(morse_led_pin, 0);
    sleep_ms(100);
}

static void morse_dash(void)
{
    gpio_put(morse_led_pin, 1);
    sleep_ms(450);

    gpio_put(morse_led_pin, 0);
    sleep_ms(100);
}

static void morse_send_character(char c)
{
    c = (char)toupper((unsigned char)c);

    const char *code = NULL;

    if (c >= 'A' && c <= 'Z')
    {
        code = morse_table[c - 'A'];
    }
    else if (c >= '0' && c <= '9')
    {
        code = morse_table[26 + (c - '0')];
    }
    else
    {
        return;
    }

    while (*code)
    {
        if (*code == '.')
        {
            morse_dot();
        }
        else if (*code == '-')
        {
            morse_dash();
        }

        code++;
    }

    // przerwa pomiędzy literami
    sleep_ms(300);
}

void morse_init(unsigned int led_pin)
{
    morse_led_pin = led_pin;

    gpio_init(morse_led_pin);
    gpio_set_dir(morse_led_pin, GPIO_OUT);
    gpio_put(morse_led_pin, 0);
}

void morse_send(const char *text)
{
    if (text == NULL)
    {
        return;
    }

    while (*text)
    {
        if (*text == ' ')
        {
            // przerwa pomiędzy słowami
            sleep_ms(700);
        }
        else
        {
            morse_send_character(*text);
        }

        text++;
    }

    gpio_put(morse_led_pin, 0);
}