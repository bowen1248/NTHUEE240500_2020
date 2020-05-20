#include "mbed.h"
#include <cmath>
#include "DA7212.h"
#include "uLCD_4DGL.h"
#include "accelerometer_handler.h"
#include "config.h"
#include "magic_wand_model_data.h"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#define signalLength 128
#define bufferLength 32
DA7212 audio;
Serial pc(USBTX, USBRX);
int16_t waveform[kAudioTxBufferSize];
EventQueue queue(64 * EVENTS_EVENT_SIZE);
EventQueue queue2(64 * EVENTS_EVENT_SIZE);
DigitalOut green_led(LED2);
InterruptIn button(SW2);
Timer debounce;
uLCD_4DGL uLCD(D1, D0, D2); // serial tx, serial rx, reset pin;
Thread t1;
Thread t2;
int gesture_index = -1;
int mode_selection = 0;
int song_selection = 0;
int state = 0;
// Create an area of memory to use for input, output, and intermediate arrays.
// The size of this will depend on the model you're using, and may need to be
// determined by experimentation.
constexpr int kTensorArenaSize = 60 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];
int custom_song_length = 0;
int custom_note[signalLength];
int custom_note_length[signalLength];
char serialInBuffer[bufferLength];
int song1[42] = {
  261, 261, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 261,
  392, 392, 349, 349, 330, 330, 294,
  392, 392, 349, 349, 330, 330, 294,
  261, 261, 392, 392, 440, 440, 392,
  349, 349, 330, 330, 294, 294, 261};
  
int noteLength1[42] = {
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 2};

int song2[42] = {
  330, 294, 262, 294, 330, 330, 330,
  294, 294, 294, 330, 392, 392, 330,
  294, 262, 294, 330, 330, 330, 294,
  294, 330, 294, 262, 330, 294, 262,
  294, 330, 330, 330, 294, 294, 294,
  330, 392, 392, 330, 294, 262, 294};

int noteLength2[42] = {
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 2, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 2, 1,
  1, 1, 1, 2, 1, 1, 1,
  1, 1, 1, 2, 1, 1, 2,
  1, 1, 2, 1, 1, 1, 1};

int song3[42] = {
  330, 330, 349, 392, 392, 349, 330,
  294, 261, 261, 294, 330, 330, 294,
  294, 330, 330, 349, 392, 392, 349,
  330, 294, 261, 261, 294, 330, 293,
  262, 262, 294, 294, 330, 261, 294,
  330, 349, 330, 261, 294, 330, 349};

int noteLength3[42] = {
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 2,
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1};

void ISR1() {
    if(debounce.read_ms() > 100){
        state += 1;
        debounce.reset();
    }
}

void loadSong(void) {
    green_led = 0;
    int i = 0;
    int serialCount = 0;
    int readit = 0;
    while (pc.readable())
        char trash = pc.getc();
    while (!readit) {
        if(pc.readable()) {
            serialInBuffer[serialCount] = pc.getc();
            serialCount++;
            if(serialCount == 4) {
                serialInBuffer[serialCount] = '\0';
                custom_song_length = (int) atof(serialInBuffer);
                readit++;
            }
        }
    }
    serialCount = 0;
    while(i < custom_song_length) {
        if(pc.readable()) {
            serialInBuffer[serialCount] = pc.getc();
            serialCount++;
        if(serialCount == 4) {
            serialInBuffer[serialCount] = '\0';
            custom_note[i] = (int) atof(serialInBuffer);
            serialCount = 0;
            i++;
            }
        }
    }
    serialCount = 0;
    i = 0;
    while(i < custom_song_length) {
        if(pc.readable()) {
            serialInBuffer[serialCount] = pc.getc();
            serialCount++;
            if(serialCount == 4) {
                serialInBuffer[serialCount] = '\0';
                custom_note_length[i] = (int) atof(serialInBuffer);
                serialCount = 0;
                i++;
            }
        }
    }
    /*
    pc.printf("%d\r\n", custom_song_length);
    for (int i = 0; i < custom_song_length; i++) {
      pc.printf(" %d", custom_note[i]);
      wait(0.1);
    }
    pc.printf("\r\n");
    for (int i = 0; i < custom_song_length; i++) {
      pc.printf(" %d", custom_note_length[i]);
      wait(0.1);
    }
    pc.printf("\n");
    */
    green_led = 1;
}
void playNote(int freq) {
    for(int i = 0; i < kAudioTxBufferSize; i++) {
        waveform[i] = (int16_t) (sin((double)i * 2. * M_PI/(double) (kAudioSampleFrequency / freq)) * ((1<<16) - 1));
    }
    audio.spk.play(waveform, kAudioTxBufferSize);
}
void playSong(int song_length, int* song_note, int* note_length) {
    for(int i = 0; i < song_length; i++) {
        int length = note_length[i];
        while(length--) {
            // the loop below will play the note for the duration of 1s
            if (state != 0) {
                queue.call(playNote, 0);
                return;
            }
            for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j) {
                queue.call(playNote, song_note[i]);
            }
        if(length < 1)
            wait(1.0);
        }
    }
}

