#include <ucos_ii.h>
#include "cpu.h"
#include "Led_driver.h"
#include "ATD_driver.h"         
#include "Drivers/CAN_driver.h"
#include "Drivers/CAN/buffer.h"

#define TASK_STK_SIZE 128
OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK Task3Stk[TASK_STK_SIZE];
OS_STK Task4Stk[TASK_STK_SIZE];

/*
 * Task parameters.
 *
 * Note that:
 * - lower number means higher priority.
 * - RTI interrupt is set to expire with frequency of 1 kHz.
 */
  
#define TaskStartPrio      2
#define SpeedDataMutexPrio 6
#define Task3Prio          7
#define Task4Prio          8

/*
 * Message IDs in use on the CAN bus.
 */
 
#define BrakeId 0x288
#define EngineSpeedId 0x280

/*
 * List of message ids which Task3 is going to receive.
 */
 
INT32U idlist3[1] = { BrakeId }; 
INT32U idlist4[1] = { EngineSpeedId };

/*
 * Message id which Task3 is going to send.
 */
 
CAN_ID EngineSpeedId11;

/*
 * The queue, where the CAN driver is going to write pointers
 * to messages destined for Task3 and Task4.
 */

OS_EVENT* queue3;
OS_EVENT* queue4;

/* 
 * Memory space where queue3 and queue4 are going to store the pointers
 * to messages in the internal buffer of the CAN driver.
 */
 
void* queue3buf[10];   
void* queue4buf[10];

 
/*
 * Types used to mask the data field in the incoming and outgoing
 * CAN messages, simplifying the processing and formatting of the
 * incoming and outgoing data.
 */
 
typedef struct {
  INT16U data1;
  INT8U  footBrake : 1;     
  INT8U  data2     : 7;     
  INT8U  data3;
  INT16U data4;
  INT16U data5;
} BRAKE_DATA;

typedef struct {
  INT16U data1;
  INT16U engineSpeed;     
  INT16U data2;     
  INT16U data3;
} ENGINE_SPEED_DATA;

/*
 * Copy of the data field of the last received endinge speed message.
 */

ENGINE_SPEED_DATA SpeedData;       

/*
 * Mutex guarding the SpeedData variable, since it is shared between
 * Task3 and Task4.
 */
 
OS_EVENT* SpeedDataMutex;

/*
 * Utiltity method for copying the incoming engine speed data.
 */

void CopyEngineSpeedData(ENGINE_SPEED_DATA* src, ENGINE_SPEED_DATA* dst) {
  INT8U err;
  OSMutexPend(SpeedDataMutex, 0, &err);
  if (err == OS_ERR_NONE) {
    *dst = *src;
    OSMutexPost(SpeedDataMutex); 
  }
}

/*
 * Utility methods for extracting data from the incoming message and 
 * populating the outgoing message.
 */

INT8U ExtractFootbrakeFromData(INT8U* data) {
  BRAKE_DATA* mask = (BRAKE_DATA*)data;
  return mask->footBrake;  
}

void InsertEngineSpeedIntoData(INT16U speed, ENGINE_SPEED_DATA* data) {
  INT8U err;
  OSMutexPend(SpeedDataMutex, 0, &err);
  if (err == OS_ERR_NONE) {      
    data->engineSpeed = speed;
    OSMutexPost(SpeedDataMutex);
  }
}

/*
 * Task3 is triggered by an arriving message with id equal to BrakeId.
 * Upon message arrival it first toggles the LED on port D22. It then
 * extracts the footbrake value from the data in the incoming message 
 * and sets the LED on port D23 if and only if the value is 1. 
 * If the value was 1, then it sends a message with id EngineSpeedId
 * and the engine speed field populated with value 55. Otherwise it
 * send a message with id EngineSpeedId and the engine speed field
 * populated with value 10.
 */ 
 
void Task3(void* pArg){  
  CAN_MSG* msg;
  INT8U err;
  INT8U footbrake;

  while (OS_TRUE) {
    msg = (CAN_MSG*)(OSQPend(queue3, 0, &err));

    ToggleLed(LED_D22);    

    if (err == OS_ERR_NONE) { 
      footbrake=ExtractFootbrakeFromData(msg->data);
      if (footbrake ==1){
        ToggleLed(LED_D23);
        InsertEngineSpeedIntoData(55,&SpeedData);
        CANSendFrame(EngineSpeedId11,8,(INT8U*)&SpeedData);  
      }else{
        InsertEngineSpeedIntoData(10,&SpeedData);
        CANSendFrame(EngineSpeedId11,8,(INT8U*)&SpeedData);  
      }
    }
    CANForget(msg);
  }
}

/* 
 * Task4 receives engine speed messages and stores the most recent copy
 * of its data in the global variable SpeedData. This data is then used
 * by Task3 when sending the engine speed over CAN.
 */
 
void Task4(void* pArg) {
  CAN_MSG* msg;
  INT8U err;

  while (OS_TRUE) {
    msg = (CAN_MSG*)(OSQPend(queue4, 0, &err));

    ToggleLed(LED_D22);    

    if (CANLastRxError() != CAN_OK) {
      ToggleLed(LED_D27);
    }

    if (err == OS_ERR_NONE) {
      CopyEngineSpeedData((ENGINE_SPEED_DATA*)(msg->data), &SpeedData);
    } else {
      /* Toggle led if message could not be retrieved from queue. */
      ToggleLed(LED_D29);
    }
  
    CANForget(msg);
  }
}

void TaskStart(void *_unused) {
  /* Do not remove this code. The timer must be started after OSStart() has
     started the start task. */
  CPUStartTimer(); 
  
  /* Here you can register your tasks */
	OSTaskCreate(Task3, 0, &Task3Stk[TASK_STK_SIZE-1], Task3Prio);         
	OSTaskCreate(Task4, 0, &Task4Stk[TASK_STK_SIZE-1], Task4Prio);

	/* Do not remove this code. This task must remain active during the entire run. */
  for (;;){
    OSTimeDly(OS_TICKS_PER_SEC); 
  }
}

void main(void) {
  INT8U err;
  
  /* Initialize the outgoing message id here, so that they doesn't have to be 
     recreated each time the message is sent. */
  EngineSpeedId11 = CANId11(EngineSpeedId);
   
  /* Initialize uC/OS-II and create the startup task. */
	OSInit();
	OSTaskCreate(TaskStart, 0, &TaskStartStk[TASK_STK_SIZE-1], TaskStartPrio);
                                   
  /* Initialize the mutex. */                                 
  SpeedDataMutex = OSMutexCreate(SpeedDataMutexPrio, &err);

  /* Initialise the CAN driver. */
  CANInit();
  CANConfigureBaudrate(500000, 0);
  queue3 = OSQCreate(queue3buf, 10);  
  queue4 = OSQCreate(queue4buf, 10);
  CANRegister(1, idlist3, queue3);
  CANRegister(1, idlist4, queue4);
  CANStart(); 	

  /* Start the application. */
	OSStart();
}
