/*
    Interacting Objects - Music Box
    http://www.interactingobjects.com

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
#define PREFER_SDFAT_LIBRARY
#include "debug.h"
#include <Adafruit_VS1053.h>
#include <Adafruit_NeoTrellis.h>

#include "box.h"
#include "player.h"
#include "diag.h"

extern Box box;
extern MusicPlayer musicPlayer;
extern Adafruit_NeoTrellis trellis;
extern Adafruit_VS1053_FilePlayer vs1053FilePlayer;
extern FatVolume onboardFS;

// Button used as selector for lib, album or track
// Set to 255 if used as control (volume, next...)
uint8_t mapButton2Track[] = {
    255, 255, 255, 255,
    0, 1, 2, 3,
    4, 5, 6, 7,
    255, 8, 9, 255};

uint32_t autoPlayColorMap[] = {COLOR_RED, COLOR_GREEN}; // To determine the color of play/pause

uint8_t button2track(uint8_t button)
{
    return mapButton2Track[button];
}

uint8_t track2button(uint8_t track)
{
    for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++)
        if (mapButton2Track[i] == track)
            return i;
    return 255;
}

uint32_t COLOR_CYCLE_ORANGE_RED[] = {COLOR_ORANGE, COLOR_RED};
uint32_t COLOR_CYCLE_BLACK_PURPLE[] = {COLOR_BLACK, COLOR_RED};
uint32_t COLOR_CYCLE_BLACK_RED[] = {COLOR_BLACK, COLOR_RED};

// Fonction de tri à bulles pour trier les entrées par ordre alphabétique
void bubbleSort(char arr[][MAX_PATH_LENGTH], int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (strcmp(arr[j], arr[j + 1]) > 0)
            {
                char temp[MAX_PATH_LENGTH];
                strncpy(temp, arr[j], MAX_PATH_LENGTH);
                strncpy(arr[j], arr[j + 1], MAX_PATH_LENGTH);
                strncpy(arr[j + 1], temp, MAX_PATH_LENGTH);
            }
        }
    }
}

//=========================================================================
//=== CALLBACKS
//=========================================================================

//-------------------------------------------------------------------------
TrellisCallback increaseVol(keyEvent event)
{
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]", event.bit.EDGE, event.bit.NUM, event.reg);
    if (event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING)
        return 0;
    box.max9744IncreaseVolume();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback decreaseVol(keyEvent event)
{
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]", event.bit.EDGE, event.bit.NUM, event.reg);
    if (event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING)
        return 0;
    box.max9744DecreaseVolume();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onBackPress(keyEvent event)
{
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]", event.bit.EDGE, event.bit.NUM, event.reg);
    if (event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING)
        return 0;
    musicPlayer.navBack();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onPlayPausePress(keyEvent event)
{
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]", event.bit.EDGE, event.bit.NUM, event.reg);
    if (event.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING)
    {
        if (!trellis.wasLongPressed(event.bit.NUM))
            musicPlayer.playPause();
    }
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onPlayPauseLongPress(keyEvent event)
{
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]", event.bit.EDGE, event.bit.NUM, event.reg);
    musicPlayer.enableAutoPlay(!musicPlayer.autoPlay);
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onNextPress(keyEvent event)
{
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]", event.bit.EDGE, event.bit.NUM, event.reg);
    if (event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING)
        return 0;
    musicPlayer.playNextTrack();
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onPreviousPress(keyEvent event)
{
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]", event.bit.EDGE, event.bit.NUM, event.reg);
    if (event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING)
        return 0;
    DEBUG_PRINT("playPreviousTrack not implemented (yet)");
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onTrackKeyPressed(keyEvent event)
{
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]", event.bit.EDGE, event.bit.NUM, event.reg);
    if (event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING)
        return 0;
    musicPlayer.selectObject(button2track(event.bit.NUM));
    return 0;
}

//-------------------------------------------------------------------------
TrellisCallback onDumpKeyPressed(keyEvent event)
{
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]", event.bit.EDGE, event.bit.NUM, event.reg);
    if (event.bit.EDGE != SEESAW_KEYPAD_EDGE_RISING)
        return 0;
    musicPlayer.dumpObject(true);
    return 0;
}

//=========================================================================
//=== Methods
//=========================================================================

/**************************************************************************/
/*!
    @brief Player initialization
*/
/**************************************************************************/
void MusicPlayer::begin()
{
    DEBUG_PRINT("StartFunction");

    for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++)
    {
        trellis.registerCallback(i, onTrackKeyPressed);
        trellis.pixels.setPixelColor(i, 0xFFFFFF);
    }

    trellis.registerCallback(BTN_ID_BACK, onBackPress);
    trellis.registerCallback(BTN_ID_PLAY_PAUSE, onPlayPausePress);
    trellis.registerCustomCallback(BTN_ID_PLAY_PAUSE, onPlayPauseLongPress);
    trellis.registerCallback(BTN_ID_NEXT, onNextPress);
    trellis.registerCallback(BTN_ID_BACK, onBackPress);
    trellis.registerCallback(BTN_ID_VOL_UP, increaseVol);
    trellis.registerCallback(BTN_ID_VOL_DOWN, decreaseVol);

    trellis.pixels.setPixelColor(BTN_ID_BACK, COLOR_ORANGE);
    trellis.pixels.setPixelColor(BTN_ID_NEXT, COLOR_BLUE);
    // trellis.pixels.setPixelColor(BTN_ID_NEXT_PAGE,COLOR_PURPLE);
    trellis.pixels.setPixelColor(BTN_ID_VOL_UP, COLOR_BLUE);
    trellis.pixels.setPixelColor(BTN_ID_VOL_DOWN, COLOR_BLUE);

