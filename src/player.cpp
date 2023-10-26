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
*/
#define PREFER_SDFAT_LIBRARY 1
#define DEBUG
#include "debug.h"

#include <Custom_NeoTrellis.h>
#include <Adafruit_VS1053.h>

//#include "misc.h"
#include "box.h"
#include "player.h"
#include "diag.h"

extern Box box;
extern MusicPlayer musicPlayer;
extern Custom_NeoTrellis trellis;
extern Adafruit_VS1053_FilePlayer vs1053FilePlayer;

// Button used as selector for lib, album or track
// Set to 255 if used as control (volume, next...)
uint8_t mapButton2Track[] = { 
    255, 255, 255, 255,
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
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    box.max9744IncreaseVolume();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback decreaseVol(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    box.max9744DecreaseVolume();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onBackPress(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    musicPlayer.navBack();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onPlayPausePress(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE)
    {
        case SEESAW_KEYPAD_EDGE_RISING:
            break;
        
        case SEESAW_KEYPAD_EDGE_FALLING:
            DEBUG_PRINT("Button released");
            musicPlayer.playPause();
            break;

        default:
            DEBUG_PRINT("No relevant event");
            break;
    }
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onPlayPauseLongPress(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onNextPress(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    musicPlayer.playNextTrack();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onPreviousPress(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    DEBUG_PRINT("playPreviousTrack not implemented (yet)");
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onTrackKeyPressed(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    musicPlayer.selectObject(button2track(event.bit.NUM));
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onDumpKeyPressed(keyEvent event)  {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    if(event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING) return 0;
    musicPlayer.dumpObject(true);
    return 0;
}

//=========================================================================
//=== Methods
//=========================================================================

//-------------------------------------------------------------------------
// Starting player
//-------------------------------------------------------------------------
void MusicPlayer::begin() { 
    DEBUG_PRINT("StartFunction");

    for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
        // trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
        // trellis.activateKey(i, SEESAW_KEYPAD_EDGE_LONGPRESS);
        // trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING,false);
        // trellis.activateKey(i, SEESAW_KEYPAD_EDGE_LOW,false);
        // trellis.activateKey(i, SEESAW_KEYPAD_EDGE_HIGH,false);
        trellis.registerCallback(i, onTrackKeyPressed);
        trellis.pixels.setPixelColor(i,0xFFFFFF);
    }

    trellis.registerCallback(BTN_ID_BACK, onBackPress);
    trellis.registerCallback(BTN_ID_PLAY_PAUSE, onPlayPausePress);
    trellis.registerCustomCallback(BTN_ID_PLAY_PAUSE, onPlayPauseLongPress);
    trellis.registerCallback(BTN_ID_NEXT, onNextPress);
    trellis.registerCallback(BTN_ID_BACK, onBackPress);
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

    loadLibraries();
    restoreIds();

    if( isLibrarySet() ) {
        DEBUG_PRINTF("Loading album in default library no %d", currentLibraryId);
        loadAlbums();
        if(isAlbumSet()) {
            DEBUG_PRINTF("Loading tracks in default album no %d", currentLibraryId);
            loadTracks();
            displayTracks();
        }
        else {
            displayAlbums();
        }
    }
    else {
        displayLibraries();
    }

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

    DEBUG_PRINT("End");
}

//-------------------------------------------------------------------------
// Load libraries from SD
//-------------------------------------------------------------------------
void MusicPlayer::loadLibraries() {

    DEBUG_PRINT("StartFunction");
    libraryCount = getLibraryList(MAX_LIBRARY_COUNT);
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Show libraries on keypad
//-------------------------------------------------------------------------
void MusicPlayer::displayLibraries() {

    DEBUG_PRINT("StartFunction");

    clearNav();
    for(byte libraryRef = 0; libraryRef < libraryCount; libraryRef++) {
        DEBUG_PRINTF("Enabling library %d",libraryRef);
        trellis.pixels.setPixelColor(track2button(libraryRef),COLOR_AQUA);
    }
    trellis.pixels.show();
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Load library content
//-------------------------------------------------------------------------
void MusicPlayer::loadAlbums() {
    DEBUG_PRINT("StartFunction");
    albumCount = getAlbumList(currentLibraryId, MAX_ALBUM_COUNT);
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Show library content
//-------------------------------------------------------------------------
void MusicPlayer::displayAlbums() {

    DEBUG_PRINT("StartFunction");

    clearNav();
    for(byte albumRef = 0; albumRef < albumCount; albumRef++) {
        DEBUG_PRINTF("Enabling album %d",albumRef);
        trellis.pixels.setPixelColor(track2button(albumRef),COLOR_NAVY);
    }
    trellis.pixels.show();
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Load album content
//-------------------------------------------------------------------------
void MusicPlayer::loadTracks() {

    DEBUG_PRINT("StartFunction");
    trackCount = getTrackList(currentLibraryId, currentAlbumId, MAX_TRACK_COUNT);
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Show album content
//-------------------------------------------------------------------------
void MusicPlayer::displayTracks() {

    DEBUG_PRINT("StartFunction");

    clearNav();
    for(byte trackRef = 0; trackRef < trackCount; trackRef++) {
        DEBUG_PRINTF("Enabling track %d",trackRef);
        if( trackRef == currentTrackId) {
            trellis.pixels.setPixelColor(track2button(trackRef),COLOR_PURPLE);
        }
        else {
            trellis.pixels.setPixelColor(track2button(trackRef),COLOR_LIGHTGREY);
        }
    }
    trellis.pixels.show();
    DEBUG_PRINT("ExitFunction");
}

//-------------------------------------------------------------------------
// Play one track
//-------------------------------------------------------------------------
void MusicPlayer::playTrack() {
    DEBUG_PRINT("StartFunction");

    if(vs1053FilePlayer.playingMusic) {
        DEBUG_PRINTF("Stop track:%d %s",currentTrackId,tracks[currentTrackId]);
        vs1053FilePlayer.stopPlaying();
        trellis.pixels.setPixelColor(track2button(currentTrackId),COLOR_PINK);
    }
    DEBUG_PRINTF("Start track:%d %s",currentTrackId,tracks[currentTrackId]);
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

//-------------------------------------------------------------------------
// Select object by its ID depending where we are in nav tree
//-------------------------------------------------------------------------
void MusicPlayer::selectObject(uint8_t id) {
    DEBUG_PRINT("Start");

    uint8_t _currentLevel = currentLevel();

    if( _currentLevel <= LEVEL_ALBUM ) {
    DEBUG_PRINT("At album level");
        if(currentTrackId == id) {
            DEBUG_PRINT("Key pressed match current playing track : play/pause");
            if(vs1053FilePlayer.stopped()) {
                DEBUG_PRINT("Starting track");
                playTrack();
            } 
            else {
                if(vs1053FilePlayer.paused()) {
                    DEBUG_PRINT("Paused, restarting");
                    unpauseTrack();
                }
                else {
                    DEBUG_PRINT("Playing, pausing");
                    pauseTrack();
                }
            }
        }
        else {
            if(id < trackCount) {
                vs1053FilePlayer.stopPlaying();
                setTrackId(id);
                playTrack();
                DEBUG_PRINT("ExitFunction | old track stopped, new track playing");  
            }
            else {
                DEBUG_PRINT("ExitFunction, Track Id out of bound");  
            }
        }
    }

    if( _currentLevel == LEVEL_LIBRARY) {
    DEBUG_PRINT("At library level");
        if(id < albumCount) {
            setAlbumId(id);
            loadTracks(); displayTracks();
            DEBUG_PRINT("ExitFunction, album id set");  
            return;
        }
        else {
            DEBUG_PRINT("ExitFunction, Album Id out of bound");  
        }
    }

    if( _currentLevel == LEVEL_ROOT ) {
    DEBUG_PRINT("At root level");
        if(id < libraryCount) {
            setLibraryId(id);
            saveParam("library",currentLibraryId);
            loadAlbums();
            displayAlbums();
            DEBUG_PRINT("ExitFunction, library set");  
            return;
        }
        else {
            DEBUG_PRINT("ExitFunction, Library Id out of bound");  
        }
    }
}

//-------------------------------------------------------------------------
// Back in nav tree
//-------------------------------------------------------------------------
void MusicPlayer::navBack() {

    autoPlayNext = false;
    playScope = SCOPE_NONE;
    
    uint8_t _currentLevel = currentLevel();

    if(_currentLevel <= LEVEL_ALBUM) {
        vs1053FilePlayer.stopPlaying();
        unsetTrackId();
        unsetAlbumId();
        displayAlbums();
        DEBUG_PRINT("ExitFunction, unset track id and album id");
    }

    if(_currentLevel == LEVEL_LIBRARY) {
        unsetAlbumId();
        unsetLibraryId();
        displayLibraries();
        DEBUG_PRINT("ExitFunction,unset album id and library id");
    }

    saveAllParams();

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
                setTrackId(0);
                setAlbumId(0);
                setLibraryId(0);

                DEBUG_PRINT("1st track in 1st album in first library");
                loadAlbums();
                loadTracks();
                displayTracks();
            }
            else {                            // Library loaded
                setTrackId(0);
                setAlbumId(0);

                DEBUG_PRINT("1st track in 1st album in current library");
                loadTracks();
                displayTracks();
            }
        }
        else {                               // Album loaded (but not track)
            DEBUG_PRINT("1st track in current album");
            setTrackId(0);
        }
    }
    else {                                               // Track loaded
        if(currentTrackId+1<trackCount) {                // Track loaded & Next track available in the current album
            DEBUG_PRINT("Next track in current album")
            setTrackId(currentTrackId+1);                           
        }          
        else {                                           // No more track in album
                      
            if(playScope == SCOPE_ALBUM) {               // No more track in album & scope is album
                DEBUG_PRINT("ExitFunction,No more tracks, album scope");
                return false;           
            }          
          
            if(currentAlbumId+1<albumCount) {            // No more track in album, next album available and scope > album
                setTrackId(0);    
                setAlbumId(currentAlbumId+1);
                DEBUG_PRINT("1st track in next album in current library")
                loadTracks();
                displayTracks();
            }
            else {                                       // No more track in album, no more album available 

                if(playScope == SCOPE_LIBRARY) {         // No more album in library & scope is library
                    DEBUG_PRINT("ExitFunction,No more albums, library scope");
                    return false;
                }

                if(currentLibraryId+1 < libraryCount) {    // No more album in library & scope is all
                    setTrackId(0);
                    setAlbumId(0);
                    setLibraryId(currentLibraryId+1);
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
void MusicPlayer::clearNav() {
    for(uint8_t i=0;i < NEO_TRELLIS_NUM_KEYS; i++) {
        if(mapButton2Track[i] != 255) {
            trellis.pixels.setPixelColor(i, COLOR_BLACK);
        }
    }
}

//-------------------------------------------------------------------------
// Stores SD Card directory entries in an array
// Returns entry count
//-------------------------------------------------------------------------
byte MusicPlayer::readSD(char* path, char entries[][MAX_PATH_LENGTH], bool isDirectory, byte maxEntries) {
    
    DEBUG_PRINTF("StartFunction,path:%s,isDirectory:%d,maxEntries:%d",path,isDirectory,maxEntries);
    FsFile dir;
    byte count = 0;

    // Resetting all entries
    for(uint8_t i=0;i< maxEntries;i++) {
        entries[i][0] = 0;
    }

    dir.open(path);

    // #ifdef DEBUG
    // dir.ls(&Serial);
    // #endif

    while(true) {

        if(count>=maxEntries) {
            DEBUG_PRINT("Too many entries, ignoring remaining");
            break;
        }
    
        FsFile dirEntry;
        dirEntry.openNext(&dir, O_RDONLY);
        
        if(!dirEntry) {
            DEBUG_PRINT("No more entries");
            break;
        }

        char fileNameBuffer[MAX_PATH_LENGTH];
        dirEntry.getName(fileNameBuffer, MAX_PATH_LENGTH);

        if(dirEntry.isDir() != isDirectory) {
            DEBUG_PRINTF("Entry %s is not of expected type %d, ignoring",fileNameBuffer,isDirectory);
            continue;
        }

        strcpy(entries[count],path); 
        strcat(entries[count],"/"); 
        strcat(entries[count],fileNameBuffer);
        DEBUG_PRINTF("Adding entry %d %s",count,entries[count]);

        count++;
        dirEntry.close();
    }

    dir.close();
    DEBUG_PRINTF("ExitFunction,count:%d,path:%s",count,path)
    return count;
}

//-------------------------------------------------------------------------
// Get library list from SD (list of directories in music path)
//-------------------------------------------------------------------------
uint8_t MusicPlayer::getLibraryList(uint8_t maxEntries) {
    return readSD(musicPath, libraries, true, MAX_LIBRARY_COUNT);
}

//-------------------------------------------------------------------------
// Get album list from SD in specified library
//-------------------------------------------------------------------------
uint8_t MusicPlayer::getAlbumList(uint8_t libraryId, uint8_t maxEntries) {
    return readSD(libraries[libraryId], albums, true, MAX_ALBUM_COUNT);
}

//-------------------------------------------------------------------------
// Get Track list from SD in specified album
//-------------------------------------------------------------------------
uint8_t MusicPlayer::getTrackList(uint8_t libraryId, uint8_t albumId, uint8_t maxEntries) {
    return readSD(albums[albumId], tracks, false, MAX_TRACK_COUNT);
}

//-------------------------------------------------------------------------
// Display debug info
//-------------------------------------------------------------------------
void MusicPlayer::dumpObject(bool dumpArrays) {

    DEBUG_PRINT("=======================================");
    if(dumpArrays) {
        DEBUG_PRINT("--------------------------------------");
        DEBUG_PRINT_ARRAY(libraries, "libraries", MAX_ALBUM_COUNT);
        DEBUG_PRINT_ARRAY(albums, "albums", MAX_ALBUM_COUNT);
        DEBUG_PRINT_ARRAY(tracks, "tracks", MAX_TRACK_COUNT);
    }

    DEBUG_PRINT("=======================================");
    DEBUG_PRINTF("vs1053 volume         %d", box.vs1053_volume);
    DEBUG_PRINTF("max9744 volume        %d", box.max9744_volume);
    DEBUG_PRINTF("musicPath              %s", musicPath );
    DEBUG_PRINTF("libraryCount          %d", libraryCount);
    DEBUG_PRINTF("albumCount            %d", albumCount);
    DEBUG_PRINTF("trackCount            %d", trackCount);
    DEBUG_PRINTF("currentLibraryId      %d", currentLibraryId);
    DEBUG_PRINTF("currentAlbumId        %d", currentAlbumId);
    DEBUG_PRINTF("currentTrackId        %d", currentTrackId);
    DEBUG_PRINTF("autoPlayNext          %d", autoPlayNext);
    DEBUG_PRINTF("currentLevel()        %d", currentLevel());
    DEBUG_PRINT("=======================================");
    DEBUG_PRINT("State files");
    DEBUG_PRINTF("library               %d", getParam("LIBRARY"));
    DEBUG_PRINTF("album                 %d", getParam("ALBUM"));
    DEBUG_PRINTF("track                 %d", getParam("TRACK"));
    DEBUG_PRINT("=======================================");
}

//-------------------------------------------------------------------------
// Saving a single param
//-------------------------------------------------------------------------
void MusicPlayer::saveParam(const char *paramName, uint8_t paramValue) {
    DEBUG_PRINT("Start");
    char fullPath[27];

    if(not SD.exists(statePath)) {
        DEBUG_PRINT("State dir does not exists, creating");
        SD.mkdir(statePath);
    }

    strcpy(fullPath, statePath);
    strcat(fullPath, paramName);

    //SD.remove(fullPath);

    //char paramStr[4];
    //itoa(paramValue,paramStr,10);

    DEBUG_PRINTF("Saving %d in %s", paramValue, fullPath);
    File f = SD.open(fullPath,O_WRITE  | O_CREAT);
    if(f) {
        DEBUG_PRINT("Writing...");
        char str[2] = { paramValue, 0  };
        f.write(str);
        DEBUG_PRINT("Closing...");
        f.close();
    }
    else {
        DEBUG_PRINTF("Cannot openfile %s for writing", fullPath)
    }

}

//-------------------------------------------------------------------------
// Get a previously saved param
//-------------------------------------------------------------------------
uint8_t MusicPlayer::getParam(const char *paramName) {
    char fullPath[27];
    strcpy(fullPath, statePath);
    strcat(fullPath, paramName);
    char str[2];

    if(not SD.exists(fullPath)) {
        DEBUG_PRINTF("No param file %s found", fullPath);
        return NO_KEY;
    }
    else {
        File f = SD.open(fullPath,FILE_READ);
        if(f) {
            f.read(str,1);        
            //uint8_t paramValue = atoi(paramStr);
            DEBUG_PRINTF("Got %d as %s value in %s", str[0], paramName, fullPath);
            return  str[0];
        }
        else {
            DEBUG_PRINTF("Cannot open file %s", fullPath);
            return 255;
        }
    }
}

//-------------------------------------------------------------------------
// Saving a bunch of params
//-------------------------------------------------------------------------
void MusicPlayer::saveAllParams() {
    DEBUG_PRINT("Start");
    saveParam("TRACK", currentTrackId);
    saveParam("ALBUM", currentAlbumId);
    saveParam("LIBRARY", currentLibraryId);
}

