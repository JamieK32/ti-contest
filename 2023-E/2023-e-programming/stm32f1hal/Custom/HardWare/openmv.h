#ifndef OPENMV_H__
#define OPENMV_H__

#define UART_BUFFER_SIZE 1024      
#define MAX_POINTS 100 
#define DIMENSIONS 2 

typedef enum {
	CAM_STATE_NONE,
	RECEIVE_XY,
	RECEIVE_PATH_POINTS,
	RECEIVE_CENTER_POINT
} CAM_STATE; 

void parse_openmv_data(const char *data);
void process_received_data(void);

extern float x_value; //当前激光点的X坐标
extern float y_value; //当前激光点的y坐标
extern float center_point_x;
extern float center_point_y;
extern int points_array[MAX_POINTS][DIMENSIONS]; //方框路径坐标
extern int num_points;	//方框路径数量


#endif 
