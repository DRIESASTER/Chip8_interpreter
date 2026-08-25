# Design Choices

## SDL2
I started out reading input using termios and creating a display in the terminal as I wanted to really stay low level. 
However as i'm approaching adding audio support I'm switching to sdl2, not because it would be 'impossibly hard' to do this without an api, but because I don't want my code to require POSIX support.