int PredictGesture(float* output) {
    // How many times the most recent gesture has been matched in a row
    static int continuous_count = 0;
    // The result of the last prediction
    static int last_predict = -1;
    // Find whichever output has a probability > 0.8 (they sum to 1)
    int this_predict = -1;
    for (int i = 0; i < label_num; i++) {
        if (output[i] > 0.8) this_predict = i;
    }
    // No gesture was detected above the threshold
    if (this_predict == -1) {
        continuous_count = 0;
        last_predict = label_num;
        return label_num;
    }
    if (last_predict == this_predict) {
        continuous_count += 1;
    } 
    else {
        continuous_count = 0;
    }
    last_predict = this_predict;

    // If we haven't yet had enough consecutive matches for this gesture,
    // report a negative result
    if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {
        return label_num;
    }

    // Otherwise, we've seen a positive result, so clear all our variables
    // and report it
    continuous_count = 0;
    last_predict = -1;
    return this_predict;
}
void change_arrow (int row) {
  uLCD.locate(0,mode_selection);
  uLCD.printf("  ");
  uLCD.locate(0,row);
  uLCD.printf("=>");
  return;
}
//-----------main function
int main(void) {
    t1.start(callback(&queue, &EventQueue::dispatch_forever));
    t2.start(callback(&queue2, &EventQueue::dispatch_forever));
    green_led = 1;
    pc.baud(9600);
    debounce.start();
    button.rise(&ISR1);
    // Whether we should clear the buffer next time we fetch data
    bool should_clear_buffer = false;
    bool got_data = false;
    gesture_index = -1;

    // ================Set up dnn network.
    static tflite::MicroErrorReporter micro_error_reporter;
    tflite::ErrorReporter* error_reporter = &micro_error_reporter;
    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report(
            "Model provided is schema version %d not equal "
            "to supported version %d.",
            model->version(), TFLITE_SCHEMA_VERSION);
        return -1;
    }

    // Pull in only the operation implementations we need.
    // This relies on a complete list of all the ops needed by this graph.
    // An easier approach is to just use the AllOpsResolver, but this will
    // incur some penalty in code space for op implementations that are not
    // needed by this graph.

    static tflite::MicroOpResolver<6> micro_op_resolver;
    micro_op_resolver.AddBuiltin(
    tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
    tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                               tflite::ops::micro::Register_MAX_POOL_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                               tflite::ops::micro::Register_CONV_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                               tflite::ops::micro::Register_FULLY_CONNECTED());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                               tflite::ops::micro::Register_SOFTMAX());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE,
                               tflite::ops::micro::Register_RESHAPE(), 1);

    // Build an interpreter to run the model with
        static tflite::MicroInterpreter static_interpreter(
            model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
    tflite::MicroInterpreter* interpreter = &static_interpreter;
    // Allocate memory from the tensor_arena for the model's tensors
    interpreter->AllocateTensors();

    // Obtain pointer to the model's input tensor
    TfLiteTensor* model_input = interpreter->input(0);
    if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
        (model_input->dims->data[1] != config.seq_length) ||
        (model_input->dims->data[2] != kChannelNumber) ||
        (model_input->type != kTfLiteFloat32)) {
        error_reporter->Report("Bad input tensor parameters in model");
        return -1;
    }
    int input_length = model_input->bytes / sizeof(float);
    TfLiteStatus setup_status = SetupAccelerometer(error_reporter);

    if (setup_status != kTfLiteOk) {
        error_reporter->Report("Set up failed\n");
        return -1;
    }
    error_reporter->Report("Set up successful...\n");
    // ======================== end of initialize

    while (1) {
        uLCD.cls();
        uLCD.locate(0, 0);
        if (mode_selection == 3) {
            uLCD.printf("%s", "Custom song\r\n");
            playSong(custom_song_length, custom_note, custom_note_length);
        }
        else if (song_selection == 0) {
            uLCD.printf("%s", "Little star\r\n"); //Default Green on black text
            playSong(42, song1, noteLength1);
        }
        else if (song_selection == 1) {
            uLCD.printf("%s", "Mary's sheep\r\n");
            playSong(42, song2, noteLength2);
        }
        else if (song_selection == 2) {
            uLCD.printf("%s", "Happy song\r\n");
            playSong(42, song3, noteLength3);
        }
        uLCD.cls();
        uLCD.locate(0, 0);
        uLCD.printf("   Forward\r\n"); //Default Green on black text
        uLCD.printf("   Backward\r\n");
        uLCD.printf("   Change song\r\n");
        uLCD.printf("   Load song from\r\n   PC\r\n");
        uLCD.locate(0, 0);
        uLCD.printf("=>");
        mode_selection = 0;
        while (state == 1) {
            gesture_index = -1;
            while (gesture_index < 0 || gesture_index >= label_num) {
                if (state != 1) {
                    should_clear_buffer = true;
                    break;
                }
                // Attempt to read new data from the accelerometer
                got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                            input_length, should_clear_buffer);

                // If there was no new data,
                // don't try to clear the buffer again and wait until next time

                if (!got_data) {
                    should_clear_buffer = false;
                    continue;
                }

                // Run inference, and report any error
                TfLiteStatus invoke_status = interpreter->Invoke();
                if (invoke_status != kTfLiteOk) {
                    error_reporter->Report("Invoke failed on index: %d\n", begin_index);
                continue;
                }

                // Analyze the results to obtain a prediction
                gesture_index = PredictGesture(interpreter->output(0)->data.f);
                // Clear the buffer next time we read data
                should_clear_buffer = gesture_index < label_num;
            }
            if (gesture_index == 0 && mode_selection > 0) {
                uLCD.locate(0,mode_selection);
                uLCD.printf("  ");
                mode_selection--;
                uLCD.locate(0,mode_selection);
                uLCD.printf("=>");
            } else if (gesture_index == 1 && mode_selection < 3) {
                uLCD.locate(0,mode_selection);
                uLCD.printf("  ");
                mode_selection++;
                uLCD.locate(0,mode_selection);
                uLCD.printf("=>");
            }
        }
        if (mode_selection == 0 && song_selection < 2)
            song_selection++;
        else if (mode_selection == 1 && song_selection >= 0)
            song_selection--;
        else if (mode_selection == 3) {
            uLCD.cls();
            uLCD.printf("Loading Song...\r\n");
            loadSong();
        }
        // change song
        if (mode_selection == 2) {
            uLCD.cls();
            uLCD.locate(0,0);
            uLCD.printf("   Little star\r\n"); //Default Green on black text
            uLCD.printf("   Mary's sheep\r\n");
            uLCD.printf("   Happy song\r\n");
            uLCD.locate(0, 0);
            uLCD.printf("=>");
        }
        while (state == 2 && mode_selection == 2) {
            gesture_index = -1;
            while (gesture_index < 0 || gesture_index >= label_num) {
                if (state != 2) {
                    should_clear_buffer = true;
                    break;
                }
                // Attempt to read new data from the accelerometer
                got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                            input_length, should_clear_buffer);

                // If there was no new data,
                // don't try to clear the buffer again and wait until next time

                if (!got_data) {
                    should_clear_buffer = false;
                    continue;
                }

                // Run inference, and report any error
                TfLiteStatus invoke_status = interpreter->Invoke();
                if (invoke_status != kTfLiteOk) {
                    error_reporter->Report("Invoke failed on index: %d\n", begin_index);
                continue;
                }

                // Analyze the results to obtain a prediction
                gesture_index = PredictGesture(interpreter->output(0)->data.f);
                // Clear the buffer next time we read data
                should_clear_buffer = gesture_index < label_num;
            }
            if (gesture_index == 0 && song_selection > 0) {
                uLCD.locate(0, song_selection);
                uLCD.printf("  ");
                song_selection--;
                uLCD.locate(0, song_selection);
                uLCD.printf("=>");
            } else if (gesture_index == 1 && song_selection < 2) {
                uLCD.locate(0, 0);
                uLCD.printf("  ");
                uLCD.locate(0, song_selection);
                uLCD.printf("  ");
                song_selection++;
                uLCD.locate(0, song_selection);
                uLCD.printf("=>");
            }
            pc.printf("song: %d %d\r\n", gesture_index, song_selection);
        }
        pc.printf("fuckyou%d\r\n", gesture_index);
        state = 0;
    }
    return 0;
}

