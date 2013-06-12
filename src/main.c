#include <stdio.h>
#include <sys/ptrace.h>
#include <stdint.h>
#include <stdbool.h>

#include <SFML/Graphics.h>

#define TEAM_NONE 0
#define TEAM_SPECTATE 1
#define TEAM_T 2
#define TEAM_CT 3

typedef struct player_s {
    uint8_t     padding1[40];
    uint32_t    index;
    uint8_t     padding2[12];
    uint8_t     name[32];
    uint32_t    team;
    uint32_t    health;
    float       x, y, z;
    float       pitch, yaw;
    uint8_t     padding3[204];
} player_t;

uint32_t find_dylib(uint32_t pid, const char* dylib, bool find_end);
void mem_read(uint32_t pid, uint32_t remote_addr, uint32_t* buf, size_t buf_size);

int main(int argc, char** argv) {

    if(argc != 2) {
        printf("Usage:\n%s `pidof hl2_linux`\n", argv[0]);
        return 0;
    }

    sfVideoMode mode = {1024, 768, 32};
    sfRenderWindow* window;

    window = sfRenderWindow_create(mode, "CSSRadar", sfTitlebar | sfClose, NULL);
    sfRenderWindow_setFramerateLimit(window, 30);

    sfView* view = sfView_create();

    sfTexture* dust1 = sfTexture_createFromFile("de_dust.png", NULL);
    sfTexture* dust2 = sfTexture_createFromFile("de_dust2.png", NULL);

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

    uint32_t players_addr = client + 0xcf35a0;
    uint32_t players_addr_val = 0;
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
            mem_read(pid, players_addr, &players_addr_val, sizeof(players_addr_val));

            if(players_addr_val != -1) {
                mem_read(pid, players_addr_val, &players, sizeof(player_t)*32);
            }

            /* Line up map with player coordinates */
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

uint32_t find_dylib(uint32_t pid, const char* dylib, bool find_end) {
    char* cmd[256];
    if(!find_end) {
        snprintf(cmd, 256, "grep \"%s\" /proc/%i/maps | head -n 1 | cut -d \"-\" -f1", dylib, pid);
    } else {
        snprintf(cmd, 256, "grep \"%s\" /proc/%i/maps | tail -n 1 | cut -d \"-\" -f2", dylib, pid);
    }

    FILE* maps = popen(cmd, "r");

    uint32_t ptr = 0;

    if(maps) {
        fscanf(maps, "%08lx", &ptr);
    }

    pclose(maps);

    return ptr;
}

void mem_read(uint32_t pid, uint32_t remote_addr, uint32_t* buf, size_t buf_size) {

    if(buf_size % 4) {
        return;
    }

    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    waitpid(pid, NULL, 0);

    for(int i = 0; i < buf_size/4; ++i) {
        buf[i] = ptrace(PTRACE_PEEKDATA, pid, remote_addr + (4*i), NULL);
    }

    ptrace(PTRACE_DETACH, pid, NULL, NULL);
}
