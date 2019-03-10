           
                                       /* ------------------------- RELTEQ --------------------------- */
#define OS_RELTEQ_EN              1    /* Enable (1) or Disable (0) Relteq support    				         */
#define OS_RELTEQ_HSF_EN          0    /* Enable (1) or Disable (0) hierarchical scheduling framework  */
                                       /* Requires OS_RELTEQ_EN.                                       */
#define OS_RELTEQ_POLLING_SERVER_EN    0  /* Enable (1) or Disable (0) polling server                  */
#define OS_RELTEQ_PERIODIC_SERVER_EN   0  /* Enable (1) or Disable (0) periodic server                 */
#define OS_RELTEQ_DEFERRABLE_SERVER_EN 0  /* Enable (1) or Disable (0) deferrable serve                */
#define OS_RELTEQ_CBS_SERVER_EN        0  /* Enable (1) or Disable (0) constant bandwidth server       */
#define OS_RELTEQ_GLOBAL_EDF_EN   0    /* (1) global EDF or (0) global FPS                             */
#define OS_RELTEQ_LOCAL_EDF_EN    0    /* (1) local EDF or (0) local FPS                               */

                                       /* Three variants are implemented for switching out a bandwidth
                                          preserving server (e.g. a deferrable servers):
                                          1. Keep the absolute queue switched in.
                                          2. Insert a wakeup event into the system queue for the 
                                             earliest event in the absolute queue, and process all
                                             past events upon a wakeup event.
                                          3. Insert a wakeup event into the system queue, and defer the
                                             handling of the events until the server is switched in.
                                        */
#define OS_RELTEQ_SERVER_WAKEUP_EVENT_EN          1  /* (0): variant 1, (1): variant 2 or 3            */
#define OS_RELTEQ_SERVER_WAKEUP_EVENT_DEFERRED_EN 1  /* (0): variant 2, (1): variant 3                 */
#define OS_RELTEQ_MAX_SERVERS     1    /* Don't forget the setup server */
#define OS_RELTEQ_MAX_EVENTS   100      /* Max. number of RELTEQ events                                 */
#define OS_RELTEQ_PERIODIC_TASK_EN     1  /* Enable (1) or Disable (0) periodic tasks                  */
#define OS_RELTEQ_SPORADIC_TASK_EN     0  /* Enable (1) or Disable (0) sporadic tasks                  */
#define OS_RELTEQ_STATS_EN             0  /* Enable (1) or Disable (0) collecting RELTEQ stats         */
#define OS_RELTEQ_NAME_SIZE       0   /* Max. size of a Relteq queue or server name                   */
#define OS_RELTEQ_DEBUG_EN        0    /* Enable (1) or Disable (0) debuggin functions                 */
#define OS_RELTEQ_NUM_CUSTOM_QUEUES 0 

#define OS_FPDS_EN 0