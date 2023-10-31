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

#define currentLevel()  (currentLibraryId / NO_KEY)+(currentAlbumId / NO_KEY)+(currentTrackId / NO_KEY)

#define LEVEL_ROOT    3
#define LEVEL_LIBRARY 2
#define LEVEL_ALBUM   1
#define LEVEL_TRACK   0

#define LONG_KEY_PRESS_DELAY 2000  // ms

class MusicPlayer {
    public:
        bool autoPlay = false;

        void begin();

        void enableAutoPlay(bool enable);

        void setLibraryId(uint8_t id)  { currentLibraryId = id; saveParam("library", id); }
        void setAlbumId(uint8_t id)  { currentAlbumId = id; saveParam("album", id); }
        void setTrackId(uint8_t id)  { currentTrackId = id; saveParam("track", id); }

        void unsetLibraryId() { currentLibraryId = NO_KEY; saveParam("library", NO_KEY); }
        void unsetAlbumId() { currentAlbumId = NO_KEY; saveParam("album", NO_KEY); }
        void unsetTrackId() { currentTrackId = NO_KEY; saveParam("track", NO_KEY); }

        bool isLibrarySet() { return currentLibraryId != NO_KEY; }
        bool isAlbumSet()   { return currentAlbumId != NO_KEY; }
        bool isTrackSet()   { return currentTrackId != NO_KEY; }

        uint8_t getCurrentTrack() { return currentTrackId; }
        uint8_t getCurrentAlbum() { return currentAlbumId; }
        uint8_t getLibraryAlbum() { return currentLibraryId; }

        void loop();

        void selectObject(uint8_t id);

        bool setNextTrack();

        void navBack();
        void clearNav();

        void loadLibraries();
        void displayLibraries();

        void loadAlbums();
        void displayAlbums();

        void loadTracks();
        void displayTracks();

        void playTrack();
        void playNextTrack();
        void pauseTrack();
        void playPause();
        
        void dumpObject(bool dumpArray = false);

        void saveParam(const char *paramName, uint8_t paramValue);
        uint8_t getParam(const char *paramName);
        void restoreState();

        uint8_t getLibraryList(uint8_t maxEntries);
        uint8_t getAlbumList(uint8_t libraryId, uint8_t maxEntries);
        uint8_t getTrackList(uint8_t libraryId, uint8_t albumId, uint8_t maxEntries);

    private:
        uint8_t currentLibraryId = NO_KEY;
        uint8_t currentAlbumId = NO_KEY;
        uint8_t currentTrackId = NO_KEY;


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

        byte playScope = SCOPE_NONE;

        uint8_t readSD(char* rootPath, char entries[][MAX_PATH_LENGTH], bool isDirectory, byte maxEntries);

};

#endif
