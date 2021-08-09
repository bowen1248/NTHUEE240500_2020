#include "mbed.h"
#include <cmath>
#include "DA7212.h"
#include "uLCD_4DGL.h"
#define MAX_NAME_LENGTH 100

Serial pc(USBTX, USBRX);
DA7212 audio;
InterruptIn button(SW2);
int16_t waveform[kAudioTxBufferSize];
EventQueue queue(32 * EVENTS_EVENT_SIZE);
uLCD_4DGL uLCD(D1, D0, D2); // serial tx, serial rx, reset pin;
Thread t;

bool pause = 0;

void ISR1(){ // set pause = 1
    pause = 1;
}

class song {
    public:
        char song_name[MAX_NAME_LENGTH];
        int song_length;
        int* song_note;
        int* note_length;

    public:
        song (char* song_name, int song_length, int* song_note, int* note_length) {
            strcpy(this->song_name, song_name);
            this->song_length = song_length;
            this->song_note = new int[song_length];
            this->note_length = new int[song_length];
            for (int i = 0; i < song_length; i++)
                this->song_note[i] = song_note[i];
            for (int i = 0; i < song_length; i++)
                this->note_length[i] = note_length[i];
        }
        void load_song (char* song_name, int song_length, int* song_note, int* note_length) {
            for (int i = 0; i < MAX_NAME_LENGTH; i++)
                this->song_name[i] = '\0';
            for (int i = 0; song_name[i] == '\0'; i++)
                this->song_name[i] = song_name[i];
            this->song_length = song_length;
            delete [] this->song_note;
            delete [] this->note_length;
            this->song_note = new int[song_length];
            this->note_length = new int[song_length];
            for (int i = 0; i < song_length; i++)
                this->song_note[i] = song_note[i];
            for (int i = 0; i < song_length; i++)
                this->note_length[i] = note_length[i];
        }
        static void play_note(int freq) {
            for(int i = 0; i < kAudioTxBufferSize; i++) {
                waveform[i] = (int16_t) (sin((double)i * 2. * M_PI/(double) (kAudioSampleFrequency / freq)) * ((1<<16) - 1));
            }
            audio.spk.play(waveform, kAudioTxBufferSize);
        }
        void play_song() {
            for(int i = 0; i < song_length; i++) {
                int length = note_length[i];
                while(length--) {
                    // the loop below will play the note for the duration of 1s
                    for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j) {
                        queue.call(play_note, song_note[i]);
                        if (pause == true)
                            return;
                    }
                    wait(1.0);
                }
            }
        }

};

class terminal {
    public:
        terminal(){

        }
        void show_song_name (int col, int row ,song in_song) {
            uLCD.locate(col, row);
            uLCD.printf("%s", in_song.song_name); //Default Green on black text
        }
};
int song1_note[42] = {
  261, 261, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 261,
  392, 392, 349, 349, 330, 330, 294,
  392, 392, 349, 349, 330, 330, 294,
  261, 261, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 261
};

int song1_noteLength[42] = {
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2
};

int main(void) {
    t.start(callback(&queue, &EventQueue::dispatch_forever));
    button.rise(&ISR1);
    terminal music_player;
    song song1 ("little star", 42, song1_note, song1_noteLength);
    while (1) {
        music_player.show_song_name(0,0,song1);
        song1.play_song();
        pause = 0;
    }
    return 0;
}

