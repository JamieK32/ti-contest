import sensor
import pyb
from pyb import UART
import ujson

# 初始化摄像头
def init_camera():
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QQVGA)
    sensor.skip_frames(10)
    sensor.set_auto_whitebal(False)
    return sensor

# 初始化串口
def init_uart(baudrate=115200):
    uart = UART(3, baudrate)
    return uart

# 查找最大Blob
def find_max_blob(blobs):
    max_size = 0
    max_blob = None
    for blob in blobs:
        blob_area = blob[2] * blob[3]
        if blob_area > max_size:
            max_blob = blob
            max_size = blob_area
    return max_blob


def largest_blob(img, threshold=(15, 75, 20, 90, 0, 60)):
    blobs = img.find_blobs([threshold])
    if blobs:
        largest_blob = find_max_blob(blobs)
        if largest_blob:
            img.draw_rectangle(largest_blob.rect(), color=(255, 0, 0), thickness=2)
            img.draw_cross(largest_blob.cx(), largest_blob.cy(), color=(0, 255, 0), thickness=1)

            return {
                "x": largest_blob.cx(),
                "y": largest_blob.cy()
            }
    return {"x": -1, "y": -1}

# 主函数
def main():
    # 初始化c
    sensor = init_camera()
    uart = init_uart(115200)
    while True:
        img = sensor.snapshot()
        data = largest_blob(img)
        json_data = ujson.dumps(data)
        uart.write(json_data)
        pyb.delay(20)



if __name__ == "__main__":
    main()
