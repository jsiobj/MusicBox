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

  Player mode header

*/

#ifndef PLAYER_H
#define PLAYER_H

#include <Arduino.h>
#include "box.h"

#define MAX_LIBRARY_COUNT      9
#define MAX_ALBUM_COUNT        9
#define MAX_TRACK_COUNT        9

#define MAX_NAV_KEY  9

#define MAX_PATH_LENGTH 64

// Main Buttons
#define BTN_ID_PREVIOUS   0
#define BTN_ID_PLAY_PAUSE 1
#define BTN_ID_NEXT       2
#define BTN_ID_RANDOM     3
#define BTN_ID_NEXT_PAGE  3
#define BTN_ID_VOL_DOWN   12
#define BTN_ID_VOL_UP     15
#define BTN_ID_DUMP       14

#define BTN_STATE_UNKNWOW 255

#define SCOPE_ALL      0
#define SCOPE_LIBRARY  1
#define SCOPE_ALBUM    2
#define SCOPE_NONE     255

#define isLibrarySet() currentLibraryId != NO_KEY
#define isAlbumSet() currentAlbumId != NO_KEY
#define isTrackSet() currentTrackId != NO_KEY

#define currentLevel()  (currentLibraryId / NO_KEY)+(currentAlbumId / NO_KEY)+(currentTrackId / NO_KEY)

#define LEVEL_ROOT    3
#define LEVEL_LIBRARY 2
#define LEVEL_ALBUM   1
#define LEVEL_TRACK   0

#define LONG_KEY_PRESS_DELAY 2000  // ms

class MusicPlayer {
    public:
        //byte shuffle=false;
        byte playScope = SCOPE_NONE;
        bool autoPlayNext = false;

        char musicPath[MAX_PATH_LENGTH] = "/music";
        char statePath[MAX_PATH_LENGTH] = "/state/";

        // Only paths of albums from current library and tracks from current album
        // are stored at any given time. Not enough memory on arduino to store
        // whole tracks of whole albums of whole library
        char libraries[MAX_LIBRARY_COUNT][MAX_PATH_LENGTH];
        char albums[MAX_ALBUM_COUNT][MAX_PATH_LENGTH];
        char tracks[MAX_TRACK_COUNT][MAX_PATH_LENGTH];

        uint8_t libraryCount;
        uint8_t albumCount;
        uint8_t trackCount;

        uint8_t currentLibraryId = NO_KEY;
        uint8_t currentAlbumId = NO_KEY;
        uint8_t currentTrackId = NO_KEY;

        // No constructor, (almost) everything in begin()
        void begin();

        bool toggleAutoPlayNext() { DEBUG_PRINTF("Toggling autoplay from %d", autoPlayNext); autoPlayNext = !autoPlayNext; return autoPlayNext; }

        void setLibraryId(uint8_t id)  { DEBUG_PRINTF("Setting id to %d", id); currentLibraryId = id; saveParam("library",id); }
        void setAlbumId(uint8_t id)  { DEBUG_PRINTF("Setting id to %d", id); currentAlbumId = id; saveParam("album",id); }
        void setTrackId(uint8_t id)  { DEBUG_PRINTF("Setting id to %d", id); currentTrackId = id; saveParam("track",id); }

        void unsetLibraryId() { DEBUG_PRINT("Run"); currentLibraryId = NO_KEY; }
        void unsetAlbumId() { DEBUG_PRINT("Run"); currentAlbumId = NO_KEY; }
        void unsetTrackId() { DEBUG_PRINT("Run"); currentTrackId = NO_KEY; }

        void restoreIds() { 
            currentLibraryId = getParam("library"); currentAlbumId = getParam("album"); currentTrackId = getParam("track"); 
            DEBUG_PRINTF("Got Ids %d %d %d", currentLibraryId, currentAlbumId, currentTrackId);
        }

        // Getters
        //char* getTrackPath();

        // Player engine
        void loop();

        //void setVolume(byte volume);
        void selectObject(uint8_t id);

        bool setNextTrack();
        //void setNextAlbum();
        //void setNextLibrary();

        //void loadLevel();
        void navBack();
        void clearNav();

        void loadLibraries();
        void displayLibraries();

        void loadAlbums();
        void displayAlbums();
        //void playAlbums();

        void loadTracks();
        void displayTracks();
        //void playTracks();

        //void loadTrack();
        void playTrack();
        void playNextTrack();
        void pauseTrack();
        void unpauseTrack();
        void playPause();
        
        void dumpObject(bool dumpArray = false);

        void saveParam(const char *paramName, uint8_t paramValue);
        uint8_t getParam(const char *paramName);
        void saveAllParams();


        uint8_t getLibraryList(uint8_t maxEntries);
        uint8_t getAlbumList(uint8_t libraryId, uint8_t maxEntries);
        uint8_t getTrackList(uint8_t libraryId, uint8_t albumId, uint8_t maxEntries);

        uint8_t readSD(char* rootPath, char entries[][MAX_PATH_LENGTH], bool isDirectory, byte maxEntries);
};

#endif
