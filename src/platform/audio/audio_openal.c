#include "audio.h"
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__

    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>

#else

    #include <AL/al.h>
    #include <AL/alc.h>
    #include <AL/alext.h>

#endif

/*
struct Athena_SoundConfig{
    float volume, pan;
    unsigned char loop;
};

enum Athena_SoundFormat { Athena_SoundU16, Athena_SoundU32, Athena_SoundFloat };
*/

#define DEFAULT_STARTING_BUFFERS 2

struct Athena_SoundContext{
    ALCdevice *device;
    ALCcontext *context;
};



struct Athena_Sound{
    struct Athena_SoundConfig config;
    struct Athena_SoundContext *context;

    unsigned format, num_channels, samples_per_second;
    float length;    
    
    ALuint source;
};

static void athena_make_context_current(const struct Athena_SoundContext *ctx){
    if(ctx)
        alcMakeContextCurrent(ctx->context);
}

struct Athena_SoundContext *Athena_CreateSoundContext(){
    ALCdevice *device;
    ALCcontext *context;
    
    if(!(device = alcOpenDevice(NULL))){
        return NULL;
    }
    
    if(!(context = alcCreateContext(device, NULL)))
        return NULL;
    
    alcMakeContextCurrent(context);
    
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_ORIENTATION, 0.0f, 0.0f, 0.0f);

    {
        struct Athena_SoundContext * const r_ctx = calloc(sizeof(struct Athena_SoundContext), 1);
        r_ctx->context = context;
        r_ctx->device = device;
        return r_ctx;
    }
}

void Athena_DestroySoundContext(struct Athena_SoundContext *ctx){
    if(!ctx)
        return;

    alcCloseDevice(ctx->device);

    free(ctx);
}

enum Athena_SoundFormat Athena_PreferredFormat(struct Athena_SoundContext *ctx){
    return Athena_SoundU16;
}

enum Athena_SoundFormat Athena_PreferredSoundFormat(struct Athena_Sound *snd){
    return Athena_PreferredFormat(NULL);
}

unsigned Athena_SupportsFormat(struct Athena_SoundContext *ctx, enum Athena_SoundFormat format){
    return Athena_PreferredFormat(ctx)==format;
}

struct Athena_Sound *Athena_CreateSound(struct Athena_SoundContext *ctx){
    struct Athena_Sound * const sound = calloc(sizeof(struct Athena_Sound), 1);
    sound->context = ctx;

    athena_make_context_current(ctx);
    
    alGenSources(1, &sound->source);
    
    return sound;

}

static void athena_free_buffers(struct Athena_Sound *sound){
    ALuint buffers[16];
    int n;

    alGetSourcei(sound->source, AL_BUFFERS_PROCESSED, &n);
    if(n==0)
        return;

    if(n>16)
        n = 16;

    alSourceUnqueueBuffers(sound->source, n, buffers);
    alDeleteBuffers(n, buffers);

    athena_free_buffers(sound);
}

void Athena_DestroySound(struct Athena_Sound *sound){
    
    athena_make_context_current(sound->context);

    athena_free_buffers(sound);
    alDeleteSources(1, &sound->source);

    free(sound);
}

void Athena_SoundInit(struct Athena_Sound *sound, unsigned num_channels, unsigned samples_per_second, enum Athena_SoundFormat format){

    if(num_channels==2)
        sound->format = AL_FORMAT_STEREO16;
    else
        sound->format = AL_FORMAT_MONO16;

    sound->samples_per_second = samples_per_second;

}

void Athena_SoundGetConfig(const struct Athena_Sound *sound, struct Athena_SoundConfig *to){
    to[0] = sound->config;
}

void Athena_SoundSetConfig(struct Athena_Sound *sound, const struct Athena_SoundConfig *to){

    if(fabs(sound->config.volume - to->volume) > 0.01f){
        alSourcef(sound->source, AL_GAIN, to->volume);
        sound->config.volume = to->volume;
    }

    if(fabs(sound->config.pan - to->pan) > 0.01f){
        sound->config.pan = to->pan;
    }
    
    if(sound->config.loop != to->loop){
        alSourcei(sound->source, AL_LOOPING, to->loop?AL_TRUE:AL_FALSE);
        sound->config.loop = to->loop;
    }

}

float Athena_SoundGetLength(const struct Athena_Sound *sound){
    return sound->length;
}

unsigned Athena_SoundGetChannels(const struct Athena_Sound *sound){
    return sound->num_channels;
}

unsigned Athena_SoundGetSamplesPerSecond(const struct Athena_Sound *sound){
    return sound->samples_per_second;
}

enum Athena_SoundFormat Athena_SoundGetFormat(const struct Athena_Sound *sound){
    return Athena_PreferredFormat(sound->context);
}

const void *Athena_SoundGetContext(const struct Athena_Sound *sound){
    return sound->context->context;
}

static ALuint athena_get_buffer(struct Athena_Sound *sound){
    ALuint buffer = 0;

    ALint i;

    alGetSourcei(sound->source, AL_BUFFERS_PROCESSED, &i);
    if(i){
        alSourceUnqueueBuffers(sound->source, 1, &buffer);
    }
    else{
        alGenBuffers(1, &buffer);
    }
    
    return buffer;
}

unsigned Athena_SoundPost(struct Athena_Sound *sound, const void *data, unsigned length){
    athena_make_context_current(sound->context); 
   
    {
        const ALuint buffer = athena_get_buffer(sound);

        alBufferData(buffer, sound->format, data, length, sound->samples_per_second);
        alSourceQueueBuffers(sound->source, 1, &buffer);
    }

    return 0;
}

void Athena_SoundPlay(struct Athena_Sound *sound){
    alSourcePlay(sound->source);
}

void Athena_SoundPause(struct Athena_Sound *sound){
    alSourcePause(sound->source);
}

void Athena_SoundStop(struct Athena_Sound *sound){
    alSourceStop(sound->source);
}

void Athena_SoundRewind(struct Athena_Sound *sound){
    alSourceRewind(sound->source);
}

