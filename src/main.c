/**
 * @file example1.c
 * @author Matías S. Ávalos (msavalos@gmail.com)
 * @brief toUpper and toLower
 * @version 0.4.8
 * @date 2021-02-08
 * 
 * In this example if you send a lowercase char, it will prompt the uppercase one
 * and vice versa.
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 * This file is part of Serial lib API for libOpenCM3
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

// Configuration de la libreria `serial.h`
#define USING (USE_ALL_USARTS)
#define MAX_BUFFER_TX1 128UL
#define MAX_BUFFER_RX1 128UL

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <string.h>
#include <serial.h>

int main(void)
{
    const char *hi = "Hi, from the BluePill!\r\n";
    uint32_t hi_len = strlen(hi);

    // Setup the system clock
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    // PC13 (LED) as output:
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
    gpio_set(GPIOC, GPIO13);

    // USART1 8,N,1 38400bps
    serial_begin(USART1, BAUD9600);

    // The amount of bytes puts in the buffer must be equal to the length of the string
    if (hi_len != serial_puts(USART1, hi))
        gpio_clear(GPIOC, GPIO13); // ups! ERROR, not enough space in the TX buffer...

    while (true)
    {
        // check if the RX buffer has data
        if (serial_available(USART1) > 0)
        {
            uint8_t c = serial_read(USART1);           // Reading from RX Buffer
            if (c >= 'A' && c <= 'Z')                  // If it's upper:
                serial_write(USART1, c + ('a' - 'A')); //      send lower.
            else if (c >= 'a' && c <= 'z')             // If it's lower:
                serial_write(USART1, c - ('a' - 'A')); //      send upper.
            else                                       // something else:
                serial_write(USART1, c);               //      echo.
        }
    }
}