#ifdef DEBUG
    trellis.pixels.setPixelColor(BTN_ID_DUMP, COLOR_RED);
    trellis.registerCallback(BTN_ID_DUMP, onDumpKeyPressed);
    mapButton2Track[BTN_ID_DUMP] = 255;
#endif

    loadLibraries();
    restoreState();

    trellis.pixels.setBlink(BTN_ID_PLAY_PAUSE, autoPlayColorMap[autoPlay], BLINK_PAUSE_PERIOD, !autoPlay);

    if (isLibrarySet())
    {
        DEBUG_PRINTF("Loading album in saved library no %d", currentLibraryId);
        loadAlbums();
        if (isAlbumSet())
        {
            DEBUG_PRINTF("Loading tracks in saved album no %d", currentAlbumId);
            loadTracks();
            displayTracks();
            if (isTrackSet())
                trellis.pixels.setBlink(track2button(getCurrentTrack()), COLOR_PURPLE, BLINK_PAUSE_PERIOD, true);
        }
        else
        {
            displayAlbums();
        }
    }
    else
    {
        displayLibraries();
    }

    DEBUG_PRINT("ExitFunction");
}

/**************************************************************************/
/*!
    @brief Player main loop
*/
/**************************************************************************/
void MusicPlayer::loop()
{
    // bool startup = true;

    DEBUG_PRINT("StartFunction");

    while (true)
    {
        trellis.read();
        trellis.pixels.showCycle();

        if (vs1053FilePlayer.playingMusic)
        {
            vs1053FilePlayer.feedBuffer();
            // Serial.print(".");
        }
        else
        { // Not playing, either never started, paused, or finished
            // Serial.print("x");
            if (getStatus() == PLAY_STATUS_PLAYING)
            { // If previously playing, end of track reached
                playStatus = PLAY_STATUS_STOPPED;
                trellis.pixels.setBlink(track2button(getCurrentTrack()), COLOR_PURPLE, BLINK_PAUSE_PERIOD, true);
                if (autoPlay)
                {
                    if (setNextTrack())
                        playTrack();
                    else
                        enableAutoPlay(false);
                }
            }
        }
    }

    DEBUG_PRINT("End");
}

/**************************************************************************/
/*!
    @brief Enable or disable autoplay
    @param enable enabled if true, disabled if false
*/
/**************************************************************************/
void MusicPlayer::enableAutoPlay(bool enable)
{
    DEBUG_PRINTF("Start: color=%ld enable=%d", autoPlayColorMap[enable], !vs1053FilePlayer.playingMusic);
    autoPlay = enable;
    if (isTrackPaused())
        trellis.pixels.setBlink(BTN_ID_PLAY_PAUSE, autoPlayColorMap[enable], BLINK_PAUSE_PERIOD, true);
    else
        trellis.pixels.setPixelColor(BTN_ID_PLAY_PAUSE, autoPlayColorMap[enable]);
    saveParam("autoplay", (uint8_t)autoPlay);
}

