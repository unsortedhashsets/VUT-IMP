/*
Autor: Mikhail Abramov
Login: xabram00
Datum: 20. 11. 2020
*/

#include <stdbool.h>

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
 * Inicializace periferii/komponent po naprogramovani FPGA
 */
void fpga_initialized()
{
  LCD_clear();
  LCD_append_string("IMP-Ckick");
}

void print_user_help(void)
{
  term_send_str("Press any key (0-9, *, #) to start measurement");
  term_send_crlf();
}

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

void show_prog_info(void)
{
  LCD_clear();
  LCD_append_string("...RPM: ");
  LCD_append_char(RPM1);
  LCD_append_char(RPM2);
  LCD_append_char(RPM3);
  LCD_append_char(RPM4);
  LCD_append_string(" c/m");
}

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

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return CMD_UNKNOWN;
}

/**
 * Obsluha klavesnice
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

interrupt(TIMERA0_VECTOR) Timer_A(void)
{
  flip_led_d6();

  RPM = (clicks * 60 + last_RPM) / 2;

  clicks = 0;

  if (RPM != last_RPM)
  {
    rpm_digits_count();
    print_prog_info();
    last_RPM = RPM;
  }
  show_prog_info();
  CCR0 += 0x8000; // nastav po kolika ticich (16384 = 0x4000, tj. za 1/2 s) ma dojit k dalsimu preruseni
}

/*******************************************************************************
 * Hlavni funkce
 *******************************************************************************/
int main(void)
{

  initialize_hardware();
  keyboard_init();

  WDG_stop();

  CCTL0 = CCIE;            // povol preruseni pro casovac (rezim vystupni komparace)
  CCR0 = 0x8000;           // nastav po kolika ticich (16384 = 0x4000, tj. za 1/2 s) ma dojit k preruseni
  TACTL = TASSEL_1 + MC_2; // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim

  // Signalaizace stojiciho vytahu
  set_led_d6(0); // rozsviceni D6
  set_led_d5(1); // rozsviceni D5

  while (1)
  {
    delay_ms(10);
    keyboard_idle(); // obsluha klavesnice
    terminal_idle(); // obsluha terminalu
  }
}