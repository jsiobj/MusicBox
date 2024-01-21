:::mermaid
flowchart

    classDef choice fill:orange,color:#ffffff,stroke:black
    classDef playing fill:#00ff99,stroke:black
    classDef pause fill:#00ff99,stroke:#000000, color:#00995c
    classDef inactive fill:#eeeeee,stroke:#999999,color:#888888
    classDef start fill:green,stroke:000000,color:#ffffff

    start((Start)):::start
    
    track_not_set["`Lib not set
    Album not set
    track not set
    scope global`"]:::inactive
    
    track_set[Track Set]:::inactive
    track_not_started[Track not started]:::inactive
    track_finished[Track is finished]:::inactive
    track_playing[Track playing]:::playing
    track_paused[Track paused]:::pause

    track_saved{{Saved track ?}}:::choice
    autoplay{{Autoplay enabled ?}}:::choice

    start                                                       .-> track_not_set
    track_saved         -- no                                   --> track_not_set
    track_set           -- back                                 --> track_not_set
    track_not_set       -. auto (at startup)                    .-> track_saved

    track_saved         -. yes                                  .-> track_set
    track_paused        -- new track                            --> track_set
    track_not_set       -- play/pause \n any track              --> track_set
    track_not_started   -- new_track                            --> track_set

    track_set           -- yes                                  --> autoplay
    track_playing       -- play/pause \n current track          --> track_paused
    autoplay            -- yes (at startup)                     --> track_paused
 
    autoplay            -- no                                   --> track_not_started

    track_set           -- new track                            --> track_playing
    track_set           -. next track & autoplay                .-> track_playing
    track_not_started   -- play/pause \n current track          --> track_playing
    track_paused        -- play/pause \n current track          --> track_playing
 
    track_playing       -- end of track reached --> track_finished

    track_finished      -. auto                                 .-> autoplay
    autoplay            -. yes                                  .-> track_set
    autoplay            -. yes (end of scope)                   .-> track_not_set
    autoplay            -- no                                   --> track_set

    linkStyle default color:blue,stroke:red,lineColor:red;
    linkStyle 2 color:red
:::