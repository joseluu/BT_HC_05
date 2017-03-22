#include "LockApp.h"
#include "main.h"
#include "stm32f3xx_hal.h"
#include "Serial.h"
#include "gpio.h"
#include <cstring>

void lockOpen(void){
  /* lock open */
	HAL_GPIO_WritePin(LOCK_P_GPIO_Port, LOCK_P_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LOCK_P_GPIO_Port, LOCK_P_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(LOCK_P_GPIO_Port, LOCK_P_Pin, GPIO_PIN_RESET);
	}

void lockClose(void){
	/* lock close */
	HAL_GPIO_WritePin(LOCK_M_GPIO_Port, LOCK_M_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LOCK_M_GPIO_Port, LOCK_M_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(LOCK_M_GPIO_Port, LOCK_M_Pin, GPIO_PIN_RESET); 
	}
	
static 	SerialOutput *pSerialOutToBlueTooth;      // via bluetooth
static SerialOutput *pSerialOutToConsole;	 // via USB
static SerialInput * pSerialInFromBlueTooth;
static SerialInput * pSerialInFromConsole;

void InitializeSerial(){
#define SERIAL_BUFFER_SIZE 50
	static char bufferOutBlueTooth[SERIAL_BUFFER_SIZE];
	static char bufferOutConsole[SERIAL_BUFFER_SIZE];

	static char bufferInBlueTooth[SERIAL_BUFFER_SIZE];
	static char bufferInConsole[SERIAL_BUFFER_SIZE];

	pSerialOutToBlueTooth = new SerialOutput(&huart1, bufferOutBlueTooth, SERIAL_BUFFER_SIZE);       // via bluetooth
	pSerialOutToConsole = new SerialOutput(&huart2, bufferOutConsole, SERIAL_BUFFER_SIZE);	 // via USB
	pSerialInFromBlueTooth = new SerialInput(&huart1, bufferInBlueTooth, SERIAL_BUFFER_SIZE);
	pSerialInFromConsole = new SerialInput(&huart2, bufferInConsole, SERIAL_BUFFER_SIZE);


}

void simpleTalk(){
	char strBT[50];
	char strConsole[50];
	while (1){
		if (pSerialInFromBlueTooth->fgetsNonBlocking(strBT, 50)) {
			pSerialOutToConsole->putsNonBlocking(strBT);
		}
		if (pSerialInFromConsole->fgetsNonBlocking(strConsole, 50)) {
			pSerialOutToBlueTooth->putsNonBlocking(strConsole);
		}

	}
}

void LockApp(void){
	HAL_StatusTypeDef statusReceive;
	HAL_StatusTypeDef statusTransmit;

	while (1){
		char buffer[100];
		statusReceive = HAL_UART_Receive(&huart1, (unsigned char*)buffer, 100, 1000);
		if (statusReceive == HAL_OK) {
			if (strstr("close",buffer) != NULL) {
				lockClose();
				statusTransmit = HAL_UART_Transmit(&huart1, (uint8_t*)"Lock is closed\n", 15, 1000);
			} else if (strstr(buffer,"open") != NULL) {
				lockOpen();
				statusTransmit = HAL_UART_Transmit(&huart1, (uint8_t*)"Lock is open\n", 13, 1000);
			}
		}
	}






}