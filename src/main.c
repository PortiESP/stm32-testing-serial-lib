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
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <serial.h>

#define STARTB  '\xAA' // Start byte for communication.
#define STOPB   '\x55' // Stop byte for communication.

volatile uint32_t millis; // Keeps time reference.

/**
 * @brief Data struct to send throw the serial port.
 */
typedef struct
{
    uint32_t time;
    uint8_t character;
} data_t;

const uint16_t id = 0xCAFE; // Serial ID, send at startup

/**
 * @brief Blocking delay
 * 
 * @param ms amount of milliseconds.
 */
void delay_ms(uint32_t ms);

/**
 * @brief Sends raw data bytes applying the custom protocol
 *  _______ _________ ________ ________ _____ ________ ________ ______
 * | START | N BYTES | BYTE_0 | BYTE_1 | ... | BYTE_N | CHKSUM | STOP |
 *  ------- --------- -------- -------- ----- -------- -------- ------
 * @param data pointer to the data to be send.
 * @param length amount of bytes of the data.
 * @param start byte that indicates the start of communication.
 * @param stop byte that indicates the end of the communication.
 * @return true data was send.
 * @return false data wasn't send.
 */
bool send_protocol(const void *data, const uint8_t length,
                   const uint8_t start, const uint8_t stop);

int main(void)
{
    // Setup the system clock
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    // Systick setup
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8); // 72M/8 = 9M
    systick_set_reload(8999); // 9M/9000 = 1k => T = 1ms
    systick_counter_enable();
    systick_interrupt_enable();

    // Init serial port
    serial_begin(USART1, BAUD115K2);

    send_protocol(&id, sizeof(id), STARTB, STOPB);

    data_t myData = {0, 0x20};

    while (true)
    {
        send_protocol(&myData, sizeof(data_t), STARTB, STOPB);
        delay_ms(1000);
        myData.time = millis;
        myData.character = (myData.character == 0x7F) ? 0x20 : myData.character + 1;
    }
}

bool send_protocol(const void *data, const uint8_t length,
                   const uint8_t start, const uint8_t stop)
{
    bool result = false;
    if (serial_sendable(USART1) > (length + 3))
    {
        uint8_t chksum = 0, *p = (uint8_t *)data;
        for (uint16_t i = 0; i < length; i++)
            chksum ^= *p++;

        serial_write(USART1, start);
        serial_write(USART1, length);
        serial_send_data(USART1, data, length);
        serial_write(USART1, chksum);
        serial_write(USART1, stop);
        result = true;
    }
    return result;
}

void delay_ms(uint32_t ms)
{
    uint32_t tm = millis + ms;
    while (millis < tm);
}

void sys_tick_handler(void)
{
    millis++;
}
