#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s input_text.txt output_audio.wav output_audio.webm\n", argv[0]);
        return 1;
    }

    char* input_file = argv[1];
    char* output_wav_file = argv[2];
    char* output_webm_file = argv[3];

    char command[1024];
    sprintf(command, "pico2wave -w %s -l fr-FR < %s", output_wav_file, input_file);
    system(command);
    
    sprintf(command, "ffmpeg -v quiet -y -i %s -c:a libvorbis -b:a 64k %s", output_wav_file, output_webm_file);
    system(command);

    return 0;
}