/**************************************************************************/
/*!
    @brief Load libraries from SD
*/
/**************************************************************************/
void MusicPlayer::loadLibraries()
{

    DEBUG_PRINT("StartFunction");
    libraryCount = getLibraryList(MAX_LIBRARY_COUNT);
    playScope = SCOPE_ALL;
    DEBUG_PRINT("ExitFunction");
}

/**************************************************************************/
/*!
    @brief Set pixels to display all libraries
*/
/**************************************************************************/
void MusicPlayer::displayLibraries()
{

    DEBUG_PRINT("StartFunction");

    clearNav();
    for (byte libraryRef = 0; libraryRef < libraryCount; libraryRef++)
    {
        DEBUG_PRINTF("Enabling library %d", libraryRef);
        trellis.pixels.setPixelColor(track2button(libraryRef), COLOR_AQUA);
    }
    // trellis.pixels.show();
    DEBUG_PRINT("ExitFunction");
}

/**************************************************************************/
/*!
    @brief Load albums of selected library from SD
*/
/**************************************************************************/
void MusicPlayer::loadAlbums()
{
    DEBUG_PRINT("StartFunction");
    albumCount = getAlbumList(currentLibraryId, MAX_ALBUM_COUNT);
    playScope = SCOPE_LIBRARY;
    DEBUG_PRINT("ExitFunction");
}

/**************************************************************************/
/*!
    @brief Set pixels to display albums of current library
*/
/**************************************************************************/
void MusicPlayer::displayAlbums()
{

    DEBUG_PRINT("StartFunction");

    clearNav();
    for (byte albumRef = 0; albumRef < albumCount; albumRef++)
    {
        DEBUG_PRINTF("Enabling album %d", albumRef);
        trellis.pixels.setPixelColor(track2button(albumRef), COLOR_NAVY);
    }
    // trellis.pixels.show();
    DEBUG_PRINT("ExitFunction");
}

/**************************************************************************/
/*!
    @brief Load track of selected album from SD
*/
/**************************************************************************/
void MusicPlayer::loadTracks()
{

    DEBUG_PRINT("StartFunction");
    trackCount = getTrackList(currentLibraryId, currentAlbumId, MAX_TRACK_COUNT);
    playScope = SCOPE_ALBUM;
    DEBUG_PRINT("ExitFunction");
}

/**************************************************************************/
/*!
    @brief Set pixels to display tracks of current album
*/
/**************************************************************************/
void MusicPlayer::displayTracks()
{

    DEBUG_PRINT("StartFunction");

    clearNav();
    for (byte trackRef = 0; trackRef < trackCount; trackRef++)
    {
        DEBUG_PRINTF("Enabling track %d", trackRef);
        if (trackRef == currentTrackId)
        {
            trellis.pixels.setBlink(track2button(trackRef), COLOR_GREEN, BLINK_PAUSE_PERIOD);
        }
        else
        {
            trellis.pixels.setPixelColor(track2button(trackRef), COLOR_WHITE);
        }
    }
    DEBUG_PRINT("ExitFunction");
}

/**************************************************************************/
/*!
    @brief Play a track and set corresponding pixel accordingly
*/
/**************************************************************************/
void MusicPlayer::playTrack()
{
    DEBUG_PRINT("StartFunction");

    switch (getStatus())
    {
    case PLAY_STATUS_STOPPED:
        DEBUG_PRINTF("Was stopped, starting | library: %d album:%d track:%d %s", currentLibraryId, currentAlbumId, currentTrackId, tracks[currentTrackId]);
        DEBUG_PRINTF("Playing state: %s", vs1053FilePlayer.playingMusic ? "Playing" : "Not playing");
        bool isFilePlaying;
        isFilePlaying = vs1053FilePlayer.startPlayingFile(tracks[currentTrackId]);
        DEBUG_PRINTF("Track started: %s", isFilePlaying ? "Yes" : "No");
        DEBUG_PRINTF("Playing state: %s", vs1053FilePlayer.playingMusic ? "Playing" : "Not playing");
        playStatus = PLAY_STATUS_PLAYING;
        trellis.pixels.setPixelColor(track2button(currentTrackId), COLOR_GREEN);
        trellis.pixels.setPixelColor(BTN_ID_PLAY_PAUSE, autoPlayColorMap[autoPlay]);
        break;

    case PLAY_STATUS_PAUSED:
        DEBUG_PRINTF("Was paused, re-starting | library:%d alblum:%d track:%d %s", currentLibraryId, currentAlbumId, currentTrackId, tracks[currentTrackId]);
        vs1053FilePlayer.pausePlaying(false);
        playStatus = PLAY_STATUS_PLAYING;
        trellis.pixels.setPixelColor(track2button(currentTrackId), COLOR_GREEN);
        trellis.pixels.setPixelColor(BTN_ID_PLAY_PAUSE, autoPlayColorMap[autoPlay]);
        break;

    case PLAY_STATUS_PLAYING:
        DEBUG_PRINTF("Already playing | library:%d alblum:%d track:%d %s", currentLibraryId, currentAlbumId, currentTrackId, tracks[currentTrackId]);
        break;

    default:
        DEBUG_PRINTF("Unknown play statut value | library:%d alblum:%d track:%d %s", currentLibraryId, currentAlbumId, currentTrackId, tracks[currentTrackId]);
        break;
    }

    DEBUG_PRINT("ExitFunction");
}

