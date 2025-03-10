#include <iostream>
#include <stdlib.h>
#include <portaudio.h>
#include <fftw3.h>

#define NUM_CHANNELS 2
#define FRAMES_PER_BUFFER 512
#define SPECTRO_FREQUENCY_START 20
#define SPECTRO_FREQUENCY_END 20000 // Hertz

typedef struct streamCallbackData {
    double* in;
    double* out;
    fftw_plan p;
    int startIndex;
    int spectroSize;
};

static streamCallbackData* spectroData;


static int patestCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags flags, void* userData 
) {
    float* in = (float*)inputBuffer;
    (void)outputBuffer;

    streamCallbackData* callbackData = (streamCallbackData*) userData;

    int dispSize = 100;
    

    float vol_l = 0;
    float vol_r = 0; 

    for(uint i = 0; i < NUM_CHANNELS*FRAMES_PER_BUFFER; i+=2) {
        vol_l =  vol_l > abs(in[i]) ? vol_l : abs(in[i]);
        vol_r =  vol_r > abs(in[i+1]) ? vol_r : abs(in[i+1]);
    }

    for(int i = 0; i < dispSize; i++) {
        float volPer = (float)i / (float)dispSize;
        if(volPer <= vol_l && volPer <= vol_r) {
            std::cout <<  "█";
        } else if(volPer <= vol_l) {
            std::cout << "▀";
        } else if(volPer <= vol_r) {
            std::cout << "▄";
        } else {
            std::cout << " ";
        }
    }
    fflush(stdout);
    return 0;
}

int main(int nargs, char* argv[]) {
    freopen("/dev/null","w",stderr);
    if(Pa_Initialize() != paNoError) {
        std::cout << "ERROR || Could not initialize PortAudio" << std::endl;
        return -1;
    }
    freopen("/dev/tty", "w", stderr);

    int audioDevices = Pa_GetDeviceCount();

    if(audioDevices < 1) {
        std::cout << "ERROR || No audio devices detected" << std::endl;
    }


    std::cout << "Number of devices: " << audioDevices << std::endl;
    spectroData = new streamCallbackData;
    spectroData->in = new double[FRAMES_PER_BUFFER];
    spectroData->out = new double[FRAMES_PER_BUFFER];

    spectroData->p = fftw_plan_r2r_1d(FRAMES_PER_BUFFER, spectroData->in, spectroData->out, FFTW_R2HC, FFTW_ESTIMATE);

    const PaDeviceInfo* deviceInfo;
    int selected_device;
    int sample_rate;
    for(int i = 0; i < audioDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
        if(deviceInfo->maxInputChannels == 32 && deviceInfo->maxOutputChannels == 32) {
            selected_device = i;
            sample_rate = deviceInfo->defaultSampleRate;
            break;
        }
    }
    
    // Create a stream
    PaStreamParameters input_params = PaStreamParameters();
    PaStreamParameters output_params = PaStreamParameters();

    input_params.channelCount = NUM_CHANNELS;
    input_params.device = selected_device;
    input_params.hostApiSpecificStreamInfo = NULL;
    input_params.sampleFormat = paFloat32;
    input_params.suggestedLatency = Pa_GetDeviceInfo(selected_device)->defaultLowInputLatency;

    output_params.channelCount = 2;
    output_params.device = selected_device;
    output_params.hostApiSpecificStreamInfo = NULL;
    output_params.sampleFormat = paFloat32;
    output_params.suggestedLatency = Pa_GetDeviceInfo(selected_device)->defaultLowInputLatency;

    PaStream* stream;
    PaError err = Pa_OpenStream(&stream, &input_params, &output_params, 
        sample_rate, FRAMES_PER_BUFFER, paNoFlag, 
        patestCallback, spectroData
        );

    if( err != paNoError) {
        std::cout << "ERROR || Could not initialize PortAudio" << std::endl;
        return -1;
    }

    err = Pa_StartStream(stream);
    Pa_Sleep(10 * 1000);
    err = Pa_CloseStream(stream);

    Pa_Terminate();
    std::cout << "\r";
    fftw_destroy_plan(spectroData->p);
    fftw_free(spectroData->in);
    fftw_free(spectroData->out);
    delete spectroData;
}