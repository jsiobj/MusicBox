<!-- :::mermaid
stateDiagram-v2

    classDef choice fill:orange,color:#ffffff,stroke:black
    classDef playing fill:#00ff99,stroke:black
    classDef pause fill:#00ff99,stroke:#000000, color:#00995c
    classDef inactive fill:#eeeeee,stroke:#999999,color:#888888

    track_not_set:::inactive : Track not set
    track_set:::inactive : Track Set
    track_not_started:::inactive : Track not started
    track_finished:::inactive : Track is finished
    track_playing:::playing: Track playing
    track_paused:::pause : Track paused
    
    track_saved:::choice : Saved track ?
    autoplay:::choice : Autoplay enabled ?


    [*]                 --> track_not_set
    track_saved         --> track_not_set           : no
    track_set           --> track_not_set           : back
    track_not_set       --> track_saved             : auto (at startup)

    track_saved         --> track_set               : auto (at startup)
    track_paused        --> track_set               : new track
    track_not_set       --> track_set               : play/pause \n any track
    track_not_started   --> track_set               : new_track

    track_set           --> autoplay                : yes
    track_playing       --> track_paused            : play/pause \n current track
    autoplay            --> track_paused            : yes (at startup)
 
    autoplay            --> track_not_started       : no

    track_set           --> track_playing           : new track \n next track & autoplay
    track_not_started   --> track_playing           : play/pause \n current track
    track_paused        --> track_playing           : play/pause \n current track
 
    track_playing       --> track_finished          : end of track reached

    track_finished      --> autoplay
    autoplay            --> track_set               : yes
    autoplay            --> track_not_set           : yes (end of scope)
    autoplay            --> track_set               : no
::: -->

:::mermaid
flowchart LR

    classDef choice fill:orange,color:#ffffff,stroke:black
    classDef playing fill:#00ff99,stroke:black
    classDef pause fill:#00ff99,stroke:#000000, color:#00995c
    classDef inactive fill:#eeeeee,stroke:#999999,color:#888888
    classDef start fill:green,stroke:000000,color:#ffffff

    start((Start)):::start
    track_not_set[Track not set]:::inactive
    track_set[Track Set]:::inactive
    track_not_started[Track not started]:::inactive
    track_finished[Track is finished]:::inactive
    track_playing[Track playing]:::playing
    track_paused[Track paused]:::pause

    track_saved{{Saved track ?}}:::choice
    autoplay{{Autoplay enabled ?}}:::choice

    start                                                       --> track_not_set
    track_saved         -- no                                   --> track_not_set
    track_set           -- back                                 --> track_not_set
    track_not_set       -- auto (at startup)                    --> track_saved

    track_saved         -- auto (at startup)                    --> track_set
    track_paused        -- new track                            --> track_set
    track_not_set       -- play/pause \n any track              --> track_set
    track_not_started   -- new_track                            --> track_set

    track_set           -- yes                                  --> autoplay
    track_playing       -- play/pause \n current track          --> track_paused
    autoplay            -- yes (at startup)                     --> track_paused
 
    autoplay            -- no                                   --> track_not_started

    track_set           -- new track \n next track & autoplay   --> track_playing
    track_not_started   -- play/pause \n current track          --> track_playing
    track_paused        -- play/pause \n current track          --> track_playing
 
    track_playing       -- end of track reached --> track_finished

    track_finished      -- auto                                 --> autoplay
    autoplay            -- yes                                  --> track_set
    autoplay            -- yes (end of scope)                   --> track_not_set
    autoplay            -- no                                   --> track_set
:::