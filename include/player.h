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

#define DEFAULT_VOLUME 100
#define MAX_VOLUME     40
#define MIN_VOLUME     140
#define MID_VOLUME     110

#define SCOPE_ALL      0
#define SCOPE_LIBRARY  1
#define SCOPE_ALBUM    2
#define SCOPE_NONE     255

class MusicPlayer {
    public:
        //byte shuffle=false;
        byte volume=DEFAULT_VOLUME;
        byte playScope = SCOPE_NONE;
        bool autoPlayNext = false;

        char rootPath[MAX_PATH_LENGTH] = "/music";

        char libraries[MAX_LIBRARY_COUNT][MAX_PATH_LENGTH];
        char albums[MAX_ALBUM_COUNT][MAX_PATH_LENGTH];
        char tracks[MAX_TRACK_COUNT][MAX_PATH_LENGTH];

        byte libraryCount;
        byte albumCount;
        byte trackCount;

        byte currentLibraryId=NO_KEY;
        byte currentAlbumId=NO_KEY;
        byte currentTrackId=NO_KEY;

        // No constructor, (almost) everything in begin()
        void begin();

        // Getters
        char* getTrackPath();

        // Player engine
        void loop();

        void setVolume(byte volume);
        void selectObject(uint8_t id);

        bool setNextTrack();
        //void setNextAlbum();
        //void setNextLibrary();

        //void loadLevel();
        void navBack();

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


        byte readSD(char* rootPath, char entries[][MAX_PATH_LENGTH], bool isDirectory, byte maxEntries);
};

#endif
