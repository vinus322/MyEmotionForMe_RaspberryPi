#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
 
#include "wavFile.h"
 
WAVHEADER wavheader;

int Play = 1;
int MusicQuit = 0;

 
int playMusic(char* argv)
{
	int fd = -1;                                                    
	int rc, buf_size, dir;
	int channels, format;                                   
        long loops, count;
	unsigned int val;
	char *buffer;                                      
	
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;
	    
	printf("Playing file : %s\n", argv);
	
	
	if((fd = open(argv, O_RDONLY)) == -1) {
	        printf("Could not open the specified wave file : %s\n", argv[1]);
	        return -1;
	}
						     
	if((count = read(fd, &wavheader, sizeof(WAVHEADER))) < 1) {
		printf("Could not read wave data\n");
		return -1;
	}
	rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	
	if(rc < 0) {
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
		return -1;
	}
								 
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);
	
	channels = wavheader.nChannels;
	printf("Wave Channel Mode : %s\n", (channels)? "Stereo":"Mono");
	snd_pcm_hw_params_set_channels(handle, params, channels);
		
	printf("Wave Bytes : %d\n", wavheader.nblockAlign);
	switch(wavheader.nblockAlign) {
		case 1: 
			format = SND_PCM_FORMAT_U8; 
			break;
		case 2:
			format = (!channels)?SND_PCM_FORMAT_S16_LE:SND_PCM_FORMAT_U8;
			break;
		case 4:
			format = SND_PCM_FORMAT_S16_LE;
			break;
		default:
			printf("Unknown byte rate for sound\n");
			break;
	};
	
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, format);
	 
	printf("Wave Sampling Rate : 0x%d\n", wavheader.sampleRate);
	val = wavheader.sampleRate;
	snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	
	frames = 32;
	snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
	rc = snd_pcm_hw_params(handle, params);
	
	if(rc < 0) { 
		fprintf(stderr, "Unable to set hw parameters: %s\n", snd_strerror(rc));
		return -1;
	}
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	buf_size = frames * channels * ((format == SND_PCM_FORMAT_S16_LE)?2:1); 
	buffer = (char*)malloc(buf_size);
	
	snd_pcm_hw_params_get_period_time(params, &val, &dir); 
	do {
		if(!Play) continue;
		if(MusicQuit) break;
		if((count = read(fd, buffer, buf_size)) <= 0) break;
		rc = snd_pcm_writei(handle, buffer, frames);
		
		if(rc == -EPIPE) {
			fprintf(stderr, "Underrun occurred\n");
			snd_pcm_prepare(handle);
		} 
		else if(rc < 0) {
	                 fprintf(stderr, "error from write: %s\n", snd_strerror(rc));
	        } else if(rc != (int)frames) {
	                 fprintf(stderr, "short write, write %d frames\n", rc);
	        }
	        } while(count == buf_size);
																		    	
end:
	MusicQuit =0;																	        close(fd);
	snd_pcm_drain(handle);
    	snd_pcm_close(handle);                  /* ?ъ슜???앸궃 ?μ튂 ?リ린 */
	free(buffer);
	return 0;
}
