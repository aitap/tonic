# What is it?

This application trains you to recognise steps of various musical keys by ear by offering randomly chosen keys and randomly chosen steps in them.

# How do I use it?

![screenshot](screenshot.png)

Setup: set your MIDI output device and click _Open_ if the default choice isn't right (known issue: on Linux, `MIDI Through` is selected by default). Use the combo box and _Set program_ button to change the instrument from the default (should be grand piano on most devices).

Tonic would offer you **single notes** instead of full chords to guess (which are easier) unless you uncheck the checkbox.

Tonic displays current randomly chosen key and the **previous** answer. If you guessed it right, it's painted green; if your guess was wrong, it's painted red and current step of the key played again. Every time you change the key, tonic is played first. To submit your choice, press `Ctrl+number` or press one of the screen buttons labeled from *I* to *VII*. Screen buttons also change their colours if your guess was wrong.

Press *Again* or `=` to play the current step of the key again. Press *Tonic* or `t` to play the tonic again (you may consider pressing `=` and `t` multiple times cheating).

Press *Change key* or `-` to choose a different randomly chosen key to guess the steps of.

# How to build?

Use [CMake](https://cmake.org/) to generate the build files. Point it at [IUP](http://webserver2.tecgraf.puc-rio.br/iup/) GUI library and [PortMidi](http://portmedia.sourceforge.net/portmidi/) MIDI abstraction headers and library files. No installation is required.
