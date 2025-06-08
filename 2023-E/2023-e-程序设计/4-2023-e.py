from pyb import Servo  # 导入舵机模块
import pyb            # 用于延时
import sensor
import math


class TwoAxisServoControl:
    def __init__(self, servo_x_pin=1, servo_y_pin=2, x_init=0, y_init=-10,
                 x_limit=(-30, 30), y_limit=(-30, 30)):
        """
        初始化舵机设置

        Parameters:
            servo_x_pin: int - X轴舵机（默认连接到 Servo(1)）
            servo_y_pin: int - Y轴舵机（默认连接到 Servo(2)）
            x_init: int - X轴舵机的初始角度
            y_init: int - Y轴舵机的初始角度
            x_limit: tuple - X轴舵机角度限制 (最小值, 最大值)
            y_limit: tuple - Y轴舵机角度限制 (最小值, 最大值)
        """
        # 初始化舵机对象
        self.servo_x = Servo(servo_x_pin)
        self.servo_y = Servo(servo_y_pin)

        # 舵机当前角度值
        self.servo_x_value = x_init
        self.servo_y_value = y_init

        # 舵机角度限制
        self.servo_x_limit = x_limit
        self.servo_y_limit = y_limit

        # 设置初始位置
        self.servo_x.angle(self.servo_x_value)
        self.servo_y.angle(self.servo_y_value)

    def move_increment(self, x_increment, y_increment):
        """
        增量式移动舵机

        Parameters:
            x_increment: float - X轴增量（正值向右，负值向左）
            y_increment: float - Y轴增量（正值向上，负值向下）
        """
        # 计算新的角度值
        new_x = self.servo_x_value + x_increment
        new_y = self.servo_y_value + y_increment

        # X轴限位检查和移动
        if self.servo_x_limit[0] < new_x < self.servo_x_limit[1]:
            self.servo_x_value = new_x
            self.servo_x.angle(self.servo_x_value)

        # Y轴限位检查和移动
        if self.servo_y_limit[0] < new_y < self.servo_y_limit[1]:
            self.servo_y_value = new_y
            self.servo_y.angle(self.servo_y_value)

    def reset(self):
        """
        将舵机复位到初始位置
        """
        self.servo_x_value = 0
        self.servo_y_value = -10
        self.servo_x.angle(self.servo_x_value)
        self.servo_y.angle(self.servo_y_value)
        pyb.delay(100)  # 延时0.5秒以稳定



class IncrementalPID:
    def __init__(self, kp, ki, kd, output_limits=(None, None), deadzone=3, filter_alpha=0.7):
        """
        初始化增量式PID参数
        Parameters:
            kp: float - 比例增益
            ki: float - 积分增益
            kd: float - 微分增益
            output_limits: tuple - 输出限制 (最小值, 最大值)
            deadzone: float - 死区大小，误差小于此值时输出为0
            filter_alpha: float - 低通滤波系数 (0-1)，越小滤波越强
        """
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.min_output, self.max_output = output_limits
        self.deadzone = deadzone

        # 低通滤波参数
        self.filter_alpha = filter_alpha
        self.filtered_error = 0  # 滤波后的误差

        # 存储前两次误差（使用滤波后的误差）
        self.prev_error = 0      # e(k-1)
        self.prev_prev_error = 0 # e(k-2)
        self.first_run = True
        self.second_run = True

    def compute(self, error):
        """
        计算增量式PID输出
        Parameters:
            error: float - 当前误差值
        Returns:
            float - PID控制增量输出
        """
        # 1. 低通滤波处理误差
        if self.first_run:
            # 第一次运行，直接使用当前误差作为滤波初值
            self.filtered_error = error
        else:
            # 低通滤波公式：filtered = α × new + (1-α) × old
            self.filtered_error = (self.filter_alpha * error +
                                 (1 - self.filter_alpha) * self.filtered_error)

        # 2. 死区检查（使用滤波后的误差）
        if abs(self.filtered_error) < self.deadzone:
            return 0

        # 3. 计算增量式PID输出（使用滤波后的误差）
        if self.first_run:
            # 第一次运行，只有比例项
            delta_output = self.kp * self.filtered_error
            self.first_run = False
        elif self.second_run:
            # 第二次运行，有比例项和积分项
            delta_output = (self.kp * (self.filtered_error - self.prev_error) +
                          self.ki * self.filtered_error)
            self.second_run = False
        else:
            # 增量式PID公式：Δu(k) = Kp[e(k)-e(k-1)] + Ki*e(k) + Kd[e(k)-2e(k-1)+e(k-2)]
            delta_output = (self.kp * (self.filtered_error - self.prev_error) +
                           self.ki * self.filtered_error +
                           self.kd * (self.filtered_error - 2 * self.prev_error + self.prev_prev_error))

        # 4. 更新误差历史（使用滤波后的误差）
        self.prev_prev_error = self.prev_error
        self.prev_error = self.filtered_error

        # 5. 输出限幅
        if self.min_output is not None and delta_output < self.min_output:
            delta_output = self.min_output
        if self.max_output is not None and delta_output > self.max_output:
            delta_output = self.max_output

        return delta_output


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

