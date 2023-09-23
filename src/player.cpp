/*
    Interacting Objects - Noe's Juke Box Project
    http://www.interactingobjects.com

    Project page : http://interactingobjects.com/category/projects/juke-box/

    ---------------------------------------------------------------------------
    The MIT License (MIT)

    Copyright (c) 2014 Interacting Objects (http://interactingobjects.com)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
    ---------------------------------------------------------------------------

    Code for Player mode
*/
#define DEBUG
#include "debug.h"

#include "Adafruit_NeoTrellis.h"
#include "Adafruit_VS1053.h"

//#include "misc.h"
#include "box.h"
#include "player.h"

extern Box box;
extern MusicPlayer musicPlayer;
extern Adafruit_NeoTrellis trellis;
extern Adafruit_VS1053_FilePlayer vs1053FilePlayer;

uint8_t mapButton2Track[] = { 255, 255, 255, 255,
                                0,   1,   2,   3,
                                4,   5,   6,   7,
                              255,   8,   9, 255
                            }; 


uint8_t button2track(uint8_t button) {
    return mapButton2Track[button];
}

uint8_t track2button(uint8_t track) {
    for(int i=0;i<NEO_TRELLIS_NUM_KEYS;i++) 
        if(mapButton2Track[i]==track) 
            return i;

    return 255;
}

//=========================================================================
//=== CALLBACKS
//=========================================================================

