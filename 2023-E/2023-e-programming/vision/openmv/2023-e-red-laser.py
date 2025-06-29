import pyb
import sensor
import math
import ujson

class CAM_STATE:
    TASK1 = 1
    TASK2 = 2


def find_laser_point(img, red_threshold=(20, 100, 10, 127, 0, 30)):
    """
    寻找激光点位置
    """
    blobs = img.find_blobs([red_threshold], pixels_threshold=1, area_threshold=1, merge=True)
    if blobs:
        # 选择面积最大的色块
        largest_blob = max(blobs, key=lambda b: b.pixels())
        return largest_blob.cx(), largest_blob.cy()
    return None

def draw_rectangle_with_path(img, points, color=(0, 0, 255)):
    """
    根据给定的四个点绘制矩形，并返回沿着矩形边缘的离散坐标点（间隔10），首尾相连

    参数:
    img: 图像对象
    points: 四个点的坐标列表 [(x1,y1), (x2,y2), (x3,y3), (x4,y4)]，按顺序连接
    color: 绘制颜色，默认红色

    返回:
    path_points: 沿矩形边缘的离散坐标点列表
    """

    # 绘制矩形（通过连接四条边）
    for i in range(4):
        start_point = points[i]
        end_point = points[(i + 1) % 4]  # 连接到下一个点，最后一条边连接回第一个点
        # line_tuple格式: (x0, y0, x1, y1)
        line_tuple = (start_point[0], start_point[1], end_point[0], end_point[1])
        img.draw_line(line_tuple, color=color)

    # 生成沿边缘的路径点
    path_points = []
    step = 5

    # 对每条边生成离散点
    for i in range(4):
        start_point = points[i]
        end_point = points[(i + 1) % 4]

        # 生成这条边上的点
        edge_points = generate_points_on_edge(start_point, end_point, step)

        if i == 0:
            # 第一条边包含起始点
            path_points.extend(edge_points)
        else:
            # 其他边跳过起始点以避免重复
            path_points.extend(edge_points[1:])

    # 最终回到起始点，确保首尾相连
    if path_points and path_points[-1] != points[0]:
        path_points.append(points[0])

    return path_points


def generate_points_on_edge(start_point, end_point, step):
    """
    在两个点之间生成等间距的离散点

    参数:
    start_point: 起始点 (x, y)
    end_point: 结束点 (x, y)
    step: 步长间距

    返回:
    points: 该边上的离散点列表
    """
    x1, y1 = start_point
    x2, y2 = end_point

    # 计算两点间的距离
    distance = math.sqrt((x2 - x1)**2 + (y2 - y1)**2)

    # 如果距离太小，直接返回起始点和结束点
    if distance < step:
        return [start_point, end_point]

    # 计算需要的步数
    num_steps = int(distance / step)

    points = []

    # 生成中间点
    for i in range(num_steps + 1):
        t = i / num_steps if num_steps > 0 else 0
        x = x1 + t * (x2 - x1)
        y = y1 + t * (y2 - y1)
        points.append((int(round(x)), int(round(y))))

    # 确保最后一个点是准确的结束点
    if points[-1] != end_point:
        points.append(end_point)

    return points

def get_rectangle_center(rectangle_points):
    """
    使用对角线交点法计算矩形中心点

    参数:
        rectangle_points: 矩形四个顶点坐标列表，格式为 [(x1,y1), (x2,y2), (x3,y3), (x4,y4)]
                         按顺序为：左上角、右上角、右下角、左下角

    返回:
        tuple: 中心点坐标 (center_x, center_y)
    """
    # 对角线1: 左上角 -> 右下角
    diagonal1_start = rectangle_points[0]  # 左上角
    diagonal1_end = rectangle_points[2]    # 右下角

    # 计算对角线1的中点，即为矩形中心
    center_x = (diagonal1_start[0] + diagonal1_end[0]) / 2
    center_y = (diagonal1_start[1] + diagonal1_end[1]) / 2

    return (center_x, center_y)


def get_inner_corners(corners, shrink_ratios=[0.92, 0.96, 0.9, 0.93]):
    """获取内框的四个角点坐标 - 每个角可以单独设置缩放比例"""
    # 计算中心点
    x_coords = [point[0] for point in corners]
    y_coords = [point[1] for point in corners]
    center_x = sum(x_coords) / len(x_coords)
    center_y = sum(y_coords) / len(y_coords)

    inner_corners = []
    for i, point in enumerate(corners):
        x, y = point
        # 使用对应角的缩放比例
        ratio = shrink_ratios[i] if i < len(shrink_ratios) else 0.8
        new_x = int(center_x + (x - center_x) * ratio)
        new_y = int(center_y + (y - center_y) * ratio)
        inner_corners.append((new_x, new_y))

    return inner_corners

#二三问识别矩形并且返回脚点坐标
def detect_box():
    while True:
        img = sensor.snapshot()
        for r in img.find_rects(threshold=10000):
            img.draw_rectangle(r.rect(), color=(255, 0, 0))

            # 外框角点
            rectangle_points = r.corners()
            rectangle_points = list(reversed(rectangle_points))

            # 内框角点
            rectangle_points = get_inner_corners(rectangle_points)

            break
        if rectangle_points is not None:
            break
    return rectangle_points

#第一问默认矩形角点坐标
def default_coord():
    rectangle_points = [
        (97, 57),   # 左上角
        (223, 63),  # 右上角
        (218, 193), # 右下角
        (90, 187)   # 左下角
    ]
    return rectangle_points

def parse_uart_cmd(uart):
    """
    从 UART 接收并解析命令数据。
    检查是否有指令，比如 "TASK1"，并返回对应的命令。
    参数:
    uart: UART 对象，用于串口通信
    返回:
    str: 接收到的命令字符串（如 "TASK1"），如果没有有效命令返回 None
    """
    if uart.any():  # 检查 UART 是否有数据
        try:
            # 从 UART 中读取数据
            data = uart.readline()
            if data is not None:
                # 解码数据为 UTF-8 字符串，去除换行符
                command = data.decode('utf-8').strip()
                return command
        except Exception as e:
            print("UART解析错误: ", e)
    return None


if __name__ == "__main__":
    # 实例化 UART 驱动
    uart_driver = pyb.UART(1, 115200)
    # 初始化摄像头
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames(time=2000)
    # 获取默认矩形坐标
    points = default_coord()
    while True:
        # 截取图像
        img = sensor.snapshot()
        # 绘制默认的矩形框
        draw_rectangle_with_path(img, points)
        # 调用封装的 UART 命令解析函数
        command = parse_uart_cmd(uart_driver)
        if command == "TASK0":
            center_x, center_y = get_rectangle_center(points)
            data = {"x" : center_x,
                    "y" : center_y}
            json_data = ujson.dumps(data)
            uart_driver.write(json_data + '\n')
        elif command == "TASK1":
            # 生成矩形边缘点路径
            path_points = draw_rectangle_with_path(img, points)
            # 将点数据打包为 JSON 格式
            json_data = ujson.dumps({"path_points": path_points})
            # 通过 UART 发送 JSON 数据
            uart_driver.write(json_data + '\n')
        pyb.delay(20)  # 合适的延时，避免过高的帧率
