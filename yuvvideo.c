#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <pthread.h>

//#include <bcm_host.h>
#include <IL/OMX_Broadcom.h>

#ifndef BARESIP
#define info printf
#define warn printf
#define error printf
#endif


static int EventHandler(OMX_HANDLETYPE hComponent, void* pAppData, OMX_EVENTTYPE eEvent,
        OMX_U32 nData1, OMX_U32 nData2, void* pEventData)
{	
	switch (eEvent) {
		case OMX_EventCmdComplete:
			//info("omx.EventHandler: Previous command completed\n");
			printf("!!!omx.EventHandler: Previous command completed\n"
				"d1=%x\td2=%x\teventData=%p\tappdata=%p\n",
				nData1, nData2, pEventData, pAppData);
			// Set a semaphor, put event into mt queue?
			break;
		case OMX_EventError:
			warn("!!!omx.EventHandler: Error event type data1=%x\tdata2=%x\n", nData1, nData2);
			break;
		default:
			warn("!!!omx.EventHandler: Unknown event type %d\tdata1=%x\tdata2=%x\n", eEvent, nData1, nData2);
			return -1;
			break;
	}
	return 0;
}
		
static int EmptyBufferDone(OMX_HANDLETYPE hComponent, void* pAppData,
        OMX_BUFFERHEADERTYPE* pBuffer)
{
	puts("EmptyBufferDone");
	return 0;
}

static OMX_ERRORTYPE FillBufferDone(OMX_HANDLETYPE hComponent,
        OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	puts("FillBufferDone2");
	return 0;
}

static struct OMX_CALLBACKTYPE callbacks = {
	&EventHandler,
	&EmptyBufferDone,
	&FillBufferDone
};

// GLOBAL BUFFER

static OMX_BUFFERHEADERTYPE* bufhdr;
static OMX_HANDLETYPE video_render;
static pthread_mutex_t omx_mutex;
  
void omx_bcm_init(void)  
{
   bcm_host_init();
}
  
int omx_alloc_render(void)
{   
   pthread_mutex_init(&omx_mutex, 0);
   pthread_mutex_lock(&omx_mutex);
   
   if(OMX_Init() != OMX_ErrorNone)
   {
      return -4;
   }
   
   OMX_VIDEO_PARAM_PORTFORMATTYPE format;
   
   int status = 0;
   assert(status == 0);
   
	OMX_ERRORTYPE err = OMX_GetHandle(&video_render, "OMX.broadcom.video_render", 0x1337, &callbacks);
		
	if (err) {
		puts("OMX_GetHandle error");
		return 0;
	}
	
	puts("OMX_GetHandle success");
	puts("created video_render component");	
	     
	unsigned int data_len = 0;
	memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
	format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
	format.nVersion.nVersion = OMX_VERSION;
	format.nPortIndex = 90;	
	format.nIndex = 0; // ????

	format.xFramerate = 15 << 16;
	format.eCompressionFormat = OMX_VIDEO_CodingUnused;
	format.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
   
	//puts("Set Format parameters");
	//if (OMX_SetParameter(video_render, OMX_IndexParamVideoPortFormat, &format) != OMX_ErrorNone) {
	//	puts("fucking error setting format on video_render");
	//	exit(-23);
	//}
	
	/*
	puts("sleep(1)");
	sleep(1);
	int state;
	puts("to idle state; sleep(1)");
	OMX_SendCommand(video_render, OMX_CommandStateSet, OMX_StateIdle, NULL);
	sleep(1);
	err = OMX_GetState(video_render, &state);
	if (err != OMX_ErrorNone || state != OMX_StateIdle) {
		puts("first state ever: not in idle state");
	    return -1;
	}*/
	
	//puts("enable port 90");
	//int portIndex = 90;
	//err = OMX_SendCommand(video_render, OMX_CommandPortEnable, portIndex, NULL);
	//assert(err == OMX_ErrorNone);	
	
	
	/*
	int state;
	OMX_SendCommand(video_render, OMX_CommandStateSet, OMX_StateIdle, NULL);
	puts("to idle state");
	sleep(1);
	err = OMX_GetState(video_render, &state);
	if (error != OMX_ErrorNone || state != OMX_StateIdle) {
		puts("omx_alloc_render: video_render not in idle state");
	    return -1;
	}
	puts("to execute state");
	OMX_SendCommand(video_render, OMX_CommandStateSet, OMX_StateExecuting, NULL);
	//sleep(1);
	err = OMX_GetState(video_render, &state);
	if (err != OMX_ErrorNone || state != OMX_StateExecuting) {
		puts("video_render not in executing state");
	    return -1;
	}*/
	
	pthread_mutex_unlock(&omx_mutex);
	return 0;
}

