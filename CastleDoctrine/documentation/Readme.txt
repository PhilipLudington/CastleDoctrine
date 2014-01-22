The Castle Doctrine
A massively-multiplayer game of burglary and home defense by Jason Rohrer.

http://thecastledoctrine.net


This game has dynamically-generated music.  Please use speakers or headphones.




== Controls ==

Press ESC to pause the game and view a menu screen.

The mouse is used for almost all actions in the game, except for:

1)  Moving your character:   Arrow keys or W A S D keys.
    
    The same keys are used for panning the view during a tape replay (when
    the replay is paused and the shroud is off). 

2)  Selecting tile under mouse for placement (eyedropper):   Control-click


If the movement keys are not comfortable on your keyboard, edit 
"settings/upDownLeftRightKeys.ini" with a text editor to 
specify your own set of four keys (change "wasd" to "tfgh", for example).




== Screen Size ==

The game defaults to windowed mode, automatically picking the largest possible
window that fits comfortably on your monitor.  You can override this behavior
by editing useLargestWindow.ini in the settings folder.  After that, you can
control the window size by editing screenWidth.ini and screenHeight.ini.

The same size settings are also used as a target screen resolution if you
switch the game to full-screen mode in the settings folder.  Once the game has
been started in full-screen mode, you can switch to windowed mode by pressing
Alt-Enter.

If the game seems sluggish at the default 60 frames-per-second, 
try switching to 30 FPS by putting a 1 in "settings/halfFrameRate.ini".

If it still seems sluggish at 30 FPS, switch to 15 FPS by putting a 2 in 
that settings file.  If it still seems sluggish, try 7.5 FPS by putting a 3 
in that settings file, and so on.




== Web Proxies ==

If your network requires that you use a proxy server to connect to the web,
edit settings/webProxy.ini and insert your proxy's address and port in the 
address:port format.  Here's an example:

197.211.197.70:3128