def draw_rectangle_with_path(img, points, color=(255, 0, 0)):
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


if __name__ == "__main__":

    # 初始化舵机控制器
    my_servo = TwoAxisServoControl()
    my_servo.reset()

    # 初始化摄像头
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames(time=2000)

    rectangle_points = None

    while True:
        img = sensor.snapshot()

        for r in img.find_rects(threshold=10000):
            img.draw_rectangle(r.rect(), color=(255, 0, 0))
            rectangle_points = r.corners()
            break  # 跳出 for 循环

        if rectangle_points is not None:
            break  # 跳出 while 循环

    # rectangle_points = [
    #     (90, 57),   # 左上角
    #     (223, 63),  # 右上角
    #     (218, 193), # 右下角
    #     (90, 187)   # 左下角
    # ]

    target_x, target_y = rectangle_points[0]

    # target_x, target_y = get_rectangle_center(rectangle_points)

    # 初始化增量式PID控制器
    # 注意：增量式PID的参数调节相对更敏感，建议从小的值开始调试
    x_pid = IncrementalPID(
        kp=0.08,   # 比例增益，控制响应速度
        ki=0.04,  # 积分增益，消除稳态误差
        kd=0.05,  # 微分增益，减少超调
        output_limits=(-50, 50)  # 限制单次移动增量，避免舵机移动过大
    )

    y_pid = IncrementalPID(
        kp=0.08,
        ki=0.04,
        kd=0.05,
        output_limits=(-50, 50)
    )

    print("激光追踪系统启动...")
    print("目标位置: ({}, {})".format(target_x, target_y))

    last_time = 0   # 上一次时间
    idx = 0         # 当前遍历到的数组下表
    interval = 5000 # 第一次运行时需要5秒

    while True:
        img = sensor.snapshot()
        current_time = pyb.millis()

        # 绘制中心十字线
        points = draw_rectangle_with_path(img, rectangle_points, color=(0, 255, 0))

        # 寻找激光点
        laser_pos = find_laser_point(img)

        if laser_pos:
            laser_x, laser_y = laser_pos

            # 绘制激光点
            img.draw_circle(laser_x, laser_y, 8, color=(255, 255, 0))

            # 计算误差（目标位置 - 当前位置）
            error_x = target_x - laser_x
            error_y = target_y - laser_y

            # 计算PID增量输出
            x_increment = x_pid.compute(error_x)
            y_increment = y_pid.compute(error_y)

            # 增量式控制舵机移动
            # 注意：这里x_increment需要取反，因为舵机X轴移动方向与图像坐标可能相反
            my_servo.move_increment(-x_increment, -y_increment)

            if current_time - last_time >= interval:
                last_time = current_time
                target_x, target_y = points[idx]
                idx = (idx + 1) % len(points)
                interval = 100

        else:
            # 如果没有检测到激光点，重置PID状态
            img.draw_string(10, 10, "No laser detected", color=(255, 0, 0))

        pyb.delay(30)  # 适当延时，避免控制频率过高