int omx_update_size(int width, int height) {
	
   pthread_mutex_lock(&omx_mutex);
	info("omx_update_size %d %d\n", width, height);
	
   // Where is the resolution?? -> In the port definition

	//status = ilclient_change_component_state(video_render, OMX_StateIdle);
	//assert(status == 0);

	OMX_PARAM_PORTDEFINITIONTYPE portdef;
	
	memset(&portdef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portdef.nVersion.nVersion = OMX_VERSION;
	portdef.nPortIndex = 90;
	// work out buffer requirements, check port is in the right state

	int err = OMX_GetParameter(video_render, OMX_IndexParamPortDefinition, &portdef);
	
	portdef.format.video.nFrameWidth = 320;
	portdef.format.video.nFrameHeight = 240;
	portdef.format.video.nStride = 320;
	portdef.format.video.nSliceHeight = 240;
	
	err = OMX_SetParameter(video_render, OMX_IndexParamPortDefinition, &portdef);
	err = OMX_GetParameter(video_render, OMX_IndexParamPortDefinition, &portdef);
	
	puts("set port definition parameter");
	
	printf("Port: enabled=%d\n", portdef.bEnabled);
	
	OMX_SendCommand(video_render, OMX_CommandStateSet, OMX_StateIdle, NULL);
	puts("to idle state");
	usleep(1000);
	
	int i;
		for (i = 0; i < portdef.nBufferCountActual; i++) {
			printf("Treating Buffer %d\n", i);
			//unsigned char *buf= vcos_malloc_aligned(portdef.nBufferSize, portdef.nBufferAlignment, "bufname");
			unsigned char *buf;
			posix_memalign(&buf, portdef.nBufferAlignment, portdef.nBufferSize);
			assert(buf && "Allocation Error");
			
		//	OMX_BUFFERHEADERTYPE* bufhdr;
		const int portIndex = 90;
			int err = OMX_UseBuffer(video_render, &bufhdr, portIndex, 0x777, portdef.nBufferSize, buf);
			assert(err == OMX_ErrorNone);
		} 
		
	   if(i != portdef.nBufferCountActual) {
		   puts("wrong buffer counter");
		}
	
	
	/*
	int state;
	puts("to execute state");
	OMX_SendCommand(video_render, OMX_CommandStateSet, OMX_StateExecuting, NULL);
	//sleep(1);
	err = OMX_GetState(video_render, &state);
	if (err != OMX_ErrorNone || state != OMX_StateExecuting) {
		puts("video_render not in executing state");
	    return -1;
	}
	//sleep(1);
	* */
	
   pthread_mutex_unlock(&omx_mutex);
	
}

int omx_send_frame(void* data, uint32_t len)
{
	
	pthread_mutex_lock(&omx_mutex);
   
	int state;
	// Verify Executing state of video_render component
	int err = OMX_GetState(video_render, &state);
	//printf("Current State of video_render: %d\n", state);   
	
	if (err != OMX_ErrorNone || state != OMX_StateExecuting) {
		puts("video_render not in executing state");
		OMX_SendCommand(video_render, OMX_CommandStateSet, OMX_StateExecuting, NULL);
		puts("OMX_SendCommand StateExecuting, sleep(1)\n");
		sleep(1);
	    //return -1;
	}
	
	
	
	static int first_packet = 1;
	int status = 0;
	uint32_t data_len = 0;

    unsigned char *dest = bufhdr->pBuffer;
/*
         if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
             (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
         {
            port_settings_changed = 1;

            ilclient_change_component_state(video_render, OMX_StateExecuting);
         }
*/
         
    if (len > bufhdr->nAllocLen) {
		puts("Too large frame for OMX Buffer size");
		return -3;
	}
    memcpy(dest, data, len);
    bufhdr->nFilledLen = len;
    bufhdr->nOffset = 0;
    
    if(first_packet)
	{
		bufhdr->nFlags = OMX_BUFFERFLAG_STARTTIME;
		first_packet = 0;
	}
	else
	{
		bufhdr->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
	}

    if(OMX_EmptyThisBuffer(video_render, bufhdr) != OMX_ErrorNone) {
		puts("OMX_EmptyThisBuffer error");
		return -6;
    }

/*
      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

      if(OMX_EmptyThisBuffer(video_render, buf) != OMX_ErrorNone)
         status = -20;
*/
      // wait for EOS from render

   //OMX_Deinit();

   //ilclient_destroy(client);
   
   pthread_mutex_unlock(&omx_mutex);
   return status;   
}


int main (int argc, char **argv)
{
   if (argc < 2) {
      printf("Usage: %s <filename>\n", argv[0]);
      exit(1);
   }
   
   FILE *in;
   if((in = fopen(argv[1], "rb")) == NULL) {
	   perror("fopen");
      return -2;
  }
      
   omx_bcm_init();   
   omx_alloc_render();
   omx_update_size(320, 240);
   
   while (1) {
	   unsigned int frame_size = 320 * 240;
	   void* data = malloc(frame_size);
	   frame_size = fread(data, 1, frame_size, in);
	   
	   omx_send_frame(data, frame_size);
	}
   
   fclose(in);
}


