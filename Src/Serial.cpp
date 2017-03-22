#include "Serial.h"
#include "main.h"
#include <string.h>

SerialInput * SerialInput::channel_1 = NULL;
SerialInput * SerialInput::channel_2 = NULL;
SerialOutput * SerialOutput::channel_1 = NULL;
SerialOutput * SerialOutput::channel_2 = NULL;

SerialInput::SerialInput(UART_HandleTypeDef * pHandle, char * buffer, unsigned int size) : pHandle(pHandle), driverBuffer (buffer), driverBufferSize (size)
{

	driverBufferNextChar = buffer;
	nChars = 0;
	eol = false;
	overrun = 0;
	inputBuffer[0] = 0;
	inputBuffer[1] = 0;

	if (HAL_UART_Receive_IT(pHandle, (uint8_t *)inputBuffer, sizeof(inputBuffer)) != HAL_OK) {
		Error_Handler();
	}
	if (pHandle == &huart1) {
		SerialInput::channel_1 = this;
	} else if (pHandle == &huart2) {
		SerialInput::channel_2 = this;
	} else {
		Error_Handler();
	}
}

// append terminating 0, returns the newline and clip to size characters
char * SerialInput::fgets(char * str, int size)
{
	while (!eol && ((driverBufferNextChar - driverBuffer) < size)) {
		HAL_Delay(1);
	}
	return fgetsNonBlocking(str, size);
}

char *  SerialInput::fgetsNonBlocking(char * str, int size)
{
	while (!eol && ((driverBufferNextChar - driverBuffer) < size)) {
		return NULL;
	}
	int sizeUseful;
	if ((driverBufferNextChar - driverBuffer) >= size) {
		sizeUseful = size - 1;
		strncpy(driverBuffer, str, sizeUseful);
		str[sizeUseful] = 0;
		goto cleanup;
	} else {
		sizeUseful = driverBufferNextChar - driverBuffer;
		strncpy(driverBuffer, str, sizeUseful);
		str[sizeUseful] = 0;
	}
cleanup:
	driverBufferNextChar = driverBuffer;
	nChars = 0;
	overrun = 0;
	eol = false;
	return str;
}

void SerialInput::doInputIT(void)
{
	if ((driverBufferNextChar - driverBuffer) >= driverBufferSize) {
		overrun++;
	} else {
		*driverBufferNextChar++ = inputBuffer[0];
		nChars++;
		if (inputBuffer[0] == '\n') {
			eol = true;
		}
	}

}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	SerialInput * serialObject;
	if (UartHandle == &huart1) {
		serialObject  = SerialInput::channel_1;
	} else if (UartHandle == &huart2) {
		serialObject  = SerialInput::channel_2;
	} else {
		Error_Handler();
	}
	serialObject->doInputIT();
}

SerialOutput::SerialOutput(UART_HandleTypeDef * pHandle, char * driverBuffer, unsigned int size)
								:pHandle(pHandle),driverBuffer(driverBuffer),driverBufferSize(size)
{
	busy = false;
	if (pHandle==&huart1){
		SerialOutput::channel_1 = this;
	}else if(pHandle == &huart2){
		SerialOutput::channel_2 = this;
	} else {
		Error_Handler();
	}
}

bool SerialOutput::puts(char * str){
	int statusTransmit;
	busy = true;
	if (strlen(str) >= driverBufferSize) {
	}
	statusTransmit = HAL_UART_Transmit(pHandle, (uint8_t*)str, strlen(str), 1000);
	busy = false;
	return statusTransmit;
}

bool SerialOutput::putsNonBlocking(char * str)
{
	int statusTransmit;
	if (busy) {
		return false;
	}
	busy = true;
	if (strlen(str) >= driverBufferSize) {
	}
	statusTransmit = HAL_UART_Transmit_IT(pHandle, (uint8_t*) str, strlen(str));
	return statusTransmit;
}

void SerialOutput::doOutputIT(){
	busy = false;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	SerialOutput * serialObject;
	if (UartHandle == &huart1) {
		serialObject  = SerialOutput::channel_1;
	} else if (UartHandle == &huart2) {
		serialObject  = SerialOutput::channel_2;
	} else {
		Error_Handler();
	}
	serialObject->doOutputIT();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	Error_Handler();
}