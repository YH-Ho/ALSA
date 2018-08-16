#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <stdint.h>
#include <alsa/asoundlib.h>

#define KEYCODE_U 0x41
#define KEYCODE_D 0x42
#define KEYCODE_R 0x43
#define KEYCODE_L 0x44

static int g_fd = 0;

struct termios g_termios, g_raw_termios;

void quit(int){
    printf("\nExit.\n");

    /* Restore original configuration */
    tcsetattr(g_fd, TCSANOW, &g_termios);

    exit(0);
}

void volumeCtrl(const char *mixer_name, const char *selem_name){
    char c;
    long volume_min = -1;
    long volume_max = -1;
    long volume_left = -1;
    long volume_right = -1;
    long volume = -1;

    snd_mixer_t *mixer;
    snd_mixer_selem_id_t *id;
    snd_mixer_elem_t* elem ;

    /* Get the console in raw mode */
    tcgetattr(g_fd, &g_termios);
    memcpy(&g_raw_termios, &g_termios, sizeof(struct termios));
    g_raw_termios.c_lflag &=~ (ICANON | ECHO);

    /* Set a new line, then end of file */
    g_raw_termios.c_cc[VEOL] = 1;
    g_raw_termios.c_cc[VEOF] = 2;
    tcsetattr(g_fd, TCSANOW, &g_raw_termios);

    /* Alsa init */
    snd_mixer_selem_id_alloca(&id);

    /* open deivce */
    if (snd_mixer_open(&mixer, 0) < 0) {
        printf("snd_mixer_open failed\n");
        return;
    }

    snd_mixer_attach(mixer, mixer_name);
    snd_mixer_selem_register(mixer, NULL, NULL);
    snd_mixer_load(mixer);

    snd_mixer_selem_id_set_index(id, 0);
    snd_mixer_selem_id_set_name(id, selem_name);
    elem = snd_mixer_find_selem(mixer, id);

    snd_mixer_selem_get_playback_volume_range(elem, &volume_min, &volume_max);

    /* Get the currunt volume value */
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &volume_left);
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, &volume_right);

    /* Calculate the average volume value of left and right channel */
    volume = (volume_left + volume_right) / 2;

    /* Display conversion value (0-100) */
    printf("Current Volume : %.0f", (float) volume / volume_max * 100);
    fflush(stdout);

    while(1){
        /* Wait for the input control */
        if(read(g_fd, &c, 1) < 0){
            perror("read");
            exit(-1);
        }

        /* Get the currunt volume value */
        snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &volume_left);
        snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, &volume_right);

        /* Calculate the average volume value of left and right channel */
        volume = (volume_left + volume_right) / 2;

        printf("\r\r\033[K");
        fflush(stdout);

        if(c == KEYCODE_L || c == KEYCODE_D){
            if(volume > volume_min)
                volume -= 1;
            snd_mixer_selem_set_playback_volume_all(elem, volume);
        }

        if(c == KEYCODE_R || c == KEYCODE_U){
            if(volume < volume_max)
                volume += 1;
            snd_mixer_selem_set_playback_volume_all(elem, volume);
        }

        /* Display conversion value (0-100) */
        printf("Current Volume : %.0f", (float) volume / volume_max * 100);
        fflush(stdout);
    }

    snd_mixer_close(mixer);
    return;
}

static void usage(FILE *fp, int argc, char **argv){
    fprintf (fp,
             "Usage: %s [options]\n\n"
             "Options:\n\n"
             "* It is not recommended to modify the parameters.\n"
             "  -m | --mixer_name   Set the mixer name. Default: default \n"
             "  -s | --selem_name   Set the selem name. Default: PCM     \n"
	     "  -h | --help         usage                              \n\n"
             "",
             basename(argv[0]));
}

static void usageOfCtrl(){
    printf ("-- Use the arrow keys to control volume --\n\n"
            "Volume Up   : right→) or up(↑)\n"
            "Volume Down : left(←) or down(↓)\n\n");
}

static const char short_options [] = "m:s:h";

static const struct option long_options [] = {
    { "mixer_name",     required_argument,      NULL,           'm' },
    { "selem_name",     required_argument,      NULL,           's' },
    { "help",       	no_argument,            NULL,           'h' },
    { 0, 0, 0, 0 }
};

int main(int argc, char** argv){
    signal(SIGINT,quit);

    const char *mixer_name = "default";
    const char *selem_name = "PCM";

    for(;;){
        int index;
        int c;

        c = getopt_long(argc, argv,short_options, long_options,&index);

        if(-1 == c)
            break;

        switch(c){
        case 0:
            exit(EXIT_FAILURE);
        case 'm':
            mixer_name = optarg;
            break;
        case 's':
            selem_name = optarg;
            break;
        case 'h':
            usage(stdout, argc, argv);
            exit(EXIT_SUCCESS);
        default:
            usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    /* Print the usage about usage */
    usageOfCtrl();

    volumeCtrl(mixer_name, selem_name);

    return 0;
}
