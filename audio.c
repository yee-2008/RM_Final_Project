#include "ff.h"
#include "i2s.h"

#define AUDIO_BUFFER_SIZE 256

FATFS fs;
FIL file;
FRESULT fres;
UINT bytesread;
int16_t audio_buffer[AUDIO_BUFFER_SIZE];
volatile uint8_t audio_ready=0;


void AudioInit(void){
    fres=f_mount(&fs,"0:",1);
    if (fres != FR_OK){
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
        return;
    }else{
        fres=f_open(&file,"0:a.wav",FA_READ);
        if (fres != FR_OK){
            HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
            return;
        }else{
            f_lseek(&file,44);
            f_read(&file,&audio_buffer[0],AUDIO_BUFFER_SIZE*sizeof(int16_t),&bytesread);
            audio_ready=1;
            HAL_I2S_Transmit_DMA(&hi2s2,(uint16_t*)audio_buffer,AUDIO_BUFFER_SIZE);
            HAL_I2S_DMAPause(&hi2s2);
            return;
        }
    }
    return;
}

void StartPlay(void){
    if (audio_ready){
        HAL_I2S_DMAResume(&hi2s2);
    }
    
    return;
}

void EndPlay(void){
    if (audio_ready){
        HAL_I2S_DMAPause(&hi2s2);
    }
    return;
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef * hi2s){
    if (hi2s->Instance==SPI2){
        f_read(&file,&audio_buffer[0],AUDIO_BUFFER_SIZE/2*sizeof(int16_t),&bytesread);
        if (bytesread/sizeof(int16_t)<AUDIO_BUFFER_SIZE/2 || f_eof(&file)){
            for (int i=bytesread/sizeof(int16_t);i<AUDIO_BUFFER_SIZE/2;i++){
                audio_buffer[i]=0;
            }
            f_lseek(&file,44);
        }
    }
}	

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef * hi2s){
    if (hi2s->Instance==SPI2){
        f_read(&file,&audio_buffer[AUDIO_BUFFER_SIZE/2],AUDIO_BUFFER_SIZE/2*sizeof(int16_t),&bytesread);
        if (bytesread/sizeof(int16_t)<AUDIO_BUFFER_SIZE/2 || f_eof(&file)){
            for (int i=bytesread/sizeof(int16_t)+AUDIO_BUFFER_SIZE/2;i<AUDIO_BUFFER_SIZE;i++){
                audio_buffer[i]=0;
            }
            f_lseek(&file,44);
        }
    }
}	
