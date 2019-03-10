
#define ENABLE_SRP                  0    /* Enable (1) or Disable (0) SRP implementation of mutexes      */


#if ENABLE_SRP > 0
  #undef  OS_MUTEX_EN                    /* To reuse the uC/OS-II interface for mutex, we need to        */
  #define OS_MUTEX_EN               0    /* disable the original implementation                          */                                            
  #define OS_MUTEX_SRP_EN           1    /* Enable (1) or Disable (0) code generation for SRP mutexes    */

  #define OS_MUTEX_SIRAP_EN         0    /* Include code for 2-level SRP by means of SIRAP               */
  #define OS_SIRAP_SPINLOCK_EN      0    /* Include code for 2-level SRP by means of SIRAP               */
  #define OS_MUTEX_OVERRUN_EN       0    /* Include code for 2-level SRP by means of overrun             */
  #define OS_MUTEX_BROE_EN          0    /* Include code for 2-level SRP by means of BROE (requires EDF) */
  #define MAX_RESOURCES            32    /* Define max number of resources on the SRP stack              */

  #define OS_MUTEX_HSTP_EN          0    /* Include code for 2-level SRP by means of HSTP                */
  #define OS_TASK_NAME_SIZE         0
  #define MAX_INT8U               255
#else
  #define Resource OS_EVENT              /* Redirect the resource type to OS_EVENT                       */
  #define OS_MUTEX_SRP_EN           0    /* Enable (1) or Disable (0) code generation for SRP mutexes    */
#endif
