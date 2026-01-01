# tex2bmp-amsh
A simple .TEX to .BMP converter for Army Men games.

I'M NOT A PROGRAMMER!
tex2bmp is a converter I made feeding ChatGPT info I dug up using ImHex and Ghidra and it helped me write this converter to edit texture files for Army Men - World War & Sarge's Heroes on PS1.
I loved these games as a kid, and was always facsinated with them. As tanky as the controls are and god awfully rushed they were, 3DO was a part of my childhood and I'll always love them.

Anyway!

Right now you should be able to convert any .TEX file from 3DO as far as I'm aware of. It'll unpackage the .TEX file into it's base directory inside a folder of all the attached tiles. It even has the ability to convert it back.

# Key information!
Couple things to note here. The game engine renders black pixels RGB(0,0,0) as transparent, so if you convert back to .TEX, you'll need to make sure your transparent pixels are exactly RGB(0,0,0). It doesn't support opacity.
I will include a documentation file on how the .TEX file works and is rendered by the game's engine on the PS1 soon, I'll have ChatGPT handle the info as all I am is a spoiled passenger princess.

# How to use.
Just drag and drop any .TEX file onto the tex2bmp.exe and it'll unpackage it with ease.
