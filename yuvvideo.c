#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

//#include <bcm_host.h>
#include <IL/OMX_Broadcom.h>


int EventHandler(
        OMX_HANDLETYPE hComponent,
        OMX_PTR pAppData,
        OMX_EVENTTYPE eEvent,
        OMX_U32 nData1,
        OMX_U32 nData2,
        OMX_PTR pEventData) {
			
			
	puts("OMX EventHandler");
			return 0;
		}
		
OMX_ERRORTYPE EmptyBufferDone(OMX_HANDLETYPE hComponent,
        OMX_PTR pAppData,
        OMX_BUFFERHEADERTYPE* pBuffer)
{
	puts("EmptyBufferDone");
	volatile int* i = 0;
	//puts("now crash");
	//return 0;
}

OMX_ERRORTYPE FillBufferDone(OMX_HANDLETYPE hComponent,
        OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	puts("FillBufferDone2");
	return 0;
}


struct OMX_CALLBACKTYPE callbacks = {
	&EventHandler,
	&EmptyBufferDone,
	&FillBufferDone
};

static int video_decode_test(char *filename)
{
   OMX_VIDEO_PARAM_PORTFORMATTYPE format;
   FILE *in;
   
   int status = 0;
   if((in = fopen(filename, "rb")) == NULL)
      return -2;

   if(OMX_Init() != OMX_ErrorNone)
   {
      fclose(in);
      return -4;
   }
   
   assert(status == 0);
   
	OMX_HANDLETYPE video_render;
	OMX_ERRORTYPE err = OMX_GetHandle(&video_render, "OMX.broadcom.video_render", 0x1337, &callbacks);
		
	if (err) {
		puts("OMX_GetHandle error");
		return 0;
	}
	
	puts("OMX_GetHandle success");
	puts("created video_render component");
	
	
	int state, error;
	
         
   unsigned int data_len = 0;
   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 90;	
   format.nIndex = 0; // ????
   
   format.xFramerate = 15 << 16;
   format.eCompressionFormat = OMX_VIDEO_CodingUnused;
   format.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
   /*
	if (OMX_SetParameter(video_render, OMX_IndexParamVideoPortFormat, &format) != OMX_ErrorNone) {
		puts("fucking error setting format on video_render");
		exit(-23);
	}
	* */
puts("Set Format parameters");
	
   // Where is the resolution?? -> In the port definition

	//status = ilclient_change_component_state(video_render, OMX_StateIdle);
	//assert(status == 0);

	OMX_PARAM_PORTDEFINITIONTYPE portdef;
	
	memset(&portdef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portdef.nVersion.nVersion = OMX_VERSION;
	portdef.nPortIndex = 90;
	// work out buffer requirements, check port is in the right state

	error = OMX_GetParameter(video_render, OMX_IndexParamPortDefinition, &portdef);
	
	portdef.format.video.nFrameWidth = 320;
	portdef.format.video.nFrameHeight = 240;
	portdef.format.video.nStride = 320;
	portdef.format.video.nSliceHeight = 240;
	
	error = OMX_SetParameter(video_render, OMX_IndexParamPortDefinition, &portdef);
	error = OMX_GetParameter(video_render, OMX_IndexParamPortDefinition, &portdef);
	
	puts("set port definition parameter");
	
	printf("Port: enabled=%d\n", portdef.bEnabled);
	
	OMX_SendCommand(video_render, OMX_CommandStateSet, OMX_StateIdle, NULL);
	puts("to idle state");
	
	// 
	
		int portIndex = 90;
	   error = OMX_SendCommand(video_render, OMX_CommandPortEnable, portIndex, NULL);
		assert(error == OMX_ErrorNone);
		
		int i;
		OMX_BUFFERHEADERTYPE* bufhdr;
		
		for (i = 0; i < portdef.nBufferCountActual; i++) {
			printf("Treating Buffer %d\n", i);
			//unsigned char *buf= vcos_malloc_aligned(portdef.nBufferSize, portdef.nBufferAlignment, "bufname");
			unsigned char *buf;
			posix_memalign(&buf, portdef.nBufferAlignment, portdef.nBufferSize);
			assert(buf && "Allocation Error");
			
		//	OMX_BUFFERHEADERTYPE* bufhdr;
	
			error = OMX_UseBuffer(video_render, &bufhdr, portIndex, 0x777, portdef.nBufferSize, buf);
			assert(error == OMX_ErrorNone);
		} 
		
		/*
	      //vcos_free(buf);
//849	      end = (OMX_BUFFERHEADERTYPE **) &((*end)->pAppPrivate);
//852	   // queue these buffers
//853	   vcos_semaphore_wait(&comp->sema);

//866	   vcos_semaphore_post(&comp->sema); */

	   if(i != portdef.nBufferCountActual) {
		   puts("wrong buffer counter");
		}
	
//869	      ilclient_wait_for_command_complete(comp, OMX_CommandPortEnable, portIndex) < 0)

// ERROR HANDLER
/*
870	   {
871	      ilclient_disable_port_buffers(comp, portIndex, NULL, ilclient_free, private);
872	
873	      // at this point the first command might have terminated with an error, which means that
874	      // the port is disabled before the disable_port_buffers function is called, so we're left
875	      // with the error bit set and an error event in the queue.  Clear these now if they exist.
876	      ilclient_remove_event(comp, OMX_EventError, 0, 1, 1, 0);
877	
878	      return -1;
879	   }
* */


	OMX_SendCommand(video_render, OMX_CommandStateSet, OMX_StateIdle, NULL);
	puts("to idle state");
	//sleep(1);
	// check component is in the right state to accept buffers
	error = OMX_GetState(video_render, &state);
	if (error != OMX_ErrorNone || state != OMX_StateIdle) {
		puts("video_render not in idle state");
	    return -1;
	}
	
	puts("to execute state");
	OMX_SendCommand(video_render, OMX_CommandStateSet, OMX_StateExecuting, NULL);
	sleep(1);
	error = OMX_GetState(video_render, &state);
	if (error != OMX_ErrorNone || state != OMX_StateExecuting) {
		puts("video_render not in executing state");
	    return -1;
	}
	//sleep(1);
	
      int first_packet = 1;

	while (1) {
      //while((buf = ilclient_get_input_buffer(video_render, 90, 1)) != NULL)
      //{
         // feed data and wait until we get port settings changed
         unsigned char *dest = bufhdr->pBuffer;

         data_len += fread(dest, 1, bufhdr->nAllocLen-data_len, in);

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
         if(!data_len)
            break;

         bufhdr->nFilledLen = data_len;
         data_len = 0;

         bufhdr->nOffset = 0;
         if(first_packet)
         {
            bufhdr->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
         }
         else
            bufhdr->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

         if(OMX_EmptyThisBuffer(video_render, bufhdr) != OMX_ErrorNone)
         {
			 puts("OMX_EmptyThisBuffer error");
            status = -6;
            break;
         }
      }

/*
      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

      if(OMX_EmptyThisBuffer(video_render, buf) != OMX_ErrorNone)
         status = -20;
*/
      // wait for EOS from render
//      ilclient_wait_for_event(video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
//                              ILCLIENT_BUFFER_FLAG_EOS, 10000);


   fclose(in);

   OMX_Deinit();

   //ilclient_destroy(client);
   return status;
   
}


int main (int argc, char **argv)
{
   if (argc < 2) {
      printf("Usage: %s <filename>\n", argv[0]);
      exit(1);
   }
   bcm_host_init();
   return video_decode_test(argv[1]);
}


