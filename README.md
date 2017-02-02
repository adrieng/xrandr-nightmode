# Overview

This utility imitates the "night mode" feature of f.lux, which changes the color
of your screen, basically turning white into black and everything else into
shades of red. This makes reading on a computer screen late at night much more
enjoyable in my experience.

This is very simple code which reuses the hard work of the Redshift project as
well as Zoltan Padrah's xrandr-invert-colors. Praise to them.

# Compiling and installing

You need the XRandR development file and a C compiler. Typing `make install`
will copy the binary to $(HOME)/bin/xrandr-nightmode by default.

# Using

When started, xrandr-nightmode will silently fork and run in the background by
default. To turn night mode on or off, send it a USR1 signal. The daemon
restores the screen to its original colors on exit.

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

To keep xrandr-nightmode in the foreground, run it with `-f`. To turn night mode
on by default, run it with `-n`.

# License

xrandr-nightmode is licensed under the GNU General Public License v3.

# Related projects

* [Redshift](http://jonls.dk/redshift/)
* [xrandr-invert-colors](https://github.com/zoltanp/xrandr-invert-colors)
* [f.lux](https://justgetflux.com/)
