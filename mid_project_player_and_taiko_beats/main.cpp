#include "mbed.h"
#include <cmath>
#include "DA7212.h"
#define MAX_NAME_LENGTH 100

Serial pc(USBTX, USBRX);
DA7212 audio;
int16_t waveform[kAudioTxBufferSize];
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;

class song {
    private:
        char song_name[MAX_NAME_LENGTH];
        int song_length;
        int* song_note;
        int* note_length;

    public:
        song (char* song_name, int song_length, int* song_note, int* note_length) {
            for (int i = 0; song_name[i] == '\0'; i++)
                this->song_name[i] = song_name[i];
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
            for (int i = 0; i < kAudioTxBufferSize; i++) {
            waveform[i] = (int16_t) (sin((double)i * 2. * M_PI/(double) (kAudioSampleFrequency / freq)) * ((1<<16) - 1));
            }
            // the loop below will play the note for the duration of 1s
            for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j) {
            audio.spk.play(waveform, kAudioTxBufferSize);
            }
        }
        void play_song() {
            for(int i = 0; i < song_length; i++) {
                int length = note_length[i];
                while(length--) {
                    queue.call(play_note, song_note[i]);
                    wait(1.0);
                }
            }
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
    song little_star ("little star\0", 42, song1_note, song1_noteLength);
    little_star.play_song();
    return 0;
}