/**************************************************************************/
/*!
    @brief Pause music and set corresponding pixel accordingly
*/
/**************************************************************************/
void MusicPlayer::pauseTrack()
{
    DEBUG_PRINT("StartFunction");
    vs1053FilePlayer.pausePlaying(true);
    playStatus = PLAY_STATUS_PAUSED;
    trellis.pixels.setMode(BTN_ID_PLAY_PAUSE, CYCLE_MODE_BLINK);
    trellis.pixels.setBlink(track2button(currentTrackId), COLOR_GREEN, BLINK_PAUSE_PERIOD, true);
    DEBUG_PRINT("ExitFunction");
}

void MusicPlayer::stopTrack()
{
    DEBUG_PRINT("StartFunction");
    if (getStatus() != PLAY_STATUS_STOPPED)
    {
        DEBUG_PRINT("Stop track (if playing or paused)");
        trellis.pixels.setPixelColor(track2button(getCurrentTrack()), COLOR_PURPLE);
        vs1053FilePlayer.stopPlaying();
    }
}
/**************************************************************************/
/*!
    @brief Play or pause music depending on current state
*/
/**************************************************************************/
void MusicPlayer::playPause()
{
    DEBUG_PRINT("StartFunction");

    // if(playScope == SCOPE_NONE) {
    //     if( isAlbumSet() )                 playScope = SCOPE_ALBUM;
    //     else if( isLibrarySet() )          playScope = SCOPE_LIBRARY;
    //     else                               playScope = SCOPE_ALL;
    // }

    // A track is already selected, just play (or pause) it
    if (isTrackSet())
    {
        if (getStatus() == PLAY_STATUS_PLAYING)
        {
            DEBUG_PRINT("Asked to pause while playing, pausing");
            pauseTrack();
        }
        else
        {
            DEBUG_PRINT("Asked to play while paused or stopped, playing");
            playTrack();
        }
    }
    // No track is selected,  Setting autoplay to true, select and play first track in scope
    else
    {
        if (setNextTrack())
        {
            DEBUG_PRINTF("Asked to play while no track active, enabling autoplay in scope");
            enableAutoPlay(true);
            playTrack();
        }
        else
        {
            DEBUG_PRINTF("No track set, nothing left in scope");
            enableAutoPlay(false);
        }
    }
    DEBUG_PRINTF("ExitFunction,scope:%d", playScope);
}

