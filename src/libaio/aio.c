#ifdef ALSA
#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alloca.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>
#endif

#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

/*---------------------------------------------------------------------------*/

#define RATE 44100             /* Sample rate.  44100Hz is CD quality.       */
#define CHAN 2                 /* Number of channels.  2 is stereo.          */
#define MAXV 128               /* Maximum number of simultaneous voices.     */
#define MAXC 8                 /* Maximum number of simultaneous cycles.     */

static unsigned int period_time = 20000;               /* microseconds       */
static unsigned int buffer_time = 40000;               /* microseconds       */

#define CK(exp) { int err = (exp); if (err) return err;	}
	
/*---------------------------------------------------------------------------*/

#ifdef ALSA
static const char *device = "plughw:0,0";

static snd_pcm_t        *pcm;
static snd_pcm_access_t  pcm_access = SND_PCM_ACCESS_RW_INTERLEAVED;
static snd_pcm_format_t  pcm_format = SND_PCM_FORMAT_S16;

static snd_pcm_uframes_t period_size;
static snd_pcm_uframes_t buffer_size;
#endif

static short *buffer = NULL;

struct voice
{
    short *buf;
    int    len;
    int    pos;
    float  vol;
};

struct cycle
{
    short *buf;
    int    len;
    int    pos;
    float  vol;
    float  frq;
};

static struct voice voice[MAXV];
static struct cycle cycle[MAXC];

/*---------------------------------------------------------------------------*/

#ifdef ALSA

static int set_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *p)
{
    int dir;

    unsigned int rate = RATE;

    CK(snd_pcm_hw_params_any(pcm, p));

    /* Set the stream parameters. */

    CK(snd_pcm_hw_params_set_access(pcm, p, pcm_access));
    CK(snd_pcm_hw_params_set_format(pcm, p, pcm_format));
    CK(snd_pcm_hw_params_set_channels(pcm, p, CHAN));
    CK(snd_pcm_hw_params_set_rate_near(pcm, p, &rate, 0));

    /* Set the buffer time. */

    CK(snd_pcm_hw_params_set_buffer_time_near(pcm, p, &buffer_time, &dir));
    CK(snd_pcm_hw_params_get_buffer_size(p, &buffer_size));

    /* Set the period time. */

    CK(snd_pcm_hw_params_set_period_time_near(pcm, p, &period_time, &dir));
    CK(snd_pcm_hw_params_get_period_size(p, &period_size, &dir));

    /* Configure the device. */

    CK(snd_pcm_hw_params(pcm, p));

    return 0;
}

static int set_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t *p)
{
    CK(snd_pcm_sw_params_current(pcm, p));

    /* Set buffering parameters. */

    CK(snd_pcm_sw_params_set_start_threshold(pcm, p, buffer_size));
    CK(snd_pcm_sw_params_set_avail_min(pcm, p, period_size));
    CK(snd_pcm_sw_params_set_xfer_align(pcm, p, 1));

    /* Configure the device. */

    CK(snd_pcm_sw_params(pcm, p));

    return 0;
}

#endif /* ALSA */

/*---------------------------------------------------------------------------*/

#ifdef ALSA

static void aio_proc_voice(void)
{
    int i, j, k, n = period_size;

    for (i = 0; i < MAXV; i++)
    {
        struct voice *v = voice + i;

        if (v->len == 0) continue;

        for (j = 0; j < n && v->pos < v->len; j++, v->pos++)
        {
            short s = (short) (v->buf[v->pos] * v->vol);

            for (k = 0; k < CHAN; k++)
                buffer[j * CHAN + k] += s;
        }
    }
}

