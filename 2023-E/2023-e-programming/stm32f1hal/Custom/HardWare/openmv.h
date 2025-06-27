#ifndef OPENMV_H__
#define OPENMV_H__

#define UART_BUFFER_SIZE 128      

void parse_openmv_data(const char *data);
void process_received_data(void);

#endif 
