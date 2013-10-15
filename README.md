# FMC (FMD Client)

Basic CLI client for [FMD](https://github.com/hzqtc/fmd). Just like MPC for MPD.

## Command Line

	fmc [-a xxx.xxx.xxx.xxx] [-p xxx] [command]

Options: -a for FMD address, -p for FMD port.

Command: one of "play", "stop", "pause", "toggle", "skip", "ban", "rate", "unrate", "info" and "end".

## Extension 

On top of the original [FMC by hzqtc](https://github.com/hzqtc/fmc), some more fine-grained controls are added

    Usage: fmc [-a address] [-p port] [cmd] [argument]
           fmc help          - show this help infomation
           fmc info [format] - show current fmd information
                               if the format argument is given, the following specifier will be replaced accordingly
                               %a -- artist 
                               %t -- song title 
                               %c -- channel 
                               %p -- currtime 
                               %l -- totaltime 
                               %u -- status 
                               %k -- kbps 
                               %r -- rate (0 or 1) 
           fmc play          - start playback
           fmc pause         - pause playback
           fmc toggle        - toggle between play and pause
           fmc stop          - stop playback
           fmc skip/next     - skip current song
           fmc ban           - don't ever play current song again
           fmc rate          - mark current song as "liked"
           fmc unrate        - unmark current song
           fmc channels      - list all FM channels
           fmc setch <id>    - set channel through channel's id
           fmc kbps <kbps>   - set music quality to the specified kbps
           fmc launch        - tell fmd to restart
           fmc end           - tell fmd to quit

As you can probably see, a new `launch` command is added. It will forcefully kill all fmd instances and restart a new one (I know this is quite hacky since as a client fmc shouldn't do such thing to the server, but I find it useful for my day to day use). 

The `info` command now supports an optional format argument that can contain specifiers related to the current player information. An example use:
    
    $ fmc info 'Status: %u; Artist: %a; Title: %t'
    Status: pause; Artist: 菅野よう子; Title: アイモ O.C.

To support backslash sequences, add a dollar sign in front of the argument

    $ fmc info $'Status: %u\nArtist: %a\nTitle: %t'
    Status: pause
    Artist: 菅野よう子
    Title: アイモ O.C.