/**************************************************************************/
/*!
    @brief Select object by its ID depending where we are in nav tree
    @param id ID of the object (track, album or library)
*/
/**************************************************************************/
void MusicPlayer::selectObject(uint8_t id)
{
    DEBUG_PRINT("Start");

    uint8_t _currentLevel = currentLevel();

    if (_currentLevel <= LEVEL_ALBUM)
    {
        DEBUG_PRINT("At album level");
        if (getCurrentTrack() == id)
        {
            DEBUG_PRINT("Key pressed match current playing track : play/pause");
            playPause();
        }
        else
        {
            if (id < trackCount)
            {
                stopTrack();
                trellis.pixels.setPixelColor(track2button(getCurrentTrack()), COLOR_PURPLE);
                setTrackId(id);
                playTrack();
                DEBUG_PRINT("Old track stopped, new track playing");
            }
        }
    }

    if (_currentLevel == LEVEL_LIBRARY)
    {
        DEBUG_PRINT("At library level");
        if (id < albumCount)
        {
            setAlbumId(id);
            loadTracks();
            displayTracks();
            DEBUG_PRINT("ExitFunction, album id set");
            return;
        }
        else
        {
            DEBUG_PRINT("ExitFunction, Album Id out of bound");
        }
    }

    if (_currentLevel == LEVEL_ROOT)
    {
        DEBUG_PRINT("At root level");
        if (id < libraryCount)
        {
            setLibraryId(id);
            loadAlbums();
            displayAlbums();
            DEBUG_PRINT("ExitFunction, library set");
            return;
        }
        else
        {
            DEBUG_PRINT("ExitFunction, Library Id out of bound");
        }
    }
}

//-------------------------------------------------------------------------
// Back in nav tree
//-------------------------------------------------------------------------
void MusicPlayer::navBack()
{
    uint8_t _currentLevel = currentLevel();
    enableAutoPlay(false);

    if (_currentLevel <= LEVEL_ALBUM)
    {
        pauseTrack();
        vs1053FilePlayer.stopPlaying();
        unsetTrackId();
        unsetAlbumId();
        displayAlbums();
        playScope = SCOPE_LIBRARY;
        DEBUG_PRINT("ExitFunction, unset track id and album id");
    }

    if (_currentLevel == LEVEL_LIBRARY)
    {
        unsetAlbumId();
        unsetLibraryId();
        displayLibraries();
        playScope = SCOPE_ALL;
        DEBUG_PRINT("ExitFunction,unset album id and library id");
    }
}

//-------------------------------------------------------------------------
// Playing next track
//-------------------------------------------------------------------------
void MusicPlayer::playNextTrack()
{
    DEBUG_PRINT("StartFunction");
    vs1053FilePlayer.stopPlaying();
    if (setNextTrack())
        playTrack();
}

//-------------------------------------------------------------------------
// Setting next track
//-------------------------------------------------------------------------
bool MusicPlayer::setNextTrack(bool anyScope)
{

    DEBUG_PRINTF("StartFunction,scope %d", playScope);

    if (!isTrackSet())
    {
        if (!isAlbumSet())
        {
            if (!isLibrarySet())
            { // Nothing loaded yet
                DEBUG_PRINT("1st track in 1st album in first library");
                setTrackId(0);
                setAlbumId(0);
                setLibraryId(0);
                loadAlbums();
                loadTracks();
                displayTracks();
            }
            else
            { // Library loaded
                DEBUG_PRINT("1st track in 1st album in current library");
                setTrackId(0);
                setAlbumId(0);
                loadTracks();
                displayTracks();
            }
        }
        else
        { // Album loaded (but not track)
            DEBUG_PRINT("1st track in current album");
            setTrackId(0);
        }
    }
    else
    { // Track loaded
        if (currentTrackId + 1 < trackCount)
        { // Track loaded & Next track available in the current album
            DEBUG_PRINT("Next track in current album");
            trellis.pixels.setPixelColor(track2button(getCurrentTrack()), COLOR_PURPLE);
            setTrackId(currentTrackId + 1);
        }
        else
        { // No more track in album

            if (playScope == SCOPE_ALBUM)
            { // No more track in album & scope is album
                DEBUG_PRINT("ExitFunction,No more tracks, album scope");
                return false;
            }

            if (currentAlbumId + 1 < albumCount)
            { // No more track in album, next album available and scope > album
                DEBUG_PRINT("1st track in next album in current library")
                setTrackId(0);
                setAlbumId(currentAlbumId + 1);
                loadTracks();
                displayTracks();
            }
            else
            { // No more track in album, no more album available

                if (playScope == SCOPE_LIBRARY)
                { // No more album in library & scope is library
                    DEBUG_PRINT("ExitFunction,No more albums, library scope");
                    return false;
                }

                if (currentLibraryId + 1 < libraryCount)
                { // No more album in library & scope is all
                    DEBUG_PRINT("1st track in first album in next library")
                    setTrackId(0);
                    setAlbumId(0);
                    setLibraryId(currentLibraryId + 1);
                    loadAlbums();
                    loadTracks();
                    displayTracks();
                }
            }
        }
    }

    DEBUG_PRINT("ExitFunction");
    return true;
}