static void aio_proc_cycle(void)
{
    int i, j, k, n = period_size;

    for (i = 0; i < MAXC; i++)
    {
        struct cycle *c = cycle + i;

        int p0 = c->pos;

        if (c->len == 0) continue;

        for (j = 0; j < n; j++)
        {
            short s = (short) (c->buf[c->pos] * c->vol);

            for (k = 0; k < CHAN; k++)
                buffer[j * CHAN + k] += s;

            c->pos = (p0 + (int) (j * c->frq)) % c->len;
        }
    }
}

/* The audio system is ready for another block. Mix one and queue it.  This  */
/* function is invoked asynchonously by libasound in response to SIGIO.      */

static void aio_proc(snd_async_handler_t *handler)
{
    memset(buffer, 0, period_size * CHAN * sizeof (short));

    aio_proc_voice();
    aio_proc_cycle();

    (void) snd_pcm_writei(pcm, buffer, period_size);
}

#endif /* ALSA */

/*---------------------------------------------------------------------------*/

/* Initialize the audio system and start the asynchronous update.  Returns   */
/* negative on failure.  snd_errstring describes the error.                  */

int aio_init(void)
{
#ifdef ALSA
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
    snd_async_handler_t *handler;

    memset(voice, 0, sizeof (struct voice));
    memset(cycle, 0, sizeof (struct voice));

    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_sw_params_alloca(&sw_params);

    /* Initialize the audio playback device. */

    CK(snd_pcm_open(&pcm, device, SND_PCM_STREAM_PLAYBACK, 0));
    CK(set_hw_params(pcm, hw_params));
    CK(set_sw_params(pcm, sw_params));

    /* Initialize the asynchronous callback. */

    CK(snd_async_add_pcm_handler(&handler, pcm, aio_proc, NULL));

    /* Queue some silence to get things rolling. */

    if (!(buffer = calloc(period_size * CHAN, sizeof (short))))
        return -1;

    CK(snd_pcm_writei(pcm, buffer, period_size) < 0);
    CK(snd_pcm_writei(pcm, buffer, period_size) < 0);
#endif

    return 0;
}

/* Stop the audio system and free resources. */

void aio_fini(void)
{
#ifdef ALSA
    snd_pcm_close(pcm);
#endif

    if (buffer) free(buffer);
}

/*---------------------------------------------------------------------------*/

/* Add the given buffer to the voice list for playback.  Len is the buffer   */
/* length in frames (NOT bytes).  Vol is an amplitude scaler.  The calling   */
/* application must handle buffer management.                                */

int aio_play(short *buf, int len, float vol)
{
    int i;

    for (i = 0; i < MAXV; i++)
        if (voice[i].pos >= voice[i].len)
        {
            voice[i].buf = buf;
            voice[i].pos =   0;
            voice[i].len = len;
            voice[i].vol = vol;

            return i;
        }
    return -1;
}

int aio_loop(short *buf, int len, float vol)
{
    int i;

    for (i = 0; i < MAXV; i++)
        if (cycle[i].len == 0)
        {
            cycle[i].buf = buf;
            cycle[i].pos =   0;
            cycle[i].len = len;
            cycle[i].vol = vol;
            cycle[i].frq =   0;

            return i;
        }
    return -1;
}

/*---------------------------------------------------------------------------*/

void aio_freq(int i, float frq)
{
    cycle[i].frq = frq;
}

void aio_ampl(int i, float vol)
{
    cycle[i].vol = vol;
}

/*---------------------------------------------------------------------------*/

/* Load the named file into a newly allocated buffer.  Len gets the length   */
/* of the buffer in frames.  Returns NULL on failure.                        */

short *aio_load(const char *name, int *len)
{
    int fd;
    struct stat info;
    short *buf = NULL;

    *len = 0;

    if ((fd = open(name, O_RDONLY)) >= 0)
    {
        if (fstat(fd, &info) == 0)
        {
            size_t n = (size_t) info.st_size;

            if ((buf = (short *) malloc(n)) != NULL)
                *len = read(fd, buf, n) / (CHAN * sizeof (short));
        }
        close(fd);
    }
    return buf;
}

