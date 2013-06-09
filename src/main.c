#include <stdio.h>
#include <sys/ptrace.h>
#include <stdint.h>

#include <SFML/Graphics.h>

#include "hook.h"


int main(int argc, char** argv) {

    if(argc != 2) {
        printf("Usage:\n%s `pidof hl2_linux`\n", argv[0]);
        return 0;
    }

    sfVideoMode mode = {768, 768, 32};
    sfRenderWindow* window;

    window = sfRenderWindow_create(mode, "CSSRadar", sfTitlebar | sfClose, NULL);
    sfRenderWindow_setFramerateLimit(window, 30);

    sfView* view = sfView_create();

    sfTexture* dust2 = sfTexture_createFromFile("de_dust2.png", NULL);
    sfTexture* dust1 = sfTexture_createFromFile("de_dust.png", NULL);

    sfShape* background = sfRectangleShape_create();
    sfShape* playerShape = sfRectangleShape_create();
    {
        sfVector2f size = {32,32};
        sfRectangleShape_setSize(playerShape, size);
    }



    int pid = atoi(argv[1]);

    uint32_t client = find_dylib(pid, "cstrike/bin/client.so", 0);
    uint32_t engine = find_dylib(pid, "bin/engine.so", 0);

    uint32_t current_map_addr = engine + 0xae769c;


    uint32_t players_base = client + 0xcf35a0;
    uint32_t players_base_val = 0;
    uint32_t players_abs = 0;
    player_t players[32];
    char current_map[40];


    sfClock* clock = sfClock_create();

    while(sfRenderWindow_isOpen(window)) {
        sfEvent event;
        while(sfRenderWindow_pollEvent(window, &event)) {
            if(event.type == sfEvtClosed) {
                sfRenderWindow_close(window);
            }
        }

        sfTime time = sfClock_getElapsedTime(clock);

        if(sfTime_asSeconds(time) > 0.5) {
            sfClock_restart(clock);

            mem_read(pid, current_map_addr, &current_map[0], 40);

            mem_read(pid, players_base, &players_base_val, sizeof(players_base_val));

            if(players_base_val != -1) {
                players_abs = players_base_val;
                mem_read(pid, players_abs, &players, sizeof(player_t)*32);
            }


            if(strncmp(current_map, "de_dust2", 40) == 0) {
                sfVector2f view_size = {5000, 4000};
                sfVector2f view_center = {-2048, -1024};
                sfVector2f bg_size = {4150, 4350};
                sfVector2f bg_pos = {-1850,-1060};
                sfView_setSize(view, view_size);
                sfView_setCenter(view, view_center);
                sfRenderWindow_setView(window, view);
                sfRectangleShape_setSize(background, bg_size);
                sfRectangleShape_setPosition(background, bg_pos);
                sfRectangleShape_setTexture(background, dust2, sfFalse);
            } else if(strncmp(current_map, "de_dust", 40) == 0) {
                sfVector2f view_size = {6525, 5800};
                sfVector2f view_center = {-200, 1000};
                sfVector2f bg_size = {5750, 5800};
                sfVector2f bg_pos = {-3250,-1900};
                sfView_setSize(view, view_size);
                sfView_setCenter(view, view_center);
                sfRenderWindow_setView(window, view);
                sfRectangleShape_setSize(background, bg_size);
                sfRectangleShape_setPosition(background, bg_pos);
                sfRectangleShape_setTexture(background, dust1, sfFalse);
            } else {
                sfRectangleShape_setTexture(background, NULL, sfFalse);
            }

        }


        sfRenderWindow_clear(window, sfBlack);

        sfRenderWindow_drawRectangleShape(window, background, NULL);

        for(int i = 0; i < 32; ++i) {
            player_t* p = &players[i];
            if(p->team != TEAM_NONE && p->health > 0) {
                sfVector2f pos = {p->x * -1,p->y};
                sfRectangleShape_setPosition(playerShape, pos);
                if(p->team == TEAM_T) {
                    sfRectangleShape_setFillColor(playerShape, sfRed);
                } else {
                    sfRectangleShape_setFillColor(playerShape, sfBlue);
                }

                sfRenderWindow_drawRectangleShape(window, playerShape, NULL);
            }
        }

        sfRenderWindow_display(window);

    }

    sfClock_destroy(clock);
    sfShape_destroy(background);
    sfShape_destroy(playerShape);
    sfTexture_destroy(dust2);
    sfTexture_destroy(dust1);
    sfView_destroy(view);
    sfRenderWindow_destroy(window);

    return 0;
}