//-------------------------------------------------------------------------
void MusicPlayer::clearNav()
{
    for (uint8_t i = 0; i < NEO_TRELLIS_NUM_KEYS; i++)
    {
        if (mapButton2Track[i] != 255)
        {
            trellis.pixels.setPixelColor(i, COLOR_BLACK);
        }
    }
}

/**************************************************************************/
/*!
    @brief Stores SD Card directory entries in an array
    @return Entry count
*/
/**************************************************************************/
byte MusicPlayer::readSD(char *path, char entries[][MAX_PATH_LENGTH], bool isDirectory, byte maxEntries)
{
    DEBUG_PRINTF("START path:%s,isDirectory:%d,maxEntries:%d", path, isDirectory, maxEntries);
    File dir = SD.open(path);
    byte count = 0;

    // Resetting all entries
    for (uint8_t i = 0; i < maxEntries; i++)
    {
        entries[i][0] = 0;
    }

    char tempEntries[maxEntries][MAX_PATH_LENGTH];

    while (true)
    {

        if (count >= maxEntries)
        {
            DEBUG_PRINT("Too many entries, ignoring remaining");
            break;
        }

        File dirEntry;
        dirEntry.openNext(&dir, O_RDONLY);

        if (!dirEntry)
        {
            DEBUG_PRINT("No more entries");
            break;
        }

        char fileNameBuffer[MAX_PATH_LENGTH];
        dirEntry.getName(fileNameBuffer, MAX_PATH_LENGTH);

        // Filter hidden files starting with a dot
        if (fileNameBuffer[0] == '.')
        {
            DEBUG_PRINTF("Ignoring hidden file: %s", fileNameBuffer);
            dirEntry.close();
            continue;
        }

        if (dirEntry.isDir() != isDirectory)
        {
            DEBUG_PRINTF("Entry %s is not of expected type %d, ignoring", fileNameBuffer, isDirectory);
            dirEntry.close();
            continue;
        }

        strncpy(tempEntries[count], fileNameBuffer, MAX_PATH_LENGTH);
        count++;
        dirEntry.close();
    }

    // Trier les entrées par ordre alphabétique
    bubbleSort(tempEntries, count);

    // Copier les entrées triées dans le tableau final
    for (byte i = 0; i < count; i++)
    {
        strcpy(entries[i], path);
        strcat(entries[i], "/");
        strcat(entries[i], tempEntries[i]);
        DEBUG_PRINTF("Adding entry %d %s", count, entries[count]);
    }

    dir.close();
    DEBUG_PRINTF("END count:%d,path:%s", count, path)
    return count;
}

//-------------------------------------------------------------------------
// Get library list from SD (list of directories in music path)
//-------------------------------------------------------------------------
uint8_t MusicPlayer::getLibraryList(uint8_t maxEntries)
{
    return readSD(musicPath, libraries, true, MAX_LIBRARY_COUNT);
}

//-------------------------------------------------------------------------
// Get album list from SD in specified library
//-------------------------------------------------------------------------
uint8_t MusicPlayer::getAlbumList(uint8_t libraryId, uint8_t maxEntries)
{
    return readSD(libraries[libraryId], albums, true, MAX_ALBUM_COUNT);
}

//-------------------------------------------------------------------------
// Get Track list from SD in specified album
//-------------------------------------------------------------------------
uint8_t MusicPlayer::getTrackList(uint8_t libraryId, uint8_t albumId, uint8_t maxEntries)
{
    return readSD(albums[albumId], tracks, false, MAX_TRACK_COUNT);
}

