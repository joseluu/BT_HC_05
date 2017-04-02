#include "LockApp.h"
#include "main.h"
#include "stm32l4xx_hal.h"
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
		if (pSerialInFromBlueTooth->fgetsNonBlocking(strBT, 48)) {
			int len = strlen(strBT);
			strBT[len++] = '\r';
			strBT[len++] = '\0';
			pSerialOutToConsole->putsNonBlocking(strBT);
		}
		if (pSerialInFromConsole->fgetsNonBlocking(strConsole, 48)) {
			int len = strlen(strConsole);
			strConsole[len-1] = '\r';
			strConsole[len++] = '\n';
			strConsole[len++] = '\0';
			pSerialOutToBlueTooth->putsNonBlocking(strConsole);
		}

	}
}

bool isLockOpen(void) {
	bool bLockState = HAL_GPIO_ReadPin(LOCK_OPEN_GPIO_Port, LOCK_OPEN_Pin) == GPIO_PIN_SET;
	return bLockState;
}
void LockApp(void){

	InitializeSerial();
	HAL_StatusTypeDef statusReceive;
	HAL_StatusTypeDef statusTransmit;

	//simpleTalk();
	while (1){
		char buffer[100];
		if (pSerialInFromBlueTooth->fgetsNonBlocking(buffer, 98)) {
			if (strstr(buffer, "close") != NULL) {
				if (isLockOpen()) {
					pSerialOutToBlueTooth->putsNonBlocking("Failure: Lock is open, must lower manually\n");
				} else {
					lockClose();
					pSerialOutToBlueTooth->putsNonBlocking("Lock is has been closed\n");
				}
			} else if (strstr(buffer, "open") != NULL) {
				if (isLockOpen()) {
					pSerialOutToBlueTooth->putsNonBlocking("Lock is open already\n");
				} else {
					lockOpen();
					pSerialOutToBlueTooth->putsNonBlocking("Lock has been opened, now press side button\n");
				}
			} else if (strstr(buffer, "state") != NULL) {
				if (isLockOpen()) {
					pSerialOutToBlueTooth->putsNonBlocking("Lock is open\n");
				} else {
					pSerialOutToBlueTooth->putsNonBlocking("Lock is closed\n");
				}
			} else if (strstr(buffer, "ID") != NULL) {
				pSerialOutToBlueTooth->putsNonBlocking("0xbb7df185451d6f1aaada5066bcb254c7844911c3\n");
			} else {
				pSerialOutToBlueTooth->putsNonBlocking("?? meh ??\n");
			}
		}
	}






}