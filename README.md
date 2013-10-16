# FMC (FMD Client)

This is a fork of the original [FMC by hzqtc](https://github.com/hzqtc/fmc). It works best with my [fork](https://github.com/lynnard/fmd) of FMD.

## Extension 

Some more fine-grained controls are added (only showing new commands)

           Usage: fmc [-a address] [-p port] [cmd] [argument]
                  fmc info [format] - show current fmd information
                                      if the format argument is given, the following specifier will be replaced accordingly
                                      %%a -- artist 
                                      %%t -- song title 
                                      %%b -- album 
                                      %%y -- release year 
                                      %%i -- cover image 
                                      %%d -- douban url 
                                      %%c -- channel 
                                      %%p -- currtime 
                                      %%l -- totaltime 
                                      %%u -- status 
                                      %%k -- kbps 
                                      %%r -- rate (0 or 1) 
                  fmc website       - open the douban page using the browser defined in $BROWSER
                  fmc kbps <kbps>   - set music quality to the specified kbps
                  fmc launch        - tell fmd to restart


`launch` command will forcefully kill all fmd instances and restart a new one (I know this is bad practice for a client, but I find it useful from time to time). 

`website` and `kbps` commands can refer to [FMD](https://github.com/lynnard/fmd)

`info` command now supports an optional format argument that can contain specifiers related to the current player information. An example use:

    $ fmc info 'Status: %u; Artist: %a; Title: %t'
    Status: pause; Artist: 菅野よう子; Title: アイモ O.C.

To support backslash sequences, add a dollar sign in front of the argument

    $ fmc info $'Status: %u\nArtist: %a\nTitle: %t'
    Status: pause
    Artist: 菅野よう子
    Title: アイモ O.C.

## Lyrics

A script demonstrating the use of the new `info` command can be found [here](fmclrc). It just outputs the lyrics line by line according to the current progression of the song. There is also an option to turn on *look beyond* mode, which is useful for showing the next line of lyrics.

## End result?

I'm a XMonad/Dzen2 guy and here's my music status bar (for the curious)

![](screenshots/bar.png)