//-------------------------------------------------------------------------
// Display debug info
//-------------------------------------------------------------------------
void MusicPlayer::dumpObject(bool dumpArrays)
{

    DEBUG_PRINT("=======================================");
    if (dumpArrays)
    {
        DEBUG_PRINT("--------------------------------------");
        DEBUG_PRINT_ARRAY(libraries, "libraries", MAX_ALBUM_COUNT);
        DEBUG_PRINT_ARRAY(albums, "albums", MAX_ALBUM_COUNT);
        DEBUG_PRINT_ARRAY(tracks, "tracks", MAX_TRACK_COUNT);
    }

    DEBUG_PRINT("=======================================");
    DEBUG_PRINTF("vs1053 volume         %d", box.vs1053_volume);
    DEBUG_PRINTF("max9744 volume        %d", box.max9744_volume);
    DEBUG_PRINTF("musicPath             %s", musicPath);
    DEBUG_PRINTF("libraryCount          %d", libraryCount);
    DEBUG_PRINTF("albumCount            %d", albumCount);
    DEBUG_PRINTF("trackCount            %d", trackCount);
    DEBUG_PRINTF("currentLibraryId      %d", currentLibraryId);
    DEBUG_PRINTF("currentAlbumId        %d", currentAlbumId);
    DEBUG_PRINTF("currentTrackId        %d", currentTrackId);
    DEBUG_PRINTF("autoPlay              %d", autoPlay);
    DEBUG_PRINTF("currentLevel()        %d", currentLevel());
    DEBUG_PRINTF("getStatus()           %d", getStatus());
    DEBUG_PRINT("=======================================");
    // DEBUG_PRINT("State files");
    // DEBUG_PRINTF("library               %d", getParam("LIBRARY"));
    // DEBUG_PRINTF("album                 %d", getParam("ALBUM"));
    // DEBUG_PRINTF("track                 %d", getParam("TRACK"));
    // DEBUG_PRINTF("autoplay              %d", getParam("autoplay"));
    // DEBUG_PRINT("=======================================");
    DEBUG_PRINTF("VS1053 status         %d", vs1053FilePlayer.playingMusic);
    DEBUG_PRINTF("VS1053 paused         %d", vs1053FilePlayer.paused());
    DEBUG_PRINTF("VS1053 stopped        %d", vs1053FilePlayer.stopped());
    DEBUG_PRINT("=======================================");
}

//-------------------------------------------------------------------------
// Saving a single param
//-------------------------------------------------------------------------
void MusicPlayer::saveParam(const char *paramName, uint8_t paramValue)
{
    DEBUG_PRINT("START");
    char fullPath[27];

    if (not onboardFS.exists(statePath))
    {
        DEBUG_PRINT("State dir does not exists, creating");
        onboardFS.mkdir(statePath);
    }

    strcpy(fullPath, statePath);
    strcat(fullPath, paramName);

    if (onboardFS.exists(fullPath))
    {
        onboardFS.remove(fullPath);
    }

    DEBUG_PRINTF("Saving %d in %s", paramValue, fullPath);
    File f = onboardFS.open(fullPath, O_WRITE | O_CREAT);
    if (f)
    {
        DEBUG_PRINT("Writing...");
        char str[2] = {paramValue, 0};
        f.write(str);
        DEBUG_PRINT("Closing...");
        f.close();
    }
    else
    {
        DEBUG_PRINTF("Cannot openfile %s for writing", fullPath)
    }
    DEBUG_PRINT("END");
}

/**************************************************************************/
/*!
    @brief Get a previously saved param
*/
/**************************************************************************/
uint8_t MusicPlayer::getParam(const char *paramName)
{
    DEBUG_PRINT("START");

    char fullPath[256];
    strcpy(fullPath, statePath);
    strcat(fullPath, paramName);
    char str[2];

    if (not onboardFS.exists(fullPath))
    {
        DEBUG_PRINTF("END No param file %s found", fullPath);
        return NO_KEY;
    }
    else
    {
        File f = onboardFS.open(fullPath, FILE_READ);
        if (f)
        {
            int bytes = f.read(str, 1);
            if (bytes == 1)
            {
                DEBUG_PRINTF("END Got %d as %s value in %s", str[0], paramName, fullPath);
                f.close();
                return str[0];
            }
            if (bytes == 0)
            {
                DEBUG_PRINTF("END Got 0 as %s value in %s", paramName, fullPath);
                f.close();
                return 0;
            }
            if (bytes == -1)
            {
                DEBUG_PRINTF("Error while reading %s", fullPath);
                f.close();
                return NO_KEY;
            }
            DEBUG_PRINTF("Error while reading, got %s in %s", str, fullPath);
            f.close();
            return 255;
        }
        else
        {
            DEBUG_PRINTF("END Cannot open file %s", fullPath);
            return 255;
        }
    }
}

void MusicPlayer::restoreState()
{
    currentLibraryId = getParam("library");
    currentAlbumId = getParam("album");
    currentTrackId = getParam("track");
    autoPlay = getParam("autoplay");
}
