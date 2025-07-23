#ifndef OPENMV_H__
#define OPENMV_H__

#define UART_BUFFER_SIZE 2048      
#define MAX_POINTS 100 
#define DIMENSIONS 2 
#define MAX_PATH_POINTS 500
#define MAX_EDGE_POINTS 100
#define DEFAULT_STEP 5



void openmv_init(void);
void parse_openmv_data(void);
void process_received_data(void);

extern float err_x;
extern float err_y;




#endif 
