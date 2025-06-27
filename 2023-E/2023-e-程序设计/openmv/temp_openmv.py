import sensor, image, time, math
from pyb import UART
import json

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

# 数据打包函数
def pack_data(x, y):
    # 限制坐标范围
    x = max(0, min(x, 160))
    y = max(0, min(y, 120))  # 假设图像高度为240
    # 打包数据
    data = bytearray([0x2C, 7, x, y, 3, 4, 0x5B])
    return data

# 主函数
def main():
    # 初始化
    sensor = init_camera()
    uart = init_uart(115200)
    clock = time.clock()
    threshold = (15, 75, 20, 90, 0, 60)  # 颜色阈值

    while True:
        clock.tick()
        img = sensor.snapshot()
        img.lens_corr(1.8)  # 矫正镜头畸变

        # 检测Blob
        blobs = img.find_blobs([threshold])
        if blobs:
            largest_blob = find_max_blob(blobs)
            if largest_blob:
                # 绘制检测结果
                img.draw_rectangle(largest_blob.rect(), color=(255, 0, 0), thickness=2)
                img.draw_cross(largest_blob.cx(), largest_blob.cy(), color=(0, 255, 0), thickness=1)

                # 打包数据并发送
                try:
                    data = pack_data(largest_blob.cx(), largest_blob.cy())
                    uart.write(data)
                    print("Sent data:", data)
                except Exception as e:
                    print("UART write error:", e)
        else:
            # 如果未检测到Blob，发送默认数据
            try:
                default_data = bytearray([0x2C, 7, 0, 0, 3, 4, 0x5B])
                uart.write(default_data)
                print("Sent default data:", default_data)
            except Exception as e:
                print("UART write error:", e)


if __name__ == "__main__":
    main()
