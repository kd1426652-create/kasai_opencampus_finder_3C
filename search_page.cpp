#include "search_page.h"
#include "ui_helper.h"
#include "DxLib.h"


void SchoolSearchPage() {
    DrawBox(0, 0, SW, 75, GetColor(15, 23, 42), TRUE);
    Text(20, 20, "KANSAI", WHITE, 18);
    Text(100, 16, "Kansai Foreign Student IT Open Campus Finder", WHITE, 18);
    Text(100, 42, "IT School Finder & Rating System", GRAY, 12);


    Panel(600, 18, 140, 40, 8, CARD, BORDER);
    Text(615, 28, "Q Find School", WHITE, 13);
    Button(750, 18, 120, 40, "Logout", RED);

    // Card 1: ECC
    Panel(30, 130, 840, 135, 10, CARD, BORDER);
    Panel(45, 142, 100, 24, 6, BLUE, BLUE);
    Text(52, 146, "Osaka", WHITE, 12);
    Panel(155, 142, 210, 24, 6, GetColor(51, 65, 85), BORDER);
    Text(162, 146, "IT & Software / Web / Game", WHITE, 12);


    Text(45, 175, "ECC Computer Academy", WHITE, 18);
    Text(45, 205, "Located in Osaka Umeda, specializing in IT Programming, AI & Game Dev.", GRAY, 13);
    Text(45, 232, "Open Campus: 2026/11/15 (Sun)", LIGHT_BLUE, 13);


    Text(800, 140, "4.8 ★", GetColor(234, 179, 8), 16);
    Button(720, 185, 130, 38, "Detail", BLUE);

    // Card 2: HAL Osaka
    Panel(30, 285, 840, 135, 10, CARD, BORDER);
    Panel(45, 297, 100, 24, 6, BLUE, BLUE);
    Text(52, 301, "Osaka", WHITE, 12);
    Panel(155, 297, 180, 24, 6, GetColor(51, 65, 85), BORDER);
    Text(162, 301, "IT / Cyber Security", WHITE, 12);


    Text(45, 330, "HAL Osaka", WHITE, 18);
    Text(45, 360, "Located near Osaka Station, top school for Modern IT & Cyber Security.", GRAY, 13);
    Text(45, 387, "Open Campus: 2026/11/22 (Sun)", LIGHT_BLUE, 13);


    Text(800, 295, "4.7 ★", GetColor(234, 179, 8), 16);
    Button(720, 340, 130, 38, "Detail", BLUE);

}
