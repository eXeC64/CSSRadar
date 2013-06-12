CSSRadar
========
A rough proof of concept external radar cheat for Counter-Strike:Source on Linux.


This was literally thrown together in one all-night'er with no preparation.

![CSSRadar Screenshot](http://i.imgur.com/8NEnqBp.jpg)


Development
------------

First step was getting decent tools, so after ten minutes of research I started using `gameconqueror`.
While a little clunky to use, it still did everything I needed, so I set about finding some useful data structures.

First, I scanned for my name, then changed it, and scanned again. This gave me half a dozen addresses, so I made a note of them and moved on. I then found all the addresses matching my health (100), and stood in the way of grenades, and deliberately inflicted fall-damage upon myself, while monitoring changes. This also produced a bunch of addresses that I took note of. I then did the same for my position, and thanks to the `cl_showpos` console variable I was able to determine my exact position in game coordinates.

Looking through the suspect addresses I discovered my health, name and position all had addresses within a few bytes of one another. It looked like a promising lead so I focused on that, watching memory addresses around that area as I changed my in-game state. I found a team number this way.

I then added a few bots into the game, and did similar tests on them, discovering the same data structure existing for them too. It was an array of structures with a stride of 0x140.

This is the structure I was able to reverse engineer at the time through memory inspection:

    typedef struct player_s {
        uint8_t     name[32];
        uint32_t    team;
        uint32_t    health;
        float       x, y, z;
        float       pitch, yaw;
        uint8_t     padding0[260];
    } player_t;

So, now I had an array of player data, the next task was to be able to find it programatically. A quick `cat /proc/$(pidof hl2_linux)/maps` showed me that the structure was on the heap, so I couldn't rely on the pointer being the same next time the game launched. So, what to do? More research.

Jumping back into gameconqueror I switched to byte array search, and started looking for pointers. Pretending that the pointer to my structure was `0x12345678` I entered `?? 56 34 12` into the bytesearch (the bytes are reversed because the source engine is compiled as x86, which is little endian).

This threw up a few results, referring back to the output from `cat /proc/$(pidof hl2_linux)/maps` I discovered one of these points was within `cstrike/bin/client.so`. So then I simply have to subtract the address that client.so was loaded into memory at from the pointer's address and I have an offset into client.so that'll reliably give me this data structure's address every time, regardless of where in the heap it is located.

So now I had that, it was time to get coding. After a bit of research I got a simple C program running that attached to cstrike, copied some memory, and printed a list of all connected players, their name, health and location. A radar cheat in the making.

Not satisfied with that though, I snatched a copy of SFML's C bindings and threw together a simple graphical program that pulls the player's locations, and projects them onto a map, allowing me to clearly see where any player in the game is.

It was fun, and a lot easier to get up and running than I expected. I think I may foray more into the world of creating video-game cheats.

Valve Anti-Cheat
----------------
While I can't condone using this in live, online play it is worth nothing that owing to the method used the likelyhood of being caught by VAC is extremely low. The cheat makes no modifications to the game, and only attaches for a split second to read out the latest data. The only thing it does that the game itself could try to detect is it attaching/dettaching.

The only viable defense against this is to pro-actively scan for this cheat in global memory. However, Punkbuster tried that a few years ago... and it did not end well. (The cheaters discovered that by dumping certain strings into IRC channels frequented by professional gamers they could get many professionals wrongly banned)


Disclaimer & License
--------------------
If you get caught cheating in VAC protected servers, or competitive leagues with this it's your own fault, not mine. I take no responsibility for what you do with it. The code is only published for academic purposes, and as such it is not licensed publically. I reserve all rights.