//-------------------------------------------------------------------------
TrellisCallback increaseVol(keyEvent event) {
    DEBUG_PRINT("StartFunction");
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    box.increaseVolume();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback decreaseVol(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    box.decreaseVolume();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onButtonBackPressed(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    musicPlayer.navBack();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onButtonPlayPausePressed(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    musicPlayer.playPause();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onButtonNextPressed(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    musicPlayer.playNextTrack();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onButtonPreviousPressed(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    DEBUG_PRINT("playPreviousTrack not implemented (yet)");
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onTrackKeyPressed(keyEvent event) {
    DEBUG_PRINT("StartFunction");
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    musicPlayer.selectObject(button2track(event.bit.NUM));
    return 0;
}

#ifdef DEBUG
TrellisCallback onDumpKeyPressed(keyEvent event)  {
    DEBUG_PRINT("StartFunction");
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    musicPlayer.dumpObject(true);
    return 0;
}
#endif

//=========================================================================
//=== Methods
//=========================================================================

//-------------------------------------------------------------------------
// Starting player
//-------------------------------------------------------------------------
void MusicPlayer::begin() { 
    DEBUG_PRINT("StartFunction");

    for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
        trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
        // trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING,false);
        // trellis.activateKey(i, SEESAW_KEYPAD_EDGE_LOW,false);
        // trellis.activateKey(i, SEESAW_KEYPAD_EDGE_HIGH,false);
        trellis.registerCallback(i, onTrackKeyPressed);
        trellis.pixels.setPixelColor(i,0xFFFFFF);
    }

    trellis.registerCallback(BTN_ID_BACK, onButtonBackPressed);
    trellis.registerCallback(BTN_ID_PLAY_PAUSE, onButtonPlayPausePressed);
    trellis.registerCallback(BTN_ID_NEXT, onButtonNextPressed);
    trellis.registerCallback(BTN_ID_BACK, onButtonBackPressed);
    trellis.registerCallback(BTN_ID_VOL_UP, increaseVol);
    trellis.registerCallback(BTN_ID_VOL_DOWN, decreaseVol);

    trellis.pixels.setPixelColor(BTN_ID_BACK,COLOR_ORANGE);
    trellis.pixels.setPixelColor(BTN_ID_PLAY_PAUSE,COLOR_RED);
    trellis.pixels.setPixelColor(BTN_ID_NEXT,COLOR_BLUE);
    trellis.pixels.setPixelColor(BTN_ID_NEXT_PAGE,COLOR_PURPLE);
    trellis.pixels.setPixelColor(BTN_ID_VOL_UP,COLOR_BLUE);
    trellis.pixels.setPixelColor(BTN_ID_VOL_DOWN,COLOR_BLUE);

#ifdef DEBUG
    trellis.pixels.setPixelColor(BTN_ID_DUMP, COLOR_RED);
    trellis.registerCallback(BTN_ID_DUMP, onDumpKeyPressed);
    mapButton2Track[BTN_ID_DUMP] = 255;
#endif

    trellis.pixels.show();

    //strcpy(rootPath, path);
    loadLibraries();
    displayLibraries();
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Player main loop
//-------------------------------------------------------------------------
void MusicPlayer::loop() {

    DEBUG_PRINT("StartFunction");
    
    while(true) {
        trellis.read();

        if(vs1053FilePlayer.playingMusic) {
            vs1053FilePlayer.feedBuffer();
        }
        else {
            if(autoPlayNext && !vs1053FilePlayer.paused()) {
                if(setNextTrack()) playTrack();
                else autoPlayNext = false;
            }
        }
    }
}

//-------------------------------------------------------------------------
// Load libraries from SD
//-------------------------------------------------------------------------
void MusicPlayer::loadLibraries() {

    DEBUG_PRINT("StartFunction");
    libraryCount = readSD(rootPath, libraries, true, MAX_LIBRARY_COUNT);
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Show libraries on keypad
//-------------------------------------------------------------------------
void MusicPlayer::displayLibraries() {

    DEBUG_PRINT("StartFunction");

    for(byte libraryRef = 0; libraryRef < libraryCount; libraryRef++) {
        DEBUG_PRINTF("Enabling library %d",libraryRef);
        trellis.pixels.setPixelColor(track2button(libraryRef),COLOR_GREEN);
    }
    trellis.pixels.show();
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Show library content
//-------------------------------------------------------------------------
void MusicPlayer::loadAlbums() {
    DEBUG_PRINT("StartFunction");
    albumCount = readSD(libraries[currentLibraryId], albums, true, MAX_ALBUM_COUNT);
    DEBUG_PRINT("ExitFunction");
}

void MusicPlayer::displayAlbums() {

    DEBUG_PRINT("StartFunction");

    for(byte albumRef = 0; albumRef < albumCount; albumRef++) {
        DEBUG_PRINTF("Enabling album %d",albumRef);
        trellis.pixels.setPixelColor(track2button(albumRef),COLOR_GREEN);
    }
    trellis.pixels.show();
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Show album content
//-------------------------------------------------------------------------
void MusicPlayer::loadTracks() {

    DEBUG_PRINT("StartFunction");
    trackCount = readSD(albums[currentAlbumId], tracks, false, MAX_TRACK_COUNT);
    DEBUG_PRINT("ExitFunction");
}

void MusicPlayer::displayTracks() {

    DEBUG_PRINT("StartFunction");

    for(byte trackRef = 0; trackRef < trackCount; trackRef++) {
        DEBUG_PRINTF("Enabling track %d",trackRef);
        trellis.pixels.setPixelColor(track2button(trackRef),COLOR_ORANGE);
    }
    trellis.pixels.show();
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Play one track
//-------------------------------------------------------------------------
void MusicPlayer::playTrack() {
    DEBUG_PRINT("StartFunction");

    DEBUG_PRINTF("Track track:%d %s",currentTrackId,tracks[currentTrackId]);
    if(vs1053FilePlayer.playingMusic) {
        vs1053FilePlayer.stopPlaying();
    }
    vs1053FilePlayer.startPlayingFile(tracks[currentTrackId]);
    trellis.pixels.setPixelColor(track2button(currentTrackId),COLOR_PURPLE);
    trellis.pixels.show();
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Pause
//-------------------------------------------------------------------------
void MusicPlayer::pauseTrack() {
    DEBUG_PRINT("StartFunction");
    vs1053FilePlayer.pausePlaying(true);
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Play
//-------------------------------------------------------------------------
void MusicPlayer::unpauseTrack() {
    DEBUG_PRINT("StartFunction");
    vs1053FilePlayer.pausePlaying(false);
    trellis.pixels.setPixelColor(track2button(currentTrackId),COLOR_PURPLE);
    trellis.pixels.show();
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Play / Pause
//-------------------------------------------------------------------------
void MusicPlayer::playPause() {

    DEBUG_PRINT("StartFunction");  
    if(vs1053FilePlayer.stopped() || vs1053FilePlayer.paused()) autoPlayNext = true;
    else autoPlayNext = false;

    if(playScope == SCOPE_NONE) {
        if(currentTrackId != NO_KEY || currentAlbumId != NO_KEY) playScope = SCOPE_ALBUM;
        else if(currentLibraryId != NO_KEY)                      playScope = SCOPE_LIBRARY;
        else                                                     playScope = SCOPE_ALL;
    }
    // A track is already selected, play it or unpause it
    if( currentTrackId != NO_KEY  ) {
        if(vs1053FilePlayer.stopped()) playTrack();   // Current track not playing (finished or never started)
        if(vs1053FilePlayer.paused())  unpauseTrack();
        else                           pauseTrack();
    }
    // No track is selected, playing the next track in scope
    else {
        setNextTrack();
        playTrack();
    }
    DEBUG_PRINTF("ExitFunction,scope:%d",playScope);
}

void MusicPlayer::selectObject(uint8_t id) {

    // Key pressed match current playing track
    if(currentTrackId == id) {

        DEBUG_PRINT("Key pressed match current playing track : play/pause");
        if(vs1053FilePlayer.stopped()) playTrack(); // Current track not playing (finished)
        if(vs1053FilePlayer.paused())  unpauseTrack();
        else                           pauseTrack();
    
    }
    else {

        // There is another track playing, stopping and playing the new one
        if(currentTrackId != NO_KEY) {
            if(id < trackCount) {
                vs1053FilePlayer.stopPlaying();
                currentTrackId = id;
                playTrack();
            }
            DEBUG_PRINT("ExitFunction, old track stopped, new track playing");  
        }

        // No track is active, but album is, playing selected track
        if(currentAlbumId != NO_KEY) {
            if(id<trackCount) {
                if( currentTrackId != id) {
                    currentTrackId = id;
                    playTrack();
                }
            }
            DEBUG_PRINT("ExitFunction, selected track playing");  
        }

        // No album is active, but library is, loading selected album
        if(currentLibraryId != NO_KEY) {
            if(id < albumCount) {
                if(currentAlbumId != id) {
                    currentAlbumId = id;
                    loadTracks();
                    displayTracks();
                }
            }
            DEBUG_PRINT("ExitFunction, library set");  
        }
 
        // No library active, loading selected library
        if(id < libraryCount) {
            currentLibraryId = id;
            loadAlbums();
            displayAlbums();
            DEBUG_PRINT("ExitFunction, library set");  
        }
    }
}

void MusicPlayer::navBack() {

    autoPlayNext = false;
    playScope = SCOPE_NONE;

    if(currentTrackId != NO_KEY) {
        vs1053FilePlayer.stopPlaying();
        currentTrackId = NO_KEY;
        displayTracks();
        DEBUG_PRINT("ExitFunction, unset trackid");
    }

    if(currentAlbumId != NO_KEY) {
        currentAlbumId = NO_KEY;
        displayAlbums();
        DEBUG_PRINT("ExitFunction,unset album id");
    }

    if(currentLibraryId != NO_KEY) {
        currentLibraryId = NO_KEY;
        displayLibraries();
        DEBUG_PRINT("ExitFunction,unset library id");
    }

    DEBUG_PRINT("ExitFunction,can't go back more !");
}

//-------------------------------------------------------------------------
// Playing next track
//-------------------------------------------------------------------------
void MusicPlayer::playNextTrack() {
    DEBUG_PRINT("StartFunction");
    vs1053FilePlayer.stopPlaying();
    if(setNextTrack()) playTrack();
}

//-------------------------------------------------------------------------
// Setting next track
//-------------------------------------------------------------------------
bool MusicPlayer::setNextTrack() {
    
    DEBUG_PRINTF("StartFunction,scope %d",playScope);

    if(currentTrackId == NO_KEY) {
        if(currentAlbumId == NO_KEY) {
            if(currentLibraryId == NO_KEY) {  // Nothing loaded yet
                currentTrackId = 0;
                currentAlbumId = 0;
                currentLibraryId = 0;

                DEBUG_PRINT("1st track in 1st album in first library");
                loadAlbums();
                loadTracks();
                displayTracks();
            }
            else {                            // Library loaded
                currentTrackId = 0;
                currentAlbumId = 0;

                DEBUG_PRINT("1st track in 1st album in current library");
                loadTracks();
                displayTracks();
            }
        }
        else {                               // Album loaded (but not track)
            DEBUG_PRINT("1st track in current album");
            currentTrackId = 0;
        }
    }
    else {                                               // Track loaded
        if(currentTrackId+1<trackCount) {                // Track loaded & Next track available in the current album
            DEBUG_PRINT("Next track in current album")
            currentTrackId++;                           
        }          
        else {                                           // No more track in album
                      
            if(playScope == SCOPE_ALBUM) {               // No more track in album & scope is album
                DEBUG_PRINT("ExitFunction,No more tracks, album scope");
                return false;           
            }          
          
            if(currentAlbumId+1<albumCount) {            // No more track in album, next album available and scope > album
                currentTrackId = 0;    
                currentAlbumId++;
                DEBUG_PRINT("1st track in next album in current library")
                loadTracks();
                displayTracks();
            }
            else {                                       // No more track in album, no more album available 

                if(playScope == SCOPE_LIBRARY) {         // No more album in library & scope is library
                    DEBUG_PRINT("ExitFunction,No more albums, library scope");
                    return false;
                }

                if(currentLibraryId+1<libraryCount) {    // No more album in library & scope is all
                    currentTrackId = 0;
                    currentAlbumId = 0;
                    currentLibraryId++;
                    DEBUG_PRINT("1st track in first album in next library")
                    loadAlbums();
                    loadTracks();
                    displayTracks();
                }
            }
        }
    }

    return true;
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Setting volume
//-------------------------------------------------------------------------
// void MusicPlayer::setVolume(byte volumeDiff) {
//     DEBUG_PRINTF("StartFunction,volumeDiff:%d",volumeDiff);

//     // byte redValue,greenValue,blueValue;
//     byte tempVol;

//     tempVol = volume - volumeDiff;
//     if(tempVol<MAX_VOLUME)      volume=MAX_VOLUME; 
//     else if(tempVol>MIN_VOLUME) volume=MIN_VOLUME;
//     else                        volume=tempVol;

//     vs1053FilePlayer.setVolume(volume,volume);     // Left and right channel volume (lower number mean louder)
//     DEBUG_PRINTF("Volume set to %d",volume);

//     // if(volume<MID_VOLUME) {
//     //     redValue=0;
//     //     greenValue=((MID_VOLUME-volume)*255.0)/(MID_VOLUME-MAX_VOLUME);
//     //     blueValue=((MID_VOLUME-volume)*255.0)/(MID_VOLUME-MAX_VOLUME);
//     // }
//     // else {
//     //     blueValue=0;
//     //     redValue=(255-((MIN_VOLUME-volume)*255.0)/(MIN_VOLUME-MID_VOLUME));
//     //     greenValue=(255-((MIN_VOLUME-volume)*255.0)/(MIN_VOLUME-MID_VOLUME));
//     // }

//     // DEBUG_PRINTF3("RGB",redValue,greenValue,blueValue);
//     // analogWrite(ENCODER_RED,redValue);
//     // analogWrite(ENCODER_GREEN,greenValue);
//     // analogWrite(ENCODER_BLUE,blueValue);

//     DEBUG_PRINT("ExitFunction");
// }

//-------------------------------------------------------------------------
// Read SD Card to build music library
// Returns entry count
//-------------------------------------------------------------------------
byte MusicPlayer::readSD(char* path, char entries[][MAX_PATH_LENGTH], bool isDirectory, byte maxEntries) {
    
    DEBUG_PRINTF("StartFunction,path:%s,isDirectory:%d,maxEntries:%d",path,isDirectory,maxEntries);

    File root = SD.open(path);
    byte count = 0;
    
    while(true) {

        if(count>=maxEntries) {
            DEBUG_PRINT("Too many entries, ignoring remaining");
            break;
        }
    
        File dirEntry = root.openNextFile();
        
        if(!dirEntry) {
            DEBUG_PRINT("No more entries");
            break;
        }


        if(dirEntry.isDirectory() != isDirectory) {
            DEBUG_PRINTF("Entry %s is not of expected type %d, ignoring",dirEntry.name(),isDirectory);
            continue;
        }

        strcpy(entries[count],path); 
        strcat(entries[count],"/"); 
        strcat(entries[count],dirEntry.name());
        DEBUG_PRINTF("Adding entry %d %s",count,entries[count]);

        count++;
        dirEntry.close();
    }

    DEBUG_PRINTF("ExitFunction,count:%d,path:%s",count,path)
    return count;
}

void MusicPlayer::dumpObject(bool dumpArrays) {

    DEBUG_PRINT("=======================================");
    DEBUG_PRINTF("volume                %d", volume);
    DEBUG_PRINTF("rootPath              %s", rootPath );
    DEBUG_PRINTF("libraryCount          %d", libraryCount);
    DEBUG_PRINTF("albumCount            %d", albumCount);
    DEBUG_PRINTF("trackCount            %d", trackCount);
    DEBUG_PRINTF("currentLibraryId      %d", currentLibraryId);
    DEBUG_PRINTF("currentAlbumId        %d", currentAlbumId);
    DEBUG_PRINTF("currentTrackId        %d", currentTrackId);

    if(dumpArrays) {
    DEBUG_PRINT("--------------------------------------");
        DEBUG_PRINT_ARRAY(libraries, "libraries", MAX_ALBUM_COUNT);
        DEBUG_PRINT_ARRAY(albums, "albums", MAX_ALBUM_COUNT);
        DEBUG_PRINT_ARRAY(tracks, "tracks", MAX_TRACK_COUNT);
    }
    DEBUG_PRINT("=======================================");
    
}