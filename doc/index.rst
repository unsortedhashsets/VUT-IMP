.. article:: apps_demo_msp_led
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>
    :updated: 20090415

    Aplikace demonstruje řízeni zelené LED (D5) pomocí obsluhy přerušení napsané pro časovač timer A0 použitý v režimu výstupní komparace. 

============================
Blikání LED pomocí časovače
============================

Popis aplikace
=================

Tato jednoduchá aplikace demonstruje využití časovače z mikrokontroléru k odměřování frekvence blikání LED. Výhodou použití časovače je, že CPU mikrokontroléru nemusí kvůli odměřování času aktivně čekat ve smyčce; měření času přenechá časovači a věnuje se jiným užitečným činnostem.

Připojení LED diody D5 k MCU
==============================

Jelikož anoda D5 je (přes rezistor R53) spojena s potenciálem 3.3 V, je ze schématu zřejmé, že D5 bude zhasnuta bude-li na vývodu P1.0 mikrokontroléru (spojeném s katodou D5) stejný potenciál (to je případ log. 1 na vývodu P1.0). Naopak, D5 bude svítit bude-li potenciál na P1.0 dostatečně menší než 3.3 V (případ log.0 na vývodu P1.0).

.. figure:: msp_led_d5.png

   Svit D5 je možno ovládat signálem z bitu 0 portu 1 (P1.0) mikrokontroléru MSP430 (detail schématu a umístění LED D5 na FITkitu).  

Princip řízení D5 z MCU
========================

Aby bylo možné D5 ovládat vývodem P1.0 (tj. 0. bit portu P1), je třeba nejprve nastavit tento vývod na výstupní, což je možno provést např. příkazem

::

  P1DIR |= 0x01;   // nastav P1.0 na vystup (budeme ovladat zelenou LED D5)

Zhasnutí D5 (tj. nastavení 0. bitu portu P1 na log.1) je pak možno provést např. příkazem

::

  P1OUT |= 0x01;   // P1.0 na log.1 (D5 off)

a obdobně rozsvícení D5 (tj. nastavení 0. bitu portu P1 na log.0) příkazem

::

  P1OUT &= 0xFE;   // P1.0 na log.0 (D5 on)

Jinou možností je použít místo uvedených příkazů funkce ``set_led_d5(1)``, ``set_led_d5(0)`` nebo ``flip_led_d5()``.

V naší aplikaci však budeme chtít, aby naše LED blikala s periodou 1 s, a to nezávisle na hlavní programové smyčce prováděné mikrokontrolérem. 
K řízení LED proto využijeme časovač, který je schopen v periodických intervalech vyvolávat přerušení. 
V obsluze tohoto přerušení budeme měnit stav LED ze zhasnuté na rozsvícenou a naopak.

**Konfigurace časovače A0**

Jelikož chceme vyvolávat přerušení s periodou 1 s, bude nejvýhodnější zvolit za zdroj hodin časovače hodinový signál ACLK, který má kmitočet 32768 Hz:

::

  TACTL = TASSEL_1 + MC_2;  // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim

Aby bylo možné využít přerušení od časovače, je nutno nejprve povolit přerušení pro časovač:

::

  CCTL0 = CCIE;   // povol preruseni pro casovac (rezim vystupni komparace)

a nastavit, po kolika ticích (hodin ACLK s kmitočtem 32768 Hz) má být vyvoláno přerušení. Jelikož chceme, aby D5 blikala s periodou 1 s, budeme potřebovat, aby byla 0,5 s (tj. po 16384 tiků hodin ACLK) zhasnutá a 0,5 s rozsvícená:

.. figure:: msp_led_timing.png

   Časování   

::

  CCR0 = 0x4000;  // nastav po kolika ticich (16384 = 0x4000, tj. za 1/2 s) ma dojit k preruseni

Dále si ukážeme, jak je možné napsat obsluhu přerušení pro časovač a co bude v jejím těle.

**Obsluha přerušení časovače A0**

Napsat obsluhu přerušení pro časovač A0 znamená napsat tělo funkce ``interrupt (TIMERA0_VECTOR) Timer_A (void)``

V případě naší aplikace by obsluha přerušení (vyvolávaná každou 1/2 s za účelem změny stavu D5) časovače A0 mohla vypadat např. takto:

::

  interrupt (TIMERA0_VECTOR) Timer_A (void)
  {
    P1OUT ^= 0x01;    // invertuj bit P1.0 (kazdou 1/2 s)
    CCR0 += 0x4000;   // nastav po kolika ticich (16384 = 0x4000, tj. za 1/2 s) ma dojit k dalsimu preruseni
  }

Kompletní zdrojové kódy je možné nalézt v souboru `mcu/main.c <SVN_APP_DIR/mcu/main.c>`_. 

Zprovoznění aplikace
========================
1. přeložte aplikaci
2. naprogramujte MCU a FPGA a spusťte terminálový program.

Aplikace nevyžaduje ke své činnosti nastavit kromě propojek umožňujících programování další propojky.