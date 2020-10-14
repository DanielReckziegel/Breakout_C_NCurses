// SDL sound for NCurses
// (c) 2020 by Stefan Roettger

#include "sound.h"

#ifdef HAVE_SDL
#   include <SDL.h>
#   include <SDL_mixer.h>
#endif

static const int max_sounds = 8;

#ifdef HAVE_SDL
static Mix_Chunk *sound[max_sounds] = {NULL};
static int channel = -1;
#endif

// init SDL
bool sound_init()
{
#ifdef HAVE_SDL

   if (SDL_Init(SDL_INIT_AUDIO) != 0)
   {
      fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
      return(false);
   }

   int audio_rate = 44100;
   Uint16 audio_format = AUDIO_S16SYS;
   int audio_channels = 2;
   int audio_buffers = 4096;

   if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0)
   {
      fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
      return(false);
   }

#endif

   return(true);
}

// play WAV file with SDL
int sound_play(const char *filename, float volume)
{
#ifdef HAVE_SDL

   channel++;
   if (channel >= max_sounds) channel = 0;

   if (sound[channel] != NULL)
   {
      Mix_HaltChannel(channel);
      Mix_FreeChunk(sound[channel]);
   }

   sound[channel] = Mix_LoadWAV(filename);

   if (sound[channel] == NULL)
   {
      fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
      return(-1);
   }

   Mix_Volume(channel, 127 * volume + 0.5f);

   int result = Mix_PlayChannel(channel, sound[channel], 0);

   if (result < 0)
   {
      fprintf(stderr, "Unable to play WAV file: %s\n", Mix_GetError());
      return(-1);
   }

   return(channel);

#else

   return(-1);

#endif
}

// check if a sound channel is still playing
bool sound_playing(int channel)
{
#ifdef HAVE_SDL

   if (channel < 0 || channel >= max_sounds)
      return(false);

   return(Mix_Playing(channel) != 0);

#endif

   return(false);
}

// stop playing a sound channel
void sound_stop(int channel)
{
#ifdef HAVE_SDL

   if (channel < 0 || channel >= max_sounds)
      return;

   if (sound[channel] != NULL)
   {
      Mix_HaltChannel(channel);
      Mix_FreeChunk(sound[channel]);
      sound[channel] = NULL;
   }

#endif
}

// exit SDL
void sound_exit()
{
#ifdef HAVE_SDL

   for (int i=0; i<max_sounds; i++)
   {
      if (sound[i])
      {
         Mix_HaltChannel(i);
         Mix_FreeChunk(sound[i]);
      }
   }

   Mix_CloseAudio();
   SDL_Quit();

#endif
}
