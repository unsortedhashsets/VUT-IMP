/*
Author: Mikhail Abramov
Login: xabram00
Date: 20. 11. 2020
*/

#include <fitkitlib.h>
#include <keyboard/keyboard.h>
#include <lcd/display.h>

char last_ch;

int clicks = 0;

int RPM = 0;
int last_RPM = 0;

char RPM1 = 48;
char RPM2 = 48;
char RPM3 = 48;
char RPM4 = 48;

/**
 * Initialization of components after FPGA programming
 */
void fpga_initialized()
{
  LCD_clear();
  LCD_append_string("IMP-Tachometer  ");
  LCD_append_string("...RPM: ");
  LCD_append_char(RPM1);
  LCD_append_char(RPM2);
  LCD_append_char(RPM3);
  LCD_append_char(RPM4);
  LCD_append_string(" r/m");
  term_send_str("IMP-Tachometer program started");
  term_send_crlf();
  term_send_str("------------------------------");
  term_send_crlf();
  term_send_str("Author: Mikhail Abramov");
  term_send_crlf();
  term_send_str("Login: xabram00");
  term_send_crlf();
  term_send_str("Date: 20. 11. 2020");
  term_send_crlf();
  term_send_str("------------------------------");
  term_send_crlf();
  term_send_str("To start measurement press:");
  term_send_crlf();
  term_send_str("0,1,2,3,4,5,6,7,8,9,*,#.");
  term_send_crlf();
  term_send_str("------------------------------");
  term_send_crlf();
}

/**
 * Response on 'help' command
 */
void print_user_help(void)
{
  term_send_str("Press any key (0-9, *, #) to start measurement");
  term_send_crlf();
}

/**
 * Print logs into the terminal window
 */
void print_prog_info(void)
{
  term_send_str("...RPM: ");
  term_send_char(RPM1);
  term_send_char(RPM2);
  term_send_char(RPM3);
  term_send_char(RPM4);
  term_send_str(" c/m");
  term_send_crlf();
}

/**
 * Show logs onto LCD screen window
 */
void show_prog_info(void)
{
  LCD_clear();
  LCD_append_string("IMP-Tachometer  ");
  LCD_append_string("...RPM: ");
  LCD_append_char(RPM1);
  LCD_append_char(RPM2);
  LCD_append_char(RPM3);
  LCD_append_char(RPM4);
  LCD_append_string(" r/m");
}

/**
 * Additional function to convert XXXX digit on four single chars
 */
void rpm_digits_count(void)
{
  if (RPM < 10)
  {
    RPM1 = 48;
    RPM2 = 48;
    RPM3 = 48;
    RPM4 = (RPM % 10) + 48;
  }
  else if (RPM < 100)
  {
    RPM1 = 48;
    RPM2 = 48;
    RPM3 = (RPM / 10 % 10) + 48;
    RPM4 = (RPM % 10) + 48;
  }
  else if (RPM < 1000)
  {
    RPM1 = 48;
    RPM2 = (RPM / 100 % 10) + 48;
    RPM3 = (RPM / 10 % 10) + 48;
    RPM4 = (RPM % 10) + 48;
  }
  else if (RPM < 10000)
  {
    RPM1 = (RPM / 1000 % 10) + 48;
    RPM2 = (RPM / 100 % 10) + 48;
    RPM3 = (RPM / 10 % 10) + 48;
    RPM4 = (RPM % 10) + 48;
  }
}

/**
 * Command operator
 */
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return CMD_UNKNOWN;
}

/**
 * Keyboard operator
 */
int keyboard_idle()
{
  char ch;
  ch = key_decode(read_word_keyboard_4x4());
  if (ch != last_ch)
  {
    last_ch = ch;
    if (ch != 0 && ch < 64)
    {
      clicks++;
      flip_led_d5();
    }
  }
  return 0;
}

/**
 * Timer interrupt handling - timer A0
 */
interrupt(TIMERA0_VECTOR) Timer_A(void)
{
  flip_led_d6();

  RPM = (clicks * 60 + last_RPM) / 2;

  clicks = 0;

  if (RPM != last_RPM)
  {
    rpm_digits_count();
    print_prog_info();
    show_prog_info();
    last_RPM = RPM;
  }
  CCR0 += 0x8000;
}

/**
 * Main function
 */
int main(void)
{

  initialize_hardware();
  keyboard_init();

  WDG_stop();              // stop watch-dog

  CCTL0 = CCIE;            // enable interrupt for timer
  CCR0 = 0x8000;           // over 0x8000 - 1 s should be interrupted
  TACTL = TASSEL_1 + MC_2; // continuous mode

  set_led_d6(0); // delighting D6
  set_led_d5(1); //   lighting D5

  while (1)
  {
    delay_ms(10);    // oscillation control
    keyboard_idle(); // keyboard operator
    terminal_idle(); // terminal operation
  }
}