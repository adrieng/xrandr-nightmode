# Overview

This utility imitates the "night mode" feature of f.lux. This changes the color
of your screen, basically turning white into black and everything else into
shades of red. I missed this feature when switching from f.lux to redshift, and
this makes it available on Linux as well.

This is very simple code which reuses the hard work of the Redshift project as
well as Zoltan Padrah's xrandr-invert-colors. Praise to them.

# Compiling and installing

You need the XRandR development file and a C compiler. Typing `make install`
will copy the binary to $(HOME)/bin/xrandr-nightmode by default.

# Using

When run, xrandr-nightmode silently forks and runs in the background. To turn
night mode on or off, send it a USR1 signal. The daemon restores the screen to
its original colors on exit.

```shell
$ xrandr-nightmode
$ killall -USR1 xrandr-nightmode # night mode turns on
$ killall -USR1 xrandr-nightmode # night mode turns off
...
$ killall xrandr-nightmode # terminates the daemon, turning night mode off if needed
```

Many window managers can be set up to send the signal on a key press. For
instance, when using i3, addding the following line to your configuration file
makes the XF86LaunchA multimedia key responsible for enabling/disabling night mode.

```
bindsym XF86 exec killall -USR1 xrandr-nightmode
```

# License

xrandr-nightmode is licensed under the GNU General Public License v3.

# Related projects

* [Redshift](http://jonls.dk/redshift/)
* [xrandr-invert-colors](https://github.com/zoltanp/xrandr-invert-colors)
* [f.lux](https://justgetflux.com/)
